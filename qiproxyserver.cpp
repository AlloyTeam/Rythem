#include "qiproxyserver.h"
#include "qipipe.h"
#include <QThreadPool>

long QiProxyServer::connectionId = 0;
QMutex QiProxyServer::connectionIdMutex;

QiProxyServer::QiProxyServer(QObject *parent) :
    QTcpServer(parent){
}

void QiProxyServer::incomingConnection(int socketId){
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    addPipe(socketId);
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
void QiProxyServer::onConnectionConnected(ConnectionData_ptr p){
    // do nothing?
    Q_UNUSED(p);
}
void QiProxyServer::onConnectionComplete(ConnectionData_ptr p){
    if(p){
        removePipe(p->id);
    }
}
void QiProxyServer::onConnectionError(ConnectionData_ptr p){
    if(p){
        removePipe(p->id);
    }
}

void QiProxyServer::onPipeFinished(){
    QiPipe* pipe = (QiPipe*)sender();
    if(pipe){
        removePipe(pipe->sockeId());
    }else{
        qDebug()<<"ERROR...QiProxyServer::onPipeFinished";
    }
}


QiPipe* QiProxyServer::addPipe(int socketDescriptor){
    QiPipe *pipe = new QiPipe(socketDescriptor);//delete in removePipe(int)
    connect(pipe,SIGNAL(connected(ConnectionData_ptr)),SLOT(onConnectionConnected(ConnectionData_ptr)));
    connect(pipe,SIGNAL(connected(ConnectionData_ptr)),SIGNAL(newPipe(ConnectionData_ptr)));
    connect(pipe,SIGNAL(completed(ConnectionData_ptr)),SLOT(onConnectionComplete(ConnectionData_ptr)));
    connect(pipe,SIGNAL(completed(ConnectionData_ptr)),SIGNAL(pipeUpdate(ConnectionData_ptr)));
    connect(pipe,SIGNAL(error(ConnectionData_ptr)),SLOT(onConnectionError(ConnectionData_ptr)));
    connect(pipe,SIGNAL(error(ConnectionData_ptr)),SIGNAL(pipeUpdate(ConnectionData_ptr)));

    connect(pipe,SIGNAL(pipeFinished()),SLOT(onPipeFinished()));


    pipes[socketDescriptor]=pipe;
    QThread *t = new QThread();
    threads[socketDescriptor] = t;
    pipe->moveToThread(t);

    connect(t,SIGNAL(started()),pipe,SLOT(run()));
    t->start();

    return pipe;
}

bool QiProxyServer::removePipe(int connectionId){
    if(pipes.contains(connectionId)){
        //qDebug()<<"removePipe contains.."<<socketId;
        QiPipe *p = pipes.value(connectionId);
        QThread *t = threads.value(connectionId);

        threads.remove(connectionId);
        pipes.remove(connectionId);

        //p->deleteLater();
        t->quit();
        t->wait(100);
        delete p;
    }
    return false;
}
void QiProxyServer::removeAllPipe(){
    while(pipes.size()>0){
        int key = pipes.begin().key();
        removePipe(key);
    }
}
