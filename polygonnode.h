#ifndef POLYGONNODE_H
#define POLYGONNODE_H

#include <rengine.h>

using namespace rengine;
using namespace std;

class PolygonNode : public RenderNode
{
public:
    PolygonNode(vec4 color);
    ~PolygonNode();

    void render() override;

    void setPoints(const vector<vec2> &points);
    void setColor(const vec4 &color) { m_color = color; }

private:
    struct : public OpenGLShaderProgram {
        int matrix;
        int color;
    } m_shaderProgram;

    GLuint m_vertexBuffer;

    vec4 m_color;

    vector<vec2> m_points;
};

#endif // POLYGONNODE_H
