#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "waterfallwindow.h"

#include <QTcpServer>
#include "rypipedata.h"
#include "ryproxyserver.h"
#include <QSettings>
#include <QVariant>
#include <QUrl>
#include <QNetworkConfigurationManager>
#include <QMAp>
#include <QList>
#include <QMessageBox>

#ifdef Q_WS_WIN32
#include "wininet.h"
#include "rywinhttp.h"
#include "winnetwk.h"
#endif
#ifdef Q_WS_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCDynamicStoreCopySpecific.h>
#endif

#include <QtCore>
#include <QItemSelectionModel>

#include "qirulesettingsdialog.h"

#ifdef Q_OS_WIN32
#include "zlib/zlib.h"
#else
#include <zlib.h>
#endif

#include <QMovie>
#include <QPixmap>

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    pipeTableModel(new QiddlerPipeTableModel()),
    ui(new Ui::MainWindow),
    isUsingCapture(false)

#ifdef Q_OS_WIN
    ,proxySetting("\\HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\",QSettings::NativeFormat)
#endif
{
    ui->setupUi(this);
    ui->tableView->setModel(&pipeTableModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setColumnWidth(0,30);
    ui->tableView->setColumnWidth(1,30);
    ui->tableView->setSortingEnabled(true);

    jsBridge = new RyJsBridge();

    addJsObject();
    connect(ui->webView->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),SLOT(addJsObject()));
    ui->webView->load(QUrl("http://127.0.0.1:8889/manager/RythemManagerUI/rules.html"));

    // should use slot to do this
    //ui->composer->setupProxy(RyProxyServer::instance()->serverAddress().toString(),
    //                         RyProxyServer::instance()->serverPort());
    ui->composer->setupProxy("127.0.0.1",
                             8889);


    itemSelectModel = ui->tableView->selectionModel();

    connect(itemSelectModel,SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),SLOT(onSelectionChange(QModelIndex,QModelIndex)));

    //ui->tableView->setItemDelegate();
    createMenus();
    //toggleCapture();

    connect(ui->ActionCapture,SIGNAL(triggered()),SLOT(toggleCapture()));
    connect(ui->actionRemoveAll,SIGNAL(triggered()),&pipeTableModel,SLOT(removeAllItem()));
    connect(ui->actionWaterfall, SIGNAL(triggered()), this, SLOT(onWaterfallActionTriggered()));
}

MainWindow::~MainWindow()
{
    //RyProxyServer* server = RyProxyServer::instance();
    //server->close();
    delete jsBridge;
    delete ui;
}
void MainWindow::createMenus(){
    fileMenu = menuBar()->addMenu(tr("&File"));
    captureAct = new QAction(tr("&Capture"),this);
    captureAct->setCheckable(true);
    fileMenu->addAction(captureAct);
    QAction *a = fileMenu->addAction(tr("&rules"));
    connect(a,SIGNAL(triggered()),SLOT(showSettingsDialog()));
    connect(captureAct,SIGNAL(triggered()),SLOT(toggleCapture()));
}

void MainWindow::showSettingsDialog(){
    QiRuleSettingsDialog dialog(this);
    dialog.exec();

}

void MainWindow::onPipeUpdate(RyPipeData_ptr pipeData){
    //qDebug()<<"connected";
    pipeTableModel.updateItem(pipeData);
}

void MainWindow::onNewPipe(RyPipeData_ptr pipeData){
    //pipeTableModel
    pipeTableModel.addItem(pipeData);
}

