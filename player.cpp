#include "player.h"

#include "gamewindow.h"

#include <SimpleJSON/json.hpp>

#include <mutex>

Bullet::Bullet() :
    m_xAnimation(make_shared<RectangleXAnimation>(this)),
    m_yAnimation(make_shared<RectangleYAnimation>(this)),
    m_startedInside(false)
{
}

void Bullet::setOwner(Player *owner)
{
    m_owner = owner;
    m_world = owner->world();
}

void Bullet::setTarget(vec2 target)
{
    m_target = target;

    m_xAnimation->keyFrames().clear();
    m_xAnimation->keyFrames().push_back(KeyFrame<float>(0, m_owner->position().x));
    m_xAnimation->keyFrames().push_back(KeyFrame<float>(1, target.x));

    m_yAnimation->keyFrames().clear();
    m_yAnimation->keyFrames().push_back(KeyFrame<float>(0, m_owner->position().y));
    m_yAnimation->keyFrames().push_back(KeyFrame<float>(1, target.y));
}

void Bullet::start()
{
    m_startedInside = m_world->isInside(position());

    float distance = hypot(position().x - m_target.x, position().y - m_target.y);

    m_xAnimation->setIterations(1);
    m_xAnimation->setDuration(distance / 1000.);

    m_yAnimation->setIterations(1);
    m_yAnimation->setDuration(distance / 1000.);

    m_xAnimation->onCompleted.connect(m_xAnimation.get(), [=](){
        if (m_yAnimation->isRunning()) {
            return;
        }
        destroy();
    });

    m_yAnimation->onCompleted.connect(m_yAnimation.get(), [=](){
        if (m_xAnimation->isRunning()) {
            return;
        }
        destroy();
    });

    onXChanged.connect(this, [=]() { checkHit(); });
    onYChanged.connect(this, [=]() { checkHit(); });

    m_world->animationManager()->start(m_xAnimation);
    m_world->animationManager()->start(m_yAnimation);

}

void Bullet::checkHit()
{
    assert(m_world);
    assert(m_owner);

    if (m_world->isInside(position()) != m_startedInside) {
        m_xAnimation->requestStop();
        m_yAnimation->requestStop();
        setColor(vec4(0, 0, 0, 0));
        return;
    }

    shared_ptr<Player> target = m_world->getPlayerAt(position());
    if (!target || target.get() == m_owner) {
        return;
    }

    target->die();
    m_xAnimation->requestStop();
    m_yAnimation->requestStop();
}

static int s_idCounter = 0;

Player::Player(const vec4 color, GameWindow *world) :
    id(s_idCounter++),
    m_world(world),
    m_color(color)
{
    m_rootNode = Node::create();
    *this << m_rootNode;

    vec4 polygonColor = color;
    polygonColor.w = 0.1;
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

    m_playerNode = RectangleNode::create(rect2d::fromXywh(-10, -10, 20, 20), color);
    *m_rotateNode << m_playerNode;

    m_xAnimation = make_shared<TransformXAnimation>(m_posNode);
    m_xAnimation->setIterations(1);
    m_xAnimation->setDuration(0.5);
    m_yAnimation = make_shared<TransformYAnimation>(m_posNode);
    m_yAnimation->setIterations(1);
    m_yAnimation->setDuration(0.5);

    m_polygon->setGeometry(rect2d::fromXywh(0, 0, m_world->size().x, m_world->size().y));

    m_nameNode = TextureNode::create();
    *m_posNode << m_nameNode;
    setName("Bot " + to_string(id));
}

Player::~Player()
{
    if (m_tcpConnection && m_tcpConnection->is_connected()) {
        m_tcpConnection->disconnect(true);
    }
}

Node *Player::node()
{
    return m_rootNode;
}

