#ifndef QIRULECOMPLEXADDRESS_H
#define QIRULECOMPLEXADDRESS_H

#include <QtCore>
#include "qirule2.h"
#include "qiconnectiondata.h"

class QiRuleComplexAddress : public QiRule2
{
	Q_OBJECT
public:
	explicit QiRuleComplexAddress(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(ConnectionData_ptr conn) const;
	void replace(ConnectionData_ptr conn) const;
	
signals:
	
public slots:
	
};

#endif // QIRULECOMPLEXADDRESS_H
