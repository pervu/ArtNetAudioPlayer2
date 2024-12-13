#include "filemanager.h"
#include <QFile>
#include "artnetsender.h"

FileManager::FileManager(QObject *parent) : QObject(parent)
{
}

bool FileManager::savePlaylist(const QVector<CueButton*>& buttons, QGridLayout *gridLayout)
{
    if (!gridLayout) return false;

    QString fileName = QFileDialog::getSaveFileName(nullptr, "Save playlist", "", "ANet Playlist Files (*.plist)");
    if (fileName.isEmpty()) return false;

    int maxRow = 0;
    int maxCol = 0;

    for (int row = 0; row < gridLayout->rowCount(); ++row) {
        for (int col = 0; col < gridLayout->columnCount(); ++col) {
            QLayoutItem *item = gridLayout->itemAtPosition(row, col);
            if (item && item->widget()) {
                maxRow = qMax(maxRow, row);
                maxCol = qMax(maxCol, col);
            }
        }
    }

    // Добавляем +1, потому что строки и столбцы начинаются с 0
    int realRows = ++maxRow;
    int realCols = ++maxCol;

    QSettings settings(fileName, QSettings::IniFormat);

    settings.beginGroup("CueConfig");

    settings.setValue("rows", realRows);
    settings.setValue("columns", realCols);

    for (int i = 0; i < buttons.size(); ++i)
    {
        QString key = QString("cue%1").arg(i);
        settings.beginGroup(key);
        saveButtonSettings(settings, buttons[i]);
        settings.endGroup();
    }

    settings.endGroup();
    return true;
}


QPair<int, int> FileManager::loadPlaylist(QVector<CueButton*>& buttons, QGridLayout* gridLayout, QWidget* parent)
{
    if (!gridLayout) return QPair<int, int>(-1, -1);

    QString fileName = QFileDialog::getOpenFileName(nullptr, "Load playlist", "", "ANet Playlist Files (*.plist)");
    if (fileName.isEmpty()) return QPair<int, int>(-1, -1);

    // Clear the layout and remove the buttons
    QLayoutItem *item;
    while ((item = gridLayout->takeAt(0)) != nullptr) {
        gridLayout->removeItem(item);
    }
    foreach (CueButton *but, buttons) {
        but->stopPlayback(); // Just in case, stop the playback
        delete but;
    }
    buttons.clear();

    QSettings settings(fileName, QSettings::IniFormat);
    settings.beginGroup("CueConfig");

    int rows = settings.value("rows", 0).toInt();
    int columns = settings.value("columns", 0).toInt();

    int totalButtons = rows * columns;
    while (buttons.size() < totalButtons) {
        CueButton *button = new CueButton(parent);
        buttons.append(button);
    }

    for (int i = 0; i < buttons.size(); ++i) {
        QString key = QString("cue%1").arg(i);
        settings.beginGroup(key);
        loadButtonSettings(settings, buttons[i]);
        settings.endGroup();
    }

    settings.endGroup();
    return QPair<int, int>(rows, columns);
}

bool FileManager::saveSettings(const QString &settingsFileName, const settings_t &settings)
{
    QSettings settingsFile(settingsFileName, QSettings::IniFormat);
    settingsFile.beginGroup("Settings");

    settingsFile.setValue("rows", settings.rows);
    settingsFile.setValue("columns", settings.columns);
    settingsFile.setValue("fps", settings.fps);
    settingsFile.setValue("slectedInterfaceName", settings.slectedInterfaceName);
    settingsFile.setValue("ip", settings.ip);
    settingsFile.setValue("port", settings.port);
    settingsFile.setValue("tcOut", settings.tcOut);

    settingsFile.endGroup();
    return true;
}

bool FileManager::loadSettings(const QString &settingsFileName, settings_t &settings)
{
    QSettings settingsFile(settingsFileName, QSettings::IniFormat);

    // Check if the file exists
    if (!QFile::exists(settingsFileName))
    {
        // If the file doesn't exist, create it with default settings
        settingsFile.beginGroup("settings");
        settingsFile.setValue("rows", 3);
        settingsFile.setValue("columns", 3);
        settingsFile.setValue("fps", "30");
        QStringList interfaces = ArtNetSender::getAvailableInterfaces();
        settingsFile.setValue("slectedInterfaceName", interfaces.first());
        settingsFile.setValue("ip", "127.0.0.1");
        settingsFile.setValue("port", 6454);
        settingsFile.setValue("tcOut", 1);
        settingsFile.endGroup();
    }

    settingsFile.beginGroup("Settings");

    settings.rows = settingsFile.value("rows", 3).toInt();
    settings.columns = settingsFile.value("columns", 3).toInt();
    settings.fps = settingsFile.value("fps", "30").toString();
    settings.slectedInterfaceName = settingsFile.value("slectedInterfaceName", "127.0.0.1").toString();
    settings.ip = settingsFile.value("ip", "127.0.0.1").toString();
    settings.port = settingsFile.value("port", 6454).toInt();
    settings.tcOut = settingsFile.value("tcOut", 1).toBool();

    settingsFile.endGroup();

    return true;
}

void FileManager::saveButtonSettings(QSettings& settings, CueButton* button)
{
    settings.setValue("fileName", button->getFilePath());
    settings.setValue("adjustmentTime", button->getAdjustmentTime());
    settings.setValue("frameRate", button->getFrameRate());
    settings.setValue("cueColor", button->getCueColor().name());
}

void FileManager::loadButtonSettings(QSettings& settings, CueButton* button)
{
    QString filePath = settings.value("fileName").toString();
    qint64 adjustmentTime = settings.value("adjustmentTime").toLongLong();
    QString frameRate = settings.value("frameRate").toString();
    QColor color(settings.value("cueColor").toString());

    if (!filePath.isEmpty()) {
        button->setAdjustmentTime(adjustmentTime);
        button->setFilePath(filePath);
        button->setFrameRate(frameRate);
        button->setCueColor(color);
    }
}
