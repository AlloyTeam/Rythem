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
void QiProxyServer::onPipeConnected(ConnectionData_ptr p){
    // do nothing?
    Q_UNUSED(p);
}
void QiProxyServer::onPipeComplete(ConnectionData_ptr p){
    if(p){
        removePipe(p->id);
    }
}
void QiProxyServer::onPipeError(ConnectionData_ptr p){
    if(p){
        removePipe(p->id);
    }
}

QiPipe* QiProxyServer::addPipe(int socketDescriptor){
    QiPipe *pipe = new QiPipe(socketDescriptor);//delete in removePipe(int)
    connect(pipe,SIGNAL(connected(ConnectionData_ptr)),SLOT(onPipeConnected(ConnectionData_ptr)));
    connect(pipe,SIGNAL(connected(ConnectionData_ptr)),SIGNAL(newPipe(ConnectionData_ptr)));
    connect(pipe,SIGNAL(completed(ConnectionData_ptr)),SLOT(onPipeComplete(ConnectionData_ptr)));
    connect(pipe,SIGNAL(completed(ConnectionData_ptr)),SIGNAL(pipeUpdate(ConnectionData_ptr)));
    connect(pipe,SIGNAL(error(ConnectionData_ptr)),SLOT(onPipeError(ConnectionData_ptr)));
    connect(pipe,SIGNAL(error(ConnectionData_ptr)),SIGNAL(pipeUpdate(ConnectionData_ptr)));


    pipes[socketDescriptor]=pipe;
    QThread *t = new QThread();
    threads[socketDescriptor] = t;
    pipe->moveToThread(t);

    connect(t,SIGNAL(started()),pipe,SLOT(run()));
    t->start();

    return pipe;
}

bool QiProxyServer::removePipe(int socketId){
    if(pipes.contains(socketId)){
        //qDebug()<<"removePipe contains.."<<socketId;
        QiPipe *p = pipes.value(socketId);
        QThread *t = threads.value(socketId);

        threads.remove(socketId);
        pipes.remove(socketId);

        p->deleteLater();
        t->quit();
        t->wait(100);
        //delete p;
    }
    return false;
}
void QiProxyServer::removeAllPipe(){
    while(pipes.size()>0){
        int key = pipes.begin().key();
        removePipe(key);
    }
}
