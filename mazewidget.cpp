/*
 *  mazewidget.cpp
 *  MazeGenerator
 *
 *  Copyright 2018-2024 Matthew T. Pandina. All rights reserved.
 *
 */

#include "mazewidget.h"
#include "generatemazeworker.h"
#include "deletemazeworker.h"
#include "openmazeworker.h"
#include "savemazeworker.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QCoreApplication>
#include <QMessageBox>
#include <QtEndian>

MazeWidget::MazeWidget(QWidget *parent) : QWidget(parent)
{
    workerThread.start();
    generateMaze();
}

MazeWidget::~MazeWidget()
{
    // Ensure free() gets called from the same thread that malloc did, though the app may exit before this can happen
    DeleteMazeWorker *worker = new DeleteMazeWorker(myMaze);
    worker->moveToThread(&workerThread);
    connect(worker, &DeleteMazeWorker::deleteMazeWorker_error, this, &MazeWidget::deleteMazeWorker_error);
    connect(worker, &DeleteMazeWorker::deleteMazeWorker_finished, this, &MazeWidget::deleteMazeWorker_finished);
    connect(worker, &DeleteMazeWorker::deleteMazeWorker_finished, worker, &DeleteMazeWorker::deleteLater);
    connect(this, &MazeWidget::deleteMazeWorker_start, worker, &DeleteMazeWorker::process);
    emit deleteMazeWorker_start();
    workerThread.quit();
    workerThread.requestInterruption();
    workerThread.wait();
}

void MazeWidget::generateMaze()
{
    creatingMaze = true;

    GenerateMazeWorker *worker = new GenerateMazeWorker(myMaze, mazeWidth, mazeHeight);
    myMaze = 0; // clear our copy
    solutionLength = 0;
    worker->moveToThread(&workerThread);
    connect(worker, &GenerateMazeWorker::generateMazeWorker_error, this, &MazeWidget::generateMazeWorker_error);
    connect(worker, &GenerateMazeWorker::generateMazeWorker_finished, this, &MazeWidget::generateMazeWorker_finished);
    connect(worker, &GenerateMazeWorker::generateMazeWorker_finished, worker, &GenerateMazeWorker::deleteLater);

    // For progress indicators
    connect(worker, &GenerateMazeWorker::generateMazeWorker_deletingOldMaze, this, &MazeWidget::generateMazeWorker_deletingOldMaze);
    connect(worker, &GenerateMazeWorker::generateMazeWorker_allocatingMemory, this, &MazeWidget::generateMazeWorker_allocatingMemory);
    connect(worker, &GenerateMazeWorker::generateMazeWorker_generatingMaze, this, &MazeWidget::generateMazeWorker_generatingMaze);
    connect(worker, &GenerateMazeWorker::generateMazeWorker_solvingMaze, this, &MazeWidget::generateMazeWorker_solvingMaze);

    connect(this, &MazeWidget::generateMazeWorker_start, worker, &GenerateMazeWorker::process);
    emit generateMazeWorker_start();

    resetWidgetSize(); // pre-maturely resize the widget to the expected size
    update();
}

void MazeWidget::openMaze()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Maze..."), QString(), "Maze Files (*.maze)" );
    if (fileName.isNull())
        return;

    creatingMaze = true;

    OpenMazeWorker *worker = new OpenMazeWorker(myMaze, fileName);
    myMaze = 0; // clear our copy
    solutionLength = 0;
    worker->moveToThread(&workerThread);
    connect(worker, &OpenMazeWorker::openMazeWorker_error, this, &MazeWidget::openMazeWorker_error);
    connect(worker, &OpenMazeWorker::openMazeWorker_finished, this, &MazeWidget::openMazeWorker_finished);
    connect(worker, &OpenMazeWorker::openMazeWorker_finished, worker, &OpenMazeWorker::deleteLater);
    connect(worker, &OpenMazeWorker::openMazeWorker_error, worker, &OpenMazeWorker::deleteLater);

    // For progress indicators
    connect(worker, &OpenMazeWorker::openMazeWorker_deletingOldMaze, this, &MazeWidget::openMazeWorker_deletingOldMaze);
    connect(worker, &OpenMazeWorker::openMazeWorker_allocatingMemory, this, &MazeWidget::openMazeWorker_allocatingMemory);
    connect(worker, &OpenMazeWorker::openMazeWorker_loadingMaze, this, &MazeWidget::openMazeWorker_loadingMaze);

    connect(this, &MazeWidget::openMazeWorker_start, worker, &OpenMazeWorker::process);
    emit openMazeWorker_start();

    update();
}

