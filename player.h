#ifndef PLAYER_H
#define PLAYER_H

#include "rengine.h"
#include <set>

#include <tacopie/network/tcp_client.hpp>
#include <SimpleJSON/json.hpp>

class PolygonNode;
class GameWindow;
class Player;
class Bullet;

using namespace rengine;
using namespace std;

using tacopie::tcp_client;

typedef Animation<RectangleNodeBase, float, &RectangleNodeBase::setX, &AnimationCurves::linear> RectangleXAnimation;
typedef Animation<RectangleNodeBase, float, &RectangleNodeBase::setY, &AnimationCurves::linear> RectangleYAnimation;
typedef Animation<TransformNode, float, &TransformNode::setMatrix_x, &AnimationCurves::linear> TransformXAnimation;
typedef Animation<TransformNode, float, &TransformNode::setMatrix_y, &AnimationCurves::linear> TransformYAnimation;

class Player : public Node
{
public:
    static int s_idCounter;

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
    void closeConnection();

    bool isActive() const;
    bool isAlive() const;

    void sendUpdate(const json::JSON &worldState) const;

    json::JSON serializeState() const;

    void update();

    const std::string &name() const { return m_name; }
    void setName(const string &name);

    vector<int> visiblePlayerIds() const;

    void addBullet(Bullet *bullet);
    void removeBullet(Bullet *bullet);

protected:
    void onPreprocess() override;

private:
    void onTcpMessage(const tcp_client::read_result& res);
    void updateVisibility();

    Node *m_rootNode = nullptr;
    TransformNode *m_posNode = nullptr;
    TransformNode *m_rotateNode = nullptr;
    PolygonNode *m_playerNode = nullptr;
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
    shared_ptr<TransformXAnimation> m_xAnimation;
    shared_ptr<TransformYAnimation> m_yAnimation;
    vector<int> m_visiblePlayers;
    set<Bullet*> m_bullets;

    std::shared_ptr<GlyphTextureJob> m_nameJob;

    std::string m_name;
};

class Bullet : public RectangleNode
{
public:
    static int s_idCounter;
    const int id;

    Bullet();
    ~Bullet();

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

    json::JSON serializeState() const;

private:
    Player *m_owner = nullptr;
    GameWindow *m_world = nullptr;
    shared_ptr<RectangleXAnimation> m_xAnimation;
    shared_ptr<RectangleYAnimation> m_yAnimation;
    vec2 m_target;
    bool m_startedInside;
};

#endif // PLAYER_H
