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
QString RyRule::toJSON(bool format,int beforeSpace)const{
    QString space=" ";
    QString newLine = "\n";
    QString currentSpace = "";
    int currentSpaceLength = 0;
    int increasement = 0;
    if(format){
        currentSpace = space.repeated(beforeSpace);
        currentSpaceLength = beforeSpace;
        increasement = 4;
    }else{
        newLine = "";
        space = "";
    }
    //qDebug()<<_pattern<<_replace<<QString::number(_type);
    QStringList retList;
    QString thePattern = _pattern;
    QString theReplace = _replace;
    thePattern.replace("\\","\\\\");
    thePattern.replace("'","\\'");
    theReplace.replace("\\","\\\\");
    theReplace.replace("'","\\'");
    QString ret;
    retList << currentSpace+"{"+newLine;
    currentSpaceLength += increasement;
    currentSpace = space.repeated(currentSpaceLength);
    retList << currentSpace +  "'enable':" + QString::number(enabled?1:0) + ","+newLine;
    retList << currentSpace +  "'id':" + QString::number(_ruleId) + ","+newLine;
    retList << currentSpace +  "'type':" + QString::number(_type) + ","+newLine;
    retList << currentSpace +  "'rule':{" +newLine;
    currentSpaceLength += increasement;
    currentSpace = space.repeated(currentSpaceLength);
    retList << currentSpace +      "'pattern':'"+thePattern+"',"+newLine;
    retList << currentSpace +      "'replace':'"+theReplace+"',"+newLine;
    currentSpaceLength -= increasement;
    currentSpace = space.repeated(currentSpaceLength);
    retList << currentSpace +  "}" +newLine;
    currentSpaceLength -= increasement;
    currentSpace = space.repeated(currentSpaceLength);
    retList << currentSpace +  "}";
    ret = retList.join("");
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
    _loop = new QEventLoop();
}
RyRuleReplaceContent::~RyRuleReplaceContent(){
    if(_loop->isRunning()){
        _loop->quit();
    }
    delete _loop;
    _loop = NULL;

}

QPair<QByteArray,QByteArray> RyRuleReplaceContent::getReplaceContent(const QString& url){
    setUrl(url);
    return getReplaceContent();
}

QPair<QByteArray,QByteArray> RyRuleReplaceContent::getReplaceContent(){
    switch(_rule->type()){
    case RyRule::LOCAL_FILE_REPLACE:
        return getLocalReplaceContent();
        break;
    case RyRule::LOCAL_FILES_REPLACE:
        return getLocalMergeReplaceContent();
        break;
    case RyRule::LOCAL_DIR_REPLACE:
        return getLocalDirReplaceContent();
        break;
    case RyRule::REMOTE_CONTENT_REPLACE:
        return getRemoteReplaceContent();
        break;
    default:// error.
        qWarning()<<"this type of rule has no content to replace";
        return QPair<QByteArray,QByteArray>(QByteArray("HTTP/1.1 404 NOT FOUND \r\nServer: Rythem\r\nContent-Length: 55\r\n\r\n"),
                                            QByteArray("error rythem connot handle this type of replacement now"));
        break;
    }
}

QPair<QByteArray,QByteArray> RyRuleReplaceContent::getRemoteReplaceContent(){
    QPair<QByteArray,QByteArray> ret;
    QByteArray header,body;

    //QTimer timer;
    QNetworkAccessManager manager;
    //timer.singleShot(20000,_loop,SLOT(quit()));
    manager.connect(&manager,SIGNAL(finished(QNetworkReply*)),_loop,SLOT(quit()));
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(_rule->replace())));
    _loop->exec();

    bool isRequestDone = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).isValid();
    if(!isRequestDone){
        body.append("<center>remote address unresponsable</center>");
        header.append(QString("HTTP/1.1 503 Serveice Unavailable\r\n"
                         "Server: Rythem \r\n"
                         "Content-Type: text/html \r\n"
                         "Content-Length: %4 \r\n\r\n")
                .arg(body.length()));
    }else{
        int responseCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString status = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();


        header.append(QByteArray("HTTP/1.1 ").append(QString::number(responseCode)).append(" ").append(status).append("\r\n"));

        QList<QNetworkReply::RawHeaderPair> tmp2 = reply->rawHeaderPairs();
        for(int i=0;i<tmp2.length();++i){
            QNetworkReply::RawHeaderPair tmp3 = tmp2.at(i);
            if(tmp3.first.toLower() == "content-encoding" ||
                    tmp3.first.toLower() == "transfer-encoding"){
                continue;
            }
            header.append(tmp3.first.append(": ").append(tmp3.second).append("\r\n"));
            //qDebug()<<QString(tmp3.first).append(": ").append(tmp3.second);
        }
        header.append("\r\n");
        body = reply->readAll();
    }
    ret.first = header;
    ret.second = body;
    return ret;
}

