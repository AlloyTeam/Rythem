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
    QPair<QByteArray,QByteArray> getReplaceContent(const QString& url);
    QPair<QByteArray,QByteArray> getReplaceContent();
    void setUrl(const QString&);
    void setRule(QSharedPointer<RyRule>);
private:

    QPair<QByteArray,QByteArray> getRemoteReplaceContent();
    QPair<QByteArray,QByteArray> getLocalReplaceContent();
    QPair<QByteArray,QByteArray> getLocalMergeReplaceContent();
    QPair<QByteArray,QByteArray> getLocalDirReplaceContent();

    QNetworkAccessManager manager;
    QString _url;
    QSharedPointer<RyRule> _rule;
    QEventLoop *_loop;
};// class RyRuleReplaceContent

} // namespace manager

#endif // RYRULEREPLACECONTENT_H
