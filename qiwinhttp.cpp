#ifndef QIWINHTTP
#define QIWINHTTP
#include "qiwinhttp.h"
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>




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
static PtrWinHttpGetProxyForUrl ptrWinHttpGetProxyForUrl = 0;
static PtrWinHttpOpen ptrWinHttpOpen = 0;
static PtrWinHttpGetDefaultProxyConfiguration ptrWinHttpGetDefaultProxyConfiguration = 0;
static PtrWinHttpGetIEProxyConfigForCurrentUser ptrWinHttpGetIEProxyConfigForCurrentUser = 0;
static PtrWinHttpCloseHandle ptrWinHttpCloseHandle = 0;
static bool isInited=false;
static QMutex mutex;
static WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;

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






QiWinHttp::QiWinHttp(QObject *parent) :
    QObject(parent){
}
void QiWinHttp::init(const QString autoConfigUrl){
    QMutexLocker locker(&mutex);
    Q_UNUSED(locker);
    QLibrary lib("winhttp");
    bool isLoaded = lib.load();
    if (!isLoaded){
       qDebug()<<"cannot load winhttp";
       return;
    }else{
        ptrWinHttpOpen = (PtrWinHttpOpen)lib.resolve("WinHttpOpen");
        ptrWinHttpCloseHandle = (PtrWinHttpCloseHandle)lib.resolve("WinHttpCloseHandle");
        ptrWinHttpGetProxyForUrl = (PtrWinHttpGetProxyForUrl)lib.resolve("WinHttpGetProxyForUrl");
        ptrWinHttpGetDefaultProxyConfiguration = (PtrWinHttpGetDefaultProxyConfiguration)lib.resolve("WinHttpGetDefaultProxyConfiguration");
        ptrWinHttpGetIEProxyConfigForCurrentUser = (PtrWinHttpGetIEProxyConfigForCurrentUser)lib.resolve("WinHttpGetIEProxyConfigForCurrentUser");
    }
    memset(&autoProxyOptions, 0, sizeof autoProxyOptions);
    autoProxyOptions.fAutoLogonIfChallenged = false;
    //if (ieProxyConfig.fAutoDetect) {
    //    autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
    //    autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP |
    //                                         WINHTTP_AUTO_DETECT_TYPE_DNS_A;
    //} else {
        autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
        autoProxyOptions.lpszAutoConfigUrl = (LPCWSTR)autoConfigUrl.utf16();
    //}
    isInited = true;
}
QList<QNetworkProxy> QiWinHttp::queryProxy(const QNetworkProxyQuery &query){
    Q_ASSERT(isInited);
    QList<QNetworkProxy> result;
    // try to get the proxy config for the URL
    QUrl url = query.url();
    // url could be empty, e.g. from QNetworkProxy::applicationProxy(), that's fine,
    // we'll still ask for the proxy.
    // But for a file url, we know we don't need one.
    if (url.scheme() == QLatin1String("file") || url.scheme() == QLatin1String("qrc"))
        return (result<<QNetworkProxy::NoProxy);
    if (query.queryType() != QNetworkProxyQuery::UrlRequest) {
        // change the scheme to https, maybe it'll work
        url.setScheme(QLatin1String("https"));
    }


    WINHTTP_PROXY_INFO proxyInfo;
    bool getProxySucceeded = ptrWinHttpGetProxyForUrl(NULL,
                                            (LPCWSTR)url.toString().utf16(),&autoProxyOptions,&proxyInfo);

    if (getProxySucceeded) {
        // yes, we got a config for this URL
        QString proxyBypass = QString::fromWCharArray(proxyInfo.lpszProxyBypass);
        QStringList proxyServerList = splitSpaceSemicolon(QString::fromWCharArray(proxyInfo.lpszProxy));
        if (proxyInfo.lpszProxy)
            GlobalFree(proxyInfo.lpszProxy);
        if (proxyInfo.lpszProxyBypass)
            GlobalFree(proxyInfo.lpszProxyBypass);

        if (isBypassed(query.peerHostName(), splitSpaceSemicolon(proxyBypass)))
            return (result<<QNetworkProxy::NoProxy);
        return parseServerList(query, proxyServerList);
    }

    // GetProxyForUrl failed

    return (result<<QNetworkProxy::NoProxy);
}

#endif
