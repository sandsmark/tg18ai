TEMPLATE = app
CONFIG += console c++17

CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    gamewindow.cpp \
    polygonnode.cpp \

LIBS += -lSDL2 -lGLEW -lGL -lpthread
DEFINES += RENGINE_BACKEND_SDL RENGINE_LOG_WARNING RENGINE_LOG_ERROR RENGINE_OPENGL_DESKTOP
QMAKE_CXXFLAGS +=  -Wno-unused-parameter -std=c++17
INCLUDEPATH += ../rengine/include/ /usr/include/SDL2/


HEADERS += \
    gamewindow.h \
    polygonnode.h \
