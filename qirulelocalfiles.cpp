#include "qirulelocalfiles.h"

QiRuleLocalFiles::QiRuleLocalFiles(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{

}

bool QiRuleLocalFiles::match(ConnectionData_ptr conn) const{
	QRegExp rx(pattern(), Qt::CaseInsensitive, QRegExp::Wildcard);
	return rx.exactMatch(conn->fullUrl);
}

void QiRuleLocalFiles::replace(ConnectionData_ptr conn) const{

}
