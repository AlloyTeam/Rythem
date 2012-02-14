#ifndef RULETREEMODEL_H
#define RULETREEMODEL_H

#include <QAbstractItemModel>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QtCore>

class RuleTreeItem{
public:
    RuleTreeItem(RuleTreeItem* parent=0){
    }

    RuleTreeItem(const RuleTreeItem* other){
        qDebug()<<"copy ctor";
        children.append(other->children);
    }

    virtual ~RuleTreeItem(){
        qDebug()<<"~RuleTreeItem";
        QListIterator<RuleTreeItem*> it(children);
        while(it.hasNext()){
            RuleTreeItem *item = it.next();
            delete item;
        }
        children.clear();
    }


    virtual void setupData(const QScriptValue data,const QString name="");

    virtual QVariant data(const QModelIndex& index){
        return QString("rules");
    }

    virtual int columnCount()const{
        return 6;
    }
    virtual int rowCount()const{
        return children.size();
    }
    virtual bool isRoot()const{
        return true;
    }
    virtual bool isGroup()const{
        return false;
    }
    virtual bool isRule()const{
        return false;
    }
    virtual bool isComplesRule()const{
        return false;
    }
    virtual void setEnabled(bool isEnabaled){
        _isEnabled = isEnabaled;
    }
    virtual bool enabled()const{
        return _isEnabled;
    }
    virtual RuleTreeItem* child(int i){
        if(children.size()>i){
            return children.at(i);
        }
        return NULL;
    }
    RuleTreeItem *parent()const{
        return _parent;
    }

protected:
    QList<RuleTreeItem*> children;
    bool _isEnabled;
    RuleTreeItem *_parent;
};

class RuleGroup:public RuleTreeItem{
public:
    RuleGroup(RuleTreeItem* parent){
        qDebug()<<"RuleGroup";
        _parent = parent;
    }
    ~RuleGroup(){
        qDebug()<<"~group";
    }

    virtual void setupData(const QScriptValue data,const QString name="");

    virtual QVariant data(const QModelIndex& index);

    virtual int columnCount()const{
        return 2;
    }
    virtual bool isRoot()const{
        return false;
    }
    virtual bool isGroup()const{
        return true;
    }
    void setName(const QString& name){
        _name = name;
    }
    QString name()const{
        return _name;
    }
protected:
    QString _name;
};
class RuleItem:public RuleTreeItem{
public:
    RuleItem(RuleTreeItem* parent){
        qDebug()<<"RuleItem";
        _parent = parent;
    }
    enum RuleType{
        RuleTypeSimpleHost = 0,
        RuleTypeLocalContent =  1,
        RuleTypeRemoteContent = 2,
        RuleTypeLocalMergeContent = 3
    };

    virtual void setupData(const QScriptValue data,const QString name=""){
        //{'type':2,'enable':true,'rule':[{'pattern':'http://abc.com','replace':'172.168.0.1'}]}
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
            if(it.name() == "type"){
                setType(it.value().toInt32());
            }else if(it.name() == "enable"){
                setEnabled(it.value().toBool());
            }else if(it.name() == "rule"){

                setReplace(it.value().property("0").property("replace").toString());
                setPattern(it.value().property("0").property("pattern").toString());
            }
        }
    }
    virtual QVariant data(const QModelIndex& index);

    virtual int columnCount()const{
        return 4;
    }
    virtual int rowCount()const{
        return 0;
    }
    virtual bool isRoot()const{
        return false;
    }
    virtual bool isRule()const{
        return true;
    }
    int type()const{
        return _type;
    }
    void setType(int theType){
        _type = theType;
    }

    void setPattern(const QString& pattern){
        _pattern = pattern;
    }
    QString pattern()const{
        return _pattern;
    }

    void setReplace(const QString& replace){
        _replace = replace;
    }
    QString replace()const{
        return _replace;
    }



protected:
    QString _name;
    int _type;
    QString _pattern;
    QString _replace;
};

class RuleTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    RuleTreeModel(QObject *parent=0);
    //RuleTreeModel(const QString& json,QObject *parent = 0);
    ~RuleTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole);

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex());
    bool removeColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex());
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());

    void setupModel(const QString& json);

private:
    RuleTreeItem *getItem(const QModelIndex &index) const;

    RuleTreeItem *rootItem;
};

#endif // RULETREEMODEL_H
