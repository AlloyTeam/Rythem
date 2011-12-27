#ifndef PIPEDATA_H
#define PIPEDATA_H

#include <QObject>
#include <QMap>

class PipeData {
private:
    QMap<QString,QString> allHeaders;
public:
    PipeData(int socketDescriptor);
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
    void setHeader(QString name,QString value);
    const QString getHeader(QString name)const;
    const QString getBodyDecoded();
};

#endif // PIPEDATA_H
