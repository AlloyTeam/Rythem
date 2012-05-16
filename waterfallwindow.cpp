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

bool compareRequestTime(RyPipeData_ptr p1, RyPipeData_ptr p2){
    return p1->performances.requestBegin < p2->performances.requestBegin;
}

void WaterfallWindow::setPipeData(QList<RyPipeData_ptr> list){
    //save the list first
    pipes = list;
    if(loaded){
        //constructor the js object
        qSort(pipes.begin(), pipes.end(), compareRequestTime);
        QStringList results;
        QListIterator<RyPipeData_ptr> it(list);
        qDebug() << "--------------------------";
        while(it.hasNext()){
            RyPipeData_ptr p = it.next();
            qDebug() << "requestBegin" << p->performances.requestBegin;
            QString result;
            QTextStream stream(&result);
            stream << "{id:" << p->id
                   << ", socketID:"                 << p->socketConnectionId
                   << ", host:\""                   << p->host << "\""
                   << ", url:\""                    << p->fullUrl << "\""
                   << ", method:\""                 << p->method << "\""
                   << ", status:\""                 << p->responseStatus+"\""
                   << ", startTime:"                << p->performances.requestBegin
                   << ", responseStartTime:"        << p->performances.responseBegin
                   << ", responseFinishTime:"       << p->performances.responseDone
                   << ", requestContentLength:"     << p->requestBodyRawData().length()
                   << ", responseContentLength:"    << p->responseBodyRawData().length()
                   << "}";
            results << result;
        }

        //invoke js update method
        QWebFrame *frame = ui->webView->page()->mainFrame();
        QString args = "[" + results.join(",") + "]";
        qDebug()<<args;
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
