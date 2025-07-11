#include "acpi.h"
#include "../../basic_renderer/basic_renderer.h"
#include "../../kernel-trace/kernel_trace.h"

mcfg_header_t *prepare_acpi(rsdp2_t *rsdp)
{
    KERN_TRACE_FUNC;
    sdt_header_t *xsdt = (sdt_header_t *)rsdp->xsdt_addr;
    mcfg_header_t *mcfg_header = (mcfg_header_t *)find_table(xsdt, "MCFG");
    return mcfg_header;
}

void *find_table(sdt_header_t *sdt_header, char *signature)
{
    int entries = (sdt_header->length - sizeof(sdt_header_t)) / 8;
    for (int t = 0; t < entries; t++)
    {
        uint64_t sdt_offset = sizeof(sdt_header_t) + (t * 8);
        uint64_t sdt_ptr = (uint64_t)sdt_header + sdt_offset;
        uint64_t *ptr = (uint64_t *)(sdt_ptr);
        sdt_header_t *new_sdt_header = (sdt_header_t *)(*ptr);
        for (int i = 0; i < 4; ++i)
        {
            if (new_sdt_header->signature[i] != signature[i])
            {
                break;
            }
            if (i == 3)
            {
                return new_sdt_header;
            }
        }
    }
    return (void *)0;
}