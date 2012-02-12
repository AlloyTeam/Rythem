#include "composer.h"
#include "ui_composer.h"
#include <QNetworkProxy>
Composer::Composer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Composer){
    ui->setupUi(this);
    socket = new QTcpSocket();
    connect(socket,SIGNAL(connected()),SLOT(onConnected()));
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),SLOT(onError(QAbstractSocket::SocketError)));
    connect(socket,SIGNAL(aboutToClose()),SLOT(onClose()));
    connect(socket,SIGNAL(readyRead()),SLOT(onData()));

    connect(ui->connectbtn,SIGNAL(clicked()),SLOT(onConnectBtnClick()));
    connect(ui->sendbtn,SIGNAL(clicked()),SLOT(onSendBtnClick()));
    connect(ui->disconnectbtn,SIGNAL(clicked()),SLOT(onDisConnectBtnClick()));

    ui->requestsComboBox->clear();

    QStringList requestStr;
    requestStr<<"w.l.qq.com"<<"80"<<
        "GET /lview?type=text&callback=auto_gen_1&loc=QQ_FC_RX_text1,QQ_FC_RX_text2,QQ_FC_RX_text3,QQ_FC_RX_text4,QQ_FC_RX_text5,QQ_FC_DZ_text1,QQ_FC_DZ_text2,QQ_FC_DZ_text3,QQ_FC_DZ_text4,QQ_FC_DZ_text5,QQ_FC_XP_text1,QQ_FC_XP_text2,QQ_FC_XP_text3,QQ_FC_XP_text4,QQ_FC_XP_text5,QQ_FC_ESF_text1,QQ_FC_ESF_text2,QQ_FC_ESF_text3,QQ_FC_ESF_text4,QQ_FC_ESF_text5,QQ_SX_ZS_Test1,QQ_SX_ZS_Test2,QQ_SX_ZS_Test3,QQ_SX_ZS_Test4,QQ_SX_ZS_Test5,QQ_SX_ZS_Test6,QQ_SX_ZS_Test7,QQ_SX_ZS_Test8,QQ_SX_ZS_Test9,QQ_SX_ZS_Test10,QQ_SX_LX_Test1,QQ_SX_LX_Test2,QQ_SX_LX_Test3,QQ_SX_LX_Test4,QQ_SX_LX_Test5,QQ_SX_LX_Test6,QQ_SX_LX_Test7,QQ_SX_LX_Test8,QQ_SX_LX_Test9,QQ_SX_LX_Test10&k=&t=%E8%85%BE%E8%AE%AF%E9%A6%96%E9%A1%B5&r=&s= HTTP/1.1"
        "\nHost: w.l.qq.com"
        "\nConnection: keep-alive"
        "\nAccept-Encoding: identity;q=1, *;q=0"
        "\nUser-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0.963.46 Safari/535.11"
        "\nAccept: */*"
        "\nReferer: http://im.qq.com/macqq/index.shtml"
        "\nAccept-Language: zh-CN,zh;q=0.8"
        "\nAccept-Charset: gb18030,utf-8;q=0.7,*;q=0.3"
        "\nCookie: pgv_r_cookie=116156043288; sd_machines=-1|-1|-1|-1; comment_uin=164473028 %u0069%u0070%u0074%u0074%u006f%u006e; comment_skey=e7eadd2dda7ad02007b2aa2da36d34a7+iptton; old_friend=true; lv_irt_id=fe103783d2a200ef99be838e606bae5d; pvid=8739289251; mbCardUserNotLoginTips=1; pt2gguin=o0164473028; ptisp=ctc; show_id=; o_cookie=164473028; verifysession=h005f594834a73e2d5de281414f85629403925acc6e89b25a779dcb0a60d2147ed189c5a866140b5cc7; pgv_pvid=o_cookie=164473028; pgv_info=ssid=s9100271653&pgvReferrer="
        "\nRange: bytes=48-6116217"
        "\n\n";
    _requests.append(requestStr);
    ui->requestsComboBox->addItem(requestStr.at(0));
    requestStr.clear();

    requestStr<<"w.qq.com"<<"80"<<
                "GET /css/webmini_main.css HTTP/1.1"
                "\nHost: w.qq.com"
                "\n\n";
    _requests.append(requestStr);
    ui->requestsComboBox->addItem(requestStr.at(0));
    requestStr.clear();

    requestStr<<"www.anjiala.com"<<"80"<<
                "GET /design/showview/2012-01-30/21393.html HTTP/1.1"
                "\nHost: www.anjiala.com"
                "\n\n";
    _requests.append(requestStr);
    ui->requestsComboBox->addItem(requestStr.at(0));
    requestStr.clear();



    requestStr<<"trace.qq.com"<<"80"<<
                "GET /collect?pj=1990&dm=w.qq.com&url=/&arg=&rdm=&rurl=&rarg=&icache=-&uv=&nu=&ol=&loc=http%3A//w.qq.com/&column=&subject=&nrnd=164473028&rnd=852 HTTP/1.1"
                "\nHost: trace.qq.com"
                "\nProxy-Connection: keep-alive"
                "\nCache-Control: no-cache"
                "\nPragma: no-cache"
                "\nUser-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0.963.46 Safari/535.11"
                "\nAccept: */*"
                "\nReferer: http://w.qq.com/"
                "\nAccept-Encoding: gzip,deflate,sdch"
                "\nAccept-Language: zh-CN,zh;q=0.8"
                "\nAccept-Charset: gb18030,utf-8;q=0.7,*;q=0.3"
                "\nCookie: pgv_r_cookie=116156043288; sd_machines=-1|-1|-1|-1; comment_uin=164473028 %u0069%u0070%u0074%u0074%u006f%u006e; comment_skey=e7eadd2dda7ad02007b2aa2da36d34a7+iptton; old_friend=true; lv_irt_id=fe103783d2a200ef99be838e606bae5d; pvid=8739289251; pt2gguin=o0164473028; qv_swfrfh=news.qq.com; qv_swfrfc=v2; verifysession=h008080f4865e1b20be8cba15fcb6b9a12b76209b5fe0c5e0355f92fe120c004d13ec81fb98ec036d86; pgv_pvid=o_cookie=164473028; pgv_info=ssid=s9694936942&pgvReferrer=; o_cookie=164473028"
                "\n\n";
    _requests.append(requestStr);
    ui->requestsComboBox->addItem(requestStr.at(0));
    requestStr.clear();


    requestStr<<"bbs.anjiala.com"<<"80"<<
                "GET /member.php?mod=loginstatus HTTP/1.1"
                "\nHost: bbs.anjiala.com"
                "\nConnection: keep-alive"
                "\nUser-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.63 Safari/535.7"
                "\nAccept: */*"
                "\nReferer: http://www.anjiala.com/design/showview/2012-01-30/21393.html"
                "\nAccept-Encoding: gzip,deflate,sdch"
                "\nAccept-Language: zh-CN,zh;q=0.8"
                "\nAccept-Charset: gb18030,utf-8;q=0.7,*;q=0.3"
                "\nCookie: gQHG_a52f_lastvisit=1329015447; gQHG_a52f_sid=G80s6O; gQHG_a52f_lastact=1329024158%09member.php%09loginstatus"
                "\n\n";
    _requests.append(requestStr);
    ui->requestsComboBox->addItem(requestStr.at(0));
    requestStr.clear();




    connect(ui->requestsComboBox,SIGNAL(currentIndexChanged(int)),SLOT(onRequestChanged(int)));


    ui->connectbtn->setDisabled(false);
    ui->sendbtn->setDisabled(true);
    ui->disconnectbtn->setDisabled(true);

    connectToHost();
}

