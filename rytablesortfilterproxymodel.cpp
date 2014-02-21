#include "rytablesortfilterproxymodel.h"

RyTableSortFilterProxyModel::RyTableSortFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent){
    _filterFlags = NoFilter;
}

void RyTableSortFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel){
    QSortFilterProxyModel::setSourceModel(sourceModel);
    _sourceModel = dynamic_cast<RyTableModel*>(sourceModel);
}
RyTableModel *RyTableSortFilterProxyModel::sourceModel()const{
    return _sourceModel;
}

bool RyTableSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &) const{
    //qDebug()<<"filterAcceptsRow comparing";
    RyPipeData_ptr p = _sourceModel->getItem(sourceRow);
    if(p.isNull()){
        qDebug()<<"filterAcceptsRow got null";
        return true;
    }

    //先处理文本过滤
    if(!_filterText.isEmpty()){
        QStringList hostFilterList = _filterText.split("|");
        bool matchHost = false;
        foreach(QString hostFilter,hostFilterList){
            if(p->fullUrl.indexOf(hostFilter) != -1){
                matchHost = true;
            }
        }
        if(!matchHost){
            return matchHost;
        }
    }
    //qDebug()<<"filterAcceptsRow comparing"<<p->responseStatus;
    if(_filterFlags & NoImageFilter){
        if(p->getResponseHeader("Content-Type").toLower().indexOf("image")!=-1){
        //if(!noImageFilterAccepted(p)){
            return false;
        }
    }
    if(_filterFlags & No304Filter){
        if(p->responseStatus == "304"){
            return false;
        }
    }
    if(_filterFlags & OnlyMatchingFilter){
        if(!p->isMatchingRule){
            return false;
        }
    }
    if(_filterFlags & HideTunnelFilter){
        if(p->isConnectTunnel){
            return false;
        }
    }
    if(_filterFlags & CustomFilter){
        //if(!_filterCallback(p)){
        //    return false;
        //}
    }
    return true;
}
bool RyTableSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const{
    //qDebug()<<QString("row:%1,col:%2").arg(left.row()).arg(left.column());
    //qDebug()<<QString("row:%1,col:%2").arg(right.row()).arg(right.column());
    //QModelIndex leftSource = mapToSource(left);
    //QModelIndex rightSource = mapToSource(right);
    return _sourceModel->itemLessThan(left,right);
}
RyPipeData_ptr RyTableSortFilterProxyModel::getItem(int sourceRow){
    return _sourceModel->getItem(sourceRow);
}
RyPipeData_ptr RyTableSortFilterProxyModel::getItem(const QModelIndex& proxyIndex){
    return getItem(mapToSource(proxyIndex).row());
}

void RyTableSortFilterProxyModel::setFilterText(QString filterText){
    _filterText = filterText;
    invalidateFilter();
}

void RyTableSortFilterProxyModel::setFilter(int flag){
    int oldFlags = _filterFlags;
    _filterFlags = flag;
    qDebug()<<"setting flags";
    if(oldFlags!=_filterFlags){
        invalidateFilter();
    }
}

int RyTableSortFilterProxyModel::filter()const{
    return _filterFlags;
}
void RyTableSortFilterProxyModel::setCustomeFilter(FilterCallBack filtercb){
    _filterCallback = filtercb;
}
void RyTableSortFilterProxyModel::setMaxRequestSize(int maxSize){
    sourceModel()->setMaxRequestSize(maxSize);
    invalidateFilter();
}

void RyTableSortFilterProxyModel::removeAllItem(){
    sourceModel()->removeAllItem();
    QSortFilterProxyModel::clear();
}

void RyTableSortFilterProxyModel::updateItem(RyPipeData_ptr p){
    sourceModel()->updateItem(p);
    //emit dataChanged(index(0,0),index(rowCount()-1,columnCount()-1));
}

void RyTableSortFilterProxyModel::addItem(RyPipeData_ptr p){
    sourceModel()->addItem(p);
    invalidateFilter();
    //emit dataChanged(index(0,0),index(rowCount()-1,columnCount()-1));
}
