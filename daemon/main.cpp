#include <QCoreApplication>
#include "wallpaperdaemon.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("swaleksy");
    QCoreApplication::setApplicationName("taptwallpaper");

    WallpaperDaemon daemon;
    return app.exec();
}
