#ifndef QIRULEMANAGER2_H
#define QIRULEMANAGER2_H

#include <QtCore>
#include <QHttp>
#include "qirule2.h"
#include "qirulecomplexaddress.h"
#include "qirulesimpleaddress.h"
#include "qiruleremotecontent.h"
#include "qirulelocalfile.h"
#include "qirulelocalfiles.h"
#include "qirulelocaldir.h"
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

	void addRuleGroup(QiRuleGroup2 *value, int index = -1);
	QString configusToJSON(int tabCount = 0, bool localOnly = false) const;

	void getMatchRules(QList<QiRule2 *> *result, const QString &url, const QString &groupName = "") const;
	void replace(ConnectionData_ptr connectionData) const;
	void replace(ConnectionData_ptr connectionData, const QList<QiRule2 *> *rules) const;

	QString localConfigFile;
	QString remoteHost;
	QString remoteAddress;
	QString remotePath;
	QList<QiRuleGroup2 *> localGroups;
	QList<QiRuleGroup2 *> remoteGroups;
	
signals:
	void localConfigLoaded();
	void remoteConfigLoaded();
	
public slots:
	void onRemoteConfigLoaded(int id, bool error);

private:
	QHttp remoteConfigLoader;
	void parseConfigContent(QList<QiRuleGroup2 *> *result, QString json, bool remote = false);
	void findMatchInGroups(QList<QiRule2 *> *result, const QString &url, const QString &groupName, const QList<QiRuleGroup2 *> &list) const;
	
};

#endif // QIRULEMANAGER2_H
