#include "ryrulemanager.h"
#include "ryrulereplacecontent.h"

extern QString appPath;

using namespace rule;


QMutex RyRuleManager::_singletonMutex;
RyRuleManager* RyRuleManager::_instancePtr = 0;
RyRuleManager* RyRuleManager::instance(){
    if(!_instancePtr){
        //未做检测，请务必在main.c初始化一次
        _instancePtr = new RyRuleManager();
    }
    return _instancePtr;
}

RyRuleManager::RyRuleManager():QObject(qApp){
}
RyRuleManager::~RyRuleManager(){
    qDebug()<<"~rulemanager";
    //save config

    if(_configFileName.isEmpty()){
        _configFileName = appPath+"/rythem_config.txt";
    }
    QFile f(_configFileName);
    bool canFileOpen = f.open(QIODevice::WriteOnly | QIODevice::Text);
    QStringList configStr;

    //save all projects
    QListIterator<QSharedPointer<RyRuleProject> > rpIt(_projects);
    while(rpIt.hasNext()){
        QSharedPointer<RyRuleProject> rp = rpIt.next();
        //qDebug()<<"saving to config"<<rp->toConfigJson();
        configStr<<rp->toConfigJson(true);
        rp.clear();
    }
    if(canFileOpen){
        QString newLine = "\n";
        QString str = "["+newLine+configStr.join(","+newLine)+newLine+"]";
        QByteArray ba;
        ba.append(str);
        f.write(ba);
        f.close();
    }
    _projects.clear();
    _projectFileNameToProjectMap.clear();
    _groupToProjectMap.clear();
    qDebug()<<"~rulemanager done";
}

void RyRuleManager::loadLocalConfig(const QString& configFileName){
    _configFileName = configFileName;
    QFile file(configFileName);
    if(file.open(QIODevice::ReadOnly)){
        qDebug()<<"opened rythem config file "<<configFileName;
        QString content = file.readAll();
        file.close();
        setupConfig(content);
    }else{
        qDebug()<<"cannot open config file:"<<configFileName;
    }
}

void RyRuleManager::setupConfig(const QString& configContent){
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("(" + configContent + ")");
    QScriptValueIterator projectIt(value);
    while(projectIt.hasNext()){
        projectIt.next();
        if (projectIt.flags() & QScriptValue::SkipInEnumeration){
            continue;
        }
        QScriptValue project = projectIt.value();
        addRuleProject(project);
    }
}


void RyRuleManager::removeGroup(quint64 groupId){
    if(_groupToProjectMap.contains(groupId)){
        QSharedPointer<RyRuleProject> p =
                _groupToProjectMap.take(groupId);
        p->removeRuleGroup(groupId);
    }
}

void RyRuleManager::removeRule(quint64 ruleId,quint64 groupId){
    if(_groupToProjectMap.contains(groupId)){
        QSharedPointer<RyRuleProject> p =
                _groupToProjectMap[groupId];
        QSharedPointer<RyRuleGroup> g = p->groupById(groupId);
        if(!g.isNull()){
            g->removeRule(ruleId);
        }
    }
}

const QSharedPointer<RyRuleProject> RyRuleManager::addRuleProject(const QScriptValue &project){
    RyRuleProject *rp = new RyRuleProject(project);
    bool isProjectValid = rp->isValid();
    if(!isProjectValid){
        qDebug()<<"invalid project";
        return QSharedPointer<RyRuleProject>();
    }else{
        QSharedPointer<RyRuleProject> rpPtr(rp);
        _projectFileNameToProjectMap[rp->localAddress()] = rpPtr;
        const QList<QSharedPointer<RyRuleGroup> > groups = rpPtr->groups();
        QListIterator<QSharedPointer<RyRuleGroup> > it(groups);
        while(it.hasNext()){
            QSharedPointer<RyRuleGroup> group = it.next();
            _groupToProjectMap[group->groupId()]=rpPtr;
        }
        _projects.append(rpPtr);
        return rpPtr;
    }
}

const QSharedPointer<RyRuleProject> RyRuleManager::addRuleProject(const QString& groupJSONData){
    //qDebug()<<groupJSONData;
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("(" + groupJSONData + ")");
    return addRuleProject(value);
}

const QSharedPointer<RyRuleProject> RyRuleManager::addRemoteProject(const QString& url,bool){
    QString urlEscaped = url;
    urlEscaped.replace("\\","\\\\");
    urlEscaped.replace("\'","\\'");
    int i=1;
    QString localAddress = appPath+"/remoteRuleCache_";
    while(true){
        if(i==0){//不可能出现的情况(当远程rule个数越过int最大值时才会出现
            break;
        }
        if(!QFile::exists(localAddress+QString::number(i)+".txt")){
            localAddress += QString::number(i)+".txt";
            break;
        }
        i++;
    }
    localAddress.replace("\\","\\\\");
    localAddress.replace("\'","\\'");
    return addRuleProject("{'localAddress':'"+localAddress+"','remoteAddress':'"+urlEscaped+"'}");
}

