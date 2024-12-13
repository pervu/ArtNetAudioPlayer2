#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QVector>
#include <QSettings>
#include <QFileDialog>
#include <QGridLayout>
#include "cuebutton.h"
#include "struct.h"

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);

    // Save load playlist
    bool savePlaylist(const QVector<CueButton*>& buttons, QGridLayout* layout);
    QPair<int, int> loadPlaylist(QVector<CueButton*>& buttons, QGridLayout* gridLayout, QWidget* parent);

    // Save load common settings
    bool saveSettings(const QString &settingsFileName, const settings_t &settings);
    bool loadSettings(const QString &settingsFileName, settings_t &settings);

private:
    void saveButtonSettings(QSettings& settings, CueButton* button);
    void loadButtonSettings(QSettings& settings, CueButton* button);
};

#endif // FILEMANAGER_H
