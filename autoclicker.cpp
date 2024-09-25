#include "autoclicker.h"
#include "ui_autoclicker.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QCursor>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QTimer>


#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_MAC)
#include <ApplicationServices/ApplicationServices.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

// Conversion helper function to convert time values to milliseconds
int convertToMilliseconds(double value, const QString &unit) {
    if (unit == "Second(s)") {
        return value * 1000;  // Convert seconds to milliseconds
    } else if (unit == "Minute(s)") {
        return value * 60000; // Convert minutes to milliseconds
    }
    return value;  // Return value as is if already in milliseconds
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , clickTimer(new QTimer(this))
    , clicking(false)
    , clickCount(0)
    , maxClicks(0)
    , randomDelay(false)
    , fixedDelay(0)
    , randomMinDelay(0)
    , randomMaxDelay(0)
    , hotkey(nullptr)
{
    ui->setupUi(this); // Setup the UI components

    // Set default values for UI elements
    ui->clickControlTimeDoubleSpinBox->setValue(10);
    ui->fixedDelayRadioButton->setChecked(true);
    ui->fixedDelayDoubleSpinBox->setValue(0);
    ui->fixedDelayTimeComboBox->setCurrentText("Second(s)");
    ui->randomDelayComboBox->setCurrentText("Second(s)");
    ui->fixedPositionRadioButton->setChecked(true);
    ui->clickTypeLeftRadioButton->setChecked(true);
    ui->clickControlForverRadioButton->setChecked(true);

    // Connect info button to link to GH
    connect(ui->infoPushButton, &QPushButton::clicked, this, &MainWindow::handleInfoButton);

    elapsedTimer = QElapsedTimer();

    // Connect the click timer timeout signal to the performClick slot
    connect(clickTimer, &QTimer::timeout, this, &MainWindow::performClick);

    // Connect UI elements to their respective update functions
    connect(ui->clickTypeLeftRadioButton, &QRadioButton::toggled, this, &MainWindow::updateClickType);
    connect(ui->clickControlForverRadioButton, &QRadioButton::toggled, this, &MainWindow::updateClickCount);
    connect(ui->clickControlStopAfterRadioButton, &QRadioButton::toggled, this, &MainWindow::updateClickCount);
    connect(ui->clickControlTimeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateMaxClicks);
    connect(ui->fixedDelayDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateFixedDelay);
    connect(ui->randomDelayStartDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateRandomMinDelay);
    connect(ui->randomDelayEndDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateRandomMaxDelay);
    connect(ui->clickControlComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateMaxClicksUnit);
    connect(ui->fixedDelayTimeComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateFixedDelayUnit);
    connect(ui->randomDelayComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateRandomDelayUnit);

    // Update cursor coordinates every 100 ms
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateCoordinates);
    updateTimer->start(100);

    // Register the hotkey initially
    registerHotkey();

    // Connect buttons to assign and reset the toggle key
    connect(ui->assignToggleKeyButton, &QPushButton::clicked, this, &MainWindow::assignToggleKey);
    connect(ui->resetToggleKeyButton, &QPushButton::clicked, this, &MainWindow::resetToggleKey);
}

MainWindow::~MainWindow()
{
    delete ui; // Clean up the UI
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QString toggleKey = ui->toggleKeyEdit->text();
    QKeySequence keySequence(toggleKey);

    // Check if the pressed key matches the toggle key
    if (keySequence.matches(QKeySequence(event->key())) == QKeySequence::ExactMatch) {
        if (clicking) {
            qDebug() << "Stopping clicking...";
            stopClicking();
        } else {
            qDebug() << "Starting clicking...";
            startClicking();
        }
    }
}

void MainWindow::assignToggleKey() {
    QString toggleKey = ui->toggleKeyEdit->text(); // Get the new toggle key from the UI
    if (!toggleKey.isEmpty()) {
        registerHotkey(); // Register the hotkey with the new key
        qDebug() << "Toggle key assigned to:" << toggleKey; // Log the assigned key
    }
}

void MainWindow::resetToggleKey() {
    // Reset hotkey to default value
    ui->toggleKeyEdit->setText("F1");
    registerHotkey();
    qDebug() << "Toggle key reset to F1";
}

void MainWindow::registerHotkey() {
    // Unregister the previous hotkey if it exists
    if (hotkey) {
        delete hotkey;
    }

    QString toggleKey = ui->toggleKeyEdit->text();

    // Use a default value if no key is set
    if (toggleKey.isEmpty()) {
        toggleKey = "F1";
    }

    // Register the new hotkey
    hotkey = new QHotkey(QKeySequence(toggleKey), true, this);

    if (hotkey->isRegistered()) {
        qDebug() << "Hotkey registered:" << toggleKey;
    } else {
        qDebug() << "Failed to register hotkey:" << toggleKey;
    }

    // Connect the hotkey activation signal to start/stop clicking
    connect(hotkey, &QHotkey::activated, this, [&]() {
        if (clicking) {
            qDebug() << "Stopping clicking...";
            stopClicking();
        } else {
            qDebug() << "Starting clicking...";
            startClicking();
        }
    });
}

