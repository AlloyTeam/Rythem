#include <QtGui/QApplication>
#include <QtGui/QStatusBar>
#include "mainwindow.h"
#include <QDebug>

#include "connectionmonitorwsserver.h"
#include <QDateTime>
#include <QtCore>
#include <QUrl>

#include "ryrulemanager.h"
#include "ryrulegroup.h"
#include "ryrule.h"

#include "ryproxyserver.h"
#include "rypipedata.h"

void myMessageHandler(QtMsgType type, const char *msg)
{
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
        QString fileName = QString("log-%1.txt").arg(QDateTime::currentDateTime().toMSecsSinceEpoch()/(1000*60*60*24));
        QFile outFile(fileName);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << endl;
        outFile.close();
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

#ifdef DEBUGTOFILE
    qInstallMsgHandler(myMessageHandler);
#endif

	//new rule manager test----------------------------------------------------
        RyRuleManager manager("/Users/moscartong/Desktop/config.txt");
	manager.loadConfig(); //2ms
        QList<RyRule *> matchResult;
	manager.getMatchRules(&matchResult, "http://abc.com/a.html"); //2ms
	qDebug() << matchResult;
	//-------------------------------------------------------------------------



    // register metatypes
    qRegisterMetaType<RyPipeData_ptr>("RyPipeData_ptr");

    //QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF8"));
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF8"));

    MainWindow w;

    RyProxyServer* server = RyProxyServer::instance();
    server->connect(server,SIGNAL(pipeBegin(RyPipeData_ptr)),&w,SLOT(onNewPipe(RyPipeData_ptr)));
    server->connect(server,SIGNAL(pipeComplete(RyPipeData_ptr)),&w,SLOT(onPipeUpdate(RyPipeData_ptr)));
    server->connect(server,SIGNAL(pipeError(RyPipeData_ptr)),&w,SLOT(onPipeUpdate(RyPipeData_ptr)));
    server->listen(QHostAddress("127.0.0.1"),8889);

    w.show();


	ConnectionMonitorWSServer wsServer;
	wsServer.start();
	QObject::connect(&w.pipeTableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), &wsServer, SLOT(handleConnectionChange(QModelIndex, QModelIndex)));
	//QObject::connect(&w.pipeTableModel, SIGNAL(connectionAdded(RyPipeData_ptr)), &wsServer, SLOT(handleConnectionAdd(RyPipeData_ptr)));
	//QObject::connect(&w.pipeTableModel, SIGNAL(connectionUpdated(RyPipeData_ptr)), &wsServer, SLOT(handleConnectionUpdate(RyPipeData_ptr)));
	//QObject::connect(&w.pipeTableModel, SIGNAL(connectionRemoved(RyPipeData_ptr)), &wsServer, SLOT(handleConnectionRemove(RyPipeData_ptr)));

    //QString status;
    //    status.sprintf("proxy listening on port %d, control waiting on port %d", server.serverPort(), wsServer.serverPort());
    //w.statusBar()->showMessage(status);

    return a.exec();
}
