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
#ifdef Q_OS_WIN
#include "qiwinhttp.h"
#endif
#include <QByteArray>
#include <qglobal.h>
#include "qiproxyserver.h"

static void isInMain(QString info){
    if(QThread::currentThread() == QApplication::instance()->thread()){
        qDebug()<<info<<" in main thread.";
    }else{
        qDebug()<<info<<" not main thread.";
    }
}

QiPipe::QiPipe(int socketDescriptor):_socketDescriptor(socketDescriptor){
}

QiPipe::~QiPipe(){
    qp->disconnect(this);
    qp->deleteLater();
    qDebug()<<"~QPipe in main:"<<((QThread::currentThread()==QApplication::instance()->thread())?"YES":"NO");
}

void QiPipe::run(){
    qp = new QiPipe_Private(_socketDescriptor);
    connect(qp,SIGNAL(connected(ConnectionData_ptr)),this,SIGNAL(connected(ConnectionData_ptr)));
    connect(qp,SIGNAL(finishSuccess(ConnectionData_ptr)),this,SIGNAL(completed(ConnectionData_ptr)));
    connect(qp,SIGNAL(finishedWithError(ConnectionData_ptr)),this,SIGNAL(error(ConnectionData_ptr)));
}
//===========QiPipe_Private
QiPipe_Private::QiPipe_Private(int descriptor):requestSocket(NULL),responseSocket(NULL){

    requestState = Initial;
    responseState = Initial;

    // setup request socket
    requestSocket = new QTcpSocket();
    connect(requestSocket,SIGNAL(readyRead()),this,SLOT(onRequestReadReady()));
    connect(requestSocket,SIGNAL(disconnected()),this,SLOT(onRequestClose()));
    connect(requestSocket,SIGNAL(aboutToClose()),this,SLOT(onRequestClose()));
    connect(requestSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onRequestError()));

    bool isSocketValid = requestSocket->setSocketDescriptor(descriptor);
    if(!isSocketValid){
        qWarning()<<"invalid socket descriptor!!";
        return;
    }
}
QiPipe_Private::~QiPipe_Private(){
    QMutexLocker locker(&mutex);
    if(requestSocket && requestSocket->isOpen()){
        requestSocket->blockSignals(true);
        requestSocket->abort();
        delete requestSocket;
    }
    if(responseSocket && responseSocket->isOpen()){
        responseSocket->blockSignals(true);
        responseSocket->abort();
        delete responseSocket;
    }
}


void QiPipe_Private::onRequestReadReady(){

    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);

    QByteArray newReqData = requestSocket->readAll();
    //qDebug()<<"onRequestReady:"<<pipeData->socketId<<" newContent:"<<newReqData;
    //update request buffer
    requestBuffer.append(newReqData);
    parseRequest(newReqData);
}

