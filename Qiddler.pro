#-------------------------------------------------
#
# Project created by QtCreator 2011-12-22T10:15:59
#
#-------------------------------------------------

QT       += core gui network script webkit


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
    qirulesettingsdialog.cpp \
    qiresponsefactory.cpp \
    qiremoteresponseworker.cpp \
    iqiresponseworker.cpp \
    qilocalfileresponseworker.cpp \
    httpdecoder.cpp

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
    httpdecoder.h

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
