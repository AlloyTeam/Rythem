#include "httpdecoder.h"

bool HTTPDecoder::decode(const QByteArray &package, bool *headerComplete, QMap<QString, QString> *headerItems, bool *bodyComplete, QByteArray *body){
	int index = package.indexOf("\r\n\r\n");
	if(index == -1){
		//header and body are both not yet complete
		*headerComplete = false;
		*bodyComplete = false;
		return false;
	}
	else{
		//we received a complete header here
		*headerComplete = true;
		//parse the header
		QString header(package.left(index));
		QStringList headerMap = header.split("\r\n");
		QStringList firstLine = headerMap.at(0).split(" "); //"GET xxx HTTP/1.1" or "HTTP/1.1 200 OK"
		for(int i=1; i<headerMap.length(); i++){
			QStringList keyValue = headerMap.at(i).split(": "); //xxx: xxx
			headerItems->insert(keyValue.at(0), keyValue.at(1));
		}

		QByteArray body = package.right(index + 4);
	}
	return false;
}
