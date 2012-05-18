#include "rywebview.h"
#include "rymimedata.h"
#include <QtEvents>
#include <QWebFrame>

RyWebView::RyWebView(QWidget *parent) :
    QWebView(parent){
    setAcceptDrops(true);
}
void RyWebView::dragEnterEvent(QDragEnterEvent *event){
    const RyMimeData* mime = dynamic_cast<const RyMimeData*>(event->mimeData());
    if(mime){
        event->accept();
    }else{
        event->ignore();
    }
}

void RyWebView::dragMoveEvent(QDragMoveEvent *event){
    event->accept();
}

void RyWebView::dropEvent(QDropEvent *event){
    const RyMimeData* mime = dynamic_cast<const RyMimeData*>(event->mimeData());
    if (mime) {
        RyPipeData_ptr d = mime->pipeData();
        QString url = d->fullUrl;
        url.replace('\'',"\\'");
        QPoint p = event->pos();
        page()->mainFrame()->evaluateJavaScript(
                    QString("callbackFromApp('newRule',{pos:{x:%1,y:%2},url:'%3'})")
                    .arg(p.x())
                    .arg(p.y())
                    .arg(url)
        );
        event->accept();
    }
}
