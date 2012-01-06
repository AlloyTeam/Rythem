#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QDebug>
#include "pipedata.h"
#include "qiproxyserver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // register metatypes
    qRegisterMetaType<ConnectionData_const_ptr>("ConnectionData_const_ptr");
    qRegisterMetaType<ConnectionData_ptr>("ConnectionData_ptr");


    MainWindow w;

    QiProxyServer *server = new QiProxyServer();
    // TODO listne should be a slot
    server->listen(QHostAddress("127.0.0.1"),8889);
    server->connect(server,SIGNAL(newPipe(ConnectionData_ptr)),&w,SLOT(onNewPipe(ConnectionData_ptr)));
    server->connect(server,SIGNAL(pipeUpdate(ConnectionData_ptr)),&w,SLOT(onPipeUpdate(ConnectionData_ptr)));
    w.show();

    return a.exec();
}
