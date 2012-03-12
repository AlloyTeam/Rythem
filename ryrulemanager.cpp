#include "ryrulemanager.h"

#include <QMutexLocker>
#include <QScriptEngine>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QNetworkReply>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QDebug>

//====== RyRule =============
QMap<QString,QString> RyRule::_mimeTypeMap;
quint64 RyRule::_nextRuleId = 0;
QMutex RyRule::mutex;
QString RyRule::getMimeType(const QString &key, const QString &defaultMimeType){
    if(RyRule::_mimeTypeMap.isEmpty()){
        RyRule::_mimeTypeMap.insert("html",	"text/html");
        RyRule::_mimeTypeMap.insert("js",	"text/javascript");
        RyRule::_mimeTypeMap.insert("css",	"text/css");
        RyRule::_mimeTypeMap.insert("txt",	"text/plain");
        RyRule::_mimeTypeMap.insert("jpg",	"image/jpeg");
        RyRule::_mimeTypeMap.insert("jpeg",	"image/jpeg");
        RyRule::_mimeTypeMap.insert("png",	"image/png");
        RyRule::_mimeTypeMap.insert("bmp",	"image/bmp");
        RyRule::_mimeTypeMap.insert("gif",	"image/gif");
        RyRule::_mimeTypeMap.insert("manifest","text/cache-manifest");
    }
    return RyRule::_mimeTypeMap.value(key,defaultMimeType);
}

quint64 RyRule::getNextRuleId(){
    QMutexLocker locker(&mutex);
    _nextRuleId++;
    locker.unlock();
    return _nextRuleId;
}
    /*
                "name":"",
                "type":4,
                "enable":1,
                "rule":{"pattern":"http://w.qq.com/a.manifest",
                "replace":"./test.manifest"}
    */
RyRule::RyRule(quint64 groupId,const QScriptValue& rule){
    init(RyRule::getNextRuleId(),groupId,
            rule.property("type").toInt32(),
         rule.property("rule").property("pattern").toString(),
        rule.property("rule").property("replace").toString(),
         (rule.property("enable").toInt32() == 1));
           // qDebug()<<"RyRule "<<this->toJSON();
}

RyRule::RyRule(quint64 groupId,int type, const QString &pattern, const QString &replace,bool enable){
    init(RyRule::getNextRuleId(),groupId,type,pattern,replace,enable);
}
RyRule::RyRule(quint64 id,quint64 groupId, int type, const QString &pattern, const QString &replace,bool enable){
    init(id,groupId,type,pattern,replace,enable);
}
QString RyRule::toJSON(bool format)const{
    if(format){
        //TODO
    }
    //qDebug()<<_pattern<<_replace<<QString::number(_type);
    QString thePattern = _pattern;
    QString theReplace = _replace;
    thePattern.replace("\\","\\\\");
    thePattern.replace("'","\\'");
    theReplace.replace("\\","\\\\");
    theReplace.replace("'","\\'");
    QString ret="{'id':"+QString::number(_ruleId)+",'type':"+QString::number(_type)+",'rule':{'pattern':'"+thePattern+"', 'replace':'"+theReplace+"'} }";
    return ret;
}
int RyRule::type(){
    return _type;
}

QString RyRule::pattern(){
    return _pattern;
}

QString RyRule::replace(){
    return _replace;
}

// ==========end of RyRule =====================


// ==========RyRuleReplaceContent ==============


RyRuleReplaceContent::RyRuleReplaceContent(QSharedPointer<RyRule> rule, const QString &url){
    setRule(rule);
    setUrl(url);
}
QPair<QByteArray,QByteArray> RyRuleReplaceContent::getReplaceContent(const QString& url){
    setUrl(url);
    return getReplaceContent();
}

