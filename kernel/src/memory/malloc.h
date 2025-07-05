#ifndef MALLOC_H
#define MALLOC_H
#include <stdint.h>
#include "../bitmap/bitmap.h"
#include "paging.h"

typedef struct
{
    void *ptr;
    uint64_t size;
} mem_block_t;

typedef struct
{
    void *buffer;
    bitmap_t bitmap;
    uint64_t bitmap_size;
    uint64_t buffer_size;
    mem_block_t *block_descriptors;
    uint64_t num_descriptor;
    uint64_t max_descriptor;
} kernel_allocator_t;

void init_allocator(kernel_allocator_t *allocator, int max_descriptors);
void *malloc(uint64_t size);
void free(void *ptr);
void *realloc(void *ptr, uint64_t size);

extern kernel_allocator_t global_kmalloc;

#endif