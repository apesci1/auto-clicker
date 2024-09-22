#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QCursor>
#include <QApplication>
#include <windows.h> // Include Windows API header
#include <QKeyEvent>

// Conversion helper function
int convertToMilliseconds(int value, const QString &unit) {
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
    ui->spinBox_6->setValue(10);      // Default max clicks to 10
    ui->spinBox_5->setValue(10);      // Default fixed delay to 10
    // ui->spinBox_3->setValue(1);       // Default random min delay to 1
    // ui->spinBox_4->setValue(5);       // Default random max delay to 5
    ui->comboBox_3->setCurrentText("Second(s)"); // Default fixed delay unit to seconds
    ui->comboBox_2->setCurrentText("Second(s)"); // Default random delay unit to seconds
    ui->radioButton->setChecked(true); // Default click position (at cursor)
    ui->radioButton_5->setChecked(true); // Default click type to left click
    ui->radioButton_2->setChecked(true); // Default to stop after max clicks



    connect(clickTimer, &QTimer::timeout, this, &MainWindow::performClick);

    // Connect UI elements
    connect(ui->radioButton_5, &QRadioButton::toggled, this, &MainWindow::updateClickType);
    connect(ui->radioButton_2, &QRadioButton::toggled, this, &MainWindow::updateClickCount);
    connect(ui->radioButton_3, &QRadioButton::toggled, this, &MainWindow::updateClickCount);
    connect(ui->spinBox_6, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateMaxClicks);
    connect(ui->spinBox_5, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateFixedDelay);
    connect(ui->spinBox_3, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateRandomMinDelay);
    connect(ui->spinBox_4, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateRandomMaxDelay);


    connect(ui->comboBox_4, &QComboBox::currentTextChanged, this, &MainWindow::updateMaxClicksUnit);

    // Connect combobox for fixed delay units
    connect(ui->comboBox_3, &QComboBox::currentTextChanged, this, &MainWindow::updateFixedDelayUnit);

    // Connect combobox for random delay units
    connect(ui->comboBox_2, &QComboBox::currentTextChanged, this, &MainWindow::updateRandomDelayUnit);

    // Handle delay selection logic
    connect(ui->spinBox_5, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::handleDelaySelection);
    connect(ui->spinBox_3, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::handleDelaySelection);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F10) {
        if (clicking) {
            qDebug() << "Stopping clicking...";
            stopClicking();
        } else {
            qDebug() << "Starting clicking...";
            startClicking();
        }
    }
}

void MainWindow::startClicking()
{
    clicking = true;
    clickCount = 0; // Reset click count
    maxClicks = ui->radioButton_2->isChecked() ? 0 : ui->spinBox_6->value(); // Forever or stop after

    randomDelay = ui->spinBox_3->value() > 0 || ui->spinBox_4->value() > 0;
    // Convert fixed delay to milliseconds based on the selected unit
    fixedDelay = convertToMilliseconds(ui->spinBox_5->value(), ui->comboBox_3->currentText());

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

    // Handle random delays if applicable
    if (randomDelay) {
        // Ensure randomMinDelay and randomMaxDelay are valid
        if (randomMinDelay > 0 && randomMaxDelay > randomMinDelay) {
            int randomDelayDuration = QRandomGenerator::global()->bounded(randomMinDelay, randomMaxDelay);
            clickTimer->start(randomDelayDuration);
            qDebug() << "Random delay set for next click:" << randomDelayDuration << "ms";
        } else {
            qDebug() << "Invalid random delay settings. Using fixed delay instead.";
            clickTimer->start(fixedDelay);
        }
    } else {
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

    if (ui->radioButton_5->isChecked()) {
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
    qDebug() << "Click type updated to:" << (ui->radioButton_5->isChecked() ? "Left" : "Right");
}

void MainWindow::updateClickCount()
{
    if (ui->radioButton_2->isChecked()) {
        maxClicks = 0; // Forever
        qDebug() << "Max clicks set to: Forever";
    } else {
        maxClicks = ui->spinBox_6->value(); // Set to spin box value
        qDebug() << "Max clicks set to:" << maxClicks;
    }
}

void MainWindow::updateMaxClicks(int value)
{
    maxClicks =  convertToMilliseconds(value, ui->comboBox_4->currentText());
    qDebug() << "Max clicks updated to:" << maxClicks;
}

void MainWindow::updateFixedDelay(int value)
{
    // Convert the fixed delay to milliseconds
    fixedDelay = convertToMilliseconds(value, ui->comboBox_3->currentText());
    qDebug() << "Fixed delay updated to:" << fixedDelay << "ms";
}

void MainWindow::updateRandomMinDelay(int value)
{
    // Convert the random min delay to milliseconds
    randomMinDelay = convertToMilliseconds(value, ui->comboBox_2->currentText());
    qDebug() << "Random min delay updated to:" << randomMinDelay << "ms";
}

void MainWindow::updateRandomMaxDelay(int value)
{
    // Convert the random max delay to milliseconds
    randomMaxDelay = convertToMilliseconds(value, ui->comboBox_2->currentText());
    qDebug() << "Random max delay updated to:" << randomMaxDelay << "ms";
}

void MainWindow::updateMaxClicksUnit()
{
    // Update the fixed delay based on the new unit
    updateMaxClicks(ui->spinBox_6->value());
}

void MainWindow::updateFixedDelayUnit()
{
    // Update the fixed delay based on the new unit
    updateFixedDelay(ui->spinBox_5->value());
}

void MainWindow::updateRandomDelayUnit()
{
    // Update both random min and max delays based on the new unit
    updateRandomMinDelay(ui->spinBox_3->value());
    updateRandomMaxDelay(ui->spinBox_4->value());
}

void MainWindow::handleDelaySelection()
{
    bool fixedDelayEnabled = ui->spinBox_5->value() > 0;
    bool randomDelayEnabled = ui->spinBox_3->value() > 0 && ui->spinBox_4->value() > ui->spinBox_3->value();

    // Disable one delay type based on the other
    ui->spinBox_5->setEnabled(!randomDelayEnabled);
    ui->comboBox_3->setEnabled(!randomDelayEnabled);  // Disable fixed delay unit comboBox

    ui->spinBox_3->setEnabled(!fixedDelayEnabled);
    ui->spinBox_4->setEnabled(!fixedDelayEnabled);
    ui->comboBox_2->setEnabled(!fixedDelayEnabled);  // Disable random delay unit comboBox
}
