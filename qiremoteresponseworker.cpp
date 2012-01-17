#include "qiremoteresponseworker.h"

QiRemoteResponseWorker::QiRemoteResponseWorker(QObject *)
{
	sendBuffer = new QByteArray();
	receiveBuffer = new QByteArray();
	//create and init the response socket
	responseSocket = new QTcpSocket();
	connect(responseSocket, SIGNAL(connected()), this, SLOT(onResponseConnected()));
	connect(responseSocket, SIGNAL(readyRead()), this, SLOT(onResponseReadReady()));
	connect(responseSocket, SIGNAL(disconnected()), this, SLOT(onResponseClose()));
	connect(responseSocket, SIGNAL(aboutToClose()), this, SLOT(onResponseClose()));
	connect(responseSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onResponseError(QAbstractSocket::SocketError)));
}

QiRemoteResponseWorker::~QiRemoteResponseWorker(){
	delete sendBuffer;
	delete receiveBuffer;
	//abort remote connection and destory the socket
	responseSocket->abort();
	responseSocket->deleteLater();
}

int QiRemoteResponseWorker::bytesAvailable(){
	return responseSocket->bytesAvailable();
}

void QiRemoteResponseWorker::start(){
	if(_url.length() && _port > 0){
		responseSocket->connectToHost(_url, _port);
		_connected = true;
	}
}

void QiRemoteResponseWorker::stop(){
	if(_connected){
		responseSocket->abort();
	}
}

void QiRemoteResponseWorker::write(const QByteArray &byteArray){
	if(responseSocket->isValid()){
		responseSocket->write(byteArray);
	}
	else{
		sendBuffer->append(byteArray);
	}
}

void QiRemoteResponseWorker::flush(){
	responseSocket->flush();
}

void QiRemoteResponseWorker::onResponseConnected(){
	emit started();
	if(sendBuffer->size()){
		responseSocket->write(*sendBuffer);
		sendBuffer->clear();
	}
}

void QiRemoteResponseWorker::onResponseReadReady(){
	QByteArray data = responseSocket->readAll();
	receiveBuffer->append(data);
	emit dataReceived(data);
}

void QiRemoteResponseWorker::onResponseClose(){
	_connected = false;
	emit stopped();
}

void QiRemoteResponseWorker::onResponseError(QAbstractSocket::SocketError e){
	_connected = false;
	emit error(e);
}
