#include "settings.h"
#include "ui_settings.h"
#include <QFile>

Settings::Settings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);
    setdat = new settings_t;

    QStringList interfaces = ArtNetSender::getAvailableInterfaces();
    ui->comboBox_nwInterfaces->addItems(interfaces);

    // Set list of artnet framerate
    QStringList fps = {
        "24",
        "25",
        "29.97",
        "30"
    };
    ui->comboBox_fps->addItems(fps);

    fileManager= new FileManager(this);
    // Read parameters from the file if they exist, otherwise use default values
    settings_t loadedSettings;
    fileManager->loadSettings("config.ini", loadedSettings);
    // Display the parameters in the settings window
    ui->checkBox_isTC->setChecked(loadedSettings.tcOut);
    ui->comboBox_fps->setCurrentText(loadedSettings.fps);
    ui->comboBox_nwInterfaces->setCurrentText(loadedSettings.slectedInterfaceName);
    ui->lineEdit_ip->setText(loadedSettings.ip);
    ui->lineEdit_port->setText(QString::number(loadedSettings.port));
    ui->spinBox_columns->setValue(loadedSettings.columns);
    ui->spinBox_rows->setValue(loadedSettings.rows);
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_pushButton_Cancel_clicked()
{
    this->close();
}


void Settings::on_pushButton_Ok_clicked()
{
    setdat->rows = ui->spinBox_rows->text().toUShort();
    setdat->columns = ui->spinBox_columns->text().toUShort();
    setdat->ip = ui->lineEdit_ip->text();
    setdat->port = ui->lineEdit_port->text().toUInt();
    setdat->slectedInterfaceName = ui->comboBox_nwInterfaces->currentText();
    setdat->fps = ui->comboBox_fps->currentText();
    setdat->tcOut = ui->checkBox_isTC->checkState();
    emit settingsData(*setdat);

    // Save settings to file
    fileManager->saveSettings("config.ini", *setdat);

    this->close();
}

