#include <QHostAddress>
#include <QThread>
#include <QApplication>
#include <QDebug>
#include "websocketserver.h"

WebSocketServer::WebSocketServer(QObject *parent) :
	QTcpServer(parent)
{
	clients = new QList<WebSocketClient *>();
}

WebSocketServer::~WebSocketServer(){
	if(isListening()){
		close();
	}
	delete clients;
}

bool WebSocketServer::start(int port){
	if(isListening()){
		close();
	}
	bool rs = listen(QHostAddress::LocalHost, port);
	if(rs){
		emit started(serverPort());
	}
	return rs;
}

void WebSocketServer::close(){
	QTcpServer::close();
	emit closed();
}

void WebSocketServer::incomingConnection(int handle){
	WebSocketClient *client = new WebSocketClient();
	connect(client, SIGNAL(message(QByteArray)), this, SLOT(onClientMessage(QByteArray)));
	connect(client, SIGNAL(finished()), this, SLOT(onClientFinished()));
	client->setSocketDescriptor(handle);
	clients->append(client);
}

void WebSocketServer::onClientMessage(const QByteArray message){
	//echo message back to client
	WebSocketClient *client = (WebSocketClient *)sender();
	client->sendMessage(message);
}

void WebSocketServer::onClientFinished(){
	//remove disconnected client
	WebSocketClient *client = (WebSocketClient *)sender();
	clients->removeOne(client);
	client->deleteLater();
}
