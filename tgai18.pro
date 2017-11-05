TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    gamewindow.cpp

LIBS += -lSDL2 -lGL -lpthread
DEFINES += RENGINE_BACKEND_SDL RENGINE_LOG_WARNING RENGINE_LOG_ERROR RENGINE_OPENGL_DESKTOP
QMAKE_CXXFLAGS +=  -Wno-unused-parameter
INCLUDEPATH += ../rengine/include/ /usr/include/SDL2/

HEADERS += \
    gamewindow.h
