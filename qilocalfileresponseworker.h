#ifndef QILOCALFILERESPONSEWORKER_H
#define QILOCALFILERESPONSEWORKER_H

#include <QtCore>
#include "iqiresponseworker.h"

class QiLocalFileResponseWorker : public IQiResponseWorker
{
	Q_OBJECT
public:
	explicit QiLocalFileResponseWorker(QObject *parent = 0);
	~QiLocalFileResponseWorker();
	enum FileError{
		FileNotFoundError = 101
	};
	int bytesAvailable();
	
signals:
	
public slots:
	void start(); //start reading file content
	void stop(); //stop reading file content
	void write(const QByteArray &byteArray); //do nothing
	void flush(); //do nothing

protected:
	QFile *file;
	QByteArray *receiveBuffer;

protected slots:
	void onFileReadReady();
	
};

#endif // QILOCALFILERESPONSEWORKER_H
