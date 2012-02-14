#include "qiconnectiondata.h"
#include <QDebug>
#include <QStringList>

QiConnectoionData::QiConnectoionData(int socketDescriptor){
    //qDebug()<<"connectionData contructed:";
    id=-1;
    returnCode = -1;
}


void QiConnectoionData::setRequestHeader(QByteArray header){
    requestHeaderRawData = header;
    header.replace("\r\n","\n");
    int i=0,l=header.length();
    //firstline
    i=header.indexOf('\n');
    if(i==-1){
        qDebug()<<"-------- invalid response header  ------"<<header;
        return;
    }
    QByteArray firstLine = header.left(i).simplified();
    QList<QByteArray> sigs = firstLine.split(' ');
    if(sigs.length() != 3){
        qDebug()<<"-------- invalid response header  ------"<<header;
        return;
    }
    requestMethod = sigs.at(0);
    fullUrl = sigs.at(1);
    protocol = sigs.at(2);

    // change http://aaa.com/a/b/c?d to /a/b/c?d
    path = "/";
    int n;
	//qDebug()<<"fullUrl="<<fullUrl;
    if(fullUrl.indexOf("://")!=-1){
        int tmp = fullUrl.indexOf("://")+3;
        n = fullUrl.mid(tmp).indexOf("/");
        if(n!=-1 && n<fullUrl.length()-1){
            path = fullUrl.mid(tmp).mid(n);
        }
    }else{
        n = fullUrl.indexOf("/");
        if(n!=-1 && n<fullUrl.length()-1){
            path = fullUrl.mid(n);
        }
    }
    //TODO..

    requestRawDataToSend = QByteArray().append(requestMethod)
            .append(' ')
            .append(path)
            .append(' ')
            .append(protocol);
    requestRawDataToSend.append(header.mid(i));

    requestRawDataToSend.replace(QString("Proxy-Connection: keep-alive\n"),QByteArray(""));
    requestRawDataToSend.replace(QString("Proxy-Connection: keep-alive\r\n"),QByteArray(""));

    requestRawDataToSend.replace(QString("Proxy-Connection: Keep-Alive\n"),QByteArray(""));
    requestRawDataToSend.replace(QString("Proxy-Connection: Keep-Alive\r\n"),QByteArray(""));
    requestRawDataToSend.replace(QString("\n"),QByteArray("\r\n"));
    requestRawDataToSend.append(QByteArray("\r\n\r\n"));
    /*
    QByteArray test = requestRawDataToSend;
    test.replace("\r","\\r\r");
    test.replace("\n","\\n\n");
    qDebug()<<"test=\n"<<test;
    */
    //the rest..
    while(i<l){
        int j=header.indexOf('\n',i);
        if(j==-1){// last line
            j=l;
        }
        QByteArray line = header.mid(i,j-i);
        //qDebug()<<line;

        int splitIndex = line.indexOf(':');
        QByteArray name = QByteArray(line.left(splitIndex));
        QByteArray value = QByteArray(line.mid(splitIndex+1).trimmed());
        //setRequestHeader(name,value);
        if(name == QString("Host")){
            int d = value.indexOf(":");
            if(d!=-1){
                allRequestHeaders["Host"] = value.left(d);
                allRequestHeaders["Port"] = value.mid(d+1);
            }else{
                allRequestHeaders["Host"] = value;
                allRequestHeaders["Port"] = "80";
            }

            host = allRequestHeaders["Host"];
            port = allRequestHeaders["Port"].toInt();
        }else{
            allRequestHeaders[name] = value;
        }
        i=j+1;
    }
}
void QiConnectoionData::setResponseHeader(QByteArray header){
    responseHeaderRawData = header;
    //TODO.. Ctrl+c & Ctrl+v from setRequestHeader
    header.replace("\r\n","\n");
    int i=0,l=header.length();

    //firstline
    //HTTP/1.1 302 Found
    i=header.indexOf('\n');
    if(i==-1){
        qDebug()<<"error header has noly one line:"<<header;
        return;
    }
    QByteArray firstLine = header.left(i).simplified();
    QList<QByteArray> sigs = firstLine.split(' ');
    if(sigs.length() < 3){
        qDebug()<<"error..."<<firstLine;
        qDebug()<<header;
        return;
    }
    returnCode = sigs.at(1).simplified().toInt();

    while(i<l){
        int j=header.indexOf('\n',i);
        if(j==-1){// last line
            j=l;
        }
        QByteArray line = header.mid(i,j-i);
        //qDebug()<<line;

        int splitIndex = line.indexOf(':');
        QByteArray name = QByteArray(line.left(splitIndex));
        QByteArray value = QByteArray(line.mid(splitIndex+1).trimmed());
        allResponseHeaders[name]=value;
        //setRequestHeader(name,value);
        i=j+1;
    }
}


QByteArray QiConnectoionData::getResponseHeader(QByteArray name)const{
    return allResponseHeaders[name];
}
QByteArray QiConnectoionData::getResponseHeader()const{

}
QByteArray QiConnectoionData::getResponseBody()const{

}

