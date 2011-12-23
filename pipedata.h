#ifndef PIPEDATA_H
#define PIPEDATA_H

#include <QObject>
#include <QMap>

class PipeData : public QObject{
private:
    QMap<QString,QString> allHeaders;
public:
    PipeData();
    QString host;
    int port;
    QString reqHeader;
    QString reqBody;

    QString resStatus;
    QString resHeader;
    QString resBody;
    void setHeader(QString name,QString value);
    const QString getHeader(QString name)const;
};

#endif // PIPEDATA_H
