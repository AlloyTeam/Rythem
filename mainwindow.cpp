#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "waterfallwindow.h"


#ifdef Q_OS_WIN32
#include "WinInet.h"
#include "winnetwk.h"
#include "windows.h"
#include "proxy/rywinhttp.h"
#endif
#ifdef Q_OS_MAC
#include "proxy/proxyautoconfig.h"
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCDynamicStoreCopySpecific.h>
#include <SystemConfiguration/SCNetworkConfiguration.h>
#endif

#ifdef Q_OS_WIN
#include "zlib/zlib.h"
#else
#include <zlib.h>
#endif

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include "rytablesortfilterproxymodel.h"
#include "ryupdatechecker.h"
#include <QWebFrame>

extern QString version;
extern QString appPath;

QByteArray gzipDecompress(QByteArray data){
    if (data.size() <= 4) {
        qWarning("gUncompress: Input data is truncated");
        return QByteArray();
    }

    QByteArray result;

    int ret;
    z_stream strm;
    static const int CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef*)(data.data());

    ret = inflateInit2(&strm, 15 +  32); // gzip decoding
    if (ret != Z_OK)
        return QByteArray();

    // run inflate()
    do {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef*)(out);

        ret = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(ret != Z_STREAM_ERROR);  // state not clobbered

        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     // and fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return QByteArray();
        }

        result.append(out, CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    inflateEnd(&strm);
    return result;
}
#ifdef Q_OS_WIN
bool setProxy(const QSettings& proxySetting){
    // 感谢maconel的帮助！

    QString proxyAutoConfigURL = proxySetting.value("AutoConfigURL","0").toString();
    int enable = proxySetting.value("ProxyEnable").toInt();
    QString proxyString =proxySetting.value("ProxyServer").toString();


    // init options
    INTERNET_PER_CONN_OPTION_LIST connOptionList;
    INTERNET_PER_CONN_OPTION connOptions[5];
    memset(&connOptionList, 0, sizeof(connOptionList));
    memset(&connOptions, 0, sizeof(connOptions));
    DWORD size = 0;
    size = sizeof(connOptionList);
    connOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    connOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    connOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    connOptions[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
    connOptions[4].dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;
    connOptionList.dwSize = sizeof(connOptionList);
    connOptionList.pszConnection = NULL;
    connOptionList.dwOptionCount = sizeof(connOptions) / sizeof(connOptions[0]);
    connOptionList.dwOptionError = 0;
    connOptionList.pOptions = connOptions;

    // direct
    if(0 == enable){
        connOptions[0].Value.dwValue |= PROXY_TYPE_DIRECT;
    }
    // auto proxy config
    if(proxyAutoConfigURL != "0"){
        connOptions[0].Value.dwValue |= PROXY_TYPE_AUTO_PROXY_URL;
        connOptions[3].Value.pszValue = (WCHAR*)proxyAutoConfigURL.toStdWString().data();
    }
    // TODO: auto detect
    //connOptions[0].Value.dwValue |= PROXY_TYPE_AUTO_DETECT;
    if(1 == enable){
        connOptions[0].Value.dwValue |= PROXY_TYPE_PROXY;
        connOptions[1].Value.pszValue = (WCHAR*)proxyString.toStdWString().data();
        //TODO pass domains
        //connOptions[2].Value.pszValue = L"localhost;127.0.0.1;<local>";
    }

    if (!InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &connOptionList, size)){
        qDebug()<<QString("fail %1").arg(GetLastError());
        return false;
    }
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, NULL);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, NULL);
    return true;
}
#endif
#ifdef Q_OS_MAC

