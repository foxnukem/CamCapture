TEMPLATE = app
TARGET = CamCapture

QT += multimedia multimediawidgets

HEADERS = \
    camera.h \
    settings.h \
    metadatadialog.h

SOURCES = \
    main.cpp \
    camera.cpp \
    settings.cpp \
    metadatadialog.cpp

FORMS += \
    settings.ui

RESOURCES += camera.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/multimedia/camera
INSTALLS += target

QT += widgets
include(../../multimedia/shared/shared.pri)
