#include "malloc.h"
#include "../basic_renderer/basic_renderer.h"
#include "../kernel-trace/kernel_trace.h"

kernel_allocator_t global_kmalloc;

void init_allocator(kernel_allocator_t *allocator)
{
    allocator->buffer_size = 4096 * 32;
    allocator->heap_buffer = request_pages(&global_allocator, 32);
    allocator->block_buffer_size = 4096;
    allocator->blocks = request_page(&global_allocator);
    for (uint64_t i = 0; i < 4096; i++)
    {
        ((uint8_t *)allocator->blocks)[i] = 0;
        ((uint8_t *)allocator->heap_buffer)[i] = 0;
    }
    allocator->allocatable = 4096 * 32;
    allocator->blocks[0].ptr = allocator->heap_buffer;
    allocator->blocks[0].size = 4096;
    allocator->blocks[0].used = false;
    allocator->num_block = 1;
}

void increase_buffer_size(void **buffer, uint64_t *buffer_size)
{
    uint64_t num_pages = (((*buffer_size) * 2) / 4096) + 1;
    uint8_t *temp = request_pages(&global_allocator, num_pages);
    if (temp == 0)
    {
        printf("COULD NOT EXPAND BUFFER!\n");
        return;
    }
    uint8_t *buf = *(uint8_t **)buffer;
    for (uint64_t i = 0; i < *buffer_size; ++i)
    {
        temp[i] = buf[i];
    }
    free_pages(&global_allocator, buf, (num_pages / 2));
    *buffer = temp;
    *buffer_size *= 2;
}

void increase_block_buffer(kernel_allocator_t *allocator)
{
    increase_buffer_size((void **)&allocator->blocks, &allocator->block_buffer_size);
}
void increase_heap_buffer(kernel_allocator_t *allocator)
{
    increase_buffer_size((void **)&allocator->heap_buffer, &allocator->buffer_size);
}

void add_block(mem_block_t block)
{
    uint64_t num_block = global_kmalloc.num_block;
    uint64_t block_size = (num_block + 1) * sizeof(mem_block_t);
    if (block_size >= global_kmalloc.block_buffer_size)
    {
        increase_block_buffer(&global_kmalloc);
    }
    global_kmalloc.blocks[global_kmalloc.num_block] = block;
    ++global_kmalloc.num_block;
}

mem_block_t split_block(uint64_t block_index, uint64_t size)
{
    mem_block_t new_block;
    new_block.size = size;
    new_block.ptr = global_kmalloc.blocks[block_index].ptr;
    new_block.used = false;
    global_kmalloc.blocks[block_index].size -= size;
    global_kmalloc.blocks[block_index].ptr += size;
    return new_block;
}

void *malloc(uint64_t size)
{
    if (size == 0)
    {
        return (void *)0;
    }
    for (uint64_t i = 0; i < global_kmalloc.num_block; ++i)
    {
        if (global_kmalloc.blocks[i].used == false)
        {
            if (global_kmalloc.blocks[i].size > size)
            {
                mem_block_t block = split_block(i, size);
                block.used = true;
                add_block(block);
                return block.ptr;
            }
            else if (global_kmalloc.blocks[i].size == size)
            {
                global_kmalloc.blocks[i].used = true;
                return global_kmalloc.blocks[i].ptr;
            }
        }
    }
    uint64_t old_size = global_kmalloc.buffer_size;
    while (global_kmalloc.buffer_size < (global_kmalloc.allocatable + size))
    {
        increase_heap_buffer(&global_kmalloc);
    }
    uint8_t *base = ((uint8_t *)global_kmalloc.heap_buffer);
    base += old_size;
    mem_block_t new_block;
    new_block.ptr = base;
    new_block.size = global_kmalloc.buffer_size - old_size;
    new_block.used = false;
    add_block(new_block);
    global_kmalloc.allocatable = global_kmalloc.buffer_size;
    return malloc(size);
}
void free(void *ptr)
{
    for (uint64_t i = 0; i < global_kmalloc.num_block; ++i)
    {
        if (global_kmalloc.blocks[i].ptr == ptr)
        {
            global_kmalloc.blocks[i].used = false;
            return;
        }
    }
}
void *realloc(void *ptr, uint64_t size)
{
    mem_block_t block;
    uint64_t i = 0;
    for (i = 0; i < global_kmalloc.num_block; ++i)
    {
        if (global_kmalloc.blocks[i].ptr == ptr)
        {
            block = global_kmalloc.blocks[i];
            break;
        }
    }
    if (block.size > size)
    {
        return block.ptr;
    }
    uint8_t *buffer = malloc(size);
    if (block.ptr != 0)
    {
        for (uint64_t k = 0; k < block.size; ++k)
        {
            buffer[k] = ((uint8_t *)block.ptr)[k];
        }
        block.used = false;
        global_kmalloc.blocks[i] = block;
    }
    return buffer;
}

void *memcpy(void *restrict to, const void *restrict from, uint64_t num)
{
    uint8_t *src = from;
    uint8_t *dest = to;
    for (uint64_t i = 0; i < num; ++i)
    {
        dest[i] = src[i];
    }
    return to;
}
