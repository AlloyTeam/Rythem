#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTcpServer>
#include "pipedata.h"
#include "qiproxyserver.h"
#include <QSettings>
#include <QVariant>

#ifdef Q_WS_WIN32
    #include "wininet.h"
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
    server->listen(QHostAddress("127.0.0.1"),8888);
    connect(server,SIGNAL(newPipe(Pipedata_const_ptr)),SLOT(onNewPipe(Pipedata_const_ptr)));
    connect(server,SIGNAL(pipeUpdate(Pipedata_const_ptr)),SLOT(onPipeUpdate(Pipedata_const_ptr)));
}

MainWindow::~MainWindow()
{
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

void MainWindow::onPipeUpdate(Pipedata_const_ptr pipeData){
    //qDebug()<<"connected";
}

void MainWindow::onNewPipe(Pipedata_const_ptr p){
    //pipeTableModel
    pipeTableModel.addItem(p);
}
void MainWindow::toggleCapture(){
#ifdef Q_WS_WIN32
    if(isUsingCapture){
        isUsingCapture = false;
        proxySetting.setValue("ProxyEnable",previousProxyInfo.enable);
        proxySetting.setValue("ProxyServer",previousProxyInfo.proxyString);
        if( previousProxyInfo.isUsingPac != "0"){
            proxySetting.setValue("AutoConfigURL",previousProxyInfo.isUsingPac);
        }
    }else{
        isUsingCapture = true;
        previousProxyInfo.isUsingPac = proxySetting.value("AutoConfigURL","0").toString();
        previousProxyInfo.enable = proxySetting.value("ProxyEnable").toInt();
        previousProxyInfo.proxyString =proxySetting.value("ProxyServer").toString();
        qDebug()<<previousProxyInfo.proxyString;
        //http=127.0.0.1:8081;https=127.0.0.1:8081;ftp=127.0.0.1:8081

        QString proxyServer="127.0.0.1:8888";
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
    captureAct->setChecked(isUsingCapture);
}


void MainWindow::closeEvent(QCloseEvent *event){
    if(isUsingCapture){
        toggleCapture();
    }
    QMainWindow::closeEvent(event);
}
