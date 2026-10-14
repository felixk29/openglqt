#include "window.h"
#include "widget.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

namespace {
//sliderfactory
QSlider *addSlider(QFormLayout *form, const QString &label, int min, int max, int value)
{
    auto *slider = new QSlider(Qt::Horizontal);
    slider->setRange(min, max);
    slider->setValue(value);
    form->addRow(label, slider);
    return slider;
}
}

Window::Window(QWidget *parent): QMainWindow(parent){

    setWindowTitle("Project in training");

    //Viewer
    m_viewport =new Widget(this);


    //Controls
    auto *positionBox = new QGroupBox("Position");
    auto *positionForm= new QFormLayout(positionBox);
    QSlider *offsetX=addSlider(positionForm, "X", -10,10,0);
    QSlider *offsetY=addSlider(positionForm, "Y", -10,10,0);
    QSlider *offsetZ=addSlider(positionForm, "Z", -10,10,0);

    auto *viewBox = new QGroupBox("View");
    auto *viewForm = new QFormLayout(viewBox);
    QSlider *zoom = addSlider(viewForm,"Zoom",25,300,100); //percentage of standard view (3 units away)

    auto *rotationBox = new QGroupBox("Rotation");
    auto *rotationForm = new QFormLayout(rotationBox);
    QSlider *yaw = addSlider(rotationForm, "Yaw", -180,180,40); //degrees
    QSlider *pitch = addSlider(rotationForm, "Pitch", -180,180,30); //degrees

    auto *resetButton = new QPushButton("Reset View");

    //Panel for Controls
    auto *panel = new QWidget(this);
    panel-> setFixedWidth(250);
    auto *panelLayout= new QVBoxLayout(panel);
    panelLayout->addWidget(positionBox);
    panelLayout->addWidget(viewBox);
    panelLayout->addWidget(rotationBox);
    panelLayout->addWidget(resetButton);
    panelLayout->addWidget(new QPushButton("Load model..."));
    panelLayout->addStretch();

    //Combining
    auto *central =new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->addWidget(m_viewport,1);
    layout->addWidget(panel);
    setCentralWidget(central);


    //Connecting
    connect(offsetX, &QSlider::valueChanged, m_viewport, [this](int v) {m_viewport->setOffsetX(v/50.f);});
    connect(offsetY, &QSlider::valueChanged, m_viewport, [this](int v) {m_viewport->setOffsetY(v/50.f);});
    connect(offsetZ, &QSlider::valueChanged, m_viewport, [this](int v) {m_viewport->setOffsetZ(v/50.f);});
    connect(zoom, &QSlider::valueChanged, m_viewport, [this](int v) {m_viewport->setZoom(v/100.f);});
    connect(yaw, &QSlider::valueChanged, m_viewport, [this](int v) {m_viewport->setYaw(v);});
    connect(pitch, &QSlider::valueChanged, m_viewport, [this](int v) {m_viewport->setPitch(v);});

    connect(resetButton, &QPushButton::clicked, this, [=] {
        offsetX->setValue(0);
        offsetY->setValue(0);
        offsetZ->setValue(0);
        zoom->setValue(100);
        yaw->setValue(40);
        pitch->setValue(30);
    });

}