#ifndef RYRULEMANAGER_H
#define RYRULEMANAGER_H

#include <QtCore>
#include <QPair>
#include <QScriptengine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QtNetwork>
#include <QNetworkAccessManager>

extern QString appPath;

class RyRule{
public:
    enum RuleType{
        COMPLEX_ADDRESS_REPLACE = 1,
        SIMPLE_ADDRESS_REPLACE = 2,
        REMOTE_CONTENT_REPLACE = 3,
        LOCAL_FILE_REPLACE = 4,
        LOCAL_FILES_REPLACE = 5,
        LOCAL_DIR_REPLACE = 6
    };
    RyRule(quint64 groupId,const QScriptValue& rule);
    RyRule(quint64 groupId,int type,const QString& pattern,const QString& replace,bool enable = true);
    RyRule(quint64 id,quint64 groupId,int type,const QString& pattern,const QString& replace,bool enable=true);
    QString toJSON(bool format=false)const;

    int type();
    QString pattern();
    QString replace();

    void update(const QScriptValue& value){
        qDebug()<<"before"<<this->toJSON();

        this->_type = value.property("type").toInt32();
        this->enabled = value.property("enable").toBool();
        this->_pattern = value.property("rule").property("pattern").toString();
        this->_replace = value.property("rule").property("replace").toString();
        qDebug()<<"after"<<this->toJSON();
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
};



class RyRuleReplaceContent{
public:
    RyRuleReplaceContent(QSharedPointer<RyRule> rule, const QString& url="");
    QPair<QByteArray,QByteArray> getReplaceContent(const QString& url);
    QPair<QByteArray,QByteArray> getReplaceContent();
    void setUrl(const QString&);
    void setRule(QSharedPointer<RyRule>);
private:
    QString _url;
    QSharedPointer<RyRule> _rule;
};

class RyRuleGroup{
public:
    RyRuleGroup(const QScriptValue& group);
    /*
    RyRuleGroup(const QString& groupName,bool isOwner=false);
    RyRuleGroup(quint64 groupId,const QString& groupName,bool isOwner=false);
    */
    QString toJSON(bool format=false)const;
    void addRules(const QString& rules);
    void addRules(const QScriptValue& rules);
    QSharedPointer<RyRule> addRule(const QScriptValue& value);
    QSharedPointer<RyRule> addRule(QSharedPointer<RyRule> rule);
    QSharedPointer<RyRule> addRule(int type,QString pattern,QString replace);
    QSharedPointer<RyRule> addRule(quint64 ruleId,int type,QString pattern,QString replace);
    QSharedPointer<RyRule> updateRule(const QString& ruleJson);

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
};

class RyRuleProject{
public:
    RyRuleProject(const QScriptValue& project){
        QString localAddress = project.property("localAddress").toString();
        QString remoteAddress = project.property("remoteAddress").toString();
        QString owner = project.property("owner").toString();
        QString pwd = project.property("pwd").toString();
        init(localAddress,remoteAddress,pwd,owner);
    }

    RyRuleProject(QString localAddress,
                  QString remoteAddress="",
                  QString pwd="",
                  QString owner=""){
        init(localAddress,remoteAddress,pwd,owner);
    }
    ~RyRuleProject(){
        //TODO save
        qDebug()<<"~RyRuleProject";
        saveToFile();
    }
    void saveToFile(){
        QFile f(_localAddress);
        if(f.open(QIODevice::WriteOnly | QIODevice::Text)){
            QString str="{'groups':[";
            QStringList groupStrList;
            for(int i=0;i<_groups.length();++i){
               QSharedPointer<RyRuleGroup> g = _groups.at(i);
               groupStrList<<g->toJSON();
            }

            str+=groupStrList.join(",")+"]}";
            f.write( QByteArray().append(str.toUtf8()));
            qDebug()<<"save to file";
            f.close();
        }else{
            qDebug()<<"cache remote rule failure"<<_localAddress;
        }
    }

    void init(QString localAddress,
              QString remoteAddress="",
              QString pwd="",
              QString owner=""){
        _needReCombineJson = true;
        _isValid = false;
        _localAddress = localAddress;
        _remoteAddress = remoteAddress;
        _pwd= pwd;
        _owner=owner;

        _isValid = addLocalRuleGroups();
        if(!_isValid){
            if(!_remoteAddress.isEmpty()){
                //remote group
                // and local content invalid
                // get from remote
                _isValid = addRemoteRuleGroups();
                if(_isValid){//如果成功，写回本地文件缓存
                    _needReCombineJson = false;
                    saveToFile();
                }
            }else{
                qDebug()<<"no local content && remote addresss is empty";
                _isValid = false;
            }
        }
    }
    bool addRemoteRuleGroups(){
        //qDebug()<<"getting remote rule"<<_remoteAddress;
        QTimer timeout;
        QNetworkAccessManager manager;
        QNetworkReply* reply;
        QEventLoop loop;
        timeout.singleShot(3000,&loop,SLOT(quit()));//3秒内不返回内容就判断为失败
        QNetworkProxyFactory::setUseSystemConfiguration(true);
        manager.connect(&manager,SIGNAL(finished(QNetworkReply*)),&loop,SLOT(quit()));
        reply = manager.get(QNetworkRequest(QUrl(_remoteAddress)));
        loop.exec();
        QTextCodec* oldCodec = QTextCodec::codecForCStrings();
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
        QString content = reply->readAll();
         QTextCodec::setCodecForCStrings(oldCodec);
        _jsonCache = content;
        qDebug()<<"remote content:"<<content;
        return addRuleGroups(content);
    }
    bool addLocalRuleGroups(){
        QFile f(_localAddress);
        if(f.open(QIODevice::ReadOnly | QIODevice::Text)){
            QByteArray content = f.readAll();
            return addRuleGroups(QString::fromUtf8(content.data()));
        }else{
            qDebug()<<"cannot open file:"<<_localAddress;
            return false;
        }
    }
    void removeRuleGroup(quint64 groupId){
        //TODO
        QListIterator<QSharedPointer<RyRuleGroup> > gIt(_groups);
        while(gIt.hasNext()){
            QSharedPointer<RyRuleGroup> g = gIt.next();
            if(g->groupId() == groupId){
                _groups.removeOne(g);
                break;
            }
        }
    }

