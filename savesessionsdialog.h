#ifndef SAVESESSIONSDIALOG_H
#define SAVESESSIONSDIALOG_H

#include <QDialog>

namespace Ui {
class SaveSessionsDialog;
}

class SaveSessionsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SaveSessionsDialog(int total,QWidget *parent = 0);
    ~SaveSessionsDialog();
public slots:
    void updateProgress(const QString& savingFileName);

    
private:
    Ui::SaveSessionsDialog *ui;

    int _totalFiles;
    int _saved;
};

#endif // SAVESESSIONSDIALOG_H