// 检查请求数据中是否有header，如果有header则检查请求包是否完整，并重置requestBuffer
void QiPipe_Private::parseRequest(const QByteArray &newContent){
    if(requestState != HeaderFound){// no header, parse one more time ( state =  Initial || PackageFound
        parseRequestHeader(newContent);//如果获取新header,则放入bufferConnectionArray
        if(requestState != HeaderFound){
            return;
        }
    }
    // parse body
    // is need to count request length?
    // 检查是否已获取所有数据
    QByteArray contentLenght = currentConnectionData->getRequestHeader("Content-Length");
    requestContentLength = contentLenght.toInt();
    if(requestContentLength == 0){
        // no body to send
        requestState = PackageFound;
    }else{
        // need body
        qDebug()<<"req content-length="<<requestContentLength<<" remain="<<requestBodyRemain;
        int bufferBodyLength = requestBuffer.length() - (requestHeaderSpliterSize + requestHeaderSpliterIndex);
        requestBodyRemain = requestContentLength - bufferBodyLength;
        if(requestBodyRemain <= 0){
            requestState = PackageFound;
        }
        //TODO?
    }
    if(responseState == Initial && requestState!= PackageFound){
        //为后续逻辑简化，第一个请求仅当收到一个完整的包才开始发送
        return;
    }

    if(responseState != Initial && responseState != Connecting){
        //同一个socket中发起n个请求的情况
        //条件是：当第二个同域请求发起时，有已经连接成功并返回数据的数据
        //所以这里可以简单处理
        if(currentConnectionData->getRequestHeader("Host") == "127.0.0.1" && currentConnectionData->getRequestHeader("Port")=="8889"){//避免死循环
            requestSocket->abort();
            emit(finishSuccess(currentConnectionData));
            return;
        }
        responseSocket->write(newContent);
        responseSocket->flush();
    }else if(responseState == Initial){
        if(!currentSendingConnectionData){
            currentSendingConnectionData = bufferConnectionArray.at(0);
            bufferConnectionArray.remove(0);
        }

        if(currentConnectionData->getRequestHeader("Host") == "127.0.0.1" && currentConnectionData->getRequestHeader("Port")=="8889"){//避免死循环
            //TODO
            QByteArray byteToWrite;
            QString s = "hello script<script src='a7.js'></a><script src='a6.js'></a><script src='a5.js'></a><script src='a4.js'></a><script src='a3.js'></a><script src='a2.js'></a>";
            int count = s.length();
            byteToWrite.append(QString("HTTP/1.1 200 OK\r\nServer: Qiddler\r\nContent-Type: text/html\r\nContent-Length: %1\r\n\r\n").arg(count));
            byteToWrite.append(s);
            requestSocket->write(byteToWrite);
            requestSocket->flush();
            requestSocket->abort();
            emit(finishSuccess(currentConnectionData));
            return;
        }else{
            responseState = Connecting;
            responseSocket = new QTcpSocket();
            connect(responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
            connect(responseSocket,SIGNAL(readyRead()),SLOT(onResponseReadReady()));
            connect(responseSocket,SIGNAL(disconnected()),SLOT(onResponseClose()));
            connect(responseSocket,SIGNAL(aboutToClose()),SLOT(onResponseClose()));
            connect(responseSocket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onResponseError(QAbstractSocket::SocketError)));

#ifdef Q_OS_WIN
            // TODO add mac pac support
            QList<QNetworkProxy> proxylist = QiWinHttp::queryProxy(QNetworkProxyQuery(QUrl(currentSendingConnectionData->fullUrl)));
            for(int i=0,l=proxylist.length();i<l;++i){
                QNetworkProxy p = proxylist.at(i);
                responseSocket->setProxy(p);
                qDebug()<<"proxy="<<p.hostName()<<p.port();
            }
#endif

            qDebug()<<"CONNECT TO "<<currentConnectionData->getRequestHeader("Host")<<" "<<currentConnectionData->getRequestHeader("Port");
            //responseSocket->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,"127.0.0.1",8888));
            responseSocket->connectToHost(currentConnectionData->getRequestHeader("Host"),currentConnectionData->getRequestHeader("Port").toInt());

        }
    }
}


void QiPipe_Private::parseRequestHeader(const QByteArray &newContent){
    Q_UNUSED(newContent);

    QByteArray header;
    int indexOfRN = requestBuffer.indexOf(QByteArray("\r\n\r\n"));
    int indexOfN = requestBuffer.indexOf(QByteArray("\n\n"));
    if(indexOfRN!=-1){
        requestHeaderSpliterSize = 4;
        requestHeaderSpliterIndex = indexOfRN;
    }else if(indexOfN!=-1){
        requestHeaderSpliterSize = 2;
        requestHeaderSpliterIndex = indexOfN;
    }else{
        return;
    }
    header = requestBuffer.left(requestHeaderSpliterIndex);

    //cut header part from buffer
    if(requestBuffer.size()>=requestHeaderSpliterIndex+requestHeaderSpliterSize){
        requestBuffer.remove(0,requestHeaderSpliterIndex+requestHeaderSpliterSize);
    }else{
        // seems will never enter here
        Q_ASSERT_X(false,"parseRequestHeader","invalid header length?");
        requestBuffer.clear();
    }

    //TODO use setRequestState
    requestState = HeaderFound;

    // create new QiConnectionData (will sharedpoiter do clear for old pointer? anwser:YES)

    ConnectionData_ptr newConnectionData = ConnectionData_ptr(new QiConnectionData());
    newConnectionData->setRequestHeader(header);
    newConnectionData->id = QiProxyServer::nextConnectionId();
    currentConnectionData = newConnectionData;
    bufferConnectionArray.append(newConnectionData);

    emit connected(newConnectionData);
}


void QiPipe_Private::onRequestError(){
    emit finishedWithError(currentConnectionData);
}


