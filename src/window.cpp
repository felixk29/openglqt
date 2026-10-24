#include "window.h"
#include "widget.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QSignalBlocker>

#include "stl.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStatusBar>

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

constexpr float OffsetScale = 5.0f;
}

Window::Window(QWidget *parent): QMainWindow(parent){

    setWindowTitle("Project in training");

    //Viewer
    m_viewport =new Widget(this);


    //Controls
    auto *positionBox = new QGroupBox("Position");
    auto *positionForm= new QFormLayout(positionBox);
    m_offsetX=addSlider(positionForm, "X", -10,10,0);
    m_offsetY=addSlider(positionForm, "Y", -10,10,0);
    m_offsetZ=addSlider(positionForm, "Z", -10,10,0);

    auto *viewBox = new QGroupBox("View");
    auto *viewForm = new QFormLayout(viewBox);
    m_zoom = addSlider(viewForm,"Zoom",25,300,100); //percentage of standard view (3 units away)

    auto *rotationBox = new QGroupBox("Rotation");
    auto *rotationForm = new QFormLayout(rotationBox);
    m_yaw = addSlider(rotationForm, "Yaw", -180,180,40); //degrees
    m_pitch = addSlider(rotationForm, "Pitch", -180,180,30); //degrees

    auto *resetButton = new QPushButton("Reset View");

    //Panel for Controls
    auto *panel = new QWidget(this);
    panel-> setFixedWidth(250);
    auto *panelLayout= new QVBoxLayout(panel);
    panelLayout->addWidget(positionBox);
    panelLayout->addWidget(viewBox);
    panelLayout->addWidget(rotationBox);
    panelLayout->addWidget(resetButton);
    auto *loadButton=new QPushButton("Load model...");
    panelLayout->addWidget(loadButton);
    panelLayout->addStretch();

    //Combining
    auto *central =new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->addWidget(m_viewport,1);
    layout->addWidget(panel);
    setCentralWidget(central);


    //Connecting
    connect(m_offsetX, &QSlider::valueChanged, m_viewport, [this](int v) { m_viewport->setOffsetX(v / OffsetScale); });
    connect(m_offsetY, &QSlider::valueChanged, m_viewport, [this](int v) { m_viewport->setOffsetY(v / OffsetScale); });
    connect(m_offsetZ, &QSlider::valueChanged, m_viewport, [this](int v) { m_viewport->setOffsetZ(v / OffsetScale); });
    connect(m_zoom,    &QSlider::valueChanged, m_viewport, [this](int v) { m_viewport->setZoom(v / 100.0f); });
    connect(m_yaw,     &QSlider::valueChanged, m_viewport, [this](int v) { m_viewport->setYaw(v); });
    connect(m_pitch,   &QSlider::valueChanged, m_viewport, [this](int v) { m_viewport->setPitch(v); });

    connect(m_viewport, &Widget::viewChanged, this, &Window::syncSliders);

    connect(resetButton, &QPushButton::clicked, m_viewport, &Widget::resetView);
    connect(loadButton, &QPushButton::clicked, this, &Window::openModel);
    syncSliders();
}

void Window::openModel(){
    const QString path= QFileDialog::getOpenFileName(this, "Open model", QStringLiteral(MODELS_DIR), "STL files (*stl);");
    if(path.isEmpty()){return;}

    MeshData mesh;
    QString error;
    if (!loadStl(path,mesh,error)){
        QMessageBox::warning(this, "Could not load model", error);
        return;
    }

    m_viewport->setMesh(mesh);
    const QVector3D size=mesh.size();
    statusBar()->showMessage(
        QString("%1 | %2 triangles").arg(QFileInfo(path).fileName()).arg(mesh.triangleCount())
        );

}


void Window::syncSliders(){
    auto setBlocked=[](QSlider *slider, float v){
        const QSignalBlocker blocker(slider);
        slider->setValue(qRound(v));
    };

    setBlocked(m_offsetX, m_viewport->offset().x() * OffsetScale);
    setBlocked(m_offsetY, m_viewport->offset().y() * OffsetScale);
    setBlocked(m_offsetZ, m_viewport->offset().z() * OffsetScale);
    setBlocked(m_zoom,    m_viewport->zoom() * 100.0f);
    setBlocked(m_yaw,     m_viewport->yaw());
    setBlocked(m_pitch,   m_viewport->pitch());

}