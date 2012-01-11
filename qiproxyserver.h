#ifndef QPROXYSERVER_H
#define QPROXYSERVER_H

#include <QMutex>
#include <QMutexLocker>
#include <QTcpServer>
#include <qiconnectiondata.h>
#include <QVector>
class QiPipe;
class QiConnectionData;
#include <QSharedPointer>

class QiProxyServer : public QTcpServer{
        Q_OBJECT
    public:
        explicit QiProxyServer(QObject *parent = 0);
         ~QiProxyServer();
        
    signals:
        void newPipe(ConnectionData_ptr);
        void pipeUpdate(ConnectionData_ptr);
    public slots:
        void onConnectionConnected(ConnectionData_ptr);
        void onConnectionComplete(ConnectionData_ptr);
        void onConnectionError(ConnectionData_ptr);
        void onPipeFinished();
    protected:
        void incomingConnection(int handle);

        QMap<int,QThread*> threads;
        QMap<int,QiPipe*> pipes;
    private:
        QiPipe* addPipe(int socketDescriptor);
        bool removePipe(int socketId);
        void removeAllPipe();

        QMutex mutex;

        static QMutex connectionIdMutex;
        static long connectionId;
    public:
        static long nextConnectionId(){
            QMutexLocker locker(&connectionIdMutex);
            Q_UNUSED(locker)
            connectionId++;
            return connectionId;
        }
        
};

#endif // QPROXYSERVER_H
