#ifndef RYRULESIMPLEADDRESS_H
#define RYRULESIMPLEADDRESS_H

#include <QtCore>
#include <QUrl>
#include "ryrule.h"
#include "rypipedata.h"

class RyRuleSimpleAddress : public RyRule
{
	Q_OBJECT
public:
        explicit RyRuleSimpleAddress(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(const QString &url) const;
        QPair<QByteArray, QByteArray> replace(RyPipeData_ptr conn) const;
	
signals:
	
public slots:

private:
	QString patternHost;
	QString replacementHost;
	
};

#endif // RYRULESIMPLEADDRESS_H
