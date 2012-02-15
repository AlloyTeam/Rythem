#ifndef RULETREEVIEW_H
#define RULETREEVIEW_H

#include <QTreeView>
#include "ruletreemodel.h"
#include "ruletreedelegate.h"

class RuleTreeView : public QTreeView
{
        Q_OBJECT
    public:
        explicit RuleTreeView(QWidget *parent = 0);
    signals:
        
    public slots:
    private:
        RuleTreeModel *model;
        RuleTreeDelegate *delegate;
};

#endif // RULETREEVIEW_H
