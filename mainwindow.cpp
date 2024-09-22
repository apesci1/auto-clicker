#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QCursor>
#include <QApplication>
#include <windows.h> // Include Windows API header
#include <QKeyEvent>
#include <QTimer>


// Conversion helper function
int convertToMilliseconds(double value, const QString &unit) {
    if (unit == "Second(s)") {
        return value * 1000;  // Convert seconds to milliseconds
    } else if (unit == "Minute(s)") {
        return value * 60000; // Convert minutes to milliseconds
    }
    return value;  // Already in milliseconds
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
{
    ui->setupUi(this);
    // Set default values
    ui->clickControlTimeDoubleSpinBox->setValue(10);      // Default max clicks to 10
    ui->fixedDelayDoubleSpinBox->setValue(1);      // Default fixed delay to 10
    ui->fixedDelayTimeComboBox->setCurrentText("Second(s)"); // Default fixed delay unit to seconds
    ui->randomDelayComboBox->setCurrentText("Second(s)"); // Default random delay unit to seconds
    ui->fixedPositionRadioButton->setChecked(true); // Default click position (at cursor)
    ui->clickTypeLeftRadioButton->setChecked(true); // Default click type to left click
    ui->clickControlForverRadioButton->setChecked(true); // Default to stop after max clicks

    connect(clickTimer, &QTimer::timeout, this, &MainWindow::performClick);

    // Connect UI elements
    connect(ui->clickTypeLeftRadioButton, &QRadioButton::toggled, this, &MainWindow::updateClickType);
    connect(ui->clickControlForverRadioButton, &QRadioButton::toggled, this, &MainWindow::updateClickCount);
    connect(ui->clickControlStopAfterRadioButton, &QRadioButton::toggled, this, &MainWindow::updateClickCount);
    connect(ui->clickControlTimeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateMaxClicks);
    connect(ui->fixedDelayDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateFixedDelay);
    connect(ui->randomDelayStartDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateRandomMinDelay);
    connect(ui->randomDelayEndDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateRandomMaxDelay);
    connect(ui->clickControlTimeComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateMaxClicksUnit);
    connect(ui->fixedDelayTimeComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateFixedDelayUnit);
    connect(ui->randomDelayComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateRandomDelayUnit);


    // Update coordinates every 100 ms
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateCoordinates);
    updateTimer->start(100);

    // Register the hotkey initially
    registerHotkey();

    // Connect the line edit text change signal to update the hotkey
    connect(ui->toggleKeyEdit, &QLineEdit::textChanged, this, &MainWindow::registerHotkey);
}


MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::registerHotkey() {
    // Unregister the previous hotkey if it exists
    if (hotkey) {
        delete hotkey;
    }

    QString toggleKey = ui->toggleKeyEdit->text();
    hotkey = new QHotkey(QKeySequence(toggleKey), true, this);

    if (hotkey->isRegistered()) {
        qDebug() << "Hotkey registered:" << toggleKey;
    } else {
        qDebug() << "Failed to register hotkey:" << toggleKey;
    }

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
    clickCount = 0; // Reset click count
    maxClicks = ui->clickControlForverRadioButton->isChecked() ? 0 : ui->clickControlTimeDoubleSpinBox->value(); // Forever or stop after

    randomDelay = ui->randomDelayStartDoubleSpinBox->value() > 0 || ui->randomDelayEndDoubleSpinBox->value() > 0;
    // Convert fixed delay to milliseconds based on the selected unit
    fixedDelay = convertToMilliseconds(ui->fixedDelayDoubleSpinBox->value(), ui->fixedDelayTimeComboBox->currentText());

    qDebug() << "Clicking started. Max Clicks:" << maxClicks << ", Fixed Delay:" << fixedDelay << "ms";

    // Start with fixed delay
    clickTimer->start(fixedDelay);
}

void MainWindow::stopClicking()
{
    clicking = false;
    clickTimer->stop();
    qDebug() << "Clicking stopped.";
}

