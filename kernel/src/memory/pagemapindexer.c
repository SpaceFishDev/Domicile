#include "pagemapindexer.h"

void init_page_map_indexer(page_map_indexer_t *indexer, uint64_t virt_addr)
{
    virt_addr >>= 12;
    indexer->p_i = virt_addr & 0x1FF;
    virt_addr >>= 9;
    indexer->pt_i = virt_addr & 0x1FF;
    virt_addr >>= 9;
    indexer->pd_i = virt_addr & 0x1FF;
    virt_addr >>= 9;
    indexer->pdp_i = virt_addr & 0x1FF;
}