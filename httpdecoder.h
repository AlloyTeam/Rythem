#ifndef HTTPDECODER_H
#define HTTPDECODER_H

#define HEADER_METHOD "method"
#define HEADER_PATH = "path"
#define HEADER_PORT = "port"
#define HEADER_URL = "url"
#define HEADER_STATUS_CODE = "statusCode"
#define HEADER_REASON_PHRASE = "reasonPhrase"
#define HEADER_HOST = "host"

#include <QtCore>

class HTTPDecoder
{
public:
	//return true if we have a complete package here(header and body)
	static bool decode(const QByteArray &package, bool *headerComplete = 0, QMap<QString, QString> *header = 0, bool *bodyComplete = 0, QByteArray *body = 0);
};

#endif // HTTPDECODER_H
