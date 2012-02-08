#include "rypiperesponse.h"

RyPipeResponse::RyPipeResponse(QTcpSocket* requestSocket):
    QObject(){
    connect(requestSocket,SIGNAL(readyRead()),this,SLOT(onRequestReadReady()));
    connect(requestSocket,SIGNAL(disconnected()),this,SLOT(onRequestClose()));
    connect(requestSocket,SIGNAL(aboutToClose()),this,SLOT(onRequestClose()));
    connect(requestSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onRequestError()));
    connect(requestSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(onRequestStateChanged(QAbstractSocket::SocketState)));

}