void MazeWidget::resetWidgetSize()
{
    setMinimumWidth((mazeWidth + 1) * gridSpacing * scaling);
    setMaximumWidth((mazeWidth + 1) * gridSpacing * scaling);
    setMinimumHeight((mazeHeight + 1) * gridSpacing * scaling);
    setMaximumHeight((mazeHeight + 1) * gridSpacing * scaling);
}

bool MazeWidget::getDebug() const
{
    return debug;
}

void MazeWidget::setDebug(bool value)
{
    debug = value;
}

uint32_t MazeWidget::getHighlight() const
{
    return highlight;
}

void MazeWidget::setHighlight(const uint32_t &value)
{
    highlight = value;
}

bool MazeWidget::getSavingMaze() const
{
    return savingMaze;
}

uint32_t MazeWidget::getSolutionLength() const
{
    return solutionLength;
}

bool MazeWidget::getInverse() const
{
    return inverse;
}

void MazeWidget::setInverse(bool value)
{
    inverse = value;
    update();
}

bool MazeWidget::getAntialiased() const
{
    return antialiased;
}

void MazeWidget::setAntialiased(bool value)
{
    antialiased = value;
    update();
}

QRect MazeWidget::scaleRect(const QRect &rect)
{
    if (scaling >= 1.0)
        return QRect(rect.left() / scaling, rect.top() / scaling, rect.width() * scaling, rect.height() * scaling);
    else
        return QRect(rect.left() / scaling, rect.top() / scaling, rect.width() / scaling, rect.height() / scaling);
}

qreal MazeWidget::getScaling() const
{
    return scaling;
}

void MazeWidget::setScaling(const qreal &value)
{
    scaling = value;
    resetWidgetSize();
    update();
}

void MazeWidget::paintBackground(QPainter *painter, const QRect &rect)
{
    QBrush whiteBrush(Qt::white);
    painter->setPen(Qt::NoPen);
    painter->setBrush(whiteBrush);
    QRect backgroundRect(0, 0, (mazeWidth + 1) * gridSpacing, (mazeHeight + 1) * gridSpacing);
    painter->drawRect(backgroundRect.intersected(rect));

    if (showMaze) {
        QBrush blackBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->setBrush(blackBrush);
        QRect mazeWalls((wallThickness + 1) / 2, (wallThickness + 1) / 2, ((mazeWidth + 1) * gridSpacing) - wallThickness, ((mazeHeight + 1) * gridSpacing) - wallThickness);
        painter->drawRect(mazeWalls.intersected(rect));
    }
}

void MazeWidget::paintPathBackground(QPainter *painter, const QRect &rect)
{
    QBrush whiteBrush(Qt::white);
    painter->setPen(Qt::NoPen);
    painter->setBrush(whiteBrush);
    //QRect backgroundRect(0, 0, (mazeWidth + 1) * gridSpacing, (mazeHeight + 1) * gridSpacing);
    painter->drawRect(/*backgroundRect.intersected(*/rect/*)*/);
}

