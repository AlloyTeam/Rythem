#include "qiproxyserver.h"
#include "qipipe.h"

QiProxyServer::QiProxyServer(QObject *parent) :
    QTcpServer(parent){
}

void QiProxyServer::incomingConnection(int socketId){
    QTcpSocket *socket = new QTcpSocket();
    socket->setSocketDescriptor(socketId);
    QiPipe *pipe = addPipe(socket);

    connect(pipe,SIGNAL(connected(Pipedata_const_ptr)),SLOT(onPipeConnected(Pipedata_const_ptr)));
    connect(pipe,SIGNAL(connected(Pipedata_const_ptr)),SIGNAL(newPipe(Pipedata_const_ptr)));
    connect(pipe,SIGNAL(completed(Pipedata_const_ptr)),SLOT(onPipeComplete(Pipedata_const_ptr)));
    connect(pipe,SIGNAL(completed(Pipedata_const_ptr)),SIGNAL(pipeUpdate(Pipedata_const_ptr)));
    connect(pipe,SIGNAL(error(Pipedata_const_ptr)),SLOT(onPipeError(Pipedata_const_ptr)));
    connect(pipe,SIGNAL(error(Pipedata_const_ptr)),SIGNAL(pipeUpdate(Pipedata_const_ptr)));

    pipe->start();
}
void QiProxyServer::onPipeConnected(Pipedata_const_ptr p){
    // do nothing?
    Q_UNUSED(p);
}
void QiProxyServer::onPipeComplete(Pipedata_const_ptr p){
    removePipe(p->socketId);
}
void QiProxyServer::onPipeError(Pipedata_const_ptr p){
    removePipe(p->socketId);
}

QiPipe* QiProxyServer::addPipe(QTcpSocket *socket){
    QiPipe *p = new QiPipe(socket);//delete in removePipe(int)
    pipes.value(socket->socketDescriptor(),p);
    return p;
}

bool QiProxyServer::removePipe(int socketId){
    if(pipes.contains(socketId)){
        QiPipe *p = pipes.value(socketId);
        pipes.remove(socketId);
        p->deleteLater();
    }
    return false;
}