bool Player::handleEvent(Event *event)
{
    if (m_dead) {
        return false;
    }

    // check if we're remote controlled
    if (isActive()) {
        return false;
    }

    string command;
    vector<string> arguments;

    switch(event->type()) {
    case Event::PointerMove: {
        vec2 cursorPos = PointerEvent::from(event)->position();
        command = "POINT_AT";
        arguments.push_back(to_string(cursorPos.x));
        arguments.push_back(to_string(cursorPos.y));
        break;
    }
    case Event::PointerDown: {
        command = "FIRE";
        break;
    }
    case Event::KeyDown: {
        KeyEvent *keyEvent = KeyEvent::from(event);
        switch(keyEvent->keyCode()) {
        case KeyEvent::Key_Up:
            command = "FORWARD";
            break;
        case KeyEvent::Key_Down:
            command = "BACKWARD";
            break;
        case KeyEvent::Key_Left:
            command = "STRAFE_LEFT";
            break;
        case KeyEvent::Key_Right:
            command = "STRAFE_RIGHT";
            break;
        case KeyEvent::Key_Escape:
            Backend::get()->quit();
            return false;
        case KeyEvent::Key_R:
            m_position = vec2(rand() % int(m_world->size().x / 2) + m_world->size().y/4, rand() % int(m_world->size().x/2) + m_world->size().y/4);
            m_posNode->setMatrix(mat4::translate2D(m_position));
            m_playerNode->setColor(m_color);
            reset();
            return true;
        default:
            return false;
        }
        break;
    }
    default:
        return false;
    }

    return handleCommand(command, arguments);
}

