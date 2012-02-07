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
#include <QNetworkAccessManager>

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
        int sockeId()const{
            return _socketDescriptor;
        }
signals:
        void completed(ConnectionData_ptr);
        void error(ConnectionData_ptr);
        void connected(ConnectionData_ptr);
        void pipeFinished();

public slots:
        void run();
public://public variables
        QMutex mutex;//seems don't need any lock?
private slots:
        void onPipeFinished();
};


class QiPipe_Private:public QObject{
    Q_OBJECT
public:
    QiPipe_Private(int descriptor);
    ~QiPipe_Private();

signals:
        void connected(ConnectionData_ptr);

        void finishedWithError(ConnectionData_ptr);
        void finishSuccess(ConnectionData_ptr);

        void pipeFinished();

public slots:
        void onRequestReadReady();
        void onRequestError();
        void onRequestClose();

        void onResponseConnected();
        void onResponseReadReady();
        void onResponseError(QAbstractSocket::SocketError);
        void onResponseClose();
        /*
        void onResponseHostFound(){
            qDebug()<<"onResponseHostFound"<<responseSocket->peerAddress().toString();
        }
        */
        void tearDown();
private:
        void parseRequest(const QByteArray &requestBa);
        void parseRequestHeader(const QByteArray & header);
        bool parseResponse(const QByteArray responseBa);
        void parseResponseHeader(const QByteArray &header);
        bool parseResponseBody(QByteArray body);//根据http协议，需由header及body共同判断请求是否结束
        void finishConnectionSuccess();
        void finishConnectionWithError(int errno);
        ConnectionData_ptr nextConnectionData();


        bool requestHeaderFound;
        long requestHeaderSpliterSize;
        long requestHeaderSpliterIndex;
        long requestContentLength;
        long requestBodyRemain;

        bool responseHeaderFound;
        long responseHeaderSpliterSize;
        long responseHeaderSpliterIndex;
        long responseContentLength;
        long responseBodyRemain;
        bool isResponseChunked;

        QString serverIp;

        QByteArray responseComressType;

        QByteArray requestBuffer;
        QByteArray responseBuffer;


        QTcpSocket* requestSocket;
        QTcpSocket* responseSocket;

        //当前未接收完毕请求内容的http包
        ConnectionData_ptr gettingRequestConnectionData;
        //当前未接收完毕返回内容的http包
        ConnectionData_ptr receivingResponseConnectinoData;
        // 此队列为已接收完毕请求内容的http包
        QVector<ConnectionData_ptr> bufferConnectionArray;

        RequestInfo requestInfo;
        QMutex mutex;

        QNetworkAccessManager networkManager;


        enum State {
                Connected,   // response only
                Connecting, // response only
                Initial,     // both
                HeaderFound, // both
                PackageFound // both
        };
        State requestState;
        State responseState;

        QString host;
};

#endif // QPIPE_H
