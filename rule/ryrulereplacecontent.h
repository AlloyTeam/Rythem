#ifndef RYRULEREPLACECONTENT_H
#define RYRULEREPLACECONTENT_H

#include <QtCore>
#include <QtNetwork>


namespace rule{
class RyRule;

class RyRuleReplaceContent{
public:
    RyRuleReplaceContent(QSharedPointer<RyRule> rule, const QString& url="");
    ~RyRuleReplaceContent();
    QPair<QByteArray,QByteArray> getReplaceContent(const QString& url,bool setLongCache=false,bool *isResouceFound=0);
    QPair<QByteArray,QByteArray> getReplaceContent(bool setLongCache=false,bool *isResouceFound=0);
    void setUrl(const QString&);
    void setRule(QSharedPointer<RyRule>);
private:

    QPair<QByteArray,QByteArray> getRemoteReplaceContent(bool setLongCache=false,bool *isResouceFound=0);
    QPair<QByteArray,QByteArray> getLocalReplaceContent(bool setLongCache=false,bool *isResouceFound=0);
    QPair<QByteArray,QByteArray> getLocalMergeReplaceContent(bool setLongCache=false,bool *isResouceFound=0);
    QPair<QByteArray,QByteArray> getLocalMergeReplaceContent2(bool setLongCache=false,bool *isResouceFound=0);
    QPair<QByteArray,QByteArray> getLocalDirReplaceContent(bool setLongCache=false,bool *isResouceFound=0);

    QPair<QByteArray,QByteArray> getMergedContents(QStringList fileNames,QString mimeType = "text/plain",bool setLongCache=false);

    QNetworkAccessManager manager;
    QString _url;
    QSharedPointer<RyRule> _rule;
    QEventLoop *_loop;
};// class RyRuleReplaceContent

} // namespace manager

#endif // RYRULEREPLACECONTENT_H
