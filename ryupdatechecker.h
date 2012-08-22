#ifndef RYUPDATECHECKER_H
#define RYUPDATECHECKER_H

#include <QtCore>
#include <QtNetwork>

class RyUpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit RyUpdateChecker(QObject *parent = 0);
    bool isLargeThanCurrent(const QString& otherVersion,bool* isOk=0);
signals:
    void gotNew(const QString& oldVersion,const QString& newVersion,const QString& updateUrl,const QString& log);
    void updated();
    void updateError(QNetworkReply::NetworkError err);

public slots:
#ifdef Q_WS_MAC
    void check(QString url="http://iptton.alloyteam.com/rythem_update.php?mac",QString currentVersion="0.1.05.31");
#endif
#ifdef Q_WS_WIN32
    void check(QString url="http://iptton.alloyteam.com/rythem_update.php?windows",QString currentVersion="0.1.05.31");
#endif
#ifdef Q_WS_X11
    void check(QString url="http://iptton.alloyteam.com/rythem_update.php?x11",QString currentVersion="0.1.05.31");
#endif

private slots:
    void finished(QNetworkReply* reply);
private:
    QNetworkAccessManager manager;
    QString _currentVersion;

    bool _isChecking;
};

#endif // RYUPDATECHECKER_H
