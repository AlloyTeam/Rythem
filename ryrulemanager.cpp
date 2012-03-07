#include "ryrulemanager.h"
#include "ryrulegroup.h"
#include <QtScript>

extern RyRuleManager *_globalManager;

//Q_GLOBAL_STATIC(RyRuleManager, ruleManager)
RyRuleManager *RyRuleManager::instance(){
    return _globalManager;
}


RyRuleManager::RyRuleManager(QString localFile, QString host, QString address, QString path) :
	QObject(),
	localConfigFile(localFile),
	remoteHost(host),
	remoteAddress(address),
	remotePath(path)
{
	connect(&remoteConfigLoader, SIGNAL(requestFinished(int,bool)), this, SLOT(onRemoteConfigLoaded(int,bool)));
}

RyRuleManager::~RyRuleManager(){
	this->saveLocalConfigChanges();
	qDebug() << "[RuleManager] i am dead";
}

void RyRuleManager::setLocalConfig(QString localFile, bool reload){
	localConfigFile = localFile;
	if(reload) loadLocalConfig();
}

void RyRuleManager::setRemoteConfig(QString host, QString addr, QString path, bool reload){
	remoteHost = host;
	remoteAddress = addr;
	remotePath = path;
	if(reload) loadRemoteConfig();
}

QString RyRuleManager::remoteConfigURL(){
	QString host = remoteAddress.length() ? remoteAddress : remoteHost;
	return "http://" + host + remotePath;
}

void RyRuleManager::parseConfigContent(QList<RyRuleGroup *> *result, QString json, bool remote){
    //qDebug() << "[RuleManager] parsing config content"<<json;
	QScriptEngine engine;
	QScriptValue value = engine.evaluate("(" + json + ")");
	QScriptValueIterator groupsIt(value.property("groups"));
	while(groupsIt.hasNext()){
		groupsIt.next();
		//constructor the rule group
        QString groupName = groupsIt.value().property("name").toString();
        //qDebug()<<"group:"<<groupName;
        if(groupName.isEmpty()){
            continue;
        }
		if(!groupName.length()) continue;
		bool groupEnable = groupsIt.value().property("enable").toBool();
        RyRuleGroup *group = new RyRuleGroup(groupName, groupEnable, remote);
		QScriptValueIterator rulesIt(groupsIt.value().property("rules"));
		while(rulesIt.hasNext()){
                        /*
                {
                        "name":"complex address example",
                        "type":1,
                        "enable":true,
                        "rule":[
                                {"pattern":"http://abc.com/a","replace":"123.com"},
                                {"pattern":"http://abc.com/b","replace":"456.com"}
                        ]
                }
                        */
			rulesIt.next();
			//constructor the rule
			RyRule *rule = 0;
			QScriptValue r = rulesIt.value();
            QString rName = r.property("name").toString();
			bool rEnable = r.property("enable").toBool();
			int rType = r.property("type").toInt32();

			QScriptValue ruleObj = r.property("rule");
            QString rPattern = ruleObj.property("pattern").toString();
            QString rReplace = ruleObj.property("replace").toString();
/*
                        QScriptValueIterator rulesIt2(r.property("rule"));
                        QString rPattern;
                        QString rReplace;
                        while(rulesIt2.hasNext()){
                            rulesIt2.next();
                            if(!rPattern.isEmpty())break;
                            rPattern = rulesIt2.value().property("pattern").toString();
                            rReplace = rulesIt2.value().property("replace").toString();
                        }
						*/
			switch(rType){
			case COMPLEX_ADDRESS_REPLACE:
				//ignore complex address replace rule
				break;
			case SIMPLE_ADDRESS_REPLACE:
                                rule = new RyRuleSimpleAddress(rName, rType, rPattern, rReplace, rEnable, remote);
				break;
			case REMOTE_CONTENT_REPLACE:
                                rule = new RyRuleRemoteContent(rName, rType, rPattern, rReplace, rEnable, remote);
				break;
			case LOCAL_FILE_REPLACE:
                                rule = new RyRuleLocalFile(rName, rType, rPattern, rReplace, rEnable, remote);
				break;
			case LOCAL_FILES_REPLACE:
                                rule = new RyRuleLocalFiles(rName, rType, rPattern, rReplace, rEnable, remote);
				break;
			case LOCAL_DIR_REPLACE:
                                rule = new RyRuleLocalDir(rName, rType, rPattern, rReplace, rEnable, remote);
				break;
			}
			if(rule){
				group->addRule(rule);
			}
		}
		result->append(group);
	}
}