QPair<QByteArray,QByteArray> RyRuleReplaceContent::getLocalReplaceContent(){
    QPair<QByteArray,QByteArray> ret;
    //open local file for read
    QString status;
    QByteArray body;
    QByteArray header;
    QString replace = _rule->replace();
    QFileInfo fileInfo(replace);
    QFile file;
    QString mimeTypeKey;
    QString mimeType = "text/plain";
    int contentLength;
    QString encode = "utf-8";
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
                     "Content-Type: %2 charset=%3\r\n"
                     "Content-Length: %4 \r\n\r\n")
            .arg(status)
            .arg(mimeType)
            .arg(encode)
            .arg(contentLength));

    ret.first = header;
    ret.second = body;
    return ret;
}

QPair<QByteArray,QByteArray> RyRuleReplaceContent::getLocalMergeReplaceContent(){
    QPair<QByteArray,QByteArray> ret;
    //open local file for read
    QString status;
    QByteArray body;
    QByteArray header;
    QString replace = _rule->replace();
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
    file.setFileName(replace);
    if(file.open(QFile::ReadOnly)){
        mimeTypeKey = fileInfo.suffix().toLower();
        mimeType = RyRule::getMimeType(mimeTypeKey,"application/javascript");
        mergeFileContent = file.readAll();
        mergeValueMap = engine.evaluate(mergeFileContent.prepend("(").append(")")).toVariant().toMap();
        if(engine.hasUncaughtException() || mergeValueMap.isEmpty()){//wrong content
            qDebug() << "wrong qzmin format:" << replace << mergeFileContent;
            mergeContentHasError = true;
        }else{
            if(mergeValueMap.contains("encode")){
                encode = mergeValueMap["encode"].toString();
            }
            //qDebug()<<mergeValueMap;
            //qDebug()<<mergeFileContent;
            if(mergeValueMap["projects"].toList().isEmpty() ){
                mergeContentHasError = true;
            }else{
                mergeValueMap = mergeValueMap["projects"].toList().first().toMap();
                if(mergeValueMap["include"].toList().isEmpty()){
                    mergeContentHasError = true;
                }
            }
        }
    }else{
        qDebug()<<"file cannot open ";
        mergeContentHasError = true;
    }
    file.close();
    if(mergeContentHasError){
        mimeType = "text/plain";
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

    ret.first = header;
    ret.second = body;
    return ret;
}

QPair<QByteArray,QByteArray> RyRuleReplaceContent::getLocalDirReplaceContent(){
    QPair<QByteArray,QByteArray> ret;
    //open local file for read
    QString status;
    QByteArray body;
    QByteArray header;
    QString replace = _rule->replace();
    QString pattern = _rule->pattern();
    QFile file;
    QString mimeType = "text/plain";
    int contentLength;
    QString encode = "utf-8";


    QString fileName;
    fileName = _url.mid(_url.indexOf(pattern)+pattern.length());
    //qDebug()<<fileName;
    if(fileName.indexOf("?")!=-1){
        fileName = fileName.left(fileName.indexOf("?"));
    }
    if(fileName.indexOf("#")!=-1){
        fileName = fileName.left(fileName.indexOf("#"));
    }
#ifdef Q_OS_WIN
    if(replace.endsWith("\\")){
        replace.remove(replace.length()-1,1);
    }
#else
    if(replace.endsWith("/")){
        replace.remove(replace.length()-1,1);
    }
#endif
    if(fileName=="/" || fileName.isEmpty()){
        fileName = "/index.html";
    }
    if(!fileName.startsWith("/")){
        fileName.prepend("/");
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
    header.append(QString("HTTP/1.1 %1 \r\nServer: Rythem \r\nContent-Type: %2 charset=%3 \r\nContent-Length: %4 \r\n\r\n")
                       .arg(status)
                       .arg(mimeType)
                       .arg(encode)
                       .arg(contentLength));
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

quint64 RyRuleGroup::groupId()const{
    return _groupId;
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
        addRule(v);
    }
}
QSharedPointer<RyRule> RyRuleGroup::addRule(const QScriptValue &value){
    RyRule *rule = new RyRule(_groupId,value);
    QSharedPointer<RyRule> p(rule);
    return addRule(p);
}

QSharedPointer<RyRule> RyRuleGroup::addRule(QSharedPointer<RyRule> rule){
    _rules.append(rule);
    return rule;
}

QSharedPointer<RyRule> RyRuleGroup::addRule(int type,QString pattern,QString replace){
    QSharedPointer<RyRule> tmp(new RyRule(_groupId,type,pattern,replace));
    return addRule(tmp);
}

QSharedPointer<RyRule> RyRuleGroup::addRule(quint64 ruleId,int type,QString pattern,QString replace){
    QSharedPointer<RyRule> tmp(new RyRule(ruleId,_groupId,type,pattern,replace));
    return addRule(tmp);
}

QSharedPointer<RyRule> RyRuleGroup::updateRule(const QString& ruleJson){
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("("+ruleJson+")");
    quint64 ruleId = value.property("id").toInt32();
    QSharedPointer<RyRule> ret;
    QListIterator<QSharedPointer<RyRule> > it(_rules);
    while(it.hasNext()){
        QSharedPointer<RyRule> rule = it.next();
        if(rule->ruleId() == ruleId){
            qDebug()<<"found Rule";
            ret = rule;
            break;
        }
    }
    if(!ret.isNull()){
        ret->update(value);
    }
    return ret;
}
void RyRuleGroup::update(const QString &groupJson){
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("("+groupJson+")");
    QString name = value.property("name").toString();
    bool enable = value.property("enable").toBool();
    qDebug()<<"update group:"<<name;
    this->_groupName = name;
    this->enabled = enable;
}

void RyRuleGroup::removeRule(quint64 ruleId){
    int i=0,l=0;
    for(l=_rules.length();i<l;++i){
        QSharedPointer<RyRule> r = _rules.at(i);
        if(r->ruleId() == ruleId){
            _rules.takeAt(i);
            break;
        }
    }
}

QList<QSharedPointer<RyRule> > RyRuleGroup::getMatchRules(const QString& url){
    //qDebug()<<"getMatchRule:"<<url<<_rules.length();
    QList<QSharedPointer<RyRule> > ret;
    if(!this->enabled){
        return ret;
    }
    QListIterator<QSharedPointer<RyRule> > it(_rules);
    while(it.hasNext()){
        QSharedPointer<RyRule> rule = it.next();
        //qDebug()<<"getMatchRule rule:"<<url<<rule->toJSON();
        if(!rule->enabled){
            continue;
        }
        int type = rule->type();
        QString pattern = rule->pattern();
        bool isRegExp = rule->pattern().toLower().startsWith("regex:");

        bool isMatch = false;
        if(type == RyRule::COMPLEX_ADDRESS_REPLACE){
            isMatch = false;//TODO
        }else if(type == RyRule::REMOTE_CONTENT_REPLACE){
            isMatch = (url.indexOf(pattern)!=-1);
        }else if(type == RyRule::SIMPLE_ADDRESS_REPLACE){
            int n = url.indexOf(pattern);
            isMatch = false;
            if(n!=-1){
                int protocolLength = 0;
                if(url.startsWith("http://")){
                    protocolLength = 7;
                }else if(url.startsWith("https://")){
                    protocolLength = 8;
                }

                if(n == protocolLength){
                    if(url.length() == protocolLength+pattern.length() ||
                            url.at(protocolLength+pattern.length()) == '/' ||
                            url.at(protocolLength+pattern.length()) == ':'){
                        isMatch = true;
                    }
                }
            }
        }else if(type == RyRule::LOCAL_DIR_REPLACE){
            isMatch = (url.indexOf(pattern) != -1);
        }else{
            if(isRegExp){
                pattern  = pattern.mid(QString("regex:").length());
                QRegExp rx(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
                isMatch = rx.exactMatch(url);
            }else{
                isMatch = (pattern == url);
            }
        }
        if(isMatch){
            ret.append(rule);
        }
    }
    return ret;
}
QString RyRuleGroup::toJSON(bool format,int beforeSpace)const{
    QString space=" ";
    QString newLine = "\n";
    QString currentSpace = "";
    int currentSpaceLength = 0;
    int increasement = 0;
    if(format){
        currentSpace = space.repeated(beforeSpace);
        currentSpaceLength = beforeSpace;
        increasement = 4;
    }else{
        newLine = "";
        space = "";
    }
    QString nameEscaped = _groupName;
    nameEscaped = nameEscaped.replace("\\","\\\\'");
    nameEscaped = nameEscaped.replace("'","\\'");
    QStringList retList;
    QString ret;
    retList<< currentSpace+"{"+newLine;
    currentSpaceLength += increasement;
    currentSpace = space.repeated(currentSpaceLength);
    retList << ( currentSpace+"'id':"+QString::number(_groupId)+","+newLine
                +currentSpace+"'name':'"+nameEscaped+"',"+newLine
                +currentSpace+"'enable':"+QString::number(enabled?1:0)+","+newLine
                +currentSpace+"'rules':[" +newLine);

    currentSpaceLength += increasement;
    currentSpace=space.repeated(currentSpaceLength);

    QStringList rulesStrList;
    for(int i=0;i<_rules.length();++i){
       QSharedPointer<RyRule> rule = _rules.at(i);
       rulesStrList<<rule->toJSON(format,currentSpaceLength);
    }
    retList <<  rulesStrList.join(","+newLine) +newLine;

    currentSpaceLength -= increasement;
    currentSpace=space.repeated(currentSpaceLength);
    retList <<  currentSpace+ "]"+newLine;
    currentSpaceLength -= increasement;
    currentSpace=space.repeated(currentSpaceLength);
    retList << currentSpace+"}/*END OF GROUP--*/";
    ret = retList.join("");
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
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
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
const QSharedPointer<RyRuleProject> RyRuleManager::addRuleProject(const QString &groupJSONData, const QString &address, bool isRemote, const QString &host){
    return QSharedPointer<RyRuleProject>();
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

const QSharedPointer<RyRuleProject> RyRuleManager::addRemoteProjectFromLocal(const QString &localAddress,
                                              const QString &remoteAddress,
                                              const QString &pwd,
                                              const QString &owner){
    return QSharedPointer<RyRuleProject>();

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
    qDebug()<<"match rule length = "<<QString::number(ret.length());
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
