#ifndef AUTOCLICKER_H
#define AUTOCLICKER_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
#include <QPoint>
#include <QHotkey>
#include <QDesktopServices>
#include <QUrl>

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
    void updateMaxClicks(double value);
    void updateFixedDelay(double value);
    void updateRandomMinDelay(double value);
    void updateRandomMaxDelay(double value);
    void updateFixedDelayUnit();
    void updateRandomDelayUnit();
    void updateMaxClicksUnit();
    void updateCoordinates();
    void registerHotkey();
    void assignToggleKey();
    void resetToggleKey();
    void handleInfoButton();

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
    QElapsedTimer elapsedTimer;
    QHotkey *hotkey;
    void simulateMouseClick(const QPoint &position);
};

#endif // AUTOCLICKER_H
