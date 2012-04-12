#ifndef RYCONNECTIONTABLEVIEW_H
#define RYCONNECTIONTABLEVIEW_H

#include <QTableView>
#include <QtCore>
#include <QtGui>

class RyConnectionTableView : public QTableView
{
    Q_OBJECT
public:
    explicit RyConnectionTableView(QWidget *parent = 0);
    
signals:
    
public slots:
    
protected:
    virtual void contextMenuEvent (QContextMenuEvent *event);
private slots:
    void onAction();
private:
    QMenu *_contextMenu;
    QMenu *_saveMenu;
    QMenu *_removeMenu;

    QAction *_saveSelectedSessionAct;
    QAction *_saveAllSessionAct;
    QAction *_saveSessionsRevertAct;
    QAction *_removeSelectedSessionAct;
    QAction *_removeAllSessionAct;
    QAction *_removeSessionsRevertAct;

    QAction *_autoScrollAct;

    void createMenu();
};

#endif // RYCONNECTIONTABLEVIEW_H
