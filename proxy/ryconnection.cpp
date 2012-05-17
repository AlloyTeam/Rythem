#include "ryconnection.h"
#include "ryproxyserver.h"

#include <QApplication>
#ifdef WIN32
#include "rywinhttp.h"
#endif

#include "rule/ryrulemanager.h"
using namespace rule;

RyConnection::RyConnection(int socketHandle,quint64 connectionId,QObject* parent)
    :QObject(parent),
      _handle(socketHandle),
      _connectionId(connectionId),
      _requestSocket(NULL),
      _responseSocket(NULL){

    closed = false;
    _pipeTotal = 0;
    _isConnectTunnel = false;
    _requestState = _responseState = ConnectionStateInit;
    _receivingPerformance.clientConnected = QDateTime::currentMSecsSinceEpoch();
}

RyConnection::~RyConnection(){
    if(_responseState!=ConnectionStatePackageFound &&
            _responseState!=ConnectionStateInit){
        qDebug()<<"response stile receving...";
    }
    //qDebug()<<"~RyConnection pipeTotal:"<<_pipeTotal<<"_connection id = "<<_connectionId;
    if(_requestSocket){
        _requestSocket->disconnect(this);
        _requestSocket->blockSignals(true);
        _requestSocket->abort();
        delete _requestSocket;
        _requestSocket = NULL;
    }
    if(_responseSocket){
        _responseSocket->disconnect(this);
        _responseSocket->blockSignals(true);
        _responseSocket->abort();
        delete _responseSocket;
        _responseSocket = NULL;
    }
}
int RyConnection::handle()const{
    return _handle;
}

void RyConnection::setHandle(int theHandle){
    _handle = theHandle;
    // run方法的出现是QThread使用方法变迁的历史遗留问题
    run();
}

