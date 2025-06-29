
#include <stdint.h>
#include <stdbool.h>
#include "typdef.h"
#include "string.h"
#include "../basic_renderer/basic_renderer.h"

#include "../memory/paging.h"
#include "../memory/pagemapindexer.h"
#include "../memory/pagetables.h"

extern uint64_t _kernel_start;
extern uint64_t _kernel_end;

typedef struct
{
    boot_info_t *boot_info;
    page_table_manager_t *page_table_manager;
} kernel_info_t;

void init_kernel(kernel_info_t *kernel_info, boot_info_t *boot_info);