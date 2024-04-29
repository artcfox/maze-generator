/*
 *  generatemazeworker.h
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef GENERATEMAZEWORKER_H
#define GENERATEMAZEWORKER_H

#include <QObject>
#include <QThread>
#include "Maze.h"

class GenerateMazeWorker : public QObject
{
    Q_OBJECT
public:
    explicit GenerateMazeWorker(MazeRef myMaze, int mazeWidth, int mazeHeight) : myMaze(myMaze), mazeWidth(mazeWidth), mazeHeight(mazeHeight)
    {

    }

signals:
    void generateMazeWorker_deletingOldMaze();
    void generateMazeWorker_allocatingMemory();
    void generateMazeWorker_generatingMaze();
    void generateMazeWorker_solvingMaze();
    void generateMazeWorker_finished(void *myMaze);
    void generateMazeWorker_error(QString err);

public slots:
    void process() {
        uint32_t dims[2] = { mazeWidth, mazeHeight };
        if ((myMaze == 0) || (myMaze->dims[0] != dims[0]) || (myMaze->dims[1] != dims[1])) {
            emit generateMazeWorker_deletingOldMaze();
            Maze_delete(myMaze);
            emit generateMazeWorker_allocatingMemory();
            myMaze = Maze_create(dims, 2, (MazeCreateFlags)(mcfOutputMaze | mcfOutputSolution /*| mcfMultipleSolves*/));
#ifdef Q_OS_WASM
            int idealThreads = 2;
#else
            int idealThreads = QThread::idealThreadCount();
#endif
            if (idealThreads > 0)
                Maze_setCores(myMaze, idealThreads);
        }

        emit generateMazeWorker_generatingMaze();
        Maze_generate(myMaze);
        emit generateMazeWorker_solvingMaze();
        Maze_solve(myMaze, 0, myMaze->totalPositions - 1);
        emit generateMazeWorker_finished((void*)myMaze);
    }

private:
    MazeRef myMaze = 0;
    uint32_t mazeWidth = 0;
    uint32_t mazeHeight = 0;
};

#endif // GENERATEMAZEWORKER_H
