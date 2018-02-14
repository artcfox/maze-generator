/*
 *  mainwindow.cpp
 *  MazeGenerator
 *
 *  Created by Matthew T Pandina on 2/11/18.
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_about.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QDialog>
#include <QFileDialog>
#include <QInputDialog>

#include <QLinearGradient>
#include <QGraphicsRectItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->action_Show_Solution->setChecked(showSolution);
    ui->actionShow_Maze->setChecked(showMaze);
    ui->actionRounded_Paths->setChecked(roundedPaths);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setCentralWidget(ui->graphicsView);

    generateMaze();
    displayMaze();
}

MainWindow::~MainWindow()
{
    if (myMaze != 0)
        Maze_delete(myMaze);
    delete scene;
    delete ui;
}

void MainWindow::generateMaze()
{
    int dims[2] = { getMazeWidth(), getMazeHeight() };
    if ((myMaze == 0) || (myMaze->dims[0] != dims[0]) || (myMaze->dims[1] != dims[1])) {
        Maze_delete(myMaze);
        myMaze = Maze_create(dims, 2, (MazeCreateFlags)(mcfOutputMaze | mcfOutputSolution /*| mcfMultipleSolves*/));
    }

    Maze_generate(myMaze);
    Maze_solve(myMaze, 0, myMaze->totalPositions - 1);
}

