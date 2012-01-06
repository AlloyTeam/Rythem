#include "qiproxyserver.h"
#include "qipipe.h"
#include <QThreadPool>

QiProxyServer::QiProxyServer(QObject *parent) :
    QTcpServer(parent){
}

void QiProxyServer::incomingConnection(int socketId){
    //QTcpSocket *socket = new QTcpSocket();
    //socket->setSocketDescriptor(socketId);
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    QiPipe *pipe = addPipe(socketId);

    connect(pipe,SIGNAL(connected(ConnectionData_ptr)),SLOT(onPipeConnected(ConnectionData_ptr)));
    connect(pipe,SIGNAL(connected(ConnectionData_ptr)),SIGNAL(newPipe(ConnectionData_ptr)));
    connect(pipe,SIGNAL(completed(ConnectionData_ptr)),SLOT(onPipeComplete(ConnectionData_ptr)));
    connect(pipe,SIGNAL(completed(ConnectionData_ptr)),SIGNAL(pipeUpdate(ConnectionData_ptr)));
    connect(pipe,SIGNAL(error(ConnectionData_ptr)),SLOT(onPipeError(ConnectionData_ptr)));
    connect(pipe,SIGNAL(error(ConnectionData_ptr)),SIGNAL(pipeUpdate(ConnectionData_ptr)));

    pipe->start();
    //pipe->setAutoDelete(false);
    //QThreadPool::globalInstance()->start(pipe);


}
QiProxyServer::~QiProxyServer(){
    this->close();
    qDebug()<<"~QiProxyServer";
    while(hasPendingConnections()){
        QTcpSocket *socket = nextPendingConnection();
        socket->close();
    }
    removeAllPipe();
}
void QiProxyServer::onPipeConnected(ConnectionData_ptr p){
    // do nothing?
    Q_UNUSED(p);
}
void QiProxyServer::onPipeComplete(ConnectionData_ptr p){
    removePipe(p->socketId);
}
void QiProxyServer::onPipeError(ConnectionData_ptr p){
    removePipe(p->socketId);
}

QiPipe* QiProxyServer::addPipe(int socketDescriptor){
    QiPipe *p = new QiPipe(socketDescriptor);//delete in removePipe(int)
    pipes[socketDescriptor]=p;
    return p;
}

bool QiProxyServer::removePipe(int socketId){
    if(pipes.contains(socketId)){
        //qDebug()<<"removePipe contains.."<<socketId;
        QiPipe *p = pipes.value(socketId);
        pipes.remove(socketId);
        p->deleteLater();
        //delete p;
    }
    return false;
}
void QiProxyServer::removeAllPipe(){
    while(pipes.size()>0){
        int key = pipes.begin().key();
        QiPipe *p = pipes[key];
        pipes.remove(key);
        delete p;
    }
}
