#include "qiddlerpipetablemodel.h"
#include "pipedata.h"
#include <QVector>
#include <QStringList>
#include <QDebug>

QiddlerPipeTableModel::QiddlerPipeTableModel(QObject *parent) :
    QAbstractTableModel(parent){
}
int QiddlerPipeTableModel::rowCount( const QModelIndex & parent ) const{
    return 3;//pipesVector.count();
}
int QiddlerPipeTableModel::columnCount(const QModelIndex &parent) const{
    return 8;
}
QVariant QiddlerPipeTableModel::data(const QModelIndex &index, int role) const{
    if(role == Qt::DisplayRole || role == Qt::ToolTipRole){
        int column = index.column();
        if(pipesVector.count()>column){
            PipeData *p = pipesVector[column];
        }
        return tr("AAAAAAAA");
        /*
        switch(index.row()){
            case 0:
                return QString("%1").arg(p->number);
            case 1:
                return QString("%1").arg(p->returnCode);
            case 2:
                return p->protocol;
            case 3:
                return p->host;
            case 4:
                return p->serverIP;
            case 5:
                return p->URL;
            default:
                return "Unknow";
        }
        */
    }else{
        return QVariant();
    }
}
QVariant QiddlerPipeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    //TODO
    QStringList headers;
    headers<<"#"<<"Result"<<"Protocol"<<"Host"<<"ServerIP"<<"URL"<<"Body"<<"Caching"<<"Content-Type";

    if (orientation == Qt::Horizontal) {
        if(section< headers.size() ){
            return headers[section];
        }
    }
    return QVariant();
}
Qt::ItemFlags QiddlerPipeTableModel::flags(const QModelIndex &index) const{
    if(!index.isValid()){
        return Qt::ItemIsEnabled;
    }
    return QAbstractTableModel::flags(index);
}