QString getServiceName(){
    // get primary serverId
    char *serviceName = NULL;
    CFStringRef primaryServiceId=NULL;
    SCDynamicStoreRef dynamicStoreDomainState = SCDynamicStoreCreate(NULL,
                                                                     CFSTR("myApplicationName"),
                                                                     NULL,
                                                                     NULL);
    // get primary interface


    if (dynamicStoreDomainState) {
        long n = (long)CFStringGetLength(kSCDynamicStoreDomainState);
        n += CFStringGetLength(kSCCompNetwork);
        n += CFStringGetLength(kSCCompGlobal);
        n += CFStringGetLength(kSCEntNetIPv4);
        n += 3;// for three '/'
        char *netIp4KeyStr = (char*)malloc(sizeof(char)*(n+1));
        sprintf(netIp4KeyStr, "%s/%s/%s/%s",
                CFStringGetCStringPtr(kSCDynamicStoreDomainState, kCFStringEncodingUTF8),
                CFStringGetCStringPtr(kSCCompNetwork, kCFStringEncodingUTF8),
                CFStringGetCStringPtr(kSCCompGlobal, kCFStringEncodingUTF8),
                CFStringGetCStringPtr(kSCEntNetIPv4, kCFStringEncodingUTF8));

        CFStringRef netIpv4KeyCFStr = CFStringCreateWithCString(NULL, netIp4KeyStr, kCFStringEncodingUTF8);
        CFPropertyListRef netIpv4List =SCDynamicStoreCopyValue(dynamicStoreDomainState, netIpv4KeyCFStr);
        if (netIpv4List) {
            primaryServiceId = (CFStringRef)CFDictionaryGetValue((CFDictionaryRef)netIpv4List, kSCDynamicStorePropNetPrimaryService);
        }
        if(primaryServiceId){
            CFRetain(primaryServiceId);
        }
        CFRelease(netIpv4List);
        CFRelease(netIpv4KeyCFStr);
        free(netIp4KeyStr);
    }
    if(!primaryServiceId){
        return QString();
    }
    CFShow(primaryServiceId);
    // Preferences.plist
    SCPreferencesRef prefs = SCPreferencesCreate(NULL, CFSTR("SystemConfiguration"), NULL);
    CFArrayRef services = (CFArrayRef)SCNetworkServiceCopyAll(prefs);
    for(CFIndex i=CFArrayGetCount(services)-1;i>=0;--i){

        SCNetworkServiceRef service = (SCNetworkServiceRef)CFArrayGetValueAtIndex(services,i);
        CFTypeID typeID = SCNetworkServiceGetTypeID();
        unsigned long tmp = (unsigned long)typeID;
        qDebug()<<QString("%1").arg((unsigned long)SCNetworkReachabilityGetTypeID());


        CFStringRef serviceId = SCNetworkServiceGetServiceID(service);
        CFShow(serviceId);
        if( CFStringCompare(primaryServiceId,serviceId,0) == kCFCompareEqualTo ){
            CFStringRef name = SCNetworkServiceGetName(service);// Wi-Fi "Bluetooth DUN" etc.
            if(name){
                unsigned long l = CFStringGetLength(name) + 1;
                serviceName = (char*)malloc(l*sizeof(char));
                sprintf(serviceName, "%s",CFStringGetCStringPtr(name, kCFStringEncodingUTF8));
            }
            //break;
        }else{
            CFStringRef name = SCNetworkServiceGetName(service);
            if(name)
                CFShow(name);
            else
                qDebug()<<"no name";
        }
    }
    CFRelease(primaryServiceId);
    if(!serviceName){
        qDebug()<<"not primary value";
        return QString();
    }
    QString result(serviceName);
    qDebug()<<"primary serviceName:"<<result;
    free(serviceName);
    return result;
}


bool execScript(const QString &script){
    QString service = getServiceName();
    QProcess process;
    qDebug()<<"script"<<script.arg(service);
    QStringList args = script.arg(service).split(" ");
    process.start(script.arg(service));
    QEventLoop eventLoop;
    eventLoop.connect(&process,SIGNAL(finished(int)),&eventLoop,SLOT(quit()));
    eventLoop.exec();

    qDebug()<<"output:\n"<<QString(process.readAllStandardOutput())<<QString(process.readAllStandardError());
    return process.exitCode() == 0;
}

