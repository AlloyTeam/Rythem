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
    connect(server,SIGNAL(newPipe(QSharedPointer<PipeData>)),SLOT(onNewPipe(QSharedPointer<PipeData>)));
    connect(server,SIGNAL(pipeUpdate(QSharedPointer<PipeData>)),SLOT(onPipeUpdate(QSharedPointer<PipeData>)));
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



void MainWindow::onPipeUpdate(QSharedPointer<PipeData> pipeData){
    //qDebug()<<"connected";
}

void MainWindow::onNewPipe(QSharedPointer<PipeData> p){
    //pipeTableModel
    pipeTableModel.addItem(p);
}
