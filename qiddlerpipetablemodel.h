#ifndef QIDDLERPIPETABLEMODEL_H
#define QIDDLERPIPETABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QMap>
#include <QSharedPointer>
#include "pipedata.h"
class QiddlerPipeTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit QiddlerPipeTableModel(QObject *parent = 0);
    QMap<int,Pipedata_const_ptr > pipesMap;
    QVector<Pipedata_const_ptr > pipesVector;

    int rowCount( const QModelIndex & parent ) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void addItem(Pipedata_const_ptr p);
    void removeItems();
    void removeAllItem();
signals:

public slots:

private:
    int pipeNumber;
};

#endif // QIDDLERPIPETABLEMODEL_H
