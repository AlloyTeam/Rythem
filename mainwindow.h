#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "rytablemodel.h"
#include "proxy/rypipedata.h"
#include <QSettings>
#include <QWebFrame>
#include <QFileDialog>
#include <QDebug>

#include <QScriptEngine>
#include <QScriptValue>

#include "rule/ryrulemanager.h"
#include "proxy/ryproxyserver.h"


#include <QScriptEngine>

using namespace rule;

namespace Ui {
    class MainWindow;
}
class RyPipeData;
class RyTableSortFilterProxyModel;
class QItemSelectionModel;
class RyJsBridge:public QObject{
        Q_OBJECT
    public:
        RyJsBridge();

    public slots:
        QString doAction(int action,const QString msg,quint64 groupId=0);
        QString getFile();
        QString getDir();
        QString getConfigs();

   signals:
        void ruleChanged(int action,QString json);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent=0);
    ~MainWindow();
public slots:
        void onNewPipe(RyPipeData_ptr);
        void onPipeUpdate(RyPipeData_ptr);

        void toggleProxy();

        void onItemDoubleClicked(QModelIndex);
        void onSelectionChange(QModelIndex);
        void onWaterfallActionTriggered();


public:
	typedef struct __proxyInfo{
		int enable;
		QString proxyString;
		QString pacUrl;
		QString isUsingPac;
	}ProxyInfo;
    RyTableModel *pipeTableModel;
    RyTableSortFilterProxyModel *sortFilterProxyModel;


private:
    Ui::MainWindow *ui;
    RyJsBridge *_jsBridge;
    void createMenus();
    RyProxyServer *_server;

    bool _isUsingCapture;
    ProxyInfo _previousProxyInfo;
    QMenu *_fileMenu;
    QMenu *_filterMenu;
    QAction *_importSessions;
    QAction *_filterNoImagesAct;
    QAction *_filterNo304sAct;
    QAction *_filterShowMatchOnly;
    QAction *_captureAct;
    QItemSelectionModel *_itemSelectModel;
protected:
    void closeEvent(QCloseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    QSettings proxySetting;

private slots:
    void addJsObject();
    void onAction(QAction*);
    void on_actionLongCache_triggered();
    void toggleCapture();
    void importSessions();
    void onActionRemoveAll();
};

#endif // MAINWINDOW_H
