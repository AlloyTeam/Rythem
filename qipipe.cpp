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
#include <qglobal.h>

static void isInMain(QString info){
    if(QThread::currentThread() == QApplication::instance()->thread()){
        qDebug()<<info<<" in main thread.";
    }else{
        qDebug()<<info<<" not main thread.";
    }
}

QiPipe::QiPipe(int socketDescriptor):_socketDescriptor(socketDescriptor){
    stoped = false;
    qDebug()<<"socketDescriptor:"<<socketDescriptor;
}

QiPipe::~QiPipe(){
    emit pipeFinished();
    qDebug()<<"~QPipe wait for QiPipe::run exit";
    stoped = true;
    wait(1000);
    qDebug()<<"~QPipe in main:"<<((QThread::currentThread()==QApplication::instance()->thread())?"YES":"NO");
}
void QiPipe::onPipeFinished(){
    qDebug()<<"onPipeFinished..";
}
class MyEventLoop:public QEventLoop{
public slots:
    void quit(){
        qDebug()<<"quite invoked..";
        QEventLoop::quit();
    }
};

void QiPipe::run(){
    qp = new QiPipe_Private(_socketDescriptor);
    connect(qp,SIGNAL(connected(ConnectionData_ptr)),this,SIGNAL(connected(ConnectionData_ptr)));
    connect(qp,SIGNAL(completed(ConnectionData_ptr)),this,SIGNAL(completed(ConnectionData_ptr)));
    connect(qp,SIGNAL(error(ConnectionData_ptr)),this,SIGNAL(error(ConnectionData_ptr)));

    connect(this,SIGNAL(pipeFinished()),qp,SIGNAL(finished()));

    //qDebug()<<"Pipe run:"<<QThread::currentThreadId();

    //TODO WARNING:  QThread: Destroyed while thread is still running
    connect(this, SIGNAL(error(PipeData_ptr)), this->currentThread(), SLOT(quit()));
    connect(this, SIGNAL(completed(PipeData_ptr)),this->currentThread(), SLOT(quit()));
    connect(this, SIGNAL(pipeFinished()), this, SLOT(quit()));
    connect(this, SIGNAL(pipeFinished()),SLOT(onPipeFinished()));
    //while(!stoped){
    //    msleep(1);
        //::Sleep(200);// win api..
    //}
    exec();
    qp->deleteLater();
    qDebug()<<"exiting QiPipe run";
}
//===========QiPipe_Private
QiPipe_Private::QiPipe_Private(int descriptor):requestSocket(NULL),responseSocket(NULL){
    responseHeaderFound = false;
    requestHeaderFound = false;
    requestSocket = new QTcpSocket();
    //isInMain("QiPipe_Private");
    pipeData = QSharedPointer<QiConnectionData>(new QiConnectionData);
    pipeData->socketId = descriptor;

    connect(requestSocket,SIGNAL(readyRead()),this,SLOT(onRequestReadReady()));
    connect(requestSocket,SIGNAL(disconnected()),this,SLOT(onRequestClose()));
    connect(requestSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onRequestError()));

    bool isSocketValid = requestSocket->setSocketDescriptor(descriptor);
    if(!isSocketValid){
        qWarning()<<"invalid socket descriptor!!";
        return;
    }else{
        //qDebug()<<"is validate socket"<<requestSocket->state()<<requestSocket->readAll();
    }
}
QiPipe_Private::~QiPipe_Private(){
    QMutexLocker locker(&mutex);
    if(requestSocket && requestSocket->isOpen()){
        requestSocket->blockSignals(true);
        requestSocket->abort();
    }
    if(responseSocket && responseSocket->isOpen()){
        responseSocket->blockSignals(true);
        responseSocket->abort();
    }

    delete requestSocket;
    delete responseSocket;
}


