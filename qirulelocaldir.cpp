#include "qirulelocaldir.h"

QiRuleLocalDir::QiRuleLocalDir(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{

}

bool QiRuleLocalDir::match(const QString &url) const{
	return (url.indexOf(pattern()) != -1);
}

QPair<QByteArray, QByteArray> QiRuleLocalDir::replace(ConnectionData_ptr) const{
	return QPair<QByteArray, QByteArray>();
}
