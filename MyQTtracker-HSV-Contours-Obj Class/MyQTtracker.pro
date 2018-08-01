#-------------------------------------------------
#
# Project created by QtCreator 2014-06-20T04:23:48
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyQTtracker
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    my_qlabel.cpp \
    trackingobj.cpp \
    obj.cpp

HEADERS  += dialog.h \
    my_qlabel.h \
    trackingobj.h \
    obj.h

FORMS    += dialog.ui

INCLUDEPATH += C:\\OpenCV-2.4.9\\opencv\\build\\include1
LIBS += -LC:\\OpenCV-2.4.9\\myBuild\\lib\\Debug \
    -lopencv_core249d \
    -lopencv_highgui249d \
    -lopencv_imgproc249d \
    -lopencv_features2d249d \
    -lopencv_calib3d249d \
    -lopencv_contrib249d \
    -lopencv_flann249d \
    -lopencv_gpu249d \
    -lopencv_haartraining_engined \
    -lopencv_legacy249d \
    -lopencv_ml249d \
    -lopencv_nonfree249d \
    -lopencv_objdetect249d \
    -lopencv_ocl249d \
    -lopencv_photo249d \
    -lopencv_stitching249d \
    -lopencv_superres249d \
    -lopencv_ts249d \
    -lopencv_video249d \
    -lopencv_videostab249d
