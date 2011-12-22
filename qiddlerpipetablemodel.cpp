#include "qiddlerpipetablemodel.h"

QiddlerPipeTableModel::QiddlerPipeTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}
int QiddlerPipeTableModel::rowCount( const QModelIndex & parent ) const{
    return 10;
}
int QiddlerPipeTableModel::columnCount(const QModelIndex &parent) const{
    return 3;
}
QVariant QiddlerPipeTableModel::data(const QModelIndex &index, int role) const{
    return tr("Hello world");
}
