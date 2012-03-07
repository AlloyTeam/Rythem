#include "ryrulegroup.h"

RyRuleGroup::RyRuleGroup() :
	QObject(),
	_groupName(""),
	_isEnable(false),
	_isRemote(false)
{
}

RyRuleGroup::RyRuleGroup(const RyRuleGroup &group) :
	QObject(),
	//copy group info
	_groupName(group.groupName()),
	_isEnable(group.isEnable()),
	_isRemote(group.isRemote())
{
	//copy rules
	int i, len = group.length();
	for(i=0; i<len; i++){
		addRule(group.getRuleAt(i));
	}
}

RyRuleGroup::RyRuleGroup(QString name, bool enable, bool remote) :
	QObject(),
	_groupName(name),
	_isEnable(enable),
	_isRemote(remote)
{
}

RyRuleGroup::~RyRuleGroup(){
	int i, len = _rules.length();
	for(i=len-1; i>=0; i--){
		this->removeRuleAt(i);
	}
}

RyRuleGroup RyRuleGroup::operator =(const RyRuleGroup &group){
	this->update(group.groupName(), group.isEnable(), group.isRemote());
	return *this;
}

bool RyRuleGroup::operator ==(const RyRuleGroup &group) const{
	return this->groupName() == group.groupName();
}

void RyRuleGroup::update(QString name, bool enable, bool remote){
	if(!isRemote()){
		_groupName = name;
		_isEnable = enable;
		_isRemote = remote;
		emit changed();
	}
}

void RyRuleGroup::update(const RyRuleGroup &group){
	update(group.groupName(), group.isEnable(), group.isRemote());
}

void RyRuleGroup::addRule(RyRule *value, int index){
	//remove existed rule with the same name first
	int existIndex = _rules.indexOf(value);
	if(existIndex != -1) this->removeRuleAt(existIndex);
	//append/insert the new rule
	if(index == -1) _rules.append(value);
	else _rules.insert(index, value);
	emit changed();
}

int RyRuleGroup::length() const{
	return _rules.length();
}

int RyRuleGroup::getRuleIndex(const QString name) const{
	int i, length = _rules.length();
	for(i=0; i<length; i++){
		if(_rules.at(i)->name() == name){
			return i;
		}
	}
	return -1;
}

RyRule *RyRuleGroup::getRule(const QString name) const{
	int index = this->getRuleIndex(name);
	return _rules.value(index);
}

RyRule *RyRuleGroup::getRuleAt(const int index) const{
	return _rules.at(index);
}

void RyRuleGroup::updateRule(const QString name, const RyRule &newValue){
	int index = this->getRuleIndex(name);
	if(index != -1){
		this->updateRuleAt(index, newValue);
	}
}

void RyRuleGroup::updateRuleAt(const int index, const RyRule &newValue){
	RyRule *rule = _rules.at(index);
	rule->update(newValue);
	emit changed();
}

void RyRuleGroup::removeRule(const QString name){
	int index = this->getRuleIndex(name);
	if(index != -1){
		this->removeRuleAt(index);
		emit changed();
	}
}

void RyRuleGroup::removeRuleAt(const int index){
	RyRule *rule = _rules.at(index);
	delete rule;
	_rules.removeAt(index);
	emit changed();
}

//判斷一下返回結果的isNull()
void RyRuleGroup::match(QList<RyRule *> *result, const QString &url) const{
    if(this->isEnable()){
        int i, length = _rules.length();
        for(i=0; i<length; i++){
            RyRule *rule = _rules.at(i);
                    //qDebug()<<rule->toJSON();
            if(rule->isEnable() && rule->match(url)){
                result->append(rule);
            }
        }
    }
}

QString RyRuleGroup::toJSON(int tabCount, bool withName) const{
	QString tabs = QString("\t").repeated(tabCount);
	QStringList list;
	int i, length = _rules.length();
	for(i=0; i<length; i++){
		list << _rules.at(i)->toJSON(tabCount + 2);
	}
	QString result;
        QString name = withName ? ("\"" + groupName() + "\": ") : "";
	QTextStream(&result) << tabs << name << "{\n"
							 << tabs << "\t\"name\":"<<"\""<<groupName()<<"\",\n"
                             << tabs << "\t\"enable\":" << isEnable() << ",\n"
                             << tabs << "\t\"rules\":[\n" << list.join(",\n") << "\n"
                             << tabs << "\t]\n"
                             << tabs << "}";
	return result;
}

bool RyRuleGroup::isNull() const{
	return !_groupName.length();
}
bool RyRuleGroup::isEnable() const{
	return _isEnable;
}
bool RyRuleGroup::isRemote() const{
	return _isRemote;
}
QString RyRuleGroup::groupName() const{
	return _groupName;
}
