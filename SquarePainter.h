#ifndef SQUAREPAINTER_H
#define SQUAREPAINTER_H

#include <QVideoWidget>
#include <QPainter>
#include <QEvent>

class SquarePainter : public QObject {
public:
    explicit SquarePainter(QVideoWidget *videoWidget) : QObject(videoWidget), videoWidget(videoWidget) {
        videoWidget->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (watched == videoWidget && event->type() == QEvent::Paint) {
            drawGrid();
        }
        return false;
    }

    void drawGrid() {
        QPainter painter(videoWidget);
        painter.setPen(Qt::gray); // Set grid color

        // Draw vertical lines
        int width = videoWidget->width();
        int height = videoWidget->height();
        int numCols = 5; // Number of columns
        int colWidth = width / numCols;
        for (int i = 1; i < numCols; ++i) {
            int x = i * colWidth;
            painter.drawLine(x, 0, x, height);
        }

        // Draw horizontal lines
        int numRows = 5; // Number of rows
        int rowHeight = height / numRows;
        for (int i = 1; i < numRows; ++i) {
            int y = i * rowHeight;
            painter.drawLine(0, y, width, y);
        }
    }

public:
    void clearGrid() {
        videoWidget->repaint();
}

private:
    QVideoWidget *videoWidget;
};


#endif // SQUAREPAINTER_H
