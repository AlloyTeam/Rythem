#include <QtGui/QApplication>
#include <QtGui/QStatusBar>
#include "mainwindow.h"
#include <QDebug>

#include <QDateTime>
#include <QtCore>
#include <QUrl>

#include "rule/ryrulemanager.h"


#include "proxy/ryproxyserver.h"
#include "proxy/rypipedata.h"

#include <QThread>

#include <QTranslator>
#include "singleapplication.h"

#ifdef Q_OS_MAC
#include "proxy/proxyautoconfig.h"
#endif

#include <QtNetwork>
QList<QNetworkProxy> macQueryInternal(const QNetworkProxyQuery &query);

using namespace rule;

QString version = "0.1.0";
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

//#define DEBUGTOFILE

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv, "a");
    if(a.isRunning()){
        a.sendMessage("from other instance");
        return 0;

    }


    ProxyAutoConfig::instance();
    a.setQuitOnLastWindowClosed(false);
    appPath =  qApp->applicationDirPath();

    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    // load app translate file
    QTranslator translator;
    bool isLoadSuccess = translator.load(appPath+"/rythem_zh_CN");
    if(!isLoadSuccess)qDebug()<<" load language error";
    a.installTranslator(&translator);

    // load system translate file
    QTranslator translator2;
    translator2.load(appPath+"/qt_zh_CN");
    a.installTranslator(&translator2);


#ifdef DEBUGTOFILE
    qInstallMsgHandler(myMessageHandler);
#endif

    RyRuleManager *manager = RyRuleManager::instance();
    manager->loadLocalConfig(appPath+"/rythem_config.txt");

    // register metatypes
    qRegisterMetaType<RyPipeData_ptr>("RyPipeData_ptr");

    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

    MainWindow w;
    //a.connect(&a,SIGNAL(messageAvailable(QString)),&w,SLOT(raise()));
    a.connect(&a,SIGNAL(messageAvailable(QString)),&w,SLOT(onMessageFromOtherInstance()));

    RyProxyServer* server = RyProxyServer::instance();


    server->connect(server,SIGNAL(pipeBegin(RyPipeData_ptr)),&w,SLOT(onNewPipe(RyPipeData_ptr)));
    server->connect(server,SIGNAL(pipeComplete(RyPipeData_ptr)),&w,SLOT(onPipeUpdate(RyPipeData_ptr)));
    server->connect(server,SIGNAL(pipeError(RyPipeData_ptr)),&w,SLOT(onPipeUpdate(RyPipeData_ptr)));
    server->listen(QHostAddress("127.0.0.1"),8889);

    //w.showMaximized();
    w.show();
    return a.exec();

}
