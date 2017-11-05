#include "gamewindow.h"

Node *GameWindow::build()
{
    Node *root = Node::create();

    const int width = size().x;
    const int height = size().y;

    PolygonNode *polygon =  new PolygonNode(vec4(1., 0.5, 0.5, 0.9));
    polygon->setPoints({{100, 100}, {100, 150}, {150, 100}});
    polygon->setGeometry(rect2d::fromXywh(0, 0, width, height));
    *root << polygon;


    for (int i=0; i<5; i++) {
        const int rectWidth = rand() % 200;
        const int rectHeight = rand() % 200;
        rect2d geometry = rect2d::fromXywh(rand() % (width - rectWidth), rand() % (height - rectHeight), rectWidth, rectHeight);
        RectangleNode *rect = RectangleNode::create(geometry, vec4(1, 1, 1, 0.8));
        *root << rect;
        m_rectangles.push_back(geometry);
    }

    m_posNode = TransformNode::create();
    m_position = vec2(width/2, height/2);
    m_posNode->setMatrix(mat4::translate2D(m_position));
    *root << m_posNode;

    m_rotation = 0;
    m_rotateNode = TransformNode::create();
    m_rotateNode->setMatrix(mat4::rotate2D(m_rotation));
    *m_posNode << m_rotateNode;

    m_playerNode = RectangleNode::create(rect2d::fromXywh(-25, - 25, 50, 50), vec4(0.5, 0.5, 1, 1));
    *m_rotateNode << m_playerNode;

    return root;
}

void GameWindow::onEvent(Event *event)
{
    vec2 requestedPosition = m_position;

    switch(event->type()) {
    case Event::PointerMove:
        m_cursorPosition = PointerEvent::from(event)->position();
        break;
    case Event::KeyDown: {
        KeyEvent *keyEvent = KeyEvent::from(event);
        switch(keyEvent->keyCode()) {
        case KeyEvent::Key_Up:
            requestedPosition.y -= 10;
            break;
        case KeyEvent::Key_Down:
            requestedPosition.y += 10;
            break;
        case KeyEvent::Key_Left:
            requestedPosition.x -= 10;
            break;
        case KeyEvent::Key_Right:
            requestedPosition.x += 10;
            break;
        case KeyEvent::Key_Escape:
            Backend::get()->quit();
            return;
        default:
            return;
        }
        break;
    }

    case Event::KeyUp: {
        std::cout << "key up: " << KeyEvent::from(event)->keyCode() << std::endl;
        break;
    }
    default:
        return;
    }

    requestedPosition.x = std::min(requestedPosition.x, size().x);
    requestedPosition.x = std::max(requestedPosition.x, 0.f);
    requestedPosition.y = std::min(requestedPosition.y, size().y);
    requestedPosition.y = std::max(requestedPosition.y, 0.f);
    mat4 translationMatrix = mat4::translate2D(requestedPosition.x, requestedPosition.y);

    rect2d testRect = m_playerNode->geometry();

    const float dx = m_cursorPosition.x - m_position.x;
    const float dy = m_cursorPosition.y - m_position.y;
    float rotation = atan2(dy, dx);
    const mat4 rotationMatrix = mat4::rotate2D(rotation);

    vec2 topLeft = testRect.tl;
    vec2 bottomRight = testRect.br;
    vec2 bottomLeft = vec2(topLeft.x, bottomRight.y);
    vec2 topRight = vec2(bottomRight.x, topLeft.y);

    topLeft     = rotationMatrix * topLeft;
    topRight    = rotationMatrix * topRight;
    bottomLeft  = rotationMatrix * bottomLeft;
    bottomRight = rotationMatrix * bottomRight;

    topLeft     = translationMatrix * topLeft;
    topRight    = translationMatrix * topRight;
    bottomLeft  = translationMatrix * bottomLeft;
    bottomRight = translationMatrix * bottomRight;

    for (const rect2d &block : m_rectangles) {
        if (block.contains(topLeft)) {
            return;
        }
        if (block.contains(topRight)) {
            return;
        }
        if (block.contains(bottomLeft)) {
            return;
        }
        if (block.contains(bottomRight)) {
            return;
        }
    }

    m_position = requestedPosition;
    m_rotation = rotation;

    m_posNode->setMatrix(mat4::translate2D(m_position));
    m_rotateNode->setMatrix(mat4::rotate2D(m_rotation));

    requestRender();
}
