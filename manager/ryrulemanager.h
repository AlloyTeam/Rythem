#ifndef RYRULEMANAGER_H
#define RYRULEMANAGER_H

#include "ryruleproject.h"

namespace manager{

class RyRuleManager:public QObject{
    Q_OBJECT
public:
    ~RyRuleManager();
    static RyRuleManager* instance();
    /*
    config formate:
    [
    {
        "localAddress":"prject_test.txt",
    },
    {
        "localAddress":"project_remote.txt",
        "remoteAddress":"http://iptton.sinaapp.com/project_test.txt",
        "pwd":"098f6bcd4621d373cade4e832627b4f6",
        "owner":"ippan"
    },
    {
        "localAddress":"project_file_not_exists",
        "remoteAddress":"http://iptton.sinaapp.com/project_test.txt"
    }

    ]
    */
    void loadLocalConfig(const QString& configFileName);
    void setupConfig(const QString& configContent);
    void setupConfig(const QScriptValue& value);


    void removeGroup(quint64 groupId);
    void removeRule(quint64 ruleId,quint64 groupId);

    const QSharedPointer<RyRuleProject> addRuleProject(const QString& groupJSONData);
    const QSharedPointer<RyRuleProject> addRuleProject(const QString& groupJSONData,
                      const QString& address,
                      bool isRemote=false,
                      const QString& host="");
    const QSharedPointer<RyRuleProject> addRuleProject(const QScriptValue& value);
    const QSharedPointer<RyRuleGroup> addGroupToLocalProject(const QString& content);//新增
    const QSharedPointer<RyRule> addRuleToGroup(const QString& msg,quint64 groupId);
    //本地project
    //可提升为远程project(需提供upload接口)
    const QSharedPointer<RyRuleProject> addLocalProject(const QString& filePath);
    //直接向远程获取project
    const QSharedPointer<RyRuleProject> addRemoteProject(const QString& url,bool fromView=false);
    //本地缓存的远程project
    //当本地解析失败或用户手动update时可更新
    const QSharedPointer<RyRuleProject> addRemoteProjectFromLocal(const QString& localAddress,
                                   const QString& remoteAddress,
                                   const QString& pwd="",
                                   const QString& owner="");

    const QSharedPointer<RyRule> updateRule(const QString& ruleJson,quint64 groupId);
    const QSharedPointer<RyRule> updateRule(QSharedPointer<RyRule> rule);
    const QSharedPointer<RyRuleGroup> updateRuleGroup(const QString& groupJson,quint64 groupId);
    const QSharedPointer<RyRuleGroup> updateRuleGroup(QSharedPointer<RyRuleGroup> ruleGroup);

    //由于可能同时命中host及远程替换两种rule，此处需返回List
    QList<QSharedPointer<RyRule> > getMatchRules(const QString& url);
    //返回header body
    QPair<QByteArray,QByteArray> getReplaceContent(QSharedPointer<RyRule> rule,const QString& url="");
    QString toJson(bool format=false)const;
private:

    QString _configFileName;
    RyRuleManager();
    // save file system (config and every project local file);
    QList<QSharedPointer<RyRuleProject> > _projects;

    QMap<quint64,QSharedPointer<RyRuleProject> > _groupToProjectMap;
    QMap<QString,QSharedPointer<RyRuleProject> > _projectFileNameToProjectMap;

    static QMutex _singletonMutex;
    static RyRuleManager* _instancePtr;

}; // class RyRuleManager

} // namespace manager

#endif // RYRULEMANAGER_H
