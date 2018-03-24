#ifndef WINDOW_H
#define WINDOW_H

#include "polygonnode.h"

#include "rengine.h"

#include <SimpleJSON/json.hpp>
#include <tacopie/network/tcp_server.hpp>

class Player;


using tacopie::tcp_client;
using tacopie::tcp_server;

using namespace rengine;
using namespace std;

class GameWindow : public rengine::StandardSurface
{
public:
    GameWindow();
    ~GameWindow();

    rengine::Node *build() override;
    void onEvent(Event *event) override;
    const vector<rect2d> &rectangles() const { return m_rectangles; }

    shared_ptr<Player> getPlayerAt(vec2 position);

    bool onNewClient(std::shared_ptr<tacopie::tcp_client> client);

    GlyphContext *font() const { return m_font; }

    vector<vec2> playerPositions(Player *exceptPlayer) const;

    void onBeforeRender() override;
    void onTick() override;

    bool isInside(const vec2 &position) const;

private:
    vector<rect2d> m_rectangles;
    vector<shared_ptr<Player>> m_players;
    tcp_server m_tcpServer;
    GlyphContext *m_font = nullptr;
};

#endif // WINDOW_H
