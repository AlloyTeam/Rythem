#ifndef RYPIPERESPONSE_H
#define RYPIPERESPONSE_H

#include <QtCore>
#include <QTcpSocket>

class RyPipeResponse:public QObject{
    Q_OBJECT
public:
    RyPipeResponse(QTcpSocket *requestSocket);

private:
};

#endif // RYPIPERESPONSE_H
