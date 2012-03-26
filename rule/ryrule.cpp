#include "ryrule.h"

using namespace rule;

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
