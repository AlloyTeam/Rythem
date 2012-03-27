#include "ryrulegroup.h"

using namespace rule;

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
    Q_UNUSED(rules)
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
        if(!rule->enabled){
            qDebug()<<"rule not enabled "<<rule->toJSON(true,0);
            continue;
        }
        //qDebug()<<"getMatchRule rule:"<<url<<rule->toJSON(true,0);
        //qDebug()<<url;
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
                //qDebug()<<"pattern="<<pattern;
                QRegExp rx(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
                isMatch = rx.exactMatch(url);
            }else{
                isMatch = (pattern == url);
            }
        }
        if(isMatch){
            //qDebug()<<"match\n"<<rule->toJSON(true,0);
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
