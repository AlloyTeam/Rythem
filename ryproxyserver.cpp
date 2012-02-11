#include "ryproxyserver.h"

Q_GLOBAL_STATIC(RyProxyServer,ryProxyServer)
RyProxyServer* RyProxyServer::instance(){
    return ryProxyServer();
}

RyProxyServer::RyProxyServer(QObject *parent) :
    QTcpServer(parent){
    //qDebug()<<"server initialing";

}
RyProxyServer::~RyProxyServer(){
    qDebug()<<"~RyProxyServer begin";
    close();
    qDebug()<<"~RyProxyServer done";
}

void RyProxyServer::close(){
    QTcpServer::close();
    // close all sockets
    for(int i=0,l=_connections.length();i<l;++i){
        //qDebug()<<"delete connection"<<l<<i;
        RyConnection *connection = _connections.takeFirst();
        QThread *thread = _threads.take(connection);
        connection->disconnect(this);
        //_cacheConnections.remove(connection);
        connection->deleteLater();
        thread->deleteLater();
    }
    for(int i=0,l=_cachedSockets.values().size();i<l;++i){
        QTcpSocket *socket = _cachedSockets.values().takeFirst();
        socket->deleteLater();
    }
}

void RyProxyServer::run(){

}


void RyProxyServer::cacheSocket(QString address,quint16 port,QTcpSocket* socket){
    qDebug()<<"CACHE SOCKET cacheSocket"<<address<<port;
    // TODO the key QString("%1:%2").arg(address.toLower()).arg(port) should be a function or macro
    QMutexLocker locker(&_socketsOpMutex);
    Q_UNUSED(locker)
    socket->moveToThread(this->thread());
    if(_cachedSockets.values().contains(socket)){
        qDebug()<<"already cache this!";
    }else{
        _cachedSockets.insert(QString("%1:%2").arg(address.toLower()).arg(port) ,socket);
    }
}
QTcpSocket* RyProxyServer::getSocket(QString address,quint16 port,bool* isFromCache,QThread* _desThread){
    qDebug()<<"getSocket";
    if(isFromCache) *isFromCache = false;
    QMutexLocker locker(&_socketsOpMutex);
    Q_UNUSED(locker)
    QTcpSocket *theSocket = NULL;
    QString theKey = QString("%1:%2").arg(address.toLower()).arg(port);
    if(_cachedSockets.contains( theKey )){
        if(isFromCache) *isFromCache = true;

        QList<QTcpSocket*> sockets = _cachedSockets.values(theKey);
        theSocket = sockets.at(0);
        qDebug()<<"return cached socket";
        int n = _cachedSockets.remove(theKey,theSocket);
        if(n!=1){
            qDebug()<<"!!!!!DELETE FAILED!!!";
        }

    }else{
        qDebug()<<"new response socket";
        theSocket = new QTcpSocket();
    }
    theSocket->moveToThread(_desThread);
    return theSocket;
}

// connection request comming
//   generate an RyConnection
//   and use handle identify it.
//
//   when new connection come and has the same handle.
//   reuse old RyConnection
void RyProxyServer::incomingConnection(int handle){
    RyConnection* connection = _getConnection(handle);
    Q_UNUSED(connection)
}
// private functions
RyConnection * RyProxyServer::_getConnection(int handle){
    QMutexLocker locker(&connectionOpMutex);
    Q_UNUSED(locker)
    //qDebug()<<"getConnection"<<handle;
    //if(!_cacheConnections.contains(handle)){
        QThread *newThread = new QThread();

        RyConnection *connection = new RyConnection(handle);

        connect(connection,SIGNAL(idleTimeout()),SLOT(onConnectionIdleTimeout()));

        connect(connection,SIGNAL(pipeBegin(RyPipeData_ptr)),
                SLOT(onPipeBegin(RyPipeData_ptr)));
        connect(connection,SIGNAL(pipeComplete(RyPipeData_ptr)),
                SLOT(onPipeComplete(RyPipeData_ptr)));
        connect(connection,SIGNAL(pipeError(RyPipeData_ptr)),
                SLOT(onPipeError(RyPipeData_ptr)));

        connect(connection,SIGNAL(connectionClose()),
                SLOT(onConnectionIdleTimeout()));

        connect(connection,SIGNAL(pipeBegin(RyPipeData_ptr)),
                SIGNAL(pipeBegin(RyPipeData_ptr)));
        connect(connection,SIGNAL(pipeComplete(RyPipeData_ptr)),
                SIGNAL(pipeComplete(RyPipeData_ptr)));
        connect(connection,SIGNAL(pipeError(RyPipeData_ptr)),
                SIGNAL(pipeError(RyPipeData_ptr)));

        _connections.append(connection);
        _threads[connection] = newThread;

        connection->moveToThread(newThread);
        connect(newThread,SIGNAL(started()),connection,SLOT(run()));
        //connect(newThread,SIGNAL(destroyed()),SLOT(onThreadTerminated()));
        newThread->start();
        return connection;
}


// private slots:
void RyProxyServer::onConnectionIdleTimeout(){
    RyConnection *connection = qobject_cast<RyConnection*>(sender());
    _connections.removeOne(connection);
    //connection->blockSignals(true);
    //connection->disconnect(this);
    //_cacheConnections.remove(connection);
    connection->deleteLater();
}


void RyProxyServer::onPipeBegin(RyPipeData_ptr){
    //qDebug()<<"pipe begin";
}

void RyProxyServer::onPipeComplete(RyPipeData_ptr){
    //qDebug()<<"pipe complete";
}

void RyProxyServer::onPipeError(RyPipeData_ptr){
    //qDebug()<<"pipe error";
}


void RyProxyServer::onThreadTerminated(){
    qDebug()<<"thread terminated";
}

void RyProxyServer::onConnectionClosed(){
    qDebug()<<"connection closed";
    RyConnection *connection = qobject_cast<RyConnection*>(sender());

    QThread *thread = _threads.take(connection);
    connection->disconnect(this);
    //_cacheConnections.remove(connection);
    connection->deleteLater();
    thread->deleteLater();
}
