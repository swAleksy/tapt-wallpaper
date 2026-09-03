#include <QGuiApplication>
#include "wallpaperdaemon.h"

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName("swaleksy");
    QGuiApplication::setApplicationName("taptwallpaper");

    WallpaperDaemon daemon;
    return app.exec();
}
