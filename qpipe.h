#ifndef QPIPE_H
#define QPIPE_H

#include <QObject>
#include <QTcpSocket>
#include "pipedata.h"
#include <QMutex>
#include <QSslSocket>
#include <QThread>
#include <QTcpSocket>
class QPipe : public QThread
{
    Q_OBJECT
private:
        QString* reqRawString;
        QTcpSocket* requestSocket;
        QByteArray reqByteArray;
        QString reqSig;// change GET http://xxx.xx.xx/a/path/to/some.index HTTP/1.1 to GET /a/path/to/some.index HTTP/1.1
        QString reqHeaderString;
        PipeData* pipeData;
        QTcpSocket* responseSocket;
        QSslSocket* responseSocketSSL;
        bool headerFound;

        bool isHttpsConnect;

        QMutex mutex;

public:
        explicit QPipe(QTcpSocket *socket = 0);
        ~QPipe();
        void tearDown();

signals:
        void completed(PipeData);
        void error(PipeData);
        void connected(PipeData);

public slots:
        void onReqSocketReadReady();
        void onReqSocketReadFinished();
        void onRequestHostFound();
        void onRequestSocketError();
        void onRequestSocketClose();
        void onResponseConnected();
        void onResponseReceived();
        void onResponseError(QAbstractSocket::SocketError);
        void onResponseClose();

protected:
        void run();

private:
        void parseHeader(const QString headerString);
};

#endif // QPIPE_H
