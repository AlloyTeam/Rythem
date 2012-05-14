#ifndef RYTABLESORTFILTERPROXYMODEL_H
#define RYTABLESORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <rytablemodel.h>

class RyTableSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:

    typedef bool (*FilterCallBack)(RyPipeData_ptr);
    enum Filter{
        NoFilter = 0,
        NoImageFilter = 0x01,
        No304Filter = 0x02,
        CustomFilter = 0x04,
        OnlyMatchingFilter = 0x08,
        HideTunnelFilter = 0x16
    };

    explicit RyTableSortFilterProxyModel(QObject *parent = 0);
    void setSourceModel(RyTableModel *sourceModel);
    RyTableModel *sourceModel()const;
    RyPipeData_ptr getItem(int sourceRow);
    RyPipeData_ptr getItem(const QModelIndex& proxyIndex);

    void setFilter(int filter);
    int filter()const;
    void setCustomeFilter(FilterCallBack);

    void removeAllItem();
    void updateItem(RyPipeData_ptr);
    void addItem(RyPipeData_ptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

signals:
    
public slots:
    
private:
    RyTableModel *_sourceModel;
    int _filterFlags;
    FilterCallBack _filterCallback;

    QMutex _filterFlagMutex;
};

#endif // RYTABLESORTFILTERPROXYMODEL_H
