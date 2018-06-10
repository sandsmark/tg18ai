SOURCES += \
           $$PWD/tacopie/sources/utils/error.cpp \
           $$PWD/tacopie/sources/utils/logger.cpp \
           $$PWD/tacopie/sources/utils/thread_pool.cpp \
           $$PWD/tacopie/sources/network/io_service.cpp \
           $$PWD/tacopie/sources/network/tcp_client.cpp \
           $$PWD/tacopie/sources/network/tcp_server.cpp \
           $$PWD/tacopie/sources/network/common/tcp_socket.cpp

win32 {
    DEFINES += _WIN32_WINNT=0x0600
    LIBS += -lws2_32

    DEFINES += WIN32_LEAN_AND_MEAN _UNICODE _UNICODE
    SOURCES += \
               $$PWD/tacopie/sources/network/windows/windows_self_pipe.cpp \
               $$PWD/tacopie/sources/network/windows/windows_tcp_socket.cpp
} else {
    SOURCES += \
               $$PWD/tacopie/sources/network/unix/unix_self_pipe.cpp \
               $$PWD/tacopie/sources/network/unix/unix_tcp_socket.cpp
}

INCLUDEPATH += $$PWD/tacopie/includes/
