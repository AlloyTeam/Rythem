#include "ryrulesimpleaddress.h"
#include <QUrl>

RyRuleSimpleAddress::RyRuleSimpleAddress(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	RyRule(name, type, pattern, replacement, enable, remote)
{
	patternHost = QUrl(pattern).host();
	replacementHost = QUrl(replacement).host();
}

bool RyRuleSimpleAddress::match(const QString &url) const{
	QUrl fullUrl(url);
	return fullUrl.host() == patternHost;
}

QPair<QByteArray, QByteArray> RyRuleSimpleAddress::replace(RyPipeData_ptr conn) const{
	//this is just a host replace
	conn->replacedHost = replacementHost;
	return QPair<QByteArray, QByteArray>();
}
