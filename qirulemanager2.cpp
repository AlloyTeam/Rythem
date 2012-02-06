#include "qirulemanager2.h"
#include <QtScript>

QiRuleManager2::QiRuleManager2(QString localFile, QString host, QString address, QString path) :
	QObject(),
	localConfigFile(localFile),
	remoteHost(host),
	remoteAddress(address),
	remotePath(path)
{
	connect(&remoteConfigLoader, SIGNAL(requestFinished(int,bool)), this, SLOT(onRemoteConfigLoaded(int,bool)));
}

void QiRuleManager2::setLocalConfig(QString localFile, bool reload){
	localConfigFile = localFile;
	if(reload) loadLocalConfig();
}

void QiRuleManager2::setRemoteConfig(QString host, QString addr, QString path, bool reload){
	remoteHost = host;
	remoteAddress = addr;
	remotePath = path;
	if(reload) loadRemoteConfig();
}

QString QiRuleManager2::remoteConfigURL(){
	QString host = remoteAddress.length() ? remoteAddress : remoteHost;
	return "http://" + host + remotePath;
}

QList<QiRuleGroup2 *> *QiRuleManager2::parseConfigContent(QString json, bool remote){
	qDebug() << "[RuleManager] parsing config content";
	QList<QiRuleGroup2 *> *groups = new QList<QiRuleGroup2 *>();
	QScriptEngine engine;
	QScriptValue value = engine.evaluate("(" + json + ")");
	QScriptValueIterator groupsIt(value);
	while(groupsIt.hasNext()){
		groupsIt.next();
		//constructor the rule group
		QiRuleGroup2 *group = new QiRuleGroup2(groupsIt.name(), groupsIt.value().property("enable").toBool(), remote);
		QScriptValueIterator rulesIt(groupsIt.value().property("rules"));
		while(rulesIt.hasNext()){
			rulesIt.next();
			//constructor the rule
			QiRule2 *rule = 0;
			QScriptValue r = rulesIt.value();
			QString rName = r.property("name").toString();
			bool rEnable = r.property("enable").toBool();
			int rType = r.property("type").toInt32();
			QString rPattern = r.property("rule").property("pattern").toString();
			QString rReplace = r.property("rule").property("replace").toString();
			switch(rType){
			case COMPLEX_ADDRESS_REPLACE:
				//ignore complex address replace rule
				break;
			case SIMPLE_ADDRESS_REPLACE:
				rule = new QiRuleSimpleAddress(rName, rType, rPattern, rReplace, rEnable, remote);
				break;
			case REMOTE_CONTENT_REPLACE:
				rule = new QiRuleRemoteContent(rName, rType, rPattern, rReplace, rEnable, remote);
				break;
			case LOCAL_FILE_REPLACE:
				rule = new QiRuleLocalFile(rName, rType, rPattern, rReplace, rEnable, remote);
				break;
			case LOCAL_FILES_REPLACE:
				rule = new QiRuleLocalFiles(rName, rType, rPattern, rReplace, rEnable, remote);
				break;
			case LOCAL_DIR_REPLACE:
				rule = new QiRuleLocalDir(rName, rType, rPattern, rReplace, rEnable, remote);
				break;
			}
			if(rule){
				group->addRule(rule);
			}
		}
		groups->append(group);
	}
	return groups;
}

void QiRuleManager2::loadLocalConfig(){
	if(localConfigFile.length()){
		QFile file(localConfigFile);
		QFileInfo fileInfo(file);
		if(fileInfo.exists() && fileInfo.isFile()){
			if(file.open(QIODevice::ReadOnly)){
				QTextStream stream(&file);
				QString content = stream.readAll();
				QList<QiRuleGroup2 *> *groups = parseConfigContent(content, false);
				localGroups.append(*groups);
				file.close();
				emit localConfigLoaded();
			}
			else{
				qWarning() << "[RuleManager] local config file open fail (read only)";
			}
		}
		else{
			qWarning() << "[RuleManager] local config file does not exist or is not a file";
		}
	}
}

void QiRuleManager2::loadRemoteConfig(){
	if(remoteHost.length() || remoteAddress.length()){
		remoteConfigLoader.clearPendingRequests();
		remoteConfigLoader.abort();
		remoteConfigLoader.get(remoteConfigURL());
	}
}

void QiRuleManager2::loadConfig(){
	loadLocalConfig();
	loadRemoteConfig();
}

void QiRuleManager2::saveLocalConfigChanges() const{
	if(localConfigFile.length()){
		QFile file(localConfigFile);
		QFileInfo fileInfo(file);
		if(file.open(QIODevice::WriteOnly)){
			QStringList groups;
			int i, length = localGroups.length();
			for(i=0; i<length; i++){
				groups << ("\"" + localGroups.at(i)->groupName() + "\":" + localGroups.at(i)->toJSON());
			}

			QTextStream stream(&file);
			stream << "{" << groups.join(", ") << "}";
			if(file.flush()){
				qDebug() << "[RuleManager] write local config done to" << localConfigFile;
			}
			else{
				qWarning() << "[RuleManager] writing local config fail";
			}
			file.close();
		}
		else{
			qWarning() << "[RuleManager] local config file open fail (write only)";
		}
	}
}

void QiRuleManager2::addRuleGroup(QiRuleGroup2 *value, int index){
	QList<QiRuleGroup2 *> &list = value->isRemote() ? remoteGroups : localGroups;
	int existGroup = list.indexOf(value);
	if(existGroup != -1) list.removeAt(existGroup);
	if(index == -1) list.append(value);
	else list.insert(index, value);
}

QiRule2 *QiRuleManager2::findMatchInGroups(const QString &url, const QString &groupName, const QList<QiRuleGroup2 *> &list) const{
	int i, len = list.length();
	for(i=0; i<len; i++){
		QiRuleGroup2 *group = list.at(i);
		if((groupName.length() && groupName == group->groupName()) || !groupName.length()){
			QiRule2 *rule = group->match(url);
			if(rule && !rule->isNull()) return rule;
		}
	}
	return 0;
}

QiRule2 *QiRuleManager2::getMatchRule(const QString &url, const QString &groupName) const{
	qDebug() << "[RuleManager] finding match rule ...";
	QiRule2 *localMatch = findMatchInGroups(url, groupName, localGroups);
	if(!localMatch) return findMatchInGroups(url, groupName, remoteGroups);
	else return localMatch;
}

void QiRuleManager2::replace(ConnectionData_ptr connectionData) const{
	QiRule2 *rule = getMatchRule(connectionData->fullUrl);
	replace(connectionData, rule);
}

void QiRuleManager2::replace(ConnectionData_ptr connectionData, const QiRule2 *rule) const{
	if(!rule->isNull()){
		rule->replace(connectionData);
	}
}

void QiRuleManager2::onRemoteConfigLoaded(int id, bool error){
	qDebug() << "[RuleManager] remote config loaded" << id << error;
	if(!error){
		QString content = remoteConfigLoader.readAll();
		remoteGroups.append(*parseConfigContent(content, true));
		emit remoteConfigLoaded();
	}
}
