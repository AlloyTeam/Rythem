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
#include <QRunnable>
#include <QEventLoop>

typedef struct RequestInfo{
    QString url;
    QMap<QByteArray,QByteArray> headers;
    QByteArray data;
    QString method;
}RequestInfo;



class QiPipe_Private;

class QiPipe : public QThread
{
    Q_OBJECT
private:
        QiPipe_Private *qp;
        int _socketDescriptor;
public:
        explicit QiPipe(int socketDescriptor = 0);
        ~QiPipe();
signals:
        void completed(ConnectionData_ptr);
        void error(ConnectionData_ptr);
        void connected(ConnectionData_ptr);
        void pipeFinished();
public slots:
        void onPipeFinished();
protected:
        void run();

        bool stoped;
        QMutex mutex;
};


class QiPipe_Private:public QObject{
    Q_OBJECT
public:
    QiPipe_Private(int descriptor);
    ~QiPipe_Private();

signals:
        void completed(ConnectionData_ptr);
        void error(ConnectionData_ptr);
        void connected(ConnectionData_ptr);
        void finished();// error or completed

public slots:
        void onRequestReadReady();
        void onRequestError();
        void onRequestClose();
        void onResponseConnected();
        void onResponseReadReady();
        void onResponseError(QAbstractSocket::SocketError);
        void onResponseClose();
        void tearDown();
private:
        void parseRequest(const QByteArray &requestBa);
        void parseRequestHeader(const QByteArray & header);
        bool parseResponse(const QByteArray &responseBa);
        void parseResponseHeader(const QByteArray &header);
        void parseResponseBody(const QByteArray &body);//根据http协议，需由header及body共同判断请求是否结束�

        QByteArray requestRawData;
        QByteArray requestRawDataHeader;
        QByteArray requestRawDataBody;

        bool requestHeaderFound;
        int requestHeaderSpliterSize;
        int requestHeaderSpliterIndex;

        bool responseHeaderFound;
        int responseHeaderSpliterSize;
        int responseHeaderSpliterInex;

        QByteArray responseRawData;
        QByteArray responseBodyRawData;

        QTcpSocket* requestSocket;
        QTcpSocket* responseSocket;

        QSharedPointer<QiConnectionData> pipeData;
        RequestInfo requestInfo;
        QMutex mutex;
};

#endif // QPIPE_H
