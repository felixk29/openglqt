#include "widget.h"
#include <QDebug>
#include <cstddef>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include <vector>

static const char *vertexShaderSource = R"(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uNormalMatrix;

out vec3 vWorldPos;
out vec3 vNormal;

void main()
{
    vWorldPos = vec3(uModel * vec4(aPos, 1.0));
    vNormal = uNormalMatrix * aNormal;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

static const char *fragmentShaderSource = R"(#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;

uniform vec3 uLightDir;
uniform vec3 uCameraPos;
uniform vec3 uColor;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 H = normalize(L + V);

    float ambient  = 0.15;
    float diffuse  = max(dot(N, L), 0.0);
    float specular = diffuse > 0.0 ? pow(max(dot(N, H), 0.0), 64.0) : 0.0;

    vec3 color = uColor * (ambient + diffuse) + vec3(0.4) * specular;
    FragColor = vec4(color, 1.0);
}
)";


namespace {

struct Vertex {
    QVector3D position;
    QVector3D normal;
};

//build cube
void makeCube(std::vector<Vertex> &vertices, std::vector<GLuint> &indices)
{
    const QVector3D normals[] = {
        { 1, 0, 0}, {-1, 0, 0},   // right, left
        { 0, 1, 0}, { 0,-1, 0},   // top, bottom
        { 0, 0, 1}, { 0, 0,-1},   // front, back
    };

    for (const QVector3D &n : normals) {
        const QVector3D u = (n.x() != 0) ? QVector3D(0, 1, 0) : QVector3D(1, 0, 0);
        const QVector3D v = QVector3D::crossProduct(n, u);

        const GLuint first = GLuint(vertices.size());
        vertices.push_back({(n - u - v) * 0.5f, n});
        vertices.push_back({(n + u - v) * 0.5f, n});
        vertices.push_back({(n + u + v) * 0.5f, n});
        vertices.push_back({(n - u + v) * 0.5f, n});
        //put first and second triangle in
        indices.insert(indices.end(), {first, first + 1, first + 2,first + 2, first + 3, first});
    }
}

} // namespace

Widget::Widget(QWidget *parent):QOpenGLWidget(parent){
}

void Widget::initializeGL(){
    initializeOpenGLFunctions();
    qDebug() << "OpenGL version:" << reinterpret_cast<const char *>(glGetString(GL_VERSION));
    glClearColor(0.11f,0.2f,0.2f,1.0f);
    glEnable(GL_DEPTH_TEST);

    m_program = new QOpenGLShaderProgram;
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource))
        qWarning() << "Vertex shader error:" << m_program->log();
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource))
        qWarning() << "Fragment shader error:" << m_program->log();
    if (!m_program->link())
        qWarning() << "Link error:" << m_program->log();
    //loading shaders and some warnings
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    makeCube(vertices, indices);
    m_indexCount = int(indices.size());

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices.data(), int(vertices.size() * sizeof(Vertex)));

    m_ebo.create();
    m_ebo.bind();
    m_ebo.allocate(indices.data(), int(indices.size() * sizeof(GLuint)));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, normal)));   // CHANGED

    m_vao.release();
    m_vbo.release();

}


void Widget::resizeGL(int w, int h)
{
    const float aspect = float(w) / float(qMax(h, 1));
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, aspect, 0.1f, 100.0f);
}


void Widget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 model;
    model.translate(m_offset);               
    model.rotate(m_pitch, 1.0f, 0.0f, 0.0f);
    model.rotate(m_yaw,   0.0f, 1.0f, 0.0f);

    QMatrix4x4 view;
    view.translate(0.0f, 0.0f, -3.0f / m_zoom);   

    const QVector3D cameraPos = view.inverted().map(QVector3D(0, 0, 0));

    m_program->bind();
    m_program->setUniformValue("uMVP", m_projection * view * model);
    m_program->setUniformValue("uModel", model);
    m_program->setUniformValue("uNormalMatrix", model.normalMatrix());
    m_program->setUniformValue("uLightDir", QVector3D(0.5f, 1.0f, 0.8f));   // from top-right-front
    m_program->setUniformValue("uCameraPos", cameraPos);
    m_program->setUniformValue("uColor", QVector3D(0.9f, 0.8f, 0.5f));     // orangey light

    m_vao.bind();
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);

    m_vao.release();
    m_program->release();
}

void Widget::resetView(){

    m_offset =QVector3D();
    m_zoom=DefaultZoom;
    m_yaw=DefaultYaw;
    m_pitch=DefaultPitch;
    emit viewChanged();
    update();
}

void Widget::mousePressEvent(QMouseEvent *event){
    m_lastMousePos=event->position();
}

void Widget::mouseMoveEvent(QMouseEvent *event){
    if(!(event->buttons() & Qt::LeftButton))
        return;
    
    const QPointF delta= event->position()-m_lastMousePos;
    m_lastMousePos=event->position();

    const float degreesPerPixel=0.5f;
    m_yaw= std::remainder(m_yaw+float(delta.x())*degreesPerPixel,360);
    m_pitch= std::remainder(m_pitch+float(delta.y())*degreesPerPixel,360);
    
    emit viewChanged();
    update();
}

void Widget::wheelEvent(QWheelEvent *event){
    const float steps=event->angleDelta().y()/120.f;
    m_zoom = qBound(0.25, m_zoom*std::pow(1.1f,steps),3.f);
    emit viewChanged();
    update();
}



Widget::~Widget()
{
    makeCurrent();      
    m_vbo.destroy();
    m_vao.destroy();
    m_ebo.destroy();
    delete m_program;
    doneCurrent();
}

void Widget::setOffsetX(float x) { m_offset.setX(x); update(); }
void Widget::setOffsetY(float y) { m_offset.setY(y); update(); }
void Widget::setOffsetZ(float z) { m_offset.setZ(z); update(); }
void Widget::setZoom(float zoom) { m_zoom = zoom;    update(); }
void Widget::setYaw(float degrees)   { m_yaw = degrees;   update(); }
void Widget::setPitch(float degrees) { m_pitch = degrees; update(); }
