#ifndef PLAYER_H
#define PLAYER_H

#include "rengine.h"

#include <tacopie/network/tcp_client.hpp>

class PolygonNode;
class GameWindow;
class Player;

using namespace rengine;
using namespace std;

using tacopie::tcp_client;

typedef Animation<RectangleNodeBase, float, &RectangleNodeBase::setX> RectangleXAnimation;
typedef Animation<RectangleNodeBase, float, &RectangleNodeBase::setY> RectangleYAnimation;

class Player : Node
{
public:
    Player(const vec4 color, GameWindow *world);
    ~Player();

    Node *node();

    bool handleEvent(Event *event);

    bool handleCommand(const string &command, const vector<string> &arguments);

    GameWindow *world() { return m_world; }

    vec2 position() const { return m_position; }
    rect2d geometry() const;

    void die();
    void setTcpConnection(shared_ptr<tcp_client> conn);

    bool isActive() const;

private:
    void onTcpMessage(const tcp_client::read_result& res);
    void updateVisibility();

    Node *m_rootNode = nullptr;
    TransformNode *m_posNode = nullptr;
    TransformNode *m_rotateNode = nullptr;
    RectangleNode *m_playerNode = nullptr;
    float m_rotation;
    vec2 m_position;
    vec2 m_cursorPosition;
    PolygonNode *m_polygon = nullptr;
    GameWindow *m_world = nullptr;
    vec4 m_color;
    shared_ptr<tcp_client> m_tcpConnection;
    vector<char> m_networkBuffer;
};

class Bullet : public RectangleNode
{
public:
    Bullet();

    RENGINE_ALLOCATION_POOL_DECLARATION(Bullet, BulletNode);

    void setOwner(Player *owner);
    void setTarget(vec2 target);
    void start();
    void checkHit();

    static Bullet *create(Player *owner, vec2 target, vec4 color) {
        Bullet *node = create();
        node->setOwner(owner);
        node->setTarget(target);
        node->setColor(color);
        node->setGeometry(rect2d::fromPosSize(owner->position() - vec2(3, 3), vec2(6, 6)));

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
