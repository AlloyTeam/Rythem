#ifndef RYRULELOCALFILE_H
#define RYRULELOCALFILE_H

#include <QtCore>
#include "ryrule.h"
#include "rypipedata.h"

class RyRuleLocalFile : public RyRule
{
	Q_OBJECT
public:
	explicit RyRuleLocalFile(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(const QString &url) const;
	QPair<QByteArray, QByteArray> replace(RyPipeData_ptr conn) const;
	
signals:
	
public slots:

public:
	static QMap<QString, QString> mimeMap;
private:
	static QMap<QString, QString> initMimeMap();
};

#endif // RYRULELOCALFILE_H
