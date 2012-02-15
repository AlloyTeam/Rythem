#ifndef RULETREEDELEGATE_H
#define RULETREEDELEGATE_H

#include <QItemDelegate>

class RuleTreeDelegate : public QItemDelegate
{
        Q_OBJECT
    public:
        explicit RuleTreeDelegate(QObject *parent = 0);

        // painting
        virtual void paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

        virtual QSize sizeHint(const QStyleOptionViewItem &option,
                               const QModelIndex &index) const;

        /*
        // editing
        virtual QWidget *createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const;

        virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;

        virtual void setModelData(QWidget *editor,
                                  QAbstractItemModel *model,
                                  const QModelIndex &index) const;

        virtual void updateEditorGeometry(QWidget *editor,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const;

        // for non-widget editors
        virtual bool editorEvent(QEvent *event,
                                 QAbstractItemModel *model,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index);
        */
    signals:
        
    public slots:
        
};

#endif // RULETREEDELEGATE_H
