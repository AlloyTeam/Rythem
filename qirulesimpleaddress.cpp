#include "qirulesimpleaddress.h"
#include <QUrl>

QiRuleSimpleAddress::QiRuleSimpleAddress(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{
}

bool QiRuleSimpleAddress::match(const QString &url) const{
	QUrl fullUrl(url);
	QUrl patternUrl(pattern());
	return fullUrl.host() == patternUrl.host();
}

QPair<QByteArray, QByteArray> QiRuleSimpleAddress::replace(ConnectionData_ptr) const{
	//this is just a host replace
	return QPair<QByteArray, QByteArray>();
}