void MazeWidget::paintMazePaths(QPainter *painter, const QRect &rect)
{
    if (creatingMaze) // make safe for something external to call
        return;

    QPainterPath mazePath;

    // Convert from view coordinates into maze coordinates
    int startX = ((rect.left()) / gridSpacing) - 1 - 1;
    if (startX < 0)
        startX = 0;

    int startY = ((rect.top()) / gridSpacing) - 1 - 1;
    if (startY < 0)
        startY = 0;

    int endX = (((rect.right())) / gridSpacing) + 1;
    if (endX > mazeWidth)
        endX = mazeWidth;

    int endY = (((rect.bottom())) / gridSpacing) + 1;
    if (endY > mazeHeight)
        endY = mazeHeight;

    // Draw entrance and exit
    mazePath.moveTo(((1) * gridSpacing), ((0) * gridSpacing));
    mazePath.lineTo(((1) * gridSpacing), ((1) * gridSpacing));
    mazePath.moveTo(((mazeWidth) * gridSpacing), ((mazeHeight) * gridSpacing));
    mazePath.lineTo(((mazeWidth) * gridSpacing), ((mazeHeight + 1) * gridSpacing));

    // Draw horizontal paths in the maze
    BitArrayRef connected = myMaze->halls[0];
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            uint32_t position = y * mazeWidth + x; // convert (x, y) coordinates into a scalar position
            if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                mazePath.moveTo(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                int offset = 0;
                while (x + offset + 1 < endX) { // see if we can extend this line more
                    position++; // move to the right one square
                    if (BitArray_readBit(connected, position)) // are the position and the one next to it connected?
                        offset++; // extend the endpoint of the line
                    else
                        break; // there is a wall, the line needs to end
                }
                mazePath.lineTo(((x + (offset + 1) + 1) * gridSpacing), ((y + 1) * gridSpacing));
                x += offset; // if we were able to extend the line, adjust the index variable
            }
        }
    }

    // Draw vertical paths in the maze
    connected = myMaze->halls[1];
    for (int x = startX; x < endX; ++x) {
        for (int y = startY; y < endY; ++y) {
            uint32_t position = y * mazeWidth + x;
            if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                mazePath.moveTo(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                int offset = 0;
                while (y + offset + 1 < endY) { // see if we can extend this line more
                    position += mazeWidth; // move down one square
                    if (BitArray_readBit(connected, position)) // are the position and the one next to it connected?
                        offset++; // extend the endpoint of the line
                    else
                        break; // there is a wall, the line needs to end
                }
                mazePath.lineTo(((x + 1) * gridSpacing), ((y + (offset + 1) + 1) * gridSpacing));
                y += offset; // if we were able to extend the line, adjust the index variable
            }
        }
    }

    QPen hallPen(Qt::black);
    hallPen.setWidth(wallThickness);
    if (roundCaps) {
        hallPen.setCapStyle(Qt::RoundCap);
        hallPen.setJoinStyle(Qt::RoundJoin);
    } else {
        hallPen.setCapStyle(Qt::SquareCap);
        hallPen.setJoinStyle((Qt::MiterJoin));
    }

    painter->setPen(hallPen);
    painter->drawPath(mazePath);
}

