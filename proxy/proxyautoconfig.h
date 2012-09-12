// -*- c++ -*-

#ifndef PROXYAUTOCONFIG_H
#define PROXYAUTOCONFIG_H

#include <qobject.h>
#include <qscriptvalue.h>
#include <QMutex>

class QScriptContext;
class QScriptEngine;

/**
 * Class implementing the proxy auto-configuration (PAC) JavaScript api.
 */
class ProxyAutoConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString config WRITE setConfig )

public:
    static ProxyAutoConfig *instance();
    ~ProxyAutoConfig();

    /**
     * add by iptton#gmail.com
     *
     * @brief setConfigByUrl
     * @param url
     */
    void setConfigByUrl(const QString &url);

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
    static ProxyAutoConfig *_instancePtr;
    bool _isSettup;
    QMutex _queryMutex;
    ProxyAutoConfig();
    QString currentPacUrl;
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
private slots:
    void onException();
};

#endif // PROXYAUTOCONFIG_H
