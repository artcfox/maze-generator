/*
 *  savemazeworker.h
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef SAVEMAZEWORKER_H
#define SAVEMAZEWORKER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QtEndian>

#include "Maze.h"

class SaveMazeWorker : public QObject
{
    Q_OBJECT
public:
    explicit SaveMazeWorker(MazeRef myMaze, QString fileName) : myMaze(myMaze), fileName(fileName)
    {

    }

signals:
    void saveMazeWorker_savingMaze();
    void saveMazeWorker_finished();
    void saveMazeWorker_error(QString err);

public slots:
    void process() {
        emit saveMazeWorker_savingMaze();

        QFile file(fileName);
        if (!file.open(QIODevice::ReadWrite)) {
            emit saveMazeWorker_error(QString("The file '%1' could not be opened.").arg(fileName));
            return;
        }
        BitArray baMaze, baSolution;
        baMaze.numBits = baSolution.numBits = myMaze->totalWalls;
        baMaze.data_length = baSolution.data_length = myMaze->totalWalls / 8 + ((myMaze->totalWalls % 8) ? 1 : 0);

        qint64 fileSize = sizeof(uint32_t) * (myMaze->dims_length + 2) + baMaze.data_length + baSolution.data_length;
        if (!file.resize(fileSize)) {
            emit saveMazeWorker_error(QString("The file '%1' could not be resized.").arg(fileName));
            return;
        }
        uchar *memory = file.map(0, fileSize);
        if (!memory) {
            emit saveMazeWorker_error(QString("The file '%1' could not be mapped to memory.").arg(fileName));
            return;
        }
        uint32_t dims_length = myMaze->dims_length;
        qToLittleEndian(dims_length, memory);
        for (uint32_t i = 0; i < myMaze->dims_length; ++i) {
            uint32_t dim = myMaze->dims[i];
            qToLittleEndian(dim, memory + sizeof(uint32_t) * (i + 1));
        }

        baMaze.data = memory + sizeof(uint32_t) * (dims_length + 1);
        BitArray_reset(&baMaze);

        uint32_t solutionLength = myMaze->solutionLength;
        qToLittleEndian(solutionLength, baMaze.data + baMaze.data_length);

        baSolution.data = baMaze.data + baMaze.data_length + sizeof(uint32_t);
        BitArray_reset(&baSolution);

        // Maze
        uint32_t lotteryIndex = 0;
        for (uint32_t position = 0; position < myMaze->totalPositions; ++position) {
            uint32_t placeValue = 1;
            for (uint32_t i = 0; i < myMaze->dims_length; ++i) {
                uint32_t valueForThisDim = (position / placeValue) % myMaze->dims[i];
                if (valueForThisDim < myMaze->dims[i] - 1) {
                    if (BitArray_readBit(myMaze->halls[i], position)) {
                        BitArray_setBit(&baMaze, lotteryIndex);
    //                        qDebug() << "Wall: (" << position << "," << (position + placeValue) << ")";
    //                        qDebug() << "Corresponds to Wall #" << lotteryIndex;
                    }
                    lotteryIndex++;
                }
                placeValue *= myMaze->dims[i];
            }
        }

        // Solution
        lotteryIndex = 0;
        for (uint32_t position = 0; position < myMaze->totalPositions; ++position) {
            uint32_t placeValue = 1;
            for (uint32_t i = 0; i < myMaze->dims_length; ++i) {
                uint32_t valueForThisDim = (position / placeValue) % myMaze->dims[i];
                if (valueForThisDim < myMaze->dims[i] - 1) {
                    if (BitArray_readBit(myMaze->solution[i], position)) {
                        BitArray_setBit(&baSolution, lotteryIndex);
    //                        qDebug() << "Solution: (" << position << "," << (position + placeValue) << ")";
    //                        qDebug() << "Corresponds to Solution #" << lotteryIndex;
                    }
                    lotteryIndex++;
                }
                placeValue *= myMaze->dims[i];
            }
        }
        file.unmap(memory);

        emit saveMazeWorker_finished();
    }

private:
    MazeRef myMaze = 0;
    QString fileName;
};

#endif // SAVEMAZEWORKER_H
