#include "widget.h"
#include <QDebug>
#include <cstddef>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

static const char *vertexShaderSource = R"(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

uniform mat4 uMVP;                     // NEW: same value for every vertex in the draw call

out vec3 vColor;

void main()
{
    vColor = aColor;
    gl_Position = uMVP * vec4(aPos, 1.0);   // CHANGED: transform the position
}
)";

static const char *fragmentShaderSource = R"(#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vColor, 1.0);
}
)";

namespace {

struct Vertex {
    QVector3D position;
    QVector3D color;
};

//    pos                   color   
const Vertex cubeVertices[] = {
    {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}},
    {{ 0.5f, -0.5f,  0.5f}, {1, 0, 1}},
    {{ 0.5f,  0.5f,  0.5f}, {1, 1, 1}},
    {{-0.5f,  0.5f,  0.5f}, {0, 1, 1}},
    {{-0.5f, -0.5f, -0.5f}, {0, 0, 0}},
    {{ 0.5f, -0.5f, -0.5f}, {1, 0, 0}},
    {{ 0.5f,  0.5f, -0.5f}, {1, 1, 0}},
    {{-0.5f,  0.5f, -0.5f}, {0, 1, 0}},
};

const GLuint cubeIndices[] = {
    0, 1, 2,  2, 3, 0,
    1, 5, 6,  6, 2, 1,
    5, 4, 7,  7, 6, 5,
    4, 0, 3,  3, 7, 4,
    3, 2, 6,  6, 7, 3,
    4, 5, 1,  1, 0, 4,
};

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

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(cubeVertices, sizeof(cubeVertices));


    m_ebo.create();
    m_ebo.bind();
    m_ebo.allocate(cubeIndices, sizeof(cubeIndices));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),reinterpret_cast<void *>(offsetof(Vertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),reinterpret_cast<void *>(offsetof(Vertex, color)));

    m_vao.release();
    m_vbo.release();
    //always vao first
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

    m_program->bind();
    m_program->setUniformValue("uMVP", m_projection * view * model);

    m_vao.bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
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
