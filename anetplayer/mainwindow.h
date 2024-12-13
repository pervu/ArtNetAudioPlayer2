#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QVector>
#include <QProgressBar>
#include <QSettings>
#include "cuebutton.h"
#include "settings.h"
#include "artnetsender.h"
#include "tcwindow.h"
#include "about.h"
#include "filemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onPlaybackStarted(CueButton *button);
    void updatePlayingTime(const QString &audioTime, const QString &tcTime, const int &sliderTimeValue);
    void onSettingsData(const settings_t &sett);
    void on_actionSettings_triggered();
    void onSliderMoved(int position);
    void on_pushButton_Play_clicked();
    void on_pushButton_Stop_clicked();
    void on_pushButton_Pause_clicked();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionExit_triggered();
    void on_actionClear_Cues_triggered();
    void on_msgReceived(const QString &msg);    // Slot for receiving errors and other text from other classes
    void checkMsgBuffer();     // Slot for checking the buffer
    void on_actionTimeCode_Window_triggered();
    void on_actionAbout_triggered();
    void onClearReceived(CueButton *button);

private:
    Ui::MainWindow *ui;
    QGridLayout *gridLayout; // Layout for the buttons
    QVector<CueButton*> buttons; // Vector for storing all created buttons
    CueButton *currentPlayingButton = nullptr;

    Settings *settingsForm;  // Settings window
    ArtNetSender *anet;     // Sending timecode and network interface settings

    QTimer *pollingTimer;     // Timer for polling
    QStringList msgBuffer;    // Buffer for text messages

    TCwindow *tcwindow;  // Timecode output window

    bool isTC = true; // Timecode output to the network
    FileManager *fileManager; // Load save common settings, playlists

    void createButtons(const uint8_t &rows, const uint8_t &columns, const QString &framerate); // create Cues
    void adjustButtonCount(const uint8_t &rows, const uint8_t &columns);
    void loadSettingsFromFile();
    void clearCues();
    void connectCues(CueButton *cueBut);  // Button event tracking
    void setUiDefaults();


signals:
    void tcSignal(const QString &audiotc, const QString &anettc);
};

#endif // MAINWINDOW_H
