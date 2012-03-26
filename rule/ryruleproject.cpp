#include "ryruleproject.h"

using namespace rule;

RyRuleProject::RyRuleProject(const QScriptValue& project){
    QString localAddress = project.property("localAddress").toString();
    QString remoteAddress = project.property("remoteAddress").toString();
    QString owner = project.property("owner").toString();
    QString pwd = project.property("pwd").toString();
    init(localAddress,remoteAddress,pwd,owner);
}

RyRuleProject::RyRuleProject(QString localAddress,
              QString remoteAddress,
              QString pwd,
              QString owner){
    init(localAddress,remoteAddress,pwd,owner);
}
RyRuleProject::~RyRuleProject(){
    //TODO save
    if(_loop->isRunning()){
        _loop->quit();
    }
    delete _loop;
    _loop = NULL;
    qDebug()<<"~RyRuleProject";
    saveToFile();
    _groups.clear();
}
void RyRuleProject::saveToFile(){
    QFile f(_localAddress);
    if(f.open(QIODevice::WriteOnly | QIODevice::Text)){
        QString str="{\n"
                    "    'groups':[\n";
        QStringList groupStrList;
        for(int i=0;i<_groups.length();++i){
           QSharedPointer<RyRuleGroup> g = _groups.at(i);
           groupStrList<<g->toJSON(true,8);
        }

        str+=groupStrList.join(",\n")+
                     "\n    ]\n}";
        f.write( QByteArray().append(str.toUtf8()));
        qDebug()<<"save to file";
        f.close();
    }else{
        qDebug()<<"cache remote rule failure"<<_localAddress;
    }
}

void RyRuleProject::init(QString localAddress,
          QString remoteAddress,
          QString pwd,
          QString owner){
    _loop = new QEventLoop();
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
bool RyRuleProject::RyRuleProject::addRemoteRuleGroups(){
    //qDebug()<<"getting remote rule"<<_remoteAddress;
    //QTimer timeout;
    QNetworkAccessManager manager;
    QNetworkReply* reply;
    //timeout.singleShot(3000,_loop,SLOT(quit()));//3秒内不返回内容就判断为失败
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    manager.connect(&manager,SIGNAL(finished(QNetworkReply*)),_loop,SLOT(quit()));
    reply = manager.get(QNetworkRequest(QUrl(_remoteAddress)));
    _loop->exec();
    QTextCodec* oldCodec = QTextCodec::codecForCStrings();
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QString content = reply->readAll();
     QTextCodec::setCodecForCStrings(oldCodec);
    _jsonCache = content;
    qDebug()<<"remote content:"<<content;
    return addRuleGroups(content);
}
bool RyRuleProject::addLocalRuleGroups(){
    QFile f(_localAddress);
    if(f.open(QIODevice::ReadOnly | QIODevice::Text)){
        QByteArray content = f.readAll();
        return addRuleGroups(QString::fromUtf8(content.data()));
    }else{
        qDebug()<<"cannot open file:"<<_localAddress;
        return false;
    }
}
void RyRuleProject::removeRuleGroup(quint64 groupId){
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

bool RyRuleProject::addRuleGroups(const QString& content){
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
            QSharedPointer<RyRuleGroup> g2 = addRuleGroup(g);
            if(g2.isNull()){
                qDebug()<<"getGroup Failed";
            }else{
                qDebug()<<"got group"<<g2->toJSON();
            }
        }
        qDebug()<<"got project!";
        return true;
    }
}
QSharedPointer<RyRuleGroup> RyRuleProject::addRuleGroup(const QScriptValue& group,bool updateLocalFile){
    Q_UNUSED(updateLocalFile)
    RyRuleGroup *g = new RyRuleGroup(group);
    QSharedPointer<RyRuleGroup> groupPtr(g);
    //qDebug()<<"group added:"<<g->toJSON();
    _groups.append(groupPtr);
    return groupPtr;
}

QString RyRuleProject::localAddress()const{
    return _localAddress;
}
QString RyRuleProject::RyRuleProject::toJson(bool format,int beforeSpace){
    QString space = "";
    QString currentSpace="";
    int increasement = 0;
    int currentSpaceLength = 0;
    QString newLine="";
    if(format){
        //TODO
        newLine = "\n";
        space = " ";
        currentSpaceLength = beforeSpace;
        increasement = 4;
        currentSpace = space.repeated(currentSpaceLength);
    }
    QString str=currentSpace +"{" + newLine;
    currentSpaceLength += increasement;//+
    currentSpace = space.repeated(currentSpaceLength);
    str += currentSpace+"'groups':["+newLine;
    currentSpaceLength += increasement;//+
    currentSpace += space.repeated(currentSpaceLength);
    QStringList groupStrList;
    for(int i=0;i<_groups.length();++i){
       QSharedPointer<RyRuleGroup> g = _groups.at(i);
       groupStrList<<g->toJSON(format,currentSpaceLength);
    }
    str+=groupStrList.join(QString(",")+newLine)+newLine;
    currentSpaceLength -= increasement;//-
    currentSpace = space.repeated(currentSpaceLength);
    str+=currentSpace+"]/*END GROUPS*/"+newLine;
    currentSpaceLength -= increasement;//-
    currentSpace = space.repeated(currentSpaceLength);
    str+=currentSpace+"}/*End Project*/";
    return str;
}

QString RyRuleProject::toConfigJson(bool format){
    _needReCombineJson = true;//TODO
    if(!_needReCombineJson){
        return _jsonCache;
    }else{
        QString newLine = "";
        QString currentSpace = "";
        int currentSpaceLenth = 0;
        QString space = "";
        if(format){
            //TODO
            space = " ";
            newLine = "\n";
            currentSpaceLenth = 4;
            currentSpace = space.repeated(currentSpaceLenth);
        }
        QString localAddressEscaped = _localAddress;
        localAddressEscaped.replace("\\","\\\\");
        localAddressEscaped.replace("'","\\'");
        QString ret = currentSpace+"{"+newLine+
                currentSpace+currentSpace+"'localAddress':'"+localAddressEscaped+"'";
        if(!_remoteAddress.isEmpty()){
            QString remoteAddressEscaped = _remoteAddress;
            remoteAddressEscaped.replace("\\","\\\\");
            remoteAddressEscaped.replace("'","\\'");
            ret+=","+newLine+currentSpace+currentSpace+"'remoteAddress':'"+remoteAddressEscaped+"'";
            ret+=","+newLine+currentSpace+currentSpace+"'pwd':'"+_pwd+"'";
            QString ownerEscaped = _owner;
            ownerEscaped.replace("\\","\\\\");
            ownerEscaped.replace("'","\\'");
            ret+=","+newLine+currentSpace+currentSpace+"'owner':'"+ownerEscaped+"'";
        }
        ret+=newLine+currentSpace+"}";
        _jsonCache = ret;
        _needReCombineJson = false;
    }
    return _jsonCache;
}
bool RyRuleProject::isValid()const{
    return _isValid;
}
QList<QSharedPointer<RyRule> > RyRuleProject::getMatchRules(const QString& url){
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
const QList<QSharedPointer<RyRuleGroup> > RyRuleProject::groups()const{
    return _groups;
}
QSharedPointer<RyRuleGroup> RyRuleProject::groupById(quint64 groupId)const{
    QListIterator<QSharedPointer<RyRuleGroup> > it(_groups);
    while(it.hasNext()){
        QSharedPointer<RyRuleGroup> g = it.next();
        if(g->groupId() == groupId){
            return g;
        }
    }
    return QSharedPointer<RyRuleGroup>();
}
