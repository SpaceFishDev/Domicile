#ifndef PAGE_TABLE_H

#define PAGE_TABLE_H

#include "paging.h"

typedef struct
{
    uint64_t value;
} page_directory_entry_t;

enum pt_flags
{
    PRESENT = 0,
    READ_WRITE = 1,
    USERSUPER = 2,
    WRITE_THROUGH = 3,
    CACHE_DISABLED = 4,
    ACCESSED = 5,
    LARGER_PAGES = 7,
    CUSTOM0 = 9,
    CUSTOM1 = 10,
    CUSTOM2 = 11,
    NX = 63,
};

void set_flag(page_directory_entry_t *p_entry, int flag, bool enabled);
bool get_flag(page_directory_entry_t *p_entry, int flag);
void set_addr(page_directory_entry_t *p_entry, uint64_t addr);
uint64_t get_addr(page_directory_entry_t *p_entry);

typedef struct
{
    page_directory_entry_t entries[512];
} page_table_t __attribute__((aligned(0x1000)));

typedef struct
{
    page_table_t *pml4_addr;
} page_table_manager_t;

void init_page_table_manager(page_table_manager_t *manager, page_table_t *pml4_addr);
void map_memory(page_table_manager_t *manager, void *virt_addr, void *phys_addr, bool cache);

#endif