QByteArray QiConnectoionData::getRequestHeader() const{
    if(requestRawData.isEmpty()){
        return requestRawData;
    }
    int i = requestRawData.indexOf("\r\n\r\n");
    if(i==-1){
        i = requestRawData.indexOf("\r\n");
    }
    if(i==-1){
        return requestRawData;
    }else{
        return requestRawData.left(i);
    }
}
QByteArray QiConnectoionData::getRequestHeader(QByteArray name) const{
    return allRequestHeaders.value(name,QByteArray());
}
QByteArray QiConnectoionData::getRequestBody()const{
    if(requestRawData.isEmpty()){
        return QByteArray();
    }
    int i = requestRawData.indexOf("\r\n\r\n");
    if(i!=-1){
        return requestRawData.mid(i+4);
    }else{
        i = requestRawData.indexOf("\r\n");
        if(i!=-1){
            return requestRawData.mid(i+2);
        }
    }
    return QByteArray();

}
bool QiConnectoionData::appendResponseBody(QByteArray newContent){
    responseBody.append(newContent);
    //qDebug()<<"appending response body:"<<responseBody;
    if(this->getResponseHeader("Transfer-Encoding").toLower() == "chunked"){
        unChunkResponse.clear();
        // TODO .. move to single function
        //qDebug()<<"appendResponseBody called:"<<responseBody;
        QByteArray theBody = responseBody;
        int i=0;
        int l=theBody.length();
        int NLSize = 1;
        int beginOfLength = 0;
        int endOfLength  = theBody.indexOf('\n',beginOfLength+1);
        if(endOfLength==-1){
            endOfLength = theBody.indexOf("\r\n",beginOfLength+2);
            NLSize = 2;
            if(endOfLength == -1){
                return false;
            }
        }
        bool isChunkValid;
        int chunkSize = theBody.mid(beginOfLength,endOfLength-beginOfLength).trimmed().toInt(&isChunkValid,16);
        if(!isChunkValid){
            qDebug()<<"invalid chunk data"<<theBody;
            qDebug()<<"res header ="<<responseHeaderRawData;
            qDebug()<<"req header:"<<requestHeaderRawData;
            qDebug()<<getRequestHeader("Content-Length");
            qDebug()<<allRequestHeaders;
            return false;
        }
        if(chunkSize == 0){
            return true;
        }
        i = chunkSize + endOfLength + NLSize;
        if(chunkSize + endOfLength + NLSize > theBody.length()){
            return false;
        }
        unChunkResponse.append(theBody.mid(endOfLength+NLSize,chunkSize));
        do{//need to valid chunk here?
            //qDebug()<<"chunked:"<<i<<" "<<l;
            beginOfLength=theBody.indexOf('\n',i);
            NLSize = 1;
            if(beginOfLength == -1){
                beginOfLength = theBody.indexOf("\r\n",i);
            }
            if(beginOfLength==-1){
                return false;
            }
            endOfLength = theBody.indexOf('\n',beginOfLength+1);
            if(endOfLength==-1){
                endOfLength = theBody.indexOf("\r\n",beginOfLength+2);
                NLSize = 2;
                if(endOfLength == -1){
                    return false;
                }
            }
            QByteArray sizeBA = theBody.mid(beginOfLength,endOfLength-beginOfLength).trimmed();
            chunkSize = sizeBA.toInt(&isChunkValid,16);
            if(!isChunkValid){
                //qDebug()<<"no valid:"<<isChunkValid<<sizeBA;
                return false;
            }

            //qDebug()<<chunkSize;
            unChunkResponse.append(theBody.mid(endOfLength+NLSize,chunkSize));
            if(chunkSize==0){
                //qDebug()<<"got chunked end";
                //qDebug()<<unChunkResponse;
                return true;
            }
            // don't do this until comfirm reponse done
            /*
            if(chunkSize+endOfLength+1<=l){
                connectionData->unChunkResponse.append(theBody.mid(endOfLength+1,chunkSize));
            }
            */
            i = chunkSize+endOfLength+NLSize;
            if(i>l){
                return false;
            }
        }while(i<=l);
    }else{
        bool isCoverSuccess=false;
        int contentlength = this->getResponseHeader("Content-Length").toInt(&isCoverSuccess);
        if(!isCoverSuccess){//无content-length
            if(returnCode == 304
                    || returnCode == 301
                    || returnCode == 302
                    || returnCode == 307
                    || returnCode == 204){//todo
                return true;
            }else{
                if(this->getResponseHeader("Connection")=="close"){
                    //qDebug()<<"connection data : connection close";
                    //qDebug()<<allResponseHeaders;
                    return true;
                }
            }
            return true;
        }
        return contentlength <= responseBody.length();
    }
    return true;
}
bool QiConnectoionData::appendRequestBody(QByteArray newContent){
    requestBody.append(newContent);
    requestRawDataToSend.append(newContent);

    QByteArray contentLenght = getRequestHeader("Content-Length");
    qDebug()<<contentLenght<<requestBody.size();
    int requestContentLength = contentLenght.toInt();
    if(requestContentLength <= requestBody.size()){
        return true;
    }
    return false;
}


//apis for replace rule
void QiConnectoionData::setHost(QString newHost){
    //QString oldHost = host;
    host = newHost;
    allRequestHeaders["Host"] = QByteArray().append(newHost);
}

void QiConnectoionData::setRemoteAddress(QString address,int port){

}

void QiConnectoionData::setPath(QString path){

}

void QiConnectoionData::setRequestRawData(QByteArray request){//only for copy Ctor

}
void QiConnectoionData::setResponseRawData(QByteArray response){//only for copy Ctor

}

