#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QDebug>
#include "qiconnectiondata.h"
#include "qiproxyserver.h"
#include <QDateTime>
#include <QtCore>



void myMessageHandler(QtMsgType type, const char *msg)
{
        QString txt;
        switch (type) {
        case QtDebugMsg:
                txt = QString("Debug: %1").arg(msg);
                break;
        case QtWarningMsg:
                txt = QString("Warning: %1").arg(msg);
        break;
        case QtCriticalMsg:
                txt = QString("Critical: %1").arg(msg);
        break;
        case QtFatalMsg:
                txt = QString("Fatal: %1").arg(msg);
                abort();
        }
        QString fileName = QString("log-").arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
        QFile outFile(fileName);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << endl;
}

#define DEBUG true
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(!DEBUG){
        qInstallMsgHandler(myMessageHandler);
    }
    // register metatypes
    qRegisterMetaType<ConnectionData_const_ptr>("ConnectionData_const_ptr");
    qRegisterMetaType<ConnectionData_ptr>("ConnectionData_ptr");

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF8"));

    MainWindow w;

    QiProxyServer *server = new QiProxyServer();
    // TODO listne should be a slot
    server->listen(QHostAddress("127.0.0.1"),8889);
    server->connect(server,SIGNAL(newPipe(ConnectionData_ptr)),&w,SLOT(onNewPipe(ConnectionData_ptr)));
    server->connect(server,SIGNAL(pipeUpdate(ConnectionData_ptr)),&w,SLOT(onPipeUpdate(ConnectionData_ptr)));
    w.show();

    return a.exec();
}
