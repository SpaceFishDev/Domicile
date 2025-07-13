#ifndef AHCI_H

#define AHCI_H
#include <stdint.h>
#include "../pci/pci.h"
#include <stdbool.h>

extern bool AHCI_exists;

#define HBA_PORT_DEVICE_PRESENT (0x3)
#define HBA_PORT_IPM_ACTIVE (0x1)

#define SATA_SIGNATURE_ATAPI 0xEB140101
#define SATA_SIGNATURE_ATA 0x00000101
#define SATA_SIGNATURE_SEMB 0xC33C0101
#define SATA_SIGNATURE_PM 0x96690101

#define HBA_PXCMD_CR 0x8000
#define HBA_PXCMD_FRE 0x0010
#define HBA_PXCMD_ST 0x0001
#define HBA_PXCMD_FR 0x4000

#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35

#define ATA_DEVICE_BUSY 0x80
#define ATA_DEVICE_DRQ 0x08

#define HBA_PXIS_TFES (1 << 30)

typedef enum ahci_port_types
{
    AHCI_NONE,
    SATA,
    SEMB,
    PM,
    SATA_PI,
} ahci_port_type;

typedef struct
{
    uint32_t command_list_base;
    uint32_t command_list_base_upper;
    uint32_t fis_base_addr;
    uint32_t fis_base_addr_upper;
    uint32_t interrupt_status;
    uint32_t interrupt_enable;
    uint32_t cmd_sts;
    uint32_t rsv0;
    uint32_t task_file_data;
    uint32_t signature;
    uint32_t sata_status;
    uint32_t sata_control;
    uint32_t sata_error;
    uint32_t sata_active;
    uint32_t command_issue;
    uint32_t sata_notif;
    uint32_t fis_switch_cntrl;
    uint32_t rsv1[11];
    uint32_t vendor_specific[4];
} hba_port_t;

typedef struct
{
    uint32_t host_capability;
    uint32_t global_host_control;
    uint32_t interrupt_status;
    uint32_t ports_impl;
    uint32_t version;
    uint32_t ccc_control;
    uint32_t ccc_ports;
    uint32_t enclosure_management_loc;
    uint32_t enclosure_management_cntrl;
    uint32_t host_capabilities_extended;
    uint32_t bios_handoff_cntrl_status;
    uint8_t rsv0[0x74];
    uint8_t vendor_specific[0x60];
    hba_port_t ports[1];

} hba_memory_t;

typedef struct
{
    uint8_t command_fis_length : 5;
    uint8_t ata_pi : 1;
    uint8_t write : 1;
    uint8_t prefetchable : 1;
    uint8_t reset : 1;
    uint8_t bist : 1;
    uint8_t clear_busy : 1;
    uint8_t rsv0 : 1;
    uint8_t port_multiplier : 4;

    uint16_t prdt_length;
    uint32_t prdb_count;
    uint32_t command_table_base_addr;
    uint32_t command_table_base_addr_upper;
    uint32_t rsv1[4];
} hba_command_header_t;

typedef struct
{
    hba_port_t *hba_port;
    ahci_port_type type;
    uint8_t *buffer;
    uint8_t port_number;
} ahci_port_t;

typedef struct
{
    pci_device_header_t *pci_base_addr;
    uint8_t initialized;
    hba_memory_t *ABAR;
    ahci_port_t ports[32];
    uint8_t port_count;
} ahci_manager_t;

typedef struct
{
    uint8_t fis_type;
    uint8_t port_multiplier : 4;
    uint8_t rsv0 : 3;
    uint8_t cmd_control : 1;
    uint8_t command;
    uint8_t feature_low;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device_register;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t feature_high;
    uint8_t count_low;
    uint8_t count_high;
    uint8_t iso_command_completion;
    uint8_t control;
    uint8_t rsv1[4];
} fis_reg_h2d_t;

typedef struct
{
    uint32_t database_addr;
    uint32_t database_addr_upper;
    uint32_t rsv0;

    uint32_t byte_count : 22;
    uint32_t rsv1 : 9;
    uint32_t interrupt_on_completion : 1;

} hba_prdt_entry_t;

typedef struct
{
    uint8_t command_fis[64];
    uint8_t ata_pi_cmd[16];
    uint8_t rsv0[48];
    hba_prdt_entry_t prdt_entry[];
} hba_command_table_t;

enum fis_types
{
    FIS_TYPE_REG_H2D = 0x27,
    FIS_TYPE_REG_D2H = 0x32,
    FIS_TYPE_DMA_ACT = 0x39,
    FIS_TYPE_DMA_SETUP = 0x41,
    FIS_TYPE_DATA = 0x46,
    FIS_TYPE_BIST = 0x58,
    FIS_TYPE_PIO_SETUP = 0x5F,
    FIS_TYPE_DEV_BITS = 0xA1,
};

void init_ahci_manager(ahci_manager_t *manager, pci_device_header_t *pci_base_addr);
void free_ahci_manager(ahci_manager_t *manager);
void probe_ports(ahci_manager_t *manager);
void ahci_configure_port(ahci_manager_t *manager, int port_number);
void ahci_start_cmd(ahci_manager_t *manager, int port_number);
void ahci_end_cmd(ahci_manager_t *manager, int port_number);
bool ahci_read(ahci_manager_t *manager, int port_number, uint64_t sector, uint32_t sector_count, void *buffer);

extern pci_device_header_t *ahci_device;

#endif