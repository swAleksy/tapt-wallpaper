#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QGuiApplication>

#include "viewmodels/galleryviewmodel.h"
#include "services/asyncimageprovider.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);


    GalleryViewModel galleryViewModel;
    QQmlApplicationEngine engine;

    engine.addImageProvider(QLatin1String("taptimage"), new TaptImageProvider);
    engine.rootContext()->setContextProperty("galleryViewModel", &galleryViewModel);
    engine.loadFromModule("org.kde.taptwallpaper", "Main");

    return app.exec();
}
