TEMPLATE = app
CONFIG += console c++17

CONFIG -= app_bundle
CONFIG -= qt

#CONFIG += sanitizer sanitize_address

SOURCES += main.cpp \
    gamewindow.cpp \
    polygonnode.cpp \
    player.cpp

LIBS += -lSDL2 -lpthread

win32 {
    DEFINES += _USE_MATH_DEFINES
    DEFINES += GLEW_STATIC

    # for SDL
    LIBS += -lmingw32
    LIBS += -lSDL2main -liconv -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid

    # for gl stuff
    LIBS += -lglew32 -lopengl32
} else {
    LIBS += -lGLEW -lGL
}

DEFINES += RENGINE_BACKEND_SDL RENGINE_LOG_WARNING RENGINE_LOG_ERROR RENGINE_OPENGL_DESKTOP
QMAKE_CXXFLAGS +=  -Wno-unused-parameter -std=c++17
INCLUDEPATH += extern/rengine/include/ extern/rengine/3rdparty/ extern/ /usr/include/SDL2/

HEADERS += \
    gamewindow.h \
    polygonnode.h \
    player.h


include(extern/tacopie.pri)
