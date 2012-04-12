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
    rytablemodel.cpp \
    qnetworkproxyfactoryexendforpac.cpp \
    composer.cpp \
    rypipetableview.cpp\
    waterfallwindow.cpp \
    rule/ryruleproject.cpp \
    rule/ryrule.cpp \
    rule/ryrulegroup.cpp \
    rule/ryrulemanager.cpp \
    rule/ryrulereplacecontent.cpp \
    proxy/rywinhttp.cpp \
    proxy/ryproxyserver.cpp \
    proxy/rypipedata.cpp \
    proxy/ryconnection.cpp \
    ryconnectiontableview.cpp \
    quazip/zip.c \
    quazip/unzip.c \
    quazip/quazipnewinfo.cpp \
    quazip/quazipfile.cpp \
    quazip/quazip.cpp \
    quazip/quacrc32.cpp \
    quazip/quaadler32.cpp \
    quazip/qioapi.cpp \
    quazip/JlCompress.cpp

mac:SOURCES -= qiwinhttp.cpp

HEADERS  += mainwindow.h \
    rytablemodel.h \
    qnetworkproxyfactoryexendforpac.h \
    composer.h \
    rypipetableview.h \
    waterfallwindow.h \
    rule/ryruleproject.h \
    rule/ryrule.h \
    rule/ryrulegroup.h \
    rule/ryrulemanager.h \
    rule/ryrulereplacecontent.h \
    proxy/rywinhttp.h \
    proxy/ryproxyserver.h \
    proxy/rypipedata.h \
    proxy/ryconnection.h \
    ryconnectiontableview.h \
    quazip/zip.h \
    quazip/unzip.h \
    quazip/quazipnewinfo.h \
    quazip/quazipfileinfo.h \
    quazip/quazipfile.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quacrc32.h \
    quazip/quachecksum32.h \
    quazip/quaadler32.h \
    quazip/JlCompress.h \
    quazip/ioapi.h \
    quazip/crypt.h
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

mac:HEADERS -= proxy/rywinhttp.h

mac:SOURCES -= proxy/rywinhttp.cpp

FORMS    += mainwindow.ui \
    composer.ui \
    waterfallwindow.ui

mac:CONFIG += app_bundle



win32:LIBS += D:\QtSDK\mingw\lib\libwininet.a
mac:LIBS += -framework SystemConfiguration -framework coreFoundation -lz

RESOURCES += \
    httpfiles.qrc

OTHER_FILES += \
    RythemManagerUI/rules.html \
    RythemManagerUI/css/rules.css \
    RythemManagerUI/js/rules.js \
    RythemManagerUI/lib/jo2/jo2.template.js \
    RythemManagerUI/lib/jo2/jo2.string.js \
    RythemManagerUI/lib/jo2/jo2.js \
    RythemManagerUI/lib/jo2/jo2.dom.js \
    RythemManagerUI/lib/jo2/jo2.css \
    RythemManagerUI/lib/jo2/jo2.console.js \
    RythemTimes/index.html \
    RythemTimes/lib/backbone.js \
    RythemTimes/js/times.js \
    RythemTimes/css/times.css \
    static/icloud_64.png \
    RythemTimes/lib/parseuri.js \
    RythemTimes/lib/jo2.js \
    RythemTimes/lib/jo2.dom.js
CONFIG(release){
    #DEFINES += DEBUGTOFILE
    #message("debug")
}else{
    #DEFINES -= DEBUGTOFILE
    #message("release")
}

ICON = rythem.icns
