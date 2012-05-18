#ifndef RYWEBVIEW_H
#define RYWEBVIEW_H

#include <QWebView>

class RyWebView : public QWebView
{
    Q_OBJECT
public:
    explicit RyWebView(QWidget *parent = 0);

signals:
    
public slots:
protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);
/*
private:// disable load
    void load(const QNetworkRequest &request, QNetworkAccessManager::Operation operation, const QByteArray &body);
    void load(const QUrl&);
*/
};

#endif // RYWEBVIEW_H
