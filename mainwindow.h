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
        QString doAction(int action,const QString msg,quint64 groupId=0){
            qDebug()<<"doAction "<<QString::number(action)<< msg<<"groupId"<<QString::number(groupId);
            RyRuleManager *manager = RyRuleManager::instance();
            QSharedPointer<RyRuleProject> pro;
            QSharedPointer<RyRuleGroup> group;
            QSharedPointer<RyRule> rule;
            switch(action){
            case 0://add local group
                group = manager->addGroupToLocalProject(msg);//暂时只允许添加到本地project
                //emit ruleChanged(0,"success");
                if(!group.isNull()){
                    qDebug()<<group->toJSON();
                    return group->toJSON();
                }
                break;
            case 1://add rule to group
                rule = manager->addRuleToGroup(msg,groupId);
                if(!rule.isNull()){
                    return QString::number(rule->ruleId());
                }
                break;
            case 2://add remote project
                pro = manager->addRemoteProject(msg);
                if(!pro.isNull()){
                    emit ruleChanged(2,pro->toJson());
                    return pro->toJson();
                }
                break;
            case 3://update group

                break;
            case 4://update rule
                manager->updateRule(msg,groupId);
                break;
            case 5://remove group
                manager->removeGroup(msg.toULongLong());
                break;
            case 6://remove rule  (6,ruleId,groupId)
                manager->removeRule(msg.toULongLong(),groupId);
                break;
            }
            return "";
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
            return true;
        }

   signals:
        void ruleChanged(int action,QString json);
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
