#include "qirule2.h"

QiRule2::QiRule2() :QObject(){

}

QiRule2::QiRule2(const QiRule2 &rule) :
	QObject(),
	_name(rule.name()),
	_type(rule.type()),
	_pattern(rule.pattern()),
	_replacement(rule.replacement()),
	_isEnable(rule.isEnable()),
	_isRemote(rule.isRemoteRule())
{

}

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

QiRule2 QiRule2::operator =(const QiRule2 &rule){
	this->update(rule.name(), rule.type(), rule.pattern(), rule.replacement(), rule.isEnable(), rule.isRemoteRule());
	return *this;
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

bool QiRule2::match(const QString path) const{
	//TODO check the path with this rule
	return false;
}

QString QiRule2::toJSON() const{
	return QString("{\"name\":\"%1\", \"type\":%2, \"enable\":%3, \"pattern\":\"%4\", \"replace\"\"%5\"}")
			.arg(_name)
			.arg(_type)
			.arg(_isEnable)
			.arg(_pattern)
			.arg(_replacement);
}

bool QiRule2::isNull() const{
	return (!_pattern.length() && !_replacement.length());
}
bool QiRule2::isRemoteRule() const{
	return _isRemote;
}
bool QiRule2::isEnable() const{
	return _isEnable;
}
QString QiRule2::name() const{
	return _name;
}
int QiRule2::type() const{
	return _type;
}
QString QiRule2::pattern() const{
	return _pattern;
}
QString QiRule2::replacement() const{
	return _replacement;
}