    bool addRuleGroups(const QString& content){
        QScriptEngine engine;
        QScriptValue value = engine.evaluate("("+content+")");
        if(engine.hasUncaughtException()){
            qDebug()<<"invalid group data"<<content;
            return false;
        }else{
            if(!value.property("groups").isValid()){
                qDebug()<<"invalid group data"<<content;
                return false;
            }
            QScriptValueIterator gIt(value.property("groups"));
            while(gIt.hasNext()){
                gIt.next();
                if(gIt.flags() & QScriptValue::SkipInEnumeration){
                    continue;
                }
                QScriptValue g = gIt.value();
                addRuleGroup(g);
            }
            //qDebug()<<"got project!";
            return true;
        }
    }
    QSharedPointer<RyRuleGroup> addRuleGroup(const QScriptValue& group,bool updateLocalFile=false){
        RyRuleGroup *g = new RyRuleGroup(group);
        QSharedPointer<RyRuleGroup> groupPtr(g);
        //qDebug()<<"group added:"<<g->toJSON();
        _groups.append(groupPtr);
        return groupPtr;
    }

    QString localAddress()const{
        return _localAddress;
    }
    QString toJson(bool format=false){
        if(format){
            //TODO
        }
        QString str="{'groups':[";
        QStringList groupStrList;
        for(int i=0;i<_groups.length();++i){
           QSharedPointer<RyRuleGroup> g = _groups.at(i);
           groupStrList<<g->toJSON();
        }

        str+=groupStrList.join(",")+"]}";
        return str;
    }

    QString toConfigJson(bool format=false){
        _needReCombineJson = true;//TODO
        if(!_needReCombineJson){
            return _jsonCache;
        }else{
            if(format){
                //TODO
            }
            QString localAddressEscaped = _localAddress;
            localAddressEscaped.replace("\\","\\\\");
            localAddressEscaped.replace("'","\\'");
            QString ret = "{'localAddress':'"+localAddressEscaped+"'";
            if(!_remoteAddress.isEmpty()){
                QString remoteAddressEscaped = _remoteAddress;
                remoteAddressEscaped.replace("\\","\\\\");
                remoteAddressEscaped.replace("'","\\'");
                ret+=",'remoteAddress':'"+remoteAddressEscaped+"'";
                ret+=",'pwd':'"+_pwd+"'";
                QString ownerEscaped = _owner;
                ownerEscaped.replace("\\","\\\\");
                ownerEscaped.replace("'","\\'");
                ret+=",'owner':'"+ownerEscaped+"'";
            }
            ret+="}";
            _jsonCache = ret;
            _needReCombineJson = false;
        }
        return _jsonCache;
    }
    bool isValid()const{
        return _isValid;
    }
    QList<QSharedPointer<RyRule> > getMatchRules(const QString& url){
        QList<QSharedPointer<RyRule> > ret;
        QListIterator<QSharedPointer<RyRuleGroup> > it(_groups);
        //qDebug()<<"project::getMatchRules: "<<url<<_groups.length();
        while(it.hasNext()){
            QSharedPointer<RyRuleGroup> g = it.next();
            //qDebug()<<"project::getMatchRules: "<<url<<g->toJSON();
            QList<QSharedPointer<RyRule> > gMatched = g->getMatchRules(url);
            ret.append(gMatched);
        }
        return ret;
    }
    const QList<QSharedPointer<RyRuleGroup> > groups()const{
        return _groups;
    }
    QSharedPointer<RyRuleGroup> groupById(quint64 groupId)const{
        QListIterator<QSharedPointer<RyRuleGroup> > it(_groups);
        while(it.hasNext()){
            QSharedPointer<RyRuleGroup> g = it.next();
            if(g->groupId() == groupId){
                return g;
            }
        }
        return QSharedPointer<RyRuleGroup>();
    }

private:
    QString _localAddress;
    QString _remoteAddress;
    QString _pwd;
    QString _owner;
    QList<QSharedPointer<RyRuleGroup> > _groups;
    bool _isValid;

    QString _jsonCache;
    bool _needReCombineJson;
};

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
    void removeRule(quint64 ruleId);

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
    QString toJson()const;
private:

    QString _configFileName;
    RyRuleManager();
    // save file system (config and every project local file);
    QList<QSharedPointer<RyRuleProject> > _projects;

    QMap<quint64,QSharedPointer<RyRuleProject> > _groupToProjectMap;
    QMap<QString,QSharedPointer<RyRuleProject> > _projectFileNameToProjectMap;

    static QMutex _singletonMutex;
    static RyRuleManager* _instancePtr;

};

#endif // RYRULEMANAGER_H
