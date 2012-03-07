#include "ryrulelocaldir.h"

RyRuleLocalDir::RyRuleLocalDir(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	RyRule(name, type, pattern, replacement, enable, remote)
{

}

bool RyRuleLocalDir::match(const QString &url) const{
    return (url.indexOf(pattern()) != -1);
}

QPair<QByteArray, QByteArray> RyRuleLocalDir::replace(RyPipeData_ptr conn) const{
	QPair<QByteArray, QByteArray> result;
	QByteArray header, body;
    QString status = "200 OK";

	int patternIndex = conn->fullUrl.indexOf(pattern());
	int patternLength = pattern().length();
	QString fileName = conn->fullUrl.mid(patternIndex+patternLength);
	//qDebug()<<fileName;
	if(fileName.indexOf("?")!=-1){
		fileName = fileName.left(fileName.indexOf("?"));
		//qDebug()<<fileName;
    }
	if(fileName.indexOf("#")!=-1){
		fileName = fileName.left(fileName.indexOf("#"));
		//qDebug()<<fileName;
	}
    QString _replaceMent = replacement();
#ifdef Q_OS_WIN
    if(_replaceMent.indexOf("\\")==_replaceMent.length()-1){
        _replaceMent.remove(_replaceMent.length()-1,1);
    }
#else
    if(_replaceMent.indexOf("/")==_replaceMent.length()-1){
        _replaceMent.remove(_replaceMent.length()-1,1);
    }
#endif
    if(fileName=="/"){
        fileName = "/index.html";
    }
    fileName.prepend(_replaceMent);
	//qDebug()<<fileName;
	QFile f(fileName);
	bool fileCanOpen = f.open(QFile::ReadOnly | QIODevice::Text);
	if(fileCanOpen){
		body = f.readAll();
	}else{
		status = "404 Not Found";
		body.append(QString("file:%1 not found").arg(fileName));
	}
	f.close();
	int count = body.size();
	header.append(QString("HTTP/1.1 %1 \r\nServer: Qiddler \r\nContent-Type: %2 \r\nContent-Length: %3 \r\n\r\n")
					   .arg(status)
					   .arg("text/html") // TODO reuse contentTypeMapping above
					   .arg(count));

	result.first = header;
	result.second = body;
	return result;
}
