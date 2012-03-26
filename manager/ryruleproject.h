#ifndef RYRULEPROJECT_H
#define RYRULEPROJECT_H

#include "ryrulegroup.h"

namespace manager{

class RyRuleProject{
public:
    RyRuleProject(const QScriptValue& project);

    RyRuleProject(QString localAddress,
                  QString remoteAddress="",
                  QString pwd="",
                  QString owner="");
    ~RyRuleProject();
    void saveToFile();

    void init(QString localAddress,
              QString remoteAddress="",
              QString pwd="",
              QString owner="");
    bool addRemoteRuleGroups();
    bool addLocalRuleGroups();
    void removeRuleGroup(quint64 groupId);

    bool addRuleGroups(const QString& content);
    QSharedPointer<RyRuleGroup> addRuleGroup(const QScriptValue& group,bool updateLocalFile=false);

    QString localAddress()const;
    QString toJson(bool format=false,int beforeSpace = 4);

    QString toConfigJson(bool format=true);
    bool isValid()const;
    QList<QSharedPointer<RyRule> > getMatchRules(const QString& url);
    const QList<QSharedPointer<RyRuleGroup> > groups()const;
    QSharedPointer<RyRuleGroup> groupById(quint64 groupId)const;

private:
    QString _localAddress;
    QString _remoteAddress;
    QString _pwd;
    QString _owner;
    QList<QSharedPointer<RyRuleGroup> > _groups;
    bool _isValid;

    QString _jsonCache;
    bool _needReCombineJson;
    QEventLoop *_loop;
}; // class RyRuleProject
} // namespace manager
#endif // RYRULEPROJECT_H