void MainWindow::onSelectionChange(QModelIndex topLeft, QModelIndex){
    //qDebug()<<"onSelectionChange";
    int row = topLeft.row();
    RyPipeData_ptr data = pipeTableModel.getItem(row);

    ui->tollTabs->setCurrentWidget(ui->inspectorTab);
    ui->requestInspectorTabs->setCurrentWidget(ui->requestInspectorTextview);

    //TODO
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
              :data->responseBodyRawData());;
    if(isEncrypted){
        decrypedData = gzipDecompress(decrypedData);
    }

    //TODO..
    QMovie *movie = ui->label->movie();
    if(movie){
        delete movie;
    }
    if(data->getResponseHeader("Content-Type").toLower().indexOf("image")!=-1){
        ui->responseInspectorTabs->setCurrentWidget(ui->responseInspectorImageView);
        if(!decrypedData.isEmpty()){
            if(data->getResponseHeader("Content-Type").toLower().indexOf("gif")!=-1){
                QBuffer *data = new QBuffer(&decrypedData);
                QMovie *movie = new QMovie(data);
                ui->label->setMovie(movie);
                //TODO start and delete movie..
            }else{
                QPixmap pixmap;
                pixmap.loadFromData(decrypedData);
                ui->label->setPixmap(pixmap);
            }
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


void MainWindow::onWaterfallActionTriggered(){
    QModelIndexList list = ui->tableView->selectionModel()->selectedRows();
    QList<RyPipeData_ptr> pipes;
    QListIterator<QModelIndex> it(list);
    while(it.hasNext()){
        QModelIndex i = it.next();
        RyPipeData_ptr data = pipeTableModel.getItem(i.row());
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


void MainWindow::mousePressEvent(QMouseEvent *event){
    qDebug()<<"mouseenter";
    QTableView *table = static_cast<QTableView*>(childAt(event->pos()));
    if (!table || table!=ui->tableView){
        return;
    }
    qDebug()<<"is table";
    QPoint hotSpot = event->pos() - table->pos();
    QMimeData *mimeData = new QMimeData;
    mimeData->setText("child->text()");
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    //drag->setPixmap(pixmap);
    drag->setHotSpot(hotSpot);
    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *){

}

void MainWindow::toggleProxy(){
    if(isUsingCapture){
        isUsingCapture = false;

        proxySetting.setValue("ProxyEnable",previousProxyInfo.enable);
        if(previousProxyInfo.enable != 0){
            proxySetting.setValue("ProxyServer",previousProxyInfo.proxyString);
        }
        if( previousProxyInfo.isUsingPac != "0"){
            proxySetting.setValue("AutoConfigURL",previousProxyInfo.isUsingPac);
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
        isUsingCapture = true;
        previousProxyInfo.isUsingPac = proxySetting.value("AutoConfigURL","0").toString();
        previousProxyInfo.enable = proxySetting.value("ProxyEnable").toInt();
        previousProxyInfo.proxyString =proxySetting.value("ProxyServer").toString();
        //qDebug()<<previousProxyInfo.proxyString;
        ///qDebug()<<previousProxyInfo.isUsingPac;
        //http=127.0.0.1:8081;https=127.0.0.1:8081;ftp=127.0.0.1:8081

        QString proxyServer="127.0.0.1:8889";
        if(previousProxyInfo.enable){
            if(previousProxyInfo.proxyString.indexOf(";")!=-1){
                proxyServer = QString("http=")+proxyServer;
                QStringList proxies = previousProxyInfo.proxyString.split(";");
                for(int i=0;i<proxies.length();++i){
                    QStringList tmp = proxies[i].split("=");
                    if(tmp.at(0).toLower()=="http"){
                        proxies[i] = proxyServer;
                        break;
                    }
                }
                proxyServer = proxies.join(";");
            }else{
                proxyServer = QString("http=%1;ftp=%2;https=%2").arg(proxyServer).arg(previousProxyInfo.proxyString);
            }
        }else{
            proxyServer = "http="+proxyServer;
        }
        //qDebug()<<proxyServer<<previousProxyInfo.isUsingPac;
        proxySetting.remove("AutoConfigURL");
        proxySetting.setValue("ProxyEnable",QVariant(1));
        proxySetting.setValue("ProxyServer",proxyServer);
    }
    proxySetting.sync();
#ifdef Q_WS_WIN32
	::InternetSetOption(0,39, INT_PTR(0),INT_PTR(0));
    ::InternetSetOption(0, 37,INT_PTR(0), INT_PTR(0));
#endif
}

void MainWindow::toggleCapture(){
    captureAct->setChecked(!isUsingCapture);
#ifdef Q_WS_WIN32
    RyWinHttp::init();
    QtConcurrent::run(this,&MainWindow::toggleProxy);
    //qDebug()<<previousProxyInfo.isUsingPac;
#endif
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
        qDebug()<<"proxies="<<proxies;
        qDebug()<<proxies.value("HTTPEnable");
        proxies["HTTPEnable"]=0;
        theService["Proxies"] = proxies;
        services[theServiceKey]=theService;
        qDebug()<<theService;
    }
    qDebug()<<services[theServiceKey];
    plist.setValue("NetworkServices",services);
    plist.sync();//doesn't work.. plist readonly
#endif
}


void MainWindow::closeEvent(QCloseEvent *event){
    if(isUsingCapture){
        toggleCapture();
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::addJsObject(){
    ui->webView->page()->mainFrame()->addToJavaScriptWindowObject(QString("App"),jsBridge);
}

