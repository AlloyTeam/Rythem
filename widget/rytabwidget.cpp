#include "rytabwidget.h"
#include "rymimedata.h"


RyTabWidget::RyTabWidget(QWidget *parent) :
    QTabWidget(parent){
    setAcceptDrops(true);
}

void RyTabWidget::dragEnterEvent(QDragEnterEvent *event){
    const RyMimeData* mime = dynamic_cast<const RyMimeData*>(event->mimeData());
    if(mime){
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }else{
        event->ignore();
    }
}

void RyTabWidget::dragMoveEvent(QDragMoveEvent *event){
    const RyMimeData* mime = dynamic_cast<const RyMimeData*>(event->mimeData());
    if(!mime){
        return;
    }
    QPoint p = event->pos();

    QPoint p2 = p-tabBar()->pos();
    int index = tabBar()->tabAt(p2);
    if(index != -1){
        event->accept();
        tabBar()->setCurrentIndex(index);
    }else{
        event->ignore();
    }
}


