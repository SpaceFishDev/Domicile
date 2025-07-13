#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include "../acpi/acpi.h"

typedef struct
{
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision_id;
    uint8_t prog_if;
    uint8_t sub_class;
    uint8_t class;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;

} pci_device_header_t;

typedef struct
{
    pci_device_header_t header;
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
    uint32_t cardbus_cis_ptr;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_base_addr;
    uint8_t capabilities_ptr;
    uint8_t rsv0;
    uint16_t rsv1;
    uint32_t rsv2;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t minimum_grant;
    uint8_t maximum_latency;
} pci_header_0_t;

void enumerate_pci(mcfg_header_t *mcfg_header);
extern char *pci_device_classes[];
char *get_vendor_name(uint16_t vendor_id);
char *get_device_name(uint16_t vendor_id, uint16_t device_id);
typedef struct
{
    uint16_t device_id, vendor_id;
    char *device_name;
} pci_device_t;

void register_device(pci_device_t device);

extern pci_device_t *pci_devices;

#endif