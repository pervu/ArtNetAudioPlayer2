#include "cuebutton.h"

CueButton::CueButton(QWidget *parent)
    : QPushButton(parent), player(new QMediaPlayer(this)), timer(new QTimer(this))
{
    // Set the size policy to allow the button to expand
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Default system button color
    cueColor = palette().color(QPalette::Button);

    // Create and bind an audio output device
    QAudioOutput *audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    player->setPlaybackRate(1.0); // Standard playback speed (1x)
    // Track the current playback position of the file
    connect(player, &QMediaPlayer::positionChanged, this, &CueButton::onPositionChanged);
    // Connect timer to update the playback time
    connect(timer, &QTimer::timeout, this, &CueButton::updateTime);
    // Connect left mouse button click to start playback
    connect(this, &QPushButton::clicked, this, &CueButton::playFile);
    // Track when playback reaches the end
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &CueButton::onMediaStatusChanged);
}

CueButton::~CueButton()
{
    if (timer) {
        timer->stop();
        delete timer;
        timer = nullptr;
    }

    if (player) {
        player->stop();
        delete player;
        player = nullptr;
    }
}

void CueButton::adjustTimeDialog(bool addTime)
{
    bool ok;
    QString input = QInputDialog::getText(this,
                                          addTime ? "Add Time" : "Subtract Time",
                                          "Enter time (hh:mm:ss:ff):",
                                          QLineEdit::Normal,
                                          "00:00:00:00",
                                          &ok);

    if (ok && validateTimeFormat(input))
    {
        adjustmentTimeMs = timeStringToMilliseconds(input);
        timeAdjustmentSign = addTime ? 1 : -1;  // Set the time adjustment sign

        QString sign = addTime ? "+" : "-";
        timeAdjustmentDisplay = QString("%1%2").arg(sign, input);

        setFileNameText(fileName); // Update the button text
    }
    else if (ok)
    {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid time format (hh:mm:ss:ff).");
    }
}

bool CueButton::validateTimeFormat(const QString &timeString)
{
    static QRegularExpression regex("^\\d{2}:\\d{2}:\\d{2}:\\d{2}$");
    return regex.match(timeString).hasMatch();
}

qint64 CueButton::timeStringToMilliseconds(const QString &timeString)
{
    QStringList parts = timeString.split(':');
    if (parts.size() != 4) {
        return 0; // Invalid time format, return 0
    }

    timecode_t tc;
    tc.hh = parts[0].toInt();
    tc.mm = parts[1].toInt();
    tc.ss = parts[2].toInt();
    tc.ff = parts[3].toInt();
    tc.fps = fps;

    return tcconverter.tc2milliseconds(tc);
}

void CueButton::contextMenuEvent(QContextMenuEvent *event)
{
    // Create the context menu
    QMenu menu(this);

    // Add actions to the menu
    QAction *selectFileAction = menu.addAction("Select audio file");
    QAction *clearTextAction = menu.addAction("Delete");
    QAction *addTimeAction = menu.addAction("Add Time");
    QAction *subtractTimeAction = menu.addAction("Subtract Time");
    QAction *setCueColorAction = menu.addAction("Set Cue Color");

    // Connect actions to slots
    connect(selectFileAction, &QAction::triggered, this, &CueButton::selectFile);
    connect(clearTextAction, &QAction::triggered, this, &CueButton::clear);
    connect(addTimeAction, &QAction::triggered, this, [this]() { adjustTimeDialog(true); });
    connect(subtractTimeAction, &QAction::triggered, this, [this]() { adjustTimeDialog(false); });
    connect(setCueColorAction, &QAction::triggered, this, &CueButton::chooseButtonColor);

    // Display the menu at the cursor position
    menu.exec(event->globalPos());
}

void CueButton::setFileNameText(const QString &fileName)
{
    QFontMetrics metrics(font());
    QString elidedText = metrics.elidedText(fileName, Qt::ElideRight, width() - 30); // Truncate file name

    QString displayText = elidedText;
    if (!timeAdjustmentDisplay.isEmpty())
    {
        displayText += "\n" + timeAdjustmentDisplay; // Add time adjustment string
    }
    else
    {
        displayText += "\n+00:00:00:00"; // Add default time adjustment string
    }

    setText(displayText); // Set button text
}

void CueButton::selectFile()
{
    // Open the file selection dialog 
    filePath = QFileDialog::getOpenFileName(this, "Select a file", "", "Audio files (*.mp3 *.wav *.ogg);;All files (*.*)");
    fileName = QUrl::fromLocalFile(filePath).fileName();
    // If a file is selected, set its name as the button's text
    if (!fileName.isEmpty())
    {
        setFileNameText(fileName);
    }
}

void CueButton::clear()
{
    // Send a signal to clear the text on the button
    emit requestClear(this);
}

void CueButton::playFile()
{

    if (filePath.isEmpty()) {
        return;
    }
    else if (!QFile::exists(filePath))
    {
        emit playingStatus("ERROR! Can't play file. File path doesn't exixsts.");
        return;
    }

    player->setSource(QUrl::fromLocalFile(filePath));
    duration = player->duration();
    if (duration == 0)
    {
        emit playingStatus("ERROR! File cannot be played: " + fileName);
        return;
    }
    player->setPosition(0);
    lastKnownPosition = 0; // Reset position
    player->play();

    timer->start(1);  // Update every 1 ms
    elapsedTimer.invalidate(); // Stop timer

    emit playbackStarted(this); // Notify that playback has started
    emit playingStatus("Playing  " + fileName);
}

