#include <qscriptengine.h>
#include <qdebug.h>
#include <qnetworkinterface.h>
#include <qhostaddress.h>
#include <qhostinfo.h>
#include <qregexp.h>

#include "proxyautoconfig.h"
#include <QtNetwork>

ProxyAutoConfig* ProxyAutoConfig::_instancePtr = 0;
ProxyAutoConfig* ProxyAutoConfig::instance(){
    if(!_instancePtr){
        // warning: no thread safe here.
        _instancePtr = new ProxyAutoConfig();
    }
    return _instancePtr;
}

ProxyAutoConfig::ProxyAutoConfig():QObject(qApp),_isSettup(false)
{
    engine = new QScriptEngine(this);
    install();
}

ProxyAutoConfig::~ProxyAutoConfig()
{
}

void ProxyAutoConfig::setConfigByUrl(const QString &url){
    if(currentPacUrl == url)return;
    currentPacUrl = url;
    QNetworkAccessManager manager;
    //qDebug()<<"url"<<url;
    QTimer timer;
    QEventLoop _loop;
    timer.singleShot(10000,&_loop,SLOT(quit()));//5秒内
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    manager.connect(&manager,SIGNAL(finished(QNetworkReply*)),&_loop,SLOT(quit()));
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
    _loop.exec();
    QString script = QString(reply->readAll());
    setConfig(script);
}

void ProxyAutoConfig::setConfig( const QString &config )
{
    //qDebug()<<config;
    _isSettup = true;
    connect(engine,SIGNAL(signalHandlerException(QScriptValue)),SLOT(onException()));
    engine->evaluate( config );
    engine->disconnect(SIGNAL(signalHandlerException(QScriptValue)),this,SLOT(onException()));
}

void ProxyAutoConfig::install()
{
    QScriptValue globalObject = engine->globalObject();

    QScriptValue fun;

    fun = engine->newFunction( debug );
    globalObject.setProperty( QString("debug"), fun );

    fun = engine->newFunction( myIpAddress );
    globalObject.setProperty( QString("myIpAddress"), fun );

    fun = engine->newFunction( isInNet );
    globalObject.setProperty( "isInNet", fun );

    fun = engine->newFunction( shExpMatch );
    globalObject.setProperty( "shExpMatch", fun );

    fun = engine->newFunction( dnsResolve );
    globalObject.setProperty( "dnsResolve", fun );
}

QScriptValue ProxyAutoConfig::debug( QScriptContext *context, QScriptEngine *engine )
{
    if ( context->argumentCount() != 1 )
	return context->throwError("Debug takes one argument");
    qDebug() << context->argument(0).toString();
    return engine->undefinedValue();
}


/* String myIpAddress */
QScriptValue ProxyAutoConfig::myIpAddress( QScriptContext *context, QScriptEngine *engine )
{
    if ( context->argumentCount() != 0 )
	return context->throwError("myIpAddress takes no arguments");

    foreach( QHostAddress address, QNetworkInterface::allAddresses() ) {
	if ( address != QHostAddress::LocalHost
	     && address != QHostAddress::LocalHostIPv6 )
	    return QScriptValue( engine, address.toString() );
    }

    return engine->undefinedValue();
}

/* bool isInNet ipaddress, netaddress, netmask */
QScriptValue ProxyAutoConfig::isInNet( QScriptContext *context, QScriptEngine *engine )
{
    if ( context->argumentCount() != 3 )
	return context->throwError("isInNet takes three arguments");

    QHostAddress addr( context->argument(0).toString() );
    QHostAddress netaddr( context->argument(1).toString() );
    QHostAddress netmask( context->argument(2).toString() );

    if ( (netaddr.toIPv4Address() & netmask.toIPv4Address()) == (addr.toIPv4Address() & netmask.toIPv4Address()) )
	return QScriptValue( engine, true );

    return QScriptValue( engine, false );
}

/* bool shExpMatch url, glob */
QScriptValue ProxyAutoConfig::shExpMatch( QScriptContext *context, QScriptEngine *engine )
{
    if ( context->argumentCount() != 2 )
	return context->throwError("shExpMatch takes two arguments");

    QRegExp re( context->argument(1).toString(), Qt::CaseSensitive, QRegExp::Wildcard );
    if ( re.exactMatch( context->argument(0).toString() ) )
	return QScriptValue( engine, true );

    return QScriptValue( engine, false );
}

/* string dnsResolve hostname */
QScriptValue ProxyAutoConfig::dnsResolve( QScriptContext *context, QScriptEngine *engine )
{
    if ( context->argumentCount() != 1 )
	return context->throwError("dnsResolve takes one arguments");

    QHostInfo info = QHostInfo::fromName( context->argument(0).toString() );
    QList<QHostAddress> addresses = info.addresses();
    if ( addresses.isEmpty() )
	return engine->nullValue(); // TODO: Should this be undefined or an exception? check other implementations

    return QScriptValue( engine, addresses.first().toString() );
}

QString ProxyAutoConfig::findProxyForUrl( const QString &url, const QString &host )
{
    if(!_isSettup){
        return "DIRECT";
    }
    QMutexLocker locker(&_queryMutex);
    QScriptValue global = engine->globalObject();
    QScriptValue fun = global.property("FindProxyForURL");
    if ( !fun.isFunction() ) {
        return QString("DIRECT");
    }

    QScriptValueList args;
    args << engine->toScriptValue( url ) << engine->toScriptValue( host );

    QScriptValue val = fun.call( global, args );
    return val.toString();
}

void ProxyAutoConfig::onException(){
    _isSettup = false;
}
