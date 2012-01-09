#ifndef QPIPE_H
#define QPIPE_H

#include <QObject>
#include <QTcpSocket>
#include "qiconnectiondata.h"
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

class QiPipe : public QObject
{
    Q_OBJECT
private:
        QiPipe_Private *qp;
        int _socketDescriptor;
public://public member functions
        explicit QiPipe(int socketDescriptor = 0);
        ~QiPipe();
signals:
        void completed(ConnectionData_ptr);
        void error(ConnectionData_ptr);
        void connected(ConnectionData_ptr);
        void pipeFinished();

public slots:
        void run();
public://public variables
        QMutex mutex;//seems don't need any lock?
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
        bool parseResponseBody(const QByteArray &body);//根据http协议，需由header及body共同判断请求是否结束

        QByteArray requestRawData;
        QByteArray requestRawDataHeader;
        QByteArray requestRawDataBody;

        bool requestHeaderFound;
        int requestHeaderSpliterSize;
        int requestHeaderSpliterIndex;
        int requestContentLength;
        int requestBodyRemain;

        bool responseHeaderFound;
        int responseHeaderSpliterSize;
        int responseHeaderSpliterInex;
        int responseContentLength;
        int responseBodyRemain;
        bool isResponseChunked;
        QByteArray responseComressType;

        QByteArray responseRawData;
        QByteArray responseBodyRawData;

        QTcpSocket* requestSocket;
        QTcpSocket* responseSocket;

        QSharedPointer<QiConnectionData> connectionData;
        RequestInfo requestInfo;
        QMutex mutex;


        enum State {
                Connected,   // response only
                BodyParsing, // response only
                Initial,     // both
                HeaderFound, // both
                PackageFound // both
        };
        State requestState;
        State responseState;
};

#endif // QPIPE_H
