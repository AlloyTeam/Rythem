#include "qirulesimpleaddress.h"

QiRuleSimpleAddress::QiRuleSimpleAddress(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{
}

bool QiRuleSimpleAddress::match(ConnectionData_ptr conn) const{
	return conn->host == pattern();
}

void QiRuleSimpleAddress::replace(ConnectionData_ptr conn) const{

}
