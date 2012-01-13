#include <QHostAddress>
#include <QThread>
#include <QApplication>
#include <QDebug>
#include "websocketserver.h"

WebSocketServer::WebSocketServer(QObject *parent) :
	QTcpServer(parent)
{
	clients = new vector<WebSocketClient *>();
}

WebSocketServer::~WebSocketServer(){
	if(isListening()){
		close();
	}
	delete clients;
}

//startup the websocket server and start listening on specify port
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

//close the server
void WebSocketServer::close(){
	QTcpServer::close();
	emit closed();
}

//create a new websocket to handle incoming connection
//the client is responsible for process websocket handshake and receive/send data frame from/to remote endpoint
void WebSocketServer::incomingConnection(int handle){
	WebSocketClient *client = new WebSocketClient();
	connect(client, SIGNAL(message(QByteArray)), this, SLOT(onClientMessage(QByteArray)));
	connect(client, SIGNAL(finished()), this, SLOT(onClientFinished()));
	client->setSocketDescriptor(handle);
	clients->push_back(client);
}

//send specify message to all connected websocket clients
void WebSocketServer::sendToAllClients(const char *message){
	sendToAllClients(QByteArray(message));
}
void WebSocketServer::sendToAllClients(const QByteArray &message){
	vector<WebSocketClient *>::iterator it;
	for(it = clients->begin(); it < clients->end(); it++){
		((WebSocketClient *)(*it))->sendMessage(message);
	}
}

void WebSocketServer::onClientMessage(const QByteArray message){
	//echo message back to client
	WebSocketClient *client = (WebSocketClient *)sender();
	client->sendMessage(message);
}

void WebSocketServer::onClientFinished(){
	//remove disconnected client
	WebSocketClient *client = (WebSocketClient *)sender();
	vector<WebSocketClient *>::iterator it;
	for(it = clients->begin(); it < clients->end(); it++){
		if(*it == client){
			clients->erase(it);
			break;
		}
	}
	client->deleteLater();
}
