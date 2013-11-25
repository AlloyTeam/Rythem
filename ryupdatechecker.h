#ifndef RYUPDATECHECKER_H
#define RYUPDATECHECKER_H

#include <QtCore>
#include <QtNetwork>

#ifdef Q_OS_MAC
#define CURRENT_VERSION "0.13.11.15"
#endif
#ifdef Q_OS_LINUX
#define CURRENT_VERSION "0.5.09.21"
#endif
#ifdef Q_OS_WIN
#define CURRENT_VERSION "0.13.11.15"
#endif

#define RYTHEM_UPDATE_PREFIX "http://rythem.alloyteam.com/update.php?"
#define RYTHEM_UPDATE_URL(sys) RYTHEM_UPDATE_PREFIX # sys

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
#ifdef Q_OS_MAC
    void check(QString url=RYTHEM_UPDATE_URL(mac),QString currentVersion=CURRENT_VERSION);
#endif
#ifdef Q_OS_WIN32
    void check(QString url=RYTHEM_UPDATE_URL(windows),QString currentVersion=CURRENT_VERSION);
#endif
#ifdef Q_OS_X11
    void check(QString url=RYTHEM_UPDATE_URL(x11),QString currentVersion=CURRENT_VERSION);
#endif

private slots:
    void finished(QNetworkReply* reply);
private:
    QNetworkAccessManager manager;
    QString _currentVersion;

    bool _isChecking;
};

#endif // RYUPDATECHECKER_H
