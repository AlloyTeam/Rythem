#include "qiddlerpipetablemodel.h"
#include "qiconnectiondata.h"
#include <QVector>
#include <QStringList>
#include <QDebug>

QiddlerPipeTableModel::QiddlerPipeTableModel(QObject *parent) :
    QAbstractTableModel(parent),pipeNumber(0){
}
QiddlerPipeTableModel::~QiddlerPipeTableModel(){
    removeAllItem();
    qDebug()<<"~QiddlerPipeTableModel";
}

int QiddlerPipeTableModel::rowCount( const QModelIndex & parent ) const{
    return pipesVector.count();
}
int QiddlerPipeTableModel::columnCount(const QModelIndex &parent) const{
    return 8;
}
QVariant QiddlerPipeTableModel::data(const QModelIndex &index, int role) const{
    if(role == Qt::DisplayRole || role == Qt::ToolTipRole){
        int row = index.row();
        ConnectionData_ptr p;
        if(pipesVector.count()>row){
            p = pipesVector.at(row);
        }else{
            return tr("unknown..%1").arg(row);
        }
        if(!p){
            return tr("unknown..2");
        }
        //return tr("AAAAAAAA");

        switch(index.column()){
            case 0:
                return QString("%1").arg(p->number);
            case 1:
                return ((p->returnCode==0)?QString("-"):QString("%1").arg(p->returnCode));
            case 2:
                return p->protocol;
            case 3:
                return p->host;
            case 4:
                return p->serverIP;
            case 5:
                return p->path;
            default:
                return QString("Unknow %1").arg(index.column());
        }

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

void QiddlerPipeTableModel::updateItem(ConnectionData_ptr p){

}

void QiddlerPipeTableModel::addItem(ConnectionData_ptr p){
    qDebug()<<"addItem...."<<p->getRequestHeader(QByteArray("Host"))<<pipesVector.count();
    ConnectionData_ptr p1 = p;
    ++pipeNumber;
    p1->number=pipeNumber;

    this->beginInsertRows(index(pipeNumber-1, 0),pipeNumber-1,pipeNumber-1);

    //TODO thread safe?
    pipesMap.value(p1->socketId,p1);
    pipesVector.append(p1);

    //QModelIndex index1 = index(pipeNumber-1, 0);
    //QModelIndex index2 = index(pipeNumber-1, 7);

    this->endInsertRows();
    //emit(dataChanged(index1,index2));
}

void QiddlerPipeTableModel::removeAllItem(){
    int l = pipesVector.size();
    for(int i=0;i<l;++i){
        ConnectionData_ptr p = pipesVector.at(i);
        p.clear();
    }
}
void QiddlerPipeTableModel::removeItems(){

}
