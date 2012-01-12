#include "connectionmonitorwsserver.h"
#include <QDebug>

ConnectionMonitorWSServer::ConnectionMonitorWSServer(QObject *parent) :
	WebSocketServer(parent)
{
}

void ConnectionMonitorWSServer::handleConnectionAdd(ConnectionData_ptr p){
	sendConnectionChangePackageToClients(p);
}

void ConnectionMonitorWSServer::handleConnectionUpdate(ConnectionData_ptr p){
	sendConnectionChangePackageToClients(p);
}

void ConnectionMonitorWSServer::handleConnectionRemove(ConnectionData_ptr){
	qDebug() << "connection removed";
}

void ConnectionMonitorWSServer::sendConnectionChangePackageToClients(ConnectionData_ptr p){
	QString package;
	package.sprintf("id:%ld, status:%d", p->id, p->returnCode);
	sendToAllClients(package.toLocal8Bit());
}
