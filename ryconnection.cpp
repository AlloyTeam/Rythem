#include "ryconnection.h"
#include "ryproxyserver.h"

#include <QApplication>

RyConnection::RyConnection(int socketHandle,QObject* parent)
    :QObject(parent),
      _handle(socketHandle),
      _requestSocket(NULL){

    closed = false;
    _isConnectTunnel = false;
    _requestState = _responseState = ConnectionStateInit;
    //qDebug()<<"RyConnection"<<socketHandle;
    //setHandle(socketHandle);
}

RyConnection::~RyConnection(){
    //qDebug()<<"~RyConnection";
    if(_requestSocket){
        _requestSocket->blockSignals(true);
        if(_requestSocket->isOpen()){
            _requestSocket->disconnectFromHost();
        }
        delete _requestSocket;
        _requestSocket = NULL;
    }
    if(_responseSocket){
        _responseSocket->blockSignals(true);
        if(_responseSocket->isOpen()){
            _responseSocket->abort();
        }
        delete _responseSocket;
        _responseSocket = NULL;
    }

    qDebug()<<"~RyConnection in main"
            << (QApplication::instance()->thread() == QThread::currentThread());
    QThread::currentThread()->quit();
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
    QMutexLocker locker(&pipeDataMutex);
    Q_UNUSED(locker)
    //qDebug()<<"onRequestReadyRead";
    QByteArray newContent = _requestSocket->readAll();

    _requestBuffer.append(newContent);
    parseRequest();
}
void RyConnection::onRequestClose(){
    qDebug()<<"request close"<<handle();
    //QMutexLocker locker(&pipeDataMutex);
    //Q_UNUSED(locker)
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
void RyConnection::onRequestError(QAbstractSocket::SocketError){
    QMutexLocker locker(&pipeDataMutex);
    Q_UNUSED(locker)
    qDebug()<<"request error";

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
            qDebug()<<"respones close when content-length not found and has content large than 100";
            //qDebug()<<_sendingPipeData->responseBodyRawData();
            emit pipeComplete(_sendingPipeData);
        }else{//maybe remote connection disconnect with error
            //if(!_sendingPipeData->isPackageFound()){
            //   _sendingPipeData->markAsError();
                emit pipeError(_sendingPipeData);
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
    qDebug()<<_responseSocket->state() ;
    _connectionNotOkTime = 0;
    //QMutexLocker locker(&pipeDataMutex);
    //Q_UNUSED(locker)
    //qDebug()<<"---onResponseConnected";
    //because the socket will be reuse. so remove unnecessary signal/slot connections
    //disconnect(_responseSocket,SIGNAL(connected()),this,SLOT(onResponseConnected()));

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
        qDebug()<<"to write:\n"<<_sendingPipeData->dataToSend();
        _responseSocket->write(_sendingPipeData->dataToSend());
        _responseSocket->flush();
    }
}
void RyConnection::onResponseReadyRead(){
    qDebug()<<_responseSocket->state();
    //QMutexLocker locker(&pipeDataMutex);
    //Q_UNUSED(locker)
    //quint64 length = _responseSocket->bytesAvailable();
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

}
void RyConnection::onResponseClose(){
    qDebug()<<"response close";
    closed = true;
}
void RyConnection::onResponseError(QAbstractSocket::SocketError err){
    QMutexLocker locker(&pipeDataMutex);
    Q_UNUSED(locker)
    qDebug()<<"response error "<<err;
    //if(!_sendingPipeData.isNull()){
    //          qDebug()<<_sendingPipeData->fullUrl;
    //}
    //if(_responseSocket && _responseSocket->bytesAvailable()!=0){
    if(_sendingPipeData){
        if(_sendingPipeData->isContentLenthUnLimit()){
            //_responseState = ConnectionStatePackageFound;
            qDebug()<<"respones close when content-length not found and has content large than 100";
            //qDebug()<<_sendingPipeData->responseBodyRawData();
            emit pipeComplete(_sendingPipeData);
        }else{//maybe remote connection disconnect with error
            //if(!_sendingPipeData->isPackageFound()){
            //   _sendingPipeData->markAsError();
                emit pipeError(_sendingPipeData);
            //}

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
}

void RyConnection::onRequestHeaderFound(){
    if(_isConnectTunnel){
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
    _requestState = ConnectionStateHeadFound;
}

void RyConnection::onRequestPackageFound(){
    if(_receivingPipeData->method == "CONNECT"){
        _isConnectTunnel = true;
        qDebug()<<"requestPackageFound"
                <<_receivingPipeData->requestHeaderRawData()
                <<_receivingPipeData->requestBodyRawData().size()
                <<_receivingPipeData->dataToSend();
    }
    _requestState = ConnectionStatePackageFound;
    /*emit pipeBegin(_receivingPipeData);
    if(false && _receivingPipeData->method == "CONNECT"){
        _requestSocket->write(QByteArray("HTTP/1.1 200 Connection established"
                                         "\r\n"
                                         "Connection: keep-alive\r\n\r\n"));
        _requestSocket->flush();
        _receivingPipeData.clear();
        return;
    }else{
    */
        emit pipeBegin(_receivingPipeData);
        appendPipe(_receivingPipeData);
        _receivingPipeData.clear();
        doRequestToNetwork();
    //}
}

void RyConnection::onResponseHeaderFound(){
    _responseState = ConnectionStateHeadFound;
    /*
    if(_connectionNotOkTime>0){
        qDebug()<<" is socket from cache:"
                <<_isSocketFromCache
                <<" not ok times:"<<_connectionNotOkTime
                <<" domain"<<_sendingPipeData->host
                <<"response ok now\n"
                <<_sendingPipeData->responseHeaderRawData()
                <<"\nremain:\n"
                <<_responseBuffer;
    }
    _connectionNotOkTime = 0;
    */
}

void RyConnection::onResponsePackageFound(){
    _responseState = ConnectionStatePackageFound;
    emit pipeComplete(_sendingPipeData);
    /*
    qDebug()<<"PackageFound:\n"
            <<_sendingPipeData->fullUrl
            <<_sendingPipeData->responseHeaderRawData()
            <<_sendingPipeData->responseBodyRawData().size();
    */
    _sendingPipeData.clear();
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
        _receivingPipeData = RyPipeData_ptr(new RyPipeData());
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
                if(_sendingPipeData->isContentLenthUnLimit() &&
                        _sendingPipeData->responseBodyRawData().size()>100){
                    qDebug()<<"some buggy server do this...";
                    return;
                }

                onResponsePackageFound();
                if(!_responseBuffer.isEmpty()){
                    qDebug()<<"after response package found:\n"
                            <<_responseBuffer.size();
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

            if(_sendingPipeData->isContentLenthUnLimit() &&
                    _sendingPipeData->responseBodyRawData().size()>100){
                qDebug()<<"some buggy server do this...";
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
        //qDebug()<<"wait for last pipe finish"
        //        <<_sendingPipeData->fullUrl
        //        <<_sendingPipeData->responseHeaderRawData()
        //        <<_sendingPipeData->responseBodyRawData();
        return;
    }
    if(_sendingPipeData.isNull()){
        //qDebug()<<"doRequestToNetwork : no any more pipe pending";
        return;
    }
    //qDebug()<<"doRequestToNetwork"<<_sendingPipeData->fullUrl;
    QString host = _sendingPipeData->host;
    quint16 port = _sendingPipeData->port;
    //qDebug()<<host<<port;
/*
    if(RyRuleManager::isMathRule(_pipeData)){
        if(isHostRule){
            host = hostToReplace;
        }else if(isContentRule){
            parseResponse(contentToReplaceWithHeaderAndBody);
            return;
        }
    }
*/
    qDebug()<<"connecting:"<<_connectingHost<<_connectingPort;
    qDebug()<<"to connect:"<<host<<port;
    _fullUrl = _sendingPipeData->fullUrl;
    if(_responseSocket){
        if(_connectingHost.toLower() == host.toLower() &&
                _connectingPort == port){
            //qDebug()<<"old responeSocket";
            // do nothing
            // just reuse previous response socket
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

    if(_responseSocket->state() != QAbstractSocket::ConnectedState){
        qDebug()<<"responseSocket is not open"
                <<"connect to "<<host<<port
                <<_responseSocket->readAll();
        connect(_responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
        _responseState = ConnectionStateConnecting;
        _responseSocket->connectToHost(host,port);
    }else{

        qDebug()<<"-----responseSocket is open already\n"
                  <<_sendingPipeData->dataToSend();
        onResponseConnected();
    }
}
RyPipeData_ptr RyConnection::nextPipe(){
    QMutexLocker locker(&pipeDataListMutex);
    Q_UNUSED(locker)
    if(_pipeList.length()>0){
        //qDebug()<<"next pipe";
        return _pipeList.takeAt(0);
    }
    return RyPipeData_ptr();
}
void RyConnection::appendPipe(RyPipeData_ptr thePipeData){
    QMutexLocker locker(&pipeDataListMutex);
    Q_UNUSED(locker)
    //qDebug()<<"append pipe to list";
    _pipeList.append(thePipeData);

}
