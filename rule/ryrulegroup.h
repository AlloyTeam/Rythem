#ifndef RYRULEGROUP_H
#define RYRULEGROUP_H

#include "ryrule.h"

namespace rule{

class RyRuleGroup{
public:
    RyRuleGroup(const QScriptValue& group);
    /*
    RyRuleGroup(const QString& groupName,bool isOwner=false);
    RyRuleGroup(quint64 groupId,const QString& groupName,bool isOwner=false);
    */
    QString toJSON(bool format=false,int space = 12)const;
    void addRules(const QString& rules);
    void addRules(const QScriptValue& rules);
    QSharedPointer<RyRule> addRule(const QScriptValue& value);
    QSharedPointer<RyRule> addRule(QSharedPointer<RyRule> rule);
    QSharedPointer<RyRule> addRule(int type,QString pattern,QString replace);
    QSharedPointer<RyRule> addRule(quint64 ruleId,int type,QString pattern,QString replace);
    QSharedPointer<RyRule> updateRule(const QString& ruleJson);
    void removeRule(quint64 ruleId);
    void update(const QString& groupJson);

    quint64 groupId()const;
    QList<QSharedPointer<RyRule> > getMatchRules(const QString& url);
    bool enabled;
    bool isRemote;
    QString address;
    QString remoteHost;

    static quint64 getNextGroupId();
private:
    quint64 _groupId;
    QString _groupName;
    bool _isOwner;
    QString _ownerPassword;
    bool _isRemote;

    QList< QSharedPointer<RyRule> > _rules;

    static quint64 _nextGroupId;
    static QMutex mutex;
}; // class RyRuleGroup

} // namespace manager

#endif // RYRULEGROUP_H
