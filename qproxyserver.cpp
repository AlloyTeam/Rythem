#include "qproxyserver.h"
#include "qpipe.h"

QProxyServer::QProxyServer(QObject *parent) :
    QTcpServer(parent){
}

void QProxyServer::incomingConnection(int socketId){
    QTcpSocket *socket = new QTcpSocket();
    socket->setSocketDescriptor(socketId);
    QPipe *pipe = addPipe(socket);

    connect(pipe,SIGNAL(connected(QSharedPointer<PipeData>)),SLOT(onPipeConnected(QSharedPointer<PipeData>)));
    connect(pipe,SIGNAL(connected(QSharedPointer<PipeData>)),SIGNAL(newPipe(QSharedPointer<PipeData>)));
    connect(pipe,SIGNAL(completed(QSharedPointer<PipeData>)),SLOT(onPipeComplete(QSharedPointer<PipeData>)));
    connect(pipe,SIGNAL(completed(QSharedPointer<PipeData>)),SIGNAL(pipeUpdate(QSharedPointer<PipeData>)));
    connect(pipe,SIGNAL(error(QSharedPointer<PipeData>)),SLOT(onPipeError(QSharedPointer<PipeData>)));
    connect(pipe,SIGNAL(error(QSharedPointer<PipeData>)),SIGNAL(pipeUpdate(QSharedPointer<PipeData>)));

    pipe->start();
}
void QProxyServer::onPipeConnected(QSharedPointer<PipeData> p){
    // do nothing?
    Q_UNUSED(p);
}
void QProxyServer::onPipeComplete(QSharedPointer<PipeData> p){
    removePipe(p->socketId);
}
void QProxyServer::onPipeError(QSharedPointer<PipeData> p){
    removePipe(p->socketId);
}

QPipe* QProxyServer::addPipe(QTcpSocket *socket){
    QPipe *p = new QPipe(socket);//delete in removePipe(int)
    pipes.value(socket->socketDescriptor(),p);
    return p;
}

bool QProxyServer::removePipe(int socketId){
    if(pipes.contains(socketId)){
        QPipe *p = pipes.value(socketId);
        pipes.remove(socketId);
        delete p;
    }
    return false;
}
