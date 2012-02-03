#include "qirulegroup2.h"

QiRuleGroup2::QiRuleGroup2(QString name, bool enable, bool remote) :
	QObject(),
	_groupName(name),
	_isEnable(enable),
	_isRemote(remote)
{
}

void QiRuleGroup2::update(QString name, bool enable, bool remote){
	_groupName = name;
	_isEnable = enable;
	_isRemote = remote;
	emit changed();
}

void QiRuleGroup2::addRule(const QiRule2 value, int index){
	//remove existed rule with the same name first
	int existIndex = this->getRuleIndex(value.name());
	if(existIndex != -1) this->removeRuleAt(existIndex);
	//append/insert the new rule
	if(index == -1) _rules.push_back(value);
	else _rules.insert(index, value);
	emit changed();
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

void QiRuleGroup2::updateRule(const QString name, const QiRule2 newValue){
	int index = this->getRuleIndex(name);
	if(index != -1){
		this->updateRuleAt(index, newValue);
		emit changed();
	}
}

void QiRuleGroup2::updateRuleAt(const int index, const QiRule2 newValue){
	_rules.replace(index, newValue);
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
	int i, length = _rules.length();
	QStringList result;
	for(i=0; i<length; i++){
		result << _rules.at(i).toJSON();
	}
	return QString("{\"enable\":%1, \"rules\":[%2]}")
			.arg(this->isEnable())
			.arg(result.join(", "));
}

bool QiRuleGroup2::isEnable() const{
	return _isEnable;
}
bool QiRuleGroup2::isRemote() const{
	return _isRemote;
}
