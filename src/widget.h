#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector3D>

class Widget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget () override;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_ebo{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject m_vao;

    QMatrix4x4 m_projection;
    float m_yaw = 40.0f;
    float m_pitch = 30.0f;
    QVector3D m_offset;
    float m_zoom = 1.0f;

public slots:
    void setOffsetX(float x);
    void setOffsetY(float y);
    void setOffsetZ(float z);
    void setZoom(float zoom);        // 1.0 = default, 2.0 = twice as close
    void setYaw(float degrees);
    void setPitch(float degrees);

};