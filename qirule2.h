#ifndef QIRULE2_H
#define QIRULE2_H

#include <QtCore>

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
	QiRule2(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(QString path);
	QString toJSON();

	bool isRemoteRule();
	bool isEnable();
	QString name();
	int type();
	QString pattern();
	QString replacement();

	void update(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	void setIsRemoteRule(bool value);
	void setEnable(bool value);
	void setName(QString value);
	void setType(int value);
	void setPattern(QString value);
	void setReplacement(QString value);

signals:
	void changed();

private:
	QString _name;
	QString _pattern;
	QString _replacement;
	int _type;
	bool _isEnable;
	bool _isRemote;
};

#endif // QIRULE2_H
