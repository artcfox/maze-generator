/*
 *  BitArray.h
 *  MazeInC
 *
 *  Created by Matthew T. Pandina on 11/26/09.
 *  Copyright 2009 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef BITARRAY_H
#define BITARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct _BitArray {
    unsigned int numBits;
    unsigned char *data;
    unsigned int data_length;
} BitArray;
typedef BitArray *BitArrayRef;

static inline void BitArray_reset(BitArrayRef ba) {
    memset(ba->data, 0, ba->data_length);
}

static inline BitArrayRef BitArray_create(unsigned int numBits, bool zeroData) {
    BitArrayRef ba = (BitArrayRef)malloc(sizeof(BitArray));
    ba->numBits = numBits;
    ba->data_length = numBits / 8 + ((numBits % 8) ? 1 : 0);
    ba->data = (unsigned char*)malloc(ba->data_length);
    if (zeroData)
        BitArray_reset(ba);
    return ba;
}

static inline void BitArray_delete(BitArrayRef ba) {
    free(ba->data);
    free(ba);
}

static inline void BitArray_setBit(BitArrayRef ba, unsigned int index) {
    ba->data[index / 8] |= 1 << (index % 8);
}

static inline void BitArray_clearBit(BitArrayRef ba, unsigned int index) {
    ba->data[index / 8] &= (1 << (index % 8)) ^ 0xFF;
}

static inline bool BitArray_readBit(BitArrayRef ba, unsigned int index) {
    return (ba->data[index / 8] & 1 << (index % 8));
}

#ifdef __cplusplus
}
#endif

#endif // BITARRAY_H
