#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QtCore>
#include <QTcpSocket>
#include <QAbstractSocket>

class WebSocketClient : public QTcpSocket
{
	Q_OBJECT
public:
	explicit WebSocketClient(QObject *parent = 0);
	~WebSocketClient();
	void sendMessage(const char *message);
	void sendMessage(const QByteArray &message);
	
signals:
	void message(QByteArray data);
	void finished();

public slots:

protected:
	//status and buffer
	bool handshaked;
	QByteArray *buffer;

	//process received handshake or data package
	void processReceivedData();
	QByteArray generateAcceptHash(const QString &key) const;
	QByteArray generateMessagePackage(const char *message, bool mask = false);
	void sendHandshakeResponse(const QByteArray &acceptHash);

protected slots:
	void onDisconnected();
	void onError(QAbstractSocket::SocketError socketError);
	void onReadyRead();

};

#endif // WEBSOCKETCLIENT_H
