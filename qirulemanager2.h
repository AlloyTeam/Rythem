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
	QiRuleGroup2 getRuleGroup(const QString &name) const;
	QiRuleGroup2 getRuleGroupAt(int index) const;
	void updateRuleGroup(const QString &name, const QiRuleGroup2 &newValue);
	void updateRuleGroupAt(int index, const QiRuleGroup2 &newValue);
	void remoteRuleGroup(const QString &name);
	void remoteRuleGroupAt(int index);

	QiRule2 getMatchRule(const QString &path, const QString &groupName = "") const;
	void replace(ConnectionData_ptr &connectionData) const;
	void replace(ConnectionData_ptr &connectionData, const QiRule2 &rule) const;

	QString localConfigFile;
	QString remoteHost;
	QString remoteAddress;
	QString remotePath;
	
signals:
	
public slots:

private:
	QList<QiRuleGroup2> localGroups;
	QList<QiRuleGroup2> remoteGroups;
	
};

#endif // QIRULEMANAGER2_H
