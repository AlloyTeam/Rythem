#ifndef QIRULEMANAGER2_H
#define QIRULEMANAGER2_H

#include <QtCore>

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
	QList localGroups;
	QList remoteGroups;
	
};

#endif // QIRULEMANAGER2_H
