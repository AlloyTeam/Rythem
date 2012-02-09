#include "qiruleremotecontent.h"

QiRuleRemoteContent::QiRuleRemoteContent(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{

}

bool QiRuleRemoteContent::match(const QString &url) const{
	QRegExp rx(pattern(), Qt::CaseInsensitive, QRegExp::Wildcard);
	return rx.exactMatch(url);
}

QPair<QByteArray, QByteArray> QiRuleRemoteContent::replace(ConnectionData_ptr) const{
	return QPair<QByteArray, QByteArray>();
}