void CueButton::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia){
        this->stopPlayback(); // Stop the timer and reset the state
        emit playingStatus("Stopped " + fileName);
    }
    else if (status == QMediaPlayer::InvalidMedia)
    {
        timer->stop();
        elapsedTimer.invalidate();
        emit playingStatus("ERROR! File cannot be played: " + fileName);
    }
}

void CueButton::updateTime()
{
    if (!elapsedTimer.isValid()) return;
    qint64 currentPosition = lastKnownPosition + elapsedTimer.elapsed(); // Interpolate
    // Get timecode format from millisecs
    timecode_t tc = tcconverter.milliseconds2tc(currentPosition, fps);

    static uint8_t prevff = 0;
    if (tc.ff != prevff)
    {
        prevff = tc.ff;

        QString audioTime = tcconverter.tc2string(tc);
        // Apply time correction for calculating Art-Net TC
        qint64 adjustedTimeMs = currentPosition + timeAdjustmentSign * adjustmentTimeMs;

        timecode_t anettc = tcconverter.milliseconds2tc(adjustedTimeMs, fps);
        QString anetTime = tcconverter.tc2string(anettc) + QString(":%1").arg(static_cast<int>(fps));

        int sliderValue = static_cast<int>((currentPosition * 1000) / duration);
        emit updatePlayTime(audioTime, anetTime, sliderValue);
    }
}

void CueButton::onPositionChanged(qint64 position)
{
    lastKnownPosition = position;
    elapsedTimer.restart();
}


void CueButton::chooseButtonColor()
{
    // Open the color selection dialog and return the selected color
    cueColor = QColorDialog::getColor(Qt::white, this, "Select a color");

    if (cueColor.isValid())
    {
        // Apply the color only to the selected button, so assign a unique identifier to the button
        this->setObjectName(QString("cueButton_%1").arg(reinterpret_cast<quintptr>(this)));
        // Apply the style only to this button
        this->setStyleSheet(QStringLiteral("#%1 { background-color: %2; }")
                                .arg(this->objectName(), cueColor.name()));
    }
}

void CueButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    if (!fileName.isEmpty())
        setFileNameText(fileName); // Update the text according to the new size
}

void CueButton::stopPlayback()
{
    if (player)
        player->stop();
    if (timer)
        timer->stop();
    elapsedTimer.invalidate(); // Timer stop
    lastKnownPosition = 0;
    emit playingStatus("Stopped  " + fileName);
}

void CueButton::startPlayback()
{
    if (!player->isPlaying())
    {
        player->play();
        timer->start(1);
        emit playingStatus("Playing  " + fileName);
    }
}

void CueButton::pausePlayback()
{
    if (player->isPlaying())
    {
        player->pause();
        timer->stop();
        elapsedTimer.invalidate(); // Timer reset
    }
    emit playingStatus("Paused  " + fileName);
}

void CueButton::setFrameRate(const QString &framerate)
{
    if (framerate == "24") fps = 24;
    else if (framerate == "25") fps = 25;
    else if (framerate == "29.97") fps = 29.97;
    else if (framerate == "30") fps = 30;
    else fps = 30;
}

void CueButton::setPlaybackPosition(qint64 position)
{
    player->setPosition(position);
}

qint64 CueButton::getDuration() const
{
    return player->duration();
}

QString CueButton::getFilePath() const
{
    return filePath;
}

qint64 CueButton::getAdjustmentTime() const
{
    if (timeAdjustmentSign > 0)
        return adjustmentTimeMs;
    else
        return -1*adjustmentTimeMs;
}

QString CueButton::getFrameRate() const
{
    return QString::number(fps);
}

void CueButton::setFilePath(const QString &path)
{
    filePath = path;
    fileName = QUrl::fromLocalFile(filePath).fileName();
    setFileNameText(fileName);
}

void CueButton::setAdjustmentTime(qint64 timeMs)
{
    adjustmentTimeMs = qAbs(timeMs);

    timeAdjustmentSign = (timeMs >= 0) ? 1 : -1;
    QString sign = (timeMs >= 0) ? "+" : "-";

    QTime time(0, 0);
    time = time.addMSecs(qAbs(timeMs));

    // Calculate the frame number, considering the frame rate
    int frames = (qAbs(timeMs) % 1000) * fps / 1000;

    // Форматируем строку времени
    QString formattedTime = QString("%1%2:%3")
                                .arg(sign)
                                .arg(time.toString("hh:mm:ss"))
                                .arg(frames, 2, 10, QChar('0'));

    timeAdjustmentDisplay = formattedTime;
}

void CueButton::setCueColor(QColor color)
{
    if (color.isValid())
    {
        // Set a unique name for the button
        this->setObjectName(QString("cueButton_%1").arg(reinterpret_cast<quintptr>(this)));
        // Apply the style only to this button
        this->setStyleSheet(QStringLiteral("#%1 { background-color: %2; }")
                                .arg(this->objectName(), color.name()));
        cueColor = color;
    }
}

QColor CueButton::getCueColor()
{
    return cueColor;
}

QString CueButton::getUiFramerate()
{
    int framerate = static_cast<int>(fps);
    switch (framerate) {
    case 24:
        return QString("24ndf");
    case 25:
        return QString("25ndf");
    case 30:
        return QString("30ndf");
    case 29:
        return QString("29df");

    default:
        return QString("00df");
    }
    return "";
}
