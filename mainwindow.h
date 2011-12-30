#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qiddlerpipetablemodel.h"
#include "pipedata.h"
#include <QSettings>

namespace Ui {
    class MainWindow;
}
class QiProxyServer;
class QiPipe;
class PipeData;



class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
        QiProxyServer *server;
        QVector<QiPipe*> *pipes;
        QiddlerPipeTableModel pipeTableModel;
private slots:
        void toggleCapture();
        void doSomeBug();
public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();
public slots:
        void onNewPipe(Pipedata_const_ptr);
        void onPipeUpdate(Pipedata_const_ptr);



public:
        typedef struct __proxyInfo{
            int enable;
            QString proxyString;
            QString pacUrl;
            QString isUsingPac;
        }ProxyInfo;
protected:
        void closeEvent(QCloseEvent *event);

        QSettings proxySetting;
private:
    Ui::MainWindow *ui;
    void createMenus();

    bool isUsingCapture;
    ProxyInfo previousProxyInfo;
    QMenu *fileMenu;
    QMenu *toolMenu;
    QAction *captureAct;
};

#endif // MAINWINDOW_H
