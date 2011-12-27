#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTcpServer>
#include "pipedata.h"
#include "qproxyserver.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    pipes(new QVector<QPipe*>),
    pipeTableModel(new QiddlerPipeTableModel()),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableView->setModel(&pipeTableModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setColumnWidth(0,30);
    //ui->tableView->setItemDelegate();
    createMenus();

    server  = new QProxyServer();
    server->listen(QHostAddress("127.0.0.1"),8080);
    connect(server,SIGNAL(newPipe(int)),SLOT(onNewPipe(int)));
    connect(server,SIGNAL(pipeUpdate(int,PipeData)),SLOT(onPipeUpdate(int,PipeData)));
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



void MainWindow::onPipeUpdate(int socketId,const PipeData pipeData){
    //qDebug()<<"connected";
}

void MainWindow::onNewPipe(int socketId){
    //pipeTableModel
}
