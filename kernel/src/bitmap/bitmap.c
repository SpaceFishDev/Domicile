#include "bitmap.h"

bool bitmap_get(bitmap_t *bitmap, size_t idx)
{
    if (idx >= bitmap->size * 8)
    {
        return false;
    }
    int byte_index = idx / 8;
    uint8_t bit_index = idx % 8;
    uint8_t bit_indexer = 0b10000000 >> bit_index;
    return (bitmap->buffer[byte_index] & bit_indexer) != 0;
}

bool bitmap_set(bitmap_t *bitmap, size_t idx, bool value)
{
    if (idx >= bitmap->size * 8)
    {
        return false;
    }
    int byte_index = idx / 8;
    uint8_t bit_index = idx % 8;
    uint8_t bit_indexer = 0b10000000 >> bit_index;
    bitmap->buffer[byte_index] &= ~bit_indexer;
    if (value)
    {
        bitmap->buffer[byte_index] |= bit_indexer;
    }
    return true;
}
