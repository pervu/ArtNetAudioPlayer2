#include "mainwindow.h"
#include "ui_mainwindow.h"

#define TIMER_INTERVAL_MS 800
#define STATUSBAR_MSG_TIMEOUT_MS 1500
#define ICON_SIZE 36

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Settings window
    settingsForm = new Settings(this);
    settingsForm->setWindowTitle("Art-Net Timecode Player 2 Settings");
    // Send artnet tc to the network
    anet = new ArtNetSender(this);
    // Show audio and anet tc window
    tcwindow = new TCwindow(this);
    // Save/load playlists, common settings
    fileManager= new FileManager(this);

    // Load icons on the buttons
    QPixmap pixmap;
    pixmap.load(":play-button.png");
    ui->pushButton_Play->setIcon(pixmap);
    ui->pushButton_Play->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    pixmap.load(":pause-button.png");
    ui->pushButton_Pause->setIcon(pixmap);
    ui->pushButton_Pause->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    pixmap.load(":stop-button.png");
    ui->pushButton_Stop->setIcon(pixmap);
    ui->pushButton_Stop->setIconSize(QSize(ICON_SIZE, ICON_SIZE));

    // Init cenral widget and main layout
    setCentralWidget(ui->centralwidget);

    // Create qgridlayout
    gridLayout = new QGridLayout();
    ui->verticalLayout_cues->addLayout(gridLayout);

    // Poll the buffer and display messages in the StatusBar
    pollingTimer = new QTimer(this);
    connect(pollingTimer, &QTimer::timeout, this, &MainWindow::checkMsgBuffer);
    pollingTimer->start(TIMER_INTERVAL_MS);

    // Get data from the settings form
    connect(settingsForm, &Settings::settingsData, this, &MainWindow::onSettingsData);
    // Track slider movement by the user
    connect(ui->horizontalSliderPlayTime, &QSlider::sliderMoved, this, &MainWindow::onSliderMoved);
    // Get text from the ArtNetSender class
    connect(anet, &ArtNetSender::sendMsg, this, &MainWindow::on_msgReceived);
    // Send timecode to the tcWindow
    connect(this, &MainWindow::tcSignal, tcwindow, &TCwindow::onTcReceived);
    // Load configuration file config.ini
    loadSettingsFromFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createButtons(const uint8_t &rows, const uint8_t &columns, const QString &framerate)
{
    int currentButtons = buttons.size();

    for (int i = currentButtons; i < rows * columns; ++i)
    {
        CueButton *button = new CueButton(this);
        buttons.append(button);
        connectCues(button);
    }

    // Clear the layout and add buttons to the new grid
    QLayoutItem *item;
    while ((item = gridLayout->takeAt(0)) != nullptr) {
        gridLayout->removeItem(item);
    }

    for (int i = 0; i < buttons.size(); ++i)
    {
        int row = i / columns;
        int col = i % columns;
        gridLayout->addWidget(buttons[i], row, col);
        buttons[i]->show();
        // In the future, each button might have its own framerate (this might not be necessary)
        // Now, the framerate is the same for all buttons
        buttons[i]->setFrameRate(framerate);
    }
}

void MainWindow::onPlaybackStarted(CueButton *button)
{
    if (currentPlayingButton != nullptr && currentPlayingButton != button) {
        currentPlayingButton->stopPlayback(); // Stop the previous button
    }
    currentPlayingButton = button; // Assign the new button before playback
}

void MainWindow::updatePlayingTime(const QString &audioTime, const QString &tcTime, const int &sliderTimeValue)
{
    ui->labelAudioTime->setText(audioTime); // Update the playback time
    ui->horizontalSliderPlayTime->setValue(sliderTimeValue); // Update the slider
    QString nofpstc = "00:00:00:00"; // Set default timecode
    QString currentFPS = "00ndf";

    if (isTC){
        nofpstc = tcTime.left(tcTime.lastIndexOf(':')); // delete fps val from tc
        int fpsVal = tcTime.mid(tcTime.lastIndexOf(':') + 1).toInt();
        switch (fpsVal) {
        case 24:
            currentFPS = "24ndf";
            break;
        case 25:
            currentFPS = "25ndf";
            break;
        case 30:
            currentFPS = "30ndf";
            break;
        case 29:
            currentFPS = "29.97df";
            break;
        default:
            currentFPS = "00ndf";
            break;
        }
    }

    if (tcwindow && tcwindow->isVisible()) {
        emit tcSignal(audioTime, nofpstc);
    }

    ui->labelTcTime->setText(nofpstc);
    ui->label_fps->setText(currentFPS);

    //if (!isTC) return;  // If TC sending is off exit

    if ((anet)&&(isTC))
    {
        if (!anet->sendTime(tcTime))
        {
            msgBuffer.append("Invalid ArtNet initialization parameters.");
            currentPlayingButton->stopPlayback();
        }
    }
}

void MainWindow::onSettingsData(const settings_t &sett)
{
    adjustButtonCount(sett.rows, sett.columns);
    createButtons(sett.rows, sett.columns, sett.fps);
    if (anet)
    {
        anet->setTargetIP(sett.ip);
        anet->setTargetPort(sett.port);
        anet->setNetworkInterface(sett.slectedInterfaceName);
        msgBuffer.append("Settings applyed");
    }
    isTC = sett.tcOut;
}


void MainWindow::on_actionSettings_triggered()
{
    settingsForm->setWindowTitle("Settings Art-Net Timecode Player 2");
    settingsForm->show();
}

