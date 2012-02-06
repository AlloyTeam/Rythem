#include "qiruleremotecontent.h"

QiRuleRemoteContent::QiRuleRemoteContent(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{

}

bool QiRuleRemoteContent::match(ConnectionData_ptr conn) const{
	QRegExp rx(pattern(), Qt::CaseInsensitive, QRegExp::Wildcard);
	return rx.exactMatch(conn->fullUrl);
}

void QiRuleRemoteContent::replace(ConnectionData_ptr conn) const{

}
