#ifndef QIRULE2_H
#define QIRULE2_H

#include <QtCore>
#include "qiconnectiondata.h"

enum RuleType{
	COMPLEX_ADDRESS_REPLACE = 1,
	SIMPLE_ADDRESS_REPLACE = 2,
	REMOTE_CONTENT_REPLACE = 3,
	LOCAL_FILE_REPLACE = 4,
	LOCAL_FILES_REPLACE = 5,
	LOCAL_DIR_REPLACE = 6
};

class QiRule2 : public QObject
{
	Q_OBJECT
public:
	QiRule2();
	QiRule2(const QiRule2 &rule);
	explicit QiRule2(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);

	QiRule2 operator =(const QiRule2 &rule);
	bool operator ==(const QiRule2 &rule) const;

	void update(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	void update(const QiRule2 &rule);
	bool match(ConnectionData_ptr conn) const;
	QString toJSON() const;

	bool isNull() const;
	bool isRemoteRule() const;
	bool isEnable() const;
	QString name() const;
	int type() const;
	QString pattern() const;
	QString replacement() const;

signals:
	void changed();

private:
	QString _name;
	int _type;
	QString _pattern;
	QString _replacement;
	bool _isEnable;
	bool _isRemote;
};

#endif // QIRULE2_H
