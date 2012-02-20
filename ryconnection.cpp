#include "ryconnection.h"
#include "ryproxyserver.h"

#include <QApplication>
#ifdef WIN32
#include "rywinhttp.h"
#endif

#include "ryrulemanager.h"

RyConnection::RyConnection(int socketHandle,quint64 connectionId,QObject* parent)
    :QObject(parent),
      _handle(socketHandle),
      _connectionId(connectionId),
      _requestSocket(NULL){

    closed = false;
    _pipeTotal = 0;
    _isConnectTunnel = false;
    _requestState = _responseState = ConnectionStateInit;
    //qDebug()<<"RyConnection"<<socketHandle;
    //setHandle(socketHandle);

    _receivingPerformance.clientConnected = QDateTime::currentMSecsSinceEpoch();
}

RyConnection::~RyConnection(){
    //qDebug()<<"~RyConnection pipeTotal:"<<_pipeTotal<<"_connection id = "<<_connectionId;
    if(_requestSocket){
        _requestSocket->disconnect(this);
        _requestSocket->blockSignals(true);
        _requestSocket->abort();
        delete _requestSocket;
        _requestSocket = NULL;
    }
    //qDebug()<<"~RyConnection end";
    //qDebug()<<"~RyConnection in main"
    //        << (QApplication::instance()->thread() == QThread::currentThread());
    //QThread::currentThread()->quit();
}
int RyConnection::handle()const{
    return _handle;
}

void RyConnection::setHandle(int theHandle){
    //if(_handle == theHandle){
    //    qDebug()<<"same handle"<<theHandle;
    //    return;
    //}
    //if(_requestSocket->isOpen()){
    //    return;
    //}
    _handle = theHandle;
    run();
}

// public slots
void RyConnection::deleteLater(){
    QObject::deleteLater();
    // do some clearup work?
}
void RyConnection::run(){
    if(!_requestSocket){
        _requestSocket = new QTcpSocket();
        //_requestSocket->abort();
    }else if(_requestSocket->isOpen()){
        qDebug()<<"socket listening.. do nothing..";
        return;
    }
    //qDebug()<<"run.."<<_handle;
    //qDebug()<<(QThread::currentThread() == QApplication::instance()->thread());

    if(_responseState!=ConnectionStatePackageFound &&
            _responseState!=ConnectionStateInit){
        qDebug()<<"response stile receving...";
        if(!_sendingPipeData.isNull()){
            qDebug()<<_sendingPipeData->fullUrl;
        }
    }
    _responseBuffer.clear();
    _requestBuffer.clear();

    _requestState = _responseState = ConnectionStateInit;
    _sendingPipeData.clear();

    _responseSocket = NULL;
    connect(_requestSocket,SIGNAL(readyRead()),SLOT(onRequestReadyRead()));
    connect(_requestSocket,SIGNAL(aboutToClose()),SLOT(onRequestClose()));
    connect(_requestSocket,SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(onRequestError(QAbstractSocket::SocketError)));
    if(!_requestSocket->isOpen()){
        _requestSocket->setSocketDescriptor(_handle);
    }

}

