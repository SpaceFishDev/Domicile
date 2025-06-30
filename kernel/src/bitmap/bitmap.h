#ifndef BITMAP_H

#define BITMAP_H
#include "../utils/typdef.h"
#include <stdbool.h>

typedef struct
{
    size_t size;
    uint8_t *buffer;
} bitmap_t;

bool bitmap_get(bitmap_t *bitmap, size_t idx);
bool bitmap_set(bitmap_t *bitmap, size_t idx, bool value);

#endif