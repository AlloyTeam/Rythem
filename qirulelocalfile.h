#ifndef QIRULELOCALFILE_H
#define QIRULELOCALFILE_H

#include <QtCore>
#include "qirule2.h"
#include "qiconnectiondata.h"

class QiRuleLocalFile : public QiRule2
{
	Q_OBJECT
public:
	explicit QiRuleLocalFile(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(const QString &url) const;
	void replace(ConnectionData_ptr conn) const;
	
signals:
	
public slots:
	
};

#endif // QIRULELOCALFILE_H
