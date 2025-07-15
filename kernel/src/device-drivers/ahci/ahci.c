#include "ahci.h"
#include "../../kernel-trace/kernel_trace.h"
#include "../../basic_renderer/basic_renderer.h"

ahci_manager_t *global_ahci_manager;

void init_ahci_manager(ahci_manager_t *manager, pci_device_header_t *pci_base_addr)
{
    if (!manager->initialized)
    {
        KERN_TRACE_FUNC;
        manager->pci_base_addr = pci_base_addr;
        manager->initialized = 1;
        printf("AHCI Manager Initialized\n");

        manager->ABAR = (hba_memory_t *)((pci_header_0_t *)pci_base_addr)->BAR5;

        map_memory(global_page_table_manager, manager->ABAR, manager->ABAR, false);
        probe_ports(manager);

        for (int i = 0; i < manager->port_count; ++i)
        {
            ahci_port_t port = manager->ports[i];
            ahci_configure_port(manager, i);
            manager->ports[i].buffer = request_page(&global_allocator);
            memset(manager->ports[i].buffer, 0, 0x1000);
        }
        global_ahci_manager = manager;
    }
}

void free_ahci_manager(ahci_manager_t *manager)
{
}

ahci_port_type check_port_type(hba_port_t *port)
{
    uint32_t sata_status = port->sata_status;

    uint8_t interface_power_management = (sata_status >> 8) & 0b111;

    uint8_t device_detection = sata_status & 0b111;

    if (device_detection != HBA_PORT_DEVICE_PRESENT)
    {
        return AHCI_NONE;
    }
    if (interface_power_management != HBA_PORT_IPM_ACTIVE)
    {
        return AHCI_NONE;
    }

    switch (port->signature)
    {
    case SATA_SIGNATURE_ATAPI:
    {
        return SATA_PI;
    }
    case SATA_SIGNATURE_ATA:
    {
        return SATA;
    }
    case SATA_SIGNATURE_PM:
    {
        return PM;
    }
    case SATA_SIGNATURE_SEMB:
    {
        return SEMB;
    }
    default:
    {
        return AHCI_NONE;
    }
    }
}

void probe_ports(ahci_manager_t *manager)
{
    KERN_TRACE_FUNC;
    uint32_t ports_impl = manager->ABAR->ports_impl;
    int num_port = 0;
    for (int i = 0; i < 32; ++i)
    {
        if (ports_impl & (1 << i))
        {
            ahci_port_type port_type = check_port_type(&manager->ABAR->ports[i]);
            if (port_type == SATA || port_type == SATA_PI)
            {
                ahci_port_t port;
                port.type = port_type;
                port.hba_port = &(manager->ABAR->ports[i]);
                port.port_number = manager->port_count;
                manager->ports[manager->port_count] = port;
                ++num_port;
                manager->port_count = num_port;
            }
        }
    }
}

ahci_port_t *get_port(ahci_manager_t *manager, int port_number)
{
    uint64_t base = manager->ports;
    uint64_t addr = base + (sizeof(ahci_port_t) * port_number);
    return (ahci_port_t *)addr;
}

void ahci_configure_port(ahci_manager_t *manager, int port_number)
{
    ahci_port_t *port = get_port(manager, port_number);
    ahci_end_cmd(manager, port_number);

    void *new_base = request_page(&global_allocator);
    port->hba_port->command_list_base = (uint32_t)(uint64_t)new_base;
    port->hba_port->command_list_base_upper = (uint32_t)((uint64_t)new_base >> 32);
    memset((void *)port->hba_port->command_list_base, 0, 1024);

    void *fis_base = request_page(&global_allocator);
    port->hba_port->fis_base_addr = (uint32_t)(uint64_t)fis_base;
    port->hba_port->fis_base_addr_upper = (uint32_t)((uint64_t)fis_base >> 32);
    memset((void *)fis_base, 0, 256);

    hba_command_header_t *cmd_header = (hba_command_header_t *)((uint64_t)port->hba_port->command_list_base + ((uint64_t)port->hba_port->command_list_base_upper << 32));

    for (int i = 0; i < 32; ++i)
    {
        cmd_header[i].prdt_length = 8;
        void *cmd_table_addr = request_page(&global_allocator);
        uint64_t addr = ((uint64_t)cmd_table_addr) + (i << 8);
        cmd_header[i].command_table_base_addr = (uint32_t)addr;
        cmd_header[i].command_table_base_addr_upper = (uint64_t)addr >> 32;
        memset(cmd_table_addr, 0, 256);
    }

    ahci_start_cmd(manager, port_number);
}

void ahci_start_cmd(ahci_manager_t *manager, int port_number)
{
    ahci_port_t *port = get_port(manager, port_number);
    while (port->hba_port->cmd_sts & HBA_PXCMD_CR)
        ;
    port->hba_port->cmd_sts |= HBA_PXCMD_FRE;
    port->hba_port->cmd_sts |= HBA_PXCMD_ST;
}

void ahci_end_cmd(ahci_manager_t *manager, int port_number)
{
    ahci_port_t *port = get_port(manager, port_number);
    port->hba_port->cmd_sts &= ~HBA_PXCMD_ST;
    port->hba_port->cmd_sts &= ~HBA_PXCMD_FRE;
    while (1)
    {
        if (port->hba_port->cmd_sts & HBA_PXCMD_FR)
        {
            continue;
        }
        if (port->hba_port->cmd_sts & HBA_PXCMD_CR)
        {
            continue;
        }
        break;
    }
}

