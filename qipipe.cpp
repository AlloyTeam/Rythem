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
}

QiPipe::~QiPipe(){
    qp->disconnect(this);
    qp->deleteLater();
    qDebug()<<"~QPipe in main:"<<((QThread::currentThread()==QApplication::instance()->thread())?"YES":"NO");
}

void QiPipe::run(){
    qp = new QiPipe_Private(_socketDescriptor);
    connect(qp,SIGNAL(connected(ConnectionData_ptr)),this,SIGNAL(connected(ConnectionData_ptr)));
    connect(qp,SIGNAL(completed(ConnectionData_ptr)),this,SIGNAL(completed(ConnectionData_ptr)));
    connect(qp,SIGNAL(error(ConnectionData_ptr)),this,SIGNAL(error(ConnectionData_ptr)));
}
//===========QiPipe_Private
QiPipe_Private::QiPipe_Private(int descriptor):requestSocket(NULL),responseSocket(NULL){

    requestState = Initial;
    responseState = Initial;

    // setup request socket
    requestSocket = new QTcpSocket();

    connectionData = QSharedPointer<QiConnectionData>(new QiConnectionData);
    //connectionArray.push_back(connectionData);
    connectionData->socketId = descriptor;

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

    //isInMain("onRequestReadReady");

    QByteArray newReqData = requestSocket->readAll();
    //qDebug()<<"onRequestReady:"<<pipeData->socketId<<" newContent:"<<newReqData;
    //update raw data?
    requestRawData.append(newReqData);
    parseRequest(newReqData);
}
void QiPipe_Private::parseRequest(const QByteArray &newContent){
    if(requestState != HeaderFound){// no header parse one more time
        parseRequestHeader(newContent);
    }
    if(requestState != HeaderFound){// stile no request header return
        return;
    }else{// check if go whole request package
        QByteArray contentLenght = connectionData->getRequestHeader("Content-Length");
        requestContentLength = contentLenght.toInt();
        if(requestContentLength == 0){
            // no body to send
            requestState = PackageFound;
        }else{
            // need body
            qDebug()<<"length="<<requestContentLength<<" ba="<<contentLenght;
            int bufferBodyLength = requestRawData.length() - (requestHeaderSpliterSize + requestHeaderSpliterIndex);
            requestBodyRemain = requestContentLength - bufferBodyLength;
            if(requestBodyRemain <= 0){
                requestState = PackageFound;
            }
        }
    }
    // parse body
    // is need to count request length?
    if(responseState == Connected){
        connectionData->requestRawDataToSend.append(newContent);
        responseSocket->write(connectionData->requestRawDataToSend);
        responseSocket->flush();
    }else{
        responseState = Initial;
        responseRawData.clear();
        connectionData->requestRawDataToSend.append(requestRawData.mid(requestHeaderSpliterIndex+requestHeaderSpliterSize));
        //QString reqSig = connectionData->requestMethod+" "+connectionData->path+" "+connectionData->protocol;
        connectionData->host = connectionData->getRequestHeader("Host");
        if(connectionData->getRequestHeader("Host") == "127.0.0.1" && connectionData->getRequestHeader("Port")=="8889"){//避免死循环
            //TODO
            QByteArray byteToWrite;
            QString s = "hello Qiddler";
            int count = s.length();
            byteToWrite.append(QString("HTTP/1.1 200 OK\r\nServer: Qiddler\r\nContent-Type: text/html\r\nContent-Length: %1\r\n\r\n").arg(count));
            byteToWrite.append(s);
            requestSocket->write(byteToWrite);
            requestSocket->flush();
            requestSocket->abort();
            emit(completed(connectionData));
            return;
        }else{
            responseSocket = new QTcpSocket();
            connect(responseSocket,SIGNAL(connected()),SLOT(onResponseConnected()));
            connect(responseSocket,SIGNAL(readyRead()),SLOT(onResponseReadReady()));
            connect(responseSocket,SIGNAL(disconnected()),SLOT(onResponseClose()));
            connect(responseSocket,SIGNAL(aboutToClose()),SLOT(onResponseClose()));
            connect(responseSocket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onResponseError(QAbstractSocket::SocketError)));

#ifdef Q_OS_WIN
            // TODO add mac pac support
            QList<QNetworkProxy> proxylist = QiWinHttp::queryProxy(QNetworkProxyQuery(QUrl(connectionData->fullUrl)));
            for(int i=0,l=proxylist.length();i<l;++i){
                QNetworkProxy p = proxylist.at(i);
                responseSocket->setProxy(p);
                qDebug()<<"proxy="<<p.hostName()<<p.port();
            }
#endif

            qDebug()<<"CONNECT TO "<<connectionData->getRequestHeader("Host")<<" "<<connectionData->getRequestHeader("Port");
            //responseSocket->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,"127.0.0.1",8888));
            responseSocket->connectToHost(connectionData->getRequestHeader("Host"),connectionData->getRequestHeader("Port").toInt());

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

    //TODO use setRequestState
    requestState = HeaderFound;

    // create new QiConnectionData (will sharedpoiter do clear for old pointer? anwser:YES)
    connectionData = QSharedPointer<QiConnectionData>(new QiConnectionData());
    connectionData->setRequestHeader(header);
    emit connected(connectionData);
}

void QiPipe_Private::onRequestError(){
    emit(error(connectionData));
}
void QiPipe_Private::onResponseConnected(){

    // need?
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker)

    responseState = Connected;
    // save the server ip address
    connectionData->serverIP = responseSocket->peerAddress().toString();
    // emit connect signal
    qDebug()<<"send this:\n"<<responseSocket->peerName()<<responseSocket->peerPort()<<connectionData->requestRawDataToSend;
    qint64 n = responseSocket->write(connectionData->requestRawDataToSend);
    connectionData->requestRawDataToSend.remove(0,n);
}
void QiPipe_Private::onResponseReadReady(){

    QByteArray ba = responseSocket->readAll();
    responseRawData.append(ba);
    if(responseState == Connected){
        responseState = BodyParsing;
    }
    qDebug()<<ba;
    QMutexLocker locker(&mutex);
    //write back to request
    //TODO check if the socket opening..
    requestSocket->write(ba);
    requestSocket->flush();

    qDebug()<<"========response========***"<<responseSocket->peerName();
    qDebug()<<connectionData->requestRawDataToSend;
    qDebug()<<ba;
    qDebug()<<"***========response========"<<responseSocket->peerName();

    if(parseResponse(ba)){
        // package got end
        responseState = Initial;
        emit completed(connectionData);

    }
    locker.unlock();
}

