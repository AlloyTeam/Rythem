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
{
    ui->setupUi(this);
    ui->tableView->setModel(&pipeTableModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setColumnWidth(0,30);
    //ui->tableView->setItemDelegate();
    createMenus();

    server  = new QiProxyServer();
    server->listen(QHostAddress("127.0.0.1"),8080);
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
    connect(captureAct,SIGNAL(triggered()),SLOT(toggleCapture()));
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
    QSettings reg("\\HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\",QSettings::NativeFormat);
    if(isUsingCapture){
        isUsingCapture = false;
        reg.setValue("ProxyEnable",previousProxyInfo.enable);
        reg.setValue("ProxyServer",previousProxyInfo.proxyString);
    }else{
        isUsingCapture = true;
        previousProxyInfo.enable = reg.value("ProxyEnable").toInt();
        previousProxyInfo.proxyString =reg.value("ProxyServer").toString();
                //(reg.value("ProxyServer").type() == QVariant::Invalid)?"":reg.value("ProxyServer").toString();
        //qDebug()<<"ProxyEnable"<<reg.value("ProxyEnable").typeName();
        //qDebug()<<"MimeExclusionListForCache"<<reg.value("MimeExclusionListForCache");

        reg.setValue("ProxyEnable",QVariant(1));
        reg.setValue("ProxyServer","127.0.0.1:8081");
    }
    reg.sync();
    ::InternetSetOption(0,39, INT_PTR(0),INT_PTR(0));
    ::InternetSetOption(0, 37,INT_PTR(0), INT_PTR(0));
#endif
    captureAct->setChecked(isUsingCapture);
}
