#include "qpipe.h"
#include <QStringList>
#include <QRegExp>
#include <QNetworkProxyFactory>
#include <QSslSocket>
#include <QThread>
#include <QApplication>


const static QRegExp headerSplitter("\r?\n\r?\n");
const static QRegExp newLine("\r?\n");
QPipe::QPipe(QTcpSocket *socket) :
    QThread(socket),
    requestSocket(socket),
    headerFound(false),
    responseSocket(NULL),
    isError(false){



    reqRawString = new QString();
    qDebug()<<"socket?"<<(socket?"has socket":"oh no..");
    pipeData = QSharedPointer<PipeData>(new PipeData(socket->socketDescriptor()));
    qDebug()<<"Pipe Ctor:"<<QThread::currentThreadId();

}
void QPipe::run(){
    qDebug()<<"Pipe run:"<<QThread::currentThreadId();
    if(QApplication::instance()->thread() == QThread::currentThread()){
        qDebug()<<"in main thread";
    }
    connect(requestSocket,SIGNAL(readyRead()),SLOT(onReqSocketReadReady()));
    connect(requestSocket,SIGNAL(readChannelFinished()),SLOT(onReqSocketReadFinished()));
    //connect(socket,SIGNAL(hostFound()),SLOT(onRequestHostFound()));??
    connect(requestSocket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onRequestSocketError()));
    connect(requestSocket,SIGNAL(aboutToClose()),SLOT(onRequestSocketClose()));
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
    //Proxy-Connection: keep-alive
    reqByteArray.append(QByteArray(socket->readAll()));
    //reqByteArray = reqByteArray.replace("\r\n","\n");
    reqHeaderString = reqByteArray.mid(reqByteArray.indexOf("\r\n"));
    qDebug()<<"readReady header NOT found:\n"<<QString(reqByteArray);
    //reqByteArray = QByteArray(socket->readAll());
    //qDebug()<<QString(reqByteArray);
    parseHeader(QString(reqByteArray));
    //TODO..
    pipeData->host = pipeData->getHeader("Host");
    pipeData->port = pipeData->getHeader("Port").toInt();

    if(!isHttpsConnect){
        reqByteArray.clear();
        reqByteArray.append(reqSig);
        reqByteArray.append(reqHeaderString);
    }
    qDebug()<<"host="<<pipeData->getHeader("Host")<<pipeData->getHeader("Port")<<reqSig;
    if(pipeData->getHeader("Host") == "127.0.0.1" && pipeData->getHeader("Port")=="8080"){//避免死循环
        emit(connected(pipeData));
        QByteArray byteToWrite;
        QString s = "hello Qiddler";
        int count = s.length();
        byteToWrite.append(QString("HTTP/1.1 200 OK\r\nServer: Qiddler\r\nContent-Type: text/html\r\nContent-Length: %1\r\n\r\n").arg(count));
        byteToWrite.append(s);
        requestSocket->write(byteToWrite);
        requestSocket->flush();
        requestSocket->close();
        tearDown();
        return;
    }else{
        if(!isHttpsConnect){
            responseSocket = new QTcpSocket();
            connect(responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
            connect(responseSocket,SIGNAL(readyRead()),SLOT(onResponseReceived()));
            connect(responseSocket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onResponseError(QAbstractSocket::SocketError)));
            connect(responseSocket,SIGNAL(aboutToClose()),SLOT(onResponseClose()));
            responseSocket->connectToHost(pipeData->getHeader("Host"),pipeData->getHeader("Port").toInt());
        }else{
            QByteArray byteToWrite;
            QString s = "HTTP/1.1 200 Connection established\r\nConnection: keep-alive\r\n\r\n";
            byteToWrite.append(s);
            requestSocket->write(byteToWrite);
            requestSocket->flush();
            //requestSocket->close();
            //return;
            //responseSocketSSL = new QSslSocket();
            //connect(responseSocketSSL,SIGNAL(connected()),SLOT(onResponseConnected()));
            //connect(responseSocketSSL,SIGNAL(readyRead()),SLOT(onResponseReceived()));
            //connect(responseSocketSSL,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onResponseError(QAbstractSocket::SocketError)));
            //connect(responseSocketSSL,SIGNAL(aboutToClose()),SLOT(onResponseClose()));
            //responseSocketSSL->connectToHostEncrypted(pipeData->getHeader("Host"),443);
        }
        //connect(responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
        //connect(responseSocket,SIGNAL(readyRead()),SLOT(onResponseReceived()));
        //connect(responseSocket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onResponseError(QAbstractSocket::SocketError)));
        //connect(responseSocket,SIGNAL(aboutToClose()),SLOT(onResponseClose()));
        //if(!isHttpsConnect){
        //    responseSocket->connectToHost(pipeData->getHeader("Host"),pipeData->getHeader("Port").toInt());
        //}else{
        //    responseSocket->connectToHost(pipeData->getHeader("Host"),443);
        //}
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
    emit(error(pipeData));
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
    //qDebug()<<"headerPosition:"<<headerPosition<<" totalLength="<<(headerString.length())<<headerString;
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
                if(method == "CONNECT"){
                    qDebug()<<"is HTTPS";
                    isHttpsConnect = true;
                }
                pipeData->URL = p;
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
    if(isHttpsConnect){
        responseSocketSSL->write(reqByteArray);
        responseSocketSSL->flush();
    }else{
        responseSocket->write(reqByteArray);
        responseSocket->flush();
    }
    reqByteArray.clear();
    emit(connected(pipeData));
}

void QPipe::onResponseReceived(){
    QByteArray response;
    if(isHttpsConnect){
        response = QByteArray(responseSocketSSL->readAll());
    }else{
        response = QByteArray(responseSocket->readAll());
    }
    qDebug()<<"receive response:\n"<<response;
    requestSocket->write(response);
    requestSocket->flush();
}
void QPipe::onResponseError(QAbstractSocket::SocketError error){
    qDebug()<<"responseError:"<<error;
    tearDown();
}
void QPipe::onRequestSocketClose(){
    tearDown();
}
void QPipe::onResponseClose(){
    tearDown();
}
void QPipe::tearDown(){
    mutex.lock();
    if(requestSocket && requestSocket->isOpen()){
        disconnect(requestSocket,SIGNAL(aboutToClose()),this,SLOT(onRequestSocketClose()));
        requestSocket->close();
        requestSocket = NULL;
    }
    if(responseSocket && responseSocket->isOpen()){
        disconnect(responseSocket,SIGNAL(aboutToClose()),this,SLOT(onResponseClose()));
        responseSocket->close();
        responseSocket = NULL;
    }
    mutex.unlock();
    if(isError){
        emit(error(pipeData));
    }else{
        emit(completed(pipeData));
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