bool Player::handleCommand(const string &command, const vector<string> &arguments)
{
    if (m_dead) {
        return false;
    }

    vec2 requestedPosition = m_position;

    int horizontal = 0;
    int vertical = 0;

    if (command == "NAME") {
        if (arguments.size() != 1) {
            cerr << "no name given to name command" << endl;
            return false;
        }
        setName(arguments[0]);
    } else if (command == "POINT_AT") {
        if (arguments.size() != 2) {
            cerr << "Invalid POINT_AT, no coordinates";
            return false;
        }

        m_cursorPosition.x = stof(arguments[0]);
        m_cursorPosition.y = stof(arguments[1]);
    } else if (command == "FIRE") {
        Bullet *bullet = Bullet::create(this, m_cursorPosition, m_color);
        *m_rootNode << bullet;
        bullet->start();
        return true;
    } else if (command == "STRAFE_LEFT") {
        horizontal = -10;
    } else if (command == "STRAFE_RIGHT") {
        horizontal = 10;
    } else if (command == "FORWARD") {
        vertical = 10;
    } else if (command == "BACKWARD") {
        vertical = -10;
    } else {
        cerr << "unknown command '" << command << "'" << endl;
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

    if (requestedPosition == m_position && rotation == m_rotation) {
        return false;
    }


    m_xAnimation->keyFrames().clear();
    m_xAnimation->keyFrames().push_back(KeyFrame<float>(0, m_position.x));
    m_xAnimation->keyFrames().push_back(KeyFrame<float>(1, requestedPosition.x));
    if (!m_xAnimation->isRunning()) {
        m_world->animationManager()->start(m_xAnimation);
    }

    m_yAnimation->keyFrames().clear();
    m_yAnimation->keyFrames().push_back(KeyFrame<float>(0, m_position.y));
    m_yAnimation->keyFrames().push_back(KeyFrame<float>(1, requestedPosition.y));
    if (!m_yAnimation->isRunning()) {
        m_world->animationManager()->start(m_yAnimation);
    }

    m_position = requestedPosition;
    m_rotation = rotation;

    m_rotateNode->setMatrix(mat4::rotate2D(m_rotation));

    return true;
}

rect2d Player::geometry() const
{
    if (m_dead) {
        return rect2d();
    }

    assert(m_playerNode);
    const rect2d orig = m_playerNode->geometry();
    const mat4 matrix = TransformNode::matrixFor(m_playerNode, m_world->renderer()->sceneRoot());
    return rect2d(matrix * orig.tl, matrix * orig.br).normalized();
}

void Player::reset()
{
    if (m_dead) {
        *this << m_rootNode;
    }

    m_dead = false;
}

void Player::die()
{
    if (m_dead) {
        return;
    }

    m_dead = true;
    remove(m_rootNode);
}

void Player::setTcpConnection(shared_ptr<tacopie::tcp_client> conn)
{
    m_tcpConnection = conn;

    if (!conn) {
        cerr << "Handed null connection" << endl;
        return;
    }

    tcp_client::read_request req;
    req.size = 1024;
    req.async_read_callback = [=](const tcp_client::read_result &result) {
        this->onTcpMessage(result);
    };

    m_tcpConnection->async_read(req);
}

bool Player::isActive() const
{
    return m_tcpConnection && m_tcpConnection->is_connected();
}

bool Player::isAlive() const
{
    return !m_dead;
}

void Player::sendUpdate(const json::JSON &worldState) const
{
    if (!m_tcpConnection) {
        return;
    }

    json::JSON message;
    message["type"] = "update";
    message["you"] = serializeState();
    message["world"] = worldState;
    string serialized = message.dump(1, " ", " ") + "\n";

    m_tcpConnection->async_write({vector<char>(serialized.begin(), serialized.end()), nullptr});
}

json::JSON Player::serializeState() const
{
    json::JSON state;

    state["x"] = m_position.x;
    state["y"] = m_position.y;
    state["pointing_at_x"] = m_cursorPosition.x;
    state["pointing_at_y"] = m_cursorPosition.y;
    state["rotation"] = m_rotation;
    state["alive"] = !m_dead;

    return state;
}

void Player::update()
{
    if (!isAlive()) {
        return;
    }

    updateVisibility();

    if (isActive()) {
        m_commandMutex.lock();
        if (!m_command.empty()) {
            handleCommand(m_command, m_arguments);
        }
        m_command.clear();
        m_arguments.clear();
        m_commandMutex.unlock();
    }
}

void Player::setName(const string &name)
{
    std::cout << name << std::endl;
    std::cout << m_world->renderer()->targetSurface()->dpi() << std::endl;
    GlyphTextureJob job(m_world->font(), name, Units(m_world).hugeFont());
    job.onExecute();
    assert(job.textureSize().x > 0 && job.textureSize().y > 0);
    Texture *t = m_world->renderer()->createTextureFromImageData(job.textureSize(), Texture::RGBA_32, job.textureData());
    const vec2 size = t->size();
    const vec2 pos(-size.x/2, 10);
    m_nameNode->setTexture(t);
    m_nameNode->setGeometry(rect2d::fromPosSize(pos, t->size()));

    requestPreprocess();
}

void Player::onTcpMessage(const tcp_client::read_result &res)
{
    if (!res.success) {
        cerr << "Error when reading" << endl;
        m_tcpConnection.reset();
        return;
    }

    // Make sure we read more
    if (m_tcpConnection) {
        tcp_client::read_request req;
        req.size = 1024;
        req.async_read_callback = [=](const tcp_client::read_result &result){
            this->onTcpMessage(result);
        };

        m_tcpConnection->async_read(req);
    }

    m_networkBuffer += std::string(res.buffer.begin(), res.buffer.end());

    string::size_type lastNewline = m_networkBuffer.rfind("\n");
    if (lastNewline == string::npos) {
        // No newline yet, hopefully comes in next packet
        return;
    }


    string::size_type prevNewline = m_networkBuffer.rfind("\n", lastNewline - 1);
    if (prevNewline == string::npos) {
        prevNewline = 0;
    }
    string line = m_networkBuffer.substr(prevNewline, lastNewline);

    vector<string> arguments;
    istringstream stream(line);
    string argument;
    string command;
    while (getline(stream, argument, ' ')) {
        if (command.empty()) {
            command = argument;
        } else {
            arguments.push_back(argument);
        }
    }

    if (arguments.empty()) {
        command = line;
    }

    if (lastNewline == m_networkBuffer.size() - 1) {
        m_networkBuffer.clear();
    } else {
        m_networkBuffer = m_networkBuffer.substr(lastNewline);
    }

    m_commandMutex.lock();
    m_arguments = arguments;
    m_command = command;
    m_commandMutex.unlock();

    requestPreprocess();
    m_world->requestRender();
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

    // Find visible players
    m_visiblePlayers.clear();

    const vector<vec2> otherPlayers = m_world->playerPositions(this);
    for (const vec2 &otherPlayer : otherPlayers) {
        const float angle = atan2(otherPlayer.y - playerCenter.y, otherPlayer.x - playerCenter.x);
        Line ray(playerCenter, vec2(playerCenter.x + cos(angle), playerCenter.y + sin(angle)));

        bool isBlocked = false;
        for (const Line &segment : segments) {
            const Line::Intersection intersection = ray.intersection(segment);

            if (intersection) {
                isBlocked = true;
                break;
            }
        }

        if (!isBlocked) {
            m_visiblePlayers.push_back(otherPlayer);
        }
    }

    // Find visible regions
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
