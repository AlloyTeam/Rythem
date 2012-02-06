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

void parseConfigContent(QString json){

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

}

QiRuleGroup2 QiRuleManager2::getRuleGroup(const QString &name) const{

}

QiRuleGroup2 QiRuleManager2::getRuleGroupAt(int index) const{

}

void QiRuleManager2::updateRuleGroup(const QString &name, const QiRuleGroup2 &newValue){

}

void QiRuleManager2::updateRuleGroupAt(int index, const QiRuleGroup2 &newValue){

}

void QiRuleManager2::remoteRuleGroup(const QString &name){

}

void QiRuleManager2::remoteRuleGroupAt(int index){

}

QiRule2 QiRuleManager2::getMatchRule(const QString &path, const QString &groupName) const{

}

void QiRuleManager2::replace(ConnectionData_ptr &connectionData) const{

}

void QiRuleManager2::replace(ConnectionData_ptr &connectionData, const QiRule2 &rule) const{

}
