//#include <QGuiApplication>
#include <QApplication> 
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
    //QGuiApplication app(argc, argv);
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    engine.loadFromModule("org.kde.taptwallpaper", "Main");
    return app.exec();
}