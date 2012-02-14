#include "ryrulecomplexaddress.h"

RyRuleComplexAddress::RyRuleComplexAddress(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	RyRule(name, type, pattern, replacement, enable, remote)
{

}

bool RyRuleComplexAddress::match(const QString &) const{
	//ignore complex address replace for now
	return false;
}

QPair<QByteArray, QByteArray> RyRuleComplexAddress::replace(RyPipeData_ptr) const{
	return QPair<QByteArray, QByteArray>();
}