void MainWindow::displayMaze()
{
    scene->clear();

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

    int cell1[2], cell2[2];
    cell1[0] = 0;
    cell1[1] = 0;
    cell2[0] = myMaze->dims[0];
    cell2[1] = myMaze->dims[1];

    // Draw a white rectangle the size of the bounding box, to keep the scene the same size in case the maze is later hidden
    QGraphicsRectItem *whiteRect = scene->addRect(0, 0, ((myMaze->dims[0] + 1) * gridSpacing), ((myMaze->dims[1] + 1) * gridSpacing), hallPen, whiteBrush);

    if (showMaze) {
        scene->addRect(0, 0, ((myMaze->dims[0] + 1) * gridSpacing), ((myMaze->dims[1] + 1) * gridSpacing), hallPen, blackBrush);

        if (linearGradient) {
            // Add a gradient on the background (walls)
            QLinearGradient gradient(whiteRect->boundingRect().topLeft(), whiteRect->boundingRect().bottomRight());
            gradient.setColorAt(0, Qt::green);
            gradient.setColorAt(1, Qt::blue);
            QBrush brush(gradient);
            brush.setStyle(Qt::BrushStyle::LinearGradientPattern);
            scene->addRect(0, 0, (myMaze->dims[0] + 1) * gridSpacing, (myMaze->dims[1] + 1) * gridSpacing, hallPen, brush);
        }

        /* Draw the entrance */ {
            QPoint p1(((1) * gridSpacing), ((0) * gridSpacing));
            QPoint p2(((1) * gridSpacing), ((1) * gridSpacing));
            scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), hallPen);
        }
        /* Draw the exit */ {
            QPoint p1(((myMaze->dims[0]) * gridSpacing), ((myMaze->dims[1]) * gridSpacing));
            QPoint p2(((myMaze->dims[0]) * gridSpacing), ((myMaze->dims[1] + 1) * gridSpacing));
            scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), hallPen);
        }

        /* Draw all horizontal paths in the maze */ {
            BitArrayRef connected = myMaze->halls[0];
            // loop over all of the horizontal paths
            for (int y = cell1[1]; y < cell2[1]; ++y) {
                for (int x = cell1[0]; x < cell2[0]; ++x) {
                    int position = y * myMaze->dims[0] + x; // convert (x, y) coordinates into a scalar position
                    if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                        QPoint p1(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                        int offset = 0;
                        while (x + 1 < cell2[0]) { // see if we can extend this line more
                            position++; // move to the right one square
                            if (BitArray_readBit(connected, position)) // are the position and the one next to it connected?
                                offset++; // extend the endpoint of the line
                            else
                                break; // there is a wall, the line needs to end
                        }
                        QPoint p2(((x + (offset + 1) + 1) * gridSpacing), ((y + 1) * gridSpacing));
                        scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), hallPen);
                        x += offset; // if we were able to extend the line, adjust the index variable
                    }
                }
            }
        }
        /* Draw all vertical paths in the maze */ {
            BitArrayRef connected = myMaze->halls[1];
            // loop over all of the vertical paths
            for (int x = cell1[0]; x < cell2[0]; ++x) {
                for (int y = cell1[1]; y < cell2[1]; ++y) {
                    int position = y * myMaze->dims[0] + x;
                    if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                        QPoint p1(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                        int offset = 0;
                        while (y + 1 < cell2[1]) { // see if we can extend this line more
                            position += myMaze->dims[0]; // move down one square
                            if (BitArray_readBit(connected, position)) // are the position and the one next to it connected?
                                offset++; // extend the endpoint of the line
                            else
                                break; // there is a wall, the line needs to end
                        }
                        QPoint p2(((x + 1) * gridSpacing), ((y + (offset + 1) + 1) * gridSpacing));
                        scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), hallPen);
                        y += offset; // if we were able to extend the line, adjust the index variable
                    }
                }
            }
        }
    }

    if (showSolution) {
        /* Connect solution to entrance */ {
            QPoint p1(((1) * gridSpacing), ((0) * gridSpacing));
            QPoint p2(((1) * gridSpacing), ((1) * gridSpacing));
            scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), solutionPen);
        }
        /* Connect solution to exit */ {
            QPoint p1(((myMaze->dims[0]) * gridSpacing), ((myMaze->dims[1]) * gridSpacing));
            QPoint p2(((myMaze->dims[0]) * gridSpacing), ((myMaze->dims[1] + 1) * gridSpacing));
            scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), solutionPen);
        }

        /* Draw all horizontal paths in the solution */ {
            BitArrayRef connected = myMaze->solution[0];
            // loop over all of the horizontal paths
            for (int y = cell1[1]; y < cell2[1]; ++y) {
                for (int x = cell1[0]; x < cell2[0]; ++x) {
                    int position = y * myMaze->dims[0] + x; // convert (x, y) coordinates into a scalar position
                    if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                        QPoint p1(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                        int offset = 0;
                        while (x + 1 < cell2[0]) { // see if we can extend this line more
                            position++; // move to the right one square
                            if (BitArray_readBit(connected, position)) // are the position and the one next to it connected?
                                offset++; // extend the endpoint of the line
                            else
                                break; // there is a wall, the line needs to end
                        }
                        QPoint p2(((x + (offset + 1) + 1) * gridSpacing), ((y + 1) * gridSpacing));
                        scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), solutionPen);
                        x += offset; // if we were able to extend the line, adjust the index variable
                    }
                }
            }
        }
        /* Draw all vertical paths in the solution */ {
            BitArrayRef connected = myMaze->solution[1];
            // loop over all of the vertical paths
            for (int x = cell1[0]; x < cell2[0]; ++x) {
                for (int y = cell1[1]; y < cell2[1]; ++y) {
                    int position = y * myMaze->dims[0] + x;
                    if (BitArray_readBit(connected, position)) { // are the position and the one next to it connected?
                        QPoint p1(((x + 1) * gridSpacing), ((y + 1) * gridSpacing));
                        int offset = 0;
                        while (y + 1 < cell2[1]) { // see if we can extend this line more
                            position += myMaze->dims[0]; // move down one square
                            if (BitArray_readBit(connected, position)) // are the position and the one next to it connected?
                                offset++; // extend the endpoint of the line
                            else
                                break; // there is a wall, the line needs to end
                        }
                        QPoint p2(((x + 1) * gridSpacing), ((y + (offset + 1) + 1) * gridSpacing));
                        scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), solutionPen);
                        y += offset; // if we were able to extend the line, adjust the index variable
                    }
                }
            }
        }
    }
    scene->setSceneRect(scene->itemsBoundingRect());
    ui->graphicsView->setSceneRect(scene->itemsBoundingRect());
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_action_New_triggered()
{
    generateMaze();
    displayMaze();
}

void MainWindow::on_action_Print_triggered()
{
    QPrinter printer;
    if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        scene->render(&painter);
    }
}

void MainWindow::on_action_Show_Solution_triggered()
{
    showSolution = !showSolution;
    ui->action_Show_Solution->setChecked(showSolution);
    displayMaze();
}

