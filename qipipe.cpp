#include "qipipe.h"
#include <QStringList>
#include <QRegExp>
#include <QNetworkProxyFactory>
#include <QSslSocket>
#include <QThread>
#include <QApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QNetworkProxy>
#include <QTcpSocket>
#include "qiwinhttp.h"
#include <QByteArray>



QiPipe::QiPipe(int socketDescriptor):_socketDescriptor(socketDescriptor){

}


void QiPipe::run(){
    requestSocket = new QTcpSocket();
    requestSocket->setSocketDescriptor(_socketDescriptor);
    pipeData = QSharedPointer<PipeData>(new PipeData);
    pipeData->socketId = _socketDescriptor;
    qDebug()<<"Pipe run:"<<QThread::currentThreadId();
    manager = new QNetworkAccessManager();
    //requestSocket->moveToThread(QThread::currentThread());
    QEventLoop eventLoop;
    connect(requestSocket, SIGNAL(readyRead()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    this->onReqSocketReadReady();

}

void QiPipe::onReqSocketReadReady(){
    QByteArray reqByteArray;
    if(QApplication::instance()->thread() == QThread::currentThread()){
        qDebug()<<"onReqSocketReadReady in main thread";
    }else{
        qDebug()<<"onReqSocketReadReady no in main";
    }
    QTcpSocket* socket = requestSocket;
    Q_ASSERT(socket);
    //Proxy-Connection: keep-alive
    reqByteArray = socket->readAll();
    if(reqByteArray.size()==0){
        return;
        socket->close();
    }
    parseRequest(reqByteArray);
    //qDebug()<<"readReady header NOT found:\n"<<reqByteArray;


    if(requestInfo.headers.value("Host") == "127.0.0.1" && requestInfo.headers.value("Port")=="8888"){//避免死循环
        //emit(connected(pipeData));
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
        QUrl url(requestInfo.url);
        QNetworkRequest request(QUrl(requestInfo.url.toLatin1()));
        //manager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,"proxy.tencent.com",8080));
        QList<QNetworkProxy> proxyList = QiWinHttp::queryProxy(QNetworkProxyQuery(url));
        foreach(QNetworkProxy p, proxyList){
            manager->setProxy(p);
        }

        QMap<QByteArray,QByteArray>::Iterator i=requestInfo.headers.begin();
        for(;i!=requestInfo.headers.end();++i){
            request.setRawHeader(i.key(),i.value());
        }
        QNetworkReply *r;
        if(requestInfo.method.toLower() == "get"){
            r = manager->get(request);
        }else{
            r = manager->post(request,requestInfo.data);
        }
        qDebug()<<"requsted..";
        QEventLoop eventLoop;
        connect(manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
        eventLoop.exec();       //block until finish
        //connect(manager,SIGNAL(finished(QNetworkReply*)),SLOT(onResponseFinished(QNetworkReply*)));
        onResponseFinished(r);

    }
}


void QiPipe::parseRequest(const QByteArray requestString){
    //qDebug()<<"parseRequest"<<requestString;
    Q_ASSERT(requestString.size()>0);
    int i=0;
    QByteArray fragment;
    fragment.reserve(512);
    bool allHeadersFound=false;
    //qDebug()<<requestString;
    do {
        char c = requestString.at(i);
        fragment=requestString.left(i+1);
        i++;
        if (c == '\n') {
            // check for possible header endings. As per HTTP rfc,
            // the header endings will be marked by CRLFCRLF. But
            // we will allow CRLFCRLF, CRLFLF, LFLF

            if (fragment.endsWith("\r\n\r\n")
                || fragment.endsWith("\r\n\n")
                || fragment.endsWith("\n\n"))
                allHeadersFound = true;
            // there is another case: We have no headers. Then the fragment equals just the line ending
            if ((fragment.length() == 2 && fragment.endsWith("\r\n"))
                || (fragment.length() == 1 && fragment.endsWith("\n")))
                allHeadersFound = true;
        }
    } while (!allHeadersFound && i < requestString.count());
    if (allHeadersFound) {
        QByteArray line;
        line.reserve(80);
        int j=0;
        bool gotEnd=false;
        bool gotLine=false;
        bool isFirstLine=true;
        int lasti=0;
        do{
            line=fragment.mid(lasti,j+1);
            if(line.endsWith("\r\n\r\n")){
                gotEnd = true;
                gotLine = true;
            }else{
                if(line.endsWith("\r\n") ||
                        line.endsWith("\n")){
                    gotLine = true;
                }
            }
            line = line.trimmed();
            if(gotLine){
                //qDebug()<<"got line"<<line;
                if(isFirstLine){//GET http://x.x.x/x/x?sdfxsd HTTP/1.1
                    isFirstLine = false;
                    QList<QByteArray> l = line.split(' ');
                    //qDebug()<<l.count();
                    requestInfo.url = line.split(' ').at(1);
                    requestInfo.method = line.split(' ').at(0);
                    qDebug()<<"got sig~"<<requestInfo.url;
                }else{//Host: pagead2.googlesyndication.com

                    int tmp = line.indexOf(":");
                    if(tmp!=-1){
                        requestInfo.headers[line.left(tmp)] = line.mid(tmp+1).trimmed();
                        //qDebug()<<line.left(tmp)<<":"<<line.mid(tmp+1).trimmed();
                    }
                }
                //qDebug()<<line;
                gotLine = false;
                line.clear();
                lasti = j;
            }
            j++;
        }while(!gotEnd && j < fragment.length());
        //qDebug()<<requestInfo.url;
        fragment.clear(); // next fragment
        requestInfo.data = requestString.mid(i);
    }
}

void QiPipe::onResponseFinished(QNetworkReply* reply){
    if(QApplication::instance()->thread() == QThread::currentThread()){
        qDebug()<<"onResponseFinished in main thread";
    }else{
        qDebug()<<"onResponseFinished no in main";
    }
    requestSocket->write("HTTP/1.1 ");
    requestSocket->write(
                QString::number(
                    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                    ).toLatin1());
    QByteArray resone = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray();
    requestSocket->write(" ");
    requestSocket->write(resone);
    requestSocket->write("\r\n");
    QByteArray ba = reply->readAll();
    if(reply->url().host().contains("oa.com")){
        qDebug()<<"response="<<ba;
    }
    QList<QByteArray> headers = reply->rawHeaderList();
    foreach(QByteArray bah,headers){
        requestSocket->write(bah);
        requestSocket->write(": ");
        requestSocket->write(reply->rawHeader(bah));
        if(reply->url().host().contains("oa.com")){
            qDebug()<<"response="<<bah<<reply->rawHeader(bah);
        }
    }
    requestSocket->write("\r\n");
    requestSocket->write(ba);
    requestSocket->flush();
    requestSocket->close();
    qRegisterMetaType<Pipedata_const_ptr>("Pipedata_const_ptr");
    emit(completed(pipeData));
}

QiPipe::~QiPipe(){
    qDebug()<<"~QPipe";

    delete reqRawString;
    reqRawString = NULL;
    delete requestSocket;
    requestSocket = NULL;

    if(responseSocket!=0){
        delete responseSocket;
        responseSocket = NULL;
    }
}
void QiPipe::onReqSocketReadFinished(){}
void QiPipe::onRequestSocketError(){
    emit(error(pipeData));
}
void QiPipe::onRequestHostFound(){}
void QiPipe::onResponseConnected(){}
void QiPipe::onResponseReceived(){}
void QiPipe::onResponseError(QAbstractSocket::SocketError error){}
void QiPipe::onRequestSocketClose(){}
void QiPipe::onResponseClose(){}
void QiPipe::tearDown(){}


