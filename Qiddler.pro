#-------------------------------------------------
#
# Project created by QtCreator 2011-12-22T10:15:59
#
#-------------------------------------------------

QT       += core gui network script


TARGET = Qiddler
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
    qirulesettingsdialog.cpp

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
    qirulesettingsdialog.h

mac:HEADERS -= qiwinhttp.h

FORMS    += mainwindow.ui \
    rule_config.ui



win32:LIBS += G:\QtSDK\mingw\lib\libwininet.a
mac:LIBS += -framework SystemConfiguration -framework coreFoundation

RESOURCES += \
    httpfiles.qrc

OTHER_FILES += \
    websocket.html \
    test.js \
    index.html \
    websocket.manifest



















