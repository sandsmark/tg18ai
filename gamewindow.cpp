#include "gamewindow.h"

#include "player.h"

#include "Perfect_Dark_Zero.ttf.h"

#include <tacopie/utils/error.hpp>
#include <unordered_map>
#include <chrono>


GameWindow::GameWindow() :
    m_gameRunning(true)
{
    m_nextUpdate = m_clock.now();

    try {
        m_tcpServer.start("localhost", 1337, [=] (const std::shared_ptr<tcp_client>& client) -> bool {
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
    m_font = new GlyphContext(resource_Perfect_Dark_Zero_ttf_data);
    if (!m_font->isValid()) {
        cerr << "Failed to load font" << endl;
    }

    assert(m_font->isValid());

    Node *root = Node::create();

    m_blurNode = BlurNode::create(20);
    *root << m_blurNode;

    const int width = size().x;
    const int height = size().y;

    rand();
    const int rectCount = (rand() % 10) + 5;
    for (int i=0; i<rectCount; i++) {
        const int rectWidth = (rand() % 200) + 20;
        const int rectHeight = (rand() % 200) + 20;
        rect2d geometry = rect2d::fromXywh(rand() % (width - rectWidth), rand() % (height - rectHeight), rectWidth, rectHeight);
        RectangleNode *rect = RectangleNode::create(geometry, vec4(1, 1, 1, 0.3));
        *m_blurNode << rect;
        m_rectangles.push_back(geometry);
    }

    m_players.push_back(make_shared<Player>(vec4(1, .6, .6, 1), this));
    m_players.push_back(make_shared<Player>(vec4(.6, 1, .6, 1), this));
    m_players.push_back(make_shared<Player>(vec4(.6, .6, 1, 1), this));

    for (shared_ptr<Player> player : m_players) {
        *m_blurNode << player.get();
    }

    m_overlay = RectangleNode::create(rect2d::fromPosSize(vec2(0, 0), size()), vec4(0.f, 0.f, 0.f, 0.5));
    *root << m_overlay;
    m_overlayText = TextureNode::create();
    *m_overlay << m_overlayText;
    setOverlayText("Press space to start");

    m_gameRunning = false;

    return root;
}

void GameWindow::onEvent(Event *event)
{
    if (event->type() == Event::KeyDown ) {
        KeyEvent *keyEvent = KeyEvent::from(event);
        if (keyEvent->keyCode() == KeyEvent::Key_Q) {
            Backend::get()->quit();
            return;
        } else if (keyEvent->keyCode() == KeyEvent::Key_Space) {
            setGameRunning(!m_gameRunning);
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

void GameWindow::setOverlayText(const string &text)
{
    GlyphTextureJob job(font(), text, Units(this).hugeFont());
    job.onExecute();
    assert(job.textureSize().x > 0 && job.textureSize().y > 0);
    Texture *t = renderer()->createTextureFromImageData(job.textureSize(), Texture::RGBA_32, job.textureData());
    const vec2 size = t->size();
    const vec2 pos(m_overlay->geometry().width()/2 - size.x/2, m_overlay->geometry().height()/2 - size.y / 2);
    m_overlayText->setTexture(t);
    m_overlayText->setGeometry(rect2d::fromPosSize(pos, t->size()));
}

void GameWindow::setGameRunning(const bool running)
{
    if (running == m_gameRunning) {
        return;
    }
    m_gameRunning = running;

    if (m_gameRunning) {
        renderer()->sceneRoot()->remove(m_overlay);
        m_blurNode->setRadius(0);
    } else {
        renderer()->sceneRoot()->append(m_overlay);
        m_blurNode->setRadius(20);
        setOverlayText("Paused, press space to continue");
    }
    requestRender();
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
    std::cout << winner->name() << " won" << std::endl;
    handleGameOver();
}
