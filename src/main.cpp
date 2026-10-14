#include <QApplication>
#include <QSurfaceFormat>

#include "window.h"

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    Window window;
    window.resize(1200, 800);
    window.show();

    return app.exec();
}