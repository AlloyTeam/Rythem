#include <QCloseEvent>
#include "waterfallwindow.h"

WaterfallWindow::WaterfallWindow(QWidget *parent) :
    QMainWindow(parent)
{
    this->setWindowTitle("Network Request Waterfall Diagram");
    this->resize(800, 480);
}

WaterfallWindow::~WaterfallWindow(){
    qDebug() << "[WaterfallWindow] i am dead";
}

void WaterfallWindow::setPipeData(QList<RyPipeData_ptr> list){
    qDebug() << "hey you selected" << list.length() << "pipes";
}

void WaterfallWindow::closeEvent(QCloseEvent *e){
    e->accept();
    deleteLater();
}
