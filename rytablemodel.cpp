#include "rytablemodel.h"
#include "proxy/rypipedata.h"
#include <QVector>
#include <QStringList>
#include <QDebug>

RyTableModel::RyTableModel(QObject *parent) :
    QAbstractTableModel(parent),_pipeNumber(0){
}
RyTableModel::~RyTableModel(){
    blockSignals(true);
    removeAllItem();
    qDebug()<<"~RyTableModel";
}

int RyTableModel::rowCount( const QModelIndex & ) const{
    return pipesVector.count();
}
int RyTableModel::columnCount(const QModelIndex &) const{
    return 10;
}

QString rypipeDataGetDataByColumn(RyPipeData_ptr p, int column){
    switch(column){
        case 0:
            return QString::number(p->number);
        case 1:
            return QString::number(p->socketConnectionId);
        case 2:
            return ((p->responseStatus.isEmpty())?QString("-"):p->responseStatus);
        case 3:
            return p->httpVersion;
        case 4:
            return p->host;
        case 5:
            return p->serverIp;
        case 6:
            return p->path;
        case 7:
            return QString::number(p->responseBodyRawData().size());
        case 8:
            return p->getResponseHeader("Cache-Control");
        case 9:
            //qDebug()<<QString::number(p->performances.responseDone)<<QString::number(p->performances.requestBegin);
            return QString::number(p->performances.responseDone - p->performances.requestBegin);
        /*
            return QString::number(p->socketHandle);
        case 3:
            return ((p->responseStatus.isEmpty())?QString("-"):p->responseStatus);
        case 4:
            return p->httpVersion;
        case 5:
            return p->host;
        case 6:
            return p->serverIp;
        case 7:
            return p->path;
        case 8:
            return QString::number(p->responseBodyRawData().size());
        case 9:
            return p->getResponseHeader("Cache-Control");
        case 10:
            qDebug()<<QString::number(p->performances.responseDone)<<QString::number(p->performances.requestBegin);
            return QString::number(p->performances.responseDone - p->performances.requestBegin);
        */
        default:
            return QString("-");
    }
}

QVariant RyTableModel::data(const QModelIndex &index, int role) const{
    if(role == Qt::DisplayRole || role == Qt::ToolTipRole){
        int row = index.row();
        int column = index.column();
        RyPipeData_ptr p;
        if(pipesVector.count()>row){
            p = pipesVector.at(row);
        }else{
            return tr("unknown..%1").arg(row);
        }
        if(!p){
            return tr("unknown..2");
        }
        return rypipeDataGetDataByColumn(p,column);

    }else if(role == Qt::BackgroundColorRole){
        if(pipesVector.count()>index.row()){

            RyPipeData_ptr d = pipesVector.at(index.row());
            if(d->isMatchingRule){
                return Qt::cyan;
            }else if(d->responseStatus.startsWith('4') || d->responseStatus.startsWith('5')){
                return Qt::darkCyan;
            }
        }else{
            return QVariant();
        }
    }else if(role == RyTableModel::RowDataRole){
        if(pipesVector.count()>index.row()){
            return QVariant(pipesVector.at(index.row()));
        }else{
            return QVariant();
        }
    }else{
        return QVariant();
    }
}
QVariant RyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    //TODO
    QStringList headers;
    headers<<"#"<<"#2(socket)"<<"Result"<<"Protocol"<<"Host"<<"ServerIP"<<"URL"<<"Body"<<"Caching"<<"all time";

    if (orientation == Qt::Horizontal) {
        if(section< headers.size() ){
            return headers[section];
        }else{
            return QString("custom");
        }
    }
    return QVariant();
}
Qt::ItemFlags RyTableModel::flags(const QModelIndex &index) const{
    if(!index.isValid()){
        return Qt::ItemIsEnabled;
    }
    return QAbstractTableModel::flags(index);
}

bool RyTableModel::itemLessThan(RyPipeData_ptr left,int leftColumn,RyPipeData_ptr right,int rightColumn){
    if(left.isNull() || right.isNull()){
        return false;
    }
    QString leftData = rypipeDataGetDataByColumn(left,leftColumn) ;
    QString rightData = rypipeDataGetDataByColumn(right,rightColumn);
    bool isNumber=false;
    long long leftNumber = leftData.toLongLong(&isNumber);
    long long rightNumber;
    if(isNumber){
        rightNumber = rightData.toLongLong(&isNumber);
        if(isNumber){
            return leftNumber<rightNumber;
        }
    }
    return leftData < rightData;
}
bool RyTableModel::itemLessThan(const QModelIndex &left, const QModelIndex &right){
    return itemLessThan(getItem(left.row()),left.column(),getItem(right.row()),right.column());
}


RyPipeData_ptr RyTableModel::getItem(int row){
    //qDebug()<<pipesVector.size()<<row;
    if(pipesVector.size() >= row){
        return pipesVector.at(row);
    }
    qDebug()<<QString("getItem invalid row:%1").arg(row);
    return RyPipeData_ptr();
}

void RyTableModel::updateItem(RyPipeData_ptr p){
    int i = pipesMap.keys().indexOf(p->id);
    if(i!=-1){

        RyPipeData_ptr ori = pipesMap[p->id];
        pipesMap[p->id] = p;
        int j = pipesVector.indexOf(ori);
        if(j!=-1){
            pipesVector.replace(j,p);
        }

        emit dataChanged(index(i,0),index(i,columnCount()));
        emit connectionUpdated(p);
    }
}

void RyTableModel::addItem(RyPipeData_ptr p){
    //qDebug()<<"addItem...."<<p->getRequestHeader(QByteArray("Host"))<<pipesVector.count();
    RyPipeData_ptr p1 = p;
    ++_pipeNumber;
    int l = pipesVector.size();
    if(l==0){
        _pipeNumber = 1;
    }
    p1->number=_pipeNumber;
    this->beginInsertRows(index(l, 0),l,l);

    //TODO thread safe?
	pipesMap[p1->id] = p1;
	pipesVector.append(p1);

    //QModelIndex index1 = index(pipeNumber-1, 0);
    //QModelIndex index2 = index(pipeNumber-1, 7);

    this->endInsertRows();
    //emit(dataChanged(index1,index2));
	emit connectionAdded(p);
}

void RyTableModel::removeAllItem(){
    beginResetModel();
    pipesMap.clear();
    pipesVector.clear();
    //qDebug()<<"length="<<pipesVector.count();
    endResetModel();
}
void RyTableModel::removeItems(){

}
