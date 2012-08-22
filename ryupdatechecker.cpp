#include "ryupdatechecker.h"
#include <QMessageBox>
#include <QNetworkProxy>

RyUpdateChecker::RyUpdateChecker(QObject *parent) :
    QObject(parent){
    _isChecking = false;
    manager.setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,"127.0.0.1",8889));
}

void RyUpdateChecker::check(QString url, QString currentVersion){
    if(_isChecking){
        return;
    }
    _isChecking = true;
    this->_currentVersion  = currentVersion;
    connect(&manager,SIGNAL(finished(QNetworkReply*)),SLOT(finished(QNetworkReply*)));
    manager.get(QNetworkRequest(QUrl(url)));
}

bool RyUpdateChecker::isLargeThanCurrent(const QString &otherVersion,bool* isOk){
    QStringList currentVersionList = this->_currentVersion.split(".");
    QStringList otherVersionList = otherVersion.split(".");
    if(currentVersionList.size() != otherVersionList.size()){
        if(isOk)*isOk=false;
        return false;
    }
    if(isOk)*isOk=true;
    for(int i=0,l=currentVersionList.size();i<l;++i){
        qDebug()<<QString::number(currentVersionList.at(i).toInt())<<QString::number(otherVersionList.at(i).toInt());
        int currentV = currentVersionList.at(i).toInt();
        int onlineV = otherVersionList.at(i).toInt();
        if( onlineV < currentV ){
            return false;
        }else if(onlineV > currentV){
            return true;
        }
    }
    return false;
}

void RyUpdateChecker::finished(QNetworkReply *reply){
    _isChecking = false;
    if(reply->error() != QNetworkReply::NoError){
        //QMessageBox::warning((QWidget*)parent(),tr("net work error"),tr("upate failure"),QMessageBox::Ok);
        emit updateError(reply->error());
        return;
    }

    QByteArray versionBa = reply->readLine();
    bool isTextOk =  false;
    qDebug()<<versionBa;
    if( !isLargeThanCurrent(versionBa,&isTextOk) ){
        /*
        QString message = tr("no update");
        if(!isTextOk){
            message = tr("remote update api error:%1").arg(QString(versionBa));
        }
        QMessageBox::information((QWidget*)this->parent(),
                                 tr("no update avaliable"),
                                 message,
                                 QMessageBox::Ok);
        */
        emit updated();
        return;
    }

    QByteArray updateUrlba = reply->readLine();
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QByteArray updateLogba = reply->readAll();
    qDebug()<<updateUrlba<<updateUrlba<<versionBa;

    QMessageBox::information((QWidget*)this->parent(),
                             tr("got new version"),
                             tr("old version:%1\nnew version:%2\nupdate:%3\nchangeLog:%4")
                                .arg(_currentVersion)
                                .arg(QString(versionBa))
                                .arg(QString(updateUrlba))
                                .arg(QString(updateLogba)),
                             QMessageBox::Ok);
    emit gotNew(_currentVersion,versionBa,updateUrlba,updateLogba);

}