//private slots
void RyConnection::onRequestReadyRead(){
    if(_receivingPerformance.requestBegin ==-1){
        _receivingPerformance.requestBegin = QDateTime::currentMSecsSinceEpoch();
    }
    QByteArray newContent = _requestSocket->readAll();
    //qDebug()<<"requestReadyRead"<<newContent;

    _requestBuffer.append(newContent);
    parseRequest();
}
void RyConnection::onRequestClose(){
    if(_responseSocket){
        _responseSocket->blockSignals(true);
        _responseSocket->disconnect(this);
        RyProxyServer::instance()->cacheSocket(
                    _connectingHost,
                    _connectingPort,
                    _responseSocket);
    }
    _responseSocket = NULL;

    qDebug()<<"request close";
    _responseState = ConnectionStateInit;
    emit connectionClose();
    closed = true;
}
void RyConnection::onRequestError(QAbstractSocket::SocketError err){
    //qDebug()<<"request error";
    if(_responseSocket){
        _responseSocket->blockSignals(true);
        _responseSocket->disconnect(this);
        RyProxyServer::instance()->cacheSocket(
                    _connectingHost,
                    _connectingPort,
                    _responseSocket);
    }
    _responseSocket = NULL;

    if(_sendingPipeData){
        if(_sendingPipeData->isContentLenthUnLimit()){
            //_responseState = ConnectionStatePackageFound;
            //qDebug()<<"respones close when content-length not found";
            //qDebug()<<_sendingPipeData->responseBodyRawData();
            //emit pipeComplete(_sendingPipeData);
            onResponsePackageFound();
        }else{//maybe remote connection disconnect with error
            //if(!_sendingPipeData->isPackageFound()){
            //   _sendingPipeData->markAsError();
                if(_sendingPipeData->responseHeaderRawData().isEmpty()){
                    QByteArray ba;
                    QByteArray bc("request error:(browser disconnect)");
                    int count = bc.size();
                    ba.append(QString("HTTP/1.0 %1 \r\n"
                                                           "Server: Rythem \r\n"
                                                           "Content-Type: %2 \r\n"
                                                           "Content-Length: %3 \r\n\r\n"
                                                           ).arg("200")
                                                            .arg("text")
                                                            .arg(count));
                    ba.append(bc);

                    _sendingPipeData->parseResponse(&ba);
                }
                //emit pipeError(_sendingPipeData);
                //TODO
                onResponsePackageFound();
            //}

        }
        _sendingPipeData.clear();
    }
    _responseState = ConnectionStateInit;

    //TODO
    emit connectionClose();
    closed = true;
}

void RyConnection::onResponseConnected(){
    if(_sendingPerformance.responseConnected==-1){
        _sendingPerformance.responseConnected = QDateTime::currentMSecsSinceEpoch();
    }


    //qDebug()<<_responseSocket->state() ;
    _connectionNotOkTime = 0;
    //qDebug()<<"onResponseConnected begin";
    //because the socket will be reuse. so remove unnecessary signal/slot connections
    disconnect(_responseSocket,SIGNAL(connected()),this,SLOT(onResponseConnected()));

    _sendingPipeData->serverIp = _responseSocket->peerAddress().toString();
    _responseState = ConnectionStateConnected;
    if(_sendingPipeData->method=="CONNECT"){
        qDebug()<<"is establishing connect tunnel";
        qDebug()<<"CONNECT method: connect success";
        QByteArray ba("HTTP/1.1 200 Connection established"
                       "\r\n"
                       "Connection: keep-alive\r\n\r\n");
        _requestSocket->write(ba);
        _responseBuffer.append(ba);
        parseResponse();
        return;
    }else{
        //qDebug()<<"to write:\n"<<_sendingPipeData->dataToSend();
        _responseSocket->write(_sendingPipeData->dataToSend());
        _responseSocket->flush();
    }
}
void RyConnection::onResponseReadyRead(){
    //qDebug()<<"onResponsReadyRead"<<handle();
    QByteArray newContent = _responseSocket->readAll();

    //qDebug()<<"response come readAll:"
    //        <<newContent.size()
    //        <<"bytesAvailable:"
    //        <<length;

    if(newContent.isEmpty()){
        qDebug()<<"empty response";
        return;
    }
    //qDebug()<<"response ready read\n"
    //        <<newContent;
    _requestSocket->write(newContent);
    _requestSocket->flush();
    _responseBuffer.append(newContent);
    //qDebug()<<_responseBuffer;
    parseResponse();
    //qDebug()<<"onResponsReadyRead end"<<handle();

}
void RyConnection::onResponseClose(){
    qDebug()<<"response close";
    closed = true;
}
void RyConnection::onResponseError(QAbstractSocket::SocketError){
    //qDebug()<<"onResponseError"<<handle();
    if(_sendingPipeData){
        if(_sendingPipeData->responseHeaderRawData().isEmpty()){

            QByteArray s = "[Rythem:remote server unreachable]";
            int count = s.length();
            QByteArray byteToWrite;
            byteToWrite.append(QString("HTTP/1.0 %1 \r\n"
                                       "Server: Rythem \r\n"
                                       "Content-Type: %2 \r\n"
                                       "Content-Length: %3 \r\n\r\n"
                                       ).arg("502")
                                        .arg("text")
                                        .arg(count)
                               );
            byteToWrite.append(s);
            QByteArray ba = QByteArray("nRythem:remote server close");
            _sendingPipeData->parseResponse(&byteToWrite);
            onResponsePackageFound();
            //emit pipeError(_sendingPipeData);
        }else if(_sendingPipeData->isContentLenthUnLimit()){
            qDebug()<<"respones close when content-length not found";
            onResponsePackageFound();
            //emit pipeComplete(_sendingPipeData);
        }else{//maybe remote connection disconnect with error
            //   when the response doesn't complete.
            //if(!_sendingPipeData->isPackageFound()){
            //   _sendingPipeData->markAsError();

            //}
            onResponsePackageFound();
            //emit pipeComplete(_sendingPipeData);

        }
        _sendingPipeData.clear();
    }
    _responseState = ConnectionStateInit;
    if(_requestSocket){
        _requestSocket->blockSignals(true);
        _requestSocket->disconnect(this);
        _requestSocket->abort();
    }
    emit connectionClose();
    closed = true;
    //qDebug()<<"onResponseError end"<<handle();
}

