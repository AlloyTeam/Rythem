#include "savesessionsdialog.h"
#include "ui_savesessionsdialog.h"

SaveSessionsDialog::SaveSessionsDialog(int totalFiles,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveSessionsDialog)
{
    ui->setupUi(this);
    _saved = 0;
    _totalFiles = totalFiles;
    ui->progressBar->setMaximum(_totalFiles);
    ui->progressBar->setTextVisible(true);
}

SaveSessionsDialog::~SaveSessionsDialog()
{
    delete ui;
}

void SaveSessionsDialog::updateProgress(const QString &savingFileName){
    ui->progressBar->setValue(_saved);
    ui->savingFileNameLabel->setText(savingFileName);
}
