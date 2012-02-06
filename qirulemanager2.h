#ifndef QIRULEMANAGER2_H
#define QIRULEMANAGER2_H

#include <QtCore>
#include "qirule2.h"
#include "qirulegroup2.h"
#include "qipipe.h"

class QiRuleManager2 : public QObject
{
	Q_OBJECT
public:
	explicit QiRuleManager2(QString localFile = "", QString host = "", QString address = "", QString path = "");
	void setLocalConfig(QString localFile, bool reload = false);
	void setRemoteConfig(QString host, QString addr, QString path, bool reload = false);

	void loadLocalConfig();
	void loadRemoteConfig();
	void loadConfig();
	void saveLocalConfigChanges();

	void addRuleGroup(const QiRuleGroup2 &value, int index = -1);

	QiRule2 getMatchRule(const QString &path, const QString &groupName = "") const;
	void replace(ConnectionData_ptr &connectionData) const;
	void replace(ConnectionData_ptr &connectionData, const QiRule2 &rule) const;

	QString localConfigFile;
	QString remoteHost;
	QString remoteAddress;
	QString remotePath;
	QList<QiRuleGroup2> localGroups;
	QList<QiRuleGroup2> remoteGroups;
	
signals:
	void changed();
	
public slots:

private:
	void parseConfigContent(QString json);
	QiRule2 findMatchInGroups(const QString &path, const QString &groupName, const QList<QiRuleGroup2> &list) const;
	
};

#endif // QIRULEMANAGER2_H
