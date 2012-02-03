#ifndef JSON_H
#define JSON_H

#include <QtCore>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

class JsonValue;

class JSON
{
public:
	JSON();
	~JSON();
	bool parse(const QString &content);
	QString toString() const;
	JsonValue operator[](const QString name) const;

private:
	QScriptEngine *engine;
	JsonValue *value;
};

class JsonValue : public QScriptValue
{
public:
	JsonValue(QScriptValue scriptValue);
	JsonValue operator[](const QString name) const;
};

#endif // JSON_H
