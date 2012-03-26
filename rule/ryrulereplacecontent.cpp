#include "ryrulereplacecontent.h"

using namespace rule;
#include "ryrule.h"


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

    QTimer timer;
    timer.singleShot(5000,_loop,SLOT(quit()));//5秒内
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    manager.connect(&manager,SIGNAL(finished(QNetworkReply*)),_loop,SLOT(quit()));
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(_rule->replace())));
    _loop->exec();

    qDebug()<<QString(reply->readAll());
    bool isOk;
    qDebug()<<QString::number( reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&isOk));
    //bool isRequestDone = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).isValid();
    if(!isOk){
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
                     "Content-Type: %2 \r\n"
                     "Content-Length: %3 \r\n\r\n")
            .arg(status)
            .arg(mimeType)
            //.arg(encode)
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
    header.append(QString("HTTP/1.1 %1 \r\nServer: Rythem \r\nContent-Type: %2 \r\nContent-Length: %3 \r\n\r\n")
                       .arg(status)
                       .arg(mimeType) // TODO
                       //.arg(encode)
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
    if(file.open(QFile::ReadOnly)){
        status = "200 OK";
        body = file.readAll();
    }else{
        status = "404 Not Found";
        body.append(QString("file:%1 not found").arg(fileName));
    }
    mimeType = RyRule::getMimeType(QFileInfo(file).suffix().toLower(),"text/plain");
    file.close();
    contentLength = body.size();//
    header.append(QString("HTTP/1.1 %1 \r\nServer: Rythem \r\nCache-Control:max-age=315360000 \r\nContent-Type: %2 \r\nContent-Length: %3 \r\n\r\n")
                       .arg(status)
                       .arg(mimeType)
                       //.arg(encode)
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
