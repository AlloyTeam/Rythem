#ifndef PIPEDATA_H
#define PIPEDATA_H

#include <QObject>
#include <QMap>
#include <QDebug>
#include <QSharedPointer>

class QiConnectionData {
private:
public:
    QiConnectionData(int socketDescriptor=-1);
    ~QiConnectionData(){
        qDebug()<<"~PipeData";
    }

    inline QiConnectionData(const QiConnectionData& p){//copy ctor
        qDebug()<<"copy ctor";
        number = p.number;

        returnCode = p.returnCode;
        protocol = p.protocol;
        serverIP = p.serverIP;
        host = p.host;
        path = p.path;
        port = p.port;
        fullUrl = p.fullUrl;
        requestMethod = p.requestMethod;
        unChunkResponse = p.unChunkResponse;
        setRequestRawData(p.requestRawData);
        setResponseRawData(p.responseRawData);
    }

    long id;
    int number;
    int returnCode;
    QString protocol;
    QString serverIP;
    QString host;
    QString path;
    QString fullUrl;
    int port;
    QString requestMethod;
    QString responseStatus;

    QByteArray unChunkResponse;

    QByteArray requestRawDataToSend;
    QByteArray requestBody;

    QByteArray responseBody;


    void setRequestHeader(QByteArray name,QByteArray value);
    void setRequestHeader(QByteArray headerBa);
    bool appendRequestBody(QByteArray newContent);
    QByteArray getRequestHeader(QByteArray name)const;
    QByteArray getRequestHeader()const;
    QByteArray getRequestBody()const;

    void setResponseHeader(QByteArray header);
    void setResponseBody(QByteArray body);
    bool appendResponseBody(QByteArray newContent);
    QByteArray getResponseHeader(QByteArray name)const;
    QByteArray getResponseHeader()const;
    QByteArray getResponseBody()const;
private:
    QByteArray requestRawData;
    QByteArray responseRawData;
    QMap<QByteArray,QByteArray> allRequestHeaders;
    QMap<QByteArray,QByteArray> allResponseHeaders;

    void setRequestRawData(QByteArray request);//only for copy Ctor
    void setResponseRawData(QByteArray response);//only for copy Ctor
};
typedef QSharedPointer<const QiConnectionData> ConnectionData_const_ptr;
typedef QSharedPointer<QiConnectionData> ConnectionData_ptr;
typedef const QSharedPointer<QiConnectionData> ConnectionData_ptr_const;
typedef const QSharedPointer<const QiConnectionData> ConnectionData_const_ptr_const;




#endif // PIPEDATA_H
