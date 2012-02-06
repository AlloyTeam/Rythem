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

QList<QiRuleGroup2> QiRuleManager2::parseConfigContent(QString json, bool remote){
	qDebug() << "[RuleManager] parsing config content";
	QList<QiRuleGroup2> groups;
	QScriptEngine engine;
	QScriptValue value = engine.evaluate("(" + json + ")");
	QScriptValueIterator groupsIt(value);
	while(groupsIt.hasNext()){
		groupsIt.next();
		qDebug() << "[RuleManager] group:" << groupsIt.name();
		//constructor the rule group
		QiRuleGroup2 group(groupsIt.name(), groupsIt.value().property("enable").toBool(), remote);
		QScriptValueIterator rulesIt(groupsIt.value().property("rules"));
		while(rulesIt.hasNext()){
			rulesIt.next();
			//constructor the rule
			QScriptValue r = rulesIt.value();
			int rType = r.property("type").toInt32();
			switch(rType){
			case COMPLEX_ADDRESS_REPLACE:
				//ignore complex address replace rule
				break;

			default:
				QiRule2 rule(
							r.property("name").toString(),
							rType,
							r.property("rule").property("pattern").toString(),
							r.property("rule").property("replace").toString(),
							remote
				);
				group.addRule(rule);
				qDebug() << rule.toJSON();
			}
		}
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
				QList<QiRuleGroup2> groups = parseConfigContent(content, false);
				localGroups.append(groups);
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
				groups << ("\"" + localGroups.at(i).groupName() + "\":" + localGroups.at(i).toJSON());
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

void QiRuleManager2::addRuleGroup(const QiRuleGroup2 &value, int index){
	QList<QiRuleGroup2> &list = value.isRemote() ? remoteGroups : localGroups;
	int existGroup = list.indexOf(value);
	if(existGroup != -1) list.removeAt(existGroup);
	if(index == -1) list.append(value);
	else list.insert(index, value);
}

QiRule2 QiRuleManager2::findMatchInGroups(ConnectionData_ptr connectionData, const QString &groupName, const QList<QiRuleGroup2> &list) const{
	int i, len = list.length();
	for(i=0; i<len; i++){
		QiRuleGroup2 group = list.at(i);
		if((groupName.length() && groupName == group.groupName()) || !groupName.length()){
			QiRule2 rule = group.match(connectionData);
			if(!rule.isNull()) return rule;
		}
	}
	return QiRule2();
}

QiRule2 QiRuleManager2::getMatchRule(ConnectionData_ptr connectionData, const QString &groupName) const{
	QiRule2 localMatch = findMatchInGroups(connectionData, groupName, localGroups);
	if(localMatch.isNull()) return findMatchInGroups(connectionData, groupName, remoteGroups);
	else return localMatch;
}

void QiRuleManager2::replace(ConnectionData_ptr connectionData) const{
	QiRule2 rule = getMatchRule(connectionData);
	replace(connectionData, rule);
}

void QiRuleManager2::replace(ConnectionData_ptr connectionData, const QiRule2 &rule) const{
	if(!rule.isNull()){
		//TODO replace connectionData content with specify rule
	}
}

void QiRuleManager2::onRemoteConfigLoaded(int id, bool error){
	qDebug() << "[RuleManager] remote config loaded" << id << error;
	if(!error){
		QString content = remoteConfigLoader.readAll();
		remoteGroups.append(parseConfigContent(content, true));
		emit remoteConfigLoaded();
	}
}
