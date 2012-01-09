#ifndef QIWINHTTP_H
#define QIWINHTTP_H



#include <QObject>
#include <QNetworkProxy>
#include <QList>
#include <qt_windows.h>
#include <wininet.h>
#include <QLibrary>
#include <QUrl>
#include <QNetworkProxyQuery>


class QiWinHttp : public QObject
{
    Q_OBJECT
public:
    explicit QiWinHttp(QObject *parent = 0);
signals:

public slots:

public:
    static void init();
    static QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query);
    static void setupProxy(QString host,int port);
    static void restoreProxy();
};

#endif // QIWINHTTP_H
