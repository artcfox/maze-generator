/*
 *  Maze.h
 *  MazeInC
 *
 *  Created by Matthew T. Pandina on 11/25/09.
 *  Copyright 2009 Matthew T. Pandina. All rights reserved.
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
    int cell1;
    int cell2;
} Wall;

typedef struct _Maze {
    int totalPositions;
    int totalWalls;
    Wall *lottery;
    unsigned char *neighborCount, *neighborCountCopy;
    bool needsNeighborCountRefreshed;

    int *dims;
    unsigned int dims_length;
    MazeCreateFlags createFlags;
    DisjSetsRef sets;

    BitArrayRef *halls;
    BitArrayRef *solution;

    // Trivia
    int solutionLength;
    int start;
    int end;
} Maze;
typedef Maze *MazeRef;

MazeRef Maze_create(int *dims, unsigned int length, MazeCreateFlags flags);
void Maze_delete(MazeRef m);

void Maze_generate(MazeRef m);
void Maze_solve(MazeRef m, int start, int end);

#ifdef __cplusplus
}
#endif

#endif // DISJSETS_H
