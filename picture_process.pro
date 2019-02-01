#-------------------------------------------------
#
# Project created by QtCreator 2018-06-20T10:36:23
#
#-------------------------------------------------

QT       += core gui
RC_ICONS = theico.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = picture_process
TEMPLATE = app


SOURCES += main.cpp\
    base_process.cpp

HEADERS  += \
    base_process.h

FORMS    += \
    base_process.ui
#INCLUDEPATH += D:\OpenCVtoQtRun\include
              #D:\OpenCVtoQtRun\include\opencv
              #D:\OpenCVtoQtRun\include\opencv2
#LIBS += D:\OpenCVtoQtRun\lib\libopencv_*.a
INCLUDEPATH += D:\opencvtoqt5\include
               D:\opencvtoqt5\include\opencv
               D:\opencvtoqt5\include\opencv2
LIBS += D:\opencvtoqt5\lib\libopencv_*.a
