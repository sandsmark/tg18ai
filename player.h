#ifndef PLAYER_H
#define PLAYER_H

#include "rengine.h"

class PolygonNode;
class GameWindow;
class Player;

using namespace rengine;
using namespace std;

typedef Animation<RectangleNodeBase, float, &RectangleNodeBase::setX> RectangleXAnimation;
typedef Animation<RectangleNodeBase, float, &RectangleNodeBase::setY> RectangleYAnimation;

class Player : Node
{
public:
    Player(const vec4 color, GameWindow *world);

    Node *node();

    bool handleEvent(Event *event);

    GameWindow *world() { return m_world; }

    vec2 position() const { return m_position; }

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
    vec4 m_color;
};

class Bullet : public RectangleNode
{
public:
    Bullet();

    RENGINE_ALLOCATION_POOL_DECLARATION(Bullet, BulletNode);

    void setOwner(Player *owner);
    void setTarget(vec2 target);
    void start();

    static Bullet *create(Player *owner, vec2 target, vec4 color) {
        Bullet *node = create();
        node->setOwner(owner);
        node->setTarget(target);
        node->setColor(color);
        node->setGeometry(rect2d::fromPosSize(owner->position(), vec2(2, 2)));

        return node;
    }

private:
    Player *m_owner = nullptr;
    GameWindow *m_world = nullptr;
    shared_ptr<RectangleXAnimation> m_xAnimation;
    shared_ptr<RectangleYAnimation> m_yAnimation;
    vec2 m_target;
};

#endif // PLAYER_H
