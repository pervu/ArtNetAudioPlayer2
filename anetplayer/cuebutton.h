#ifndef CUEBUTTON_H
#define CUEBUTTON_H

#include <QPushButton>
#include <QMenu>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QTime>
#include <QTimer>
#include <QAudioOutput>
#include <QElapsedTimer>
#include <QInputDialog>
#include <QRegularExpression>
#include <QMessageBox>
#include <QColorDialog>
#include "tcconverter.h"

class CueButton : public QPushButton
{
    Q_OBJECT

public:
    explicit CueButton(QWidget *parent = nullptr);
    ~CueButton();
    QMediaPlayer *player;

    void stopPlayback();
    void startPlayback();
    void pausePlayback();
    void setFrameRate(const QString &framerate);
    void setPlaybackPosition(qint64 position);
    qint64 getDuration() const;

    // Methods for loading the configuration
    QString getFilePath() const;
    qint64 getAdjustmentTime() const;
    QString getFrameRate() const;
    void setFilePath(const QString &path);
    void setAdjustmentTime(qint64 timeMs);
    void setCueColor(QColor color);
    QColor getCueColor();
    QString getUiFramerate();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override; // Handle right-click for the context menu

    void resizeEvent(QResizeEvent *event) override;

private slots:
    void selectFile(); // Slot for file selection
    void clear(); // Slot for clearing the button
    void playFile(); // Play the file
    void updateTime(); // Update the time
    void onPositionChanged(qint64 position);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void chooseButtonColor();

private:
    TCconverter tcconverter;  // Instance of the TCconverter class
    void setFileNameText(const QString &fileName);
    QString filePath; // Full path to audio file
    QString fileName;
    QTimer *timer;
    QElapsedTimer elapsedTimer;
    qint64 lastKnownPosition = 0;
    qint64 duration = 10000; // Total duration in milliseconds
    double fps = 30; // Art-Net frame rate

    void adjustTimeDialog(bool addTime);
    bool validateTimeFormat(const QString &timeString);
    qint64 timeStringToMilliseconds(const QString &timeString);
    qint64 adjustmentTimeMs = 0;  // Time adjustment in milliseconds
    QString timeAdjustmentDisplay; // Stores the displayed adjustment time
    int timeAdjustmentSign = 1; // 1 for addition, -1 for subtraction
    QColor cueColor;
    int counter = 0;

signals:
    void updatePlayTime(const QString &audioTime, const QString &tcTime, const int &sliderTime); // Signal to update the playback time
    void playbackStarted(CueButton *button);    // Signal indicating the start of playback
    void playingStatus(const QString &stat);
    void requestClear(CueButton *button);
};

#endif // CUEBUTTON_H