// public slots
void RyConnection::run(){
    //初始化socket

    if(!_requestSocket){
        _requestSocket = new QTcpSocket();
    }else if(_requestSocket->isOpen()){
        qDebug()<<"socket listening.. do nothing..";
        return;
    }
    if(_responseState!=ConnectionStatePackageFound &&
            _responseState!=ConnectionStateInit){
        qDebug()<<"response stile receving...";
        if(!_sendingPipeData.isNull()){
            qDebug()<<_sendingPipeData->fullUrl;
        }
    }
    //初始化状态，缓存数据
    _responseBuffer.clear();
    _requestBuffer.clear();
    _requestState = _responseState = ConnectionStateInit;
    _sendingPipeData.clear();
    _responseSocket = NULL;

    //监听事件
    connect(_requestSocket,SIGNAL(readyRead()),SLOT(onRequestReadyRead()));
    connect(_requestSocket,SIGNAL(aboutToClose()),SLOT(onRequestClose()));
    connect(_requestSocket,SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(onRequestError(QAbstractSocket::SocketError)));
    if(!_requestSocket->isOpen()){
        //打开连接
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
    if(_isConnectTunnel){
        // https隧道,直接透传不解析
        // TODO: decrypt https tunnel
        // TODO: rename _isConnectTunnel
        _responseSocket->write(newContent);
        _responseSocket->flush();
        return;
    }

    // append new content and parse request
    _requestBuffer.append(newContent);
    parseRequest();
}
void RyConnection::onRequestClose(){
    if(_responseSocket){
        _responseSocket->blockSignals(true);
        _responseSocket->disconnect(this);
        _responseSocket->abort();
        delete _responseSocket;
        // 暂时不缓存socket
        /*
        RyProxyServer::instance()->cacheSocket(
                    _connectingHost,
                    _connectingPort,
                    _responseSocket);
         */
    }
    _responseSocket = NULL;

    qDebug()<<"request close";
    _responseState = ConnectionStateInit;
    emit connectionClose();
    closed = true;
}
void RyConnection::onRequestError(QAbstractSocket::SocketError){
    if(_responseSocket){
        _responseSocket->blockSignals(true);
        _responseSocket->disconnect(this);

        _responseSocket->abort();
        delete _responseSocket;
        // 暂时不缓存socket
        /*
        RyProxyServer::instance()->cacheSocket(
                    _connectingHost,
                    _connectingPort,
                    _responseSocket);
        */
    }
    _responseSocket = NULL;

    if(_sendingPipeData){
        if(_sendingPipeData->isContentLenthUnLimit()){
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
                // TODO: emit pipeError(_sendingPipeData);
                onResponsePackageFound();
            //}
        }
    }
    _responseState = ConnectionStateInit;
    emit connectionClose();
    closed = true;
}

void RyConnection::onResponseConnected(){
    if(_sendingPerformance.responseConnected==-1){
        _sendingPerformance.responseConnected = QDateTime::currentMSecsSinceEpoch();
    }


    _connectionNotOkTime = 0;
    //because the socket will be reuse. so remove unnecessary signal/slot connections
    disconnect(_responseSocket,SIGNAL(connected()),this,SLOT(onResponseConnected()));

    _sendingPipeData->serverIp = _responseSocket->peerAddress().toString();
    _responseState = ConnectionStateConnected;
    if(_sendingPipeData->method=="CONNECT"){
        QByteArray ba("HTTP/1.1 200 Connection established\r\n"
                       "Proxy-Server: Rythem\r\n"
                       "Connection: keep-alive\r\n\r\n");
        _requestSocket->write(ba);
        _requestSocket->flush();
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
    if(_sendingPerformance.responseBegin == -1){
        _sendingPerformance.responseBegin = QDateTime::currentMSecsSinceEpoch();
    }
    QByteArray newContent = _responseSocket->readAll();
    if(_isConnectTunnel){
        // HTTPS Tunnel 直接透传
        // TODO: https解析
        _requestSocket->write(newContent);
        _requestSocket->flush();
        return;
    }

    if(newContent.isEmpty()){
        qDebug()<<"empty response";
        return;
    }

    _requestSocket->write(newContent);
    _requestSocket->flush();
    _responseBuffer.append(newContent);
    parseResponse();

}
void RyConnection::onResponseClose(){
    qDebug()<<"response close";
    if(closed){
        return;
    }
    onResponseError();
}
void RyConnection::onResponseError(QAbstractSocket::SocketError){
    // TODO: markAsError
    if(_sendingPipeData){
        if(_sendingPipeData->responseHeaderRawData().isEmpty()){

            QByteArray s = QByteArray().append(QString("[Rythem:remote server unreachable]\n host:%1\n port:%2\n fullUrl:%3")
                    .arg(_sendingPipeData->host)
                    .arg(_sendingPipeData->port)
                    .arg(_sendingPipeData->fullUrl));
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
            _requestSocket->write(byteToWrite);
            _requestSocket->flush();
            _sendingPipeData->parseResponse(&byteToWrite);
            onResponsePackageFound();
            _sendingPipeData.clear();
            _responseState = ConnectionStateInit;
            if(_requestSocket){
                _requestSocket->blockSignals(true);
                _requestSocket->disconnect(this);
                _requestSocket->abort();
            }
            emit connectionClose();
            return;
        }else if(_sendingPipeData->isContentLenthUnLimit()){
            // response without content-length
            // qDebug()<<"respones close when content-length not found";
            onResponsePackageFound();
            _sendingPipeData.clear();
            _responseState = ConnectionStateInit;
            if(_requestSocket){
                _requestSocket->blockSignals(true);
                _requestSocket->disconnect(this);
                _requestSocket->abort();
            }
            emit connectionClose();
            closed = true;
        }else{
            //maybe remote connection disconnect with error
            //   when the response doesn't complete.
            //if(!_sendingPipeData->isPackageFound()){
            //   _sendingPipeData->markAsError();
            //}
            //onResponsePackageFound();
        }
        //_sendingPipeData.clear();
    }
    /*
    _responseState = ConnectionStateInit;
    if(_requestSocket){
        _requestSocket->blockSignals(true);
        _requestSocket->disconnect(this);
        _requestSocket->abort();
    }
    emit connectionClose();
    closed = true;
    */
}

void RyConnection::onRequestHeaderFound(){
    _requestState = ConnectionStateHeadFound;
}

void RyConnection::onRequestPackageFound(){
    if(closed){
        return;
    }
    _receivingPerformance.requestDone = QDateTime::currentMSecsSinceEpoch();
    if(_receivingPipeData->method == "CONNECT"/* &&
            _receivingPipeData->port != 80*/){// TODO: 80端口一定不是https?
        _isConnectTunnel = true;
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
    if(closed){
        return;
    }
    _sendingPerformance.responseDone = QDateTime::currentMSecsSinceEpoch();
    _sendingPipeData->performances = _sendingPerformance;
    _responseState = ConnectionStatePackageFound;
    emit pipeComplete(_sendingPipeData);

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

    RyRuleManager *manager = RyRuleManager::instance();//qApp->applicationDirPath()+"/config.txt");
    QList<QSharedPointer<RyRule> > matchResult;
    matchResult = manager->getMatchRules(_sendingPipeData->fullUrl);
    if(matchResult.size()>0){
        _sendingPipeData->isMatchingRule = true;
    }else{
        _sendingPipeData->isMatchingRule = false;
    }
    for(int i=0,l=matchResult.size();i<l;++i){
        QSharedPointer<RyRule> rule = matchResult.at(i);
        qDebug()<<"rule found"<<rule->toJSON();
        if(rule->type() == RyRule::COMPLEX_ADDRESS_REPLACE ||
                rule->type() == RyRule::SIMPLE_ADDRESS_REPLACE){
            host = rule->replace();
            _sendingPipeData->replacedHost = host;
        }else{
            QPair<QByteArray,QByteArray> headerAndBody = manager->getReplaceContent(rule,_sendingPipeData->fullUrl);
            //qDebug()<<headerAndBody.second;
            //qDebug()<<headerAndBody.first;
            bool isOk;
            QByteArray ba = headerAndBody.first;
            _sendingPipeData->parseResponse(&ba,&isOk);
            if(isOk){
                _requestSocket->write(headerAndBody.first);
                _requestSocket->flush();
                onResponseHeaderFound();
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


    // check if is request to self

    //qDebug()<<host<<QString::number(port);
    if(host == RyProxyServer::instance()->serverAddress().toString()
            && port == RyProxyServer::instance()->serverPort()){
        // TODO: move these mapping to global util
        qDebug()<<"simple http"<<_sendingPipeData->path;
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

            /*
            RyProxyServer::instance()->cacheSocket(
                        _connectingHost,
                        _connectingPort,
                        _responseSocket);

            */
            _connectingHost = host;
            _connectingPort = port;

            //   get new socket
            /*
            QMetaObject::invokeMethod(
                                  RyProxyServer::instance(),
                                  "getSocket",
                                  Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(QTcpSocket *,_responseSocket),
                                  Q_ARG(QString ,host),
                                  Q_ARG(quint16,port),
                                  Q_ARG(bool *,&_isSocketFromCache),
                                  Q_ARG(QThread*,QThread::currentThread()));
            */
            // 暂时不缓存socket
            _responseSocket->abort();
            _responseSocket->blockSignals(true);
            delete _responseSocket;
            _responseSocket = NULL;
            _responseSocket = new QTcpSocket(this);


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
        /*
        QMetaObject::invokeMethod(
                              RyProxyServer::instance(),
                              "getSocket",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QTcpSocket *,_responseSocket),
                              Q_ARG(QString ,host),
                              Q_ARG(quint16,port),
                              Q_ARG(bool *,&_isSocketFromCache),
                              Q_ARG(QThread*,QThread::currentThread()));
        */
        _responseSocket = new QTcpSocket(this);

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
        /*
        QPair<QString,int> serverAndPort = queryProxy(_sendingPipeData->fullUrl);
        if(serverAndPort.first == RyProxyServer::instance()->serverAddress().toString()
                && serverAndPort.second == RyProxyServer::instance()->serverPort()){
            qWarning()<<"warining: proxy is your self!";
        }else{
            if(!serverAndPort.first.isEmpty()){
                _responseSocket->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,serverAndPort.first,serverAndPort.second));
            }
        }
        */
        _responseSocket->connectToHost(host,port);
    }else{
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
    locker.unlock();
    return RyPipeData_ptr();
}
void RyConnection::appendPipe(RyPipeData_ptr thePipeData){
    _pipeList.append(thePipeData);

}
