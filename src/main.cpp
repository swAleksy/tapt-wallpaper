#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QGuiApplication>

//#include "viewmodels/galleryviewmodel.h"
#include "services/asyncimageprovider.h"
#include "services/lutimageprovider.h"

int main(int argc, char *argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("taptimage"), new TaptImageProvider());
    engine.addImageProvider(QStringLiteral("lut"), new LutImageProvider());
    engine.loadFromModule("org.kde.taptwallpaper", "Main");

    return app.exec();
}