void QiPipe_Private::onResponseError(QAbstractSocket::SocketError e){
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    emit(error(connectionData));
    emit finished();
}
void QiPipe_Private::onRequestClose(){
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    emit error(connectionData);
    emit finished();
}
void QiPipe_Private::onResponseClose(){
    qDebug()<<"response close";
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    requestSocket->disconnect();
    requestSocket->close();
    emit error(connectionData);
    emit finished();
}

bool QiPipe_Private::parseResponse(const QByteArray &newContent){
    // if got response end return true
    if(responseState != HeaderFound){
        parseResponseHeader(newContent);
    }

    if(responseState != HeaderFound){//check if got end
        return false;
    }
    return parseResponseBody(newContent);
}

void QiPipe_Private::parseResponseHeader(const QByteArray &newContent){
    Q_UNUSED(newContent)
    responseHeaderSpliterInex = responseRawData.indexOf(QByteArray("\r\n\r\n"));
    if(responseHeaderSpliterInex!=-1){
        responseState = HeaderFound;
        responseHeaderSpliterSize = 4;
        connectionData->setResponseHeader(responseRawData.left(responseHeaderSpliterInex));
    }else{
        responseHeaderSpliterInex = responseRawData.indexOf(QByteArray("\r\n\r\n"));
        if(responseHeaderSpliterInex != -1){
            responseState = HeaderFound;
            connectionData->setResponseHeader(responseRawData.left(responseHeaderSpliterInex));
        }
    }
    if(responseState == HeaderFound){
        isResponseChunked = connectionData->getResponseHeader("Transfer-Encoding").toLower() == "chunked";
        responseContentLength = connectionData->getResponseHeader("Content-Length").toInt();
        responseComressType = connectionData->getResponseHeader("Content-Encoding");
    }
}

bool QiPipe_Private::parseResponseBody(const QByteArray &newContent){
    if(connectionData->returnCode == 302
            || connectionData->returnCode == 301
            || connectionData->returnCode == 307
            || connectionData->returnCode == 204){//todo
        return true;
    }
    Q_UNUSED(newContent)
    //根据http协议，需由header及body共同判断请求是否结束。
    if(isResponseChunked){//is chuncked
        QByteArray theBody = responseRawData.mid(responseHeaderSpliterSize+responseHeaderSpliterInex);
        //theBody.replace("\r\n","\n");
        int i=0;
        int l=theBody.length();
        while(i<=l){//need to valid chunk here?
            qDebug()<<"chunked:"<<i<<" "<<l;
            int beginOfLength=theBody.indexOf('\n',i);
            if(beginOfLength == -1){
                beginOfLength = theBody.indexOf('\r\n',i);
            }
            if(beginOfLength==-1){
                return false;
            }
            int endOfLength = theBody.indexOf('\n',beginOfLength);
            if(endOfLength==-1){
                endOfLength = theBody.indexOf('\r\n',beginOfLength);
                if(endOfLength == -1){
                    return false;
                }
            }
            bool isChunkValid;
            int chunkSize = theBody.mid(beginOfLength,endOfLength-beginOfLength).toInt(&isChunkValid,16);
            if(chunkSize==0){
                return true;
            }
            // don't do this until comfirm reponse done
            /*
            if(chunkSize+endOfLength+1<=l){
                connectionData->unChunkResponse.append(theBody.mid(endOfLength+1,chunkSize));
            }
            */
            if(!isChunkValid){
                return false;
            }
            i = chunkSize+endOfLength+1;
            if(i>l){
                return false;
            }
        }
    }else{
        if(responseContentLength<=responseRawData.length()-(responseHeaderSpliterSize+responseHeaderSpliterInex)){
            return true;
        }
    }
    return false;

}
void QiPipe_Private::tearDown(){

}

void QiPipe_Private::finishConnectionSuccess(){

}

void QiPipe_Private::finishConnectionWithError(int errno){

}
