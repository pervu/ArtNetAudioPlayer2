#include "about.h"
#include "ui_about.h"

#define PROGRAM_VER "v. 2.0.0"

about::about(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::about)
{
    ui->setupUi(this);

    ui->label_ver->setText(PROGRAM_VER);

    ui->label_siteLink->setText("More projects: <a href=\"https://pervu.github.io/projects.html\">pervu.github.io</a>");
    ui->label_siteLink->setTextFormat(Qt::RichText); // HTML
    ui->label_siteLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->label_siteLink->setOpenExternalLinks(true);

    ui->label_mailto->setText("mailto: <a href=\"mailto:pervushkinp@gmail.com\">pervushkinp@gmail.com</a>");
    ui->label_mailto->setTextFormat(Qt::RichText);
    ui->label_mailto->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->label_mailto->setOpenExternalLinks(true);

    QPixmap pixmap;
    pixmap.load(":anet2.ico");
    QPixmap scaledPixmap = pixmap.scaled(ui->label_ico->size(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
    ui->label_ico->setPixmap(scaledPixmap);
}

about::~about()
{
    delete ui;
}

void about::on_pushButton_Ok_clicked()
{
    this->close();
}

