#pragma once

#include <QMainWindow> 
class QSlider;
class Widget; 

class Window: public QMainWindow{
    Q_OBJECT

public: 
    explicit Window(QWidget *parent =nullptr);

private:
    Widget *m_viewport=nullptr;

    void syncSliders();

    QSlider *m_offsetX = nullptr;
    QSlider *m_offsetY = nullptr;
    QSlider *m_offsetZ = nullptr;
    QSlider *m_zoom = nullptr;
    QSlider *m_yaw = nullptr;
    QSlider *m_pitch = nullptr;
};