#ifndef QIRULEREMOTECONTENT_H
#define QIRULEREMOTECONTENT_H

#include <QtCore>
#include "qirule2.h"
#include "qiconnectiondata.h"

class QiRuleRemoteContent : public QiRule2
{
	Q_OBJECT
public:
	explicit QiRuleRemoteContent(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(const QString &url) const;
	QPair<QByteArray, QByteArray> replace(ConnectionData_ptr conn) const;
	
signals:
	
public slots:
	
};

#endif // QIRULEREMOTECONTENT_H
