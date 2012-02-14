#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ruletreemodel.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    model = new RuleTreeModel();
    ui->ruleTreeView->setModel(model);

}

MainWindow::~MainWindow()
{
    delete ui;
}
