#ifndef QIRULELOCALDIR_H
#define QIRULELOCALDIR_H

#include <QtCore>
#include "qirule2.h"
#include "qiconnectiondata.h"

class QiRuleLocalDir : public QiRule2
{
	Q_OBJECT
public:
	explicit QiRuleLocalDir(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(const QString &url) const;
	void replace(ConnectionData_ptr conn) const;
	
signals:
	
public slots:
	
};

#endif // QIRULELOCALDIR_H
