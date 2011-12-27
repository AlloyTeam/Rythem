#ifndef QPROXYSERVER_H
#define QPROXYSERVER_H

#include <QTcpServer>
#include <pipedata.h>
#include <QVector>
class QPipe;
class PipeData;

class QProxyServer : public QTcpServer{
        Q_OBJECT
    public:
        explicit QProxyServer(QObject *parent = 0);
        
    signals:
        void newPipe(int socketId);
        void pipeUpdate(int socketId,const PipeData pipeData);
    public slots:
        void onPipeConnected();
        void onPipeComplete();
        void onPipeError();
    protected:
        void incomingConnection(int handle);

        QVector<QPipe*> pipes;
        
};

#endif // QPROXYSERVER_H
