#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTcpServer>
#include "pipedata.h"
#include "qiproxyserver.h"
#include <QSettings>
#include <QVariant>

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    pipes(new QVector<QiPipe*>),
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
    //ui->tableView->setItemDelegate();
    createMenus();

    server  = new QiProxyServer();
    server->listen(QHostAddress("127.0.0.1"),8889);
    connect(server,SIGNAL(newPipe(PipeData_ptr)),SLOT(onNewPipe(PipeData_ptr)));
    connect(server,SIGNAL(pipeUpdate(PipeData_ptr)),SLOT(onPipeUpdate(PipeData_ptr)));

    //toggleCapture();
}

MainWindow::~MainWindow()
{
    delete server;
    delete ui;
}
void MainWindow::createMenus(){
    fileMenu = menuBar()->addMenu(tr("&File"));
    captureAct = new QAction(tr("&Capture"),this);
    captureAct->setCheckable(true);
    fileMenu->addAction(captureAct);
    QAction *a = fileMenu->addAction(tr("&get error"));
    connect(a,SIGNAL(triggered()),SLOT(doSomeBug()));
    connect(captureAct,SIGNAL(triggered()),SLOT(toggleCapture()));
}

void MainWindow::doSomeBug(){
    QStringList s;
    s.at(10);
}

void MainWindow::onPipeUpdate(PipeData_ptr pipeData){
    //qDebug()<<"connected";
}

void MainWindow::onNewPipe(PipeData_ptr p){
    //pipeTableModel
    pipeTableModel.addItem(p);
}
void MainWindow::toggleCapture(){
#ifdef Q_WS_WIN32
    QiWinHttp::init(previousProxyInfo.isUsingPac);
    if(isUsingCapture){
        isUsingCapture = false;
        /*
        proxySetting.setValue("ProxyEnable",previousProxyInfo.enable);
        proxySetting.setValue("ProxyServer",previousProxyInfo.proxyString);
        if( previousProxyInfo.isUsingPac != "0"){
            proxySetting.setValue("AutoConfigURL",previousProxyInfo.isUsingPac);
        }
        */
        // hard code just for some crash issue
        proxySetting.setValue("ProxyEnable",1);
        proxySetting.setValue("ProxyServer","proxy.tencent.com:8080");
        if( previousProxyInfo.isUsingPac != "0"){
            proxySetting.setValue("AutoConfigURL","http://txp-01.tencent.com/lvsproxy.pac");
        }
    }else{
        isUsingCapture = true;
        previousProxyInfo.isUsingPac = proxySetting.value("AutoConfigURL","0").toString();
        previousProxyInfo.enable = proxySetting.value("ProxyEnable").toInt();
        previousProxyInfo.proxyString =proxySetting.value("ProxyServer").toString();
        qDebug()<<previousProxyInfo.proxyString;
        //http=127.0.0.1:8081;https=127.0.0.1:8081;ftp=127.0.0.1:8081

        QString proxyServer="127.0.0.1:8889";
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
            qDebug()<<proxyServer<<previousProxyInfo.isUsingPac;
        }
        proxySetting.remove("AutoConfigURL");
        proxySetting.setValue("ProxyEnable",QVariant(1));
        proxySetting.setValue("ProxyServer",proxyServer);
    }
    proxySetting.sync();
    ::InternetSetOption(0,39, INT_PTR(0),INT_PTR(0));
    ::InternetSetOption(0, 37,INT_PTR(0), INT_PTR(0));
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
    captureAct->setChecked(isUsingCapture);
    qDebug()<<"toggle capture";
}


void MainWindow::closeEvent(QCloseEvent *event){
    if(isUsingCapture){
        toggleCapture();
    }
    QMainWindow::closeEvent(event);
}
