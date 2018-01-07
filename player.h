#ifndef PLAYER_H
#define PLAYER_H

#include "rengine.h"

class PolygonNode;
class GameWindow;

using namespace rengine;
using namespace std;

class Player
{
public:
    Player(const vec4 color, GameWindow *world);

    Node *node();

    bool handleEvent(Event *event);

private:
    void updateVisibility();

    Node *m_rootNode;
    TransformNode *m_posNode;
    TransformNode *m_rotateNode;
    RectangleNode *m_playerNode;
    float m_rotation;
    vec2 m_position;
    vec2 m_cursorPosition;
    PolygonNode *m_polygon;
    GameWindow *m_world;
};

#endif // PLAYER_H
