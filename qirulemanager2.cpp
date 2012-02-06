#include "qirulemanager2.h"

QiRuleManager2::QiRuleManager2(QString localFile, QString host, QString address, QString path) :
	QObject(),
	localConfigFile(localFile),
	remoteHost(host),
	remoteAddress(address),
	remotePath(path)
{
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

void QiRuleManager2::parseConfigContent(QString json){

}

void QiRuleManager2::loadLocalConfig(){

}

void QiRuleManager2::loadRemoteConfig(){

}

void QiRuleManager2::loadConfig(){

}

void QiRuleManager2::saveLocalConfigChanges(){

}

void QiRuleManager2::addRuleGroup(const QiRuleGroup2 &value, int index){
	QList<QiRuleGroup2> &list = value.isRemote() ? remoteGroups : localGroups;
	int existGroup = list.indexOf(value);
	if(existGroup != -1) list.removeAt(existGroup);
	if(index == -1) list.append(value);
	else list.insert(index, value);
}

QiRule2 QiRuleManager2::findMatchInGroups(const QString &path, const QString &groupName, const QList<QiRuleGroup2> &list) const{
	int i, len = list.length();
	for(i=0; i<len; i++){
		QiRuleGroup2 group = list.at(i);
		if((groupName.length() && groupName == group.groupName()) || !groupName.length()){
			QiRule2 rule = group.match(path);
			if(!rule.isNull()) return rule;
		}
	}
	return QiRule2();
}

QiRule2 QiRuleManager2::getMatchRule(const QString &path, const QString &groupName) const{
	QiRule2 localMatch = findMatchInGroups(path, groupName, localGroups);
	if(localMatch.isNull()) return findMatchInGroups(path, groupName, remoteGroups);
	else return localMatch;
}

void QiRuleManager2::replace(ConnectionData_ptr &connectionData) const{

}

void QiRuleManager2::replace(ConnectionData_ptr &connectionData, const QiRule2 &rule) const{

}
