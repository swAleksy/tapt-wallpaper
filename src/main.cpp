#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "viewmodels/galleryviewmodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    GalleryViewModel galleryViewModel(&engine);  // ← engine jako parent
    engine.rootContext()->setContextProperty("galleryViewModel", &galleryViewModel);

    engine.loadFromModule("org.kde.taptwallpaper", "Main");
    return app.exec();
}