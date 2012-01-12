#include "connectionmonitorwsserver.h"
#include <QDebug>

ConnectionMonitorWSServer::ConnectionMonitorWSServer(QObject *parent) :
	WebSocketServer(parent)
{
}

void ConnectionMonitorWSServer::handleConnectionAdd(ConnectionData_ptr p){
	qDebug() << "connection added";
}

void ConnectionMonitorWSServer::handleConnectionUpdate(ConnectionData_ptr p){
	qDebug() << "connecton updated";
}

void ConnectionMonitorWSServer::handleConnectionRemove(ConnectionData_ptr p){
	qDebug() << "connection removed";
}
