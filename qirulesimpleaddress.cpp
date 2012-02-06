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

void QiRuleSimpleAddress::replace(ConnectionData_ptr conn) const{
	qDebug() << "checking simple address rule" << pattern();
}
