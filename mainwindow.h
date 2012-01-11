#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qiddlerpipetablemodel.h"
#include "qiconnectiondata.h"
#include <QSettings>

namespace Ui {
    class MainWindow;
}
class QiProxyServer;
class QiPipe;
class QiConnectionData;



class MainWindow : public QMainWindow
{
    Q_OBJECT


private slots:
        void toggleCapture();
        void doSomeBug();
public:
    explicit MainWindow(QWidget *parent=0);
    ~MainWindow();
public slots:
        void onNewPipe(ConnectionData_ptr);
        void onPipeUpdate(ConnectionData_ptr);

        void toggleProxy();



public:
        typedef struct __proxyInfo{
            int enable;
            QString proxyString;
            QString pacUrl;
            QString isUsingPac;
        }ProxyInfo;

private:
    Ui::MainWindow *ui;
    void createMenus();
    QiProxyServer *server;
    QVector<QiPipe*> *pipes;
    QiddlerPipeTableModel pipeTableModel;

    bool isUsingCapture;
    ProxyInfo previousProxyInfo;
    QMenu *fileMenu;
    QMenu *toolMenu;
    QAction *captureAct;
protected:
    void closeEvent(QCloseEvent *event);

    QSettings proxySetting;
};

#endif // MAINWINDOW_H
