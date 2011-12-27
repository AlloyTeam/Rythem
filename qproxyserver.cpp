#include "qproxyserver.h"
#include "qpipe.h"

QProxyServer::QProxyServer(QObject *parent) :
    QTcpServer(parent){
}

void QProxyServer::incomingConnection(int socketId){
    QPipe *pipe = new QPipe(nextPendingConnection());
    pipes.append(pipe);

    connect(pipe,SIGNAL(connected()),SLOT(onPipeConnected()));
    connect(pipe,SIGNAL(completed()),SLOT(onPipeComplete()));
    connect(pipe,SIGNAL(error()),SLOT(onPipeError()));

    pipe->start();
}
void QProxyServer::onPipeConnected(){

}
void QProxyServer::onPipeComplete(){

}
void QProxyServer::onPipeError(){

}
