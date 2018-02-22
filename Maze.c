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

MazeRef Maze_create(uint32_t *dims, uint32_t length, MazeCreateFlags flags) {
    MazeRef m;
    m = (MazeRef)malloc(sizeof(Maze));
    m->totalPositions = 1;
    m->totalWalls = 0;
    m->lottery = NULL;
    m->neighborCount = m->neighborCountCopy = NULL;
    m->dims = (uint32_t*)malloc(sizeof(uint32_t) * length);
    m->dims_length = length;
    memcpy(m->dims, dims, sizeof(uint32_t) * length);
    m->halls = m->solution = NULL;
    m->createFlags = flags;
    m->sets = NULL;
    m->needsNeighborCountRefreshed = false;
    m->solutionLength = m->start = m->end = 0;

    for (uint32_t i = 0; i < length; ++i)
        m->totalPositions *= dims[i];

    for (uint32_t i = 0; i < length; ++i) {
        uint32_t subTotal = 1;
        for (uint32_t j = 0; j < length; ++j)
            if (j != i)
                subTotal *= dims[j];
        m->totalWalls += subTotal * (dims[i] - 1);
    }

    m->lottery = (Wall*)malloc(sizeof(Wall) * m->totalWalls);

    if (m->createFlags & mcfOutputSolution)
        m->neighborCount = (unsigned char*)malloc(sizeof(uint8_t) * m->totalPositions);

    if (m->createFlags & mcfMultipleSolves)
        m->neighborCountCopy = (unsigned char*)malloc(sizeof(uint8_t) * m->totalPositions);

    m->sets = DisjSets_create(m->totalPositions);

    if (m->createFlags & mcfOutputMaze) {
        m->halls = (BitArrayRef*)malloc(sizeof(BitArrayRef) * m->dims_length);
        for (unsigned int i = 0; i < m->dims_length; ++i)
            m->halls[i] = BitArray_create(m->totalPositions, false);
    }
    if (m->createFlags & mcfOutputSolution) {
        m->solution = (BitArrayRef*)malloc(sizeof(BitArrayRef) * m->dims_length);
        for (uint32_t i = 0; i < m->dims_length; ++i)
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
        for (uint32_t i = 0; i < m->dims_length; ++i)
            BitArray_delete(m->halls[i]);
        free(m->halls);
    }
    if (m->solution) {
        for (uint32_t i = 0; i < m->dims_length; ++i)
            BitArray_delete(m->solution[i]);
        free(m->solution);
    }
    free(m);
}

void Maze_generate(MazeRef m) {
    if (!m || !m->totalPositions)
        return;

    uint32_t lotteryIndex = 0;
    for (uint32_t position = 0; position < m->totalPositions; ++position) {
        int placeValue = 1;
        for (uint32_t i = 0; i < m->dims_length; ++i) {
            uint32_t valueForThisDim = (position / placeValue) % m->dims[i];
            if (valueForThisDim < m->dims[i] - 1) {
                m->lottery[lotteryIndex].cell1 = position;
                m->lottery[lotteryIndex].cell2 = position + placeValue;
                lotteryIndex++;
            }
            placeValue *= m->dims[i];
        }
    }

    DisjSets_reset(m->sets, m->totalPositions);

    uint32_t lotteryExtent = m->totalWalls;
    uint32_t knockedOutWalls = 0;

    if (m->createFlags & mcfOutputSolution) {
        memset(m->neighborCount, 0, sizeof(uint8_t) * m->totalPositions);
        m->needsNeighborCountRefreshed = false;

        while (knockedOutWalls < m->totalPositions - 1) {
            uint32_t r = (uint32_t)( (float)(lotteryExtent - knockedOutWalls) * random() / (RAND_MAX + 1.0) ) + knockedOutWalls;
            int32_t root1 = DisjSets_find(m->sets, m->lottery[r].cell1);
            int32_t root2 = DisjSets_find(m->sets, m->lottery[r].cell2);
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
            uint32_t r = (uint32_t)( (float)(lotteryExtent - knockedOutWalls) * random() / (RAND_MAX + 1.0) ) + knockedOutWalls;
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

    if (m->createFlags & mcfOutputMaze) {
        for (uint32_t i = 0; i < m->dims_length; ++i)
            BitArray_reset(m->halls[i]);

        for (uint32_t i = 0; i < knockedOutWalls; ++i) {
            uint32_t placeValue = 1;
            for (uint32_t d = 0; d < m->dims_length; ++d) {
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
}

void Maze_solve(MazeRef m, uint32_t start, uint32_t end) {
    if (!(m->createFlags & mcfOutputSolution)) {
        fprintf(stderr, "Error: Maze_solve cannot be called without setting mcfOutputSolution in Maze_create\n");
        return;
    }

    if (!m || !m->lottery || !m->neighborCount)
        return;

    if (m->createFlags & mcfMultipleSolves) {
        if (m->needsNeighborCountRefreshed)
            memcpy(m->neighborCount, m->neighborCountCopy, sizeof(uint8_t) * m->totalPositions);
        else
            memcpy(m->neighborCountCopy, m->neighborCount, sizeof(uint8_t) * m->totalPositions);
    } else if (m->needsNeighborCountRefreshed) {
        fprintf(stderr, "Error: Maze_solve cannot be called more than once without setting mcfMultipleSolve in Maze_create\n");
        return;
    }

    m->start = start;
    m->end = end;

    uint32_t knockedOutWalls = m->totalPositions - 1;
    while (true) {
        bool filledDeadEnd = false;
        for (uint32_t i = 0; i < knockedOutWalls; ++i) {
            const uint32_t cell1 = m->lottery[i].cell1;
            const uint32_t cell2 = m->lottery[i].cell2;
            if ((m->neighborCount[cell1] == 1 && cell1 != start && cell1 != end) || (m->neighborCount[cell2] == 1 && cell2 != start && cell2 != end)) {
                m->neighborCount[cell1]--;
                m->neighborCount[cell2]--;
                filledDeadEnd = true;
                Wall tmp = m->lottery[knockedOutWalls - 1]; // swap to the end of the knocked out wall part of the list
                m->lottery[knockedOutWalls - 1] = m->lottery[i];
                m->lottery[i] = tmp;
                knockedOutWalls--;
            }
        }
        if (!filledDeadEnd)
            break;
    }

    m->solutionLength = knockedOutWalls;
    m->needsNeighborCountRefreshed = true;

    if (m->createFlags & mcfOutputSolution) {
        for (uint32_t i = 0; i < m->dims_length; ++i)
            BitArray_reset(m->solution[i]);

        for (uint32_t i = 0; i < m->solutionLength; ++i) {
            uint32_t placeValue = 1;
            for (uint32_t d = 0; d < m->dims_length; ++d) {
                if (m->dims[d] == 1)
                    continue;
                if (m->lottery[i].cell2 == m->lottery[i].cell1 + placeValue) {
                    BitArray_setBit(m->solution[d], m->lottery[i].cell1);
                    break;
                }
                placeValue *= m->dims[d];
            }
        }
    }
}
