#include "qirulelocalfiles.h"

QiRuleLocalFiles::QiRuleLocalFiles(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{

}

bool QiRuleLocalFiles::match(const QString &url) const{
	QRegExp rx(pattern(), Qt::CaseInsensitive, QRegExp::Wildcard);
	return rx.exactMatch(url);
}

QPair<QByteArray, QByteArray> QiRuleLocalFiles::replace(ConnectionData_ptr) const{
	QPair<QByteArray, QByteArray> result;
	return result;
}
