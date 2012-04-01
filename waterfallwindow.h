#ifndef WATERFALLWINDOW_H
#define WATERFALLWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QCloseEvent>
#include <proxy/rypipedata.h>

namespace Ui {
class WaterfallWindow;
}

class WaterfallWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit WaterfallWindow(QWidget *parent = 0);
    ~WaterfallWindow();
    void setPipeData(QList<RyPipeData_ptr> list);

public slots:
    void onWebContentLoaded();
    
private:
    Ui::WaterfallWindow *ui;
    bool loaded;
    QList<RyPipeData_ptr> pipes;

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // WATERFALLWINDOW_H
