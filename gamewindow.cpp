#include "gamewindow.h"

#include "player.h"

Node *GameWindow::build()
{
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
    }

    if (needRender) {
        requestRender();
    }
}
