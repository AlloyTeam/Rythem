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
#include <QNetworkAccessManager>

typedef struct RequestInfo{
    QString url;
    QMap<QByteArray,QByteArray> headers;
    QByteArray data;
    QString method;
}RequestInfo;

class QiPipe : public QThread
{
    Q_OBJECT
private:
        QString* reqRawString;
        QTcpSocket* requestSocket;

        QSharedPointer<PipeData> pipeData;
        RequestInfo requestInfo;

        QTcpSocket* responseSocket;
        QSslSocket* responseSocketSSL;
        QMutex mutex;
        QNetworkAccessManager *manager;

        int _socketDescriptor;

public:
        explicit QiPipe(int socketDescriptor = 0);
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

        void onResponseFinished(QNetworkReply*);

protected:
        void run();


private:
        void parseRequest(const QByteArray requestString);
};

#endif // QPIPE_H
