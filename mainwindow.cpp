#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTcpServer>
#include "pipedata.h"
#include "qiproxyserver.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    pipes(new QVector<QiPipe*>),
    pipeTableModel(new QiddlerPipeTableModel()),
    ui(new Ui::MainWindow)
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
