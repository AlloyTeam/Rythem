#include "rypipedata.h"
#include <QDebug>
#include <QList>
#include <QStringList>


RyPipeData::RyPipeData(quint64 connectionId,QObject *parent):
    QObject(parent){
    socketConnectionId = connectionId;
}


int RyPipeData::parseRequest(QByteArray* request,bool* isRequestOk){
    // get header body devider
    int deviderSize=4;
    int deviderIndex = request->indexOf("\r\n\r\n");
    if(deviderIndex==-1){
        deviderSize = 2;
        deviderIndex =request->indexOf("\n\n");
    }
    if(deviderIndex == -1){// no devider
        _dataToSend.clear();
        *isRequestOk = false;
        return -1;
    }
    QByteArray headerBa = request->left(deviderIndex);
    if( ! parseRequestHeader(headerBa) ){// invalid header
        _dataToSend.clear();
        *isRequestOk = false;
        return -1;
    }
    // eat the header bytes
    request->remove(0,deviderIndex+deviderSize);

    //qDebug()<<_requestHeaders;
    *isRequestOk = true;
    if(appendRequestBody(request)){
        return 0;
    }else{
        return _requestBodyRemain;
    }
}

int RyPipeData::parseResponse(QByteArray* response,bool* isResponseOk){
    // get header body devider
    // TODO some http server response doesn't contains\r\n\r\n
    if(response->isEmpty()){
        qDebug()<<"response empty";
        *isResponseOk = false;
        return -1;
    }
    int deviderSize=4;
    int deviderIndex = response->indexOf("\r\n\r\n");
    if(deviderIndex==-1){
        deviderSize = 2;
        deviderIndex =response->indexOf("\n\n");
    }
    if(deviderIndex == -1){// no devider
        //qDebug()<<"response has no devider";//<<response->at(0);
        if(isResponseOk){
            *isResponseOk = false;
        }
        return -1;
    }
    QByteArray headerBa = response->left(deviderIndex);
    if( ! parseResponseHeader(headerBa) ){// invalid header
        if(isResponseOk){
            *isResponseOk = false;
        }
        return -1;
    }
    //qDebug()<<_requestHeaders;
    if(isResponseOk){
        *isResponseOk = true;
    }
    response->remove(0,deviderIndex+deviderSize);
    if(appendResponseBody(response)){
        return 0;
    }else{
        if(_isResponseChunked){
            return 1;
        }
        return _responseBodyRemain;
    }
}
bool RyPipeData::parseRequestHeader(const QByteArray& headers){
    QByteArray tmp = headers;
    tmp.replace("\r\n","\n");
    QList<QByteArray> headerLines = tmp.split('\n');
    if(headerLines.length() == 0){
        return false;
    }

    // parse "GET / HTTP/1.1"
    QByteArray firstLine = headerLines.takeAt(0);
    firstLine = firstLine.simplified();
    QList<QByteArray> sigs = firstLine.split(' ');
    if(sigs.length()<2){// maybe like this "CONNECT github.com:443"
        return false;
    }
    method = sigs[0];
    fullUrl = sigs[1];
    if(sigs.length()>2){
        httpVersion = sigs[2];
    }
    QString metodToUpper = method.toUpper() ;
    if(metodToUpper      != "OPTIONS" &&
            metodToUpper != "GET"     &&
            metodToUpper != "POST"    &&
            metodToUpper != "HEAD"    &&
            metodToUpper != "PUT"     &&
            metodToUpper != "DELETE"  &&
            metodToUpper != "TRACE"   &&
            metodToUpper != "CONNECT"){
        // see http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html
        return false;
    }
    path = "/";
    port = 80;//TODO

    QString withouProtocol = fullUrl;
    int indexOfHost = fullUrl.indexOf("://");
    if(indexOfHost!=-1){
        withouProtocol = fullUrl.mid(indexOfHost+3);
    }

    if(indexOfHost==-1){
        // request after CONNECT tunnel
        path = fullUrl;
    }else{
        QString hostAndPort = withouProtocol;
        int indexOfPath = withouProtocol.indexOf("/");
        if(indexOfPath!=-1){
            hostAndPort = withouProtocol.left(indexOfPath);
            path = withouProtocol.mid(indexOfPath);
        }
        int indexOfPort = hostAndPort.indexOf(":");
        host = hostAndPort;
        if(indexOfPort!=-1){
            host = hostAndPort.left(indexOfPort);
            port = hostAndPort.mid(indexOfPort+1).toInt();
        }
        if(method == "CONNECT"){
            fullUrl.prepend("http://");
        }
    }

    //qDebug()<<"host="<<host<<port;
    QString sigsToSend = method+" "+path+" "+httpVersion+"\r\n";
    _dataToSend.append(sigsToSend);
    parseHeaders(headerLines,&_requestHeaders);
    foreach(QString headerName,_requestHeaders.keys()){
        QString value=_requestHeaders[headerName];
        if(headerName=="Proxy-Connection"){
            headerName = "Connection";
        }

        if(headerName == "Host"){
            int d = value.indexOf(":");
            if(d!=-1){
                host = value.left(d);
                port = value.mid(d+1).trimmed().toInt();
            }else{
                host = value;
                port = 80;
            }
        }

        _dataToSend.append(headerName);
        _dataToSend.append(": ");
        _dataToSend.append(value);
        _dataToSend.append("\r\n");
    }

    if(path == fullUrl){
        if(method!="CONNECT"){
            if(port == 80){
                fullUrl.prepend(QString("http://").append(host));
            }else{
                fullUrl.prepend(
                            QString("http://")
                            .append(host)
                            .append(":")
                            .append(QString("").setNum(port)));
            }
        }
    }

    _dataToSend.append("\r\n");
    _requestBodyRemain = 0;
    if(_requestHeaders.keys().contains("Content-Length")){
        _requestBodyRemain = _requestHeaders["Content-Length"].toULongLong();
    }
    _requestHeaderRawData = headers;
    //qDebug()<<"requestHeaders:"<<_requestHeaders;
    return true;
}

