#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "timeshiftplayer.h"
#include "iconprovider.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Create an instance of TimeshiftPlayer
    TimeshiftPlayer player;
    
    QQmlApplicationEngine engine;
    engine.addImageProvider("icon", new IconProvider(":/MaterialIconsSharp-Regular.otf", ":/MaterialIconsSharp-Regular.codepoints"));
    engine.rootContext()->setContextProperty("timeshiftPlayer", &player);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
