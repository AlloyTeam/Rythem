#ifndef RYWINHTTP
#define RYWINHTTP
#include "rywinhttp.h"
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>
#include <QSettings>

#include <QDateTime>





//=== from source code

// We don't want to include winhttp.h because that's not
// present in some Windows SDKs (I don't know why)
// So, instead, copy the definitions here

typedef struct {
  DWORD dwFlags;
  DWORD dwAutoDetectFlags;
  LPCWSTR lpszAutoConfigUrl;
  LPVOID lpvReserved;
  DWORD dwReserved;
  BOOL fAutoLogonIfChallenged;
} WINHTTP_AUTOPROXY_OPTIONS;

typedef struct _winhttp_proxy_info {
  DWORD dwAccessType;
  LPWSTR lpszProxy;
  LPWSTR lpszProxyBypass;
} WINHTTP_PROXY_INFO;

typedef struct {
  BOOL fAutoDetect;
  LPWSTR lpszAutoConfigUrl;
  LPWSTR lpszProxy;
  LPWSTR lpszProxyBypass;
} WINHTTP_CURRENT_USER_IE_PROXY_CONFIG;

#define WINHTTP_AUTOPROXY_AUTO_DETECT           0x00000001
#define WINHTTP_AUTOPROXY_CONFIG_URL            0x00000002

#define WINHTTP_AUTO_DETECT_TYPE_DHCP           0x00000001
#define WINHTTP_AUTO_DETECT_TYPE_DNS_A          0x00000002

#define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY               0
#define WINHTTP_ACCESS_TYPE_NO_PROXY                    1
#define WINHTTP_ACCESS_TYPE_NAMED_PROXY                 3

#define WINHTTP_NO_PROXY_NAME     NULL
#define WINHTTP_NO_PROXY_BYPASS   NULL

#define WINHTTP_ERROR_BASE                      12000
#define ERROR_WINHTTP_LOGIN_FAILURE             (WINHTTP_ERROR_BASE + 15)
#define ERROR_WINHTTP_AUTODETECTION_FAILED      (WINHTTP_ERROR_BASE + 180)



typedef BOOL (WINAPI * PtrWinHttpGetProxyForUrl)(HINTERNET, LPCWSTR, WINHTTP_AUTOPROXY_OPTIONS*, WINHTTP_PROXY_INFO*);
typedef HINTERNET (WINAPI * PtrWinHttpOpen)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR,DWORD);
typedef BOOL (WINAPI * PtrWinHttpGetDefaultProxyConfiguration)(WINHTTP_PROXY_INFO*);
typedef BOOL (WINAPI * PtrWinHttpGetIEProxyConfigForCurrentUser)(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG*);
typedef BOOL (WINAPI * PtrWinHttpCloseHandle)(HINTERNET);
typedef BOOL (WINAPI * PtrWinHttpSetDefaultProxyConfiguration)(WINHTTP_PROXY_INFO*);
static PtrWinHttpGetProxyForUrl ptrWinHttpGetProxyForUrl = 0;
static PtrWinHttpOpen ptrWinHttpOpen = 0;
static PtrWinHttpGetDefaultProxyConfiguration ptrWinHttpGetDefaultProxyConfiguration = 0;
static PtrWinHttpGetIEProxyConfigForCurrentUser ptrWinHttpGetIEProxyConfigForCurrentUser = 0;
static PtrWinHttpCloseHandle ptrWinHttpCloseHandle = 0;
static PtrWinHttpSetDefaultProxyConfiguration ptrWinHttpSetDefaultProxyConfiguration = 0;
static QMutex mutex;















static QStringList splitSpaceSemicolon(const QString &source)
{
    QStringList list;
    int start = 0;
    int end;
    while (true) {
        int space = source.indexOf(QLatin1Char(' '), start);
        int semicolon = source.indexOf(QLatin1Char(';'), start);
        end = space;
        if (semicolon != -1 && (end == -1 || semicolon < end))
            end = semicolon;

        if (end == -1) {
            if (start != source.length())
                list.append(source.mid(start));
            return list;
        }
        if (start != end)
            list.append(source.mid(start, end - start));
        start = end + 1;
    }
    return list;
}

static bool isBypassed(const QString &host, const QStringList &bypassList)
{
    if (host.isEmpty())
        return true;

    bool isSimple = !host.contains(QLatin1Char('.')) && !host.contains(QLatin1Char(':'));

    QHostAddress ipAddress;
    bool isIpAddress = ipAddress.setAddress(host);

    // does it match the list of exclusions?
    foreach (const QString &entry, bypassList) {
        if (isSimple && entry == QLatin1String("<local>"))
            return true;
        if (isIpAddress && ipAddress.isInSubnet(QHostAddress::parseSubnet(entry))) {
            return true;        // excluded
        } else {
            // do wildcard matching
            QRegExp rx(entry, Qt::CaseInsensitive, QRegExp::Wildcard);
            if (rx.exactMatch(host))
                return true;
        }
    }

    // host was not excluded
    return false;
}