void QiPipe_Private::onResponseConnected(){

    // need?
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker)

    responseState = Connected;
    // save the server ip address
    currentSendingConnectionData->serverIP = responseSocket->peerAddress().toString();
    // emit connect signal
    //qDebug()<<"send this:\n"<<responseSocket->peerName()<<responseSocket->peerPort()<<connectionData->requestRawDataToSend;
    while(currentSendingConnectionData->requestRawDataToSend.size()>0){
        qint64 n = responseSocket->write(currentSendingConnectionData->requestRawDataToSend);
        responseSocket->flush();
        if(n==-1){
            qDebug()<<"write error !!!!!";
            break;
        }
        currentSendingConnectionData->requestRawDataToSend.remove(0,n);
    }
}


void QiPipe_Private::onResponseReadReady(){

    QMutexLocker locker(&mutex);
    QByteArray ba = responseSocket->readAll();
    requestSocket->write(ba);
    requestSocket->flush();

    //qDebug()<<"========response========***"<<responseSocket->peerName()<<connectionData->path;
    //qDebug()<<connectionData->requestRawDataToSend;
    //qDebug()<<ba;
    //qDebug()<<"***========response========"<<responseSocket->peerName()<<connectionData->path;

    responseBuffer.append(ba);
    if(parseResponse(ba)){
        // package got end
        if(bufferConnectionArray.size()>0){
            currentSendingConnectionData = bufferConnectionArray.at(0);
            bufferConnectionArray.remove(0);
        }
        emit finishSuccess(currentConnectionData);
    }
}


void QiPipe_Private::onResponseError(QAbstractSocket::SocketError e){
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    emit(finishedWithError(currentConnectionData));
}


void QiPipe_Private::onRequestClose(){
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    emit finishSuccess(currentConnectionData);
}


void QiPipe_Private::onResponseClose(){
    qDebug()<<"response close";
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    requestSocket->disconnect();
    requestSocket->close();
    emit finishSuccess(currentConnectionData);
}



bool QiPipe_Private::parseResponse(const QByteArray &newContent){
    // if got response end return true
    if(responseState != HeaderFound){
        parseResponseHeader(newContent);
    }

    if(responseState != HeaderFound){//check if got end
        return false;
    }
    if(parseResponseBody(newContent)){
        return true;
    }
    return false;
}

void QiPipe_Private::parseResponseHeader(const QByteArray &newContent){
    Q_UNUSED(newContent)
    responseHeaderSpliterIndex = responseBuffer.indexOf(QByteArray("\r\n\r\n"));
    if(responseHeaderSpliterIndex!=-1){
        responseHeaderSpliterSize = 4;
    }else{
        responseHeaderSpliterIndex = responseBuffer.indexOf(QByteArray("\n\n"));
        if(responseHeaderSpliterIndex != -1){
            responseHeaderSpliterSize = 2;
        }else{
            return;
        }
    }

    // got header : cut buffer & set state to HeaderFound
    responseState = HeaderFound;
    currentConnectionData->setResponseHeader(responseBuffer.left(responseHeaderSpliterIndex));
    responseBuffer.remove(0,responseHeaderSpliterIndex+responseHeaderSpliterSize);

    // 需要在pipe这里保存一份吗？
    isResponseChunked = currentConnectionData->getResponseHeader("Transfer-Encoding").toLower() == "chunked";
    responseContentLength = currentConnectionData->getResponseHeader("Content-Length").toInt();
    responseComressType = currentConnectionData->getResponseHeader("Content-Encoding");
}

bool QiPipe_Private::parseResponseBody(QByteArray newContent){
    /*
    if(currentConnectionData->returnCode == 302
            || currentConnectionData->returnCode == 301
            || currentConnectionData->returnCode == 307
            || currentConnectionData->returnCode == 204){//todo

        responseState = PackageFound;
        return true;
    }
    */
    //根据http协议，需由header及body共同判断请求是否结束。
    responseBuffer.clear();
    if(currentSendingConnectionData->appendResponseBody(QByteArray(newContent))){
        responseState = PackageFound;
        return true;
    }
    return false;

}
void QiPipe_Private::tearDown(){

}

void QiPipe_Private::finishConnectionSuccess(){

}

void QiPipe_Private::finishConnectionWithError(int errno){

}
