#ifndef TCWINDOW_H
#define TCWINDOW_H

#include "stretcher.h"
#include <QDialog>
#include <QMenu>
#include <QMouseEvent>

namespace Ui {
class TCwindow;
}

class TCwindow : public QDialog {
    Q_OBJECT

public:
    explicit TCwindow(QWidget *parent = nullptr);
    ~TCwindow();

public slots:
    void onTcReceived(const QString &audiotc, const QString &anettc);

signals:
    void windowClosed();

protected:
    void mousePressEvent(QMouseEvent *event) override;  // Handle mouse press event
    void mouseMoveEvent(QMouseEvent *event) override;   // Handle mouse move event
    void mouseReleaseEvent(QMouseEvent *event) override; // Handle mouse release event

private:
    Ui::TCwindow *ui;
    LabelStretcher *stretcher;

    bool isMousePressed = false;      // Flag indicating mouse press
    QPoint lastMousePosition;         // Last mouse cursor position

    bool isResizing = false;
    Qt::Edges resizeDirection = Qt::Edges();

    void showContextMenu(const QPoint &pos);
    void toggleFullScreen();
    void toggleAnetTC();
    void toggleAudioTC();
};

#endif // TCWINDOW_H
