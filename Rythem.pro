#-------------------------------------------------
#
# Project created by QtCreator 2011-12-22T10:15:59
#
#-------------------------------------------------

QT       += core gui network script webkit


TARGET = Rythem
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qiddlerpipetablemodel.cpp \
    qnetworkproxyfactoryexendforpac.cpp \
    qiproxyserver.cpp \
    qipipe.cpp \
    qiresponse.cpp \
    qiwinhttp.cpp \
    qiconnectiondata.cpp \
    qirulemanager.cpp \
    bytearray.cpp \
    websocketserver.cpp \
    websocketclient.cpp \
    connectionmonitorwsserver.cpp \
    qirulesettingsdialog.cpp \
    qiresponsefactory.cpp \
    qiremoteresponseworker.cpp \
    iqiresponseworker.cpp \
    qilocalfileresponseworker.cpp \
    httpdecoder.cpp \
    qirulemanager2.cpp \
    json.cpp \
    qirulegroup2.cpp \
    qirule2.cpp \
    qirulecomplexaddress.cpp \
    qirulesimpleaddress.cpp \
    qiruleremotecontent.cpp \
    qirulelocalfile.cpp \
    qirulelocalfiles.cpp \
	qirulelocaldir.cpp

mac:SOURCES -= qiwinhttp.cpp

HEADERS  += mainwindow.h \
    qiddlerpipetablemodel.h \
    qnetworkproxyfactoryexendforpac.h \
    qiproxyserver.h \
    qipipe.h \
    qiresponse.h \
    qiwinhttp.h \
    qiconnectiondata.h \
    qirulemanager.h \
    bytearray.h \
    websocketserver.h \
    websocketclient.h \
    connectionmonitorwsserver.h \
    qirulesettingsdialog.h \
    qiresponsefactory.h \
    iqiresponseworker.h \
    qiremoteresponseworker.h \
    qilocalfileresponseworker.h \
    httpdecoder.h \
    qirulemanager2.h \
    json.h \
    qirulegroup2.h \
    qirule2.h \
    qirulecomplexaddress.h \
    qirulesimpleaddress.h \
    qiruleremotecontent.h \
    qirulelocalfile.h \
    qirulelocalfiles.h \
	qirulelocaldir.h

mac:HEADERS -= qiwinhttp.h

FORMS    += mainwindow.ui \
    rule_config.ui

mac:CONFIG += app_bundle



win32:LIBS += D:\QtSDK\mingw\lib\libwininet.a
mac:LIBS += -framework SystemConfiguration -framework coreFoundation

RESOURCES += \
    httpfiles.qrc

OTHER_FILES += \
    wsAPI/index.css \
    wsAPI/index.html \
    wsAPI/index.js \
    index.html
CONFIG(release){
    #DEFINES += DEBUGTOFILE
    #message("debug")
}else{
    #DEFINES -= DEBUGTOFILE
    #message("release")
}
