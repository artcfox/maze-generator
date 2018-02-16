/*
 *  Maze.c
 *  MazeInC
 *
 *  Copyright 2009-2018 Matthew T. Pandina. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "Maze.h"

MazeRef Maze_create(int *dims, unsigned int length, MazeCreateFlags flags) {
    MazeRef m;
    m = (MazeRef)malloc(sizeof(Maze));
    m->totalPositions = 1;
    m->totalWalls = 0;
    m->lottery = NULL;
    m->neighborCount = m->neighborCountCopy = NULL;
    m->dims = (int*)malloc(sizeof(unsigned int) * length);
    m->dims_length = length;
    memcpy(m->dims, dims, sizeof(unsigned int) * length);
    m->halls = m->solution = NULL;
    m->createFlags = flags;
    m->sets = NULL;
    m->needsNeighborCountRefreshed = false;
    m->solutionLength = m->start = m->end = 0;

    for (unsigned int i = 0; i < length; ++i)
        m->totalPositions *= dims[i];

    for (unsigned int i = 0; i < length; ++i) {
        int subTotal = 1;
        for (unsigned int j = 0; j < length; ++j)
            if (j != i)
                subTotal *= dims[j];
        m->totalWalls += subTotal * (dims[i] - 1);
    }

    int maxSize;
    if (m->createFlags & mcfOutputSolution)
        maxSize = (m->totalWalls > (m->totalPositions - 1) * 3) ? m->totalWalls : (m->totalPositions - 1) * 3;
    else
        maxSize = m->totalWalls;

    m->lottery = (Wall*)malloc(sizeof(Wall) * maxSize);

    if (m->createFlags & mcfOutputSolution)
        m->neighborCount = (unsigned char*)malloc(sizeof(unsigned char) * m->totalPositions);

    if (m->createFlags & mcfMultipleSolves)
        m->neighborCountCopy = (unsigned char*)malloc(sizeof(unsigned char) * m->totalPositions);

    m->sets = DisjSets_create(m->totalPositions);

    if (m->createFlags & mcfOutputMaze) {
        m->halls = (BitArrayRef*)malloc(sizeof(BitArrayRef) * m->dims_length);
        for (unsigned int i = 0; i < m->dims_length; ++i)
            m->halls[i] = BitArray_create(m->totalPositions, false);
    }
    if (m->createFlags & mcfOutputSolution) {
        m->solution = (BitArrayRef*)malloc(sizeof(BitArrayRef) * m->dims_length);
        for (unsigned int i = 0; i < m->dims_length; ++i)
            m->solution[i] = BitArray_create(m->totalPositions, false);
    }

    srandom(time(0));

    return m;
}

void Maze_delete(MazeRef m) {
    if (!m)
        return;
    free(m->dims);
    if (m->sets)
        DisjSets_delete(m->sets);
    if (m->neighborCount)
        free(m->neighborCount);
    if (m->neighborCountCopy)
        free(m->neighborCountCopy);
    if (m->lottery)
        free(m->lottery);
    if (m->halls) {
        for (unsigned int i = 0; i < m->dims_length; ++i)
            BitArray_delete(m->halls[i]);
        free(m->halls);
    }
    if (m->solution) {
        for (unsigned int i = 0; i < m->dims_length; ++i)
            BitArray_delete(m->solution[i]);
        free(m->solution);
    }
    free(m);
}

void Maze_generate(MazeRef m) {
    if (!m || !m->totalPositions)
        return;

    int lotteryIndex = 0;
    for (int position = 0; position < m->totalPositions; ++position) {
        int placeValue = 1;
        for (unsigned int i = 0; i < m->dims_length; ++i) {
            int valueForThisDim = (position / placeValue) % m->dims[i];
            if (valueForThisDim < m->dims[i] - 1) {
                m->lottery[lotteryIndex].cell1 = position;
                m->lottery[lotteryIndex].cell2 = position + placeValue;
                lotteryIndex++;
            }
            placeValue *= m->dims[i];
        }
    }

    DisjSets_reset(m->sets, m->totalPositions);

    int lotteryExtent = m->totalWalls;
    int knockedOutWalls = 0;

    if (m->createFlags & mcfOutputSolution) {
        memset(m->neighborCount, 0, sizeof(unsigned char) * m->totalPositions);
        m->needsNeighborCountRefreshed = false;

        while (knockedOutWalls < m->totalPositions - 1) {
            int r = (int)( (float)(lotteryExtent - knockedOutWalls) * random() / (RAND_MAX + 1.0) ) + knockedOutWalls;
            int root1 = DisjSets_find(m->sets, m->lottery[r].cell1);
            int root2 = DisjSets_find(m->sets, m->lottery[r].cell2);
            if (root1 != root2) {
                DisjSets_union(m->sets, root1, root2);
                m->neighborCount[m->lottery[r].cell1]++;
                m->neighborCount[m->lottery[r].cell2]++;
                Wall tmp = m->lottery[knockedOutWalls];
                m->lottery[knockedOutWalls] = m->lottery[r];
                m->lottery[r] = tmp;
                knockedOutWalls++;
            } else {
                m->lottery[r] = m->lottery[lotteryExtent - 1];
                lotteryExtent--;
            }
        }
    } else {
        while (knockedOutWalls < m->totalPositions - 1) {
            int r = (int)( (float)(lotteryExtent - knockedOutWalls) * random() / (RAND_MAX + 1.0) ) + knockedOutWalls;
            int root1 = DisjSets_find(m->sets, m->lottery[r].cell1);
            int root2 = DisjSets_find(m->sets, m->lottery[r].cell2);
            if (root1 != root2) {
                DisjSets_union(m->sets, root1, root2);
                Wall tmp = m->lottery[knockedOutWalls];
                m->lottery[knockedOutWalls] = m->lottery[r];
                m->lottery[r] = tmp;
                knockedOutWalls++;
            } else {
                m->lottery[r] = m->lottery[lotteryExtent - 1];
                lotteryExtent--;
            }
        }
    }
}

void Maze_solve(MazeRef m, int start, int end) {
    if (!(m->createFlags & mcfOutputSolution)) {
        fprintf(stderr, "Error: Maze_solve cannot be called without setting mcfOutputSolution in Maze_create\n");
        return;
    }

    if (!m || !m->lottery || !m->neighborCount)
        return;

    m->start = start;
    m->end = end;

    int knockedOutWalls = m->totalPositions - 1;

    Wall *input = &m->lottery[knockedOutWalls];
    Wall *output = &m->lottery[knockedOutWalls * 2];
    Wall *inputPtr = input + knockedOutWalls;
    Wall *outputPtr;

    memcpy(input, m->lottery, sizeof(Wall) * knockedOutWalls);

    if (m->createFlags & mcfMultipleSolves) {
        if (m->needsNeighborCountRefreshed)
            memcpy(m->neighborCount, m->neighborCountCopy, sizeof(unsigned char) * m->totalPositions);
        else
            memcpy(m->neighborCountCopy, m->neighborCount, sizeof(unsigned char) * m->totalPositions);
    } else if (m->needsNeighborCountRefreshed) {
        fprintf(stderr, "Error: Maze_solve cannot be called more than once without setting mcfMultipleSolve in Maze_create\n");
        return;
    }

    while (true) {
        outputPtr = output;
        unsigned int inputLength = inputPtr - input;
        for (unsigned int i = 0; i < inputLength; ++i) {
            if ((m->neighborCount[input[i].cell1] == 1 && input[i].cell1 != start && input[i].cell1 != end) ||
                (m->neighborCount[input[i].cell2] == 1 && input[i].cell2 != start && input[i].cell2 != end)) {
                m->neighborCount[input[i].cell1]--;
                m->neighborCount[input[i].cell2]--;
            } else {
                output[outputPtr - output] = input[i];
                outputPtr++;
            }
        }

        if (inputPtr - input == outputPtr - output)
            break;

        Wall *tmp1 = output;
        Wall *tmp2 = outputPtr;
        output = input;
        outputPtr = inputPtr;
        input = tmp1;
        inputPtr = tmp2;
    }

    m->solutionLength = outputPtr - output;

    m->needsNeighborCountRefreshed = true;

    if (m->createFlags & mcfOutputMaze) {
        for (unsigned int i = 0; i < m->dims_length; ++i)
            BitArray_reset(m->halls[i]);

        for (int i = 0; i < knockedOutWalls; ++i) {
            int placeValue = 1;
            for (unsigned int d = 0; d < m->dims_length; ++d) {
                if (m->dims[d] == 1)
                    continue;
                if (m->lottery[i].cell2 == m->lottery[i].cell1 + placeValue) {
                    BitArray_setBit(m->halls[d], m->lottery[i].cell1);
                    break;
                }
                placeValue *= m->dims[d];
            }
        }
    }

    if (m->createFlags & mcfOutputSolution) {
        for (unsigned int i = 0; i < m->dims_length; ++i)
            BitArray_reset(m->solution[i]);

        for (int i = 0; i < m->solutionLength; i++) {
            int placeValue = 1;
            for (unsigned int d = 0; d < m->dims_length; ++d) {
                if (m->dims[d] == 1)
                    continue;
                if (output[i].cell2 == output[i].cell1 + placeValue) {
                    BitArray_setBit(m->solution[d], output[i].cell1);
                    break;
                }
                placeValue *= m->dims[d];
            }
        }
    }
}
