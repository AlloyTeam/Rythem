#include "qiproxyserver.h"
#include "qipipe.h"
#include <QThreadPool>

#define MAXSOCKET 100

long QiProxyServer::connectionId = 0;
QMutex QiProxyServer::connectionIdMutex;

QiProxyServer::QiProxyServer(QObject *parent) :
    QTcpServer(parent){
}

void QiProxyServer::incomingConnection(int socketId){
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    if(pushSocket(socketId) == socketId){
        addPipe(socketId);
    }
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
        bool isRemoveSuccess = removePipe(pipe->sockeId());
        if(!isRemoveSuccess){
            //qDebug()<<"remove pipe fail";
        }
    }else{
        qDebug()<<"ERROR...QiProxyServer::onPipeFinished";
    }
    int handle = fetchPendingSocket();
    if(handle!=-1){
        addPipe(handle);
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
        //qDebug()<<"removePipe contains.."<<connectionId;
        QiPipe *p = pipes.value(connectionId);
        QThread *t = threads.value(connectionId);

        threads.remove(connectionId);
        pipes.remove(connectionId);

        p->deleteLater();
        t->quit();
        //qDebug()<<"waiting..";
        //t->wait(10);
        //qDebug()<<"quit thread..";
        return true;
    }
    return false;
}
void QiProxyServer::removeAllPipe(){
    while(pipes.size()>0){
        int key = pipes.begin().key();
        removePipe(key);
    }
}

int QiProxyServer::fetchPendingSocket(){
    //qDebug()<<"fetching:"<<pendingSocketHanles.size();
    if(pendingSocketHanles.size()==0){
        return -1;
    }
    int n = pendingSocketHanles.at(0);
    pendingSocketHanles.remove(0,1);
    return n;
}

//如果需要等待，返回-1，否则返回handle
int QiProxyServer::pushSocket(int handle){
    if(pipes.size() >= MAXSOCKET){
        qDebug()<<"socket length more than "<<MAXSOCKET;
        pendingSocketHanles.append(handle);
        return -1;
    }
    return handle;
}
