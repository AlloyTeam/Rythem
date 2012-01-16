#ifndef QIRULEMANAGER_H
#define QIRULEMANAGER_H

#include <QObject>

class QiRuleManager : public QObject
{
    Q_OBJECT
public:
    inline QiRuleManager(QObject *parent = 0):QObject(parent){
    }
    static QiRuleManager* instance();
    
signals:
    
public slots:
    
};

#endif // QIRULEMANAGER_H
