#ifndef QIRESPONSE_H
#define QIRESPONSE_H

#include <QObject>
#include "qiconnectiondata.h"
#include <QTcpSocket>




class QiResponse : public QObject
{
        Q_OBJECT
    public:
        typedef enum ResponseType{
            HostReplaceResponse,
            ContentReplaceResponse,
            ContentCombineReplaceRespone,
            SimpleRespose
        } ResponseType;
        typedef enum BreakType{
            BreakOnRequest,
            BreakOnResponse
        } BreakType;
        explicit QiResponse(QTcpSocket *socket,ResponseType type);
        void setBreakpoint(BreakType breakType);

    signals:
        
    public slots:
};

#endif // QIRESPONSE_H
