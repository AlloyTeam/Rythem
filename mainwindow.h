#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qiddlerpipetablemodel.h"

namespace Ui {
    class MainWindow;
}
class QTcpServer;
class QPipe;
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
        QTcpServer *server;
        QVector<QPipe*> *pipes;
        QiddlerPipeTableModel *pipeTableModel;
public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();
public slots:
        void onConnections();
        void onPipeConnected();
        void onPipeError();
        void onPipeCompleted();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
