#ifndef QIRULELOCALFILES_H
#define QIRULELOCALFILES_H

#include <QtCore>
#include "qirule2.h"
#include "qiconnectiondata.h"

class QiRuleLocalFiles : public QiRule2
{
	Q_OBJECT
public:
	explicit QiRuleLocalFiles(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(ConnectionData_ptr conn) const;
	void replace(ConnectionData_ptr conn) const;
	
signals:
	
public slots:
	
};

#endif // QIRULELOCALFILES_H
