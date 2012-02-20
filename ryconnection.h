#ifndef RYCONNECTIONSOCKET_H
#define RYCONNECTIONSOCKET_H

#include <QtCore>
#include <QTcpSocket>
#include <QAbstractSocket>
#include "rypipedata.h"

class RyConnection:public QObject{
        Q_OBJECT
        Q_ENUMS(ConnectionState)
    public:
        //type defines
        enum ConnectionState{
            ConnectionStateInit=0,
            ConnectionStateConnecting = 1,// response only
            ConnectionStateConnected = 2, // response only
            ConnectionStateHeadFound=3,
            ConnectionStatePackageFound=4
        };
        //public functions
        explicit RyConnection(int handle,quint64 connectionId,QObject *parent = 0);
        ~RyConnection();
        int handle() const;
        void setHandle(int theHandle);
    public slots:
        void deleteLater();
        void run();//for qthread
        void printStates(){
            qDebug()<<_requestState
                      <<_responseState;
        }

    signals:
        void idleTimeout(); // idleTimeout should be release
        void pipeBegin(RyPipeData_ptr);
        void pipeComplete(RyPipeData_ptr);
        void pipeError(RyPipeData_ptr);

        void connectionClose();

    private slots:
        void onRequestReadyRead();
        void onRequestClose();
        void onRequestError(QAbstractSocket::SocketError);

        void onResponseConnected();
        void onResponseReadyRead();
        void onResponseClose();
        void onResponseError(QAbstractSocket::SocketError);

        void onRequestHeaderFound();
        void onRequestPackageFound();
        void onResponseHeaderFound();
        void onResponsePackageFound();

    private:
        //type defines

        //variables
        QMutex pipeDataMutex;
        QMutex pipeDataListMutex;
        QByteArray _requestBuffer;
        QByteArray _responseBuffer;

        int _pipeTotal;

        int _handle;
        bool closed;

        QString _connectingHost;
        quint16 _connectingPort;
        bool _isConnectTunnel;
        QString _fullUrl;

        quint64 _connectionId;

        QTcpSocket *_requestSocket;
        ConnectionState _requestState;

        QTcpSocket *_responseSocket;
        ConnectionState _responseState;

        RyPipeData_ptr _sendingPipeData;
        RyPipeData_ptr _receivingPipeData;
        QList<RyPipeData_ptr> _pipeList;

        int _connectionNotOkTime;
        bool _isSocketFromCache;
        QTimer timer;

        void parseRequest();
        void parseResponse();
        void doRequestToNetwork();

        RyPipeData_ptr nextPipe();
        void appendPipe(RyPipeData_ptr);

        //for performance
        RyPipeData::PerformanceDateTime _receivingPerformance;
        RyPipeData::PerformanceDateTime _sendingPerformance;
        //because one connection may has more than one socets;
        qint64 _lastSocketConnectedDateTime;

};

#endif // RYCONNECTIONSOCKET_H
