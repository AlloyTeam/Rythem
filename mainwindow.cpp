#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "waterfallwindow.h"

#include <QTcpServer>
#include "proxy/rypipedata.h"
#include "proxy/ryproxyserver.h"
#include <QSettings>
#include <QVariant>
#include <QUrl>
#include <QNetworkConfigurationManager>
#include <QMap>
#include <QList>
#include <QMessageBox>

#ifdef Q_WS_WIN32
#include "wininet.h"
#include "winnetwk.h"
#include "proxy/rywinhttp.h"
#endif
#ifdef Q_WS_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCDynamicStoreCopySpecific.h>
#endif

#include <QtCore>
#include <QItemSelectionModel>


#ifdef Q_OS_WIN32
#include "zlib/zlib.h"
#else
#include <zlib.h>
#endif

#include <QMovie>
#include <QPixmap>

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include "rytablesortfilterproxymodel.h"

extern QString version;

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
    _isUsingCapture(false)

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
    connect(ui->webView->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),SLOT(addJsObject()));
    ui->webView->load(QUrl("http://127.0.0.1:8889/manager/RythemManagerUI/rules.html"));

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
/*
#ifdef Q_WS_MAC
    // TODO: mac下需手动设置代理
    ui->ActionCapture->setEnabled(false);
    ui->ActionCapture->setText(tr("SetupProxyManually"));
    ui->ActionCapture->setToolTip(tr("non-windows OS need to set proxy to:127.0.0.1:8889 manually"));
#endif
*/

}

MainWindow::~MainWindow()
{
    RyProxyServer* server = RyProxyServer::instance();
    server->close();
    delete _jsBridge;
    delete ui;
}


void MainWindow::checkNewVersion(){

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
    QTextCodec* oldCodec = QTextCodec::codecForCStrings();
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(encoding));
    ui->responseTextEdit->setPlainText(QString((
                                              data->responseHeaderRawData()
                                            +"\r\n\r\n"
                                            + decrypedData
                                           ).data())
                                       );
    QTextCodec::setCodecForCStrings(oldCodec);



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
#ifdef Q_WS_MAC
    /*
    CFDictionaryRef dict = SCDynamicStoreCopyProxies(NULL);
    if(!dict){
        qDebug()<<"no proxy";
    }
    */


    QNetworkConfigurationManager mgr;
    QList<QNetworkConfiguration> activeConfigs = mgr.allConfigurations(QNetworkConfiguration::Active);
    QList<QString> activeNames;
    foreach(QNetworkConfiguration cf,activeConfigs){
        //qDebug()<<"new work activated:"<<cf.type()<<cf.state()<<cf.name()<<cf.bearerTypeName()<<cf.identifier();

        activeNames.append(cf.name());
    }
    ///Library/Preferences/SystemConfiguration
    QSettings::setPath(QSettings::NativeFormat,QSettings::SystemScope,"/Library/Preferences/SystemConfiguration/");
    QSettings plist("/Library/Preferences/SystemConfiguration/preferences.plist",QSettings::NativeFormat);

    QMap<QString,QVariant> services = plist.value("NetworkServices").toMap();
    QMap<QString,QVariant>::Iterator i;
    QString theServiceKey;
    QMap<QString,QVariant> theService;
    QMap<QString,QVariant> interface;

    for(i = services.begin();i!=services.end();i++){
        //qDebug()<<i.key();
        theService = i.value().toMap();
        theServiceKey = i.key();
        interface = theService["Interface"].toMap();
        //qDebug()<<"interface"<<interface;
        if(activeNames.contains(interface.value("DeviceName").toString())){
            qDebug()<<"got it.."<<theService["Proxies"].toMap().value("HTTPEnable");
            break;
        }
    }
    if(i!=services.end()){
        QMap<QString,QVariant> proxies = theService["Proxies"].toMap();
        //qDebug()<<"proxies="<<proxies;
        //qDebug()<<proxies.value("HTTPEnable");
        proxies["HTTPEnable"]=0;
        theService["Proxies"] = proxies;
        services[theServiceKey]=theService;
        //qDebug()<<theService;
    }
    //qDebug()<<services[theServiceKey];
    plist.setValue("NetworkServices",services);
    if(plist.isWritable()){
        qDebug()<<"writable";
        plist.sync();//doesn't work.. plist readonly
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

        /*
        // hard code just for some crash issue
        proxySetting.setValue("ProxyEnable",1);
        proxySetting.setValue("ProxyServer","proxy.tencent.com:8080");
        //if( previousProxyInfo.isUsingPac != "0"){
            proxySetting.setValue("AutoConfigURL","http://txp-01.tencent.com/lvsproxy.pac");
        //}
        */
    }else{
        ui->ActionCapture->setText(tr("stop capture"));
        _previousProxyInfo.isUsingPac = proxySetting.value("AutoConfigURL","0").toString();
        _previousProxyInfo.enable = proxySetting.value("ProxyEnable").toInt();
        _previousProxyInfo.proxyString =proxySetting.value("ProxyServer").toString();
        //qDebug()<<previousProxyInfo.proxyString;
        ///qDebug()<<previousProxyInfo.isUsingPac;
        //http=127.0.0.1:8081;https=127.0.0.1:8081;ftp=127.0.0.1:8081


        //initPac(previousProxyInfo.isUsingPac,previousProxyInfo.enable?previousProxyInfo.proxyString:"");
        QString proxyServer="127.0.0.1:8889";
        /*
        if(_previousProxyInfo.enable){
            if(_previousProxyInfo.proxyString.indexOf(";")!=-1){
                proxyServer = QString("http=")+proxyServer;
                QStringList proxies = _previousProxyInfo.proxyString.split(";");
                for(int i=0;i<proxies.length();++i){
                    QStringList tmp = proxies[i].split("=");
                    if(tmp.at(0).toLower()=="http"){
                        proxies[i] = proxyServer;
                        break;
                    }
                }
                proxyServer = proxies.join(";");
            }else{
                // since https has been supported remove code b
                //proxyServer = QString("http=%1;ftp=%2;https=%2").arg(proxyServer).arg(_previousProxyInfo.proxyString);
            }
        }else{
            proxyServer = "http="+proxyServer;
        }
        */
        //qDebug()<<proxyServer<<previousProxyInfo.isUsingPac;
        proxySetting.remove("AutoConfigURL");
        //proxySetting.setValue("AutoConfigURL",_previousProxyInfo.isUsingPac);
        proxySetting.setValue("ProxyEnable",QVariant(1));
        proxySetting.setValue("ProxyServer",proxyServer);
    }
    proxySetting.sync();
	::InternetSetOption(0,39, INT_PTR(0),INT_PTR(0));
    ::InternetSetOption(0, 37,INT_PTR(0), INT_PTR(0));
#endif
    _isUsingCapture = !_isUsingCapture;
    if(_isUsingCapture){
        ui->ActionCapture->setText(tr("stop capture"));
    }else{
        ui->ActionCapture->setText(tr("start capture"));
    }
}

void MainWindow::toggleCapture(){
    toggleProxy();
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

