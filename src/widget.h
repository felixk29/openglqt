#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QPointF>

class Widget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget () override;

    QVector3D offset() const {return m_offset;}
    float zoom() const {return m_zoom;}
    float yaw() const {return m_yaw;}
    float pitch() const {return m_pitch;}


protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    //Defaultvalues
    static constexpr float DefaultYaw=40.f;
    static constexpr float DefaultPitch=30.f;
    static constexpr float DefaultZoom=1.f;
    //

    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_ebo{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject m_vao;

    QMatrix4x4 m_projection;
    QVector3D m_offset;

    float m_yaw = DefaultYaw;
    float m_pitch = DefaultPitch;
    float m_zoom = DefaultZoom;

    QPointF m_lastMousePos;

public slots:
    void setOffsetX(float x);
    void setOffsetY(float y);
    void setOffsetZ(float z);
    void setZoom(float zoom);     
    void setYaw(float degrees);
    void setPitch(float degrees);
    void resetView(); 

signals: 
    void viewChanged();

};