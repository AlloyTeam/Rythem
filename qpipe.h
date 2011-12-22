#ifndef QPIPE_H
#define QPIPE_H

#include <QObject>
#include <QTcpSocket>
#include "pipedata.h"
class QPipe : public QObject
{
    Q_OBJECT

private:
        QString* reqRawString;
        QTcpSocket* requestSocket;
        QByteArray* reqByteArray;
        PipeData* pipeData;

public:
        explicit QPipe(QTcpSocket *socket = 0);
        ~QPipe();

signals:
        void completed();
        void error();
        void connected();

public slots:
        void onReqSocketReadReady();
        void onReqSocketReadFinished();
        void onRequestHostFound();
        void onRequestSocketError();

private:
        void parseHeader(const QString headerString);
};

#endif // QPIPE_H
