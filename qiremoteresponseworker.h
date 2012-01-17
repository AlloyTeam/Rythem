#ifndef QIREMOTERESPONSEWORKER_H
#define QIREMOTERESPONSEWORKER_H

#include <QObject>
#include "iqiresponseworker.h"

class QiRemoteResponseWorker : public QObject
{
	Q_OBJECT
public:
	explicit QiRemoteResponseWorker(QObject *parent = 0);
	
signals:
	
public slots:
	
};

#endif // QIREMOTERESPONSEWORKER_H
