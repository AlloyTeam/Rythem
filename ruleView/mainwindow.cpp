#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ruletreemodel.h"
#include "ruletreedelegate.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    model = new RuleTreeModel();
    delegate = new RuleTreeDelegate();
    ui->ruleTreeView->setModel(model);
    ui->ruleTreeView->setItemDelegate(delegate);

}

MainWindow::~MainWindow()
{
    delete ui;
}
