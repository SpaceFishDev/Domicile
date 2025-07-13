#include "pci.h"
#include "../../memory/pagetables.h"
#include "../../memory/malloc.h"
#include "../../basic_renderer/basic_renderer.h"
#include "../../kernel-trace/kernel_trace.h"
#include "../ahci/ahci.h"

pci_device_t *pci_devices;
uint64_t num_pci_device;
bool AHCI_exists = false;
pci_device_header_t *ahci_device;

void check_ahci(pci_device_header_t *pci_device_header)
{
    if (pci_device_header->class == 0x01 && pci_device_header->sub_class == 0x06 && pci_device_header->prog_if == 0x01)
    {
        AHCI_exists = true;
        ahci_device = pci_device_header;
    }
}

void enumerate_function(uint64_t device_addr, uint64_t function)
{
    uint64_t offset = function << 12;
    uint64_t func_addr = device_addr + offset;
    map_memory(global_page_table_manager, (void *)func_addr, (void *)func_addr, false);

    pci_device_header_t *pci_device_header = (pci_device_header_t *)func_addr;
    if (pci_device_header->device_id == 0)
    {
        return;
    }
    if (pci_device_header->device_id == 0xFFFF)
    {
        return;
    }
    uint64_t device_id = pci_device_header->device_id;
    char buffer[32];
    itoa_hex(device_id, buffer);
    printf("%s 0x%s 0x%x %s %s", get_vendor_name(pci_device_header->vendor_id), buffer, pci_device_header->vendor_id, pci_device_classes[pci_device_header->class], get_device_name(pci_device_header->vendor_id, pci_device_header->device_id));
    printf("\n");

    check_ahci(pci_device_header);
}
void enumerate_device(uint64_t bus_addr, uint64_t device)
{
    uint64_t offset = device << 15;
    uint64_t device_addr = bus_addr + offset;
    map_memory(global_page_table_manager, (void *)device_addr, (void *)device_addr, false);

    pci_device_header_t *pci_device_header = (pci_device_header_t *)device_addr;
    if (pci_device_header->device_id == 0)
    {
        return;
    }
    if (pci_device_header->device_id == 0xFFFF)
    {
        return;
    }
    for (uint64_t func = 0; func < 8; ++func)
    {
        enumerate_function(device_addr, func);
    }
}

void enumerate_bus(uint64_t base_addr, uint64_t bus)
{
    uint64_t offset = bus << 20;
    uint64_t bus_addr = base_addr + offset;
    map_memory(global_page_table_manager, (void *)bus_addr, (void *)bus_addr, false);

    pci_device_header_t *pci_device_header = (pci_device_header_t *)bus_addr;
    if (pci_device_header->device_id == 0)
    {
        return;
    }
    if (pci_device_header->device_id == 0xFFFF)
    {
        return;
    }
    for (uint64_t device = 0; device < 32; ++device)
    {
        enumerate_device(bus_addr, device);
    }
}

void enumerate_pci(mcfg_header_t *mcfg_header)
{
    KERN_TRACE_FUNC;
    int entries = ((mcfg_header->header.length) - sizeof(mcfg_header_t)) / sizeof(device_config_t);
    for (int t = 0; t < entries; t++)
    {
        uint64_t mcfg_offset = (sizeof(mcfg_header_t)) + (t * sizeof(device_config_t));
        uint64_t mcfg_ptr = (uint64_t)mcfg_header + mcfg_offset;
        device_config_t *new_device_config = (device_config_t *)mcfg_ptr;
        for (uint64_t bus = new_device_config->start_bus; bus < new_device_config->end_bus; ++bus)
        {
            enumerate_bus(new_device_config->base_addr, bus);
        }
    }
}

#pragma region PCI_DESCRIPTORS

char *pci_device_classes[] = {
    "Unclassified",
    "Mass Storage Controller",
    "Network Controller",
    "Display Controller",
    "Multimedia Controller",
    "Memory Controller",
    "Bridge",
    "Simple Communication Controller",
    "Base System Peripheral",
    "Input Device Controller",
    "Docking Station",
    "Processor",
    "Serial Bus Controller",
    "Wireless Controller",
    "Intelligent Controller",
    "Sattelite Communication Controller"
    "Signal Processing Controller",
    "Processing Accelerator",
    "Non-Essential Instrument",
    "Reserved",
    "Co-Processor",
    "Reserved",
    "Unnasigned Class (Vendor Specific)",
};

char *get_vendor_name(uint16_t vendor_id)
{
    switch (vendor_id)
    {
    case 0x8086:
    {
        return "Intel Corp.";
    }
    case 0x1022:
    {
        return "AMD";
    }
    case 0x1dDE:
    {
        return "NVIDIA Corporation";
    }
    case 0x1234:
    {
        return "QEMU (Technical Corp.)";
    }
    }
    return "UNK VENDOR";
}
#define MAX_PCI_DEVICES 1024

void register_device(pci_device_t device)
{
    if (num_pci_device > MAX_PCI_DEVICES)
    {
        printf("COULD NOT REGISTER PCI DEVICE '%s'\n", device.device_name);
        return;
    }
    if (!num_pci_device)
    {
        pci_devices = malloc(sizeof(pci_device_t) * MAX_PCI_DEVICES);
    }
    pci_devices[num_pci_device] = device;
    ++num_pci_device;
}

char *get_device_name(uint16_t vendor_id, uint16_t device_id)
{
    for (int i = 0; i < num_pci_device; ++i)
    {
        pci_device_t device = pci_devices[i];
        if (device.device_id == device_id && device.vendor_id == vendor_id)
        {
            return device.device_name;
        }
    }
    return "UNK Device";
}
#pragma endregion