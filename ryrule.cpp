#include "ryrule.h"

RyRule::RyRule() :QObject(){

}

RyRule::RyRule(const RyRule &rule) :
	QObject(),
	_name(rule.name()),
	_type(rule.type()),
	_pattern(rule.pattern()),
	_replacement(rule.replacement()),
	_isEnable(rule.isEnable()),
	_isRemote(rule.isRemoteRule())
{

}

RyRule::RyRule(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QObject(),
	_name(name),
	_type(type),
	_pattern(pattern),
	_replacement(replacement),
	_isEnable(enable),
	_isRemote(remote)
{
}

RyRule RyRule::operator =(const RyRule &rule){
	this->update(rule.name(), rule.type(), rule.pattern(), rule.replacement(), rule.isEnable(), rule.isRemoteRule());
	return *this;
}

bool RyRule::operator ==(const RyRule &rule) const{
	return this->name() == rule.name();
}

bool RyRule::operator <(const RyRule &rule) const{
	return this->type() < rule.type();
}

void RyRule::update(QString name, int type, QString pattern, QString replacement, bool enable, bool remote){
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

void RyRule::update(const RyRule &rule){
	this->update(rule.name(), rule.type(), rule.pattern(), rule.replacement(), rule.isEnable(), rule.isRemoteRule());
}

bool RyRule::match(const QString &) const{
	//override this method in different type of rule
	return false;
}

QPair<QByteArray, QByteArray> RyRule::replace(RyPipeData_ptr) const{
	//override this method in different type of rule
	return QPair<QByteArray, QByteArray>();
}

QString RyRule::toJSON(int tabCount) const{
	QString tabs = QString("\t").repeated(tabCount);
	if(!isNull()){
		QString result;
		QTextStream(&result) << tabs << "{\n"
							 << tabs << "\t\"name\":\"" << _name << "\",\n"
							 << tabs << "\t\"type\":" << _type << ",\n"
							 << tabs << "\t\"enable\":" << _isEnable << ",\n"
                                                         << tabs << "\t\"rule\":{\"pattern\":\"" << _pattern << "\",\n"
                                                         << tabs << "\t\"replace\":\"" << _replacement << "\"}\n"
							 << tabs << "}";
		return result;
	}
	else return "{}";
}

bool RyRule::isNull() const{
	return (!_pattern.length() && !_replacement.length());
}
bool RyRule::isRemoteRule() const{
	return _isRemote;
}
bool RyRule::isEnable() const{
	return _isEnable;
}
QString RyRule::name() const{
	return _name;
}
int RyRule::type() const{
	return _type;
}
QString RyRule::pattern() const{
	return _pattern;
}
QString RyRule::replacement() const{
	return _replacement;
}

QDebug operator <<(QDebug dbg, const RyRule &rule){
	dbg.nospace() << "(pattern=" << rule.pattern() << ", replace=" << rule.replacement() << ")";
	return dbg.space();
}
