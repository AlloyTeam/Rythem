#ifndef RYUPDATECHECKER_H
#define RYUPDATECHECKER_H

#include <QtCore>
#include <QtNetwork>

#define CURRENT_VERSION "0.5.08.22"

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
    void check(QString url="http://iptton.alloyteam.com/rythem_update.php?mac",QString currentVersion=CURRENT_VERSION);
#endif
#ifdef Q_WS_WIN32
    void check(QString url="http://iptton.alloyteam.com/rythem_update.php?windows",QString currentVersion=CURRENT_VERSION);
#endif
#ifdef Q_WS_X11
    void check(QString url="http://iptton.alloyteam.com/rythem_update.php?x11",QString currentVersion=CURRENT_VERSION);
#endif

private slots:
    void finished(QNetworkReply* reply);
private:
    QNetworkAccessManager manager;
    QString _currentVersion;

    bool _isChecking;
};

#endif // RYUPDATECHECKER_H
