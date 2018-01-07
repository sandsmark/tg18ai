#ifndef WINDOW_H
#define WINDOW_H

#include "polygonnode.h"
#include "rengine.h"

class Player;

using namespace rengine;
using namespace std;

class GameWindow : public rengine::StandardSurface
{
public:
    rengine::Node *build() override;
    void onEvent(Event *event) override;
    const vector<rect2d> &rectangles() const { return m_rectangles; }

private:
    vector<rect2d> m_rectangles;
    vector<shared_ptr<Player>> m_players;
};

#endif // WINDOW_H
