/*
 *  DisjSets.h
 *  MazeInC
 *
 *  Copyright 2009-2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef DISJSETS_H
#define DISJSETS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef int32_t *DisjSetsRef;

static inline void DisjSets_reset(DisjSetsRef djs, uint32_t size) {
    memset(djs, 0xFF, sizeof(int32_t) * size); // quickly sets all int32_t's to -1
    // The intent of the above code is:
    // for (uint32_t i = 0; i < size; ++i)
    //     djs[i] = -1;
}

static inline DisjSetsRef DisjSets_create(uint32_t size) {
    DisjSetsRef djs = (DisjSetsRef)malloc(sizeof(int32_t) * size);
    DisjSets_reset(djs, size);
    return djs;
}

static inline void DisjSets_delete(DisjSetsRef djs) {
    free(djs);
}

static inline void DisjSets_union(DisjSetsRef djs, int32_t root1, int32_t root2) {
    if (djs[root2] < djs[root1])
        djs[root1] = root2;
    else {
        if (djs[root1] == djs[root2])
            djs[root1] -= 1;
        djs[root2] = root1;
    }
}

static inline int DisjSets_find(DisjSetsRef djs, int32_t x) {
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

static inline bool DisjSets_sameSet(DisjSetsRef djs, int32_t x, int32_t y) {
    return (DisjSets_find(djs, x) == DisjSets_find(djs, y));
}

#ifdef __cplusplus
}
#endif

#endif // DISJSETS_H