QPair<QByteArray,QByteArray> RyRuleReplaceContent::getReplaceContent(){
    QPair<QByteArray,QByteArray> ret;
    //open local file for read
    QString status;
    QByteArray body;
    QByteArray header;
    QString replace = _rule->replace();
    QString pattern = _rule->pattern();
    QFileInfo fileInfo(replace);
    QFile file;
    QString mimeTypeKey;
    QString mimeType = "text/plain";
    int contentLength;
    QString encode = "utf-8";
    QScriptEngine engine;
    QString mergeFileContent;
    QMap<QString,QVariant> mergeValueMap;
    bool mergeContentHasError=false;

    bool fileCanOpen;

    QString fileName;

    switch(_rule->type()){
    case RyRule::LOCAL_FILE_REPLACE:
        file.setFileName(replace);
        if(file.open(QFile::ReadOnly)){
            mimeTypeKey = fileInfo.suffix().toLower();
            mimeType = RyRule::getMimeType(mimeTypeKey);
            status = "200 OK";
            body = file.readAll();
        }else{
            status = "404 Not Found";
            body.append(QString("file %1 not found").arg(replace));
        }
        file.close();
        contentLength = body.size();

        //create response
        header.append(QString("HTTP/1.1 %1 \r\n"
                         "Server: Rythem \r\n"
                         "Content-Type: %2 \r\n"
                         "Content-Length: %3 \r\n\r\n")
                .arg(status)
                .arg(mimeType)
                .arg(contentLength));
        //qDebug()<<" header  = "<<header;
        break;
    case RyRule::LOCAL_FILES_REPLACE:
        if(file.open(QFile::ReadOnly)){
            mimeTypeKey = fileInfo.suffix().toLower();
            mimeType = RyRule::getMimeType(mimeTypeKey,"application/javascript");
            mergeFileContent = file.readAll();
            mergeValueMap = engine.evaluate(mergeFileContent.prepend("(").append(")")).toVariant().toMap();
            if(mergeValueMap.contains("encode")){
                encode = mergeValueMap["encode"].toString();
            }
            //qDebug()<<mergeValueMap;
            //qDebug()<<mergeFileContent;
            mergeValueMap = mergeValueMap["projects"].toList().first().toMap();
            if(engine.hasUncaughtException() || mergeValueMap.isEmpty()){//wrong content
                qDebug() << "wrong qzmin format:" << replace << mergeFileContent;
                mergeContentHasError = true;
            }
        }else{
            qDebug()<<"file cannot open ";
            mergeContentHasError = true;
        }
        file.close();
        if(mergeContentHasError){
            status = "404 NOT FOUND";
            body.append(QString("merge file with wrong format:").append(replace).append(mergeFileContent));
        }else{
            status = "200 OK";
            foreach(QVariant item,mergeValueMap["include"].toList()){
                //qDebug()<<item.toString();
                file.setFileName(item.toString());
                fileCanOpen = file.open(QFile::ReadOnly);
                if(fileCanOpen){
                    body.append(file.readAll());
                }else{
                    body.append(QString("/*file:【%1】 not found*/").arg(item.toString()));
                }
                file.close();
            }
        }
        contentLength = body.size();
        header.append(QString("HTTP/1.1 %1 \r\nServer: Rythem \r\nContent-Type: %2 charset=%3 \r\nContent-Length: %4 \r\n\r\n")
                           .arg(status)
                           .arg(mimeType) // TODO
                           .arg(encode)
                           .arg(contentLength));
        break;
    case RyRule::LOCAL_DIR_REPLACE:
        fileName = _url.mid(_url.indexOf(pattern)+pattern.length());
        //qDebug()<<fileName;
        if(fileName.indexOf("?")!=-1){
            fileName = fileName.left(fileName.indexOf("?"));
        }
        if(fileName.indexOf("#")!=-1){
            fileName = fileName.left(fileName.indexOf("#"));
        }
    #ifdef Q_OS_WIN
        if(replace.indexOf("\\")==replace.length()-1){
            replace.remove(replace.length()-1,1);
        }
    #else
        if(replace.indexOf("/")==replace.length()-1){
            replace.remove(replace.length()-1,1);
        }
    #endif
        if(fileName=="/" || fileName.isEmpty()){
            fileName = "/index.html";
        }
        fileName.prepend(replace);
        //qDebug()<<fileName;
        file.setFileName(fileName);
        if(file.open(QFile::ReadOnly | QIODevice::Text)){
            status = "200 OK";
            body = file.readAll();
        }else{
            status = "404 Not Found";
            body.append(QString("file:%1 not found").arg(fileName));
        }
        mimeType = RyRule::getMimeType(QFileInfo(file).suffix().toLower(),"text/plain");
        file.close();
        contentLength = body.size();
        header.append(QString("HTTP/1.1 %1 \r\nServer: Rythem \r\nContent-Type: %2 \r\nContent-Length: %3 \r\n\r\n")
                           .arg(status)
                           .arg(mimeType)
                           .arg(contentLength));
        break;
    default:// error.
        qWarning()<<"this type of rule has no content to replace";
        break;
    }
    ret.first = header;
    ret.second = body;
    return ret;
}