void MazeWidget::paintMazeWalls(QPainter *painter, const QRect &rect)
{
    if (creatingMaze) // make safe for something external to call
        return;

    QPainterPath mazePath;

    // Convert from view coordinates into maze coordinates
    int startX = ((rect.left()) / gridSpacing) - 1 - 1;
    if (startX < 0)
        startX = 0;

    int startY = ((rect.top()) / gridSpacing) - 1 - 1;
    if (startY < 0)
        startY = 0;

    int endX = (((rect.right())) / gridSpacing) + 1;
    if (endX > mazeWidth)
        endX = mazeWidth;

    int endY = (((rect.bottom())) / gridSpacing) + 1;
    if (endY > mazeHeight)
        endY = mazeHeight;

    // Draw the border around the maze
    mazePath.moveTo(0.5 * gridSpacing, 0.5 * gridSpacing);
    mazePath.lineTo(0.5 * gridSpacing, (mazeHeight + 0.5) * gridSpacing);
    mazePath.lineTo((mazeWidth - 1 + 0.5) * gridSpacing, (mazeHeight + 0.5) * gridSpacing);
    mazePath.moveTo((mazeWidth + 0.5) * gridSpacing, (mazeHeight + 0.5) * gridSpacing);
    mazePath.lineTo((mazeWidth + 0.5) * gridSpacing, 0.5 * gridSpacing);
    mazePath.lineTo((1 + 0.5) * gridSpacing, 0.5 * gridSpacing);

    // Draw vertical walls in the maze
    BitArrayRef connected = myMaze->halls[0];
    for (int x = startX; x < endX - 1; ++x) {
        for (int y = startY; y < endY; ++y) {
            uint32_t position = y * mazeWidth + x; // convert (x, y) coordinates into a scalar position
            if (!BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                mazePath.moveTo(((x + 1.5) * gridSpacing), ((y + 0.5) * gridSpacing));
                int offset = 0;
                while (y + offset + 1 < endY) { // see if we can extend this line more
                    position += mazeWidth; // move down one square
                    if (!BitArray_readBit(connected, position)) // are the position and the one next to it connected?
                        offset++; // extend the endpoint of the line
                    else
                        break; // there is a wall, the line needs to end
                }
                mazePath.lineTo(((x + 1.5) * gridSpacing), ((y + (offset + 1.5)) * gridSpacing));
                y += offset; // if we were able to extend the line, adjust the index variable
            }
        }
    }

    // Draw horizontal walls in the maze
    connected = myMaze->halls[1];
    for (int y = startY; y < endY - 1; ++y) {
        for (int x = startX; x < endX; ++x) {
            uint32_t position = y * mazeWidth + x;
            if (!BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                mazePath.moveTo(((x + 0.5) * gridSpacing), ((y + 1.5) * gridSpacing));
                int offset = 0;
                while (x + offset + 1 < endX) { // see if we can extend this line more
                    position++; // move to the right one square
                    if (!BitArray_readBit(connected, position)) // are the position and the one next to it connected?
                        offset++; // extend the endpoint of the line
                    else
                        break; // there is a wall, the line needs to end
                }
                mazePath.lineTo(((x + (offset + 1.5)) * gridSpacing), ((y + 1.5) * gridSpacing));
                x += offset; // if we were able to extend the line, adjust the index variable
            }
        }
    }

    QPen hallPen(Qt::black);
    hallPen.setWidth(wallThickness);
    if (roundCaps) {
        hallPen.setCapStyle(Qt::RoundCap);
        hallPen.setJoinStyle(Qt::RoundJoin);
    } else {
        hallPen.setCapStyle(Qt::SquareCap);
        hallPen.setJoinStyle((Qt::MiterJoin));
    }

    painter->setPen(hallPen);
    painter->drawPath(mazePath);
}

void MazeWidget::paintDebug(QPainter *painter, const QRect &rect)
{
    QPainterPath debugPath;

    // Convert from view coordinates into maze coordinates
    int startX = ((rect.left()) / gridSpacing) - 1 - 1;
    if (startX < 0)
        startX = 0;

    int startY = ((rect.top()) / gridSpacing) - 1 - 1;
    if (startY < 0)
        startY = 0;

    int endX = (((rect.right())) / gridSpacing) + 1;
    if (endX > mazeWidth)
        endX = mazeWidth;

    int endY = (((rect.bottom())) / gridSpacing) + 1;
    if (endY > mazeHeight)
        endY = mazeHeight;

    // Draw a circle highlighting a position set with the debug menu
    for (int x = startX; x < endX; ++x) {
        for (int y = startY; y < endY; ++y) {
            uint32_t position = y * mazeWidth + x; // convert (x, y) coordinates into a scalar position
            if (position == highlight)
                debugPath.addEllipse(QPoint( ((x + 1) * gridSpacing), ((y + 1) * gridSpacing) ), gridSpacing / 4, gridSpacing / 4);
        }
    }

    QPen debugPen(Qt::blue);
    QBrush debugBrush(Qt::yellow);
    debugPen.setWidth(2);
    painter->setPen(debugPen);
    painter->setBrush(debugBrush);
    painter->drawPath(debugPath);
}

