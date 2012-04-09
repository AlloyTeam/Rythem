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
    QPair<QByteArray,QByteArray> getReplaceContent(const QString& url,bool setLongCache=false);
    QPair<QByteArray,QByteArray> getReplaceContent(bool setLongCache=false);
    void setUrl(const QString&);
    void setRule(QSharedPointer<RyRule>);
private:

    QPair<QByteArray,QByteArray> getRemoteReplaceContent(bool setLongCache=false);
    QPair<QByteArray,QByteArray> getLocalReplaceContent(bool setLongCache=false);
    QPair<QByteArray,QByteArray> getLocalMergeReplaceContent(bool setLongCache=false);
    QPair<QByteArray,QByteArray> getLocalDirReplaceContent(bool setLongCache=false);

    QNetworkAccessManager manager;
    QString _url;
    QSharedPointer<RyRule> _rule;
    QEventLoop *_loop;
};// class RyRuleReplaceContent

} // namespace manager

#endif // RYRULEREPLACECONTENT_H