void RyRuleReplaceContent::setUrl(const QString& url){
    _url = url;
}

void RyRuleReplaceContent::setRule(QSharedPointer<RyRule> rule){
    _rule = rule;
}

// ==========end of RyRuleReplaceContent =======


// ==========RyRuleGroup ====================
quint64 RyRuleGroup::_nextGroupId = 0;
QMutex RyRuleGroup::mutex;
quint64 RyRuleGroup::getNextGroupId(){
    QMutexLocker locker(&mutex);
    _nextGroupId++;
    locker.unlock();
    return _nextGroupId;
}
RyRuleGroup::RyRuleGroup(const QScriptValue &group){
    // name enable rules
    _groupId = RyRuleGroup::getNextGroupId();
    QString name = group.property("name").toString();
    bool enable = group.property("enable").toBoolean();
    QScriptValue rules = group.property("rules");
    if(! rules.isValid()){
    }else{
        _groupName = name;
        enabled = enable;
        addRules(rules);
    }
}
/*
RyRuleGroup::RyRuleGroup(const QString &groupName, bool isOwner){
    RyRuleGroup(RyRuleGroup::getNextGroupId(),groupName,isOwner);
}
RyRuleGroup::RyRuleGroup(quint64 groupId, const QString &groupName, bool isOwner){
    _groupId = groupId;
    _groupName = groupName;
    _isOwner = isOwner;
}
*/
void RyRuleGroup::addRules(const QString& rules){

}
void RyRuleGroup::addRules(const QScriptValue& rules){
    QScriptValueIterator it(rules);
    while(it.hasNext()){
        it.next();
        if(it.flags() & QScriptValue::SkipInEnumeration){
            continue;
        }
        QScriptValue v = it.value();
        RyRule *rule = new RyRule(_groupId,v);
        QSharedPointer<RyRule> p(rule);
        _rules.append(p);
    }
}

void RyRuleGroup::addRule(QSharedPointer<RyRule> rule){
    _rules.append(rule);
}

void RyRuleGroup::addRle(int type,QString pattern,QString replace){
    QSharedPointer<RyRule> tmp(new RyRule(_groupId,type,pattern,replace));
    addRule(tmp);
}

