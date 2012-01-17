#ifndef QIREMOTERESPONSEWORKER_H
#define QIREMOTERESPONSEWORKER_H

#include <QtCore>
#include <QTcpSocket>
#include "iqiresponseworker.h"

class QiRemoteResponseWorker : public IQiResponseWorker
{
	Q_OBJECT
public:
	explicit QiRemoteResponseWorker(QObject *parent = 0);
	~QiRemoteResponseWorker();
	int bytesAvailable();
	
signals:
	
public slots:
	void start(); //start connecting to remote destination
	void stop(); //stop the connection
	void write(const QByteArray &byteArray); //write data to remote destination
	void flush(); //flush data to remote destination

protected:
	QTcpSocket *responseSocket;
	QByteArray *sendBuffer;
	QByteArray *receiveBuffer;

protected slots:
	void onResponseConnected();
	void onResponseReadReady();
	void onResponseClose();
	void onResponseError(QAbstractSocket::SocketError error);
	
};

#endif // QIREMOTERESPONSEWORKER_H
