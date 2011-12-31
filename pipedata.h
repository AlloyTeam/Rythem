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
    inline PipeData(PipeData& p){//copy ctor
        number = p.number;
        returnCode = p.returnCode;
        protocol = p.protocol;
        serverIP = p.serverIP;
        host = p.host;
        URL = p.URL;
        port = p.port;
        reqHeader = p.reqHeader;
        reqBody = p.reqBody;
        resStatus = p.resStatus;
        resHeader = p.resHeader;
        resBody = p.resBody;
        allHeaders = p.allHeaders;
    }
    inline PipeData(const PipeData& p){
        qDebug()<<"PipeData(const PipeData&) called";
    }

    int socketId;
    int number;
    int returnCode;
    QString protocol;
    QString serverIP;
    QString host;
    QString URL;
    int port;
    QString reqHeader;
    QString reqBody;

    QString resStatus;
    QString resHeader;
    QString resBody;
    QMap<QByteArray,QByteArray> allHeaders;
    void setHeader(QByteArray name,QByteArray value);
    const QByteArray getHeader(QByteArray name)const;
    const QString getBodyDecoded();
};
typedef QSharedPointer<const PipeData> Pipedata_const_ptr;
typedef QSharedPointer<PipeData> PipeData_ptr;
typedef const QSharedPointer<PipeData> PipeData_ptr_const;
typedef const QSharedPointer<const PipeData> PipeData_const_ptr_const;




#endif // PIPEDATA_H
