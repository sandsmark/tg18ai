#include "gamewindow.h"

#include "player.h"

#include <tacopie/utils/error.hpp>


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

    for (int i=0; i<10; i++) {
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
        *root << player->node();
    }


    return root;
}

void GameWindow::onEvent(Event *event)
{
    bool needRender = false;
    for (shared_ptr<Player> player : m_players) {
        needRender = player->handleEvent(event) || needRender;

        player->sendUpdate();
        if (needRender) {
            break;
        }
    }

    if (needRender) {
        requestRender();
    }
}

shared_ptr<Player> GameWindow::getPlayerAt(vec2 position)
{
    for (shared_ptr<Player> player : m_players) {
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

void GameWindow::onBeforeRender()
{
    for (shared_ptr<Player> player : m_players) {
        player->update();
    }
}
