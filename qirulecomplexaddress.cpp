#include "qirulecomplexaddress.h"

QiRuleComplexAddress::QiRuleComplexAddress(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{
}

bool QiRuleComplexAddress::match(ConnectionData_ptr conn) const{
	//ignore complex address replace for now
	return false;
}

void QiRuleComplexAddress::replace(ConnectionData_ptr conn) const{

}
