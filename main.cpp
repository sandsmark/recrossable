#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "crossword.h"

#include "cwc/symbol.hh"

int main(int argc, char *argv[])
{
    Symbol::buildindex();
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);


    QQmlApplicationEngine engine;
    qmlRegisterSingletonType<Crossword>("com.iskrembilen", 1, 0, "Crossword", [](QQmlEngine *engine, QJSEngine*) -> QObject* {
        Crossword *crossword = new Crossword;
        engine->setObjectOwnership(crossword, QQmlEngine::JavaScriptOwnership);
        return crossword;
    });
//    engine.rootContext()->setProperty("Crossword", &crossword);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
