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
	QPair<QByteArray, QByteArray> replace(ConnectionData_ptr conn) const;
	
signals:
	
public slots:

private:
	static QMap<QString, QString> mimeMap;
	static QMap<QString, QString> initMimeMap();
};

#endif // QIRULELOCALFILE_H
