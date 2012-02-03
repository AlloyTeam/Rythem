#include "json.h"

// JSON ///////////////////////////////////////////////////////////////////////

JSON::JSON()
{
	engine = new QScriptEngine();
	value = 0;
}

JSON::~JSON(){
	delete engine;
	if(value) delete value;
}

bool JSON::parse(const QString &content){
	QString data = QString("(%1)").arg(content);
	if(value) delete value;
	value = new JsonValue(engine->evaluate(data));
	return true;
}

QString JSON::toString() const{
	return QString("just a test");
}

JsonValue JSON::operator [](const QString name) const{
	return (*value)[name];
}

// JsonValue //////////////////////////////////////////////////////////////////

JsonValue::JsonValue(QScriptValue scriptValue) : QScriptValue(scriptValue)
{

}

JsonValue JsonValue::operator[](const QString name) const{
	return JsonValue(this->property(name));
}
