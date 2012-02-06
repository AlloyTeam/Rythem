#include "qirulelocalfile.h"

QiRuleLocalFile::QiRuleLocalFile(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{

}

bool QiRuleLocalFile::match(const QString &url) const{
	QRegExp rx(pattern(), Qt::CaseInsensitive, QRegExp::Wildcard);
	return rx.exactMatch(url);
}

void QiRuleLocalFile::replace(ConnectionData_ptr conn) const{
	qDebug() << "checking local file rule" << pattern();
}
