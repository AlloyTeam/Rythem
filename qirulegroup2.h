#ifndef QIRULEGROUP2_H
#define QIRULEGROUP2_H

#include <QtCore>
#include "qirule2.h"
#include "qiconnectiondata.h"

class QiRuleGroup2: public QObject
{
	Q_OBJECT
public:
	QiRuleGroup2();
	QiRuleGroup2(const QiRuleGroup2 &group);
	explicit QiRuleGroup2(QString name, bool enable = true, bool remote = false);

	QiRuleGroup2 operator =(const QiRuleGroup2 &group);
	bool operator ==(const QiRuleGroup2 &group) const;

	void update(QString name, bool enable = true, bool remote = false);
	void update(const QiRuleGroup2 &group);
	void addRule(const QiRule2 &value, int index = -1);
	int length() const;
	int getRuleIndex(const QString name) const;
	QiRule2 getRule(const QString name) const;
	QiRule2 getRuleAt(const int index) const;
	void updateRule(const QString name, const QiRule2 &newValue);
	void updateRuleAt(const int index, const QiRule2 &newValue);
	void removeRule(const QString name);
	void removeRuleAt(const int index);
	QiRule2 match(ConnectionData_ptr conn) const;
	QString toJSON() const;

	bool isNull() const;
	bool isRemote() const;
	bool isEnable() const;
	QString groupName() const;

signals:
	void changed();

private:
	QString _groupName;
	bool _isEnable;
	bool _isRemote;
	QList<QiRule2> _rules;
};

#endif // QIRULEGROUP2_H
