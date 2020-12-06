#include "polygonnode.h"

PolygonNode::PolygonNode(vec4 color) :
    m_color(color)
{
    std::vector<const char *> attrsV;
    attrsV.push_back("aV");

    m_shaderProgram.initialize(openglrenderer_vsh_solid(), openglrenderer_fsh_solid(), attrsV);
    m_shaderProgram.matrix = m_shaderProgram.resolve("m");
    m_shaderProgram.color = m_shaderProgram.resolve("color");

    glGenBuffers(1, &m_vertexBuffer);
}

PolygonNode::~PolygonNode()
{
    glDeleteBuffers(1, &m_vertexBuffer);
}

void PolygonNode::render(const mat4 &proj)
{
    if (m_points.empty()) {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

    glUseProgram(m_shaderProgram.id());

    for (int i=0; i<m_shaderProgram.attributeCount(); ++i) {
        glEnableVertexAttribArray(i);
    }

    glUniformMatrix4fv(m_shaderProgram.matrix, 1, true, proj.m);
    glUniform4f(m_shaderProgram.color, m_color.x * m_color.w, m_color.y * m_color.w, m_color.z * m_color.w, m_color.w);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_points.size());

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
}

void PolygonNode::setPoints(const vector<vec2> &points)
{
    m_points = points;

    if (!points.empty()) {
        rect2d boundingBox(points[0], points[0]);

        for (size_t i=1; i<points.size(); i++) {
            boundingBox |= points[i];
        }
        setGeometry(boundingBox);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_points.size() * sizeof(vec2), m_points.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
