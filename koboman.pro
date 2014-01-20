# Add more folders to ship with the application, here
folder_01.source = qml/koboman
folder_01.target = qml
folder_02.source = imports/net
folder_02.target = imports
DEPLOYMENTFOLDERS = folder_01 folder_02

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH = imports

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    zbar/decoder.c \
    zbar/decoder/code39.c \
    zbar/decoder/code128.c \
    zbar/decoder/ean.c \
    zbar/decoder/i25.c \
    zbar/scanner.c \
    zbar/img_scanner.c \
    zbar/symbol.c \
    zbar/error.c \
    zbar/image.c \
    zbar/refcnt.c \
    mainwindow.cpp \
    barcodescannerbackend.cpp \
    barcodescanner.cpp \
    barcodescannerthread.cpp \
    barcode.cpp \
    utility.cpp \
    library.cpp \
    booklistmodel.cpp \
    items/toplevelitem.cpp \
    items/textureitem.cpp \
    items/toplevelcontainer.cpp \
    items/itemlistitem.cpp \
    items/textlistitem.cpp \
    items/buttonboxitem.cpp

# Installation path
# target.path =

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()

HEADERS += \
    zbar/decoder.h \
    zbar/zbar.h \
    zbar/decoder/code39.h \
    zbar/decoder/code128.h \
    zbar/decoder/ean.h \
    zbar/decoder/i25.h \
    zbar/img_scanner.h \
    zbar/symbol.h \
    zbar/error.h \
    zbar/image.h \
    zbar/refcnt.h \
    mainwindow.hpp \
    barcodescannerbackend.hpp \
    barcodescanner.hpp \
    barcodescannerthread.hpp \
    barcode.hpp \
    utils.hpp \
    utility.hpp \
    library.hpp \
    booklistmodel.hpp \
    items/toplevelitem.hpp \
    items/textureitem.hpp \
    items/toplevelcontainer.hpp \
    items/itemlistitem.hpp \
    items/textlistitem.hpp \
    items/buttonboxitem.hpp

INCLUDEPATH += zbar

CONFIG += C++11
QT += multimedia multimediawidgets widgets opengl network xml sql

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml \
    imports/net/xylosper/Mobile/qmldir

RESOURCES += \
    resources.qrc
