#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QDebug>
#include "pipedata.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<Pipedata_const_ptr>("Pipedata_const_ptr");
    qRegisterMetaType<PipeData_ptr>("PipeData_ptr");
    MainWindow w;
    w.show();
    return a.exec();
}
