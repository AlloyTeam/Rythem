#ifndef RYPROXYSERVER_H
#define RYPROXYSERVER_H

#include <QtCore>
#include <QTcpServer>
#include "ryconnection.h"

class RyProxyServer : public QTcpServer,public QRunnable
{
        Q_OBJECT
    public:
        static RyProxyServer* instance();
        explicit RyProxyServer(QObject *parent = 0);
        ~RyProxyServer();
        void run();
        void close();
    signals:
        void pipeBegin(RyPipeData_ptr);
        void pipeComplete(RyPipeData_ptr);
        void pipeError(RyPipeData_ptr);
    public slots:
        void cacheSocket(QString address,quint16 port,QTcpSocket* socket);
        QTcpSocket* getSocket(QString address,quint16 port,bool* isFromCache,QThread* _desThread);


    protected:
        void incomingConnection(int handle);

    private:
        QMutex connectionOpMutex;
        QMutex _socketsOpMutex;
        //sockets
        // cache for reuse
        QMultiMap<QString,QTcpSocket*> _cachedSockets;
        QMultiMap<QString,QTcpSocket*> _liveSockets;
        //connections
        //TODO  should has a timeout to kill connections
        //      when those idle to much time
        QList<RyConnection*> _connections;

        QMap<RyConnection*,QThread*> _threads;

        RyConnection *_getConnection(int handle);

    private slots:
        void onConnectionIdleTimeout();
        void onPipeBegin(RyPipeData_ptr);
        void onPipeComplete(RyPipeData_ptr);
        void onPipeError(RyPipeData_ptr);
        void onThreadTerminated();
        void onConnectionClosed();

        
};

#endif // RYPROXYSERVER_H