void RyConnection::onRequestHeaderFound(){
    /*
    if(_isConnectTunnel){
        _receivingPipeData->path = _receivingPipeData->fullUrl;
        _receivingPipeData->fullUrl
                .prepend(
                    QString("http://")
                    .append(_connectingHost)
                    .append(":")
                    .append(QString("%1").arg(_connectingPort))
                 );
        _receivingPipeData->host = _connectingHost;
        _receivingPipeData->port = _connectingPort;
    }
    */
    _requestState = ConnectionStateHeadFound;
}

void RyConnection::onRequestPackageFound(){
    _receivingPerformance.requestDone = QDateTime::currentMSecsSinceEpoch();
    if(_receivingPipeData->method == "CONNECT"){
        _isConnectTunnel = true;
        qDebug()<<"requestPackageFound"
                <<_receivingPipeData->requestHeaderRawData()
                <<_receivingPipeData->requestBodyRawData().size()
                <<_receivingPipeData->dataToSend();
    }
    _requestState = ConnectionStatePackageFound;
    _receivingPipeData->performances = _receivingPerformance;
    emit pipeBegin(_receivingPipeData);
    appendPipe(_receivingPipeData);
    _receivingPipeData.clear();

    _lastSocketConnectedDateTime = _receivingPerformance.clientConnected;
    _receivingPerformance.reset();
    doRequestToNetwork();
}

void RyConnection::onResponseHeaderFound(){
    _responseState = ConnectionStateHeadFound;
}

void RyConnection::onResponsePackageFound(){
    _sendingPerformance.responseDone = QDateTime::currentMSecsSinceEpoch();
    _sendingPipeData->performances = _sendingPerformance;
    _responseState = ConnectionStatePackageFound;
    emit pipeComplete(_sendingPipeData);
    /*
    qDebug()<<"PackageFound:\n"
            <<_sendingPipeData->fullUrl
            <<_sendingPipeData->responseHeaderRawData()
            <<_sendingPipeData->responseBodyRawData().size();
    */
    _sendingPipeData.clear();
    _sendingPerformance.reset();
    doRequestToNetwork();


}

// ============ end of private slots

