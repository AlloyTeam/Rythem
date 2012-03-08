#ifndef RYRULE_H
#define RYRULE_H

#include <QtCore>
#include "rypipedata.h"


//TODO move this in class
enum RuleType{
	COMPLEX_ADDRESS_REPLACE = 1,
	SIMPLE_ADDRESS_REPLACE = 2,
	REMOTE_CONTENT_REPLACE = 3,
	LOCAL_FILE_REPLACE = 4,
	LOCAL_FILES_REPLACE = 5,
	LOCAL_DIR_REPLACE = 6
};

class RyRule : public QObject
{
	Q_OBJECT
public:
        RyRule();
        RyRule(const RyRule &rule);
        explicit RyRule(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);

        RyRule operator =(const RyRule &rule);
        bool operator ==(const RyRule &rule) const;
        bool operator <(const RyRule &rule) const;

	void update(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
        void update(const RyRule &rule);
	virtual bool match(const QString &url) const;
        virtual QPair<QByteArray, QByteArray> replace(RyPipeData_ptr conn);
	QString toJSON(int tabCount = 0) const;

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

QDebug operator <<(QDebug dbg, const RyRule &rule);

#endif // RYRULE_H