void MainWindow::startClicking()
{
    clicking = true;
    clickCount = 0;

    // Determine the stopping condition based on the selected control type
    if (ui->clickControlForverRadioButton->isChecked()) {
        maxClicks = 0; // Forever
        qDebug() << "Clicking will continue forever.";
    } else {
        double value = ui->clickControlTimeDoubleSpinBox->value();
        QString unit = ui->clickControlComboBox->currentText();

        if (unit == "Click(s)") {
            maxClicks = value; // Set max clicks directly
            qDebug() << "Clicking will stop after" << maxClicks << "clicks.";
        } else {
            maxClicks = 0; // Reset maxClicks for time-based control
            elapsedTimer.start();
            qDebug() << "Clicking will stop after" << value << unit << ".";
        }
    }

    // Determine if random delay is used
    randomDelay = ui->randomDelayStartDoubleSpinBox->value() > 0 || ui->randomDelayEndDoubleSpinBox->value() > 0;

    // Convert fixed delay to milliseconds based on the selected unit
    fixedDelay = convertToMilliseconds(ui->fixedDelayDoubleSpinBox->value(), ui->fixedDelayTimeComboBox->currentText());

    qDebug() << "Clicking started. Fixed Delay:" << fixedDelay << "ms";

    // Start with fixed delay
    clickTimer->start(fixedDelay); // Start the timer with fixed delay
}

void MainWindow::stopClicking()
{
    clicking = false;
    clickTimer->stop();
    qDebug() << "Clicking stopped.";
}

void MainWindow::performClick()
{
    // Check for time-based stopping condition
    if (!ui->clickControlForverRadioButton->isChecked()) {
        double value = ui->clickControlTimeDoubleSpinBox->value();
        QString unit = ui->clickControlComboBox->currentText();
        int durationInMilliseconds = convertToMilliseconds(value, unit);

        // Check if the elapsed time exceeds the duration
        if (elapsedTimer.elapsed() >= durationInMilliseconds) {
            qDebug() << "Time limit of" << value << unit << "reached. Stopping...";
            stopClicking();
            return;
        }
    }

    QPoint cursorPos;

    // Determine the cursor position based on user selection
    if (ui->fixedPositionRadioButton->isChecked()) {
        cursorPos = QCursor::pos();
    } else if (ui->dynamicPositionRadioButton->isChecked()) {

        // Get bounding box coordinates from UI
        int x1 = ui->dyanmicPositionCoords_x1->value();
        int y1 = ui->dyanmicPositionCoords_y1->value();
        int x2 = ui->dyanmicPositionCoords_x2->value();
        int y2 = ui->dyanmicPositionCoords_y2->value();

        // Ensure coordinates are within screen bounds
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        x1 = qMax(0, qMin(x1, screenGeometry.width() - 1));
        y1 = qMax(0, qMin(y1, screenGeometry.height() - 1));
        x2 = qMax(0, qMin(x2, screenGeometry.width() - 1));
        y2 = qMax(0, qMin(y2, screenGeometry.height() - 1));

        // Generate a random position within the bounding box
        int randomX = QRandomGenerator::global()->bounded(qMin(x1, x2), qMax(x1, x2) + 1);
        int randomY = QRandomGenerator::global()->bounded(qMin(y1, y2), qMax(y1, y2) + 1);
        cursorPos = QPoint(randomX, randomY);
    }

    // Simulate the mouse click at the determined position
    simulateMouseClick(cursorPos);

    clickCount++;
    qDebug() << "Click performed at:" << cursorPos << ". Total Clicks:" << clickCount;

    // Check which delay option is selected
    if (ui->randomDelayRadioButton->isChecked()) {
        // Handle random delays if applicable
        if (randomMinDelay >= 0 && randomMaxDelay > randomMinDelay) {
            int randomDelayDuration = QRandomGenerator::global()->bounded(randomMinDelay, randomMaxDelay); // Generate random delay
            clickTimer->start(randomDelayDuration);
            qDebug() << "Random delay set for next click:" << randomDelayDuration << "ms";
        } else {
            qDebug() << "Invalid random delay settings. Using fixed delay instead.";
            clickTimer->start(fixedDelay);
        }
    } else {
        // Use fixed delay
        clickTimer->start(fixedDelay);
    }
}

