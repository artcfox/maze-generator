/*
 *  dragscrollarea.cpp
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#include "dragscrollarea.h"

#include <QMouseEvent>
#include <QScrollBar>

DragScrollArea::DragScrollArea(QWidget *parent)
    : QScrollArea(parent)
{

}

void DragScrollArea::mousePressEvent(QMouseEvent *event)
{
    lastPosition = event->pos();
    if ((dragMode == DragScrollArea::DragMode::ScrollHandDrag) && (event->button() == Qt::LeftButton)) {
        // Left-button press in scroll hand mode initiates hand scrolling.
        event->accept();
        handScrolling = true;
        originalCursor = viewport()->cursor();
        viewport()->setCursor(Qt::ClosedHandCursor);
    }

}

void DragScrollArea::mouseMoveEvent(QMouseEvent *event)
{
    if ((dragMode == DragScrollArea::DragMode::ScrollHandDrag) && handScrolling) {
        QScrollBar *hBar = horizontalScrollBar();
        QScrollBar *vBar = verticalScrollBar();
        QPoint delta = event->pos() - lastPosition;
        hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
        vBar->setValue(vBar->value() - delta.y());
    }
    lastPosition = event->pos();
}

void DragScrollArea::mouseReleaseEvent(QMouseEvent *event)
{
    if ((dragMode == DragScrollArea::DragMode::ScrollHandDrag) && (event->button() == Qt::LeftButton)) {
        handScrolling = false;
        viewport()->setCursor(originalCursor);
    }
}

DragScrollArea::DragMode DragScrollArea::getDragMode() const
{
    return dragMode;
}

void DragScrollArea::setDragMode(const DragScrollArea::DragMode &value)
{
    dragMode = value;
}
