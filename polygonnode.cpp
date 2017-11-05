#include "polygonnode.h"

const int vertexCount = 6;

PolygonNode::PolygonNode(vec4 color) :
    m_color(color)
{
    std::vector<const char *> attrsV;
    attrsV.push_back("aV");

    m_shaderProgram.initialize(openglrenderer_vsh_solid(), openglrenderer_fsh_solid(), attrsV);
    m_shaderProgram.matrix = m_shaderProgram.resolve("m");
    m_shaderProgram.color = m_shaderProgram.resolve("color");
    std::cout << m_shaderProgram.matrix << std::endl;
    std::cout << m_shaderProgram.color << std::endl;

    m_vertices = (vec2 *) malloc(vertexCount * sizeof(vec2));
    m_vertices[0] = vec2(100, 100);
    m_vertices[1] = vec2(100, 200);
    m_vertices[2] = vec2(200, 100);

    m_vertices[3] = vec2(200, 200);
    m_vertices[4] = vec2(200, 300);
    m_vertices[5] = vec2(300, 200);

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vec2), m_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::cout << m_vertexBuffer << std::endl;
}

PolygonNode::~PolygonNode()
{
    glDeleteBuffers(1, &m_vertexBuffer);
}

void PolygonNode::render()
{
    if (m_points.empty()) {
        return;
    }

    mat4 proj = mat4::translate2D(-1.0, 1.0)
             * mat4::scale2D(2.0f / geometry().width(), -2.0f / geometry().height());

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

    glUseProgram(m_shaderProgram.id());

    for (int i=0; i<m_shaderProgram.attributeCount(); ++i) {
        glEnableVertexAttribArray(i);
    }

    glUniformMatrix4fv(m_shaderProgram.matrix, 1, true, proj.m);
    glUniform4f(m_shaderProgram.color, m_color.x * m_color.w, m_color.y * m_color.w, m_color.z * m_color.w, m_color.w);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_points.size());

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
}

void PolygonNode::setPoints(const vector<vec2> &points)
{
    m_points = points;

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_points.size() * sizeof(vec2), m_points.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
