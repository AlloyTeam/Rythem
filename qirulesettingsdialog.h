#ifndef QIRULESETTINGSDIALOG_H
#define QIRULESETTINGSDIALOG_H

#include <QDialog>

namespace Ui{
    class QiRuleConfigDialog;
}

class QiRuleSettingsDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit QiRuleSettingsDialog(QWidget *parent = 0);
        ~QiRuleSettingsDialog();
    private:
        Ui::QiRuleConfigDialog *ui;

    signals:
        
    public slots:
        int exec();
};

#endif // QIRULESETTINGSDIALOG_H