void RyRuleGroup::addRle(quint64 ruleId,int type,QString pattern,QString replace){
    QSharedPointer<RyRule> tmp(new RyRule(ruleId,_groupId,type,pattern,replace));
    addRule(tmp);
}
QList<QSharedPointer<RyRule> > RyRuleGroup::getMatchRules(const QString& url){
    //qDebug()<<"getMatchRule:"<<url<<_rules.length();
    QList<QSharedPointer<RyRule> > ret;
    QListIterator<QSharedPointer<RyRule> > it(_rules);
    while(it.hasNext()){
        QSharedPointer<RyRule> rule = it.next();
        int type = rule->type();
        QString pattern = rule->pattern();

        bool isMatch = false;
        if(type == RyRule::COMPLEX_ADDRESS_REPLACE){
            isMatch = false;//TODO
        }else if(type == RyRule::REMOTE_CONTENT_REPLACE){
            isMatch = (url.indexOf(pattern)!=-1);
        }else{
            QRegExp rx(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
            isMatch = rx.exactMatch(url);
        }
        if(isMatch){
            ret.append(rule);
        }
    }
    return ret;
}
QString RyRuleGroup::toJSON(bool format)const{
    if(format){
        //TODO
    }
    QStringList rulesStrList;
    for(int i=0;i<_rules.length();++i){
       QSharedPointer<RyRule> rule = _rules.at(i);
       rulesStrList<<rule->toJSON();
    }
    QString nameEscaped = _groupName;
    nameEscaped = nameEscaped.replace("\\","\\\\'");
    nameEscaped = nameEscaped.replace("'","\\'");
    QString ret="{'id':"+QString::number(_groupId)+",'name':'"+nameEscaped+
            "',"+"'enable':"+QString::number(enabled?1:0)+
            ",'rules':[" +rulesStrList.join(",")+"]}";
    return ret;
}

// ==========end of RyRuleGroup ====================

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

    //save all projects
    QListIterator<QSharedPointer<RyRuleProject> > rpIt(_projects);
    while(rpIt.hasNext()){
        QSharedPointer<RyRuleProject> rp = rpIt.next();
        rp.clear();
    }
}

void RyRuleManager::loadLocalConfig(const QString& configFileName){
    QFile file(configFileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"opened file "<<configFileName;
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
void RyRuleManager::addRuleProject(const QScriptValue &project){
    RyRuleProject *rp = new RyRuleProject(project);
    bool isProjectValid = rp->isValid();
    if(!isProjectValid){
        qDebug()<<"invalid project";
    }else{
        _projects.append(QSharedPointer<RyRuleProject>(rp));
    }
}

void RyRuleManager::addRuleProject(const QString& groupJSONData){
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("(" + groupJSONData + ")");
    QScriptValueIterator groupsIt(value.property("groups"));
}
void RyRuleManager::addRuleProject(const QString &groupJSONData, const QString &address, bool isRemote, const QString &host){

}

void RyRuleManager::addRemoteProject(const QString& url){
    /*
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    connect(&manager,SIGNAL(finished(QNetworkReply*)),&loop,SLOT(quit()));
    loop.exec();
    QString content = reply->readAll();
    addRuleGroups(content,url,true);
    */
}

void RyRuleManager::addRemoteProjectFromLocal(const QString &localAddress,
                                              const QString &remoteAddress,
                                              const QString &pwd,
                                              const QString &owner){


}

void RyRuleManager::addLocalProject(const QString& filePath){
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
}

void RyRuleManager::updateRule(QSharedPointer<RyRule> rule){
    qDebug()<<"updateRule";//TODO
}

void RyRuleManager::updateRuleGroup(QSharedPointer<RyRuleGroup> ruleGroup){
    qDebug()<<"updateRuleGroup";//TODO
}

//由于可能同时命中host及远程替换两种rule，此处需返回List
QList<QSharedPointer<RyRule> > RyRuleManager::getMatchRules(const QString& url){
    QList<QSharedPointer<RyRule> > ret;
    QListIterator<QSharedPointer<RyRuleProject> > it(_projects);
    while(it.hasNext()){
        QSharedPointer<RyRuleProject> p = it.next();
        ret.append(p->getMatchRules(url));

    }
    return ret;
}

//返回header body
QPair<QByteArray,QByteArray> RyRuleManager::getReplaceContent(QSharedPointer<RyRule> rule,const QString& url){
    RyRuleReplaceContent rc(rule,url);
    return rc.getReplaceContent();
}

QString RyRuleManager::toJson()const{
    QString str="[";
    QStringList projectsList;
    QListIterator<QSharedPointer<RyRuleProject> > it(_projects);
    while(it.hasNext()){
        QSharedPointer<RyRuleProject> p = it.next();
        projectsList<<p->toJson();
    }
    str += projectsList.join(",");
    str += "]";
    return str;
}
