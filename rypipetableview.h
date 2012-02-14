#ifndef RYPIPETABLEVIEW_H
#define RYPIPETABLEVIEW_H

#include <QTableView>

class RyPipeTableView : public QTableView
{
        Q_OBJECT
    public:
        explicit RyPipeTableView(QWidget *parent = 0);
    protected:
        void mousePressEvent(QMouseEvent *event);
    signals:
        
    public slots:
        
};

#endif // RYPIPETABLEVIEW_H