static QList<QNetworkProxy> parseServerList(const QNetworkProxyQuery &query, const QStringList &proxyList)
{
    // Reference documentation from Microsoft:
    // http://msdn.microsoft.com/en-us/library/aa383912(VS.85).aspx
    //
    // According to the website, the proxy server list is
    // one or more of the space- or semicolon-separated strings in the format:
    //   ([<scheme>=][<scheme>"://"]<server>[":"<port>])

    QList<QNetworkProxy> result;
    //qDebug()<<"parseServerList"<<result.count();
    foreach (const QString &entry, proxyList) {
        int server = 0;

        int pos = entry.indexOf(QLatin1Char('='));
        if (pos != -1) {
            QStringRef scheme = entry.leftRef(pos);
            if (scheme != query.protocolTag())
                continue;

            server = pos + 1;
        }

        QNetworkProxy::ProxyType proxyType = QNetworkProxy::HttpProxy;
        quint16 port = 8080;

        pos = entry.indexOf(QLatin1String("://"), server);
        if (pos != -1) {
            QStringRef scheme = entry.midRef(server, pos - server);
            if (scheme == QLatin1String("http") || scheme == QLatin1String("https")) {
                // no-op
                // defaults are above
            } else if (scheme == QLatin1String("socks") || scheme == QLatin1String("socks5")) {
                proxyType = QNetworkProxy::Socks5Proxy;
                port = 1080;
            } else {
                // unknown proxy type
                continue;
            }

            server = pos + 3;
        }

        pos = entry.indexOf(QLatin1Char(':'), server);
        if (pos != -1) {
            bool ok;
            uint value = entry.mid(pos + 1).toUInt(&ok);
            if (!ok || value > 65535)
                continue;       // invalid port number

            port = value;
        } else {
            pos = entry.length();
        }

        result << QNetworkProxy(proxyType, entry.mid(server, pos - server), port);
    }

    return result;
}

//===





RyWinHttp::RyWinHttp(QObject *parent) :
    QObject(parent){
}

//== from souce

class QWindowsSystemProxy
{
public:
    QWindowsSystemProxy();
    ~QWindowsSystemProxy();
    void init();

    QMutex mutex;

    HINTERNET hHttpSession;
    WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;
    WINHTTP_PROXY_INFO proxyInfo;
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig;

    QString autoConfigUrl;
    QStringList proxyServerList;
    QStringList proxyBypass;
    QList<QNetworkProxy> defaultResult;

    bool initialized;
    bool functional;
    bool isAutoConfig;
    bool isAutoDetect;
};

Q_GLOBAL_STATIC(QWindowsSystemProxy, systemProxy)

QWindowsSystemProxy::QWindowsSystemProxy()
    : initialized(false), functional(false), isAutoConfig(false)
{
    defaultResult << QNetworkProxy::NoProxy;
    hHttpSession = NULL;
}

QWindowsSystemProxy::~QWindowsSystemProxy()
{
    if (hHttpSession)
        ptrWinHttpCloseHandle(hHttpSession);
}

