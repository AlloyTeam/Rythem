#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qiddlerpipetablemodel.h"
#include "rypipedata.h"
#include <QSettings>
#include <QWebFrame>
#include <QFileDialog>
#include <QDebug>

namespace Ui {
    class MainWindow;
}
class QiProxyServer;
class QiPipe;
class QiConnectionData;
class QItemSelectionModel;
class QiJsBridge:public QObject{
        Q_OBJECT
    public slots:
        void doAction(const QString &msgType,const QString msg=""){
            qDebug()<<"doAction "<<msgType<< msg;
        }
        QString getFile(){
            return QFileDialog::getOpenFileName();
        }
        QString getDir(){
            return QFileDialog::getExistingDirectory();
        }
        QString getRules(){
            return "";
        }
   signals:
        void ruleChanged();
};

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
        void onNewPipe(RyPipeData_ptr);
        void onPipeUpdate(RyPipeData_ptr);

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
    QiJsBridge *jsBridge;
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

private slots:
    void addJsObject();
};

#endif // MAINWINDOW_H
