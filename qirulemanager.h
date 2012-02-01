#ifndef QIRULEMANAGER_H
#define QIRULEMANAGER_H

#include <QObject>
#include <QtCore>
#include <QMap>
#include <qiconnectiondata.h>
#include <QApplication>
#include <QScriptEngine>



class QiRuleManager : public QObject
{
    Q_OBJECT
public:
        enum ConfigKey{
            //配置组信息，需用户提供
            ConfigKey_Author = 0,
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

        // 为配置文件兼容，以下enum的值只能添加，不能修改！！！
        enum RuleType{
            RuleType_LocalContentSingleReplace  = 0,  // 替换本地内容
            RuleType_LocalContentMergeReplace   = 6,  // 合并替换本地内容，格式兼容qzmin
            RuleType_RemoteContentReplace       = 1,  // 替换远程内容   （本地预读取还是远程获取？本地预读需处理“更新”逻辑)
            RuleType_SimpleAddressReplace       = 2,  // 替换远程ip地址 一个域名对应一个host
            RuleType_ComplexAddressReplace      = 3,  // 替换远程ip地址一个域名对应多个host
            // TODO
            RuleType_PathReplace                = 4,            // 路径重定向(仅保留域名) 如 http://qplus.com/cgi_developing_or_has_error 重定向至 http://qplus.com/cgi_ok_but_will_discard
            RuleType_DomainReplace              = 5 // 域名重定向(仅替换域名) 如 http://domain.NOT.deploy.com/cgi 重定向至 http://domain.deploy.com/
        };
        enum ContentReplaceRuleType{
            ContentReplaceRuleType_SingleReplace = 0,          //单个文件替换
            ContentReplaceRuleType_MutiReplace   = 1         //多文件合并替换
        };
    //typedef QVector< QMap<QiRuleManager::ConfigKey,QVariant> > QiRulesVector_type;
    //typedef QList< QMap<ConfigKey,QVariant> > QiRulesList_type;
    typedef QMap<QiRuleManager::ConfigKey,QVariant> QiRuleConent_type;
    inline QiRuleManager(QObject *parent = 0):QObject(parent){
        qRegisterMetaType<QiRuleManager::RuleType>("QiRuleManager::RuleType");

        // data for test only
        QMap<ConfigKey,QVariant> configGroup;

        QString configFilePath = QApplication::instance()->applicationDirPath()+"/config.txt";
        qDebug()<<configFilePath;
        QFile configFile(configFilePath);


        if(configFile.exists() && configFile.open(QFile::ReadOnly)){
            qDebug()<<"config file exists";
            configGroup[ConfigKey_Author] = "ippan";
            configGroup[ConfigKey_LocalConfigFile] = configFilePath;
            QList<QVariant> theRules;
            while(!configFile.atEnd()){
                QiRuleConent_type rule;
                QByteArray line = configFile.readLine().trimmed();
                rule[ConfigKey_RuleType] = line.toInt();
                if(configFile.atEnd()){
                    break;
                }
                line = configFile.readLine().trimmed();
                rule[ConfigKey_RulePattern] = QString(line);
                if(configFile.atEnd()){
                    break;
                }
                line = configFile.readLine().trimmed();
                rule[ConfigKey_RuleReplace] = QString(line);
                theRules.append(qVariantFromValue(rule));
            }
            configGroup[ConfigKey_Rules] = qVariantFromValue(theRules);
        }else{

            QStringList configForDebug;
            configForDebug<<"0"<<"http://www.itisme.com"<<"./config.txt";
            configForDebug<<"0"<<"http://iptton.sinaapp.com/a0.js"<<"./a.js";
            configForDebug<<"1"<<"http://www.qq.com"<<"http://w.qq.com/js/main.js";
            configForDebug<<"6"<<"http://rythem.com/"<<"./rythem.merge.qzmin";
            configForDebug.join("\r\n");

            configGroup[ConfigKey_Author] = "ippan";
            configGroup[ConfigKey_RemoteHost] = "ippan.web.qq.com";
            configGroup[ConfigKey_RemoteAddress] = "113.108.4.143";
            configGroup[ConfigKey_RemotePath] = "ip_config.json";
            QList<QVariant> theRules;
            //QList< QMap<ConfigKey,QVariant> > theRules;
            QiRuleConent_type rule1;
            rule1[ConfigKey_RuleType] = RuleType_RemoteContentReplace;
            rule1[ConfigKey_RulePattern] = QString("http://www.qq.com/");
            rule1[ConfigKey_RuleReplace] = QString("http://w.qq.com/js/main.js");

            QiRuleConent_type rule2;
            rule2[ConfigKey_RuleType] = RuleType_SimpleAddressReplace;
            rule2[ConfigKey_RulePattern] = QString("ippan.web.qq.com");
            rule2[ConfigKey_RuleReplace] = QString("113.108.4.143");

            //http://iptton.sinaapp.com/a0.js
            QiRuleConent_type rule3;
            rule3[ConfigKey_RuleType] = RuleType_LocalContentSingleReplace;
            rule3[ConfigKey_RulePattern] = QString("http://iptton.sinaapp.com/a0.js");
            rule3[ConfigKey_RuleReplace] = QString("/Users/emrelax/a.js");

            QiRuleConent_type rule4;
            rule4[ConfigKey_RuleType] = RuleType_LocalContentMergeReplace;
            rule4[ConfigKey_RulePattern] = QString("http://www.rythem.com/merge");
            rule4[ConfigKey_RuleReplace] = QString("./test.js");


            theRules.append(qVariantFromValue(rule1));
            theRules.append(qVariantFromValue(rule2));
            theRules.append(qVariantFromValue(rule3));

            //theRules.append(rule2);

            configGroup[ConfigKey_Rules] = qVariantFromValue(theRules);
        }
        configGroups.append(configGroup);
    }
    static QiRuleManager* instance();

    QMap<ConfigKey,QVariant> getConfig(int i){
        return configGroups.at(i);
    }
    QMap<ConfigKey,QVariant> getRule(ConnectionData_ptr connectionData,bool* isMatch=0);

    bool isRuleMatch(QMap<ConfigKey,QVariant> rule, ConnectionData_ptr connectionData);
    static bool isRuleNeedBlockOrientResponse(int ruleType){
        //qDebug()<<ruleType;
        switch(ruleType){
            case RuleType_SimpleAddressReplace:
            case RuleType_ComplexAddressReplace:
            case RuleType_PathReplace:
                return false;
            default:
                return true;
        }
    }

    static QPair<QByteArray,QByteArray> getReplaceContent(QMap<ConfigKey,QVariant> rule);

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
