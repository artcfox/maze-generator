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

// For maximum portability, this file may be compiled as C++, or C, and it
// will automatically switch between using pthreads or std::thread.

#ifdef __cplusplus
#include <thread>
#else
#include <pthread.h>
#endif

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
    m->cores = 1; // default to single core solves; for multi-core solves, call Maze_setCores() after calling Maze_create()

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

void Maze_setCores(MazeRef m, uint32_t value) {
    if (value > 0 && value <= 1024)
        m->cores = value;
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

typedef struct _DeadEndFillInfo {
    MazeRef m;
    uint32_t startWall;
    uint32_t endWall;
    uint32_t knockedOutWalls;
} DeadEndFillInfo;

void *deadEndFillThreaded(void *arg) {
    DeadEndFillInfo *defi = (DeadEndFillInfo*)arg;

    uint32_t knockedOutWalls = defi->endWall;
    while (true) {
        bool filledDeadEnd = false;
        for (uint32_t i = defi->startWall; i < knockedOutWalls; ++i) {
            const uint32_t cell1 = defi->m->lottery[i].cell1;
            const uint32_t cell2 = defi->m->lottery[i].cell2;
            if ((defi->m->neighborCount[cell1] == 1 && cell1 != defi->m->start && cell1 != defi->m->end) || (defi->m->neighborCount[cell2] == 1 && cell2 != defi->m->start && cell2 != defi->m->end)) {
                __atomic_fetch_sub(&defi->m->neighborCount[cell1], 1, __ATOMIC_SEQ_CST); // defi->m->neighborCount[cell1]--;
                __atomic_fetch_sub(&defi->m->neighborCount[cell2], 1, __ATOMIC_SEQ_CST); // defi->m->neighborCount[cell2]--;
                filledDeadEnd = true;
                Wall tmp = defi->m->lottery[knockedOutWalls - 1]; // swap to the end of the knocked out wall part of the list
                defi->m->lottery[knockedOutWalls - 1] = defi->m->lottery[i];
                defi->m->lottery[i] = tmp;
                knockedOutWalls--;
            }
        }
        if (!filledDeadEnd)
            break;
    }
    defi->knockedOutWalls = knockedOutWalls - defi->startWall;

    return 0;
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

    if (m->cores > 1) {
        // For a parallel solve, we want the list of walls sorted, so when it gets distributed among threads,
        // each thread gets to work on a contiguous section of the maze across all of its dimensions.
        // The fastest way I can think of to get the list of walls sorted in-place is simply to read (in
        // sorted order) from the halls[] bit arrays that the Maze_generate() function creates, and use that
        // to re-write the beginning of the lottery[] array (up to knockedOutWalls), causing them to all be
        // written in sorted order. This has the added benefit of not requiring any extra memory.

        uint32_t knockedOutWallsIndex = 0;
        for (uint32_t position = 0; position < m->totalPositions; ++position) {
            uint32_t placeValue = 1;
            for (uint32_t i = 0; i < m->dims_length; ++i) {
                uint32_t valueForThisDim = (position / placeValue) % m->dims[i];
                if (valueForThisDim < m->dims[i] - 1) {
                    if (BitArray_readBit(m->halls[i], position)) {
                        m->lottery[knockedOutWallsIndex].cell1 = position;
                        m->lottery[knockedOutWallsIndex].cell2 = position + placeValue;
                        knockedOutWallsIndex++;
                    }
                }
                placeValue *= m->dims[i];
            }
        }

        DeadEndFillInfo defi[m->cores];
        for (uint32_t i = 0; i < m->cores; ++i) {
            defi[i].m = m;
            defi[i].startWall = i * (m->totalPositions - 1) / m->cores;
            defi[i].endWall = (i + 1) * (m->totalPositions - 1) / m->cores;
            defi[i].knockedOutWalls = 0;
        }

#ifdef __cplusplus
        std::thread t[m->cores];
        for (uint32_t i = 0; i < m->cores; ++i)
            t[i] = std::thread(deadEndFillThreaded, (void*)&defi[i]);
        for (uint32_t i = 0; i < m->cores; ++i)
            t[i].join();
#else
        pthread_t t[m->cores];
        for (uint32_t i = 0; i < m->cores; ++i) {
            int rc = pthread_create(&t[i], NULL, deadEndFillThreaded, (void*)&defi[i]);
            if (rc) {
                fprintf(stderr, "Error: pthread_create() returned code %d\n", rc);
                exit(-1);
            }
        }
        for (uint32_t i = 0; i < m->cores; ++i) {
            void *status;
            int rc = pthread_join(t[i], &status);
            if (rc) {
                fprintf(stderr, "Error: pthread_join() returned code %d\n", rc);
                exit(-1);
            }
        }
#endif
        uint32_t knockedOutWalls = 0;
        for (uint32_t i = 0; i < m->cores; ++i)
            knockedOutWalls += defi[i].knockedOutWalls;

        // Reorder the lottery, so every sub-solution is joined at the beginning
        uint32_t writeAt = defi[0].startWall + defi[0].knockedOutWalls;
        for (uint32_t i = 1; i < m->cores; ++i) {
            for (uint32_t j = 0; j < defi[i].knockedOutWalls; ++j) {
                Wall tmp = m->lottery[writeAt];
                m->lottery[writeAt] = m->lottery[defi[i].startWall + j];
                m->lottery[defi[i].startWall + j] = tmp;
                writeAt++;
            }
        }

        // Do one final pass after each thread has finished doing its work to catch paths that crossed thread boundaries
        DeadEndFillInfo finalPass;
        finalPass.m = m;
        finalPass.startWall = 0;
        finalPass.endWall = knockedOutWalls;
        finalPass.knockedOutWalls = 0;
        deadEndFillThreaded(&finalPass);
        m->solutionLength = knockedOutWalls - (knockedOutWalls - finalPass.knockedOutWalls);
    } else {
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
    }

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