bool RyPipeData::parseResponseHeader(const QByteArray& headers){
    QByteArray tmp = headers;
    tmp.replace("\r\n","\n");
    QList<QByteArray> headerLines = tmp.split('\n');
    if(headerLines.length() == 0){
        qDebug()<<"header line count=0\n"<<headers;
        return false;
    }

    // parse "HTTP/1.1 404 NOT FOUND"
    QByteArray firstLine = headerLines.takeAt(0);
    firstLine = firstLine.simplified();
    QList<QByteArray> sigs = firstLine.split(' ');
    if(sigs.length()<2){//some server return only HTTP/1.1 304
        qDebug()<<"sig length<2\n"<<firstLine;
        return false;
    }
    responseStatus = sigs[1];
    parseHeaders(headerLines,&_responseHeaders);
    bool hasContentLength;
    _responseBodyRemain = _responseHeaders.value("Content-Length","-1").toULongLong(&hasContentLength);
    //qDebug()<<"_respnoseBodyRemain"<<_responseBodyRemain<<_responseHeaders.value("Content-Length","-1");
    if(hasContentLength){
        _isContentLenthUnLimit = false;
    }else{
        _isContentLenthUnLimit = true;
    }
    _isResponseChunked = false;
    if(_responseHeaders.keys().contains("Transfer-Encoding")){
        if(_responseHeaders.value("Transfer-Encoding","other")=="chunked"){
            _isResponseChunked = true;
            _isContentLenthUnLimit = false;
            _lastChunkSize = -1;
        }
    }
    _responseHeaderRawData = headers;
    //qDebug()<<"requestHeaders:"<<_requestHeaders;
    return true;

}


const QByteArray& RyPipeData::dataToSend()const{
    return _dataToSend;
}

bool RyPipeData::appendRequestBody(QByteArray* newData){
    if(_requestBodyRemain>=newData->length()){
        _requestBody.append(*newData);
        _dataToSend.append(*newData);
        _requestBodyRemain -= newData->length();
        newData->clear();
    }else{
        _dataToSend.append(newData->left(_requestBodyRemain));
        _requestBody.append(newData->left(_requestBodyRemain));
        _requestBodyRemain = 0;
        newData->remove(0,_requestBodyRemain);
        qDebug()<<"RyPipeData::appendRequestBody"
                <<"   should not happend here!!";
    }
    return _requestBodyRemain<=0;
}


