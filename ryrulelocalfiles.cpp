#include "ryrulelocalfiles.h"
#include <QScriptEngine>
#include <QScriptValue>
#include <QDir>

RyRuleLocalFiles::RyRuleLocalFiles(QString name, int type, QString pattern, QString replacement, bool enable, bool remote) :
	RyRule(name, type, pattern, replacement, enable, remote)
{

}

bool RyRuleLocalFiles::match(const QString &url) const{
	QRegExp rx(pattern(), Qt::CaseInsensitive, QRegExp::Wildcard);
	return rx.exactMatch(url);
}

/**
  how does a .qzmin file looks like ?
  {
	projects: [
		{
			name: "Project Name",
			target: "target file's relative path",
			include: [
				"source file's relative path",
				...
			]
		},
		...
	],
	level: 0,
	shrink: false,
	encode: "utf-8",
	comment: "just some comment"
  }
  */
QPair<QByteArray, QByteArray> RyRuleLocalFiles::replace(RyPipeData_ptr) const{
	QPair<QByteArray, QByteArray> result;
	QFile f(replacement());
	bool fileCanOpen = f.open(QFile::ReadOnly| QIODevice::Text);

	QByteArray mergeFileContent;
	QMap<QString, QVariant> mergeValueMap;
	QString encode;
	QString status;
	QByteArray body, header;
	bool mergeContentHasError;

	if(fileCanOpen){
		QScriptEngine engine;
		mergeFileContent = f.readAll();
		mergeValueMap = engine.evaluate(mergeFileContent.prepend("(").append(")")).toVariant().toMap();
		if(mergeValueMap.contains("encode")){
			encode = mergeValueMap["encode"].toString();
		}
		//qDebug()<<mergeValueMap;
		//qDebug()<<mergeFileContent;
		mergeValueMap = mergeValueMap["projects"].toList().first().toMap();
		if(engine.hasUncaughtException() || mergeValueMap.isEmpty()){//wrong content
			qDebug() << "wrong qzmin format:" << replacement() << mergeFileContent;
			mergeContentHasError = true;
		}
	}else{
		qDebug()<<"file cannot open";
		mergeContentHasError = true;
	}
	f.close();
	if(mergeContentHasError){
		status = "404 NOT FOUND";
		body.append(QString("merge file with wrong format:").append(replacement()).append(mergeFileContent));
	}else{
		status = "200 OK";
		foreach(QVariant item,mergeValueMap["include"].toList()){
			//qDebug()<<item.toString();
			f.setFileName(item.toString());
			fileCanOpen = f.open(QFile::ReadOnly);
			if(fileCanOpen){
				body.append(f.readAll());
			}else{
				body.append(QString("/*file:【%1】 not found*/").arg(item.toString()));
			}
			f.close();
		}
	}
	int count = body.size();
	header.append(QString("HTTP/1.1 %1 \r\nServer: Qiddler \r\nContent-Type: %2 charset=%3 \r\nContent-Length: %4 \r\n\r\n")
					   .arg(status)
					   .arg("text/javascript") // TODO reuse contentTypeMapping above
					   .arg(encode)
					   .arg(count));

	result.first = header;
	result.second = body;
	return result;
}
