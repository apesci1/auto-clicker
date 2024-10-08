#include "autoclicker.h"
#include <QApplication>
#include <QDebug>
#include <QHotkey>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Register quit hotkey
    QHotkey hotkey(QKeySequence("Ctrl+Alt+Q"), true, &a);
    qDebug() << "Is registered:" << hotkey.isRegistered();

    // Connect quit hotkey
    QObject::connect(&hotkey, &QHotkey::activated, qApp, [&](){
        qDebug() << "Hotkey Activated - the application will quit now";
        qApp->quit();
    });

    MainWindow w;
    w.show();
    return a.exec();
}
