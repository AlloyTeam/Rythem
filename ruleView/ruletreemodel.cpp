#include "ruletreemodel.h"
#include <QScriptEngine>




void RuleTreeItem::setupData(const QScriptValue data,const QString){
    //[{},{}...]
    if(!data.isArray()){
        qDebug()<<"data is not array!";
        return;
    }
    QScriptValueIterator it(data);
    while(it.hasNext()){
        it.next();
        if (it.flags() & QScriptValue::SkipInEnumeration){
            continue;
        }
        RuleTreeItem *item = new RuleGroup(this);
        item->setupData(it.value());
        children.append(item);
    }
    qDebug()<<"root length "<<children.size();
}

void RuleGroup::setupData(const QScriptValue data,const QString){
    qDebug()<<"ruleGroup setupData";
    //{'name':'group1','enable':true,'rules':[...]}
    if(!data.isObject()){
        qDebug()<<"data is not object!";
        return;
    }
    QScriptValueIterator it(data);
    while(it.hasNext()){
        it.next();
        if (it.flags() & QScriptValue::SkipInEnumeration){
            continue;
        }
        if(it.name() == "name"){
            setName(it.value().toString());
        }else if(it.name() == "enable"){
            setEnabled(it.value().toBool());
        }else if(it.name() == "rules"){
            QScriptValue items = it.value();
            QScriptValueIterator it2(items);
            while(it2.hasNext()){
                it2.next();
                if (it2.flags() & QScriptValue::SkipInEnumeration){
                    continue;
                }
                RuleTreeItem *item = new RuleItem(this);
                item->setupData(it2.value());
                children.append(item);
            }
        }
    }
    qDebug()<<"group length "<<children.size();
}

QVariant RuleGroup::data(const QModelIndex& index){
    switch(index.column()){
    case 0:
        return enabled();
    case 1:
        return name();
    }
    return QVariant();
}

QVariant RuleItem::data(const QModelIndex& index){
    switch(index.column()){
    case 0:
        return enabled();
    case 1:
        return type();
    case 2:
        return pattern();
    case 3:
        return replace();
    }
    return QVariant();
}

#define DEBUG_DATA "[{'name':'group1','enable':true,'rules':[{'name':'complex address example','type':1,'enable':true,'rule':[{'pattern':'http://abc.com/a','replace':'123.com'},{'pattern':'http://abc.com/b','replace':'456.com'}]},{'name':'simple address example','type':2,'enable':true,'rule':[{'pattern':'http://abc.com','replace':'172.168.0.1'}]},{'name':'remote content example','type':3,'enable':true,'rule':[{'pattern':'http://abc.com/a.html','replace':'http://123.com/just_a_test/somedir/b.html'}]},{'name':'local file example','type':4,'enable':true,'rule':[{'pattern':'http://abc.com/b.html','replace':'files.qzmin'}] },{'name':'local files example','type':5,'enable':true,'rule':[{'pattern':'http://abc.com/c.html','replace':'C:/a.html.qzmin'}]},{'name':'local directory example','type':6,'enable':true,'rule':[{'pattern':'http://abc.com/mydir/','replace':'C:/replacement/'}]}]}]"
/*
RuleTreeModel::RuleTreeModel(const QString& json,QObject *parent)
    : QAbstractItemModel(parent){
    rootItem = new RuleTreeItem();
    if(!json.isEmpty()){
        setupModel(json);
    }
    qDebug()<<"model();";
}
*/

RuleTreeModel::RuleTreeModel(QObject *parent)
    : QAbstractItemModel(parent){
    rootItem = new RuleTreeItem();
    setupModel(QString(DEBUG_DATA));
    //RuleTreeModel(QString(DEBUG_DATA),parent);
}

RuleTreeModel::~RuleTreeModel(){
    qDebug()<<"~model";
    delete rootItem;
}

int RuleTreeModel::columnCount(const QModelIndex &  parent) const{
    int c = getItem(parent)->columnCount();
    qDebug()<<"column count = "<<c<<parent.row()<<parent.column();
    return  c;
}


QVariant RuleTreeModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    RuleTreeItem *item = getItem(index);

    return item->data(index);
}

//! [3]
Qt::ItemFlags RuleTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//! [3]

//! [4]
RuleTreeItem *RuleTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        RuleTreeItem *item = static_cast<RuleTreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return rootItem;
}
//! [4]

QVariant RuleTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return "rootItem->data(section)";

    return QVariant();
}

//! [5]
QModelIndex RuleTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0){
        return QModelIndex();
    }

    RuleTreeItem *parentItem = getItem(parent);

    RuleTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}
//! [6]

bool RuleTreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success=false;
/*
    beginInsertColumns(parent, position, position + columns - 1);
    success = rootItem->insertColumns(position, columns);
    endInsertColumns();
*/
    return success;
}

bool RuleTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    RuleTreeItem *parentItem = getItem(parent);
    bool success=false;
/*
    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();
*/
    return success;
}

//! [7]
QModelIndex RuleTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    RuleTreeItem *childItem = getItem(index);
    RuleTreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->rowCount(), 0, parentItem);
}
//! [7]

bool RuleTreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success=true;
/*
    beginRemoveColumns(parent, position, position + columns - 1);
    success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());
*/
    return success;
}

bool RuleTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    RuleTreeItem *parentItem = getItem(parent);
    bool success = true;
/*
    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();
*/
    return success;
}

//! [8]
int RuleTreeModel::rowCount(const QModelIndex &parent) const
{
    RuleTreeItem *parentItem = getItem(parent);

    return parentItem->rowCount();
}
void RuleTreeModel::setupModel(const QString& json){
    QString data = json;
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("("+json+")");
    if(engine.hasUncaughtException()){
        qDebug()<<"error!";
        return;
    }
    rootItem->setupData(value);
}

bool RuleTreeModel::setData(const QModelIndex &index, const QVariant &value,
                        int role)
{
    if (role != Qt::EditRole)
        return false;
    bool result=false;
/*
    RuleTreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);
*/
    return result;
}

bool RuleTreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;
    bool result=false;
/*
    bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);
*/
    return result;
}