void MainWindow::adjustButtonCount(const uint8_t &rows, const uint8_t &columns)
{
    // Remove buttons if their new quantity is greater than the current button matrix
    int requiredButtons = rows * columns;
    while (buttons.size() > requiredButtons) {
        CueButton *button = buttons.takeLast();
        delete button;
    }
}

void MainWindow::onSliderMoved(int position)
{
    if (currentPlayingButton) {
        // Convert the slider value from the range [0, 1000] to milliseconds
        qint64 newPosition = (position * currentPlayingButton->getDuration()) / 1000;
        currentPlayingButton->setPlaybackPosition(newPosition);
    }
}

void MainWindow::on_pushButton_Play_clicked()
{
    if (currentPlayingButton){
        currentPlayingButton->startPlayback();
    }
}

void MainWindow::on_pushButton_Stop_clicked()
{
    if (currentPlayingButton){
        currentPlayingButton->stopPlayback();
        this->setUiDefaults();
    }
}

void MainWindow::on_pushButton_Pause_clicked()
{
    if (currentPlayingButton)
        currentPlayingButton->pausePlayback();
}

// Load playlist with file selection
void MainWindow::on_actionOpen_triggered()
{
    this->clearCues();
    QPair<int, int> matrix = fileManager->loadPlaylist(buttons, gridLayout, this);
    if (matrix.first != -1 && matrix.second != -1) {
        for (int i = 0; i < buttons.size(); ++i)
        {
            connectCues(buttons[i]);
            int row = i / matrix.first;
            int col = i % matrix.second;
            gridLayout->addWidget(buttons[i], row, col);
        }
        this->setUiDefaults();
        msgBuffer.append("Playlist loaded successfully");
    } else {
        msgBuffer.append("Failed to load playlist");
    }
}

// Save playlist with file selection
void MainWindow::on_actionSave_triggered()
{
    if (fileManager->savePlaylist(buttons, gridLayout)){
        msgBuffer.append("Playlist saved successfully");
    } else {
       msgBuffer.append("Failed to save playlist");
    }
}

void MainWindow::loadSettingsFromFile()
{
    // Read parameters from the file if they exist, otherwise use default values
    settings_t loadedSettings;
    fileManager->loadSettings("config.ini", loadedSettings);
    this->onSettingsData(loadedSettings);
}

void MainWindow::clearCues()
{
    currentPlayingButton = nullptr;
    this->setUiDefaults();
    // Clear the layout and remove the buttons
    QLayoutItem *item;
    while ((item = gridLayout->takeAt(0)) != nullptr) {
        gridLayout->removeItem(item);
    }
    foreach (CueButton *but, buttons) {
        but->stopPlayback();
        delete but;
    }
    buttons.clear();
}

void MainWindow::connectCues(CueButton *cueBut)
{
    // Connect signals
    connect(cueBut, &CueButton::playbackStarted, this, &MainWindow::onPlaybackStarted);
    connect(cueBut, &CueButton::updatePlayTime, this, &MainWindow::updatePlayingTime);
    // Get playing status
    connect(cueBut, &CueButton::playingStatus, this, [this](const QString &stat) {
        ui->label_PlayStatus->setText(stat);
    });
    // Delete selected cue
    connect(cueBut, &CueButton::requestClear, this, &MainWindow::onClearReceived);
}

void MainWindow::setUiDefaults()
{
    ui->horizontalSliderPlayTime->setSliderPosition(0);
    ui->labelAudioTime->setText("00:00:00:00");
    ui->labelTcTime->setText("00:00:00:00");
    ui->label_fps->setText("00ndf");
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionClear_Cues_triggered()
{
    clearCues();
    loadSettingsFromFile();
}

void MainWindow::on_msgReceived(const QString &msg)
{
    qDebug() << msg;
    msgBuffer.append(msg);
}

void MainWindow::checkMsgBuffer()
{
    if (!msgBuffer.isEmpty())
    {
        ui->statusBar->showMessage(msgBuffer.takeFirst(), STATUSBAR_MSG_TIMEOUT_MS);
    }
}

void MainWindow::on_actionTimeCode_Window_triggered()
{
    if (tcwindow) {
        tcwindow->setWindowTitle("Art-Net Timecode Player 2");
        tcwindow->show();
    }
}


void MainWindow::on_actionAbout_triggered()
{
    about aboutwindow;
    aboutwindow.setWindowTitle("About Art-Net Timecode Player 2");
    aboutwindow.exec();
}

void MainWindow::onClearReceived(CueButton *button)
{
    if (!button) return;
    currentPlayingButton = nullptr;
    this->setUiDefaults();

    // Find the index of the button to be removed in the buttons array
    int index = buttons.indexOf(button);
    if (index == -1) {
        qWarning() << "Button not found in the buttons array.";
        return;
    }

    buttons[index]->stopPlayback();
    buttons[index]->deleteLater(); // Remove the old button
    buttons[index] = nullptr; // Clear the pointer for safety

    // Create a new empty button and add it in place of the removed one
    auto *newButton = new CueButton(this);
    buttons[index] = newButton;
    // Connect all necessary signals for the new button
    connectCues(newButton);
    // Determine the position of the new button in the grid layout
    int row = index / gridLayout->columnCount();
    int col = index % gridLayout->columnCount();

    // Remove the old layout item (if any)
    QLayoutItem *item = gridLayout->itemAtPosition(row, col);
    if (item) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->deleteLater();
        }
        gridLayout->removeItem(item);
    }
    // Add the new button to the grid layout and show it
    gridLayout->addWidget(newButton, row, col);
    newButton->show();
}



