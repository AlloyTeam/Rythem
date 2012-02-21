#ifndef WATERFALLWINDOW_H
#define WATERFALLWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include "rypipedata.h"

class WaterfallWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit WaterfallWindow(QWidget *parent = 0);
    ~WaterfallWindow();
    void setPipeData(QList<RyPipeData_ptr> list);
    
signals:
    
public slots:

protected:
    void closeEvent(QCloseEvent *);
    
};

#endif // WATERFALLWINDOW_H
