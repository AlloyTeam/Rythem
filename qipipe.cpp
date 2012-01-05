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


QiPipe::QiPipe(int socketDescriptor):_socketDescriptor(socketDescriptor){

}

QiPipe::~QiPipe(){
    qDebug()<<"~QPipe in main:"<<((QThread::currentThread()==QApplication::instance()->thread())?"YES":"NO");
}

void QiPipe::run(){
    qp = new QiPipe_Private(_socketDescriptor);
    connect(qp,SIGNAL(connected(PipeData_ptr)),this,SIGNAL(connected(PipeData_ptr)));
    connect(qp,SIGNAL(completed(PipeData_ptr)),this,SIGNAL(completed(PipeData_ptr)));
    connect(qp,SIGNAL(error(PipeData_ptr)),this,SIGNAL(error(PipeData_ptr)));

    qDebug()<<"Pipe run:"<<QThread::currentThreadId();

    QEventLoop eventLoop;//use eventloop keep the thread running
    connect(this, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    qp->deleteLater();
}
//===========QiPipe_Private
QiPipe_Private::QiPipe_Private(int descriptor):responseSocket(NULL){
    responseHeaderFound = false;
    requestHeaderFound = false;
    requestSocket = new QTcpSocket();
    bool isSocketValid = requestSocket->setSocketDescriptor(descriptor);
    if(!isSocketValid){
        qWarning()<<"invalid socket descriptor!!";
        emit(finished());
        return;
    }
    pipeData = QSharedPointer<PipeData>(new PipeData);
    pipeData->socketId = descriptor;
    connect(requestSocket,SIGNAL(readyRead()),this,SLOT(onRequestReadReady()));
    connect(requestSocket,SIGNAL(disconnected()),this,SLOT(onRequestClose()));
    connect(requestSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onRequestError()));
}



void QiPipe_Private::onRequestReadReady(){

    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);

    QByteArray newReqData = requestSocket->readAll();

    //update raw data
    requestRawData.append(newReqData);
    parseRequest(newReqData);
}
void QiPipe_Private::parseRequest(const QByteArray &newContent){
    if(!requestHeaderFound){
        parseRequestHeader(newContent);
    }else{
        //应该不会来到这一步吧..
        Q_ASSERT(true);
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
    pipeData->requestRawDataToSend.append(requestRawData.mid(requestHeaderSpliterIndex));

    QString reqSig = QString("%1 %2 %3")
            .arg(pipeData->requestMethod)
            .arg(pipeData->path)
            .arg(pipeData->protocol);

    qDebug()<<"host="<<pipeData->getRequestHeader("Host")<<pipeData->getRequestHeader("Port")<<reqSig;

    if(pipeData->getRequestHeader("Host") == "127.0.0.1" && pipeData->getRequestHeader("Port")=="8889"){//避免死循环
        emit(connected(pipeData));
        QByteArray byteToWrite;
        QString s = "hello Qiddler";
        int count = s.length();
        byteToWrite.append(QString("HTTP/1.1 200 OK\r\nServer: Qiddler\r\nContent-Type: text/html\r\nContent-Length: %1\r\n\r\n").arg(count));
        byteToWrite.append(s);
        requestSocket->write(byteToWrite);
        requestSocket->flush();
        requestSocket->close();
        emit(finished());
        return;
    }else{
        responseSocket = new QTcpSocket();
        connect(responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
        connect(responseSocket,SIGNAL(readyRead()),SLOT(onResponseReadReady()));
        connect(responseSocket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onResponseError(QAbstractSocket::SocketError)));
        connect(responseSocket,SIGNAL(aboutToClose()),SLOT(onResponseClose()));

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
    emit(connected(pipeData));
    responseSocket->write(pipeData->requestRawDataToSend);

}
void QiPipe_Private::onResponseReadReady(){
    QByteArray ba = responseSocket->readAll();
    responseRawData.append(ba);

    //write back to request
    //TODO check if the socket opening..
    requestSocket->write(ba);
    requestSocket->flush();

    if(parseResponse(ba)){
        responseSocket->close();
        requestSocket->close();
        //todo emit
    }
}

void QiPipe_Private::onResponseError(QAbstractSocket::SocketError error){
}
void QiPipe_Private::onRequestClose(){
}
void QiPipe_Private::onResponseClose(){
}

bool QiPipe_Private::parseResponse(const QByteArray &newContent){
    if(!responseHeaderFound){
        parseResponseHeader(newContent);
        if(responseHeaderFound){//check if got end

        }
    }else{
        parseResponseBody(newContent);
    }
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


