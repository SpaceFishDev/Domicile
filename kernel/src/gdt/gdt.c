#include "gdt.h"

__attribute__((aligned(0x1000)))
gdt_t default_gdt = {
    {0, 0, 0, 0x00, 0x00, 0}, // null
    {0, 0, 0, 0x9a, 0xa0, 0}, // kernel code segment
    {0, 0, 0, 0x92, 0xa0, 0}, // kernel data segment
    {0, 0, 0, 0x00, 0x00, 0}, // user null
    {0, 0, 0, 0xFA, 0xA0, 0}, // user code (ring 3, execute/read)
    {0, 0, 0, 0xF2, 0xA0, 0}, // user data (ring 3, read/write)
};
