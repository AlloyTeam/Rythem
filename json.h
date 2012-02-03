#ifndef JSON_H
#define JSON_H

#include <QtCore>
#include <QtScript>

class JSON
{
public:
	JSON();
	static QScriptValue parse(const QString raw);
	static QString toString();
};

#endif // JSON_H
