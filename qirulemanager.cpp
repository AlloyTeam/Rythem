#include "qirulemanager.h"
#include <QVariant>

Q_GLOBAL_STATIC(QiRuleManager, ruleManager)
QiRuleManager *QiRuleManager::instance(){
    return ruleManager();
}


QMap<QiRuleManager::ConfigKey,QVariant> QiRuleManager::getRule(ConnectionData_ptr connectionData,bool *isMatch){
    *isMatch = false;
    for(int i=0,l=configGroups.size();i<l;++i){//TODO 优先级
        QMap<ConfigKey,QVariant> configGroup = configGroups.at(i);
        QList<QVariant> rules = configGroup[ConfigKey_Rules].toList();
        for(int j=0,l2=rules.size();j<l2;++j){
            QVariant ruleVariant = qVariantFromValue(rules.at(j));
            QiRuleConent_type rule = qVariantValue<QiRuleConent_type> (ruleVariant);
            if( isRuleMatch(rule,connectionData) ){
                *isMatch = true;
                qDebug()<<"rule MATCHED..........----------";
                return rule;
            }
        }
    }
    return emptyRule;
}

bool QiRuleManager::isRuleMatch(QMap<ConfigKey,QVariant> rule, ConnectionData_ptr connectionData){
    int type = qVariantValue<int>(rule[ConfigKey_RuleType]);
    //TODO
    QString entry = rule[ConfigKey_RulePattern].toString();
    QString replace = rule[ConfigKey_RuleReplace].toString();
    qDebug()<<type<<entry<<replace;

    QRegExp rx(entry, Qt::CaseInsensitive, QRegExp::Wildcard);
    if( type == RuleType_SimpleAddressReplace){
        qDebug()<<"host="<<connectionData->host;
        if (rx.exactMatch(connectionData->host)){
            return true;
        }
    }else if(type == RuleType_ComplexAddressReplace){
        qDebug()<<"fullUrl="<<connectionData->fullUrl;
        return rx.exactMatch(connectionData->fullUrl);
    }else if(type == RuleType_LocalContentReplace){
        qDebug()<<"fullUrl="<<connectionData->fullUrl;
        return rx.exactMatch(connectionData->fullUrl);
    }else if(type == RuleType_RemoteContentReplace){
        return rx.exactMatch(connectionData->fullUrl);
    }

    return false;
}
