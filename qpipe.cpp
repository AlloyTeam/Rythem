#include "qpipe.h"
#include <QStringList>
#include <QRegExp>
const static QRegExp headerSplitter("\r?\n\r?\n");
const static QRegExp newLine("\r?\n");
QPipe::QPipe(QTcpSocket *socket) :
    QObject(socket),
    requestSocket(socket),
    headerFound(false){

    reqRawString = new QString();
    pipeData = new PipeData();

    connect(socket,SIGNAL(readyRead()),SLOT(onReqSocketReadReady()));
    connect(socket,SIGNAL(readChannelFinished()),SLOT(onReqSocketReadFinished()));
    //connect(socket,SIGNAL(hostFound()),SLOT(onRequestHostFound()));??
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onRequestSocketError()));
    connect(socket,SIGNAL(aboutToClose()),SLOT(onRequestSocketClose()));
}
void QPipe::onReqSocketReadReady(){
    QObject *id = sender();
    if(id==0)return;
    QTcpSocket* socket = static_cast<QTcpSocket*>(id);
    if(!socket)return;



    if(headerFound){
        if(responseSocket && responseSocket->isWritable()){
            responseSocket->write(QByteArray(socket->readAll()));
        }else{
            reqByteArray.append(QByteArray(socket->readAll()));
        }
        qDebug()<<"readReady headerFound:\n"<<QString(reqByteArray);
        return;
    }
    reqByteArray.append(QByteArray(socket->readAll()));
    //reqByteArray = reqByteArray.replace("\r\n","\n");
    reqHeaderString = reqByteArray.mid(reqByteArray.indexOf("\r\n"));
    qDebug()<<"readReady header NOT found:\n"<<QString(reqByteArray);
    //reqByteArray = QByteArray(socket->readAll());
    //qDebug()<<QString(reqByteArray);
    parseHeader(QString(reqByteArray));
    reqByteArray.clear();
    reqByteArray.append(reqSig);
    reqByteArray.append(reqHeaderString);
    qDebug()<<"host="<<pipeData->getHeader("Host")<<pipeData->getHeader("Port");
    if(pipeData->getHeader("Host") == "127.0.0.1" && pipeData->getHeader("Port")=="8080"){//避免死循环
        QByteArray byteToWrite;
        QString s = "hello Qiddler";
        int count = s.length();
        byteToWrite.append(QString("HTTP/1.1 200 OK\r\nServer: Qiddler\r\nContent-Type: text/html\r\nContent-Length: %1\r\n\r\n").arg(count));
        byteToWrite.append(s);
        requestSocket->write(byteToWrite);
        requestSocket->flush();
        requestSocket->close();
        return;
    }else{
        responseSocket = new QTcpSocket();
        connect(responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
        connect(responseSocket,SIGNAL(readyRead()),SLOT(onResponseReceived()));
        connect(responseSocket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onResponseError(QAbstractSocket::SocketError)));
        connect(responseSocket,SIGNAL(aboutToClose()),SLOT(onResponseClose()));
        responseSocket->connectToHost(pipeData->getHeader("Host"),pipeData->getHeader("Port").toInt());
    }
}
void QPipe::onReqSocketReadFinished(){
    QObject *id = sender();
    if(id==0)return;
    QTcpSocket* socket = static_cast<QTcpSocket*>(id);
    if(!socket)return;
    //qDebug()<<"readfinished..";
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
/*
CONNECT clients4.google.com:443 HTTP/1.1
Host: clients4.google.com
Proxy-Connection: keep-alive
User-Agent: Chrome MAC 16.0.912.63 (113337)
*/
/*
GET http://m.hi.baidu.com/i/msg/listen?r=0.7547130892053246 HTTP/1.1
Host: m.hi.baidu.com
Proxy-Connection: keep-alive
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_6_8) AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.63 Safari/535.7
Accept: * / *
Referer: http://hi.baidu.com/huangh0z0/blog/item/1a97cd3198cf0890a8018ec2.html
Accept-Encoding: gzip,deflate,sdch
Accept-Language: zh-CN,zh;q=0.8
Accept-Charset: gb18030,utf-8;q=0.7,*;q=0.3
*/
    int headerPosition = headerString.indexOf(headerSplitter);
    if(headerPosition==-1)return;
    headerFound = true;
    qDebug()<<"headerPosition:"<<headerPosition<<" totalLength="<<(headerString.length())<<headerString;
    QStringList ll = headerString.split(headerSplitter);
    QString header = ll[0];
    QStringList headers = header.split(newLine);

    //parse http version request method path
    // example:GET / HTTP/1.1

    int majVer = 1;
    int minVer = 0;

    QString line = headers[0];
    QStringList lst = line.simplified().split(QLatin1String(" "));
    if (lst.count() > 0) {
        if (lst.count() > 1) {
            if (lst.count() > 2) {
                QString v = lst[2];
                if (v.length() >= 8 && v.left(5) == QLatin1String("HTTP/") &&
                    v[5].isDigit() && v[6] == QLatin1Char('.') && v[7].isDigit()) {
                    majVer = v[5].toLatin1() - '0';
                    minVer = v[7].toLatin1() - '0';
                }
                QString p="/";
                int n;

                qDebug()<<"lst[1]="<<lst[1];
                if(lst[1].indexOf("://")!=-1){
                    n = lst[1].split("://")[1].indexOf("/");
                    if(n!=-1 && n<lst[1].length()-1){
                        p = lst[1].split("://")[1].mid(n);
                    }
                }else{
                    n = lst[1].indexOf("/");
                    if(n!=-1 && n<lst[1].length()-1){
                        p = lst[1].mid(n);
                    }
                }
                QString method = lst[0];
                reqSig = method+" "+p+" "+lst[2];
                qDebug()<<"sig="<<reqSig;
            }
        }
    }

    int l = headers.size();
    int i=1;
    for(;i<l;++i){
        QString s = headers[i];
        int j = s.indexOf(": ");
        if(j!= -1 && j<s.length()){
            pipeData->setHeader(s.left(j),s.mid(j+2));
        }
    }
}


void QPipe::onResponseConnected(){
    //responseSocket->write(reqSig.toUtf8().append("\n"));
    //responseSocket->write(reqHeaderString.toUtf8());
    qDebug()<<"connected:"<<reqByteArray;
    responseSocket->write(reqByteArray);
    responseSocket->flush();
    reqByteArray.clear();
}

void QPipe::onResponseReceived(){
    QByteArray response = QByteArray(responseSocket->readAll());
    qDebug()<<"receive response:\n"<<response;
    requestSocket->write(response);
    requestSocket->flush();
}
void QPipe::onResponseError(QAbstractSocket::SocketError error){

}
void QPipe::onRequestSocketClose(){
    if(responseSocket && responseSocket->isOpen()){
        responseSocket->close();
    }
}
void QPipe::onResponseClose(){
    if(requestSocket->isOpen()){
        requestSocket->close();
    }
}

QPipe::~QPipe(){
    delete reqRawString;
    reqRawString = NULL;
    delete requestSocket;
    requestSocket = NULL;

    //need?
    reqByteArray.clear();
    reqByteArray = NULL;

    if(responseSocket!=0){
        delete responseSocket;
        responseSocket = NULL;
    }
}
