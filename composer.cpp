#include "composer.h"
#include "ui_composer.h"
#include <QNetworkProxy>

#include "rymimedata.h"
#include "proxy/ryproxyserver.h"
#include "proxy/rypipedata.h"

Composer::Composer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Composer){
    ui->setupUi(this);
    setAcceptDrops(true);
    manager = new QNetworkAccessManager(this);
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    //ui->sendbtn->setDisabled(true);
    pipeData = RyPipeData_ptr( new RyPipeData(0,0));
    connect(ui->sendbtn,SIGNAL(clicked()),SLOT(onSendClicked()));
    connect(manager,SIGNAL(finished(QNetworkReply*)),SLOT(onFinished(QNetworkReply*)));
}

Composer::~Composer()
{
    delete ui;
}



void Composer::setupProxy(QString host,qint16 port){

    qDebug()<<host<<port;
    //socket->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,host,port));

}

void Composer::dragEnterEvent(QDragEnterEvent *event){

    //qDebug()<<"drag enter";
    if (event->mimeData()->hasText()) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }

}

void Composer::dropEvent(QDropEvent *event){
    //qDebug()<<"drop";
    const RyMimeData* mime = dynamic_cast<const RyMimeData*>(event->mimeData());

    if (mime) {
        RyPipeData_ptr d = mime->pipeData();
        QString a = d->requestHeaderRawData();
        a.append("\r\n\r\n");
        a.append(d->requestBodyRawData());
        ui->request->setPlainText(a);
        //qDebug()<<"drop text="<<mime->text();
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void Composer::onFinished(QNetworkReply *reply){
    int result = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString reson = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray();

    QString str = QString("HTTP/1.1 %1 %2\r\n").arg(result).arg(reson);
    QList<QByteArray> headers = reply->rawHeaderList();
    foreach(QByteArray header, headers){
        str.append(header).append(": ").append(reply->rawHeader(header)).append("\r\n");
    }
    str.append("\r\n");
    str.append(reply->readAll());
    qDebug()<<str;
    ui->response->setPlainText(str);
}

void Composer::onSendClicked(){
    bool isOk;
    QByteArray ba = ui->request->toPlainText().toUtf8();
    pipeData->parseRequest(&ba,&isOk);
    //QByteArray ba = ui->request->toPlainText().toUtf8();
    //pipeData->parseRequest(&ba,&isOk);
    if(!isOk){
        qDebug()<<"no OK!";
    }else{
        QNetworkRequest req = QNetworkRequest(QUrl(pipeData->fullUrl));
        const QMap<QString,QString> headers = pipeData->requestHeaders();
        foreach(QString key,headers){
            req.setRawHeader(key.toUtf8(),headers.value(key,QString()).toUtf8());
        }
        manager->setProxy(
                    QNetworkProxy(
                        QNetworkProxy::HttpProxy,RyProxyServer::instance()->serverAddress().toString(),
                        RyProxyServer::instance()->serverPort()
                    )
        );
        ui->response->setPlainText(tr("sending"));
        if(pipeData->method.toUpper() == "GET"){
            manager->get(req);
        }else if(pipeData->method.toUpper() == "POST"){
            manager->post(req,pipeData->requestBodyRawData());
        }else{
            ui->response->setPlainText(tr("no support this method:%1").arg(pipeData->method));
        }
    }
}
