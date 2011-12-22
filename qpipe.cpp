#include "qpipe.h"

#include <QRegExp>

QPipe::QPipe(QTcpSocket *socket) :
    QObject(socket),
    requestSocket(socket){

    reqRawString = new QString();
    reqByteArray = new QByteArray();

    connect(socket,SIGNAL(readyRead()),SLOT(onReqSocketReadReady()));
    connect(socket,SIGNAL(readChannelFinished()),SLOT(onReqSocketReadFinished()));
    //connect(socket,SIGNAL(hostFound()),SLOT(onRequestHostFound()));??
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onRequestSocketError()));
}
void QPipe::onReqSocketReadReady(){
    QObject *id = sender();
    if(id==0)return;
    QTcpSocket* socket = static_cast<QTcpSocket*>(id);
    if(!socket)return;
    QString rawRequest;
    char buffer[1024];
    while(socket->bytesAvailable()){
        int readCount = socket->read((char*)buffer,1024);
        rawRequest.append((char*)buffer);
        reqByteArray->append(buffer,readCount);
    }
    qDebug()<<rawRequest;
    qDebug()<<QString(*reqByteArray);
    parseHeader(rawRequest);

}
void QPipe::onReqSocketReadFinished(){
    QObject *id = sender();
    if(id==0)return;
    QTcpSocket* socket = static_cast<QTcpSocket*>(id);
    if(!socket)return;
    qDebug()<<"readfinished..";
}
void QPipe::onRequestHostFound(){//??
    /*
    QObject *id = sender();
    if(id==0)return;
    QTcpSocket* socket = static_cast<QTcpSocket*>(id);
    if(!socket)return;
    */

}
void QPipe::onRequestSocketError(){
    emit(error());
}

QMap<QString,QString>* QPipe::parseHeader(const QString headerString){
    int headerPosition = headerString.indexOf(QRegExp("\r?\n\r?\n"));
    qDebug()<<"headerPosition:"<<headerPosition<<" totalLength="<<(headerString.length());

}
QPipe::~QPipe(){
    delete reqRawString;
    reqRawString = NULL;
    delete requestSocket;
    requestSocket = NULL;
    delete reqByteArray;
    reqByteArray = NULL;
}
