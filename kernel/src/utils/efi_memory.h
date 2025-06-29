#ifndef EFI_MEM_H
#define EFI_MEM_H

#include <stdint.h>

typedef struct
{
    uint32_t type;
    void *phys_addr;
    void *virt_addr;
    uint64_t no_pages;
    uint64_t attribs;
} efi_memory_descriptor_t;

extern const char *EFI_MEMORY_TYPE_STRINGS[];

#endif