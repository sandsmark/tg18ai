#include "gamewindow.h"

#include "player.h"

#include <tacopie/utils/error.hpp>
#include <unordered_map>
#include <chrono>


GameWindow::GameWindow() :
    m_gameRunning(true)
{
    m_nextUpdate = m_clock.now();

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

    m_gameRunning = true;

    return root;
}

void GameWindow::onEvent(Event *event)
{
    if (event->type() == Event::KeyDown ) {
        KeyEvent *keyEvent = KeyEvent::from(event);
        if (keyEvent->keyCode() == KeyEvent::Key_Q) {
            Backend::get()->quit();
            return;
        }
    }
    if (!m_gameRunning) {
        return;
    }

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
    if (!m_gameRunning) {
        return false;
    }

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

vector<shared_ptr<Player> > GameWindow::players(const int exceptPlayer) const
{
    vector<shared_ptr<Player>> ret;
    for (shared_ptr<Player> player : m_players) {
        if (player->id == exceptPlayer) {
            continue;
        }
        if (!player->isAlive()) {
            continue;
        }
        ret.push_back(player);
    }

    return ret;
}

void GameWindow::onBeforeRender()
{
}

void GameWindow::onTick()
{
    if (!m_gameRunning) {
        return;
    }

    if (m_clock.now() < m_nextUpdate) {
        return;
    }
    m_nextUpdate = m_clock.now() + 20ms;

    vector<shared_ptr<Player>> playersAlive;
    // Collect current states
    unordered_map<int, json::JSON> states;
    for (shared_ptr<Player> player : m_players) {
        if (!player->isAlive()) {
            continue;
        }

        playersAlive.push_back(player);

        player->update();
        states[player->id] = player->serializeState();
    }

    if (playersAlive.empty()) {
        std::cout << "no players alive" << std::endl;
        handleDraw();
        return;
    }

    if (playersAlive.size() == 1) {
        std::cout << "player won" << std::endl;
        handleWinner(playersAlive[0]);
        return;
    }

    for (shared_ptr<Player> player : m_players) {
        json::JSON others = json::Array();

        for (shared_ptr<Player> other : m_players) {
            if (other->id == player->id) {
                continue;
            }
            others.append(other->serializeState());
        }
        json::JSON worldState;
        worldState["others"] = move(others);
        player->sendUpdate(worldState);
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

void GameWindow::handleGameOver()
{
    m_gameRunning = false;
    for (shared_ptr<Player> player : m_players) {
        player->closeConnection();
    }
}

void GameWindow::handleDraw()
{
    handleGameOver();

}

void GameWindow::handleWinner(shared_ptr<Player> winner)
{
    handleGameOver();
}
