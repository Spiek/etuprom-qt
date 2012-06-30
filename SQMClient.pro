#-------------------------------------------------
#
# Project created by QtCreator 2012-04-21T20:16:11
#
#-------------------------------------------------

QT       += core gui network

TARGET = SQMClient
TEMPLATE = app


SOURCES +=  client/src/main.cpp\
		client/src/mainwindow.cpp \
		collective/src/sqmpackethandler.cpp

HEADERS  += client/src/mainwindow.h  \
			collective/src/sqmpackethandler.h

FORMS    += client/ui/mainwindow.ui

INCLUDEPATH += "include/"
