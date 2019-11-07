#include "gamewindow.h"

#include "player.h"

#define  STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <iostream>

extern "C" {
#include <signal.h>
}

#ifdef _WIN32
#include <winsock2.h>
#endif//_WIN32


RENGINE_DEFINE_GLOBALS

RENGINE_ALLOCATION_POOL_DEFINITION(Bullet, BulletNode);

void sigintHandler(int)
{
    cout << "quitting gracefully" << endl;

    Backend::get()->quit();
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
#ifdef _WIN32
    //! Windows netword DLL init
    WORD version = MAKEWORD(2, 2);
    WSADATA data;

    if (WSAStartup(version, &data) != 0) {
        std::cerr << "WSAStartup() failure" << std::endl;
        return -1;
    }
#endif//_WIN32

    RENGINE_ALLOCATION_POOL(Bullet, BulletNode, 1024);
    RENGINE_ALLOCATION_POOL(rengine::TransformNode, rengine_TransformNode, 256);
    RENGINE_ALLOCATION_POOL(rengine::SimplifiedTransformNode, rengine_SimplifiedTransformNode, 256);
    RENGINE_ALLOCATION_POOL(rengine::RectangleNode, rengine_RectangleNode, 256);
    RENGINE_ALLOCATION_POOL(rengine::TextureNode, rengine_TextureNode, 256);
    RENGINE_ALLOCATION_POOL(rengine::OpacityNode, rengine_OpacityNode, 32);
    RENGINE_ALLOCATION_POOL(rengine::ColorFilterNode, rengine_ColorFilterNode, 8);
    RENGINE_ALLOCATION_POOL(rengine::BlurNode, rengine_BlurNode, 8);
    RENGINE_ALLOCATION_POOL(rengine::ShadowNode, rengine_ShadowNode, 8);


    RENGINE_BACKEND backend;

    GameWindow window;
    window.show();

    signal(SIGINT, &sigintHandler);

    backend.run();

#ifdef _WIN32
    WSACleanup();
#endif//_WIN32


    return 0;
}