int RyPipeData::readChunk(int chunkSize,QByteArray* chunkData,QByteArray* unChunkDes){
    int sourceSize = (*chunkData).size();
    if(sourceSize < chunkSize){
        (*unChunkDes).append(*chunkData);
        (*chunkData).clear();
        return sourceSize;
    }else{
        QByteArray ba = (*chunkData).left(chunkSize);
        (*unChunkDes).append(chunkData->left(chunkSize));
        (*chunkData).remove(0,chunkSize);
        return chunkSize;
    }
}
int RyPipeData::getChunkSize(QByteArray* chunkedData){
    QByteArray theBody = *chunkedData;
    //qDebug()<<"_lastChunkSizeByteArray:"<<_lastChunkSizeByteArray;

    int chunkEndNewLineSize = 2; // if using \r\n
    int beginOfLength = 0;
    int endOfLength;

    if(theBody.startsWith("\r\n")){
        beginOfLength = 2;
    }else if(theBody.startsWith("\n")){
        beginOfLength = 1;
    }

    if(!_lastChunkSizeByteArray.isEmpty()){
        if(beginOfLength==0){
            //the cach chunksize bytes was whole bytes
            // do nothing here
        }else{
            endOfLength  = theBody.indexOf("\r\n");
            if(endOfLength==-1){
                // not ending with \r\n
                // try \n
                endOfLength = theBody.indexOf("\n");
                chunkEndNewLineSize = 1; // using \n
            }
            if(endOfLength == -1){
                // newLine not found
                //  append bytes to _lastChunkSizeByteArray
                //  and return -1 to identify not found
                /*
                qDebug()<<"A invalid getChunkSize\n"
                        <<chunkedData->size()
                       <<"\n<===>\n"
                       <<theBody.replace("\r","\\r").replace("\n","\\n")
                       <<"\n<===>\n"
                        <<beginOfLength;
                qDebug()<<"\n_lastChunkSizeByteArray="<<_lastChunkSizeByteArray;
                */
                /*
                   meet this condition:
                       the data doesn't contains whole chunksize bytes
                */
                _lastChunkSizeByteArray.append(theBody);
                (*chunkedData).clear();
                return -1;
            }else{
                // found new line
                //  update _lastChunkSizeByteArray and calculate and clear it
                _lastChunkSizeByteArray.append(theBody.left(endOfLength));
            }
        }
    }else{
        // has no chunksizeByte cache
        //theBody.mid(beginOfLength,endOfLength-beginOfLength).trimmed()
        endOfLength  = theBody.indexOf("\r\n",beginOfLength);
        if(endOfLength==-1){
            endOfLength = theBody.indexOf("\n",beginOfLength);
            chunkEndNewLineSize = 1; // using \n
        }
        if(endOfLength == -1){
            // newLine not found
            //  cache bytes to _lastChunkSizeByteArray
            //  and return -1 to identify not found
            _lastChunkSizeByteArray.append(chunkedData->mid(beginOfLength));
            /*
            qDebug()<<"B invalid getChunkSize\n"
                    <<chunkedData->size()
                    <<"\n<===>\n"
                    <<theBody.replace("\r","\\r").replace("\n","\\n")
                    <<"\n<===>\n"
                    <<beginOfLength;
            qDebug()<<"\n_lastChunkSizeByteArray="<<_lastChunkSizeByteArray;
            */
            (*chunkedData).clear();
            return -1;
        }else{
            _lastChunkSizeByteArray = (*chunkedData).mid(beginOfLength,endOfLength-beginOfLength).trimmed();
        }
    }
    bool isChunkValid;
    //qDebug()<<"chunkSize ba="<<theBody.mid(beginOfLength,endOfLength-beginOfLength).trimmed();

    _lastChunkSizeByteArray = _lastChunkSizeByteArray.trimmed();
    int chunkSize = _lastChunkSizeByteArray.toULongLong(&isChunkValid,16);
    _lastChunkSizeByteArray.clear();

    if(!isChunkValid){
        qDebug()<<"CHUNKED PARSE ERROR:invalid getChunkSize\n"
                <<theBody.replace("\r","\\r").replace("\n","\\n")
                <<"\n"<<_lastChunkSizeByteArray;
        return -1;
    }

    (*chunkedData).remove(0,endOfLength+chunkEndNewLineSize);

    return chunkSize;
}