void QiPipe_Private::onRequestReadReady(){

    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);

    //isInMain("onRequestReadReady");

    QByteArray newReqData = requestSocket->readAll();
    qDebug()<<"onRequestReady:"<<pipeData->socketId<<" newContent:"<<newReqData;
    //update raw data
    requestRawData.append(newReqData);
    parseRequest(newReqData);
}
void QiPipe_Private::parseRequest(const QByteArray &newContent){
    if(!requestHeaderFound){
        parseRequestHeader(newContent);
        emit(connected(pipeData));
    }else{
        //qDebug()<<"browser reuse socket:\n+++----\n"<<newContent<<"\n---\n"<<requestRawData<<"\n----+++";

        if(responseSocket){
            if(responseSocket->isOpen()){
                responseSocket->write(newContent);
                responseSocket->flush();
            }
        }else{
            //should no be an else..
        }
    }
}
void QiPipe_Private::parseRequestHeader(const QByteArray &newContent){
    Q_UNUSED(newContent);

    QByteArray header;
    int indexOfRN = requestRawData.indexOf(QByteArray("\r\n\r\n"));
    int indexOfN = requestRawData.indexOf(QByteArray("\n\n"));
    if(indexOfRN!=-1){
        requestHeaderSpliterSize = 4;
        requestHeaderSpliterIndex = indexOfRN;
        header = requestRawData.left(indexOfRN);
    }else if(indexOfN!=-1){
        requestHeaderSpliterSize = 2;
        requestHeaderSpliterIndex = indexOfN;
        header = requestRawData.left(indexOfN);
    }else{
        return;
    }
    requestHeaderFound = true;
    pipeData->setRequestHeader(header);

    //TODO
    pipeData->requestRawDataToSend.append(requestRawData.mid(requestHeaderSpliterIndex+requestHeaderSpliterSize));

    /* code below cause bug when arg has %n
    QString reqSig = QString("%1 %2 %3")
            .arg(pipeData->requestMethod)
            .arg(pipeData->path)
            .arg(pipeData->protocol);
    */
    QString reqSig = pipeData->requestMethod+" "+pipeData->path+" "+pipeData->protocol;

    //qDebug()<<"host="<<pipeData->getRequestHeader("Host")<<pipeData->getRequestHeader("Port")<<reqSig;
    pipeData->host = pipeData->getRequestHeader("Host");

    if(pipeData->getRequestHeader("Host") == "127.0.0.1" && pipeData->getRequestHeader("Port")=="8889"){//避免死循环
        emit(connected(pipeData));
        QByteArray byteToWrite;
        QString s = "hello Qiddler";
        int count = s.length();
        byteToWrite.append(QString("HTTP/1.1 200 OK\r\nServer: Qiddler\r\nContent-Type: text/html\r\nContent-Length: %1\r\n\r\n").arg(count));
        byteToWrite.append(s);
        requestSocket->write(byteToWrite);
        requestSocket->flush();
        requestSocket->abort();
        emit(completed(pipeData));
        emit(finished());
        return;
    }else{
        responseSocket = new QTcpSocket();
        connect(responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
        connect(responseSocket,SIGNAL(readyRead()),SLOT(onResponseReadReady()));
        connect(responseSocket,SIGNAL(disconnected()),SLOT(onResponseClose()));
        connect(responseSocket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onResponseError(QAbstractSocket::SocketError)));
        connect(responseSocket,SIGNAL(aboutToClose()),SLOT(onResponseClose()));

        QList<QNetworkProxy> proxylist = QiWinHttp::queryProxy(QNetworkProxyQuery(QUrl(pipeData->fullUrl)));
        for(int i=0,l=proxylist.length();i<l;++i){
            QNetworkProxy p = proxylist.at(i);
            responseSocket->setProxy(p);
            qDebug()<<"proxy="<<p.hostName()<<p.port();
        }
        //responseSocket->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,"127.0.0.1",8888));
        responseSocket->connectToHost(pipeData->getRequestHeader("Host"),pipeData->getRequestHeader("Port").toInt());

    }
}

void QiPipe_Private::onRequestError(){
    emit(error(pipeData));
}
void QiPipe_Private::onResponseConnected(){
    // save the server ip address
    pipeData->serverIP = responseSocket->peerAddress().toString();
    // emit connect signal
    //qDebug()<<"send this:\n"<<responseSocket->peerName()<<responseSocket->peerPort()<<pipeData->requestRawDataToSend;
    responseSocket->write(pipeData->requestRawDataToSend);

}
void QiPipe_Private::onResponseReadReady(){

    QByteArray ba = responseSocket->readAll();
    responseRawData.append(ba);

    QMutexLocker locker(&mutex);
    //write back to request
    //TODO check if the socket opening..
    requestSocket->write(ba);
    requestSocket->flush();
    /*
    qDebug()<<"========response========***"<<responseSocket->peerName();
    qDebug()<<pipeData->requestRawDataToSend;
    qDebug()<<ba;
    qDebug()<<"***========response========"<<responseSocket->peerName();
    */
    if(parseResponse(ba)){
        responseSocket->abort();
        requestSocket->abort();
        //todo emit
    }
    locker.unlock();
}

void QiPipe_Private::onResponseError(QAbstractSocket::SocketError e){
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    emit(error(pipeData));
    emit finished();
}
void QiPipe_Private::onRequestClose(){
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    emit error(pipeData);
    emit finished();
}
void QiPipe_Private::onResponseClose(){
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    emit error(pipeData);
    emit finished();
}

bool QiPipe_Private::parseResponse(const QByteArray &newContent){
    if(!responseHeaderFound){
        parseResponseHeader(newContent);
        if(responseHeaderFound){//check if got end

        }
    }else{
        parseResponseBody(newContent);
    }
    return false;
}

void QiPipe_Private::parseResponseHeader(const QByteArray &newContent){
    Q_UNUSED(newContent);
    responseHeaderFound = true;
    responseHeaderSpliterInex = responseRawData.indexOf(QByteArray("\r\n\r\n"));
    if(responseHeaderSpliterInex!=-1){
        responseHeaderSpliterSize = 4;
        pipeData->setResponseHeader(responseRawData.left(responseHeaderSpliterInex));
    }else{
        responseHeaderSpliterInex = responseRawData.indexOf(QByteArray("\r\n\r\n"));
        if(responseHeaderSpliterInex != -1){
            responseHeaderSpliterSize = 2;
            pipeData->setResponseHeader(responseRawData.left(responseHeaderSpliterInex));
        }else{
            responseHeaderSpliterInex =-1;
            responseHeaderFound = false;
        }
    }
}

void QiPipe_Private::parseResponseBody(const QByteArray &body){
    //根据http协议，需由header及body共同判断请求是否结束。
}
void QiPipe_Private::tearDown(){
}