const QSharedPointer<RyRuleGroup> RyRuleManager::addGroupToLocalProject(const QString& content){
    // add to project default_local_project
    QString projectName = "default_local_project.txt";
    QString defaultProjectFullFileName = appPath+"/"+projectName;
    if(_projectFileNameToProjectMap.contains(defaultProjectFullFileName)){
        QScriptEngine engine;
        QScriptValue value = engine.evaluate("("+content+")");
        qDebug()<<"default project exists";
        QSharedPointer<RyRuleProject> project = _projectFileNameToProjectMap.value(defaultProjectFullFileName);
        QSharedPointer<RyRuleGroup> group = project->addRuleGroup(value,true);
        _groupToProjectMap[group->groupId()] = project;
        return group;
    }else{
        qDebug()<<"default project not exists";
        QFile f(defaultProjectFullFileName);
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        QByteArray ba;
        ba.append(QString("{\"groups\":[")+content+"]}");
        //qDebug()<<QString(ba);
        f.write(ba);
        f.close();
        QScriptEngine engine;
        QScriptValue project = engine.globalObject();
        project.setProperty("localAddress",QScriptValue(defaultProjectFullFileName));
        //qDebug()<<project.property("localAddress").toString();
        QSharedPointer<RyRuleProject> p = addRuleProject(project);
        //qDebug()<<"project = "<<p->toJson();

        //TODO
        QList<QSharedPointer<RyRuleGroup> > groups = p->groups();
        //qDebug()<<"length = "<<QString::number(groups.length());
        if(groups.length()>0){
            QSharedPointer<RyRuleGroup> group = groups.last();
            _groupToProjectMap[group->groupId()] = p;
            return group;
        }
        return QSharedPointer<RyRuleGroup>();
    }
}
const QSharedPointer<RyRule> RyRuleManager::addRuleToGroup(const QString& msg,quint64 groupId){
    if(_groupToProjectMap.contains(groupId)){
        QSharedPointer<RyRuleProject> p = _groupToProjectMap[groupId];
        QSharedPointer<RyRuleGroup>  g = p->groupById(groupId);
        QScriptEngine engine;
        QScriptValue v = engine.evaluate("("+msg+")");
        return g->addRule(v);

    }else{
        qDebug()<<"not such group";
        return QSharedPointer<RyRule>();
    }
}

const QSharedPointer<RyRuleProject> RyRuleManager::addLocalProject(const QString& filePath){
    /*
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QString content = file.readAll();
        file.close();
        addRuleGroups(content,filePath);
    }else{
        qWarning() << "[RuleManager] local config file open fail";
    }
    */
    if(QFile::exists(filePath)){
        QScriptEngine engine;
        QScriptValue project = engine.globalObject();
        project.setProperty("localAddress",QScriptValue(filePath));
        return this->addRuleProject(project);
    }
    return QSharedPointer<RyRuleProject>();
}

const QSharedPointer<RyRule> RyRuleManager::updateRule(const QString& ruleJson,quint64 groupId){
    if(_groupToProjectMap.contains(groupId)){
        QSharedPointer<RyRuleProject> p = _groupToProjectMap[groupId];
        if(!p.isNull()){
            return p->groupById(groupId)->updateRule(ruleJson);
        }
    }
    return QSharedPointer<RyRule>();
}

const QSharedPointer<RyRule> RyRuleManager::updateRule(QSharedPointer<RyRule> rule){
    qDebug()<<"updateRule";//TODO
    return rule;
}
const QSharedPointer<RyRuleGroup> RyRuleManager::updateRuleGroup(const QString& groupJson,quint64 groupId){
    if(_groupToProjectMap.contains(groupId)){
        QSharedPointer<RyRuleProject> p = _groupToProjectMap[groupId];
        QSharedPointer<RyRuleGroup> g = p->groupById(groupId);
        if(!g.isNull()){
            g->update(groupJson);
            return g;
        }else{
            qDebug()<<"cannot found group";
        }
    }
    return QSharedPointer<RyRuleGroup>();
}
const QSharedPointer<RyRuleGroup> RyRuleManager::updateRuleGroup(QSharedPointer<RyRuleGroup> ruleGroup){
    qDebug()<<"updateRuleGroup";//TODO
    return ruleGroup;
}

//由于可能同时命中host及远程替换两种rule，此处需返回List
QList<QSharedPointer<RyRule> > RyRuleManager::getMatchRules(const QString& url){
    QList<QSharedPointer<RyRule> > ret;
    QListIterator<QSharedPointer<RyRuleProject> > it(_projects);
    while(it.hasNext()){
        QSharedPointer<RyRuleProject> p = it.next();
        ret.append(p->getMatchRules(url));
    }
    //qDebug()<<"match rule length = "<<QString::number(ret.length());
    return ret;
}

//返回header body
QPair<QByteArray,QByteArray> RyRuleManager::getReplaceContent(QSharedPointer<RyRule> rule,const QString& url){
    RyRuleReplaceContent rc(rule,url);
    return rc.getReplaceContent();
}

QString RyRuleManager::toJson(bool format)const{
    QString newLine = "";
    if(format){
        newLine = "\n";
    }
    QString str="["+newLine;
    QStringList projectsList;
    QListIterator<QSharedPointer<RyRuleProject> > it(_projects);
    while(it.hasNext()){
        QSharedPointer<RyRuleProject> p = it.next();
        projectsList<<p->toJson(format,4);
    }
    str += projectsList.join(","+newLine)+newLine;
    str += "]";
    return str;
}
