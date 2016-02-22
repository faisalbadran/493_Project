#-------------------------------------------------
#
# Project created by QtCreator 2016-02-21T16:25:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RegionTracking
TEMPLATE = app

INCLUDEPATH += . /usr/local/Cellar/imagemagick/6.9.3-0_2/include/ImageMagick-6

LIBS += -L/usr/local/lib -lMagick++-6.Q16

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
