#include "kernel_util.h"

page_table_manager_t page_table_manager;
void prepare_memory(boot_info_t *boot_info)
{
    global_allocator = (page_frame_allocator_t){};
    read_efi_memory_map(&global_allocator, *boot_info->memory_info);
    uint64_t kernel_size = (uint64_t)&_kernel_end - (uint64_t)&_kernel_start;
    uint64_t kernel_pages = (uint64_t)kernel_size / 4096 + 1;
    lock_pages(&global_allocator, &_kernel_start, kernel_pages);

    page_table_t *pml4 = (page_table_t *)request_page(&global_allocator);
    memset(pml4, 0, 0x1000);
    init_page_table_manager(&page_table_manager, pml4);

    for (uint64_t t = 0; t < get_memory_size(*boot_info->memory_info); t += 0x1000)
    {
        map_memory(&page_table_manager, (void *)t, (void *)t, true);
    }

    uint64_t fb_base = (uint64_t)boot_info->frame_buffer->base_addr;
    uint64_t fb_size = (uint64_t)boot_info->frame_buffer->buffer_size + 4096;

    lock_pages(&global_allocator, (void *)fb_base, fb_size / 4096 + 1);

    for (uint64_t t = fb_base; t < (fb_base + fb_size); t += 0x1000)
    {
        map_memory(&page_table_manager, (void *)t, (void *)t, false);
    }

    asm("mov %0, %%cr3" ::"r"(pml4));
}

void init_kernel(kernel_info_t *kernel_info, boot_info_t *boot_info)
{
    prepare_memory(boot_info);
    kernel_info->page_table_manager = &page_table_manager;
}