#ifndef QPROXYSERVER_H
#define QPROXYSERVER_H

#include <QTcpServer>
#include <pipedata.h>
#include <QVector>
class QPipe;
class PipeData;
#include <QSharedPointer>

class QProxyServer : public QTcpServer{
        Q_OBJECT
    public:
        explicit QProxyServer(QObject *parent = 0);
        
    signals:
        void newPipe(QSharedPointer<PipeData>);
        void pipeUpdate(QSharedPointer<PipeData>);
    public slots:
        void onPipeConnected(QSharedPointer<PipeData>);
        void onPipeComplete(QSharedPointer<PipeData>);
        void onPipeError(QSharedPointer<PipeData>);
    protected:
        void incomingConnection(int handle);

        QMap<int,QPipe*> pipes;
    private:
        QPipe* addPipe(QTcpSocket* socket);
        bool removePipe(int socketId);
        
};

#endif // QPROXYSERVER_H
