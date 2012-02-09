#include "qiruleremotecontent.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

QiRuleRemoteContent::QiRuleRemoteContent(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{

}

bool QiRuleRemoteContent::match(const QString &url) const{
	QRegExp rx(pattern(), Qt::CaseInsensitive, QRegExp::Wildcard);
	return rx.exactMatch(url);
}

QPair<QByteArray, QByteArray> QiRuleRemoteContent::replace(ConnectionData_ptr) const{
	//load remote content
	QNetworkAccessManager networkAccessManager;
	QNetworkReply *reply = networkAccessManager.get(QNetworkRequest(QUrl(replacement())));
	QEventLoop loop;
	connect(&networkAccessManager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
	loop.exec();

	//get status, header and body
	bool isStatusOK;
	int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&isStatusOK);
	if(!isStatusOK){
		status = 400;
	}
	QByteArray reasonPhrase = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray();
	QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
	QString contentLength = reply->header(QNetworkRequest::ContentLengthHeader).toString();
	QByteArray body = reply->readAll();
	QByteArray responseHeader;
	QTextStream(&responseHeader) << "HTTP/1.1 " << status << " " << reasonPhrase << "\r\n"
								 << "Server: Rythem\r\n"
								 << "Content-Type: " << contentType << "\r\n"
								 << "Content-Length: " << contentLength << "\r\n\r\n";

	QPair<QByteArray, QByteArray> result(responseHeader, body);
	return result;
}
