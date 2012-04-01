#ifndef RYWINHTTP_H
#define RYWINHTTP_H



#include <QObject>
#include <QNetworkProxy>
#include <QList>
#include <qt_windows.h>
#include <wininet.h>
#include <QLibrary>
#include <QUrl>
#include <QNetworkProxyQuery>


class RyWinHttp : public QObject
{
    Q_OBJECT
public:
    explicit RyWinHttp(QObject *parent = 0);
signals:

public slots:

public:
    static void init();
    static QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query);
    static void setupProxy(QString host,int port);
    static void restoreProxy();
};

#endif // RYWINHTTP_H

