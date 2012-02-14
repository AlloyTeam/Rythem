#include "qirulesettingsdialog.h"
#include "ui_rule_config.h"

QiRuleSettingsDialog::QiRuleSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QiRuleConfigDialog()){
    ui->setupUi(this);
}
QiRuleSettingsDialog::~QiRuleSettingsDialog(){
    delete ui;
}

int QiRuleSettingsDialog::exec(){

    return QDialog::exec();
}
