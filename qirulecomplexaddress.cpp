#include "qirulecomplexaddress.h"

QiRuleComplexAddress::QiRuleComplexAddress(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{

}

bool QiRuleComplexAddress::match(const QString &) const{
	//ignore complex address replace for now
	return false;
}

QPair<QByteArray, QByteArray> QiRuleComplexAddress::replace(ConnectionData_ptr) const{
	return QPair<QByteArray, QByteArray>();
}
