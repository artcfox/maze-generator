/*
 *  dragscrollarea.h
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef DRAGSCROLLAREA_H
#define DRAGSCROLLAREA_H

#include <QScrollArea>
#include <QPoint>
#include <QCursor>

class DragScrollArea : public QScrollArea
{
    Q_PROPERTY(DragMode dragMode READ dragMode WRITE setDragMode)

public:
    enum DragMode {
        NoDrag,
        ScrollHandDrag
    };
    Q_ENUM(DragMode)

    explicit DragScrollArea(QWidget *parent = nullptr);

    DragScrollArea::DragMode getDragMode() const;
    void setDragMode(const DragScrollArea::DragMode &value);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPoint lastPosition;
    QCursor originalCursor;

    DragMode dragMode = DragMode::ScrollHandDrag;
    bool handScrolling = false;
};

#endif // DRAGSCROLLAREA_H
