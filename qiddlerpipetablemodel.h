#ifndef QIDDLERPIPETABLEMODEL_H
#define QIDDLERPIPETABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QMap>
#include <QSharedPointer>
class PipeData;
class QiddlerPipeTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit QiddlerPipeTableModel(QObject *parent = 0);
    QMap<int,QSharedPointer<PipeData> > pipesMap;
    QVector<QSharedPointer<PipeData> > pipesVector;

    int rowCount( const QModelIndex & parent ) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void addItem(QSharedPointer<PipeData> p);
signals:

public slots:

private:
    int pipeNumber;
};

#endif // QIDDLERPIPETABLEMODEL_H
