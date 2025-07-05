#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include "../utils/typdef.h"
#include "../utils/efi_memory.h"
#include "../bitmap/bitmap.h"

uint64_t get_memory_size(mem_info_t mem_info);

typedef struct
{
    bitmap_t page_bitmap;
} page_frame_allocator_t;

void read_efi_memory_map(page_frame_allocator_t *allocator, mem_info_t mem_info);

void pf_allocator_init_bitmap(page_frame_allocator_t *allocator, size_t bitmap_size, void *buffer_addr);

void free_page(page_frame_allocator_t *allocator, void *addr);
void lock_page(page_frame_allocator_t *allocator, void *addr);
void free_pages(page_frame_allocator_t *allocator, void *addr, size_t page_count);
void lock_pages(page_frame_allocator_t *allocator, void *addr, size_t page_count);
void *request_page(page_frame_allocator_t *allocator);
void *request_pages(page_frame_allocator_t *allocator, int num_pages);

uint64_t get_free_memory();
uint64_t get_used_memory();
uint64_t get_reserved_memory();

extern page_frame_allocator_t global_allocator;

#endif