#ifndef PAGE_MAP_INDEXER_H

#define PAGE_MAP_INDEXER_H

#include "paging.h"

typedef struct
{
    uint64_t pdp_i;
    uint64_t pd_i;
    uint64_t pt_i;
    uint64_t p_i;
} page_map_indexer_t;

void init_page_map_indexer(page_map_indexer_t *indexer, uint64_t virt_addr);

#endif