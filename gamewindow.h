#ifndef WINDOW_H
#define WINDOW_H

#include "polygonnode.h"

#include "rengine.h"

#include <SimpleJSON/json.hpp>
#include <tacopie/network/tcp_server.hpp>
#include <chrono>

class Player;


using tacopie::tcp_client;
using tacopie::tcp_server;

using namespace rengine;
using namespace std;
using namespace std::chrono_literals;

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

    vector<shared_ptr<Player>> players(const int exceptPlayer) const;

    void onBeforeRender() override;
    void onTick() override;

    bool isInside(const vec2 &position) const;

private:
    void handleGameOver();
    void handleDraw();
    void handleWinner(shared_ptr<Player> winner);

    vector<rect2d> m_rectangles;
    vector<shared_ptr<Player>> m_players;
    tcp_server m_tcpServer;
    GlyphContext *m_font = nullptr;
    chrono::steady_clock m_clock;
    chrono::steady_clock::time_point m_nextUpdate;
    bool m_gameRunning;

    RectangleNode *m_overlay;
    TextureNode *m_overlayText;
};

#endif // WINDOW_H
