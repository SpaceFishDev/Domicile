#include "pagetables.h"
#include "pagemapindexer.h"
#include "../utils/string.h"

void set_flag(page_directory_entry_t *p_entry, int flag, bool enabled)
{
    uint64_t bit_selector = (uint64_t)1 << flag;
    p_entry->value &= ~bit_selector;
    if (enabled)
    {
        p_entry->value |= bit_selector;
    }
}

bool get_flag(page_directory_entry_t *p_entry, int flag)
{
    uint64_t bit_selector = (uint64_t)1 << flag;
    return ((p_entry->value & bit_selector) > 0) ? true : false;
}

uint64_t get_addr(page_directory_entry_t *p_entry)
{
    return (p_entry->value & 0x000ffffffffff000) >> 12;
}

void set_addr(page_directory_entry_t *p_entry, uint64_t addr)
{
    addr &= 0x000000FFFFFFFFFF;
    p_entry->value &= 0xFFF0000000000;
    p_entry->value |= (addr << 12);
}

void init_page_table_manager(page_table_manager_t *manager, page_table_t *pml4_addr)
{
    manager->pml4_addr = pml4_addr;
}

void map_memory(page_table_manager_t *manager, void *virt_addr, void *phys_addr, bool cache)
{
    page_map_indexer_t indexer;
    init_page_map_indexer(&indexer, (uint64_t)virt_addr);
    page_directory_entry_t PDE;

    PDE = manager->pml4_addr->entries[indexer.pdp_i];
    page_table_t *pdp;
    if (!get_flag(&PDE, PRESENT))
    {
        pdp = (page_table_t *)request_page(&global_allocator);
        memset(pdp, 0, 0x1000);
        set_addr(&PDE, (uint64_t)pdp >> 12);
        set_flag(&PDE, PRESENT, true);
        set_flag(&PDE, READ_WRITE, true);
        set_flag(&PDE, CACHE_DISABLED, !cache);
        manager->pml4_addr->entries[indexer.pdp_i] = PDE;
    }
    else
    {
        pdp = (page_table_t *)((uint64_t)get_addr(&PDE) << 12);
    }

    PDE = pdp->entries[indexer.pd_i];
    page_table_t *pd;
    if (!get_flag(&PDE, PRESENT))
    {
        pd = (page_table_t *)request_page(&global_allocator);
        memset(pd, 0, 0x1000);
        set_addr(&PDE, (uint64_t)pd >> 12);
        set_flag(&PDE, PRESENT, true);
        set_flag(&PDE, READ_WRITE, true);
        set_flag(&PDE, CACHE_DISABLED, !cache);
        pdp->entries[indexer.pd_i] = PDE;
    }
    else
    {
        pd = (page_table_t *)((uint64_t)get_addr(&PDE) << 12);
    }
    PDE = pd->entries[indexer.pt_i];
    page_table_t *pt;
    if (!get_flag(&PDE, PRESENT))
    {
        pt = (page_table_t *)request_page(&global_allocator);
        memset(pt, 0, 0x1000);
        set_addr(&PDE, (uint64_t)pt >> 12);
        set_flag(&PDE, PRESENT, true);
        set_flag(&PDE, READ_WRITE, true);
        set_flag(&PDE, CACHE_DISABLED, !cache);
        pd->entries[indexer.pt_i] = PDE;
    }
    else
    {
        pt = (page_table_t *)((uint64_t)get_addr(&PDE) << 12);
    }

    PDE = pt->entries[indexer.p_i];
    set_addr(&PDE, (uint64_t)phys_addr >> 12);
    set_flag(&PDE, PRESENT, true);
    set_flag(&PDE, READ_WRITE, true);
    set_flag(&PDE, CACHE_DISABLED, !cache);
    pt->entries[indexer.p_i] = PDE;
}