#include "connectionmonitorwsserver.h"
#include <QDebug>

ConnectionMonitorWSServer::ConnectionMonitorWSServer(QObject *parent) :
	WebSocketServer(parent)
{
}

void ConnectionMonitorWSServer::handleConnectionAdd(RyPipeData_ptr p){
	sendConnectionChangePackageToClients(p);
}

void ConnectionMonitorWSServer::handleConnectionUpdate(RyPipeData_ptr p){
	sendConnectionChangePackageToClients(p);
}

void ConnectionMonitorWSServer::handleConnectionRemove(RyPipeData_ptr){
	qDebug() << "connection removed";
}

void ConnectionMonitorWSServer::handleConnectionChange(QModelIndex , QModelIndex ){

}

void ConnectionMonitorWSServer::sendConnectionChangePackageToClients(RyPipeData_ptr p){
	char s[] = ", ";
	QString package;
	QTextStream (&package) << p->id << s
                                                   << p->method << s
                                                   << p->responseStatus << s
						   << p->getRequestHeader("Content-Type");
	sendToAllClients(package.toLocal8Bit());
}
