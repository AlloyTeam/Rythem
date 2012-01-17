#ifndef IQIRESPONSEWORKER_H
#define IQIRESPONSEWORKER_H

#include <QtCore>

class IQiResponseWorker : public QObject
{
	Q_OBJECT
public:
	bool isConnected();
	QString url();
	int port();
	virtual int bytesAvailable();

public slots:
	void setDestination(const QString url, const int port = 80);
	virtual void start();
	virtual void stop();
	virtual void write(const QByteArray &byteArray);
	virtual void flush();

signals:
	void started();
	void stopped();
	void dataReceived(QByteArray &data);
	void error(int code);

protected:
	QString _url;
	int _port;
	bool _connected;
};

#endif // IQIRESPONSEWORKER_H