void MazeWidget::paintSolution(QPainter *painter, const QRect &rect)
{
    if (creatingMaze) // make safe for something external to call
        return;

    QPainterPath solutionPath;

    // Convert from view coordinates into maze coordinates
    int startX = ((rect.left()) / gridSpacing) - 1 - 1;
    if (startX < 0)
        startX = 0;

    int startY = ((rect.top()) / gridSpacing) - 1 - 1;
    if (startY < 0)
        startY = 0;

    int endX = (((rect.right())) / gridSpacing) + 1;
    if (endX > mazeWidth)
        endX = mazeWidth;

    int endY = (((rect.bottom())) / gridSpacing) + 1;
    if (endY > mazeHeight)
        endY = mazeHeight;

    // Draw solution above entrance and exit
    solutionPath.moveTo(((1) * gridSpacing), ((0) * gridSpacing));
    solutionPath.lineTo(((1) * gridSpacing), ((1) * gridSpacing));
    solutionPath.moveTo(((mazeWidth) * gridSpacing), ((mazeHeight) * gridSpacing));
    solutionPath.lineTo(((mazeWidth) * gridSpacing), ((mazeHeight + 1) * gridSpacing));

    // Draw horizontal paths in the solution
    BitArrayRef connected = myMaze->solution[0];
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            uint32_t position = y * mazeWidth + x; // convert (x, y) coordinates into a scalar position
            if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                solutionPath.moveTo(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                int offset = 0;
                while (x + offset + 1 < endX) { // see if we can extend this line more
                    position++; // move to the right one square
                    if (BitArray_readBit(connected, position)) // are the position and the one next to it connected?
                        offset++; // extend the endpoint of the line
                    else
                        break; // there is a wall, the line needs to end
                }
                solutionPath.lineTo(((x + (offset + 1) + 1) * gridSpacing), ((y + 1) * gridSpacing));
                x += offset; // if we were able to extend the line, adjust the index variable
            }
        }
    }

    // Draw vertical paths in the solution
    connected = myMaze->solution[1];
    for (int x = startX; x < endX; ++x) {
        for (int y = startY; y < endY; ++y) {
            uint32_t position = y * mazeWidth + x;
            if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                solutionPath.moveTo(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                int offset = 0;
                while (y + offset + 1 < endY) { // see if we can extend this line more
                    position += mazeWidth; // move down one square
                    if (BitArray_readBit(connected, position)) // are the position and the one next to it connected?
                        offset++; // extend the endpoint of the line
                    else
                        break; // there is a wall, the line needs to end
                }
                solutionPath.lineTo(((x + 1) * gridSpacing), ((y + (offset + 1) + 1) * gridSpacing));
                y += offset; // if we were able to extend the line, adjust the index variable
            }
        }
    }

    QPen solutionPen(Qt::red);
    solutionPen.setWidth(solutionThickness);
    if (roundCaps)
        solutionPen.setCapStyle(Qt::RoundCap);
    else
        solutionPen.setCapStyle(Qt::SquareCap);

    painter->setPen(solutionPen);
    painter->drawPath(solutionPath);
}

void MazeWidget::printMaze()
{
    QPrinter printer;
    printer.setResolution(600);
    printer.setPageMargins(QMarginsF(0.25, 0.25, 0.25, 0.25), QPageLayout::Inch);
    printer.setPageSize(QPageSize(QPageSize::Letter));

    if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
        QPainter painter;
        painter.begin(&printer);

        qreal scaleToFit = qMin((qreal)painter.viewport().width() / width(), (qreal)painter.viewport().height() / height());
        painter.scale(scaleToFit * scaling, scaleToFit * scaling);

        if (antialiased)
            painter.setRenderHint(QPainter::Antialiasing);

        QRect rect(0, 0, ((mazeWidth + 1) * gridSpacing), ((mazeHeight + 1) * gridSpacing));

        if (creatingMaze) {
            QBrush brush(Qt::white);
            painter.setPen(Qt::NoPen);
            painter.setBrush(brush);
            painter.drawRect(scaleRect(rect));
        } else {
            if (showMaze) {
                if (inverse)
                    paintMazePaths(&painter, scaleRect(rect));
                else
                    paintMazeWalls(&painter, scaleRect(rect));
            }
            if (showSolution)
                paintSolution(&painter, scaleRect(rect));
        }
        painter.end();
    }
}

