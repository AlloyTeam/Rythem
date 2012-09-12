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
    //qDebug()<<"responseConnected";
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
    if(closed){
        qDebug()<<"closed already";
        return;
    }
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
        qDebug()<<"closed";
        return;
    }
    _receivingPerformance.requestDone = QDateTime::currentMSecsSinceEpoch();
    if(_receivingPipeData->method == "CONNECT"/* &&
            _receivingPipeData->port != 80*/){// TODO: 80端口一定不是https?
        qDebug()<<"is connect:"<<QString(_receivingPipeData->requestHeaderRawData());
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

    QString oldHost = _connectingHost;
    quint64 oldPort = _connectingPort;

    _connectingHost = _sendingPipeData->host;
    _connectingPort = _sendingPipeData->port;
    if(checkRule(_sendingPipeData)){
        return;
    }
    if(checkLocalWebServer(_sendingPipeData)){
        return;
    }



    //qDebug()<<"connecting:"<<_connectingHost<<_connectingPort;
    //qDebug()<<"to connect:"<<host<<port;
    _fullUrl = _sendingPipeData->fullUrl;
    if(!_responseSocket ||
        _connectingHost.toLower() != oldHost ||
        _connectingPort != oldPort){

        getNewResponseSocket(_sendingPipeData);
    }else{
        //qDebug()<<"reuse old socket";
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
        //qDebug()<<"connecting to "<<_connectingHost;
        connect(_responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
        _responseSocket->connectToHost(_connectingHost,_connectingPort);
    }else{
        //qDebug()<<"state = "<<_responseSocket->state();
        if(responseState == QAbstractSocket::ConnectedState){
            onResponseConnected();
        }else{
            //qDebug()<<"responseSocket not open yet"<<responseState;
            _responseSocket->blockSignals(true);
            _responseSocket->disconnectFromHost();
            _responseSocket->abort();
            _responseSocket->blockSignals(false);
            connect(_responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
            _responseSocket->connectToHost(_connectingHost,_connectingPort);
        }
    }
}

bool RyConnection::checkLocalWebServer(RyPipeData_ptr& pipe){
    QString host = pipe->host;
    quint16 port = pipe->port;
    // check if is request to self
    //qDebug()<<host<<QString::number(port);
    if(host == RyProxyServer::instance()->serverAddress().toString()
            && port == RyProxyServer::instance()->serverPort()){
        // TODO: move these mapping to global util
        qDebug()<<"simple http"<<pipe->path;
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
        QString filePath = pipe->path;
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
        pipe->parseResponse(&byteToWrite);
        onResponseHeaderFound();
        onResponsePackageFound();
        return true;
    }
    return false;
}

bool RyConnection::checkRule(RyPipeData_ptr& pipe){

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
            _connectingHost = rule->replace();
            _sendingPipeData->replacedHost = _connectingHost;
        }else{
            bool isResouceFound = true;
            QPair<QByteArray,QByteArray> headerAndBody = manager->getReplaceContent(rule,_sendingPipeData->fullUrl,&isResouceFound);
            // TODO check settings if go through when local replace match but got 404
            if(rule->type() == RyRule::LOCAL_DIR_REPLACE && !isResouceFound){
                return false;
            }

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
                return true;
            }else{
                break;
            }
        }
    }
    return false;
}

void RyConnection::getNewResponseSocket(RyPipeData_ptr&){
    if(_responseSocket){
        _responseSocket->blockSignals(true);
        _responseSocket->disconnect(this);
        _responseSocket->abort();
        _responseSocket->blockSignals(false);
        delete _responseSocket;
        _responseSocket = NULL;
    }
    _responseSocket = new QTcpSocket(this);

    connect(_responseSocket,SIGNAL(readyRead()),SLOT(onResponseReadyRead()));
    connect(_responseSocket,SIGNAL(aboutToClose()),SLOT(onResponseClose()));
    connect(_responseSocket,SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(onResponseError(QAbstractSocket::SocketError)));
    //connect(_responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
}

RyPipeData_ptr RyConnection::nextPipe(){
    QMutexLocker locker(&pipeDataListMutex);
    Q_UNUSED(locker)
    if(_pipeList.length()>0){
        RyPipeData_ptr thePipeData = _pipeList.takeAt(0);
        //qDebug()<<"taking "<<thePipeData->fullUrl;
        return thePipeData;
    }
    locker.unlock();
    return RyPipeData_ptr();
}
void RyConnection::appendPipe(RyPipeData_ptr thePipeData){
    //qDebug()<<"appending "<<thePipeData->fullUrl;
    _pipeList.append(thePipeData);

}
