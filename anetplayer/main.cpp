#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

// Function to load and apply QSS
void applyStyleSheet(QApplication &app, const QString &path) {
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        app.setStyleSheet(styleSheet);
        file.close();
    } else {
        qDebug() << "Failed to load stylesheet:" << path;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    w.setWindowTitle("Art-Net Timecode Player 2");

    qApp->setStyle(QStyleFactory::create("Fusion"));

    w.show();
    return a.exec();
}
