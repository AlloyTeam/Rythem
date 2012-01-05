#ifndef QPROXYSERVER_H
#define QPROXYSERVER_H

#include <QMutex>
#include <QMutexLocker>
#include <QTcpServer>
#include <pipedata.h>
#include <QVector>
class QiPipe;
class PipeData;
#include <QSharedPointer>

class QiProxyServer : public QTcpServer{
        Q_OBJECT
    public:
        explicit QiProxyServer(QObject *parent = 0);
         ~QiProxyServer();
        
    signals:
        void newPipe(PipeData_ptr);
        void pipeUpdate(PipeData_ptr);
    public slots:
        void onPipeConnected(PipeData_ptr);
        void onPipeComplete(PipeData_ptr);
        void onPipeError(PipeData_ptr);
    protected:
        void incomingConnection(int handle);

        QMap<int,QiPipe*> pipes;
    private:
        QiPipe* addPipe(int socketDescriptor);
        bool removePipe(int socketId);
        void removeAllPipe();

        QMutex mutex;
        
};

#endif // QPROXYSERVER_H
