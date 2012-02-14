#ifndef CONNECTIONMONITORWSSERVER_H
#define CONNECTIONMONITORWSSERVER_H
#include <QtCore>
#include "websocketserver.h"
#include "rypipedata.h"

//this class is for monitoring proxy connection and forward changes of them via websocket to client(s)
class ConnectionMonitorWSServer : public WebSocketServer
{
	Q_OBJECT
public:
	explicit ConnectionMonitorWSServer(QObject *parent = 0);
	
signals:
	
public slots:
	void handleConnectionAdd(RyPipeData_ptr p);
	void handleConnectionUpdate(RyPipeData_ptr p);
	void handleConnectionRemove(RyPipeData_ptr p);
	void handleConnectionChange(QModelIndex topLeft, QModelIndex bottomRight);

protected:
	void sendConnectionChangePackageToClients(RyPipeData_ptr p);
	
};

#endif // CONNECTIONMONITORWSSERVER_H
