/*
 *  Maze.h
 *  MazeInC
 *
 *  Copyright 2009-2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef MAZE_H
#define MAZE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include "BitArray.h"
#include "DisjSets.h"

typedef enum _MazeCreateFlags {
    mcfOutputMaze = 1,
    mcfOutputSolution = 2,
    mcfMultipleSolves = 4,
} MazeCreateFlags;

typedef struct _Wall {
    uint32_t cell1;
    uint32_t cell2;
} Wall;

typedef struct _Maze {
    uint32_t totalPositions;
    uint32_t totalWalls;
    Wall *lottery;
    uint8_t *neighborCount, *neighborCountCopy;
    bool needsNeighborCountRefreshed;

    uint32_t *dims;
    uint32_t dims_length;
    MazeCreateFlags createFlags;
    DisjSetsRef sets;

    BitArrayRef *halls;
    BitArrayRef *solution;

    // Trivia
    uint32_t solutionLength;
    uint32_t start;
    uint32_t end;
} Maze;
typedef Maze *MazeRef;

MazeRef Maze_create(uint32_t *dims, uint32_t length, MazeCreateFlags flags);
void Maze_delete(MazeRef m);

void Maze_generate(MazeRef m);
void Maze_solve(MazeRef m, uint32_t start, uint32_t end);

#ifdef __cplusplus
}
#endif

#endif // DISJSETS_H
