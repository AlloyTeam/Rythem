#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QTcpServer>
#include <QtCore>
#include "websocketclient.h"

class WebSocketServer : public QTcpServer
{
	Q_OBJECT
public:
	explicit WebSocketServer(QObject *parent = 0);
	~WebSocketServer();
	
signals:
	void started(int port);
	void closed();
	
public slots:
	bool start(int port = 9828);
	void close();

protected:
	void incomingConnection(int handle);

protected slots:
	void onClientMessage(const QByteArray message);
	void onClientFinished();

private:
	QList<WebSocketClient *> *clients;
};

#endif // WEBSOCKETSERVER_H
