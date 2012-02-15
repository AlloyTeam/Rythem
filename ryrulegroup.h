#ifndef QIRULEGROUP2_H
#define QIRULEGROUP2_H

#include <QtCore>
#include "ryrule.h"
#include "rypipedata.h"
#include "ryrulesimpleaddress.h"

class QiRuleGroup2: public QObject
{
	Q_OBJECT
public:
	QiRuleGroup2();
	QiRuleGroup2(const QiRuleGroup2 &group);
	explicit QiRuleGroup2(QString name, bool enable = true, bool remote = false);
	~QiRuleGroup2();

	QiRuleGroup2 operator =(const QiRuleGroup2 &group);
	bool operator ==(const QiRuleGroup2 &group) const;

	void update(QString name, bool enable = true, bool remote = false);
	void update(const QiRuleGroup2 &group);
        void addRule(RyRule *value, int index = -1);
	int length() const;
	int getRuleIndex(const QString name) const;
        RyRule *getRule(const QString name) const;
        RyRule *getRuleAt(const int index) const;
        void updateRule(const QString name, const RyRule &newValue);
        void updateRuleAt(const int index, const RyRule &newValue);
	void removeRule(const QString name);
	void removeRuleAt(const int index);
        void match(QList<RyRule *> *result, const QString &url) const;
	QString toJSON(int tabCount = 0, bool withName = false) const;

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
        QList<RyRule *> _rules;
};

#endif // QIRULEGROUP2_H
