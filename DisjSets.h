/*
 *  DisjSets.h
 *  MazeInC
 *
 *  Created by Matthew T. Pandina on 11/25/09.
 *  Copyright 2009 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef DISJSETS_H
#define DISJSETS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef int *DisjSetsRef;

static inline void DisjSets_reset(DisjSetsRef djs, int size) {
    memset(djs, 0xFF, sizeof(int) * size);
//    for (int i = 0; i < size; ++i)
//        djs[i] = -1;
}

static inline DisjSetsRef DisjSets_create(int size) {
    DisjSetsRef djs = (DisjSetsRef)malloc(sizeof(int) * size);
    DisjSets_reset(djs, size);
    return djs;
}

static inline void DisjSets_delete(DisjSetsRef djs) {
    free(djs);
}

static inline void DisjSets_union(DisjSetsRef djs, int root1, int root2) {
    if (djs[root2] < djs[root1])
        djs[root1] = root2;
    else {
        if (djs[root1] == djs[root2])
            djs[root1] -= 1;
        djs[root2] = root1;
    }
}

static inline int DisjSets_find(DisjSetsRef djs, int x) {
    int root, var, prevVar;
    root = var = x;
    while (djs[root] >= 0)
        root = djs[root];
    while (djs[var] >= 0) {
        prevVar = var;
        var = djs[var];
        djs[prevVar] = root;
    }
    return root;
}

static inline bool DisjSets_sameSet(DisjSetsRef djs, int x, int y) {
    return (DisjSets_find(djs, x) == DisjSets_find(djs, y));
}

#ifdef __cplusplus
}
#endif

#endif // DISJSETS_H
