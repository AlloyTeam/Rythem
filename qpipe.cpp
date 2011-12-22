#include "qpipe.h"
#include <QStringList>
#include <QRegExp>
const static QRegExp headerSplitter("\r?\n\r?\n");
const static QRegExp newLine("\r?\n");
QPipe::QPipe(QTcpSocket *socket) :
    QObject(socket),
    requestSocket(socket){

    reqRawString = new QString();
    reqByteArray = new QByteArray();
    pipeData = new PipeData();

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

void QPipe::parseHeader(const QString headerString){
    int headerPosition = headerString.indexOf(headerSplitter);
    if(headerPosition==-1)return;
    qDebug()<<"headerPosition:"<<headerPosition<<" totalLength="<<(headerString.length());
    QStringList ll = headerString.split(headerSplitter);
    QString header = ll[0];
    QStringList headers = header.split(newLine);
    int l = headers.size();
    int i=0;
    for(;i<l;++i){
        QString s = headers[i];
        QStringList tmp = s.split(": ");
        if(tmp.size() == 2){
            pipeData->setHeader(tmp[0],tmp[1]);
        }
    }
/*
"GET / HTTP/1.1
Host: 127.0.0.1:8080
Connection: keep-alive
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_6_8) AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.63 Safari/535.7
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,* /*;q=0.8
Accept-Encoding: gzip,deflate,sdch
Accept-Language: zh-CN,zh;q=0.8
Accept-Charset: gb18030,utf-8;q=0.7,*;q=0.3
*/
    qDebug()<<"host="<<pipeData->getHeader("Host");
    if(pipeData->getHeader("Host") == "127.0.0.1:8080"){//避免死循环
        QByteArray byteToWrite;
        QString s = "hello Qiddler";
        int count = s.length();
        byteToWrite.append(QString("HTTP/1.1 200 OK\r\nServer: Qiddler\r\nContent-Type: text/html\r\nContent-Length: %1\r\n\r\n").arg(count));
        byteToWrite.append(s);
        requestSocket->write(byteToWrite);
        requestSocket->flush();
        return;
    }
}
QPipe::~QPipe(){
    delete reqRawString;
    reqRawString = NULL;
    delete requestSocket;
    requestSocket = NULL;
    delete reqByteArray;
    reqByteArray = NULL;
}