Composer::~Composer()
{
    delete ui;
}

void Composer::setupProxy(QString host,qint16 port){
    _proxyHost = host;
    _proxyPort = port;
    socket->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,host,port));

}

void Composer::onConnected(){
    ui->disconnectbtn->setDisabled(false);
    ui->connectbtn->setDisabled(true);
    ui->sendbtn->setDisabled(false);
    qDebug()<<"connected";
    //QByteArray ba = "GET /empty.html HTTP/1.1\r\nHost: w.qq.com\r\n\r\nGET / HTTP/1.1\r\nHost: w.qq.com\r\n\r\n";
    //socket->write(ba);
    //socket->flush();
}

void Composer::onData(){
    QByteArray ba = socket->readAll();
    //qDebug()<<"response:"<<QString(ba);
    ui->response->setPlainText(ui->response->toPlainText()+QString(ba).replace("\r\n","\\r\\n\r\n"));

}

void Composer::onError(QAbstractSocket::SocketError err){
    qDebug()<<"error"<<err;
    ui->connectbtn->setDisabled(false);
    ui->sendbtn->setDisabled(true);
    ui->disconnectbtn->setDisabled(true);
}

void Composer::onClose(){
    //qDebug()<<"close";
    ui->connectbtn->setDisabled(false);
    ui->sendbtn->setDisabled(true);
    ui->disconnectbtn->setDisabled(true);
}


