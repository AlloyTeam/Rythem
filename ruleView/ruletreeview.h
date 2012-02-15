#ifndef RULETREEVIEW_H
#define RULETREEVIEW_H

#include <QTreeView>

class RuleTreeView : public QTreeView
{
        Q_OBJECT
    public:
        explicit RuleTreeView(QWidget *parent = 0);
        QAbstractItemDelegate *	itemDelegate(const QModelIndex & index ) const;
        QAbstractItemDelegate *	itemDelegateForColumn( int column ) const;
        QAbstractItemDelegate *	itemDelegateForRow( int row ) const;
    signals:
        
    public slots:
        
};

#endif // RULETREEVIEW_H