bool ahci_read(ahci_manager_t *manager, int port_number, uint64_t sector, uint32_t sector_count, void *buffer)
{

    ahci_port_t *port = get_port(manager, port_number);

    // wait for other commands to finish.
    uint64_t spin = 0;
    while ((port->hba_port->task_file_data & (ATA_DEVICE_BUSY | ATA_DEVICE_DRQ)) && spin < 1000000)
    {
        ++spin;
    }
    if (spin == 1000000)
    {
        return false;
    }

    uint32_t sector_l = (uint32_t)sector;
    uint32_t sector_h = (uint32_t)(sector >> 32);
    port->hba_port->interrupt_status = (uint32_t)-1;
    hba_command_header_t *cmd_header = (hba_command_header_t *)port->hba_port->command_list_base;
    cmd_header->command_fis_length = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd_header->write = 0;
    cmd_header->prdt_length = 1;

    hba_command_table_t *command_table = (hba_command_table_t *)(cmd_header->command_table_base_addr);
    memset(command_table, 0, sizeof(hba_command_table_t) + (cmd_header->prdt_length - 1) * sizeof(hba_prdt_entry_t));

    command_table->prdt_entry[0].database_addr = (uint32_t)(uint64_t)buffer;
    command_table->prdt_entry[0].database_addr_upper = (uint32_t)((uint64_t)buffer >> 32);
    command_table->prdt_entry[0].byte_count = (sector_count << 9) - 1;
    command_table->prdt_entry[0].interrupt_on_completion = 1;

    fis_reg_h2d_t *command_fis = (fis_reg_h2d_t *)(&command_table->command_fis);
    command_fis->fis_type = FIS_TYPE_REG_H2D;
    command_fis->cmd_control = 1;
    command_fis->command = ATA_CMD_READ_DMA_EX;

    command_fis->lba0 = (uint8_t)sector_l;
    command_fis->lba1 = (uint8_t)(sector_l >> 8);
    command_fis->lba2 = (uint8_t)(sector_l >> 16);
    command_fis->lba3 = (uint8_t)(sector_h);
    command_fis->lba4 = (uint8_t)(sector_h >> 8);
    command_fis->lba5 = (uint8_t)(sector_h >> 16);
    command_fis->device_register = 1 << 6; // LBA Mode

    command_fis->count_low = sector_count & 0xFF;
    command_fis->count_high = (sector_count >> 8) & 0xFF;

    port->hba_port->command_issue = 1;

    while (true)
    {
        if ((port->hba_port->command_issue == 0))
            break;
        if ((port->hba_port->interrupt_status & HBA_PXIS_TFES))
        {
            return false; // failure
        }
    }

    return true;
}

bool ahci_write(ahci_manager_t *manager, int port_number, uint64_t sector, uint32_t sector_count, void *buffer)
{
    ahci_port_t *port = get_port(manager, port_number);

    // Wait until port is not busy
    uint64_t spin = 0;
    while ((port->hba_port->task_file_data & (ATA_DEVICE_BUSY | ATA_DEVICE_DRQ)) && spin < 1000000)
    {
        ++spin;
    }
    if (spin == 1000000)
    {
        return false;
    }

    uint32_t sector_l = (uint32_t)sector;
    uint32_t sector_h = (uint32_t)(sector >> 32);
    port->hba_port->interrupt_status = (uint32_t)-1;

    hba_command_header_t *cmd_header = (hba_command_header_t *)port->hba_port->command_list_base;
    cmd_header->command_fis_length = sizeof(fis_reg_h2d_t) / sizeof(uint32_t); // in DWORDS
    cmd_header->write = 1;                                                     // <- This indicates it's a write
    cmd_header->prdt_length = 1;

    hba_command_table_t *command_table = (hba_command_table_t *)(cmd_header->command_table_base_addr);
    memset(command_table, 0, sizeof(hba_command_table_t) + (cmd_header->prdt_length - 1) * sizeof(hba_prdt_entry_t));

    command_table->prdt_entry[0].database_addr = (uint32_t)(uint64_t)buffer;
    command_table->prdt_entry[0].database_addr_upper = (uint32_t)((uint64_t)buffer >> 32);
    command_table->prdt_entry[0].byte_count = (sector_count << 9) - 1; // 512 bytes per sector
    command_table->prdt_entry[0].interrupt_on_completion = 1;

    fis_reg_h2d_t *command_fis = (fis_reg_h2d_t *)(&command_table->command_fis);
    command_fis->fis_type = FIS_TYPE_REG_H2D;
    command_fis->cmd_control = 1;                // This is a command
    command_fis->command = ATA_CMD_WRITE_DMA_EX; // <- This is the command for write

    command_fis->lba0 = (uint8_t)sector_l;
    command_fis->lba1 = (uint8_t)(sector_l >> 8);
    command_fis->lba2 = (uint8_t)(sector_l >> 16);
    command_fis->lba3 = (uint8_t)sector_h;
    command_fis->lba4 = (uint8_t)(sector_h >> 8);
    command_fis->lba5 = (uint8_t)(sector_h >> 16);
    command_fis->device_register = 1 << 6; // LBA mode

    command_fis->count_low = sector_count & 0xFF;
    command_fis->count_high = (sector_count >> 8) & 0xFF;

    port->hba_port->command_issue = 1;

    // Wait for command to complete
    while (true)
    {
        if ((port->hba_port->command_issue == 0))
            break;
        if ((port->hba_port->interrupt_status & HBA_PXIS_TFES))
        {
            return false; // failure
        }
    }

    return true;
}