//private functions
void RyConnection::parseRequest(){
    if(_requestBuffer.isEmpty()){
        //qDebug()<<"empty request";
        return;
    }
    if(_receivingPipeData.isNull()){
        _receivingPipeData = RyPipeData_ptr(new RyPipeData(_handle,_connectionId));
        _pipeTotal++;
        _receivingPipeData->id = RyProxyServer::instance()->nextPipeId();
    }
    //qDebug()<<"parsing request"<<newContent;
    if(_requestState == ConnectionStateInit ||
            _requestState == ConnectionStatePackageFound ){
        bool isRequestOk=false;
        int byteRemain = _receivingPipeData->parseRequest(&_requestBuffer,&isRequestOk);
        if(isRequestOk){
            onRequestHeaderFound();
            if(byteRemain == 0){//package found
                onRequestPackageFound();
            }
        }else{
            qDebug()<<"wrong request"<<_requestBuffer;
            return;
        }

    }else if(_requestState == ConnectionStateHeadFound){
        //qDebug()<<"request HeaderFound:"<<newContent;
        bool isConnectionComplete = _receivingPipeData->appendRequestBody(&_requestBuffer);
        if(isConnectionComplete){
            onRequestPackageFound();
        }

    }
    if(!_requestBuffer.isEmpty()){
        qDebug()<<"after parsing  request buffer not empty!!!";
        parseRequest();
    }
}
void RyConnection::parseResponse(){
    if(_sendingPipeData.isNull()){// should not enter here.

        qDebug()<<"ERROR:sendingPipeData iSNULL!!!!\nis closed:"
                << closed
                <<"\nsize="<<_responseBuffer.size()
                <<"\npre_host="<<_connectingHost
                <<"\npre_port="<<_connectingPort
                <<"\nfullUrl="<<_fullUrl
                <<"\nnew_content="<<_responseBuffer;
        return;
    }
    if(_responseState == ConnectionStateConnected ||
            _responseState == ConnectionStatePackageFound){
        bool isResponseOk;
        //qDebug()<<newContent;
        int byteRemain = _sendingPipeData->parseResponse(&_responseBuffer,&isResponseOk);
        if(isResponseOk){
            onResponseHeaderFound();
            if(byteRemain == 0){
                if(_sendingPipeData->method!="CONNECT"
                        && _sendingPipeData->isContentLenthUnLimit()
                        && _sendingPipeData->responseStatus != "304"
                        && _sendingPipeData->responseStatus != "302"
                        && _sendingPipeData->responseStatus != "301"){
                    //qDebug()<<"no content-length wait for closed..."
                    //        <<_sendingPipeData->dataToSend();
                    return;
                }

                onResponsePackageFound();
                if(!_responseBuffer.isEmpty()){
                    //qDebug()<<"after response package found:\n"
                    //        <<_responseBuffer.size();
                    parseResponse();
                }
            }
        }else{
            _connectionNotOkTime++;
            /*
            printStates();
            qDebug()<<"response not ok"
                    <<_sendingPipeData->responseBodyRawData()
                    <<_sendingPipeData->responseHeaderRawData()
                    <<_sendingPipeData->host
                    <<"response not ok is socket from cache:"
                    <<_isSocketFromCache
                    <<_connectionNotOkTime
                    <<_responseBuffer;
            */
            return;
        }
    }else if(_responseState == ConnectionStateHeadFound){
        bool isResponseComplete = _sendingPipeData->appendResponseBody(&_responseBuffer);
        if(isResponseComplete){

            if(_sendingPipeData->isContentLenthUnLimit()
                    && _sendingPipeData->responseStatus != "304"
                    && _sendingPipeData->responseStatus != "302"
                    && _sendingPipeData->responseStatus != "301"){
                //qDebug()<<"no content-length wait for closed...";
                return;
            }

            onResponsePackageFound();
            if(!_responseBuffer.isEmpty()){
                qDebug()<<"after response package found:\n"
                        <<_responseBuffer.size();
                parseResponse();
            }
        }
    }
}

