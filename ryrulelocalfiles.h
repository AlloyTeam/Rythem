#ifndef RYRULELOCALFILES_H
#define RYRULELOCALFILES_H

#include <QtCore>
#include "ryrule.h"
#include "rypipedata.h"

class RyRuleLocalFiles : public RyRule
{
	Q_OBJECT
public:
        explicit RyRuleLocalFiles(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(const QString &url) const;
    QPair<QByteArray, QByteArray> replace(RyPipeData_ptr conn);
	
signals:
	
public slots:
	
};

#endif // RYRULELOCALFILES_H
