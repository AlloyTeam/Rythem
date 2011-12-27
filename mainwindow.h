#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qiddlerpipetablemodel.h"

namespace Ui {
    class MainWindow;
}
class QProxyServer;
class QPipe;
class PipeData;
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
        QProxyServer *server;
        QVector<QPipe*> *pipes;
        QiddlerPipeTableModel pipeTableModel;
private slots:
        void toggleCapture(){}
public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();
public slots:
        void onNewPipe(int socketId);
        void onPipeUpdate(int socketId,const PipeData);

private:
    Ui::MainWindow *ui;
    void createMenus();
    void removePipe(QPipe*);


    QMenu *fileMenu;
    QMenu *toolMenu;
    QAction *captureAct;
};

#endif // MAINWINDOW_H
