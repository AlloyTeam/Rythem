#include "qilocalfileresponseworker.h"

QiLocalFileResponseWorker::QiLocalFileResponseWorker(QObject *)
{
	receiveBuffer = new QByteArray();
	file = new QFile();
	connect(file, SIGNAL(readyRead()), this, SLOT(onFileReadReady()));
}

QiLocalFileResponseWorker::~QiLocalFileResponseWorker(){
	delete receiveBuffer;
	file->close();
	file->deleteLater();
}

int QiLocalFileResponseWorker::bytesAvailable(){
	return file->bytesAvailable();
}

void QiLocalFileResponseWorker::start(){
	if(_url.length()){
		file->setFileName(_url);
		if(file->exists()){
			file->open(QIODevice::ReadOnly);
			emit started();
		}
		else{
			emit error(FileNotFoundError);
		}
	}
}

void QiLocalFileResponseWorker::stop(){
	if(file->isOpen()){
		file->close();
	}
}

void QiLocalFileResponseWorker::write(const QByteArray &){

}

void QiLocalFileResponseWorker::flush(){

}

void QiLocalFileResponseWorker::onFileReadReady(){
	QByteArray data = file->readAll();
	receiveBuffer->append(data);
	emit dataReceived(data);
}
