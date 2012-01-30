#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTcpServer>
#include "qiconnectiondata.h"
#include "qiproxyserver.h"
#include <QSettings>
#include <QVariant>
#include <QUrl>


#include <QNetworkConfigurationManager>
#include <QMAp>
#include <QList>

#ifdef Q_WS_WIN32
#include "wininet.h"
#include "qiwinhttp.h"
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



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    pipes(new QVector<QiPipe*>),
    pipeTableModel(new QiddlerPipeTableModel()),
    isUsingCapture(false)

#ifdef Q_OS_WIN
    ,proxySetting("\\HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\",QSettings::NativeFormat)
#endif
{
    ui->setupUi(this);
    ui->tableView->setModel(&pipeTableModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setColumnWidth(0,30);

    jsBridge = new QiJsBridge();

    addJsObject();
    connect(ui->webView->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),SLOT(addJsObject()));
    ui->webView->load(QUrl("http://127.0.0.1:8889/test/"));


    itemSelectModel = ui->tableView->selectionModel();

    connect(itemSelectModel,SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),SLOT(onSelectionChange(QModelIndex,QModelIndex)));

    //ui->tableView->setItemDelegate();
    createMenus();
    //toggleCapture();

    connect(ui->ActionCapture,SIGNAL(triggered()),SLOT(toggleCapture()));
    connect(ui->actionRemoveAll,SIGNAL(triggered()),&pipeTableModel,SLOT(removeAllItem()));
}

MainWindow::~MainWindow()
{
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

void MainWindow::onPipeUpdate(ConnectionData_ptr pipeData){
    //qDebug()<<"connected";
    pipeTableModel.updateItem(pipeData);
}

void MainWindow::onNewPipe(ConnectionData_ptr pipeData){
    //pipeTableModel
    pipeTableModel.addItem(pipeData);
}

void MainWindow::onSelectionChange(QModelIndex topLeft, QModelIndex bottomRight){
    //qDebug()<<"onSelectionChange";
    int row = topLeft.row();
    ConnectionData_ptr data = pipeTableModel.getItem(row);
    ui->requestTextEdit->setPlainText(data->requestHeaderRawData +"\r\n\r\n"+data->requestBody );
    ui->responseTextEdit->setPlainText(QString::fromUtf8(
                                           (data->responseHeaderRawData+"\r\n\r\n"
                                            + (data->unChunkResponse.isEmpty()?data->responseBody:data->unChunkResponse)).data())
                                       );
    data.clear();
}

void MainWindow::toggleProxy(){
    if(isUsingCapture){
        isUsingCapture = false;
        /*
        proxySetting.setValue("ProxyEnable",previousProxyInfo.enable);
        proxySetting.setValue("ProxyServer",previousProxyInfo.proxyString);
        if( previousProxyInfo.isUsingPac != "0"){
            proxySetting.setValue("AutoConfigURL",previousProxyInfo.isUsingPac);
        }
        */
        ///*
        // hard code just for some crash issue
        proxySetting.setValue("ProxyEnable",1);
        proxySetting.setValue("ProxyServer","proxy.tencent.com:8080");
        //if( previousProxyInfo.isUsingPac != "0"){
            proxySetting.setValue("AutoConfigURL","http://txp-01.tencent.com/lvsproxy.pac");
        //}
        //*/
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
    QiWinHttp::init();
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

