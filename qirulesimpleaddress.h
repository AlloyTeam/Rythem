#ifndef QIRULESIMPLEADDRESS_H
#define QIRULESIMPLEADDRESS_H

#include <QtCore>
#include "qirule2.h"
#include "qiconnectiondata.h"

class QiRuleSimpleAddress : public QiRule2
{
	Q_OBJECT
public:
	explicit QiRuleSimpleAddress(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(ConnectionData_ptr conn) const;
	void replace(ConnectionData_ptr conn) const;
	
signals:
	
public slots:
	
};

#endif // QIRULESIMPLEADDRESS_H
