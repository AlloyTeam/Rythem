#ifndef RYRULE_H
#define RYRULE_H

#include <QtCore>
#include <QtNetwork>
#include <QtScript>

#include <QDebug>


namespace rule{

class RyRule{
public:
    enum RuleType{
        COMPLEX_ADDRESS_REPLACE = 1,
        SIMPLE_ADDRESS_REPLACE = 2,
        REMOTE_CONTENT_REPLACE = 3,
        LOCAL_FILE_REPLACE = 4,
        LOCAL_FILES_REPLACE = 5,
        LOCAL_DIR_REPLACE = 6,
        LOCAL_FILES_REPLACE2 = 7
    };
    RyRule(quint64 groupId,const QScriptValue& rule);
    RyRule(quint64 groupId,int type,const QString& pattern,const QString& replace,bool enable = true);
    RyRule(quint64 id,quint64 groupId,int type,const QString& pattern,const QString& replace,bool enable=true);

    QString toJSON(bool format=false,int space=16)const;

    int type();
    QString pattern();
    QString replace();

    void update(const QScriptValue& value){
        //qDebug()<<"before"<<this->toJSON();
        this->_type = value.property("type").toInt32();
        this->enabled = value.property("enable").toBool();
        this->_pattern = value.property("rule").property("pattern").toString();
        this->_replace = value.property("rule").property("replace").toString();
        //qDebug()<<"after"<<this->toJSON();
    }

    quint64 ruleId()const{
        return _ruleId;
    }
    quint64 groupId()const{
        return _groupId;
    }
    bool enabled;

    static quint64 getNextRuleId();
    static QString getMimeType(const QString& key,const QString& defaultMimeType="text/plain");


private:
    void init(quint64 id,quint64 groupId,int type,const QString& pattern,const QString& replace,bool enable){
        _ruleId = id;
        _groupId = groupId;
        _type = type;
        _pattern = pattern;
        _replace = replace;
        enabled = enable;
        //qDebug()<<enabled;
    }

    quint64 _ruleId;
    quint64 _groupId;
    int _type;
    QString _pattern;
    QString _replace;

    static quint64 _nextRuleId;
    static QMutex mutex;
    static QMap<QString,QString> _mimeTypeMap;
}; // class RyRule
} // namespace manager
#endif // RYRULE_H
