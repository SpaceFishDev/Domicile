#ifndef ACPI_H
#define ACPI_H
#include <stdint.h>

typedef struct __attribute__((packed))
{
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} rsdp2_t;

typedef struct __attribute__((packed))
{
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} sdt_header_t;

typedef struct __attribute__((packed))
{
    sdt_header_t header;
    uint64_t reserved_bytes;
} mcfg_header_t;

typedef struct __attribute__((packed))
{
    uint64_t base_addr;
    uint16_t pci_seg_grp;
    uint8_t start_bus;
    uint8_t end_bus;
    uint32_t reserved;
} device_config_t;

mcfg_header_t *prepare_acpi(rsdp2_t *rsdp);
void *find_table(sdt_header_t *sdt_header, char *signature);

#endif