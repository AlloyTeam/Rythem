#ifndef RYRULELOCALDIR_H
#define RYRULELOCALDIR_H

#include <QtCore>
#include "ryrule.h"
#include "rypipedata.h"

#include <QMutex>

class RyRuleLocalDir : public RyRule
{
	Q_OBJECT
public:
        explicit RyRuleLocalDir(QString name, int type, QString pattern, QString replacement, bool enable = true, bool remote = false);
	bool match(const QString &url) const;
        QPair<QByteArray, QByteArray> replace(RyPipeData_ptr conn);
	
signals:
	
public slots:
private:
    QMutex mutex;
	
};

#endif // RYRULELOCALDIR_H
