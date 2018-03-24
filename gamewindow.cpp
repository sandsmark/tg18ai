#include "gamewindow.h"

#include "player.h"

#include <tacopie/utils/error.hpp>
#include <chrono>


GameWindow::GameWindow()
{
    try {
        m_tcpServer.start("127.0.0.1", 1337, [=] (const std::shared_ptr<tcp_client>& client) -> bool {
            std::cout << "New client" << std::endl;
            return this->onNewClient(client);
        });
    } catch (const tacopie::tacopie_error &error) {
        cerr << "error when listening: " << error.what() << endl;
        Backend::get()->quit();
        return;
    }
}

GameWindow::~GameWindow()
{
    m_tcpServer.stop(true, true);
}

Node *GameWindow::build()
{
    const char *fontFile = "Perfect_Dark_Zero.ttf";
    m_font = new GlyphContext(fontFile);
    if (!m_font->isValid()) {
        cerr << "Failed to load '" << fontFile << endl;
    }

    assert(m_font->isValid());

    Node *root = Node::create();

    const int width = size().x;
    const int height = size().y;

    rand();
    const int rectCount = (rand() % 10) + 5;
    for (int i=0; i<rectCount; i++) {
        const int rectWidth = (rand() % 200) + 20;
        const int rectHeight = (rand() % 200) + 20;
        rect2d geometry = rect2d::fromXywh(rand() % (width - rectWidth), rand() % (height - rectHeight), rectWidth, rectHeight);
        RectangleNode *rect = RectangleNode::create(geometry, vec4(1, 1, 1, 0.3));
        *root << rect;
        m_rectangles.push_back(geometry);
    }

    m_players.push_back(make_shared<Player>(vec4(1, .5, .5, 1), this));
    m_players.push_back(make_shared<Player>(vec4(.5, 1, .5, 1), this));
    m_players.push_back(make_shared<Player>(vec4(.5, .5, 1, 1), this));

    for (shared_ptr<Player> player : m_players) {
        *root << player.get();
    }


    return root;
}

void GameWindow::onEvent(Event *event)
{
//    static std::chrono::steady_clock clock;
//    static std::chrono::steady_clock::time_point last_update = clock.now();
//    std::cout << chrono::duration_cast<chrono::milliseconds>(last_update - clock.now()).count() << std::endl;

    bool needRender = false;
    for (shared_ptr<Player> player : m_players) {
        needRender = player->handleEvent(event) || needRender;

        player->sendUpdate(json::JSON());
    }

    if (needRender) {
        requestRender();
    }
}

shared_ptr<Player> GameWindow::getPlayerAt(vec2 position)
{
    for (shared_ptr<Player> player : m_players) {
        if (!player->isAlive()) {
            continue;
        }

        if (player->geometry().contains(position)) {
            return player;
        }
    }

    return nullptr;
}

bool GameWindow::onNewClient(std::shared_ptr<tcp_client> client)
{
    for (shared_ptr<Player> player : m_players) {
        if (player->isActive()) {
            continue;
        }
        player->setTcpConnection(client);
        return true;
    }

    cerr << "Unable to find free player" << endl;
    return false;
}

vector<vec2> GameWindow::playerPositions(Player *exceptPlayer) const
{
    vector<vec2> positions;
    for (shared_ptr<Player> player : m_players) {
        if (player.get() == exceptPlayer) {
            continue;
        }
        if (!player->isAlive()) {
            continue;
        }
        positions.push_back(player->geometry().center());
    }

    return positions;
}

void GameWindow::onBeforeRender()
{
}

void GameWindow::onTick()
{
    for (shared_ptr<Player> player : m_players) {
        player->update();
    }
}

bool GameWindow::isInside(const vec2 &position) const
{
    for (const rect2d &rectangle : m_rectangles) {
        if (rectangle.contains(position)) {
            return true;
        }
    }

    return false;
}