void MazeWidget::exportImage()
{
    QString fileName= QFileDialog::getSaveFileName(this, tr("Export Image..."), QCoreApplication::applicationDirPath(), "BMP Files (*.bmp)" );
    if (!fileName.isNull())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().isEmpty())
            fileName.append(".bmp");

        QImage image(((mazeWidth + 1) * gridSpacing) * scaling, ((mazeHeight + 1) * gridSpacing) * scaling, QImage::Format_ARGB32_Premultiplied);
        QPainter painter;
        if (!painter.begin(&image)) {
            QMessageBox::warning(this, "Error Exporting Image", "There was an error exporting the image; the maze is most likely too large.\n\nYou may configure its rendered size using the View > Maze Styles menu, or create a new maze with smaller dimensions.");
            return;
        }

        painter.scale(scaling, scaling);
        if (antialiased)
            painter.setRenderHint(QPainter::Antialiasing);

        QRect rect(0, 0, ((mazeWidth + 1) * gridSpacing), ((mazeHeight + 1) * gridSpacing));

        if (creatingMaze) {
            QBrush brush(Qt::gray);
            painter.setPen(Qt::NoPen);
            painter.setBrush(brush);
            painter.drawRect(scaleRect(rect));
        } else {
            paintPathBackground(&painter, scaleRect(rect));
            if (showMaze) {
                if (inverse)
                    paintMazePaths(&painter, scaleRect(rect));
                else
                    paintMazeWalls(&painter, scaleRect(rect));
            }
            if (showSolution)
                paintSolution(&painter, scaleRect(rect));
        }

        painter.end();
        image.save(fileName);
    }
}

void MazeWidget::saveMazeAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Maze..."), QString(), "Maze Files (*.maze)" );

    if (fileName.isNull())
        return;

    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().isEmpty())
        fileName.append(".maze");

    savingMaze = true;

    SaveMazeWorker *worker = new SaveMazeWorker(myMaze, fileName);
    worker->moveToThread(&workerThread);
    connect(worker, &SaveMazeWorker::saveMazeWorker_error, this, &MazeWidget::saveMazeWorker_error);
    connect(worker, &SaveMazeWorker::saveMazeWorker_finished, this, &MazeWidget::saveMazeWorker_finished);
    connect(worker, &SaveMazeWorker::saveMazeWorker_finished, worker, &SaveMazeWorker::deleteLater);
    connect(worker, &SaveMazeWorker::saveMazeWorker_error, worker, &SaveMazeWorker::deleteLater);

    // For progress indicators
    connect(worker, &SaveMazeWorker::saveMazeWorker_savingMaze, this, &MazeWidget::saveMazeWorker_savingMaze);

    connect(this, &MazeWidget::saveMazeWorker_start, worker, &SaveMazeWorker::process);
    emit saveMazeWorker_start();
}

void MazeWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);

    painter.scale(scaling, scaling);
    if (antialiased)
        painter.setRenderHint(QPainter::Antialiasing);

    if (creatingMaze) {
        QBrush brush(Qt::gray);
        painter.setPen(Qt::NoPen);
        painter.setBrush(brush);
        painter.drawRect(scaleRect(event->rect()));
    } else {
        if (showMaze) {
            if (inverse)
                for(QRegion::const_iterator i = event->region().begin(); i != event->region().end(); i++)
                    paintMazePaths(&painter, scaleRect(*i));
            else
                for(QRegion::const_iterator i = event->region().begin(); i != event->region().end(); i++)
                    paintMazeWalls(&painter, scaleRect(*i));
        }
        if (showSolution)
            for(QRegion::const_iterator i = event->region().begin(); i != event->region().end(); i++)
                paintSolution(&painter, scaleRect(*i));
        if (debug)
            for(QRegion::const_iterator i = event->region().begin(); i != event->region().end(); i++)
                paintDebug(&painter, scaleRect(*i));
    }

    painter.end();
}

bool MazeWidget::getRoundCaps() const
{
    return roundCaps;
}

void MazeWidget::setRoundCaps(bool value)
{
    roundCaps = value;
    update();
}

void MazeWidget::generateMazeWorker_deletingOldMaze()
{
    emit on_deletingOldMaze();
}

void MazeWidget::generateMazeWorker_allocatingMemory()
{
    emit on_allocatingMemory();
}

