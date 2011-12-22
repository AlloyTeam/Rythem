#ifndef QIDDLERPIPETABLEMODEL_H
#define QIDDLERPIPETABLEMODEL_H

#include <QAbstractTableModel>

class QiddlerPipeTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit QiddlerPipeTableModel(QObject *parent = 0);


int rowCount( const QModelIndex & parent ) const;
int columnCount(const QModelIndex &parent) const;
QVariant data(const QModelIndex &index, int role) const;
signals:

public slots:

};

#endif // QIDDLERPIPETABLEMODEL_H
