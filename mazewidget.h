/*
 *  mazewidget.h
 *  MazeGenerator
 *
 *  Copyright 2018-2024 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef MAZEWIDGET_H
#define MAZEWIDGET_H

#include <QWidget>
#include <QThread>
#include <QRect>
#include "Maze.h"

#define DEFAULT_GRID_SPACING 24
#define DEFAULT_WALL_THICKNESS 8
#define DEFAULT_SOLUTION_THICKNESS 8

class MazeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MazeWidget(QWidget *parent = 0);
    ~MazeWidget();
    void generateMaze();
    void openMaze();

    int getMazeWidth() const;
    void setMazeWidth(int value);

    int getMazeHeight() const;
    void setMazeHeight(int value);

    int getGridSpacing() const;
    void setGridSpacing(int value);

    int getWallThickness() const;
    void setWallThickness(int value);

    int getSolutionThickness() const;
    void setSolutionThickness(int value);

    void resetDefaultSpacing();

    bool getShowSolution() const;
    void setShowSolution(bool value);

    bool getShowMaze() const;
    void setShowMaze(bool value);

    bool getRoundCaps() const;
    void setRoundCaps(bool value);

    void paintBackground(QPainter *painter, const QRect &rect);
    void paintMazePaths(QPainter *painter, const QRect &rect);
    void paintSolution(QPainter *painter, const QRect &rect);
    void paintDebug(QPainter *painter, const QRect &rect);

    void printMaze();
    void exportImage();
    void saveMazeAs();
    void loadNative();

    qreal getScaling() const;
    void setScaling(const qreal &value);

    bool getAntialiased() const;
    void setAntialiased(bool value);

    void paintMazeWalls(QPainter *painter, const QRect &rect);
    void paintPathBackground(QPainter *painter, const QRect &rect);
    bool getInverse() const;
    void setInverse(bool value);

    uint32_t getSolutionLength() const;

    bool getSavingMaze() const;

    void setHighlight(const uint32_t &value);

    uint32_t getHighlight() const;

    bool getDebug() const;
    void setDebug(bool value);

signals:
    void generateMazeWorker_start();
    void deleteMazeWorker_start();
    void openMazeWorker_start();
    void saveMazeWorker_start();

    void on_deletingOldMaze();
    void on_allocatingMemory();
    void on_generatingMaze();
    void on_solvingMaze();
    void on_mazeCreated();
    void on_openMazePostError();

    void on_openMaze();
    void on_openMazeError(QString err);

    void on_savingMaze();
    void on_saveMazeError(QString err);
    void on_mazeSaved();

public slots:
    void generateMazeWorker_deletingOldMaze();
    void generateMazeWorker_allocatingMemory();
    void generateMazeWorker_generatingMaze();
    void generateMazeWorker_solvingMaze();
    void generateMazeWorker_finished(void *maze);
    void generateMazeWorker_error(QString err);

    void deleteMazeWorker_finished(void *maze);
    void deleteMazeWorker_error(QString err);

    void openMazeWorker_deletingOldMaze();
    void openMazeWorker_allocatingMemory();
    void openMazeWorker_loadingMaze(int width, int height);
    void openMazeWorker_finished(void *maze);
    void openMazeWorker_error(void* maze, QString err);

    void saveMazeWorker_savingMaze();
    void saveMazeWorker_finished();
    void saveMazeWorker_error(QString err);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void resetWidgetSize();

    QThread workerThread;
    bool creatingMaze = false;
    bool savingMaze = false;

    MazeRef myMaze = 0;
    int mazeWidth = 25;
    int mazeHeight = 25;
    uint32_t solutionLength = 0; // trivia returned from the maze solver

    int gridSpacing = DEFAULT_GRID_SPACING;
    int wallThickness = DEFAULT_WALL_THICKNESS;
    int solutionThickness = DEFAULT_SOLUTION_THICKNESS;

    bool showSolution = true;
    bool showMaze = true;
    bool roundCaps = true;
    bool antialiased = false;
    bool inverse = false;
    bool debug = false;
    uint32_t highlight = 0;

    qreal scaling = 1.0;
    QRect scaleRect(const QRect &rect);
};

#endif // MAZEWIDGET_H
