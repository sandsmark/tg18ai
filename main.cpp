#include "gamewindow.h"

#include "player.h"

#define  STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <iostream>

RENGINE_DEFINE_GLOBALS

RENGINE_ALLOCATION_POOL_DEFINITION(Bullet, BulletNode);

int main(int argc, char **argv) {
    RENGINE_ALLOCATION_POOL(Bullet, BulletNode, 1024);
    RENGINE_ALLOCATION_POOL(rengine::TransformNode, rengine_TransformNode, 256);
    RENGINE_ALLOCATION_POOL(rengine::SimplifiedTransformNode, rengine_SimplifiedTransformNode, 256);
    RENGINE_ALLOCATION_POOL(rengine::RectangleNode, rengine_RectangleNode, 256);
    RENGINE_ALLOCATION_POOL(rengine::TextureNode, rengine_TextureNode, 256);
    RENGINE_ALLOCATION_POOL(rengine::OpacityNode, rengine_OpacityNode, 32);
    RENGINE_ALLOCATION_POOL(rengine::ColorFilterNode, rengine_ColorFilterNode, 8);
    RENGINE_ALLOCATION_POOL(rengine::BlurNode, rengine_BlurNode, 8);
    RENGINE_ALLOCATION_POOL(rengine::ShadowNode, rengine_ShadowNode, 8);

    return RENGINE_NAMESPACE_PREFIX rengine_main<GameWindow>(argc, argv);
}
