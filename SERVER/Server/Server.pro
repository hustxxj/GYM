QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    source.cpp \
    IDRec.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


CONFIG += console c++11
QMAKE_CXXFLAGS += -std=c++11
OR
CONFIG += c++11  #opencv需要C++11支持
LIBS='-lX11'

INCLUDEPATH += "/home/xuxuejie/OpenCV/opencv3.4.3/envir/include"\
"/home/xuxuejie/OpenCV/opencv3.4.3/envir/include/opencv"\
"/home/xuxuejie/OpenCV/opencv3.4.3/envir/include"\
"/home/xuxuejie/Dlib/dlib-19.18/dlib/external/zlib"\
"/home/xuxuejie/Dlib/dlib-19.18/dlib/external/libjpeg"\
"/home/xuxuejie/Dlib/dlib-19.18/dlib/external"\
"/home/xuxuejie/Dlib/dlib-19.18"\


LIBS += "/home/xuxuejie/OpenCV/opencv3.4.3/envir/build/lib/libopencv_world.so.3.4.3" \
        "/home/xuxuejie/OpenCV/opencv3.4.3/envir/build/lib/libopencv_img_hash.so.3.4.3" \
        "/home/xuxuejie/Dlib/dlib-19.18/build/dlib/libdlib.so.19.18.0" \
        "/home/xuxuejie/Dlib/dlib-19.18/build/dlib/libdlib.so"

DEFINES += DLIB_JPEG_SUPPORT
DEFINES += NDEBUG
DEFINES += _CONSOLE
DEFINES += _CRT_SECURE_NO_WARNINGS

HEADERS += \
    main.h \
    IDRec.h
