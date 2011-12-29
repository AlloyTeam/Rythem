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
    pipedata.cpp \
    qnetworkproxyfactoryexendforpac.cpp \
    qiproxyserver.cpp \
    qipipe.cpp \
    qiresponse.cpp

HEADERS  += mainwindow.h \
    qiddlerpipetablemodel.h \
    pipedata.h \
    qnetworkproxyfactoryexendforpac.h \
    qiproxyserver.h \
    qipipe.h \
    qiresponse.h

FORMS    += mainwindow.ui



win32:LIBS += G:\qtsdk\mingw\lib\libwininet.a