void MazeWidget::generateMazeWorker_generatingMaze()
{
    emit on_generatingMaze();
}

void MazeWidget::generateMazeWorker_solvingMaze()
{
    emit on_solvingMaze();
}

void MazeWidget::generateMazeWorker_finished(void *maze)
{
    creatingMaze = false;
    solutionLength = ((MazeRef)maze)->solutionLength;
    myMaze = (MazeRef)maze;
    resetWidgetSize();
    update();
    emit on_mazeCreated();
}

void MazeWidget::generateMazeWorker_error(QString err)
{
    (void)err;
    //qDebug() << "generateMazeWorker_error: " << err;
}

void MazeWidget::deleteMazeWorker_finished(void *maze)
{
    myMaze = (MazeRef)maze;
}

void MazeWidget::deleteMazeWorker_error(QString err)
{
    (void)err;
    //qDebug() << "deleteMazeWorker_error: " << err;
}

void MazeWidget::openMazeWorker_deletingOldMaze()
{
    emit on_deletingOldMaze();
}

void MazeWidget::openMazeWorker_allocatingMemory()
{
    emit on_allocatingMemory();
}

void MazeWidget::openMazeWorker_loadingMaze(int width, int height)
{
    setMazeWidth(width);
    setMazeHeight(height);
    resetWidgetSize();
    update();
    emit on_openMaze();
}

void MazeWidget::openMazeWorker_finished(void *maze)
{
    creatingMaze = false;
    solutionLength = ((MazeRef)maze)->solutionLength;
    myMaze = (MazeRef)maze;
    resetWidgetSize();
    update();
    emit on_mazeCreated();
}

void MazeWidget::openMazeWorker_error(void *maze, QString err)
{
    // Avoid displaying the busy cursor when displaying the error message
    emit on_openMazeError(err);

    // Restore the old maze, since the new one couldn't be loaded
    creatingMaze = false;
    solutionLength = ((MazeRef)maze)->solutionLength;
    myMaze = (MazeRef)maze;
    resetWidgetSize();
    update();

    // Display the error message, and re-enable the menus
    QMessageBox::warning(this, "Error Opening Maze", err);
    emit on_openMazePostError();
}

void MazeWidget::saveMazeWorker_savingMaze()
{
    emit on_savingMaze();
}

void MazeWidget::saveMazeWorker_finished()
{
    savingMaze = false;
    emit on_mazeSaved();
}

void MazeWidget::saveMazeWorker_error(QString err)
{
    savingMaze = false;
    emit on_saveMazeError(err);
    QMessageBox::warning(this, "Error Saving Maze", err);
}

bool MazeWidget::getShowMaze() const
{
    return showMaze;
}

void MazeWidget::setShowMaze(bool value)
{
    showMaze = value;
    update();
}

bool MazeWidget::getShowSolution() const
{
    return showSolution;
}

void MazeWidget::setShowSolution(bool value)
{
    showSolution = value;
    update();
}

int MazeWidget::getSolutionThickness() const
{
    return solutionThickness;
}

void MazeWidget::setSolutionThickness(int value)
{
    solutionThickness = value;
    update();
}

void MazeWidget::resetDefaultSpacing()
{
    gridSpacing = DEFAULT_GRID_SPACING;
    wallThickness = DEFAULT_WALL_THICKNESS;
    solutionThickness = DEFAULT_SOLUTION_THICKNESS;
    resetWidgetSize();
    update();
}

int MazeWidget::getWallThickness() const
{
    return wallThickness;
}

void MazeWidget::setWallThickness(int value)
{
    wallThickness = value;
    update();
}

int MazeWidget::getGridSpacing() const
{
    return gridSpacing;
}

void MazeWidget::setGridSpacing(int value)
{
    gridSpacing = value;
    resetWidgetSize();
    update();
}

int MazeWidget::getMazeHeight() const
{
    return mazeHeight;
}

void MazeWidget::setMazeHeight(int value)
{
    mazeHeight = value;
}

int MazeWidget::getMazeWidth() const
{
    return mazeWidth;
}

void MazeWidget::setMazeWidth(int value)
{
    mazeWidth = value;
}

