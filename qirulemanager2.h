#ifndef QIRULEMANAGER2_H
#define QIRULEMANAGER2_H

#include <QtCore>
#include <QHttp>
#include "qirule2.h"
#include "qirulegroup2.h"
#include "qipipe.h"
#include "qiconnectiondata.h"

class QiRuleManager2 : public QObject
{
	Q_OBJECT
public:
	explicit QiRuleManager2(QString localFile = "", QString host = "", QString address = "", QString path = "");
	void setLocalConfig(QString localFile, bool reload = false);
	void setRemoteConfig(QString host, QString addr, QString path, bool reload = false);
	QString remoteConfigURL();

	void loadLocalConfig();
	void loadRemoteConfig();
	void loadConfig();
	void saveLocalConfigChanges() const;

	void addRuleGroup(const QiRuleGroup2 &value, int index = -1);

	QiRule2 getMatchRule(ConnectionData_ptr connectionData, const QString &groupName = "") const;
	void replace(ConnectionData_ptr connectionData) const;
	void replace(ConnectionData_ptr connectionData, const QiRule2 &rule) const;

	QString localConfigFile;
	QString remoteHost;
	QString remoteAddress;
	QString remotePath;
	QList<QiRuleGroup2> localGroups;
	QList<QiRuleGroup2> remoteGroups;
	
signals:
	void localConfigLoaded();
	void remoteConfigLoaded();
	
public slots:
	void onRemoteConfigLoaded(int id, bool error);

private:
	QHttp remoteConfigLoader;
	QList<QiRuleGroup2> parseConfigContent(QString json, bool remote = false);
	QiRule2 findMatchInGroups(ConnectionData_ptr connectionData, const QString &groupName, const QList<QiRuleGroup2> &list) const;
	
};

#endif // QIRULEMANAGER2_H
