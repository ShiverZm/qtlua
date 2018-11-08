#-------------------------------------------------
#
# Project created by QtCreator 2018-01-05T17:03:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


QMAKE_CXXFLAGS += -Wno-unused-parameter

SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui




# -------------------------------------------------
# sources for libusb
# -------------------------------------------------
DEFINES += _WIN32_WINNT=0x0500

INCLUDEPATH += ../../libusb-1.0.21 ../../libusb-1.0.21/libusb
HEADERS += ../../libusb-1.0.21/libusb/libusbi.h \
           ../../libusb-1.0.21/libusb/libusb.h \
           ../../libusb-1.0.21/libusb/version.h \
           ../../libusb-1.0.21/libusb/version_nano.h \
           ../../libusb-1.0.21/libusb/os/poll_windows.h \
           ../../libusb-1.0.21/libusb/os/threads_windows.h \
           ../../libusb-1.0.21/libusb/os/windows_common.h \
           ../../libusb-1.0.21/libusb/os/windows_nt_common.h \
           ../../libusb-1.0.21/libusb/os/windows_winusb.h

SOURCES += ../../libusb-1.0.21/libusb/core.c \
           ../../libusb-1.0.21/libusb/descriptor.c \
           ../../libusb-1.0.21/libusb/hotplug.c \
           ../../libusb-1.0.21/libusb/io.c \
           ../../libusb-1.0.21/libusb/strerror.c \
           ../../libusb-1.0.21/libusb/sync.c
win32{
SOURCES += ../../libusb-1.0.21/libusb/os/poll_windows.c \
           ../../libusb-1.0.21/libusb/os/threads_windows.c \
           ../../libusb-1.0.21/libusb/os/windows_nt_common.c \
           ../../libusb-1.0.21/libusb/os/windows_winusb.c
}


# -------------------------------------------------
# sources for qlibusb
# -------------------------------------------------
INCLUDEPATH += ../../qlibusb
HEADERS += ../../qlibusb/qlibusb.h \
           ../../qlibusb/config.h
SOURCES += ../../qlibusb/qlibusb.cpp \
           ../../qlibusb/qlibusb_ex.c