void MainWindow::on_actionSave_As_Image_triggered()
{
    QString fileName= QFileDialog::getSaveFileName(this, tr("Save Image As..."), QCoreApplication::applicationDirPath(), "PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp)" );
    if (!fileName.isNull())
    {
        int padWidth = 4;
        QImage image;
        QImage paddedImage(scene->width() + 2 * padWidth, scene->height() + 2 * padWidth, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&paddedImage);
        scene->render(&painter);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        image = paddedImage.copy(padWidth, padWidth, scene->width(), scene->height());
        image.save(fileName);
    }
}

int MainWindow::getMazeWidth() const
{
    return mazeWidth;
}

void MainWindow::setMazeWidth(int value)
{
    mazeWidth = value;
}

int MainWindow::getMazeHeight() const
{
    return mazeHeight;
}

void MainWindow::setMazeHeight(int value)
{
    mazeHeight = value;
}

void MainWindow::on_action_Default_Width_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Edit Default Width"), tr("New Maze Width:"), getMazeWidth(), 2, 32767, 1, &ok);
    if (ok)
        setMazeWidth(value);
}

void MainWindow::on_action_Default_Height_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Edit Default Height"), tr("New Maze Height:"), getMazeHeight(), 2, 32767, 1, &ok);
    if (ok)
        setMazeHeight(value);
}

int MainWindow::getSolutionThickness() const
{
    return solutionThickness;
}

void MainWindow::setSolutionThickness(int value)
{
    solutionThickness = value;
}

int MainWindow::getHallThickness() const
{
    return hallThickness;
}

void MainWindow::setHallThickness(int value)
{
    hallThickness = value;
}

int MainWindow::getGridSpacing() const
{
    return gridSpacing;
}

void MainWindow::setGridSpacing(int value)
{
    gridSpacing = value;
}

void MainWindow::on_action_Grid_Spacing_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Set Distance Between Paths"), tr("Distance Between Paths:                   "), getGridSpacing(), getHallThickness() + 1, 256, 1, &ok);
    if (ok)
        setGridSpacing(value);
    displayMaze();
}

void MainWindow::on_action_Path_Width_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Set Path Width"), tr("Path Width:"), getHallThickness(), getSolutionThickness(), getGridSpacing() - 1, 1, &ok);
    if (ok)
        setHallThickness(value);
    displayMaze();
}

void MainWindow::on_action_Solution_Width_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Set Solution Width"), tr("Solution Width:"), getSolutionThickness(), 1, getHallThickness(), 1, &ok);
    if (ok)
        setSolutionThickness(value);
    displayMaze();
}

void MainWindow::on_actionRounded_Paths_triggered()
{
    roundedPaths = !roundedPaths;
    ui->actionRounded_Paths->setChecked(roundedPaths);

    foreach(QGraphicsItem *item, scene->items()) {
        QGraphicsLineItem *line;
        if ((line = dynamic_cast<QGraphicsLineItem*>(item))) {
            QPen pen = line->pen();

            if (pen.capStyle() == Qt::RoundCap)
                pen.setCapStyle(Qt::SquareCap);
            else
                pen.setCapStyle(Qt::RoundCap);

            line->setPen(pen);
        }
    }
}

void MainWindow::on_actionZoom_In_triggered()
{
    ui->graphicsView->scale(2.0, 2.0);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    ui->graphicsView->scale(0.5, 0.5);
}

void MainWindow::on_actionZoom_Normal_triggered()
{
    ui->graphicsView->resetTransform();
}

void MainWindow::on_actionShow_Maze_triggered()
{
    showMaze = !showMaze;
    ui->actionShow_Maze->setChecked(showMaze);
    displayMaze();
}

void MainWindow::on_action_About_triggered()
{
    QDialog *about = new QDialog(this, Qt::Dialog);
    Ui_Dialog aboutUi;
    aboutUi.setupUi(about);
    about->exec();
    delete about;
}

void MainWindow::on_action_Reset_Widths_triggered()
{
    setGridSpacing(DEFAULT_GRID_SPACING);
    setHallThickness(DEFAULT_HALL_THICKNESS);
    setSolutionThickness(DEFAULT_SOLUTION_THICKNESS);
    displayMaze();
}

void MainWindow::on_action_Linear_Gradient_triggered()
{
    linearGradient = !linearGradient;
    ui->action_Linear_Gradient->setChecked(linearGradient);
    displayMaze();
}
