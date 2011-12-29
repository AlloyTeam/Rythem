#ifndef QPROXYSERVER_H
#define QPROXYSERVER_H

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
        
    signals:
        void newPipe(Pipedata_const_ptr);
        void pipeUpdate(Pipedata_const_ptr);
    public slots:
        void onPipeConnected(Pipedata_const_ptr);
        void onPipeComplete(Pipedata_const_ptr);
        void onPipeError(Pipedata_const_ptr);
    protected:
        void incomingConnection(int handle);

        QMap<int,QiPipe*> pipes;
    private:
        QiPipe* addPipe(QTcpSocket* socket);
        bool removePipe(int socketId);
        
};

#endif // QPROXYSERVER_H
