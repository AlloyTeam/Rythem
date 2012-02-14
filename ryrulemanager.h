#ifndef RYRULEMANAGER_H
#define RYRULEMANAGER_H

#include <QtCore>
#include <QHttp>
#include "ryrule.h"
#include "ryrulecomplexaddress.h"
#include "ryrulesimpleaddress.h"
#include "ryruleremotecontent.h"
#include "ryrulelocalfile.h"
#include "ryrulelocalfiles.h"
#include "ryrulelocaldir.h"
#include "ryrulegroup.h"
#include "ryconnection.h"
#include "rypipedata.h"

class RyRuleManager : public QObject
{
	Q_OBJECT
public:
        explicit RyRuleManager(QString localFile = "", QString host = "", QString address = "", QString path = "");
	void setLocalConfig(QString localFile, bool reload = false);
	void setRemoteConfig(QString host, QString addr, QString path, bool reload = false);
	QString remoteConfigURL();

	void loadLocalConfig();
	void loadRemoteConfig();
	void loadConfig();
	void saveLocalConfigChanges() const;

	void addRuleGroup(QiRuleGroup2 *value, int index = -1);
	QString configusToJSON(int tabCount = 0, bool localOnly = false) const;

	void getMatchRules(QList<RyRule *> *result, const QString &url, const QString &groupName = "") const;
        void replace(RyPipeData_ptr connectionData) const;
        void replace(RyPipeData_ptr connectionData, const QList<RyRule *> *rules) const;

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
	void findMatchInGroups(QList<RyRule *> *result, const QString &url, const QString &groupName, const QList<QiRuleGroup2 *> &list) const;
	
};

#endif // RYRULEMANAGER_H
