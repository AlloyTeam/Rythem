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
    rywinhttp.cpp \
    bytearray.cpp \
    websocketserver.cpp \
    websocketclient.cpp \
    connectionmonitorwsserver.cpp \
    qirulesettingsdialog.cpp\
    httpdecoder.cpp \
    json.cpp \
    ryrulecomplexaddress.cpp \
    ryproxyserver.cpp \
    rypipedata.cpp \
    ryconnection.cpp \
    composer.cpp \
    rypipetableview.cpp \
    ryrulegroup.cpp \
    ryrule.cpp \
    ryrulemanager.cpp \
    ryrulelocalfiles.cpp \
    ryrulesimpleaddress.cpp \
    ryruleremotecontent.cpp \
    ryrulelocalfile.cpp \
    ryrulelocaldir.cpp

mac:SOURCES -= qiwinhttp.cpp

HEADERS  += mainwindow.h \
    qiddlerpipetablemodel.h \
    qnetworkproxyfactoryexendforpac.h \
    rywinhttp.h \
    rypipedata.h \
    bytearray.h \
    websocketserver.h \
    websocketclient.h \
    connectionmonitorwsserver.h \
    qirulesettingsdialog.h \
    httpdecoder.h \
    json.h \
    ryproxyserver.h \
    rypipedata.h \
    ryconnection.h \
    composer.h \
    rypipetableview.h \
    ryrulemanager.h \
    ryrulegroup.h \
    ryrulelocaldir.h \
    ryrule.h \
    ryrulesimpleaddress.h \
    ryruleremotecontent.h \
    ryrulelocalfiles.h \
    ryrulelocalfile.h \
    ryrulecomplexaddress.h
win32:HEADERS += zlib/zutil.h \
    zlib/zlib.h \
    zlib/zconf.h \
    zlib/trees.h \
    zlib/inftrees.h \
    zlib/inflate.h \
    zlib/inffixed.h \
    zlib/inffast.h \
    zlib/gzguts.h \
    zlib/deflate.h \
    zlib/crc32.h

mac:HEADERS -= qiwinhttp.h

FORMS    += mainwindow.ui \
    rule_config.ui \
    composer.ui

mac:CONFIG += app_bundle



win32:LIBS += G:\QtSDK\mingw\lib\libwininet.a
mac:LIBS += -framework SystemConfiguration -framework coreFoundation -lz

RESOURCES += \
    httpfiles.qrc

OTHER_FILES += \
    wsAPI/index.css \
    wsAPI/index.html \
    wsAPI/index.js \
    index.html \
    RythemManagerUI/rules.html \
    RythemManagerUI/css/rules.css \
    RythemManagerUI/js/rules.js \
    RythemManagerUI/lib/jo2/jo2.template.js \
    RythemManagerUI/lib/jo2/jo2.string.js \
    RythemManagerUI/lib/jo2/jo2.js \
    RythemManagerUI/lib/jo2/jo2.dom.js \
    RythemManagerUI/lib/jo2/jo2.css \
    RythemManagerUI/lib/jo2/jo2.console.js
CONFIG(release){
    #DEFINES += DEBUGTOFILE
    #message("debug")
}else{
    #DEFINES -= DEBUGTOFILE
    #message("release")
}
