#include "malloc.h"
#include "../basic_renderer/basic_renderer.h"

kernel_allocator_t global_kmalloc = {};
void init_allocator(kernel_allocator_t *allocator, int max_descriptors)
{
    int bitmap_pages = ((max_descriptors / 8) / 4096) + 1;
    void *bitmap_buffer = (void *)request_pages(&global_allocator, bitmap_pages);
    int descriptor_pages = ((max_descriptors * sizeof(mem_block_t)) / 4096) + 1;
    void *descriptor_buffer = (void *)request_pages(&global_allocator, descriptor_pages);
    allocator->bitmap.buffer = (uint8_t *)bitmap_buffer;
    allocator->bitmap_size = 1;
    allocator->block_descriptors = (mem_block_t *)descriptor_buffer;
    allocator->buffer = (void *)request_page(&global_allocator);
    allocator->buffer_size = 4096;
    allocator->block_descriptors[0].ptr = allocator->buffer;
    allocator->block_descriptors[0].size = 4096;
    allocator->max_descriptor = max_descriptors;
    allocator->num_descriptor = 1;
    allocator->bitmap.size = max_descriptors;
    bitmap_set(&allocator->bitmap, 0, false);
}

void *malloc(uint64_t size)
{
    for (uint64_t i = 0; i < global_kmalloc.bitmap_size; ++i)
    {
        if (!bitmap_get(&global_kmalloc.bitmap, (size_t)i))
        {
            mem_block_t block = global_kmalloc.block_descriptors[i];
            if (block.size > size)
            {
                mem_block_t new_block = {};
                new_block.ptr = ((uint8_t *)block.ptr) + size;
                new_block.size = block.size - size;
                block.size = size;
                global_kmalloc.block_descriptors[i] = block;
                if (global_kmalloc.num_descriptor >= global_kmalloc.max_descriptor)
                {
                    return (void *)0;
                }
                global_kmalloc.block_descriptors[global_kmalloc.num_descriptor] = new_block;
                ++global_kmalloc.num_descriptor;
                ++global_kmalloc.bitmap_size;
                bitmap_set(&global_kmalloc.bitmap, i, true);
                return block.ptr;
            }
            else if (block.size == size)
            {
                bitmap_set(&global_kmalloc.bitmap, i, true);
                return block.ptr;
            }
        }
    }
    if (global_kmalloc.num_descriptor < global_kmalloc.max_descriptor)
    {
        global_kmalloc.buffer_size *= 2;
        void *temp = (void *)request_pages(&global_allocator, (global_kmalloc.buffer_size / 4096) + 1);
        for (uint64_t i = 0; i < (global_kmalloc.buffer_size / 2); ++i)
        {
            ((char *)temp)[i] = ((char *)global_kmalloc.buffer)[i];
        }
        void *old_ptr = global_kmalloc.buffer;
        for (uint64_t i = 0; i < (global_kmalloc.buffer_size / 2); ++i)
        {
            uint64_t offset = (uint64_t)global_kmalloc.block_descriptors[i].ptr - (uint64_t)old_ptr;
            global_kmalloc.block_descriptors[i].ptr = (void *)(((char *)temp) + offset);
        }
        free_pages(&global_allocator, global_kmalloc.buffer, ((global_kmalloc.buffer_size / 2) / 4096) + 1);
        global_kmalloc.buffer = temp;
        mem_block_t new_block = {};
        new_block.ptr = ((uint8_t *)global_kmalloc.buffer) + (global_kmalloc.bitmap_size / 2);
        new_block.size = global_kmalloc.buffer_size / 2;
        ++global_kmalloc.num_descriptor;
        ++global_kmalloc.bitmap_size;
        global_kmalloc.block_descriptors[global_kmalloc.num_descriptor - 1] = new_block;
        return malloc(size);
    }
    return (void *)0;
}

void free(void *ptr)
{
    for (uint64_t i = 0; i < global_kmalloc.bitmap_size; ++i)
    {
        if (global_kmalloc.block_descriptors[i].ptr == ptr)
        {
            bitmap_set(&global_kmalloc.bitmap, i, false);
            return;
        }
    }
}

void *realloc(void *ptr, uint64_t size)
{
    if (!ptr)
    {
        return malloc(size);
    }
    for (uint64_t i = 0; i < global_kmalloc.bitmap_size; ++i)
    {
        if (global_kmalloc.block_descriptors[i].ptr == ptr)
        {
            uint64_t old_size = global_kmalloc.block_descriptors[i].size;
            if (size <= old_size)
            {
                return ptr;
            }
            void *new_ptr = malloc(size);
            if (!new_ptr)
            {
                return (void *)0;
            }
            for (uint64_t j = 0; j < old_size; ++j)
            {
                ((char *)new_ptr)[j] = ((char *)global_kmalloc.block_descriptors[i].ptr)[j];
            }
            free(global_kmalloc.block_descriptors[i].ptr);
            return new_ptr;
        }
    }
    return (void *)0;
}
