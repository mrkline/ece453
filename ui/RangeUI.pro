#-------------------------------------------------
#
# Project created by QtCreator 2013-07-01T11:50:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = range_ui
TEMPLATE = app

CONFIG += c++11 debug -D
#CONFIG += c++11 release

DEFINES += WITH_JSON

LIBS += -ljsoncpp -pthread -ljsoncpp -lboost_system

INCLUDEPATH += ../common/

QMAKE_CXXFLAGS += -Wall -Wextra

SOURCES += main.cpp \
RangeUI.cpp \
../common/*.cpp

HEADERS  += RangeUI.hpp \
../common/*.hpp

FORMS    += RangeUI.ui
