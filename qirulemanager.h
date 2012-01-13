#ifndef QIRULEMANAGER_H
#define QIRULEMANAGER_H

#include <QObject>
#include <QtCore>
#include <QMap>
#include <qiconnectiondata.h>



class QiRuleManager : public QObject
{
    Q_OBJECT
public:
        enum ConfigKey{
            //配置组信息，需用户提供
            ConfigKey_Author,
            ConfigKey_RemoteHost,
            ConfigKey_RemoteAddress,
            ConfigKey_RemotePath,
            ConfigKey_LocalConfigFile,

            //配置数组，由以上内容决定
            ConfigKey_Rules,

            //具体每条配置信息
            ConfigKey_ContentType,
            ConfigKey_Encoding,
            ConfigKey_RuleType,
            ConfigKey_RulePattern,
            ConfigKey_RuleReplace
        };
        enum RuleType{
            RuleType_SimpleAddressReplace,   // 替换远程ip地址 一个域名对应一个host
            RuleType_ComplexAddressReplace,  // 替换远程ip地址一个域名对应多个host
            RuleType_LocalContentReplace,    // 替换本地内容
            RuleType_RemoteContentReplace,   // 替换远程内容   （本地预读取还是远程获取？本地预读需处理“更新”逻辑)
            // TODO
            RuleType_PathReplace,            // 路径重定向(仅保留域名) 如 http://qplus.com/cgi_developing_or_has_error 重定向至 http://qplus.com/cgi_ok_but_will_discard
            RuleType_DomainReplace           // 域名重定向(仅替换域名) 如 http://domain.NOT.deploy.com/cgi 重定向至 http://domain.deploy.com/
        };
        enum ContentReplaceRuleType{
            ContentReplaceRuleType_SingleReplace,          //单个文件替换
            ContentReplaceRuleType_MutiReplace             //多文件合并替换
        };
    //typedef QVector< QMap<QiRuleManager::ConfigKey,QVariant> > QiRulesVector_type;
    //typedef QList< QMap<ConfigKey,QVariant> > QiRulesList_type;
    typedef QMap<QiRuleManager::ConfigKey,QVariant> QiRuleConent_type;
    inline QiRuleManager(QObject *parent = 0):QObject(parent){

        // data for test only
        QMap<ConfigKey,QVariant> configGroup;
        configGroup[ConfigKey_Author] = "ippan";
        configGroup[ConfigKey_RemoteHost] = "ippan.web.qq.com";
        configGroup[ConfigKey_RemoteAddress] = "183.60.11.120.";
        configGroup[ConfigKey_RemotePath] = "ip_config.json";

        QList<QVariant> theRules;
        //QList< QMap<ConfigKey,QVariant> > theRules;
        QiRuleConent_type rule1;
        rule1[ConfigKey_RuleType] = RuleType_RemoteContentReplace;
        rule1[ConfigKey_RulePattern] = QString("http://www.qq.com/");
        rule1[ConfigKey_RuleReplace] = QString("http://ippan.web.qq.com/ip_config.json");
        QiRuleConent_type rule2;
        rule2[ConfigKey_RuleType] = RuleType_SimpleAddressReplace;
        rule2[ConfigKey_RulePattern] = QString("ippan.web.qq.com");
        rule2[ConfigKey_RuleReplace] = QString("183.60.11.120");
        theRules.append(qVariantFromValue(rule1));
        theRules.append(qVariantFromValue(rule2));

        //theRules.append(rule2);

        configGroup[ConfigKey_Rules] = qVariantFromValue(theRules);

        configGroups.append(configGroup);
    }
    static QiRuleManager* instance();

    QMap<ConfigKey,QVariant> getConfig(int i){
        return configGroups.at(i);
    }
    QMap<ConfigKey,QVariant> getRule(ConnectionData_ptr connectionData,bool* isMatch=0);

    bool isRuleMatch(QMap<ConfigKey,QVariant> rule, ConnectionData_ptr connectionData);
    bool isRuleNeedBlockOrientResponse(RuleType ruleType){
        switch(ruleType){
            case RuleType_SimpleAddressReplace:
            case RuleType_ComplexAddressReplace:
            case RuleType_PathReplace:
                return false;
            default:
                return true;
        }
    }

private:

    /*******************
      configGroups结构：(QVector)
            [configGroup1,configGroup2,configGroup3,configGroup4...]
      configGroup结构   (QMap)
        {
            ConfigKey_Author :
            ConfigKey_RemoteHost,
            ConfigKey_RemoteAddress,
            ConfigKey_RemotePath,
            ConfigKey_LocalConfigFile,
            ConfigKey_Rules: [ config1,config2  ]
          }
       config结构
          {
               ConfigKey_ContentType,
               ConfigKey_Encoding,
               ConfigKey_RuleType,
               ConfigKey_RulePattern,
               ConfigKey_RuleReplace
          }

    **************/
    QVector <QMap<ConfigKey,QVariant> >configGroups;
    QMap<ConfigKey,QVariant> emptyRule;    // for no rule match case

signals:
    
public slots:
    
};

Q_DECLARE_METATYPE(QiRuleManager::QiRuleConent_type)
Q_DECLARE_METATYPE(QiRuleManager::RuleType)
#endif // QIRULEMANAGER_H
