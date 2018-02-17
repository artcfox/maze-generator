/*
 *  mazewidget.h
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef MAZEWIDGET_H
#define MAZEWIDGET_H

#include <QWidget>
#include <QThread>
#include "Maze.h"

#define DEFAULT_GRID_SPACING 24
#define DEFAULT_HALL_THICKNESS 16
#define DEFAULT_SOLUTION_THICKNESS 8

class MazeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MazeWidget(QWidget *parent = 0);
    ~MazeWidget();
    void generateMaze();

    int getMazeWidth() const;
    void setMazeWidth(int value);

    int getMazeHeight() const;
    void setMazeHeight(int value);

    int getGridSpacing() const;
    void setGridSpacing(int value);

    int getHallThickness() const;
    void setHallThickness(int value);

    int getSolutionThickness() const;
    void setSolutionThickness(int value);

    void resetDefaultSpacing();

    bool getShowSolution() const;
    void setShowSolution(bool value);

    bool getShowMaze() const;
    void setShowMaze(bool value);

    bool getRoundedPaths() const;
    void setRoundedPaths(bool value);

    void paintBackground(QPainter *painter, const QRect &rect);
    void paintMaze(QPainter *painter, const QRect &rect);
    void paintSolution(QPainter *painter, const QRect &rect);

    void printMaze();
    void exportImage();

signals:
    void operate();
    void on_deletingOldMaze();
    void on_allocatingMemory();
    void on_generatingMaze();
    void on_solvingMaze();
    void on_mazeCreated();

public slots:
    void generateMazeWorker_deletingOldMaze();
    void generateMazeWorker_allocatingMemory();
    void generateMazeWorker_generatingMaze();
    void generateMazeWorker_solvingMaze();

    void generateMazeWorker_finished(void *maze);
    void generateMazeWorker_error(QString err);
    void deleteMazeWorker_finished(void *maze);
    void deleteMazeWorker_error(QString err);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void resetWidgetSize();

    QThread workerThread;
    bool creatingMaze = false;

    MazeRef myMaze = 0;
    int mazeWidth = 25;
    int mazeHeight = 25;

    int gridSpacing = DEFAULT_GRID_SPACING;
    int hallThickness = DEFAULT_HALL_THICKNESS;
    int solutionThickness = DEFAULT_SOLUTION_THICKNESS;

    bool showSolution = true;
    bool showMaze = true;
    bool roundedPaths = true;
};

#endif // MAZEWIDGET_H
