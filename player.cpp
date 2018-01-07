#include "player.h"
#include "gamewindow.h"

Player::Player(const vec4 color, GameWindow *world) :
    m_world(world)
{
    m_rootNode = Node::create();


    vec4 polygonColor = color;
    polygonColor.w = 0.3;
    m_polygon =  new PolygonNode(polygonColor);
    *m_rootNode << m_polygon;

    m_posNode = TransformNode::create();
    int wwidth = world->size().x;
    int wheight = world->size().y;
    m_position = vec2(rand() % wwidth / 2 + wwidth/4, rand() % wheight/2 + wheight/4);
    m_posNode->setMatrix(mat4::translate2D(m_position));
    *m_rootNode << m_posNode;

    m_rotation = 0;
    m_rotateNode = TransformNode::create();
    m_rotateNode->setMatrix(mat4::rotate2D(m_rotation));
    *m_posNode << m_rotateNode;

    m_playerNode = RectangleNode::create(rect2d::fromXywh(-5, -5, 10, 10), color);
    *m_rotateNode << m_playerNode;

    updateVisibility();
    m_polygon->setGeometry(rect2d::fromXywh(0, 0, m_world->size().x, m_world->size().y));
}

Node *Player::node()
{
    return m_rootNode;
}

bool Player::handleEvent(Event *event)
{
    vec2 requestedPosition = m_position;

    int horizontal = 0;
    int vertical = 0;
    switch(event->type()) {
    case Event::PointerMove:
        m_cursorPosition = PointerEvent::from(event)->position();
        break;
    case Event::KeyDown: {
        KeyEvent *keyEvent = KeyEvent::from(event);
        switch(keyEvent->keyCode()) {
        case KeyEvent::Key_Up:
            vertical = 10;
            break;
        case KeyEvent::Key_Down:
            vertical = -10;
            break;
        case KeyEvent::Key_Left:
            horizontal = -10;
            break;
        case KeyEvent::Key_Right:
            horizontal = 10;
            break;
        case KeyEvent::Key_Escape:
            Backend::get()->quit();
            return false;
        case KeyEvent::Key_R:
            m_position = vec2(rand() % int(m_world->size().x / 2) + m_world->size().y/4, rand() % int(m_world->size().x/2) + m_world->size().y/4);
            m_posNode->setMatrix(mat4::translate2D(m_position));
            return true;
        default:
            return false;
        }
        break;
    }

    case Event::KeyUp: {
        std::cout << "key up: " << KeyEvent::from(event)->keyCode() << std::endl;
        break;
    }
    default:
        return false;
    }

    const float dx = m_cursorPosition.x - m_position.x;
    const float dy = m_cursorPosition.y - m_position.y;
    float rotation = atan2(dy, dx);

    if (vertical) {
        requestedPosition.x += cos(rotation) * vertical;
        requestedPosition.y += sin(rotation) * vertical;
    } else if (horizontal) {
        requestedPosition.x += cos(rotation + M_PI_2) * horizontal;
        requestedPosition.y += sin(rotation + M_PI_2) * horizontal;
    }

    requestedPosition.x = std::min(requestedPosition.x, m_world->size().x);
    requestedPosition.x = std::max(requestedPosition.x, 0.f);
    requestedPosition.y = std::min(requestedPosition.y, m_world->size().y);
    requestedPosition.y = std::max(requestedPosition.y, 0.f);
    mat4 translationMatrix = mat4::translate2D(requestedPosition.x, requestedPosition.y);

    rect2d testRect = m_playerNode->geometry();
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

//    for (const rect2d &block : m_rectangles) {
//        if (block.contains(topLeft)) {
//            return false;
//        }
//        if (block.contains(topRight)) {
//            return false;
//        }
//        if (block.contains(bottomLeft)) {
//            return false;
//        }
//        if (block.contains(bottomRight)) {
//            return false;
//        }
//    }

    if (requestedPosition == m_position && rotation == m_rotation) {
        return false;
    }

    m_position = requestedPosition;
    m_rotation = rotation;

    m_posNode->setMatrix(mat4::translate2D(m_position));
    m_rotateNode->setMatrix(mat4::rotate2D(m_rotation));
    updateVisibility();

    return true;
}

