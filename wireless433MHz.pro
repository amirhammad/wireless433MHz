QT       += core

QT       -= gui

TARGET = wireless433MHz
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
#DEFINES += DEBUG_DATA
LIBS += -L/usr/local/lib -lyahdlc
SOURCES += main.cpp \
    Communication.cpp

HEADERS += \
    Communication.h