bool setProxyState(bool enable){
    return execScript(QString("./setupproxy setHttpAndHttpsProxyState %1 %2").arg("%1",(enable?"on":"off")));
}

bool setAutoProxyUrl(const QString& url){
    return execScript(QString("./setupproxy setAutoProxyUrl %1 %2").arg("%1",url));
}

bool setProxyForService(const QString& host, const int port){
    return execScript(QString("./setupproxy setHttpAndHttpsProxy %1 %2 %3").arg(QString("%1"),host,QString::number(port)));
}
bool disableAutoProxyAndSetProxyForService(const QString& host, const int port){
    bool a = execScript(QString("./setupproxy setAutoProxyState %1 off"));
    return a & setProxyForService(host,port);
}


bool execProxysetting(const QString &pacUrl=QString()){
    QProcess process;
    QString service = getServiceName();
    QString script = "proxysetting";
    if(pacUrl.isEmpty()){
        script = QString(appPath+"/proxysetting --disablepac %1").arg(service);
    }else{
        script = QString(appPath+"/proxysetting --setpac %1 %2").arg(service,pacUrl);
    }
    qDebug()<<script;
    process.start(script);
    QEventLoop eventLoop;
    eventLoop.connect(&process,SIGNAL(error(QProcess::ProcessError)),&eventLoop,SLOT(quit()));
    eventLoop.connect(&process,SIGNAL(finished(int)),&eventLoop,SLOT(quit()));
    eventLoop.exec();

    qDebug()<<"output:\n"<<QString(process.readAllStandardOutput())<<QString(process.readAllStandardError())
           <<QString("\nretcode:%1\n").arg(process.exitCode());
    return process.exitCode() == 0;
}

bool setPAC(const QString& url){
    return execProxysetting(url);
    //return execScript(QString("networksetup -setautoproxyurl %1 %2").arg("%1",url));
}
bool setMyPAC(){
    return setPAC("http://127.0.0.1:8889/rythem_pac");
    //return setPAC(QString("file://local%1/rythem_pac").arg(appPath));
}

bool disableMyPac(){
    return execProxysetting();
    //return execScript(QString("networksetup -setautoproxystate %1 off"));
}


#endif
// RyJsBridge
RyJsBridge::RyJsBridge(){

}
QString RyJsBridge::doAction(int action, const QString msg, quint64 groupId){
    qDebug()<<"doAction "<<QString::number(action)<< msg<<"groupId"<<QString::number(groupId);
    RyRuleManager *manager = RyRuleManager::instance();
    QSharedPointer<RyRuleProject> pro;
    QSharedPointer<RyRuleGroup> group;
    QSharedPointer<RyRule> rule;
    switch(action){
    case 0://add local group
        group = manager->addGroupToLocalProject(msg);//暂时只允许添加到本地project
        //emit ruleChanged(0,"success");
        if(!group.isNull()){
            qDebug()<<group->toJSON();
            return group->toJSON();
        }
        break;
    case 1://add rule to group
        rule = manager->addRuleToGroup(msg,groupId);
        if(!rule.isNull()){
            //qDebug()<<QString::number(rule->ruleId());
            //return QString::number(rule->ruleId());
            return rule->toJSON();
        }
        break;
    case 2://add remote project
        pro = manager->addRemoteProject(msg);
        if(!pro.isNull()){
            emit ruleChanged(2,pro->toJson());
            return pro->toJson();
        }
        break;
    case 3://update group
        manager->updateRuleGroup(msg,groupId);
        break;
    case 4://update rule
        manager->updateRule(msg,groupId);
        break;
    case 5://remove group
        manager->removeGroup(msg.toULongLong());
        break;
    case 6://remove rule  (6,ruleId,groupId)
        manager->removeRule(msg.toULongLong(),groupId);
        break;
    case 7://import local config
        pro = manager->addLocalProject(msg);
        if(!pro.isNull()){
            emit ruleChanged(2,pro->toJson());
            return pro->toJson();
        }
        break;
    }
    return "";
}
QString RyJsBridge::getFile(){
    return QFileDialog::getOpenFileName();
}
QString RyJsBridge::getDir(){
    return QFileDialog::getExistingDirectory();
}

