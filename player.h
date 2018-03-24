#ifndef PLAYER_H
#define PLAYER_H

#include "rengine.h"

#include <tacopie/network/tcp_client.hpp>
#include <SimpleJSON/json.hpp>

class PolygonNode;
class GameWindow;
class Player;

using namespace rengine;
using namespace std;

using tacopie::tcp_client;

typedef Animation<RectangleNodeBase, float, &RectangleNodeBase::setX, &AnimationCurves::linear> RectangleXAnimation;
typedef Animation<RectangleNodeBase, float, &RectangleNodeBase::setY, &AnimationCurves::linear> RectangleYAnimation;

class Player : public Node
{
public:
    const int id;

    Player() = delete;
    Player(const Player &) = delete;

    Player(const vec4 color, GameWindow *world);
    ~Player();

    Node *node();

    bool handleEvent(Event *event);

    bool handleCommand(const string &command, const vector<string> &arguments);

    GameWindow *world() { return m_world; }

    vec2 position() const { return m_position; }
    rect2d geometry() const;

    void reset();
    void die();
    void setTcpConnection(shared_ptr<tcp_client> conn);

    bool isActive() const;
    bool isAlive() const;

    void sendUpdate(const json::JSON &worldState) const;

    json::JSON serializeState() const;

    void update();

    void setName(const string &name);

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
    string m_networkBuffer;
    bool m_dead = false;

    mutex m_commandMutex;
    string m_command;
    vector<string> m_arguments;


    TextureNode *m_nameNode;
    shared_ptr<RectangleXAnimation> m_xAnimation;
    shared_ptr<RectangleYAnimation> m_yAnimation;
    vector<vec2> m_visiblePlayers;
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