void Composer::onConnectBtnClick(){
    ui->response->setPlainText("");
    connectToHost();
}

void Composer::onDisConnectBtnClick(){
    //qDebug()<<"close";
    socket->abort();
}

void Composer::onSendBtnClick(){
    sendData(QByteArray().append(ui->request->toPlainText()));
}

void Composer::onRequestChanged(int i){
    if(i<_requests.size()){
        QStringList str = _requests.at(i);
        //qDebug()<<str;
        ui->server->setText(str.at(0));
        ui->port->setText(str.at(1));
        ui->request->setPlainText(str.at(2));
    }
}

void Composer::connectToHost(){
    if(socket->state() != QAbstractSocket::ClosingState
            && socket->state() != QAbstractSocket::UnconnectedState){
        qDebug()<<"already connected";
        socket->abort();
    }
    socket->connectToHost(ui->server->text(),ui->port->text().toInt());
}

void Composer::sendData(const QByteArray &ba){
    /*

      /lview?type=text&callback=auto_gen_1&loc=QQ_FC_RX_text1,QQ_FC_RX_text2,QQ_FC_RX_text3,QQ_FC_RX_text4,QQ_FC_RX_text5,QQ_FC_DZ_text1,QQ_FC_DZ_text2,QQ_FC_DZ_text3,QQ_FC_DZ_text4,QQ_FC_DZ_text5,QQ_FC_XP_text1,QQ_FC_XP_text2,QQ_FC_XP_text3,QQ_FC_XP_text4,QQ_FC_XP_text5,QQ_FC_ESF_text1,QQ_FC_ESF_text2,QQ_FC_ESF_text3,QQ_FC_ESF_text4,QQ_FC_ESF_text5,QQ_SX_ZS_Test1,QQ_SX_ZS_Test2,QQ_SX_ZS_Test3,QQ_SX_ZS_Test4,QQ_SX_ZS_Test5,QQ_SX_ZS_Test6,QQ_SX_ZS_Test7,QQ_SX_ZS_Test8,QQ_SX_ZS_Test9,QQ_SX_ZS_Test10,QQ_SX_LX_Test1,QQ_SX_LX_Test2,QQ_SX_LX_Test3,QQ_SX_LX_Test4,QQ_SX_LX_Test5,QQ_SX_LX_Test6,QQ_SX_LX_Test7,QQ_SX_LX_Test8,QQ_SX_LX_Test9,QQ_SX_LX_Test10&k=&t=%E8%85%BE%E8%AE%AF%E9%A6%96%E9%A1%B5&r=&s=

    */
    QByteArray ba2 = ba;
    ba2.replace("\n","\r\n");
    //qDebug()<<"sending:"<<QString(ba2).replace("\r","\\r").replace("\n","\\n");

    socket->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,"127.0.0.1",8999));
    if(socket->isOpen()){
        socket->write(ba2);
        socket->flush();
    }else{
        qDebug()<<"socket not open";
    }
}
