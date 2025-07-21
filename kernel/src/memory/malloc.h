#ifndef MALLOC_H
#define MALLOC_H
#include <stdint.h>
#include "../bitmap/bitmap.h"
#include "paging.h"

typedef struct
{
    void *ptr;
    uint64_t size;
    bool used;
} mem_block_t;

typedef struct
{
    void *heap_buffer;
    uint64_t buffer_size;
    uint64_t allocatable;
    uint64_t num_block;
    uint64_t block_buffer_size;
    mem_block_t *blocks;
} kernel_allocator_t;

void init_allocator(kernel_allocator_t *allocator);

void increase_free_block_buffer(kernel_allocator_t *allocator);
void increase_used_block_buffer(kernel_allocator_t *allocator);
void increase_heap_buffer(kernel_allocator_t *allocator);

void *malloc(uint64_t size);
void free(void *ptr);
void *realloc(void *ptr, uint64_t size);
void *memcpy(void *restrict to, const void *restrict from, uint64_t num);

extern kernel_allocator_t global_kmalloc;

#endif