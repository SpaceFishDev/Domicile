#include "paging.h"

void reserve_page(page_frame_allocator_t *allocator, void *addr);
void unreserve_page(page_frame_allocator_t *allocator, void *addr);
void reserve_pages(page_frame_allocator_t *allocator, void *addr, size_t page_count);
void unreserve_pages(page_frame_allocator_t *allocator, void *addr, size_t page_count);

uint64_t get_memory_size(mem_info_t mem_info)
{
    static uint64_t memory_size_bytes = 0;
    if (memory_size_bytes > 0)
    {
        return memory_size_bytes;
    }

    uint64_t no_entries = mem_info.map_size / mem_info.map_descriptor_size;
    for (int i = 0; i < no_entries; ++i)
    {
        efi_memory_descriptor_t *desc = (efi_memory_descriptor_t *)((uint64_t)mem_info.map + (i * mem_info.map_descriptor_size));
        memory_size_bytes += desc->no_pages * 4096;
    }

    return memory_size_bytes;
}

uint64_t free_memory;
uint64_t reserved_memory;
uint64_t used_memory;

bool intialized = false;

uint64_t get_free_memory()
{
    return free_memory;
}
uint64_t get_used_memory()
{
    return used_memory;
}
uint64_t get_reserved_memory()
{
    return reserved_memory;
}

void read_efi_memory_map(page_frame_allocator_t *allocator, mem_info_t mem_info)
{
    if (intialized)
    {
        return;
    }
    intialized = true;

    uint64_t memory_map_entries = mem_info.map_size / mem_info.map_descriptor_size;

    void *largest_free_mem_seg = 0;
    size_t largest_free_mem_seg_size = 0;
    for (int i = 0; i < memory_map_entries; ++i)
    {
        efi_memory_descriptor_t *desc = (efi_memory_descriptor_t *)((uint64_t)mem_info.map + (i * mem_info.map_descriptor_size));
        if (desc->type == 7)
        {
            if ((desc->no_pages * 4096) > largest_free_mem_seg_size)
            {
                largest_free_mem_seg = desc->phys_addr;
                largest_free_mem_seg_size = desc->no_pages * 4096;
            }
        }
    }
    uint64_t mem_size = get_memory_size(mem_info);
    free_memory = mem_size;
    uint64_t bitmap_size = ((mem_size / 4096) / 8) + 1;
    pf_allocator_init_bitmap(allocator, bitmap_size, largest_free_mem_seg);
    lock_pages(allocator, allocator->page_bitmap.buffer, allocator->page_bitmap.size / 4096 + 1);
    for (int i = 0; i < memory_map_entries; ++i)
    {
        efi_memory_descriptor_t *desc = (efi_memory_descriptor_t *)((uint64_t)mem_info.map + (i * mem_info.map_descriptor_size));
        if (desc->type != 7)
        {
            reserve_pages(allocator, desc->phys_addr, desc->no_pages);
        }
    }
}

void pf_allocator_init_bitmap(page_frame_allocator_t *allocator, size_t bitmap_size, void *buffer_addr)
{
    allocator->page_bitmap.size = bitmap_size;
    allocator->page_bitmap.buffer = (uint8_t *)buffer_addr;
    for (int i = 0; i < bitmap_size; ++i)
    {
        *((uint8_t *)(allocator->page_bitmap.buffer + i)) = 0;
    }
}

void free_page(page_frame_allocator_t *allocator, void *addr)
{
    uint64_t index = (uint64_t)addr / 4096;
    if (bitmap_get(&allocator->page_bitmap, index) == false)
    {
        return;
    }
    if (bitmap_set(&allocator->page_bitmap, index, false))
    {
        free_memory += 4096;
        used_memory -= 4096;
    }
}
void lock_page(page_frame_allocator_t *allocator, void *addr)
{
    uint64_t index = (uint64_t)addr / 4096;
    if (bitmap_get(&allocator->page_bitmap, index) == true)
    {
        return;
    }
    if (bitmap_set(&allocator->page_bitmap, index, true))
    {
        free_memory -= 4096;
        used_memory += 4096;
    }
}
void reserve_page(page_frame_allocator_t *allocator, void *addr)
{
    uint64_t index = (uint64_t)addr / 4096;
    if (bitmap_get(&allocator->page_bitmap, index) == true)
    {
        return;
    }
    if (bitmap_set(&allocator->page_bitmap, index, true))
    {
        free_memory -= 4096;
        reserved_memory += 4096;
    }
}
void unreserve_page(page_frame_allocator_t *allocator, void *addr)
{
    uint64_t index = (uint64_t)addr / 4096;
    if (bitmap_get(&allocator->page_bitmap, index) == false)
    {
        return;
    }
    if (bitmap_set(&allocator->page_bitmap, index, false))
    {
        free_memory += 4096;
        reserved_memory -= 4096;
    }
}

void free_pages(page_frame_allocator_t *allocator, void *addr, size_t page_count)
{
    for (int i = 0; i < page_count; ++i)
    {
        free_page(allocator, ((void *)((uint64_t)addr + (i * 4096))));
    }
}
void lock_pages(page_frame_allocator_t *allocator, void *addr, size_t page_count)
{
    for (int i = 0; i < page_count; ++i)
    {
        lock_page(allocator, ((void *)((uint64_t)addr + (i * 4096))));
    }
}

void unreserve_pages(page_frame_allocator_t *allocator, void *addr, size_t page_count)
{
    for (int i = 0; i < page_count; ++i)
    {
        unreserve_page(allocator, ((void *)((uint64_t)addr + (i * 4096))));
    }
}

void reserve_pages(page_frame_allocator_t *allocator, void *addr, size_t page_count)
{
    for (int i = 0; i < page_count; ++i)
    {
        reserve_page(allocator, ((void *)((uint64_t)addr + (i * 4096))));
    }
}

void *request_page(page_frame_allocator_t *allocator)
{
    for (uint64_t idx = 0; idx < allocator->page_bitmap.size * 8; ++idx)
    {
        if (!bitmap_get(&allocator->page_bitmap, idx))
        {
            lock_page(allocator, (void *)(idx * 4096));
            return (void *)(idx * 4096);
        }
    }
    return (void *)0;
}

page_frame_allocator_t global_allocator = {};