#include "qnetworkproxyfactoryexendforpac.h"
#include <QScriptEngine>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QRegExp>


QNetworkProxyFactoryExendForPAC::QNetworkProxyFactoryExendForPAC()
{
    engine = new QScriptEngine(this);
    install();
}

QNetworkProxyFactoryExendForPAC::~QNetworkProxyFactoryExendForPAC()
{
}

void QNetworkProxyFactoryExendForPAC::setConfig( const QString &config )
{
    engine->evaluate( config );
}

void QNetworkProxyFactoryExendForPAC::install()
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

QScriptValue QNetworkProxyFactoryExendForPAC::debug( QScriptContext *context, QScriptEngine *engine )
{
    if ( context->argumentCount() != 1 )
        return context->throwError("Debug takes one argument");
    qDebug() << context->argument(0).toString();
    return engine->undefinedValue();
}


/* String myIpAddress */
QScriptValue QNetworkProxyFactoryExendForPAC::myIpAddress( QScriptContext *context, QScriptEngine *engine )
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
QScriptValue QNetworkProxyFactoryExendForPAC::isInNet( QScriptContext *context, QScriptEngine *engine )
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
QScriptValue QNetworkProxyFactoryExendForPAC::shExpMatch( QScriptContext *context, QScriptEngine *engine )
{
    if ( context->argumentCount() != 2 )
        return context->throwError("shExpMatch takes two arguments");

    QRegExp re( context->argument(1).toString(), Qt::CaseSensitive, QRegExp::Wildcard );
    if ( re.exactMatch( context->argument(0).toString() ) )
        return QScriptValue( engine, true );

    return QScriptValue( engine, false );
}

/* string dnsResolve hostname */
QScriptValue QNetworkProxyFactoryExendForPAC::dnsResolve( QScriptContext *context, QScriptEngine *engine )
{
    if ( context->argumentCount() != 1 )
        return context->throwError("dnsResolve takes one arguments");

    QHostInfo info = QHostInfo::fromName( context->argument(0).toString() );
    QList<QHostAddress> addresses = info.addresses();
    if ( addresses.isEmpty() )
        return engine->nullValue(); // TODO: Should this be undefined or an exception? check other implementations

    return QScriptValue( engine, addresses.first().toString() );
}

QString QNetworkProxyFactoryExendForPAC::findProxyForUrl( const QString &url, const QString &host )
{
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

