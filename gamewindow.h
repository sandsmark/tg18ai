#ifndef WINDOW_H
#define WINDOW_H

#include "polygonnode.h"
#include "rengine.h"

using namespace rengine;
using namespace std;

class GameWindow : public rengine::StandardSurface
{
public:
    rengine::Node *build() override;
    void onEvent(Event *event) override;

private:
    void updateVisibility();

    std::vector<rect2d> m_rectangles;

    TransformNode *m_posNode;
    TransformNode *m_rotateNode;
    RectangleNode *m_playerNode;
    float m_rotation;
    vec2 m_position;
    vec2 m_cursorPosition;
    PolygonNode *m_polygon;
};

#endif // WINDOW_H
