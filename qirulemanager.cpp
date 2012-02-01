#include "qirulemanager.h"
#include <QVariant>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

Q_GLOBAL_STATIC(QiRuleManager, ruleManager)
QiRuleManager *QiRuleManager::instance(){
    return ruleManager();
}

//extern static QScriptEngine QiRuleManager::engine;

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
        qDebug()<<"--- host="<<connectionData->host<<entry;
        if (entry == connectionData->host){
            return true;
        }
    }else if(type == RuleType_ComplexAddressReplace){
        qDebug()<<"fullUrl="<<connectionData->fullUrl;
        return (entry.indexOf(connectionData->fullUrl) != -1);// TODO
    }else if(type == RuleType_LocalContentSingleReplace){
        qDebug()<<"fullUrl="<<connectionData->fullUrl;
        return rx.exactMatch(connectionData->fullUrl);
    }else if(type == RuleType_RemoteContentReplace){
        return rx.exactMatch(connectionData->fullUrl);
    }else if(type == RuleType_LocalContentMergeReplace){
        return rx.exactMatch(connectionData->fullUrl);
    }

    return false;
}

QPair<QByteArray,QByteArray> QiRuleManager::getReplaceContent(QMap<ConfigKey,QVariant> rule){
    QPair<QByteArray,QByteArray> headerAndContent;
    QByteArray header;
    QByteArray body;

    int type = rule[ QiRuleManager::ConfigKey_RuleType].toInt();
    QString pattern = rule[QiRuleManager::ConfigKey_RulePattern].toString();
    Q_UNUSED(pattern)
    QString replace = rule[QiRuleManager::ConfigKey_RuleReplace].toString();
    int count;
    QEventLoop theLoop;
    QNetworkReply* reply;
    QFile f;
    QString status;
    bool fileCanOpen;

    QByteArray mergeFileContent;
    QMap<QString,QVariant> mergeValueMap;
    bool mergeConentHasError = false;

    qDebug()<<"rultype="<<type;
    switch(type){
         case QiRuleManager::RuleType_LocalContentSingleReplace:
            f.setFileName(replace);
            fileCanOpen = f.open(QFile::ReadOnly);
            status = "200 OK";
            if(fileCanOpen){
                body = f.readAll();
                f.close();
            }else{
                status = "404 Not Found";
                body.append(QString("file:%1 not found").arg(replace));
            }
            count = body.size();
            header.append(QString("HTTP/1.1 %1 \r\nServer: Qiddler \r\nContent-Type: %2 \r\nContent-Length: %3 \r\n\r\n")
                               .arg(status)
                               .arg("text/html") // TODO reuse contentTypeMapping above
                               .arg(count));
            break;
        case QiRuleManager::RuleType_LocalContentMergeReplace:
            f.setFileName(replace);
            fileCanOpen = f.open(QFile::ReadOnly);
            if(fileCanOpen){
                QScriptEngine engine;
                mergeFileContent = f.readAll();
                mergeValueMap = engine.evaluate(mergeFileContent.prepend("(").append(")")).toVariant().toMap();
                mergeValueMap = mergeValueMap["projects"].toList().first().toMap();
                if(engine.hasUncaughtException() || mergeValueMap.isEmpty()){//wrong content
                    qDebug()<<"wrong qzmin format:"<<mergeFileContent;
                    mergeConentHasError = true;
                }
                f.close();
            }else{
                mergeConentHasError = true;
            }
            if(mergeConentHasError){
                status = "404 NOT FOUND";
                body.append(QString("merge file with wrong format:").append(mergeFileContent));
            }else{
                foreach(QVariant item,mergeValueMap["includes"].toList()){
                    f.setFileName(item.toString());
                    fileCanOpen = f.open(QFile::ReadOnly);
                    if(fileCanOpen){
                        body.append(f.readAll());
                    }else{
                        body.append(QString("/*file:%1 not found\r\n*/").arg(item.toString()));
                    }
                }
            }
            count = body.size();
            header.append(QString("HTTP/1.1 %1 \r\nServer: Qiddler \r\nContent-Type: %2 \r\nContent-Length: %3 \r\n\r\n")
                               .arg(status)
                               .arg("x-application/javascript") // TODO reuse contentTypeMapping above
                               .arg(count));
            break;

        case QiRuleManager::RuleType_RemoteContentReplace:
            QNetworkAccessManager networkManager;
            reply = networkManager.get(QNetworkRequest(QUrl(replace)));
            connect(&networkManager,SIGNAL(finished(QNetworkReply*)),&theLoop,SLOT(quit()));
            theLoop.exec();
            bool isStatusOk;
            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&isStatusOk);
            if(!isStatusOk){
                status = 2000;
            }
            QByteArray resone = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray();
            header.append(QString("HTTP/1.1 %1 %2 \r\nServer: Qiddler \r\nContent-Type: %3 \r\nContent-Length: %4 \r\n\r\n")
                    .arg(status)
                    .arg(QString(resone))
                    .arg(reply->header(QNetworkRequest::ContentTypeHeader).toString())
                    .arg(reply->header(QNetworkRequest::ContentLengthHeader).toString()));
            body = reply->readAll();
            break;

    }

    headerAndContent.first = header;
    headerAndContent.second = body;
    return headerAndContent;
}
