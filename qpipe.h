#ifndef QPIPE_H
#define QPIPE_H

#include <QObject>
#include <QTcpSocket>
#include "pipedata.h"
class QPipe : public QObject
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
        bool headerFound;

public:
        explicit QPipe(QTcpSocket *socket = 0);
        ~QPipe();

signals:
        void completed();
        void error();
        void connected();

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

private:
        void parseHeader(const QString headerString);
};

#endif // QPIPE_H
