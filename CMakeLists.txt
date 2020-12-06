cmake_minimum_required(VERSION 3.6)
project(tg18ai)

set (CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()


################################################################################
#
# Options
#
option(RENGINE_LOG_INFO "LogInfo" ON)
if (RENGINE_LOG_INFO)
    message("LogInfo: enabled")
    add_definitions(-DRENGINE_LOG_INFO)
endif()

option(RENGINE_LOG_DEBUG "LogDebug" OFF)
if (RENGINE_LOG_DEBUG)
    message("LogDebug: enabled")
    add_definitions(-DRENGINE_LOG_DEBUG)
endif()

option(RENGINE_LOG_WARNING "LogWarning" ON)
if (RENGINE_LOG_WARNING)
    message("LogWarning: enabled")
    add_definitions(-DRENGINE_LOG_WARNING)
endif()

option(RENGINE_LOG_ERROR "LogError" ON)
if (RENGINE_LOG_ERROR)
    message("LogError: enabled")
    add_definitions(-DRENGINE_LOG_ERROR)
endif()

option(RENGINE_USE_SDL "SDL Backend" OFF)

################################################################################
#
# Resolving backend stuff
#
if(RENGINE_USE_SFHWC)
    message("-- SFHWC backend")
    add_definitions(-DRENGINE_BACKEND_SFHWC)
    include_directories(# android headers
                        /usr/lib/droid-devel/droid-headers
                        # libhybris headers
                        /usr/include/hybris/hwcomposerwindow
                        /usr/include/hybris/eglplatformcommon)
    set(RENGINE_LIBS ${RENGINE_LIBS}
        -lhardware
        -lhybris-common
        -lhybris-eglplatformcommon
        -lhybris-hwcomposerwindow
        -lsync
        -lEGL
        -lpthread
        -lmtdev)
else() # (RENGINE_USE_SDL)
    message("-- SDL backend")
    include(FindPkgConfig)
    pkg_search_module(SDL2 REQUIRED sdl2)
    include_directories(${SDL2_INCLUDE_DIRS})
    if (LINUX)
        set(RENGINE_LIBS ${RENGINE_LIBS} -lpthread)
    endif()
    set(RENGINE_LIBS ${RENGINE_LIBS} -L/usr/local/lib ${SDL2_LIBRARIES})
    add_definitions(-DRENGINE_BACKEND_SDL)
endif()

find_package(OpenGL COMPONENTS EGL)
find_package(GLEW REQUIRED)

if (OpenGL_EGL_FOUND)
    set(RENGINE_LIBS ${RENGINE_LIBS} OpenGL::EGL GLEW::GLEW)
    add_definitions(-DRENGINE_OPENGL_DESKTOP)
else()
    message(WARNING, "OpenGL was not found, assuming OpenGL ES 2.0 in default locations...")
    set(RENGINE_LIBS ${RENGINE_LIBS} -lGLESv2)
endif()

set(TACOPIE_SOURCES
    extern/tacopie/sources/utils/error.cpp
    extern/tacopie/sources/utils/logger.cpp
    extern/tacopie/sources/utils/thread_pool.cpp
    extern/tacopie/sources/network/io_service.cpp
    extern/tacopie/sources/network/tcp_client.cpp
    extern/tacopie/sources/network/tcp_server.cpp
    extern/tacopie/sources/network/common/tcp_socket.cpp
)
set(WIN_LIBS )
if (WIN32)
    message("Using wintendo")
    set(CMAKE_CXX_FLAGS "-DWIN32_LEAN_AND_MEAN -D_UNICODE ${CMAKE_CXX_FLAGS}")
    set(WIN_LIBS -lws2_32 -lSDL2 -lGLESv2 -lANGLE)
    set(TACOPIE_SOURCES
               extern/tacopie/sources/network/windows/windows_self_pipe.cpp
               extern/tacopie/sources/network/windows/windows_tcp_socket.cpp
               ${TACOPIE_SOURCES}
    )
else()
    set(TACOPIE_SOURCES
               extern/tacopie/sources/network/unix/unix_self_pipe.cpp
               extern/tacopie/sources/network/unix/unix_tcp_socket.cpp
               ${TACOPIE_SOURCES}
    )
endif()

add_subdirectory(tools/resgen)
add_resource(FONT_PERFECT_DARK_ZERO Perfect_Dark_Zero.ttf)

set(APP_SOURCES
    main.cpp
    gamewindow.cpp
    player.cpp
    polygonnode.cpp
    ${FONT_PERFECT_DARK_ZERO}
)

add_executable(tg18ai ${APP_SOURCES} ${TACOPIE_SOURCES})
target_link_libraries(tg18ai ${RENGINE_LIBS} ${WIN_LIBS})
include_directories(extern/rengine/include/ extern/rengine/3rdparty/ extern/tacopie/includes/ extern/ ${PROJECT_BINARY_DIR})

