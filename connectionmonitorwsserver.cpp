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

void ConnectionMonitorWSServer::handleConnectionChange(QModelIndex topLeft, QModelIndex bottomRight){

}

void ConnectionMonitorWSServer::sendConnectionChangePackageToClients(ConnectionData_ptr p){
	char s[] = ", ";
	QString package;
	QTextStream (&package) << p->id << s
						   << p->requestMethod << s
						   << p->returnCode << s
						   << p->getRequestHeader("Content-Type");
	sendToAllClients(package.toLocal8Bit());
}
