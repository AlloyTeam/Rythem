#include <QWebFrame>
#include "waterfallwindow.h"
#include "ui_waterfallwindow.h"

WaterfallWindow::WaterfallWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WaterfallWindow)
{
    ui->setupUi(this);
    connect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(onWebContentLoaded()));
}

WaterfallWindow::~WaterfallWindow()
{
    delete ui;
    qDebug() << "[WaterfallWindow] i am dead";
}

void WaterfallWindow::setPipeData(QList<RyPipeData_ptr> list){
    //save the list first
    pipes = list;
    if(loaded){
        //constructor the js object
        qSort(pipes);
        QStringList results;
        QListIterator<RyPipeData_ptr> it(list);
        while(it.hasNext()){
            RyPipeData_ptr p = it.next();
            QString result;
            QTextStream stream(&result);
            stream << "{id:" << p->id
                   << ", socketID:"   << p->socketConnectionId
                   << ", requestHeader:\""      << QString(p->requestHeaderRawData()).replace("\r\n", "\\r\\n") << "\""
                   << ", responseHeader:\""     << QString(p->responseHeaderRawData()).replace("\r\n", "\\r\\n") << "\""
                   << ", startTime:"            << p->performances.requestBegin
                   << ", responseStartTime:"    << p->performances.responseBegin
                   << ", responseFinishTime:"   << p->performances.responseDone
                   << "}";
            results << result;
        }

        //invoke js update method
        QWebFrame *frame = ui->webView->page()->mainFrame();
        QString args = "[" + results.join(",") + "]";
        qDebug() << args;
        frame->evaluateJavaScript("window.updateAllConnections(" + args + ")");
    }
}

void WaterfallWindow::onWebContentLoaded(){
    loaded = true;
    if(pipes.length()){
        setPipeData(pipes);
    }
}

void WaterfallWindow::closeEvent(QCloseEvent *event){
    event->accept();
    deleteLater();
}
