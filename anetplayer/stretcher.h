#ifndef STRETCHER_H
#define STRETCHER_H

#include <QDialog>
#include <QResizeEvent>
#include <QFont>
#include <QTimer>
#include <QLabel>

class LabelStretcher : public QObject {
    Q_OBJECT

public:
    LabelStretcher(QObject *parent = nullptr) : QObject(parent) {}

    void apply(QWidget *widget) {
        if (!widget) return;
        setMinimumSizeForWidget(widget);
        widget->installEventFilter(this);
        widgets.append(widget);
    }

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override {
        if (ev->type() == QEvent::Resize) {
            resizeAll();
        }
        return false;
    }

private:
    QList<QWidget*> widgets;

    void resizeAll() {
        if (widgets.isEmpty()) return;

        qreal minFontSize = std::numeric_limits<qreal>::max();
        for (auto widget : widgets) {
            auto fontSize = widget->font().pointSizeF();
            minFontSize = std::min(minFontSize, calculateFontSize(widget, fontSize));
        }

        for (auto widget : widgets) {
            auto font = widget->font();
            font.setPointSizeF(minFontSize);
            widget->setFont(font);
        }
    }

    qreal calculateFontSize(QWidget *widget, qreal initialSize) {
        qreal dStep = 1.0;
        int i;
        qreal fontSize = initialSize;
        for (i = 0; i < 10; ++i) {
            auto prevFontSize = fontSize;
            qreal d = df(fontSize, dStep, widget);
            if (d == 0) {
                dStep *= 2.0;
                continue;
            }
            fontSize -= f(fontSize, widget) / d;
            fontSize = std::max(dStep + 1.0, fontSize);
            auto change = fabs(prevFontSize - fontSize) / fontSize;
            if (change < 0.01) break;
        }
        return fontSize;
    }

    qreal f(qreal fontSize, QWidget *widget) {
        auto font = widget->font();
        font.setPointSizeF(fontSize);
        widget->setFont(font);
        return std::max(widget->sizeHint().width() - widget->width(), widget->sizeHint().height() - widget->height());
    }

    qreal df(qreal fontSize, qreal dStep, QWidget *widget) {
        fontSize = std::max(dStep + 1.0, fontSize);
        return (f(fontSize + dStep, widget) - f(fontSize - dStep, widget)) / dStep;
    }

    void setMinimumSizeForWidget(QWidget *widget) {
        if (widget) {
            widget->setMinimumSize(widget->minimumSizeHint());
        }
    }
};

#endif // STRETCHER_H
