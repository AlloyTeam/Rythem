#include <QtGui/QApplication>
#include <QtGui/QStatusBar>
#include "mainwindow.h"
#include <QDebug>

#include "connectionmonitorwsserver.h"
#include <QDateTime>
#include <QtCore>
#include <QUrl>

#include "rule/ryrulemanager.h"


#include "ryproxyserver.h"
#include "rypipedata.h"

#include <QThread>

using namespace rule;


QString appPath = "";
void myMessageHandler(QtMsgType type, const char *msg)
{
    QString fileName = appPath+QString("/log-%1.txt").arg(QDateTime::currentDateTime().toMSecsSinceEpoch()/(1000*60*60*24));
    QFile outFile;
    outFile.setFileName(fileName);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QString txt;
        switch (type) {
        case QtDebugMsg:
                txt = QString("Debug: %1\r\n").arg(msg);
                break;
        case QtWarningMsg:
                txt = QString("Warning: %1\r\n").arg(msg);
        break;
        case QtCriticalMsg:
                txt = QString("Critical: %1\r\n").arg(msg);
        break;
        case QtFatalMsg:
                txt = QString("Fatal: %1\r\n").arg(msg);
                abort();
        }
        QTextStream ts(&outFile);
        ts << txt << endl;
        outFile.close();
}

#define DEBUGTOFILE

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    appPath =  qApp->applicationDirPath();


#ifdef DEBUGTOFILE
    qInstallMsgHandler(myMessageHandler);
#endif

    RyRuleManager *manager = RyRuleManager::instance();
#ifdef Q_WS_WIN32
    manager->loadLocalConfig(appPath+"/rythem_config.txt");
#else
    manager->loadLocalConfig(appPath+"/../../../../Rythem/rythem_config.txt");
#endif

    // register metatypes
    qRegisterMetaType<RyPipeData_ptr>("RyPipeData_ptr");

    //QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

    MainWindow w;

    RyProxyServer* server = RyProxyServer::instance();


    server->connect(server,SIGNAL(pipeBegin(RyPipeData_ptr)),&w,SLOT(onNewPipe(RyPipeData_ptr)));
    server->connect(server,SIGNAL(pipeComplete(RyPipeData_ptr)),&w,SLOT(onPipeUpdate(RyPipeData_ptr)));
    server->connect(server,SIGNAL(pipeError(RyPipeData_ptr)),&w,SLOT(onPipeUpdate(RyPipeData_ptr)));
    server->listen(QHostAddress("127.0.0.1"),8889);

    w.show();

    return a.exec();

}