struct Line {
    Line(vec2 p1, vec2 p2) : a(p1), b(p2), dx(p2.x - p1.x), dy(p2.y - p1.y), magnitude(hypot(dx, dy)) { }

    Line() = default;

    const vec2 a;
    const vec2 b;

    const float dx = 0;
    const float dy = 0;
    const float magnitude = 0;

    float startAngle = 0;
    float endAngle = 0;

    struct Intersection
    {
        Intersection() = default;
        Intersection(float x_, float y_, float d) : x(x_), y(y_), distance(d), valid(true) {}

        operator bool() const { return valid; }
        operator vec2() const { return vec2(x, y); }
        bool operator<(const Intersection &other) const { return distance < other.distance; }

    private:
        float x = 0, y = 0, distance = 0.;
        bool valid = false;
    };

    Intersection intersection(const Line &other) const {
        // check if we're parallel
        if (dx / magnitude == other.dx / other.magnitude &&
            dy / magnitude == other.dy / other.magnitude) {
            return Intersection();
        }

        const float otherLength = (dx*(other.a.y - a.y) + dy * (a.x - other.a.x)) / (other.dx*dy - other.dy*dx);
        const float distance = (other.a.x + other.dx * otherLength-a.x) / dx;

        if (distance < 0) {
            return Intersection();
        }

        if (otherLength < 0 || otherLength > 1) {
            return Intersection();
        }

        return Intersection(a.x + dx * distance, a.y + dy * distance, distance);
    }
};

void Player::updateVisibility()
{
    vec2 playerCenter = m_posNode->matrix() * m_playerNode->geometry().center();

    vector<rect2d> rectangles = m_world->rectangles();
    rectangles.push_back(rect2d(0, 0, m_world->size().x, m_world->size().y)); // add borders of the map

    vector<float> angles;
    vector<Line> segments;
    for (const rect2d &block : rectangles) {
        const vec2 topLeft (block.tl);
        float angle = atan2(topLeft.y - playerCenter.y, topLeft.x - playerCenter.x);
        angles.push_back(angle - 0.0001);
        angles.push_back(angle);
        angles.push_back(angle + 0.0001);


        const vec2 bottomRight (block.br);
        angle = atan2(bottomRight.y - playerCenter.y, bottomRight.x - playerCenter.x);
        angles.push_back(angle - 0.0001);
        angles.push_back(angle);
        angles.push_back(angle + 0.0001);

        const vec2 bottomLeft (vec2(topLeft.x, bottomRight.y));
        angle = atan2(bottomLeft.y - playerCenter.y, bottomLeft.x - playerCenter.x);
        angles.push_back(angle - 0.0001);
        angles.push_back(angle);
        angles.push_back(angle + 0.0001);

        const vec2 topRight (vec2(bottomRight.x, topLeft.y));
        angle = atan2(topRight.y - playerCenter.y, topRight.x - playerCenter.x);
        angles.push_back(angle - 0.0001);
        angles.push_back(angle);
        angles.push_back(angle + 0.0001);

        segments.push_back(Line(topLeft, topRight));
        segments.push_back(Line(bottomLeft, bottomRight));
        segments.push_back(Line(topLeft, bottomLeft));
        segments.push_back(Line(topRight, bottomRight));
    }

    std::sort(angles.begin(), angles.end());

    vector<vec2> points;
    points.push_back(playerCenter);
    for (const float angle : angles) {
        Line ray(playerCenter, vec2(playerCenter.x + cos(angle), playerCenter.y + sin(angle)));

        Line::Intersection closestIntersection;
        for (const Line &segment : segments) {
            const Line::Intersection intersection = ray.intersection(segment);

            if (!intersection) {
                continue;
            }

            if (intersection < closestIntersection || !closestIntersection) {
                closestIntersection = intersection;
            }
        }

        if (!closestIntersection) {
            continue;
        }

        points.push_back(closestIntersection);
    }

    if (points.size() < 3) {
        return;
    }

    points.push_back(points[1]); // complete it
    m_polygon->setPoints(points);
}