void QWindowsSystemProxy::init()
{
    if (initialized)
        return;
    initialized = true;

#ifdef Q_OS_WINCE
    // Windows CE does not have any of the following API
    return;
#else
    // load the winhttp.dll library
    QLibrary lib("winhttp");
    Q_ASSERT(lib.load());
    if (!lib.load()){
        return;                 // failed to load
    }

    ptrWinHttpOpen = (PtrWinHttpOpen)lib.resolve("WinHttpOpen");
    ptrWinHttpCloseHandle = (PtrWinHttpCloseHandle)lib.resolve("WinHttpCloseHandle");
    ptrWinHttpGetProxyForUrl = (PtrWinHttpGetProxyForUrl)lib.resolve("WinHttpGetProxyForUrl");
    ptrWinHttpGetDefaultProxyConfiguration = (PtrWinHttpGetDefaultProxyConfiguration)lib.resolve("WinHttpGetDefaultProxyConfiguration");
    ptrWinHttpGetIEProxyConfigForCurrentUser = (PtrWinHttpGetIEProxyConfigForCurrentUser)lib.resolve("WinHttpGetIEProxyConfigForCurrentUser");
    ptrWinHttpSetDefaultProxyConfiguration = (PtrWinHttpSetDefaultProxyConfiguration)lib.resolve("WinHttpSetDefaultProxyConfiguration");

    // Try to obtain the Internet Explorer configuration.
    if (ptrWinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig)) {
        if (ieProxyConfig.lpszAutoConfigUrl) {
            autoConfigUrl = QString::fromWCharArray(ieProxyConfig.lpszAutoConfigUrl);
            //GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
        }
        if (ieProxyConfig.lpszProxy) {
            // http://msdn.microsoft.com/en-us/library/aa384250%28VS.85%29.aspx speaks only about a "proxy URL",
            // not multiple URLs. However we tested this and it can return multiple URLs. So we use splitSpaceSemicolon
            // on it.
            proxyServerList = splitSpaceSemicolon(QString::fromWCharArray(ieProxyConfig.lpszProxy));
            //GlobalFree(ieProxyConfig.lpszProxy);
        }
        if (ieProxyConfig.lpszProxyBypass) {
            proxyBypass = splitSpaceSemicolon(QString::fromWCharArray(ieProxyConfig.lpszProxyBypass));
            //GlobalFree(ieProxyConfig.lpszProxyBypass);
        }
    }

    hHttpSession = NULL;
    if (ieProxyConfig.fAutoDetect || !autoConfigUrl.isEmpty()) {
        // using proxy autoconfiguration
        proxyServerList.clear();
        proxyBypass.clear();

        // open the handle and obtain the options
        hHttpSession = ptrWinHttpOpen(L"Qt System Proxy access/1.0",
                                      WINHTTP_ACCESS_TYPE_NO_PROXY,
                                      WINHTTP_NO_PROXY_NAME,
                                      WINHTTP_NO_PROXY_BYPASS,
                                      0);
        if (!hHttpSession)
            return;

        isAutoConfig = true;
        memset(&autoProxyOptions, 0, sizeof autoProxyOptions);
        autoProxyOptions.fAutoLogonIfChallenged = false;
        if (ieProxyConfig.fAutoDetect) {
            autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
            autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP |
                                                 WINHTTP_AUTO_DETECT_TYPE_DNS_A;
        } else {
            autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
            autoProxyOptions.lpszAutoConfigUrl = (LPCWSTR)autoConfigUrl.utf16();
        }
    } else {
        // not auto-detected
        // attempt to get the static configuration instead
        if (ptrWinHttpGetDefaultProxyConfiguration(&proxyInfo) &&
            proxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY) {
            // we got information from the registry
            // overwrite the IE configuration, if any

            proxyBypass = splitSpaceSemicolon(QString::fromWCharArray(proxyInfo.lpszProxyBypass));
            proxyServerList = splitSpaceSemicolon(QString::fromWCharArray(proxyInfo.lpszProxy));
        }
        /*
        if (proxyInfo.lpszProxy)
            GlobalFree(proxyInfo.lpszProxy);
        if (proxyInfo.lpszProxyBypass)
            GlobalFree(proxyInfo.lpszProxyBypass);
        */
    }

    functional = isAutoConfig || !proxyServerList.isEmpty();
#endif
}

