#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qiddlerpipetablemodel.h"
#include "rypipedata.h"
#include <QSettings>
#include <QWebFrame>
#include <QFileDialog>
#include <QDebug>

#include <QScriptEngine>
#include <QScriptValue>

#include "ryrulemanager.h"
#include "ryproxyserver.h"

namespace Ui {
    class MainWindow;
}
class RyPipeData;
class QItemSelectionModel;
class RyJsBridge:public QObject{
        Q_OBJECT
    public:
        RyJsBridge():QObject(){


        }

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
        QString getConfigs(){
            RyRuleManager *manager = RyRuleManager::instance();
            QScriptEngine engine;
            QString s = manager->configusToJSON();
            s.prepend("configs =  ");
            s.remove("\r");
            s.remove("\n");
            s.remove("\t");
            s.append(";");
            return s;
        }
   signals:
        void ruleChanged(QString json);
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

protected:
        void mousePressEvent(QMouseEvent *);
        void dragEnterEvent(QDragEnterEvent *);

private:
    Ui::MainWindow *ui;
    RyJsBridge *jsBridge;
    void createMenus();
    RyProxyServer *server;

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
