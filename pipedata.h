#ifndef PIPEDATA_H
#define PIPEDATA_H

#include <QObject>
#include <QMap>
#include <QDebug>
#include <QSharedPointer>

class PipeData {
private:
public:
    PipeData(int socketDescriptor=-1);
    inline PipeData(const PipeData& p){//copy ctor
        number = p.number;
        /*
        returnCode = p.returnCode;
        protocol = p.protocol;
        serverIP = p.serverIP;
        host = p.host;
        path = p.path;
        port = p.port;
        resStatus = p.resStatus;
        resHeader = p.resHeader;
        resBody = p.resBody;
        fullUrl = p.fullUrl;
        requestMethod = p.requestMethod;
        */
        setRequestRawData(p.requestRawData);
        setResponseRawData(p.responseRawData);
    }

    int socketId;
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


    void setRequestHeader(QByteArray name,QByteArray value);
    void setRequestHeader(QByteArray headerBa);
    QByteArray getRequestHeader(QByteArray name)const;
    QByteArray getRequestHeader()const;
    QByteArray getRequestBody()const;

    void setResponseHeader(QByteArray name,QByteArray value);
    void setResponseBody(QByteArray body);
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
typedef QSharedPointer<const PipeData> Pipedata_const_ptr;
typedef QSharedPointer<PipeData> PipeData_ptr;
typedef const QSharedPointer<PipeData> PipeData_ptr_const;
typedef const QSharedPointer<const PipeData> PipeData_const_ptr_const;




#endif // PIPEDATA_H
