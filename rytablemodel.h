#ifndef RYTABLEMODEL_H
#define RYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QMap>
#include <QSharedPointer>
#include "proxy/rypipedata.h"
#include <QtCore>

class RyTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum Role{
        RowDataRole = Qt::UserRole + 1
    };

    explicit RyTableModel(QObject *parent = 0);
    ~RyTableModel();
    QMap<int,RyPipeData_ptr > pipesMap;
    QVector<RyPipeData_ptr > pipesVector;

    int rowCount( const QModelIndex & parent ) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool itemLessThan(RyPipeData_ptr left,int leftColumn,RyPipeData_ptr right,int rightColumn);
    bool itemLessThan(const QModelIndex& left,const QModelIndex& right);
public slots:
    void addItem(RyPipeData_ptr p);
    void removeItems();
    //void removeItemAtIndex();
    void removeAllItem();

    RyPipeData_ptr getItem(int column);

    void updateItem(RyPipeData_ptr p);

signals:
	//signals emit when connections are added/updated/removed
    void connectionAdded(RyPipeData_ptr p);
    void connectionUpdated(RyPipeData_ptr p);
    void connectionRemoved(RyPipeData_ptr p);

public slots:

private:
    int _pipeNumber;
    int _sortingColumn;
};

#endif // RYTABLEMODEL_H
