#include "iqiresponseworker.h"

bool IQiResponseWorker::isConnected(){
	return _connected;
}

QString IQiResponseWorker::url(){
	return _url;
}

int IQiResponseWorker::port(){
	return _port;
}

int IQiResponseWorker::bytesAvailable(){
	return 0;
}

void IQiResponseWorker::setDestination(const QString url, const int port){
	_url = url;
	_port = port;
}

void IQiResponseWorker::start(){

}

void IQiResponseWorker::stop(){

}

void IQiResponseWorker::write(const QByteArray &){

}

void IQiResponseWorker::flush(){

}