QString RyJsBridge::getConfigs(){
    RyRuleManager *manager = RyRuleManager::instance();
    QString s = manager->toJson();
    s.remove('\r');
    s.remove('\n');
    s.remove('\t');
    return s;
}

// end of RyJsBridge



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _isUsingCapture(false),
    isFirstTimeToggle(true)

#ifdef Q_OS_WIN
    ,proxySetting("\\HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\",QSettings::NativeFormat)
#endif
{
    ui->setupUi(this);

    pipeTableModel = new RyTableModel(this);
    sortFilterProxyModel = new RyTableSortFilterProxyModel(this);

    ui->tableView->setSortingEnabled(true);
    ui->tableView->setModel(sortFilterProxyModel);
    sortFilterProxyModel->setDynamicSortFilter(true);
    sortFilterProxyModel->setSourceModel(pipeTableModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setColumnWidth(0,30);
    ui->tableView->setColumnWidth(1,30);
    ui->tableView->setColumnWidth(2,50);
    ui->tableView->setColumnWidth(3,50);
    ui->tableView->setColumnWidth(7,50);
    ui->tableView->verticalHeader()->setDefaultSectionSize(20);
    ui->tableView->verticalHeader()->hide();


    _jsBridge = new RyJsBridge();
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    addJsObject();
    connect(ui->webView->page()->mainFrame(),&QWebFrame::javaScriptWindowObjectCleared,this,&MainWindow::addJsObject);
    //修复proxy服务器过完启动导致配置页面空白的问题 延迟一秒加载
    QTimer::singleShot(1000, this, SLOT(loadConfigPage()));

    // should use slot to do this
    //ui->composer->setupProxy(RyProxyServer::instance()->serverAddress().toString(),
    //                         RyProxyServer::instance()->serverPort());
    ui->composer->setupProxy("127.0.0.1",
                             8889);
    _itemSelectModel = ui->tableView->selectionModel();

    connect(ui->tableView,SIGNAL(doubleClicked(QModelIndex)),SLOT(onItemDoubleClicked(QModelIndex)));
    //connect(_itemSelectModel,SIGNAL(currentChanged(QModelIndex,QModelIndex)),SLOT(onSelectionChange(QModelIndex)));
    connect(_itemSelectModel,SIGNAL(selectionChanged(QItemSelection,QItemSelection)),SLOT(onSelectionChange(QItemSelection,QItemSelection)));

    //ui->tableView->setItemDelegate();
    createMenus();
    //toggleCapture();

    connect(ui->ActionCapture,SIGNAL(triggered()),SLOT(toggleCapture()));
    connect(ui->actionRemoveAll,SIGNAL(triggered()),this,SLOT(onActionRemoveAll()));
    connect(ui->actionWaterfall, SIGNAL(triggered()), this, SLOT(onWaterfallActionTriggered()));

    checker = new RyUpdateChecker(this);
    QTimer timer;
    timer.singleShot(1000,this,SLOT(checkNewVersion()));
    //checkNewVersion();

#ifdef Q_OS_MAC
    if(appPath.startsWith("/Volumes")){
        ui->ActionCapture->setEnabled(false);
        ui->ActionCapture->setToolTip("to ENABLE this.Pls move Rythem to /Applications directory");
        QMessageBox::information(this,
                                 tr("-"),
                                 tr("Please drag to Applications dir first \n\n otherwise creat replace rule will cause crash on MacOS 10.8 (Mountain Lion)"),
                                 QMessageBox::Ok);
    }
#endif
}

MainWindow::~MainWindow()
{
    RyProxyServer* server = RyProxyServer::instance();
    server->close();
    delete _jsBridge;
    delete ui;
}


void MainWindow::checkNewVersion(){

#ifdef Q_OS_WIN
    //toggleCapture();
#endif
    checker->check();
}

void MainWindow::createMenus(){
    _fileMenu = menuBar()->addMenu(tr("&File"));
    _importSessionsAct = _fileMenu->addAction(tr("&import session..."));
    _filterNoImagesAct = _fileMenu->addAction(tr("hide image requests"));
    _filterNoImagesAct->setCheckable(true);
    _filterNo304sAct = _fileMenu->addAction(tr("hide 304s"));
    _filterNo304sAct->setCheckable(true);
    _filterShowMatchOnlyAct = _fileMenu->addAction(tr("show matching sessions only"));
    _filterShowMatchOnlyAct->setCheckable(true);
    _hideConnectTunnelAct = _fileMenu->addAction(tr("hide connect tunnels"));
    _hideConnectTunnelAct->setCheckable(true);

    connect(_fileMenu,SIGNAL(triggered(QAction*)),SLOT(onAction(QAction*)));
}

void MainWindow::importSessions(){
    QFileDialog dialog;
    QString fileName = dialog.getOpenFileName(this,tr("select file to open"),"","Archiev Files(*.saz *.zip)");
    QuaZip zip(fileName);
    if(!zip.open(QuaZip::mdUnzip)){
        qDebug()<<"cannot open "<<fileName;
        return;
    }
    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    RyPipeData_ptr pipeData;

    QString name;
    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {

        if (!zip.getCurrentFileInfo(&info)) {
            qWarning("testRead(): getCurrentFileInfo(): %d\n", zip.getZipError());
            return;
        }

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("testRead(): file.open(): %d", file.getZipError());
            return;
        }

        name = file.getActualFileName();
        QByteArray ba = file.readAll();
        if(name.endsWith("_c.txt")){
            //pipeData.clear();
            pipeData = RyPipeData_ptr(new RyPipeData(0,0));
            pipeData->isImported = true;
            pipeData->parseRequest(&ba);
            onNewPipe(pipeData);
        }else if(name.endsWith("_s.txt")){
            pipeData->parseResponse(&ba);
            onPipeUpdate(pipeData);
            pipeData.clear();
        }

        if (file.getZipError() != UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return ;
        }

        file.close();

        if (file.getZipError() != UNZ_OK) {
            qWarning("testRead(): file.close(): %d", file.getZipError());
            return ;
        }

    }

    zip.close();

}

void MainWindow::onPipeUpdate(RyPipeData_ptr pipeData){
    //qDebug()<<"updated "<<pipeData->responseStatus;
    //pipeTableModel->updateItem(pipeData);
    sortFilterProxyModel->updateItem(pipeData);
}

void MainWindow::onNewPipe(RyPipeData_ptr pipeData){
    //pipeTableModel
    //pipeTableModel->addItem(pipeData);
    sortFilterProxyModel->addItem(pipeData);
}

void MainWindow::onSelectionChange(QItemSelection selected,QItemSelection){
    if(selected.indexes().isEmpty()){
        ui->actionWaterfall->setEnabled(false);
    }else{
        if(ui->toolTabs->currentWidget() == ui->inspectorTab){
            QModelIndex index = selected.indexes().first();
            onItemDoubleClicked(index);
        }
        ui->actionWaterfall->setEnabled(true);
    }
}

void MainWindow::onItemDoubleClicked(QModelIndex topLeft){
    RyPipeData_ptr data = sortFilterProxyModel->getItem(topLeft);
    if(data.isNull()){
        qDebug()<<QString("isNull %1").arg(topLeft.row());
        return;
    }

    ui->toolTabs->setCurrentWidget(ui->inspectorTab);
    ui->requestInspectorTabs->setCurrentWidget(ui->requestInspectorTextview);

    // TODO: test
    ui->requestTextEdit->setPlainText(data->requestHeaderRawData() +"\r\n\r\n"+data->requestBodyRawData() );



    QByteArray encoding("UTF-8");
    QString contentType = data->getResponseHeader("Content-Type");
    int encodingIndex = contentType.indexOf("charset=");
    if(encodingIndex!=-1){
        encoding.clear();
        encoding.append(contentType.mid(encodingIndex + 8));
    }
    bool isEncrypted = !data->getResponseHeader("Content-Encoding").isEmpty();




    QByteArray decrypedData =
            (data->isResponseChunked()?
              data->responseBodyRawDataUnChunked()
              :data->responseBodyRawData());
    if(isEncrypted){
        decrypedData = gzipDecompress(decrypedData);
    }

    //TODO: gif
    //QMovie *movie = ui->label->movie();
    //if(movie){
    //    delete movie;
    //}
    if(data->getResponseHeader("Content-Type").toLower().indexOf("image")!=-1){
        if(!decrypedData.isEmpty()){
            if(data->getResponseHeader("Content-Type").toLower().indexOf("gif")!=-1){
                //QBuffer *data = new QBuffer(&decrypedData);
                //QMovie *movie = new QMovie(data);
                //ui->label->setMovie(movie);
                //TODO: start and delete movie..
            }else{
                QPixmap pixmap;
                pixmap.loadFromData(decrypedData);
                ui->label->setPixmap(pixmap);
            }
            ui->responseInspectorTabs->setCurrentWidget(ui->responseInspectorImageView);
        }else{
            ui->responseInspectorTabs->setCurrentWidget(ui->responseInspectorTextView);
            ui->label->clear();
        }
    }else{
        ui->label->clear();
        ui->responseInspectorTabs->setCurrentWidget(ui->responseInspectorTextView);
    }


    // show in textview
    //TODO for gbk
    ui->responseTextEdit->setPlainText(QString((
                                              data->responseHeaderRawData()
                                            +"\r\n\r\n"
                                            + decrypedData
                                           ).data())
                                       );



    data.clear();
}

void MainWindow::onMessageFromOtherInstance(){
    setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();
    activateWindow();
}

void MainWindow::onWaterfallActionTriggered(){
    QModelIndexList list = ui->tableView->selectionModel()->selectedRows();
    QList<RyPipeData_ptr> pipes;
    QListIterator<QModelIndex> it(list);
    while(it.hasNext()){
        QModelIndex i = it.next();
        RyPipeData_ptr data = sortFilterProxyModel->getItem(i);
        pipes.append(data);
    }

    if(pipes.length()){
        //deleteLater would be call when win close, don't worry about the memory
        WaterfallWindow *win = new WaterfallWindow();
        win->setPipeData(pipes);
        win->show();
    }
    else{
        QMessageBox box;
        box.setText("Please select some network request first");
        box.exec();
    }
}

void MainWindow::toggleProxy(){
    QMutexLocker locker(&proxyMutex);
#ifdef Q_OS_MAC
    bool setProxySuccess = false;
    if(_isUsingCapture){
        if(_previousProxyInfo.isUsingPac == "1"){
            setProxySuccess = setPAC(_previousProxyInfo.pacUrl);
        }else{
            setProxySuccess = disableMyPac();
        }
    }else{
        if(isFirstTimeToggle){
            isFirstTimeToggle = false;
            CFDictionaryRef proxies = SCDynamicStoreCopyProxies(NULL);
            //CFShow(proxies);
            int isPacEnabled = 0;
            //CFStringRef CFPacUrl = (CFStringRef)CFDictionaryGetValue(proxies, kSCPropNetProxiesProxyAutoConfigURLString);
            //if(CFPacUrl){
            //    _previousProxyInfo.pacUrl = QString::fromUtf8( CFStringGetCStringPtr(CFPacUrl,kCFStringEncodingUTF8) );
            //}
            if (proxies){
                CFNumberRef pacEnabled;
                //kSCPropNetProxiesHTTPSProxy
                if ((pacEnabled = (CFNumberRef)CFDictionaryGetValue(proxies, kSCPropNetProxiesProxyAutoConfigEnable))){
                    if (CFNumberGetValue(pacEnabled, kCFNumberIntType, &pacEnabled) && pacEnabled){
                        isPacEnabled = 1;
                        _previousProxyInfo.isUsingPac = QString("1");
                        CFStringRef CFPacUrl = (CFStringRef)CFDictionaryGetValue(proxies, kSCPropNetProxiesProxyAutoConfigURLString);
                        _previousProxyInfo.pacUrl = QString::fromUtf8( CFStringGetCStringPtr(CFPacUrl,kCFStringEncodingUTF8) );
                    }
                }
            }
            //qDebug()<<QString("pacEnabeld %1").arg(isPacEnabled);
            if(isPacEnabled){
                qDebug()<<_previousProxyInfo.pacUrl;
                ProxyAutoConfig::instance()->setConfigByUrl(_previousProxyInfo.pacUrl);
            }else{
                CFNumberRef httpEnabled = (CFNumberRef)CFDictionaryGetValue(proxies, kSCPropNetProxiesHTTPEnable);
                CFNumberRef httpsEnabled = (CFNumberRef)CFDictionaryGetValue(proxies, kSCPropNetProxiesHTTPSEnable);
                int tmp;
                if (httpEnabled && CFNumberGetValue(httpEnabled, kCFNumberIntType, &tmp) && tmp){
                    CFStringRef host = (CFStringRef)CFDictionaryGetValue(proxies, kSCPropNetProxiesHTTPProxy);
                    CFNumberRef port = (CFNumberRef)CFDictionaryGetValue(proxies, kSCPropNetProxiesHTTPPort);
                    QString hostQ = QString::fromUtf8( CFStringGetCStringPtr(host,kCFStringEncodingUTF8) );
                    UInt64 portQ=20;
                    CFNumberGetValue(port, kCFNumberSInt64Type, &portQ);
                    QString proxyStr = QString("Proxy %1:%2").arg(hostQ).arg(portQ);
                    //qDebug()<<proxyStr;
                    ProxyAutoConfig::instance()->setHttpProxy(proxyStr);
                }
                if (httpsEnabled && CFNumberGetValue(httpsEnabled, kCFNumberIntType, &tmp) && tmp){
                    CFStringRef host = (CFStringRef)CFDictionaryGetValue(proxies, kSCPropNetProxiesHTTPSProxy);
                    CFNumberRef port = (CFNumberRef)CFDictionaryGetValue(proxies, kSCPropNetProxiesHTTPSPort);
                    QString hostQ = QString::fromUtf8( CFStringGetCStringPtr(host,kCFStringEncodingUTF8) );
                    UInt64 portQ=20;
                    CFNumberGetValue(port, kCFNumberSInt64Type, &portQ);
                    QString proxyStr = QString("Proxy %1:%2").arg(hostQ).arg(portQ);
                    ProxyAutoConfig::instance()->setHttpProxy(proxyStr);
                }

            }
            CFRelease(proxies);
        }
        setProxySuccess = setMyPAC();
    }

    if(!setProxySuccess){// hack..
        _isUsingCapture = !_isUsingCapture;
    }

#endif

#ifdef Q_WS_WIN32
    RyWinHttp::init();
    if(_isUsingCapture){
        ui->ActionCapture->setText(tr("start capture"));
        proxySetting.setValue("ProxyEnable",_previousProxyInfo.enable);
        if(_previousProxyInfo.enable != 0){
            proxySetting.setValue("ProxyServer",_previousProxyInfo.proxyString);
        }
        if( _previousProxyInfo.isUsingPac != "0"){
            proxySetting.setValue("AutoConfigURL",_previousProxyInfo.isUsingPac);
        }
    }else{
        ui->ActionCapture->setText(tr("stop capture"));
        _previousProxyInfo.isUsingPac = proxySetting.value("AutoConfigURL","0").toString();
        _previousProxyInfo.enable = proxySetting.value("ProxyEnable").toInt();
        _previousProxyInfo.proxyString =proxySetting.value("ProxyServer").toString();
        QString proxyServer="127.0.0.1:8889";
        proxySetting.remove("AutoConfigURL");
        proxySetting.setValue("ProxyEnable",QVariant(1));
        proxySetting.setValue("ProxyServer",proxyServer);
    }
    if(!setProxy(proxySetting)){// hack..
        _isUsingCapture = !_isUsingCapture;
    }
#endif
    _isUsingCapture = !_isUsingCapture;
    if(_isUsingCapture){
        ui->ActionCapture->setChecked(true);
        ui->ActionCapture->setText(tr("stop capture"));
        ui->ActionCapture->setToolTip(tr("stop capture"));
    }else{
        ui->ActionCapture->setChecked(false);
        ui->ActionCapture->setText(tr("start capture"));
        ui->ActionCapture->setToolTip(tr("start capture"));
    }
}

void MainWindow::toggleCapture(){
    ui->ActionCapture->setEnabled(false);
    toggleProxy();
    ui->ActionCapture->setEnabled(true);
}


void MainWindow::closeEvent(QCloseEvent *event){
    if(_isUsingCapture){
        toggleCapture();
    }
    QMainWindow::closeEvent(event);
    qDebug()<<"close event";
    qApp->quit();
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event){
    event->accept();
}

void MainWindow::addJsObject(){
    ui->webView->page()->mainFrame()->addToJavaScriptWindowObject(QString("App"),_jsBridge);
}

void MainWindow::onAction(QAction *action){
    if(action == _filterNoImagesAct){
        if(action->isChecked()){
            sortFilterProxyModel->setFilter(sortFilterProxyModel->filter() | RyTableSortFilterProxyModel::NoImageFilter);
        }else{
            sortFilterProxyModel->setFilter(sortFilterProxyModel->filter() & (~RyTableSortFilterProxyModel::NoImageFilter));
        }
    }else if(action == _filterNo304sAct){
        if(action->isChecked()){
            sortFilterProxyModel->setFilter(sortFilterProxyModel->filter() | RyTableSortFilterProxyModel::No304Filter);
        }else{
            sortFilterProxyModel->setFilter(sortFilterProxyModel->filter() & (~RyTableSortFilterProxyModel::No304Filter));
        }
    }else if(action == _importSessionsAct){
        importSessions();
    }else if(action == _filterShowMatchOnlyAct){
        if(action->isChecked()){
            sortFilterProxyModel->setFilter(sortFilterProxyModel->filter() | RyTableSortFilterProxyModel::OnlyMatchingFilter);
        }else{
            sortFilterProxyModel->setFilter(sortFilterProxyModel->filter() & (~RyTableSortFilterProxyModel::OnlyMatchingFilter));
        }
    }else if(action == _hideConnectTunnelAct){
        if(action->isChecked()){
            sortFilterProxyModel->setFilter(sortFilterProxyModel->filter() | RyTableSortFilterProxyModel::HideTunnelFilter);
        }else{
            sortFilterProxyModel->setFilter(sortFilterProxyModel->filter() & (~RyTableSortFilterProxyModel::HideTunnelFilter));
        }
    }
}

void MainWindow::on_actionLongCache_triggered(){
    RyRuleManager::instance()->toggleLongCache();
}

void MainWindow::onActionRemoveAll(){
    pipeTableModel->removeAllItem();
    sortFilterProxyModel->removeAllItem();
    ui->actionWaterfall->setEnabled(false);
}

void MainWindow::on_actionDebug_triggered(){
    qDebug()<<RyRuleManager::instance()->toJson(true);
}

void MainWindow::on_actionCheckNew_triggered(){
    checkNewVersion();
}

void MainWindow::loadConfigPage(){
    ui->webView->load(QUrl("http://127.0.0.1:8889/manager/RythemManagerUI/rules.html"));
}

