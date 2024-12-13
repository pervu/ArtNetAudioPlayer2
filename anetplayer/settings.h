#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include "struct.h"
#include "artnetsender.h"
#include "filemanager.h"

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

private slots:
    void on_pushButton_Cancel_clicked();
    void on_pushButton_Ok_clicked();

signals:
    void settingsData(const settings_t &sett);

private:
    Ui::Settings *ui;
    settings_t *setdat;
    FileManager *fileManager;
};

#endif // SETTINGS_H
