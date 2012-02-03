#include "qirule2.h"

QiRule2::QiRule2(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QObject(),
	_name(name),
	_type(type),
	_pattern(pattern),
	_replacement(replacement),
	_isEnable(enable),
	_isRemote(remote)
{
}

bool QiRule2::match(QString path){
	//TODO check the path with this rule
	return false;
}

QString QiRule2::toJSON(){
	return QString("{\"name\":\"%1\", \"type\":%2, \"enable\":%3, \"pattern\":\"%4\", \"replace\"\"%5\"}")
			.arg(_name)
			.arg(_type)
			.arg(_isEnable)
			.arg(_pattern)
			.arg(_replacement);
}

bool QiRule2::isRemoteRule(){
	return _isRemote;
}
bool QiRule2::isEnable(){
	return _isEnable;
}
QString QiRule2::name(){
	return _name;
}
int QiRule2::type(){
	return _type;
}
QString QiRule2::pattern(){
	return _pattern;
}
QString QiRule2::replacement(){
	return _replacement;
}

void QiRule2::update(QString name, int type, QString pattern, QString replacement, bool enable, bool remote){
	if(!_isRemote){
		_isRemote = remote;
		_isEnable = enable;
		_name = name;
		_type = type;
		_pattern = pattern;
		_replacement = replacement;
		emit changed();
	}
}

void QiRule2::setIsRemoteRule(bool value){
	if(!_isRemote){
		_isRemote = value;
		emit changed();
	}
}
void QiRule2::setEnable(bool value){
	if(!_isRemote){
		_isEnable = value;
		emit changed();
	}
}
void QiRule2::setName(QString value){
	if(!_isRemote){
		_name = value;
		emit changed();
	}
}
void QiRule2::setType(int value){
	if(!_isRemote){
		_type = value;
		emit changed();
	}
}
void QiRule2::setPattern(QString value){
	if(!_isRemote){
		_pattern = value;
		emit changed();
	}
}
void QiRule2::setReplacement(QString value){
	if(!_isRemote){
		_replacement = value;
		emit changed();
	}
}
