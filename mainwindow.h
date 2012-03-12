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

#include <QScriptEngine>

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
        void doAction(int action,const QString msg){
            qDebug()<<"doAction "<<QString::number(action)<< msg;
            RyRuleManager *manager = RyRuleManager::instance();
            switch(action){
            case 0://add group
                manager->addGroupToLocalProject(msg);//暂时只允许添加到本地project
                break;
            case 1://add rule
            case 2://update group
            case 3://update rule
            case 4://delete group
            case 5://delete rule
                break;
            }
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
            QString s = manager->toJson();
            s.remove('\r');
            s.remove('\n');
            s.remove('\t');
            s.replace("\\","\\\\");
			return s;
        }
        bool updateConfigs(QString json){
            RyRuleManager *manager = RyRuleManager::instance();
			qDebug() << "[JSBridge] update" << json;
			qDebug() << "-------------------------";
            //manager->setLocalConfigContent(json, true);
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
        void onWaterfallActionTriggered();


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
