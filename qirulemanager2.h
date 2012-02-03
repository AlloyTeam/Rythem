#ifndef QIRULEMANAGER2_H
#define QIRULEMANAGER2_H

#include <QtCore>
#include "qirule2.h"
#include "qirulegroup2.h"

class QiRuleManager2 : public QObject
{
	Q_OBJECT
public:
	explicit QiRuleManager2(QString localFile = "", QString host = "", QString address = "", QString path = "");

	QString localConfigFile;
	QString remoteHost;
	QString remoteAddress;
	QString remotePath;
	
signals:
	
public slots:

private:
	QList<QiRuleGroup2> localGroups;
	QList<QiRuleGroup2> remoteGroups;
	
};

#endif // QIRULEMANAGER2_H
