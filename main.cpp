#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "include/QHotKey/qhotkey.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QHotkey hotkey(QKeySequence("Ctrl+Alt+Q"), true, &a);
    qDebug() << "Is registered:" << hotkey.isRegistered();

    QObject::connect(&hotkey, &QHotkey::activated, qApp, [&](){
        qDebug() << "Hotkey Activated - the application will quit now";
        qApp->quit();
    });

    MainWindow w;
    w.show();
    return a.exec();
}
