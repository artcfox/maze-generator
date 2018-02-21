/*
 *  loadmazeworker.h
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef OPENMAZEWORKER_H
#define OPENMAZEWORKER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QtEndian>

#include "Maze.h"

class OpenMazeWorker : public QObject
{
    Q_OBJECT
public:
    explicit OpenMazeWorker(MazeRef myMaze, QString fileName) : myMaze(myMaze), fileName(fileName)
    {

    }

signals:
    void openMazeWorker_deletingOldMaze();
    void openMazeWorker_allocatingMemory();
    void openMazeWorker_loadingMaze(int width, int height);
    void openMazeWorker_finished(void *myMaze);
    void openMazeWorker_error(QString err);

public slots:
    void process() {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            emit openMazeWorker_error(QString("The file '%1' could not be opened.").arg(fileName));
            return;
        }
        uchar *memory = file.map(0, file.size());
        if (!memory) {
            emit openMazeWorker_error(QString("The file '%1' could not be mapped to memory.").arg(fileName));
            return;
        }
        if (file.size() < (qint64)sizeof(uint32_t)) {
            emit openMazeWorker_error(QString("The file '%1' is not valid.").arg(fileName));
            return;
        }

        uint32_t dims_length = qFromLittleEndian<uint32_t>(memory);

        if (dims_length != 2) {
            emit openMazeWorker_error(QString("Cannot load a maze with %2 dimensions.").arg(dims_length));
            return;
        }

        if (file.size() < (qint64)sizeof(uint32_t) * (dims_length + 1)) {
            emit openMazeWorker_error(QString("The file '%1' is not valid.").arg(fileName));
            return;
        }

        uint32_t *dims = new uint32_t[dims_length];
        for (uint32_t i = 0; i < dims_length; ++i)
            dims[i] = qFromLittleEndian<uint32_t>(memory + sizeof(uint32_t) * (i + 1));

        uint32_t totalPositions = 1;
        for (uint32_t i = 0; i < dims_length; ++i)
            totalPositions *= dims[i];

        uint32_t totalWalls = 0;
        for (uint32_t i = 0; i < dims_length; ++i) {
            uint32_t subTotal = 1;
            for (uint32_t j = 0; j < dims_length; ++j)
                if (j != i)
                    subTotal *= dims[j];
            totalWalls += subTotal * (dims[i] - 1);
        }

        BitArray baMaze, baSolution;
        baMaze.numBits = baSolution.numBits = totalWalls;
        baMaze.data_length = baSolution.data_length = totalWalls / 8 + ((totalWalls % 8) ? 1 : 0);

        if (file.size() < (qint64)sizeof(uint32_t) * (dims_length + 2) + baMaze.data_length + baSolution.data_length) {
            emit openMazeWorker_error(QString("The file '%1' is not valid.").arg(fileName));
            return;
        }

        baMaze.data = memory + sizeof(uint32_t) * (dims_length + 1); // The + 1 is to skip the dims_length header

        uint32_t solutionLength = qFromLittleEndian<uint32_t>(baMaze.data + baMaze.data_length);
        baSolution.data = baMaze.data + baMaze.data_length + sizeof(uint32_t);

        if ((myMaze == 0) || (myMaze->dims[0] != dims[0]) || (myMaze->dims[1] != dims[1])) {
            emit openMazeWorker_deletingOldMaze();
            Maze_delete(myMaze);
            emit openMazeWorker_allocatingMemory();
            myMaze = Maze_create(dims, dims_length, (MazeCreateFlags)(mcfOutputMaze | mcfOutputSolution /*| mcfMultipleSolves*/));
        }

        emit openMazeWorker_loadingMaze((int)dims[0], (int)dims[1]);

        for (uint32_t i = 0; i < myMaze->dims_length; ++i) {
            BitArray_reset(myMaze->halls[i]);
            BitArray_reset(myMaze->solution[i]);
        }

        // Maze
        uint32_t lotteryIndex = 0;
        for (uint32_t position = 0; position < totalPositions; ++position) {
            uint32_t placeValue = 1;
            for (uint32_t i = 0; i < dims_length; ++i) {
                uint32_t valueForThisDim = (position / placeValue) % dims[i];
                if (valueForThisDim < dims[i] - 1) {
                    if (BitArray_readBit(&baMaze, lotteryIndex)) {
                        BitArray_setBit(myMaze->halls[i], position);
    //                    qDebug() << "Wall: (" << position << "," << (position + placeValue) << ")";
    //                    qDebug() << "Corresponds to Wall #" << lotteryIndex;
                    }
                    lotteryIndex++;
                }
                placeValue *= dims[i];
            }
        }

        // Solution
        lotteryIndex = 0;
        for (uint32_t position = 0; position < totalPositions; ++position) {
            uint32_t placeValue = 1;
            for (uint32_t i = 0; i < dims_length; ++i) {
                uint32_t valueForThisDim = (position / placeValue) % dims[i];
                if (valueForThisDim < dims[i] - 1) {
                    if (BitArray_readBit(&baSolution, lotteryIndex)) {
                        BitArray_setBit(myMaze->solution[i], position);
    //                    qDebug() << "Wall: (" << position << "," << (position + placeValue) << ")";
    //                    qDebug() << "Corresponds to Wall #" << lotteryIndex;
                    }
                    lotteryIndex++;
                }
                placeValue *= dims[i];
            }
        }

        delete [] dims;
        file.unmap(memory);

        myMaze->solutionLength = solutionLength;

//        emit on_mazeCreated();
//        creatingMaze = false;
//        update();








//        uint32_t dims[2] = { mazeWidth, mazeHeight };
//        if ((myMaze == 0) || (myMaze->dims[0] != dims[0]) || (myMaze->dims[1] != dims[1])) {
//            emit generateMazeWorker_deletingOldMaze();
//            Maze_delete(myMaze);
//            emit generateMazeWorker_allocatingMemory();
//            myMaze = Maze_create(dims, 2, (MazeCreateFlags)(mcfOutputMaze | mcfOutputSolution /*| mcfMultipleSolves*/));
//        }

//        emit generateMazeWorker_generatingMaze();
//        Maze_generate(myMaze);
//        emit generateMazeWorker_solvingMaze();
//        Maze_solve(myMaze, 0, myMaze->totalPositions - 1);
        emit openMazeWorker_finished((void*)myMaze);
    }

private:
    MazeRef myMaze = 0;
    QString fileName;
};

#endif // OPENMAZEWORKER_H
