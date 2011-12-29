#ifndef QPIPE_H
#define QPIPE_H

#include <QObject>
#include <QTcpSocket>
#include "pipedata.h"
#include <QMutex>
#include <QSslSocket>
#include <QThread>
#include <QTcpSocket>
#include <QSharedPointer>

class QiPipe : public QThread
{
    Q_OBJECT
private:
        QString* reqRawString;
        QTcpSocket* requestSocket;
        QByteArray reqByteArray;
        QString reqSig;// change GET http://xxx.xx.xx/a/path/to/some.index HTTP/1.1 to GET /a/path/to/some.index HTTP/1.1
        QString reqHeaderString;
        QSharedPointer<PipeData> pipeData;
        QTcpSocket* responseSocket;
        QSslSocket* responseSocketSSL;
        bool headerFound;

        bool isHttpsConnect;
        bool isError;

        QMutex mutex;

public:
        explicit QiPipe(QTcpSocket *socket = 0);
        ~QiPipe();
        void tearDown();

signals:
        void completed(Pipedata_const_ptr);
        void error(Pipedata_const_ptr);
        void connected(Pipedata_const_ptr);

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
