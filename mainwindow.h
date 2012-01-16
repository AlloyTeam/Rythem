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
class QItemSelectionModel;


class MainWindow : public QMainWindow
{
    Q_OBJECT


private slots:
        void toggleCapture();
        void showSettingsDialog();
public:
    explicit MainWindow(QWidget *parent=0);
    ~MainWindow();
public slots:
        void onNewPipe(ConnectionData_ptr);
        void onPipeUpdate(ConnectionData_ptr);

        void toggleProxy();

        void onSelectionChange(QModelIndex,QModelIndex);



public:
	typedef struct __proxyInfo{
		int enable;
		QString proxyString;
		QString pacUrl;
		QString isUsingPac;
	}ProxyInfo;
	QiddlerPipeTableModel pipeTableModel;

private:
    Ui::MainWindow *ui;
    void createMenus();
    QiProxyServer *server;
	QVector<QiPipe*> *pipes;

    bool isUsingCapture;
    ProxyInfo previousProxyInfo;
    QMenu *fileMenu;
    QMenu *toolMenu;
    QAction *captureAct;
    QItemSelectionModel *itemSelectModel;
protected:
    void closeEvent(QCloseEvent *event);

    QSettings proxySetting;
};

#endif // MAINWINDOW_H