void RyRuleManager::loadLocalConfig(){
	if(localConfigFile.length()){
		QFile file(localConfigFile);
		QFileInfo fileInfo(file);
		if(fileInfo.exists() && fileInfo.isFile()){
			if(file.open(QIODevice::ReadOnly)){
                                QTextStream stream(&file);
                                QString content = stream.readAll();
                                file.close();
                                bool isSuccess = setLocalConfigContent(content,true);
                                if(isSuccess){
                                    emit localConfigLoaded();
                                }
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

bool RyRuleManager::setLocalConfigContent(QString content,bool dontSave){
    //qDebug() << "setLocalConfigContent" << content << dontSave;
    //qDebug()<<"set local config content"<<content;
    QListIterator<RyRuleGroup *> it(localGroups);
	while(it.hasNext()){
        RyRuleGroup *group = it.next();
		delete group;
	}
	localGroups.clear();
	parseConfigContent(&localGroups, content, false);
    if(!dontSave){
        //qDebug()<<"toSave";
        saveLocalConfigChanges();
    }
    return true;
}

void RyRuleManager::loadRemoteConfig(){
	if(remoteHost.length() || remoteAddress.length()){
		remoteConfigLoader.clearPendingRequests();
		remoteConfigLoader.abort();
		remoteConfigLoader.get(remoteConfigURL());
	}
}

void RyRuleManager::loadConfig(){
	loadLocalConfig();
	loadRemoteConfig();
}

void RyRuleManager::saveLocalConfigChanges() const{
    //qDebug()<<"save:"
    //        <<localConfigFile;
	if(localConfigFile.length()){
		QFile file(localConfigFile);
		QFileInfo fileInfo(file);
		if(!fileInfo.isDir()){
			if(file.open(QIODevice::WriteOnly)){
				QTextStream stream(&file);
                QString content = this->configusToJSON(0, true);
                stream << content.replace("\\","\\\\");
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
                }else{
                    qWarning() << "[RuleManager] you can't save config to a directory";
                }
	}
}

QString RyRuleManager::configusToJSON(int tabCount, bool localOnly) const{
	QString tabs = QString("\t").repeated(tabCount);
	QStringList groups;
	int i, length = localGroups.length();
	for(i=0; i<length; i++){
				groups << localGroups.at(i)->toJSON(tabCount + 1, false);
	}
	if(!localOnly){
		length = remoteGroups.length();
		for(i=0; i<length; i++){
                        groups << remoteGroups.at(i)->toJSON(tabCount + 1, false);
		}
	}
	QString result;
        QTextStream(&result) << tabs << "{\"groups\":[\r\n"
												 << tabs << groups.join(",\r\n") << "\r\n"
                                                 << tabs << "]}";
        //qDebug()<<"configToJSon"<<result;
	return result;
}

void RyRuleManager::addRuleGroup(RyRuleGroup *value, int index){
    QList<RyRuleGroup *> &list = value->isRemote() ? remoteGroups : localGroups;
	int existGroup = list.indexOf(value);
	if(existGroup != -1) list.removeAt(existGroup);
	if(index == -1) list.append(value);
	else list.insert(index, value);
}

void RyRuleManager::findMatchInGroups(QList<RyRule *> *result, const QString &url, const QString &groupName, const QList<RyRuleGroup *> &list) const{
	int i, len = list.length();
	for(i=0; i<len; i++){
        RyRuleGroup *group = list.at(i);
		if((groupName.length() && groupName == group->groupName()) || !groupName.length()){
			QList<RyRule *> groupMatch;
			group->match(&groupMatch, url);
			if(groupMatch.length()){
				result->append(groupMatch);
			}
		}
	}
}

void RyRuleManager::getMatchRules(QList<RyRule *> *result, const QString &url, const QString &groupName) const{
    //qDebug() << "[RuleManager] finding match rule ...";
	findMatchInGroups(result, url, groupName, localGroups);
	findMatchInGroups(result, url, groupName, remoteGroups);
}

void RyRuleManager::replace(RyPipeData_ptr connectionData) const{
	QList<RyRule *> rules;
	getMatchRules(&rules, connectionData->fullUrl);
	replace(connectionData, &rules);
}

void RyRuleManager::replace(RyPipeData_ptr connectionData, const QList<RyRule *> *rules) const{
	int length = rules->length();
	if(length){
		bool hostReplaced, otherReplaced;
		for(int i=0; i<length; i++){
			//get the next rule and its type
			RyRule *rule = rules->at(i);
            if(rule->isEnable()){
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
}

void RyRuleManager::onRemoteConfigLoaded(int id, bool error){
	qDebug() << "[RuleManager] remote config loaded" << id << error;
	if(!error){
		QString content = remoteConfigLoader.readAll();
        QList<RyRuleGroup *> groups;
		parseConfigContent(&groups, content, true);
		remoteGroups.append(groups);
		emit remoteConfigLoaded();
	}
}
