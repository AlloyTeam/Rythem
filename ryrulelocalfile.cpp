#include "ryrulelocalfile.h"

QMap<QString, QString> RyRuleLocalFile::mimeMap = RyRuleLocalFile::initMimeMap();

QMap<QString, QString> RyRuleLocalFile::initMimeMap(){
	QMap<QString, QString> map;
	map.insert("html",	"text/html");
	map.insert("js",	"text/javascript");
	map.insert("css",	"text/css");
	map.insert("txt",	"text/plain");
	map.insert("jpg",	"image/jpeg");
	map.insert("png",	"image/png");
	map.insert("bmp",	"image/bmp");
	map.insert("gif",	"image/gif");
	return map;
}

RyRuleLocalFile::RyRuleLocalFile(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	RyRule(name, type, pattern, replacement, enable, remote)
{

}

bool RyRuleLocalFile::match(const QString &url) const{
	QRegExp rx(pattern(), Qt::CaseInsensitive, QRegExp::Wildcard);
	return rx.exactMatch(url);
}

QPair<QByteArray, QByteArray> RyRuleLocalFile::replace(RyPipeData_ptr) const{
	//open local file for read
	QFile file(replacement());
	QFileInfo fileInfo(file);
	QString ext = fileInfo.suffix().toLower();
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

	//content length and content type
	int contentLength = body.size();
	QString contentType = mimeMap.value(ext, "text/plain");

	//create response
	QByteArray responseHeader;
	QTextStream(&responseHeader) << "HTTP/1.1 " << status << "\r\n"
								 << "Server: Rythem\r\n"
								 << "Content-Type: " << contentType << "\r\n"
								 << "Content-Length: " << contentLength << "\r\n\r\n";

	QPair<QByteArray, QByteArray> result;
	result.first = responseHeader;
	result.second = body;
	return result;
}
