/*
 *  mazewidget.cpp
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#include "mazewidget.h"
#include "generatemazeworker.h"
#include "deletemazeworker.h"

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
    connect(worker, SIGNAL(deleteMazeWorker_error(QString)), this, SLOT(deleteMazeWorker_error(QString)));
    connect(worker, SIGNAL(deleteMazeWorker_finished(void*)), this, SLOT(deleteMazeWorker_finished(void*)));
    connect(worker, SIGNAL(deleteMazeWorker_finished(void*)), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(operate()), worker, SLOT(process()));
    emit operate();
}

void MazeWidget::generateMaze()
{
    if (creatingMaze) {
        qDebug() << "Still waiting for previous maze to be created!";
        return;
    }

    creatingMaze = true;

    GenerateMazeWorker *worker = new GenerateMazeWorker(myMaze, mazeWidth, mazeHeight);
    myMaze = 0; // clear our copy
    worker->moveToThread(&workerThread);
    connect(worker, SIGNAL(generateMazeWorker_error(QString)), this, SLOT(generateMazeWorker_error(QString)));
    connect(worker, SIGNAL(generateMazeWorker_finished(void*)), this, SLOT(generateMazeWorker_finished(void*)));
    connect(worker, SIGNAL(generateMazeWorker_finished(void*)), worker, SLOT(deleteLater()));

    // For progress indicators
    connect(worker, SIGNAL(generateMazeWorker_deletingOldMaze()), this, SLOT(generateMazeWorker_deletingOldMaze()));
    connect(worker, SIGNAL(generateMazeWorker_allocatingMemory()), this, SLOT(generateMazeWorker_allocatingMemory()));
    connect(worker, SIGNAL(generateMazeWorker_generatingMaze()), this, SLOT(generateMazeWorker_generatingMaze()));
    connect(worker, SIGNAL(generateMazeWorker_solvingMaze()), this, SLOT(generateMazeWorker_solvingMaze()));

    connect(this, SIGNAL(operate()), worker, SLOT(process()));
    emit operate();

    resetWidgetSize(); // pre-maturely resize the widget to the expected size
    update();
}

void MazeWidget::resetWidgetSize()
{
    setMinimumWidth((mazeWidth + 1) * gridSpacing);
    setMaximumWidth((mazeWidth + 1) * gridSpacing);
    setMinimumHeight((mazeHeight + 1) * gridSpacing);
    setMaximumHeight((mazeHeight + 1) * gridSpacing);
}


void MazeWidget::paintOnto(QPainter *painter, QPaintEvent *event)
{
    if (creatingMaze) {
        QBrush brush(Qt::gray);
        painter->setPen(Qt::NoPen);
        painter->setBrush(brush);
        painter->drawRect(event->rect());
        return;
    }

    QPainterPath mazePath;
    QPainterPath solutionPath;

    QBrush blackBrush(Qt::black);
    QBrush whiteBrush(Qt::white);

    QPen hallPen(Qt::white);
    hallPen.setWidth(hallThickness);

    QPen solutionPen(Qt::red);
    solutionPen.setWidth(solutionThickness);

    if (roundedPaths) {
        hallPen.setCapStyle(Qt::RoundCap);
        solutionPen.setCapStyle(Qt::RoundCap);
    } else {
        hallPen.setCapStyle(Qt::SquareCap);
        solutionPen.setCapStyle(Qt::SquareCap);
    }

    // Convert from view coordinates into maze coordinates
    int cell1[2], cell2[2];
    cell1[0] = ((event->rect().left()) / gridSpacing) - 1 - 1;
    if (cell1[0] < 0)
        cell1[0] = 0;

    cell1[1] = ((event->rect().top()) / gridSpacing) - 1 - 1;
    if (cell1[1] < 0)
        cell1[1] = 0;

    cell2[0] = (((event->rect().right())) / gridSpacing) + 1;
    if (cell2[0] > myMaze->dims[0])
        cell2[0] = myMaze->dims[0];

    cell2[1] = (((event->rect().bottom())) / gridSpacing) + 1;
    if (cell2[1] > myMaze->dims[1])
        cell2[1] = myMaze->dims[1];

    if (showMaze) {
        painter->setPen(hallPen);
        painter->setBrush(blackBrush);
        painter->drawRect(0, 0, ((myMaze->dims[0] + 1) * gridSpacing), ((myMaze->dims[1] + 1) * gridSpacing));

        // Draw entrance and exit
        mazePath.moveTo(((1) * gridSpacing), ((0) * gridSpacing));
        mazePath.lineTo(((1) * gridSpacing), ((1) * gridSpacing));
        mazePath.moveTo(((myMaze->dims[0]) * gridSpacing), ((myMaze->dims[1]) * gridSpacing));
        mazePath.lineTo(((myMaze->dims[0]) * gridSpacing), ((myMaze->dims[1] + 1) * gridSpacing));

        // Draw horizontal paths in the maze
        BitArrayRef connected = myMaze->halls[0];
        for (int y = cell1[1]; y < cell2[1]; ++y) {
            for (int x = cell1[0]; x < cell2[0]; ++x) {
                int position = y * myMaze->dims[0] + x; // convert (x, y) coordinates into a scalar position
                if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                    mazePath.moveTo(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                    int offset = 0;
                    while (x + 1 < cell2[0]) { // see if we can extend this line more
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
        for (int x = cell1[0]; x < cell2[0]; ++x) {
            for (int y = cell1[1]; y < cell2[1]; ++y) {
                int position = y * myMaze->dims[0] + x;
                if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                    mazePath.moveTo(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                    int offset = 0;
                    while (y + 1 < cell2[1]) { // see if we can extend this line more
                        position += myMaze->dims[0]; // move down one square
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
    } else {
        painter->setPen(hallPen);
        painter->setBrush(whiteBrush);
        painter->drawRect(0, 0, ((myMaze->dims[0] + 1) * gridSpacing), ((myMaze->dims[1] + 1) * gridSpacing));
    }

    if (showSolution) {
        // Draw solution above entrance and exit
        solutionPath.moveTo(((1) * gridSpacing), ((0) * gridSpacing));
        solutionPath.lineTo(((1) * gridSpacing), ((1) * gridSpacing));
        solutionPath.moveTo(((myMaze->dims[0]) * gridSpacing), ((myMaze->dims[1]) * gridSpacing));
        solutionPath.lineTo(((myMaze->dims[0]) * gridSpacing), ((myMaze->dims[1] + 1) * gridSpacing));

        // Draw horizontal paths in the solution
        BitArrayRef connected = myMaze->solution[0];
        for (int y = cell1[1]; y < cell2[1]; ++y) {
            for (int x = cell1[0]; x < cell2[0]; ++x) {
                int position = y * myMaze->dims[0] + x; // convert (x, y) coordinates into a scalar position
                if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                    solutionPath.moveTo(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                    int offset = 0;
                    while (x + 1 < cell2[0]) { // see if we can extend this line more
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
        for (int x = cell1[0]; x < cell2[0]; ++x) {
            for (int y = cell1[1]; y < cell2[1]; ++y) {
                int position = y * myMaze->dims[0] + x;
                if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                    solutionPath.moveTo(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                    int offset = 0;
                    while (y + 1 < cell2[1]) { // see if we can extend this line more
                        position += myMaze->dims[0]; // move down one square
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
    }

    painter->setPen(hallPen);
    painter->drawPath(mazePath);
    painter->setPen(solutionPen);
    painter->drawPath(solutionPath);
}

void MazeWidget::printMaze()
{
    QPrinter printer;
    if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
        QPainter painter;
        painter.begin(&printer);
        //painter.setRenderHint(QPainter::Antialiasing);
        QPaintEvent event(QRect(0, 0, ((mazeWidth + 1) * gridSpacing), ((mazeHeight + 1) * gridSpacing)));
        paintOnto(&painter, &event);
        painter.end();
    }
}

void MazeWidget::exportImage()
{
    QString fileName= QFileDialog::getSaveFileName(this, tr("Export Image..."), QCoreApplication::applicationDirPath(), "BMP (*.bmp)" );
    if (!fileName.isNull())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().isEmpty())
            fileName.append(".bmp");

        QImage image(((mazeWidth + 1) * gridSpacing), ((mazeHeight + 1) * gridSpacing), QImage::Format_ARGB32_Premultiplied);
        QPainter painter;
        if (!painter.begin(&image)) {
            QMessageBox::warning(this, "Error Exporting Image", "There was an error exporting the image; the maze is most likely too large.\n\nYou may configure its rendered size using the View > Advanced menu, or create a new maze with smaller dimensions.");
            return;
        }

        //painter.setRenderHints(QPainter::Antialiasing);
        QPaintEvent event(QRect(0, 0, ((mazeWidth + 1) * gridSpacing), ((mazeHeight + 1) * gridSpacing)));
        paintOnto(&painter, &event);
        painter.end();
        image.save(fileName);
    }
}

void MazeWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    //painter.setRenderHint(QPainter::Antialiasing);
    paintOnto(&painter, event);
    painter.end();
}

bool MazeWidget::getRoundedPaths() const
{
    return roundedPaths;
}

void MazeWidget::setRoundedPaths(bool value)
{
    roundedPaths = value;
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
    myMaze = (MazeRef)maze;
    resetWidgetSize();
    update();
    emit on_mazeCreated();
}

void MazeWidget::generateMazeWorker_error(QString err)
{
    qDebug() << "generateMazeWorker_error: " << err;
}

void MazeWidget::deleteMazeWorker_finished(void *maze)
{
    myMaze = (MazeRef)maze;
}

void MazeWidget::deleteMazeWorker_error(QString err)
{
    qDebug() << "deleteMazeWorker_error: " << err;
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
    hallThickness = DEFAULT_HALL_THICKNESS;
    solutionThickness = DEFAULT_SOLUTION_THICKNESS;
    resetWidgetSize();
    update();
}

int MazeWidget::getHallThickness() const
{
    return hallThickness;
}

void MazeWidget::setHallThickness(int value)
{
    hallThickness = value;
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

