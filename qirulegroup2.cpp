#include "qirulegroup2.h"

QiRuleGroup2::QiRuleGroup2() :
	QObject(),
	_groupName(""),
	_isEnable(false),
	_isRemote(false)
{
}

QiRuleGroup2::QiRuleGroup2(const QiRuleGroup2 &group) :
	QObject(),
	_groupName(group.groupName()),
	_isEnable(group.isEnable()),
	_isRemote(group.isRemote())
{
}

QiRuleGroup2::QiRuleGroup2(QString name, bool enable, bool remote) :
	QObject(),
	_groupName(name),
	_isEnable(enable),
	_isRemote(remote)
{
}

QiRuleGroup2 QiRuleGroup2::operator =(const QiRuleGroup2 &group){
	this->update(group.groupName(), group.isEnable(), group.isRemote());
	return *this;
}

bool QiRuleGroup2::operator ==(const QiRuleGroup2 &group) const{
	return this->groupName() == group.groupName();
}

void QiRuleGroup2::update(QString name, bool enable, bool remote){
	if(!isRemote()){
		_groupName = name;
		_isEnable = enable;
		_isRemote = remote;
		emit changed();
	}
}

void QiRuleGroup2::update(const QiRuleGroup2 &group){
	update(group.groupName(), group.isEnable(), group.isRemote());
}

void QiRuleGroup2::addRule(const QiRule2 &value, int index){
	//remove existed rule with the same name first
	int existIndex = _rules.indexOf(value);
	if(existIndex != -1) this->removeRuleAt(existIndex);
	//append/insert the new rule
	if(index == -1) _rules.append(value);
	else _rules.insert(index, value);
	emit changed();
}

int QiRuleGroup2::length() const{
	return _rules.length();
}

int QiRuleGroup2::getRuleIndex(const QString name) const{
	int i, length = _rules.length();
	for(i=0; i<length; i++){
		if(_rules.at(i).name() == name){
			return i;
		}
	}
	return -1;
}

QiRule2 QiRuleGroup2::getRule(const QString name) const{
	int index = this->getRuleIndex(name);
	return _rules.value(index);
}

QiRule2 QiRuleGroup2::getRuleAt(const int index) const{
	return _rules.value(index);
}

void QiRuleGroup2::updateRule(const QString name, const QiRule2 &newValue){
	int index = this->getRuleIndex(name);
	if(index != -1){
		this->updateRuleAt(index, newValue);
	}
}

void QiRuleGroup2::updateRuleAt(const int index, const QiRule2 &newValue){
	QiRule2 rule = _rules.at(index);
	rule.update(newValue);
	emit changed();
}

void QiRuleGroup2::removeRule(const QString name){
	int index = this->getRuleIndex(name);
	if(index != -1){
		this->removeRuleAt(index);
		emit changed();
	}
}

void QiRuleGroup2::removeRuleAt(const int index){
	_rules.removeAt(index);
	emit changed();
}

//判斷一下返回結果的isNull()
QiRule2 QiRuleGroup2::match(const QString path) const{
	int i, length = _rules.length();
	for(i=0; i<length; i++){
		if(_rules.at(i).match(path)){
			return _rules.at(i);
		}
	}
	return QiRule2();
}

QString QiRuleGroup2::toJSON() const{
	QStringList list;
	int i, length = _rules.length();
	for(i=0; i<length; i++){
		list << _rules.at(i).toJSON();
	}
	QString result;
	QTextStream(&result) << "{\"enable\":" << isEnable() << ", \"rules\":[" << list.join(", ") << "]}";
	return result;
}

bool QiRuleGroup2::isNull() const{
	return !_groupName.length();
}
bool QiRuleGroup2::isEnable() const{
	return _isEnable;
}
bool QiRuleGroup2::isRemote() const{
	return _isRemote;
}
QString QiRuleGroup2::groupName() const{
	return _groupName;
}
