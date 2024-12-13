#include "tcwindow.h"
#include "ui_tcwindow.h"

#define RESIZE_MARGIN 8  // Ширина зоны изменения размера окна

TCwindow::TCwindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TCwindow)
{
    ui->setupUi(this);

    // Enable mouse tracking
    this->setMouseTracking(true);

    // Apply LabelStretcher
    stretcher = new LabelStretcher(this);  // Create an object for stretching
    stretcher->apply(ui->label_anetTc);  // Apply stretching to the specific QLabel
    stretcher->apply(ui->label_audioTc);

    ui->label_audioTc->setStyleSheet("QLabel { color: green; }");
    ui->label_anetTc->setStyleSheet("QLabel { color: darkblue; }");

    // Remove the window title
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    // Set a black background
    setStyleSheet("QDialog { background-color: black; }");
    // Set initial window size
    resize(600, 320);
}

TCwindow::~TCwindow()
{
    delete ui;
}

void TCwindow::onTcReceived(const QString &audiotc, const QString &anettc)
{
    ui->label_anetTc->setText(anettc);
    ui->label_audioTc->setText(audiotc);
}

void TCwindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QRect rect = this->rect();
        if (event->pos().x() >= rect.width() - RESIZE_MARGIN) {
            resizeDirection = Qt::RightEdge;
        } else if (event->pos().x() <= RESIZE_MARGIN) {
            resizeDirection = Qt::LeftEdge;
        } else if (event->pos().y() >= rect.height() - RESIZE_MARGIN) {
            resizeDirection = Qt::BottomEdge;
        } else if (event->pos().y() <= RESIZE_MARGIN) {
            resizeDirection = Qt::TopEdge;
        } else {
            resizeDirection = Qt::Edges(); // Никакая грань не выбрана
        }

        if (resizeDirection != Qt::Edges()) {
            isResizing = true;
        } else {
            isMousePressed = true;
            lastMousePosition = event->globalPosition().toPoint() - this->pos();
        }

        event->accept();
    } else if (event->button() == Qt::RightButton) {
        showContextMenu(event->globalPosition().toPoint());
        event->accept();
    } else {
        QDialog::mousePressEvent(event);
    }
    QDialog::mousePressEvent(event);
}

void TCwindow::mouseMoveEvent(QMouseEvent *event) {
    if (isResizing) {
        QRect rect = this->geometry();
        QPoint globalPos = event->globalPosition().toPoint();

        if (resizeDirection == Qt::RightEdge) {
            int newWidth = globalPos.x() - rect.left();
            if (newWidth >= minimumWidth()) {
                rect.setWidth(newWidth);
            }
        } else if (resizeDirection == Qt::LeftEdge) {
            int newLeft = globalPos.x();
            int newWidth = rect.right() - newLeft;
            if (newWidth >= minimumWidth()) {
                rect.setLeft(newLeft);
            }
        } else if (resizeDirection == Qt::BottomEdge) {
            int newHeight = globalPos.y() - rect.top();
            if (newHeight >= minimumHeight()) {
                rect.setHeight(newHeight);
            }
        } else if (resizeDirection == Qt::TopEdge) {
            int newTop = globalPos.y();
            int newHeight = rect.bottom() - newTop;
            if (newHeight >= minimumHeight()) {
                rect.setTop(newTop);
            }
        }

        this->setGeometry(rect);
        event->accept();
    }
    else if (isMousePressed) {
        this->move(event->globalPosition().toPoint() - lastMousePosition);
        event->accept();
    } else {
        // Show the resize cursor if the mouse enters the edge area
        QRect rect = this->rect();
        if (event->pos().x() >= rect.width() - RESIZE_MARGIN) {
            setCursor(Qt::SizeHorCursor);
        } else if (event->pos().x() <= RESIZE_MARGIN) {
            setCursor(Qt::SizeHorCursor);
        } else if (event->pos().y() >= rect.height() - RESIZE_MARGIN) {
            setCursor(Qt::SizeVerCursor);
        } else if (event->pos().y() <= RESIZE_MARGIN) {
            setCursor(Qt::SizeVerCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
        QDialog::mouseMoveEvent(event);
    }
}

void TCwindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isMousePressed = false;
        isResizing = false;
        resizeDirection = Qt::Edges();
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QDialog::mouseReleaseEvent(event);
    }
}

// Display context menu on mouse right button
void TCwindow::showContextMenu(const QPoint &pos) {
    QMenu contextMenu(this);

    // Set the text for QAction depending on the visibility of the labels
    QString anetActionText = ui->label_anetTc->isVisible() ? "Hide AnetTC" : "Show AnetTC";
    QString audioActionText = ui->label_audioTc->isVisible() ? "Hide AudioTC" : "Show AudioTC";

    QAction *toggleAnetAction = new QAction(anetActionText, &contextMenu);
    QAction *toggleAudioAction = new QAction(audioActionText, &contextMenu);
    QAction *fullScreenAction = new QAction("FullScreen", &contextMenu);
    QAction *closeAction = new QAction("Close", &contextMenu);

    // Connect actions to slots
    connect(toggleAnetAction, &QAction::triggered, this, &TCwindow::toggleAnetTC);
    connect(toggleAudioAction, &QAction::triggered, this, &TCwindow::toggleAudioTC);
    connect(fullScreenAction, &QAction::triggered, this, &TCwindow::toggleFullScreen);
    connect(closeAction, &QAction::triggered, this, &TCwindow::close);

    contextMenu.addAction(toggleAnetAction);
    contextMenu.addAction(toggleAudioAction);
    contextMenu.addSeparator(); // Разделитель
    contextMenu.addAction(fullScreenAction);
    contextMenu.addAction(closeAction);

    contextMenu.exec(pos);
}

// Fullscreen
void TCwindow::toggleFullScreen() {
    if (isFullScreen()) {
        showNormal();
        resize(640, 480);
    } else {
        showFullScreen();
    }
}

// Enable/disable visibility of the AnetTC
void TCwindow::toggleAnetTC() {
    bool isVisible = ui->label_anetTc->isVisible();
    ui->label_anetTc->setVisible(!isVisible);
}

// Enable/disable visibility of the AudioTC
void TCwindow::toggleAudioTC() {
    bool isVisible = ui->label_audioTc->isVisible();
    ui->label_audioTc->setVisible(!isVisible);
}
