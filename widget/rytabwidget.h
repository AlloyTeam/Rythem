#ifndef RYTABWIDGET_H
#define RYTABWIDGET_H

#include <QtEvents>
#include <QtWidgets>

class RyTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit RyTabWidget(QWidget *parent = 0);
    
signals:
    
public slots:
protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
};

#endif // RYTABWIDGET_H