void MainWindow::simulateMouseClick(const QPoint &position)
{
    // Move the cursor to the desired position
#ifdef Q_OS_WIN
    SetCursorPos(position.x(), position.y());

    // Create INPUT structure for mouse event
    INPUT input = {0};
    input.type = INPUT_MOUSE;

    if (ui->clickTypeLeftRadioButton->isChecked()) {
        // Left click
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN; // Mouse down event for left click
        SendInput(1, &input, sizeof(INPUT));     // Send mouse down event

        // Create another INPUT for mouse release
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;   // Mouse up event for left click
        SendInput(1, &input, sizeof(INPUT));     // Send mouse up event
    } else {
        // Right click
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN; // Mouse down event for right click
        SendInput(1, &input, sizeof(INPUT));      // Send mouse down event

        // Create another INPUT for mouse release
        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;   // Mouse up event for right click
        SendInput(1, &input, sizeof(INPUT));      // Send mouse up event
    }
#elif defined(Q_OS_MAC)

    // Move the cursor to the desired position
    CGEventRef moveEvent = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, CGPointMake(position.x(), position.y()), kCGEventLeftMouseDown);
    CGEventPost(kCGEventGlobal, moveEvent); // Post mouse move event
    CFRelease(moveEvent); // Release the move event

    // Create mouse click event
    CGEventRef clickEvent = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDown, CGPointMake(position.x(), position.y()), kCGEventLeftMouseDown);
    CGEventPost(kCGEventGlobal, clickEvent); // Post mouse down event
    CFRelease(clickEvent); // Release the click event

    // Release mouse up event
    clickEvent = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseUp, CGPointMake(position.x(), position.y()), kCGEventLeftMouseUp);
    CGEventPost(kCGEventGlobal, clickEvent); // Post mouse up event
    CFRelease(clickEvent); // Release the mouse up event
#elif defined(Q_OS_LINUX)

    // Move the cursor to the desired position
    Display *display = XOpenDisplay(NULL);
    if (display) {
        XWarpPointer(display, None, DefaultRootWindow(display), 0, 0, 0, 0, position.x(), position.y());
        XFlush(display);

        // Simulate mouse click
        XEvent event;
        event.xbutton.type = ButtonPress;
        event.xbutton.display = display;
        event.xbutton.window = DefaultRootWindow(display);
        event.xbutton.x = position.x();
        event.xbutton.y = position.y();
        event.xbutton.button = 1;
        event.xbutton.same_screen = True;
        XSendEvent(display, InputFocus, True, ButtonPressMask, &event);
        XFlush(display);

        event.xbutton.type = ButtonRelease; // Set event type to button release
        XSendEvent(display, InputFocus, True, ButtonReleaseMask, &event); // Send button release event
        XFlush(display);

        XCloseDisplay(display);
    }
#endif
}

void MainWindow::updateClickType()
{
    // Log the updated click type based on user selection
    qDebug() << "Click type updated to:" << (ui->clickTypeLeftRadioButton->isChecked() ? "Left" : "Right");
}

void MainWindow::updateClickCount()
{
    // Update max clicks based on user selection
    if (ui->clickControlForverRadioButton->isChecked()) {
        maxClicks = 0;
        qDebug() << "Max clicks set to: Forever";
    } else {
        maxClicks = ui->clickControlTimeDoubleSpinBox->value();
        qDebug() << "Max clicks set to:" << maxClicks;
    }
}

void MainWindow::updateMaxClicks(double value)
{
    // Update max clicks based on the value and unit selected
    maxClicks = convertToMilliseconds(value, ui->clickControlComboBox->currentText());
    qDebug() << "Max clicks updated to:" << maxClicks;
}

void MainWindow::updateFixedDelay(double value)
{
    // Convert the fixed delay to milliseconds and log the update
    fixedDelay = convertToMilliseconds(value, ui->fixedDelayTimeComboBox->currentText());
    qDebug() << "Fixed delay updated to:" << fixedDelay << "ms";
}

void MainWindow::updateRandomMinDelay(double value)
{
    // Convert the random min delay to milliseconds and log the update
    randomMinDelay = convertToMilliseconds(value, ui->randomDelayComboBox->currentText());
    qDebug() << "Random min delay updated to:" << randomMinDelay << "ms";
}

void MainWindow::updateRandomMaxDelay(double value)
{
    // Convert the random max delay to milliseconds and log the update
    randomMaxDelay = convertToMilliseconds(value, ui->randomDelayComboBox->currentText());
    qDebug() << "Random max delay updated to:" << randomMaxDelay << "ms";
}

void MainWindow::updateMaxClicksUnit()
{
    // Update the max clicks based on the new unit selected
    updateMaxClicks(ui->clickControlTimeDoubleSpinBox->value());
}

void MainWindow::updateFixedDelayUnit()
{
    // Update the fixed delay based on the new unit selected
    updateFixedDelay(ui->fixedDelayDoubleSpinBox->value());
}

void MainWindow::updateRandomDelayUnit()
{
    // Update both random min and max delays based on the new unit selected
    updateRandomMinDelay(ui->randomDelayStartDoubleSpinBox->value());
    updateRandomMaxDelay(ui->randomDelayEndDoubleSpinBox->value());
}

void MainWindow::updateCoordinates() {
    // Update the displayed cursor coordinates in the UI
    QPoint cursorPos = QCursor::pos();
    ui->fixedPositionLabel_x->setText(QString("X: %1").arg(cursorPos.x()));
    ui->fixedPositionLabel_y->setText(QString("Y: %1").arg(cursorPos.y()));
}

void MainWindow::handleInfoButton()
{
    QDesktopServices::openUrl(QUrl("https://github.com/apesci1"));
}
