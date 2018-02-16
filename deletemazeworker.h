/*
 *  deletemazeworker.h
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef DELETEMAZEWORKER_H
#define DELETEMAZEWORKER_H

#include <QObject>
#include "Maze.h"

class DeleteMazeWorker : public QObject
{
    Q_OBJECT
public:
    explicit DeleteMazeWorker(MazeRef myMaze) : myMaze(myMaze)
    {

    }

signals:
    void deleteMazeWorker_finished(void *maze);
    void deleteMazeWorker_error(QString err);

public slots:
    void process() {
        if (myMaze != 0) {
            Maze_delete(myMaze);
            myMaze = 0;
        }
        emit deleteMazeWorker_finished((void*)myMaze);
    }

private:
    MazeRef myMaze = 0;
};

#endif // DELETEMAZEWORKER_H