void RyConnection::doRequestToNetwork(){
    if(_sendingPipeData.isNull()){
        _sendingPipeData = nextPipe();
    }else{
        qDebug()<<"wait for last pipe finish"
                <<_sendingPipeData->fullUrl
                <<_sendingPipeData->responseHeaderRawData()
                <<_sendingPipeData->responseBodyRawData();
        return;
    }
    if(_sendingPipeData.isNull()){
        //qDebug()<<"doRequestToNetwork : no any more pipe pending";
        return;
    }
    QString host = _sendingPipeData->host;
    quint16 port = _sendingPipeData->port;
    //qDebug()<<"doRequestToNetwork"<<_sendingPipeData->fullUrl<<host<<port;
    //qDebug()<<host<<port;

    // check if is request to self

    if(host == RyProxyServer::instance()->serverAddress().toString()
            && port == RyProxyServer::instance()->serverPort()){
        //TODO
        //qDebug()<<"simple http"<<_sendingPipeData->path;
        QMap<QString,QString> contentTypeMapping;
        contentTypeMapping["jpg"] = "image/jpeg";
        contentTypeMapping["js"] = "application/javascript";
        contentTypeMapping["png"] = "image/png";
        contentTypeMapping["gif"] = "image/gif";
        contentTypeMapping["css"] = "text/css";
        contentTypeMapping["html"] = "text/html";
        contentTypeMapping["htm"] = "text/html";
        contentTypeMapping["txt"] = "text/plain";
        contentTypeMapping["jpeg"] = "image/jpeg";
        contentTypeMapping["manifest"] = "text/cache-manifest";

        // output content
        QByteArray s;
        QString returnStatus = "200 OK";
        QString contentType = "text/html";

        QByteArray byteToWrite;
        QString filePath = _sendingPipeData->path;
        //remove ?xxx
        int queryIndex = filePath.indexOf('?');
        if(queryIndex != -1) filePath = filePath.left(queryIndex);

        //remote #xxx
        int hashIndex = filePath.indexOf('#');
        if(hashIndex != -1) filePath = filePath.left(hashIndex);
        if(filePath.endsWith("/")){
            filePath.append("index.html");
        }
        filePath.prepend(":/web");
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly )){
            returnStatus = "404 NOT FOUND";
            s.append(filePath + " Not Found");
        }else{
            s.clear();
            s = file.readAll();
        }
        file.close();
        int postFixIndex = filePath.lastIndexOf(".");
        if(postFixIndex!=-1){
            QString postFix = filePath.mid(postFixIndex+1);
            contentType = contentTypeMapping.value(postFix,"text/plain");
        }


        int count = s.length();

        byteToWrite.append(QString("HTTP/1.0 %1 \r\n"
                                   "Server: Rythem \r\n"
                                   "Content-Type: %2 \r\n"
                                   "Content-Length: %3 \r\n\r\n"
                                   ).arg(returnStatus)
                                    .arg(contentType)
                                    .arg(count)
                           );
        byteToWrite.append(s);
        //qDebug()<<"byteToWrite"<<byteToWrite;
        _requestSocket->write(byteToWrite);
        _requestSocket->flush();
        _sendingPipeData->parseResponse(&byteToWrite);
        onResponseHeaderFound();
        onResponsePackageFound();
        return;
    }

    RyRuleManager *manager = RyRuleManager::instance();//qApp->applicationDirPath()+"/config.txt");
    QList<RyRule *> matchResult;
    manager->getMatchRules(&matchResult, _sendingPipeData->fullUrl); //2ms
    for(int i=0,l=matchResult.size();i<l;++i){
        RyRule *rule = matchResult.at(i);
        qDebug()<<"rule found"<<rule->toJSON();
        if(rule->type() == COMPLEX_ADDRESS_REPLACE ||
                rule->type() ==SIMPLE_ADDRESS_REPLACE){
            host = _sendingPipeData->replacedHost;
        }else{
            QPair<QByteArray,QByteArray> headerAndBody = rule->replace(_sendingPipeData);
            bool isOk;
            QByteArray ba = headerAndBody.first;
            _sendingPipeData->parseResponse(&ba,&isOk);
            if(isOk){
                _requestSocket->write(headerAndBody.first);
                _requestSocket->flush();
                onResponseHeaderFound();
                qDebug()<<headerAndBody.second;
                ba = headerAndBody.second;
                _sendingPipeData->appendResponseBody(&ba);
                _requestSocket->write(headerAndBody.second);
                _requestSocket->flush();
                onResponsePackageFound();
                return;
            }else{
                break;
            }
        }
    }

    //qDebug()<<"connecting:"<<_connectingHost<<_connectingPort;
    //qDebug()<<"to connect:"<<host<<port;
    _fullUrl = _sendingPipeData->fullUrl;
    bool isGettingSocketFromServer = true;
    if(_responseSocket){
        if(_connectingHost.toLower() == host.toLower() &&
                _connectingPort == port){
            //qDebug()<<"old responeSocket";
            // do nothing
            // just reuse previous response socket
            isGettingSocketFromServer = false;
        }else{
            /*
            qDebug()<<"dif host: to connect:"
                    << host
                    << " socket connecting:"
                    << _responseSocket->peerName()
                    << _responseState;
            */
            // cache previous response socket and disconnect all signals/slots
            _responseSocket->disconnect(this);
            RyProxyServer::instance()->cacheSocket(
                        _connectingHost,
                        _connectingPort,
                        _responseSocket);

            //   get new socket
            _connectingHost = host;
            _connectingPort = port;
            QMetaObject::invokeMethod(
                                  RyProxyServer::instance(),
                                  "getSocket",
                                  Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(QTcpSocket *,_responseSocket),
                                  Q_ARG(QString ,host),
                                  Q_ARG(quint16,port),
                                  Q_ARG(bool *,&_isSocketFromCache),
                                  Q_ARG(QThread*,QThread::currentThread()));
            /*
            _responseSocket->abort();
            _responseSocket->blockSignals(true);
            delete _responseSocket;
            _responseSocket = NULL;
            _responseSocket = new QTcpSocket(this);
            */
            connect(_responseSocket,SIGNAL(readyRead()),SLOT(onResponseReadyRead()));
            connect(_responseSocket,SIGNAL(aboutToClose()),SLOT(onResponseClose()));
            connect(_responseSocket,SIGNAL(error(QAbstractSocket::SocketError)),
                    SLOT(onResponseError(QAbstractSocket::SocketError)));
            //qDebug()<<"move to thread..";
            _responseSocket->moveToThread(thread());
            //qDebug()<<"after move to thread..";
        }
    }else{
        //qDebug()<<"new connection"<<host<<port<<" handle"<<handle();
        // get new socket

        _connectingHost = host;
        _connectingPort = port;
        QMetaObject::invokeMethod(
                              RyProxyServer::instance(),
                              "getSocket",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QTcpSocket *,_responseSocket),
                              Q_ARG(QString ,host),
                              Q_ARG(quint16,port),
                              Q_ARG(bool *,&_isSocketFromCache),
                              Q_ARG(QThread*,QThread::currentThread()));

        //_responseSocket = new QTcpSocket(this);
        connect(_responseSocket,SIGNAL(readyRead()),SLOT(onResponseReadyRead()));
        connect(_responseSocket,SIGNAL(aboutToClose()),SLOT(onResponseClose()));
        connect(_responseSocket,SIGNAL(error(QAbstractSocket::SocketError)),
                SLOT(onResponseError(QAbstractSocket::SocketError)));
        //qDebug()<<"move to thread..";
        //_responseSocket->moveToThread(thread());
        //qDebug()<<"after move to thread..";
    }

    if(isGettingSocketFromServer){
        //qDebug()<<"responseSocket from proxyserver";
        _responseSocket->blockSignals(true);
        _responseSocket->disconnectFromHost();
        _responseSocket->abort();
        _responseSocket->blockSignals(false);
        connect(_responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
    }

    QAbstractSocket::SocketState responseState = _responseSocket->state();
    if(responseState == QAbstractSocket::UnconnectedState
            || responseState == QAbstractSocket::ClosingState){
        //qDebug()<<"responseSocket is not open"
        //        <<"connect to "<<host<<port
        //        <<responseState
        //        <<_responseSocket->readAll();
        _responseState = ConnectionStateConnecting;
#ifdef Q_OS_WIN
        // TODO add mac pac support
        QList<QNetworkProxy> proxylist = RyWinHttp::queryProxy(QNetworkProxyQuery(QUrl(_sendingPipeData->fullUrl)));
        for(int i=0,l=proxylist.length();i<l;++i){
            QNetworkProxy p = proxylist.at(i);
            if(p.hostName() == RyProxyServer::instance()->serverAddress().toString()
                    && p.port() == RyProxyServer::instance()->serverPort()){
                qWarning()<<"warining: proxy is your self!";
                continue;
            }
            _responseSocket->setProxy(p);
            //qDebug()<<"proxy="<<p.hostName()<<p.port();
        }
 #endif
        _responseSocket->connectToHost(host,port);
    }else{
        qDebug()<<"responseSocket opened"
                <<responseState;
        if(responseState == QAbstractSocket::ConnectedState){
            onResponseConnected();
        }else{
            qDebug()<<"responseSocket not open yet"<<responseState;
            if(isGettingSocketFromServer){
                connect(_responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
            }
        }
    }
}
RyPipeData_ptr RyConnection::nextPipe(){
    QMutexLocker locker(&pipeDataListMutex);
    Q_UNUSED(locker)
    if(_pipeList.length()>0){
        return _pipeList.takeAt(0);
    }
    return RyPipeData_ptr();
}
void RyConnection::appendPipe(RyPipeData_ptr thePipeData){
    _pipeList.append(thePipeData);

}
