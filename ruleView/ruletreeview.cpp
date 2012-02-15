#include "ruletreeview.h"

RuleTreeView::RuleTreeView(QWidget *parent) :
    QTreeView(parent){

    model = new RuleTreeModel();
    delegate = new RuleTreeDelegate();

    setModel(model);
    setItemDelegate(delegate);

}