bool RyPipeData::appendResponseBody(QByteArray* newData){
    //qDebug()<<"appendResponseBody _isResponseChunked:"<<_isResponseChunked<<newData;
    if(_isResponseChunked){
        _responseBody.append(*newData);
        _responseBodyRemain = 1;
        int newDataSize = newData->size();
        if(_lastChunkSize == -1){
            _lastChunkSize = getChunkSize(newData);
        }
        while(!newData->isEmpty()){
            if(_lastChunkSize == 0){
                newData->remove(0,newDataSize-newData->size());
                if(newData->startsWith("\r\n\r\n")){
                    newData->remove(0,2);
                }else if(newData->startsWith("\n\n")){
                    newData->remove(0,1);
                }
                //qDebug()<<"unchunked data=\n"<<responseBodyRawDataUnChunked();
                //qDebug()<<"after chunked success:"<<*newData;
                return true;
            }
            //qDebug()<<"before read sourc-size:"<<newData->size()<<" chunk-to-read:"<<_lastChunkSize;
            int read = readChunk(_lastChunkSize,newData,&_responseUnChunked);
            if(read < _lastChunkSize){
                _lastChunkSize -= read;
                newData->clear();
                //qDebug()<<"chunked not finished"<<_lastChunkSize;
                return false;
            }
            _lastChunkSize = getChunkSize(newData);
            //qDebug()<<"before read sourc-size:"<<newData->size()<<" chunk-to-read:"<<_lastChunkSize;

            if(_lastChunkSize ==-1){
                if(newData->size() != 0){
                    qDebug()<<"=== invalid chunk data"<<*newData
                            <<"\n responseHeader:"<<_responseHeaderRawData
                            <<"\n requestHeader:"<<_requestHeaderRawData
                            <<"\n responseBody:"<<_responseBody.size()
                            <<"\n responseBodyUnChunked"<<_responseUnChunked;
                    //qDebug()<<"chunked not finished"<<_lastChunkSize;
                }
                return false;
            }
        }
        return false;
    }else{
        if(_isContentLenthUnLimit){
            //has no Content-Length
            // just read all buffer
            // and consider it's done
            _responseBody.append(*newData);
            newData->clear();
            _responseBodyRemain = 0;
        }else if(_responseBodyRemain>=newData->length()){
            _responseBodyRemain -= newData->length();
            _responseBody.append(*newData);
            newData->clear();
        }else{
            _responseBody.append(newData->left(_responseBodyRemain));
            newData->remove(0,_responseBodyRemain);
            _responseBodyRemain = 0;
        }
        return _responseBodyRemain<=0;
    }
}
/*
//does not need this setters&getters just use public variable
QString& RyPipeData::host()const{
    QString str;
    return str;
}
quint16 RyPipeData::port()const{
    return 80;
}

QString& RyPipeData::method()const{
    return _method;
}
QString& RyPipeData::path()const{
    return _path;
}
QString& RyPipeData::httpVersion()const{
    return _httpVersion;
}
*/

QString RyPipeData::getRequestHeader(const QString &name) const{
    return _requestHeaders.value(name,"undefined");
}
QString RyPipeData::getResponseHeader(const QString &name) const{
    return _responseHeaders.value(name,"");
}

// private functions
void RyPipeData::parseHeaders(const QList<QByteArray>& header,QMap<QString,QString>* toStore){
    for(int i=0,l=header.size();i<l;++i ){
        QByteArray line = header.at(i).simplified();
        //qDebug()<<line;
        int seperatorIndex = line.indexOf(':');
        if(seperatorIndex==-1){
            continue;
        }
        QString name = line.left(seperatorIndex).trimmed();
        QString value = line.mid(seperatorIndex+1).trimmed();
        if(name.isEmpty() || value.isEmpty()){
            qDebug()<<"some empty header?"<<line;
            continue;
        }

        //format name to make sure do the right thing..
        //todo move this to utils
        //qDebug()<<name;
        QStringList nameParts = name.split("-");
        //qDebug()<<"name to change:"<<name;
        for(int j=0,sl=nameParts.length();j<sl;++j){
            QString part = nameParts.at(j).trimmed();
            //qDebug()<<part;
            if(part.length()==1){
                nameParts[j] = part.at(0).toUpper();
            }else if(part.length()>1){
                nameParts[j] = part.at(0).toUpper() + part.mid(1).toLower();
            }else{
                qDebug()<<"------wrong header---"<<name;
            }
        }
        name = nameParts.join("-");
        (*toStore)[name] = value;
    }
}