void MainWindow::performClick()
{
    if (maxClicks > 0 && clickCount >= maxClicks) {
        qDebug() << "Max clicks reached. Stopping...";
        stopClicking();
        return;
    }

    QPoint cursorPos = QCursor::pos();
    simulateMouseClick(cursorPos);

    clickCount++;
    qDebug() << "Click performed at:" << cursorPos << ". Total Clicks:" << clickCount;

    // Check which delay option is selected
    if (ui->randomDelayRadioButton->isChecked()) {
        // Handle random delays if applicable
        if (randomMinDelay >= 0 && randomMaxDelay > randomMinDelay) {
            int randomDelayDuration = QRandomGenerator::global()->bounded(randomMinDelay, randomMaxDelay);
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
    SetCursorPos(position.x(), position.y());

    // Create INPUT structure for mouse event
    INPUT input = {0};
    input.type = INPUT_MOUSE;

    if (ui->clickTypeLeftRadioButton->isChecked()) {
        // Left click
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN; // Mouse down event for left click
        SendInput(1, &input, sizeof(INPUT));     // Send mouse down

        // Create another INPUT for mouse release
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;   // Mouse up event for left click
        SendInput(1, &input, sizeof(INPUT));     // Send mouse up
    } else {
        // Right click
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN; // Mouse down event for right click
        SendInput(1, &input, sizeof(INPUT));      // Send mouse down

        // Create another INPUT for mouse release
        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;   // Mouse up event for right click
        SendInput(1, &input, sizeof(INPUT));      // Send mouse up
    }
}

void MainWindow::updateClickType()
{
    qDebug() << "Click type updated to:" << (ui->clickTypeLeftRadioButton->isChecked() ? "Left" : "Right");
}

void MainWindow::updateClickCount()
{
    if (ui->clickControlForverRadioButton->isChecked()) {
        maxClicks = 0; // Forever
        qDebug() << "Max clicks set to: Forever";
    } else {
        maxClicks = ui->clickControlTimeDoubleSpinBox->value(); // Set to spin box value
        qDebug() << "Max clicks set to:" << maxClicks;
    }
}

void MainWindow::updateMaxClicks(double value)
{
    maxClicks =  convertToMilliseconds(value, ui->clickControlTimeComboBox->currentText());
    qDebug() << "Max clicks updated to:" << maxClicks;
}

void MainWindow::updateFixedDelay(double value)
{
    // Convert the fixed delay to milliseconds
    fixedDelay = convertToMilliseconds(value, ui->fixedDelayTimeComboBox->currentText());
    qDebug() << "Fixed delay updated to:" << fixedDelay << "ms";
}

void MainWindow::updateRandomMinDelay(double value)
{
    // Convert the random min delay to milliseconds
    randomMinDelay = convertToMilliseconds(value, ui->randomDelayComboBox->currentText());
    qDebug() << "Random min delay updated to:" << randomMinDelay << "ms";
}

void MainWindow::updateRandomMaxDelay(double value)
{
    // Convert the random max delay to milliseconds
    randomMaxDelay = convertToMilliseconds(value, ui->randomDelayComboBox->currentText());
    qDebug() << "Random max delay updated to:" << randomMaxDelay << "ms";
}

void MainWindow::updateMaxClicksUnit()
{
    // Update the fixed delay based on the new unit
    updateMaxClicks(ui->clickControlTimeDoubleSpinBox->value());
}

void MainWindow::updateFixedDelayUnit()
{
    // Update the fixed delay based on the new unit
    updateFixedDelay(ui->fixedDelayDoubleSpinBox->value());
}

void MainWindow::updateRandomDelayUnit()
{
    // Update both random min and max delays based on the new unit
    updateRandomMinDelay(ui->randomDelayStartDoubleSpinBox->value());
    updateRandomMaxDelay(ui->randomDelayEndDoubleSpinBox->value());
}

void MainWindow::updateCoordinates() {
    QPoint cursorPos = QCursor::pos();
    ui->fixedPositionLabel_x->setText(QString("X: %1").arg(cursorPos.x()));
    ui->fixedPositionLabel_y->setText(QString("Y: %1").arg(cursorPos.y()));
}
