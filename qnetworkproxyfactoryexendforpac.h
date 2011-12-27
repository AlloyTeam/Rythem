#ifndef QNETWORKPROXYFACTORYEXENDFORPAC_H
#define QNETWORKPROXYFACTORYEXENDFORPAC_H

#include <QNetworkProxyFactory>
#include <QScriptEngine>

class QNetworkProxyFactoryExendForPAC : public QObject
{


    Q_OBJECT
    Q_PROPERTY( QString config WRITE setConfig )

public:
    QNetworkProxyFactoryExendForPAC();
    ~QNetworkProxyFactoryExendForPAC();

    /**
     * Call this to set the script to be executed. Note that the argument should be
     * the content of the .pac file to be used, not the URL where it is located.
     */
    void setConfig( const QString &config );

    /**
     * Returns the result
     */
    Q_SCRIPTABLE QString findProxyForUrl( const QString &url, const QString &host );

private:
    void install();

    static QScriptValue debug( QScriptContext *context, QScriptEngine *engine );

    /* String myIpAddress */
    static QScriptValue myIpAddress( QScriptContext *context, QScriptEngine *engine );

    /* bool isInNet ipaddress, netaddress, netmask */
    static QScriptValue isInNet( QScriptContext *context, QScriptEngine *engine );

    /* bool shExpMatch url, glob */
    static QScriptValue shExpMatch( QScriptContext *context, QScriptEngine *engine );

    /* string dnsResolve hostname */
    static QScriptValue dnsResolve( QScriptContext *context, QScriptEngine *engine );

private:
    QScriptEngine *engine;
};


#endif // QNETWORKPROXYFACTORYEXENDFORPAC_H
