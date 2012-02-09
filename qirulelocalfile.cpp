#include "qirulelocalfile.h"

QiRuleLocalFile::QiRuleLocalFile(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	QiRule2(name, type, pattern, replacement, enable, remote)
{

}

bool QiRuleLocalFile::match(const QString &url) const{
	QRegExp rx(pattern(), Qt::CaseInsensitive, QRegExp::Wildcard);
	return rx.exactMatch(url);
}

QPair<QByteArray, QByteArray> QiRuleLocalFile::replace(ConnectionData_ptr) const{
	//open local file for read
	QFile file(replacement());
	QString status;
	QByteArray body;
	if(file.open(QFile::ReadOnly)){
		status = "200 OK";
		body = file.readAll();
	}
	else{
		status = "404 Not Found";
		body.append(QString("file %1 not found").arg(replacement()));
	}
	file.close();
	int contentLength = body.size();

	//create response
	QByteArray responseHeader;
	QTextStream(&responseHeader) << "HTTP/1.1 " << status << "\r\n"
								 << "Server: Rythem\r\n"
								 << "Content-Type: text/html\r\n"
								 << "Content-Length: " << contentLength << "\r\n\r\n";

	QPair<QByteArray, QByteArray> result;
	result.first = responseHeader;
	result.second = body;
	return result;
}
