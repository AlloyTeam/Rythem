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

void QiRuleManager2::parseConfigContent(QList<QiRuleGroup2 *> *result, QString json, bool remote){
	qDebug() << "[RuleManager] parsing config content";
	QScriptEngine engine;
	QScriptValue value = engine.evaluate("(" + json + ")");
	QScriptValueIterator groupsIt(value.property("groups"));
	while(groupsIt.hasNext()){
		groupsIt.next();
		//constructor the rule group
		QString groupName = groupsIt.value().property("name").toString();
		bool groupEnable = groupsIt.value().property("enable").toBool();
		QiRuleGroup2 *group = new QiRuleGroup2(groupName, groupEnable, remote);
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
		result->append(group);
	}
}

void QiRuleManager2::loadLocalConfig(){
	if(localConfigFile.length()){
		QFile file(localConfigFile);
		QFileInfo fileInfo(file);
		if(fileInfo.exists() && fileInfo.isFile()){
			if(file.open(QIODevice::ReadOnly)){
				QTextStream stream(&file);
				QString content = stream.readAll();
				QList<QiRuleGroup2 *> groups;
				parseConfigContent(&groups, content, false);
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
		if(!fileInfo.isDir()){
			if(file.open(QIODevice::WriteOnly)){
				QTextStream stream(&file);
				stream << this->configusToJSON(0, true);
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
		qWarning() << "[RuleManager] you can't save config to a directory";
	}
}

QString QiRuleManager2::configusToJSON(int tabCount, bool localOnly) const{
	QString tabs = QString("\t").repeated(tabCount);
	QStringList groups;
	int i, length = localGroups.length();
	for(i=0; i<length; i++){
		groups << localGroups.at(i)->toJSON(tabCount + 1, true);
	}
	if(!localOnly){
		length = remoteGroups.length();
		for(i=0; i<length; i++){
			groups << remoteGroups.at(i)->toJSON(tabCount + 1, true);
		}
	}
	QString result;
	QTextStream(&result) << tabs << "{\n"
						 << tabs << groups.join(",\n") << "\n"
						 << tabs << "}";
	return result;
}

void QiRuleManager2::addRuleGroup(QiRuleGroup2 *value, int index){
	QList<QiRuleGroup2 *> &list = value->isRemote() ? remoteGroups : localGroups;
	int existGroup = list.indexOf(value);
	if(existGroup != -1) list.removeAt(existGroup);
	if(index == -1) list.append(value);
	else list.insert(index, value);
}

void QiRuleManager2::findMatchInGroups(QList<QiRule2 *> *result, const QString &url, const QString &groupName, const QList<QiRuleGroup2 *> &list) const{
	int i, len = list.length();
	for(i=0; i<len; i++){
		QiRuleGroup2 *group = list.at(i);
		if((groupName.length() && groupName == group->groupName()) || !groupName.length()){
			QList<QiRule2 *> groupMatch;
			group->match(&groupMatch, url);
			if(groupMatch.length()){
				result->append(groupMatch);
			}
		}
	}
}

void QiRuleManager2::getMatchRules(QList<QiRule2 *> *result, const QString &url, const QString &groupName) const{
	qDebug() << "[RuleManager] finding match rule ...";
	findMatchInGroups(result, url, groupName, localGroups);
	findMatchInGroups(result, url, groupName, remoteGroups);
}

void QiRuleManager2::replace(ConnectionData_ptr connectionData) const{
	QList<QiRule2 *> rules;
	getMatchRules(&rules, connectionData->fullUrl);
	replace(connectionData, &rules);
}

void QiRuleManager2::replace(ConnectionData_ptr connectionData, const QList<QiRule2 *> *rules) const{
	int length = rules->length();
	if(length){
		bool hostReplaced, otherReplaced;
		for(int i=0; i<length; i++){
			//get the next rule and its type
			QiRule2 *rule = rules->at(i);
			bool isHostReplaceRule = (rule->type() == SIMPLE_ADDRESS_REPLACE || rule->type() == COMPLEX_ADDRESS_REPLACE);

			//replace host
			if(isHostReplaceRule && !hostReplaced){
				rule->replace(connectionData);
				hostReplaced = true;
			}
			//replace content
			else if(!isHostReplaceRule && !otherReplaced){
				rule->replace(connectionData);
				otherReplaced = true;
			}

			//host and content needs only to replace once
			if(hostReplaced && otherReplaced) return;
		}
	}
}

void QiRuleManager2::onRemoteConfigLoaded(int id, bool error){
	qDebug() << "[RuleManager] remote config loaded" << id << error;
	if(!error){
		QString content = remoteConfigLoader.readAll();
		QList<QiRuleGroup2 *> groups;
		parseConfigContent(&groups, content, true);
		remoteGroups.append(groups);
		emit remoteConfigLoaded();
	}
}
