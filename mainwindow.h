#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPoint>
#include "include/QHotKey/qhotkey.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void startClicking();
    void stopClicking();
    void performClick();
    void updateClickType();
    void updateClickCount();
    void updateMaxClicks(int value);
    void updateFixedDelay(int value);
    void updateRandomMinDelay(int value);
    void updateRandomMaxDelay(int value);
    void updateFixedDelayUnit();
    void updateRandomDelayUnit();
    void updateMaxClicksUnit();
    void handleDelaySelection();
    void updateCoordinates();
    void registerHotkey();

private:
    Ui::MainWindow *ui;
    QTimer *clickTimer;
    bool clicking;
    int clickCount;
    int maxClicks;
    bool randomDelay;
    int fixedDelay;
    int randomMinDelay;
    int randomMaxDelay;
    QHotkey *hotkey;
    void simulateMouseClick(const QPoint &position); // Add this line
};

#endif // MAINWINDOW_H
