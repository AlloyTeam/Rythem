#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTcpServer>
#include "qpipe.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    pipes(new QVector<QPipe*>),
    pipeTableModel(new QiddlerPipeTableModel()),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableView->setModel(pipeTableModel);
    server  = new QTcpServer();
    server->listen(QHostAddress("127.0.0.1"),8080);
    connect(server,SIGNAL(newConnection()),SLOT(onConnections()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnections(){
    QTcpSocket* socket = server->nextPendingConnection();
    //qDebug()<<"got new connection \n";


    QPipe *pipe = new QPipe(socket);
    pipes->append(pipe);

    connect(pipe,SIGNAL(connected()),SLOT(onPipeConnected()));
    connect(pipe,SIGNAL(completed()),SLOT(onPipeCompleted()));
    connect(pipe,SIGNAL(error()),SLOT(onPipeError()));
}

void MainWindow::onPipeConnected(){
    //qDebug()<<"connected";
    QPipe* pipe = (QPipe*)sender();
    if(pipe == 0){
        return;
    }
}

void MainWindow::onPipeError(){

}

void MainWindow::onPipeCompleted(){

}