//== end 1
QList<QNetworkProxy> RyWinHttp::queryProxy(const QNetworkProxyQuery &query){
    //qDebug()<<"querying "<<query.url().toString();


    QWindowsSystemProxy *sp = systemProxy();
    if (!sp)
        return QList<QNetworkProxy>() << QNetworkProxy();

    //QMutexLocker locker(&sp->mutex);
    //Q_UNUSED(locker);

    //qint64 beginTime = QDateTime::currentDateTime().toMSecsSinceEpoch();


    //sp->init();
    //locker.unlock();
    if (!sp->functional)
        return sp->defaultResult;
    //qDebug()<<"isautoconfig?"<<(sp->isAutoConfig?"a":"no");
    if (sp->isAutoConfig) {
        WINHTTP_PROXY_INFO proxyInfo;

        // try to get the proxy config for the URL
        QUrl url = query.url();
        // url could be empty, e.g. from QNetworkProxy::applicationProxy(), that's fine,
        // we'll still ask for the proxy.
        // But for a file url, we know we don't need one.
        if (url.scheme() == QLatin1String("file") || url.scheme() == QLatin1String("qrc")){
            //qDebug()<<"queryProxyTime:"<<(QDateTime::currentDateTime().toMSecsSinceEpoch() - beginTime);
            return sp->defaultResult;
        }
        if (query.queryType() != QNetworkProxyQuery::UrlRequest) {
            // change the scheme to https, maybe it'll work
            url.setScheme(QLatin1String("https"));
        }
        //qDebug()<<"getting..";
        //printf("%ls",sp->autoProxyOptions.lpszAutoConfigUrl);
        //LPCWSTR theUrl= L"http://gdutbbs.com/";
        //LPCWSTR theUrl = (LPCWSTR)url.toString().utf16();

        //sp->autoProxyOptions.lpszAutoConfigUrl = theAuto;
        bool getProxySucceeded = ptrWinHttpGetProxyForUrl(sp->hHttpSession,
                                                (LPCWSTR)url.toString().utf16(),
                                                &sp->autoProxyOptions
                                                          /*theAuto*/,
                                                &proxyInfo);
        DWORD getProxyError = GetLastError();
        if (!getProxySucceeded
            && (ERROR_WINHTTP_LOGIN_FAILURE == getProxyError)) {
            // We first tried without AutoLogon, because this might prevent caching the result.
            // But now we've to enable it (http://msdn.microsoft.com/en-us/library/aa383153%28v=VS.85%29.aspx)
            sp->autoProxyOptions.fAutoLogonIfChallenged = TRUE;
            getProxySucceeded = ptrWinHttpGetProxyForUrl(sp->hHttpSession,
                                               (LPCWSTR)url.toString().utf16(),
                                                &sp->autoProxyOptions,
                                                &proxyInfo);
            getProxyError = GetLastError();
        }

        if (getProxySucceeded) {
            // yes, we got a config for this URL
            QString proxyBypass = QString::fromWCharArray(proxyInfo.lpszProxyBypass);
            QStringList proxyServerList = splitSpaceSemicolon(QString::fromWCharArray(proxyInfo.lpszProxy));
            if (proxyInfo.lpszProxy)
                //GlobalFree(proxyInfo.lpszProxy);
            if (proxyInfo.lpszProxyBypass)
                //GlobalFree(proxyInfo.lpszProxyBypass);

            if (isBypassed(query.peerHostName(), splitSpaceSemicolon(proxyBypass))){
                //qDebug()<<"queryProxyTime:"<<(QDateTime::currentDateTime().toMSecsSinceEpoch() - beginTime);
                return sp->defaultResult;
            }
            //qDebug()<<"queryProxyTime:"<<(QDateTime::currentDateTime().toMSecsSinceEpoch() - beginTime);
            return parseServerList(query, proxyServerList);
        }

        // GetProxyForUrl failed

        if (ERROR_WINHTTP_AUTODETECTION_FAILED == getProxyError) {
            //No config file could be retrieved on the network.
            //Don't search for it next time again.
            sp->isAutoConfig = false;
        }
        //qDebug()<<"queryProxyTime:"<<(QDateTime::currentDateTime().toMSecsSinceEpoch() - beginTime);
        return sp->defaultResult;
    }

    // static configuration
    if (isBypassed(query.peerHostName(), sp->proxyBypass)){
        //qDebug()<<"queryProxyTime:"<<(QDateTime::currentDateTime().toMSecsSinceEpoch() - beginTime);
        return sp->defaultResult;
    }

    QList<QNetworkProxy> result = parseServerList(query, sp->proxyServerList);
    // In some cases, this was empty. See SF task 00062670
    if (result.isEmpty()){
        //qDebug()<<"queryProxyTime:"<<(QDateTime::currentDateTime().toMSecsSinceEpoch() - beginTime);
        return sp->defaultResult;
    }

    //qDebug()<<"queryProxyTime:"<<(QDateTime::currentDateTime().toMSecsSinceEpoch() - beginTime);
    return result;
}

void RyWinHttp::setupProxy(QString host,int port){
    QWindowsSystemProxy *sp = systemProxy();
    if (!sp)
        return ;

    QMutexLocker locker(&sp->mutex);
    Q_UNUSED(locker);
}
void RyWinHttp::restoreProxy(){
    QWindowsSystemProxy *sp = systemProxy();
    if (!sp)
        return ;

    QMutexLocker locker(&sp->mutex);
    Q_UNUSED(locker);
    //sp->proxyInfo.lpszProxyBypass = L"<local>;*qq.com;<-loopback>;192.168.*.*";
    //swprintf_s(sp->proxyInfo.lpszProxyBypass, 25, L"<local>");
    qDebug()<<"setting proxy config:"
           <<QString::fromWCharArray(sp->proxyInfo.lpszProxyBypass)
            <<QString::fromWCharArray(sp->proxyInfo.lpszProxyBypass);
    BOOL setSuccess = ptrWinHttpSetDefaultProxyConfiguration(&sp->proxyInfo);
    if(!setSuccess){
        qDebug()<<"error when setting:"<<GetLastError();
    }
}

void RyWinHttp::init(){
    QWindowsSystemProxy *sp = systemProxy();
    if (!sp)
        return;
    QMutexLocker locker(&sp->mutex);
    Q_UNUSED(locker);
    sp->init();
}
#endif

