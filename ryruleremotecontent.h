#ifndef QIRULEREMOTECONTENT_H
#define QIRULEREMOTECONTENT_H

#include <QtCore>
#include "ryrule.h"
#include "rypipedata.h"

class RyRuleRemoteContent : public RyRule
{
	Q_OBJECT
public:
        explicit RyRuleRemoteContent(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(const QString &url) const;
        QPair<QByteArray, QByteArray> replace(RyPipeData_ptr conn) const;
	
signals:
	
public slots:
	
};

#endif // QIRULEREMOTECONTENT_H
