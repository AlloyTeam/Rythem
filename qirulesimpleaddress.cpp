#include "qirulesimpleaddress.h"
#include <QUrl>

QiRuleSimpleAddress::QiRuleSimpleAddress(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{
	patternHost = QUrl(pattern).host();
	replacementHost = QUrl(replacement).host();
}

bool QiRuleSimpleAddress::match(const QString &url) const{
	QUrl fullUrl(url);
	return fullUrl.host() == patternHost;
}

QPair<QByteArray, QByteArray> QiRuleSimpleAddress::replace(ConnectionData_ptr conn) const{
	//this is just a host replace
	conn->replacedHost = replacementHost;
	return QPair<QByteArray, QByteArray>();
}
