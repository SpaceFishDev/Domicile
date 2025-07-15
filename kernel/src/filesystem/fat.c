#include "fat.h"
#include "../memory/paging.h"
#include "../memory/malloc.h"
#include "../kernel-trace/kernel_trace.h"

void fat_read_bpb(bios_parameter_block_t *bpb, int drive_no)
{
    KERN_TRACE_FUNC;
    ahci_read(global_ahci_manager, drive_no, 0, 1, global_ahci_manager->ports[drive_no].buffer);
    uint8_t *buffer = global_ahci_manager->ports[drive_no].buffer;
    *bpb = *((bios_parameter_block_t *)buffer);
}

void fat_read_f32_ext_boot_record(fat32_extended_boot_record_t *extended_boot_record, int drive_no)
{
    KERN_TRACE_FUNC;
    ahci_read(global_ahci_manager, drive_no, 0, 1, global_ahci_manager->ports[drive_no].buffer);
    uint8_t *buffer = global_ahci_manager->ports[drive_no].buffer;
    buffer += sizeof(bios_parameter_block_t);
    *extended_boot_record = *((fat32_extended_boot_record_t *)buffer);
}

void fat_read_fs_info(fs_info_t *fs_info, fat32_extended_boot_record_t *extended_boot_record, int drive_no)
{
    KERN_TRACE_FUNC;
    uint32_t sector = extended_boot_record->fsinfo_location;
    uint8_t *buffer = request_page(&global_allocator);
    ahci_read(global_ahci_manager, drive_no, sector, 1, buffer);
    *fs_info = *((fs_info_t *)buffer);
}

fat32_directory_t get_root_dir(fat32_manager_t *manager)
{
    int drive_no = manager->drive_no;
    uint32_t root_cluster = manager->root_cluster;
    uint32_t root_sector = cluster_to_sector(root_cluster, manager);
    uint32_t size_of_cluster = manager->sectors_per_cluster;
    size_of_cluster *= 512;
}

void init_fat32_manager(fat32_manager_t *manager, int drive_no)
{
    KERN_TRACE_FUNC;
    bios_parameter_block_t *bpb = malloc(sizeof(bios_parameter_block_t));
    fat_read_bpb(bpb, drive_no);
    fat32_extended_boot_record_t *extended_boot_record = malloc(sizeof(fat32_extended_boot_record_t));
    fat_read_f32_ext_boot_record(extended_boot_record, drive_no);
    fs_info_t *fs_info = malloc(sizeof(fs_info_t));
    fat_read_fs_info(fs_info, extended_boot_record, drive_no);
    manager->drive_no = drive_no;
    manager->bpb = bpb;
    manager->extended_boot_record = extended_boot_record;
    manager->fs_info = fs_info;
    manager->sectors_per_cluster = bpb->sectors_per_cluster;
    manager->sectors_per_fat = extended_boot_record->sectors_per_fat;
    manager->root_cluster = extended_boot_record->root_cluster;
    manager->FAT = malloc(sizeof(uint32_t *));
    uint64_t sector = manager->bpb->reserved_sectors;
    // for (int i = 0; i < manager->bpb->no_fat; ++i)
    // {
    manager->FAT[0] = malloc(manager->sectors_per_fat * 512);
    printf("%u\n", manager->sectors_per_fat);
    printf("FAT PTR: 0x%x\n", manager->FAT[0]);
    // ahci_read(global_ahci_manager, drive_no, sector, manager->sectors_per_fat, manager->FAT[i]);
    // sector += manager->sectors_per_fat;
    // }
}

uint32_t cluster_to_sector(uint32_t cluster, fat32_manager_t *manager)
{
    return manager->bpb->reserved_sectors +
           (manager->bpb->no_fat * manager->sectors_per_fat) +
           ((cluster - 2) * manager->sectors_per_cluster);
}

bool is_lfn(fat32_directory_t *dir)
{
    return (dir->attributes & LFN) == LFN;
}

fat_entry_descriptor_t parse_fat_entry(uint32_t raw)
{
    raw &= FAT32_CLUSTER_MASK;

    if (raw == FAT32_CLUSTER_FREE)
        return (fat_entry_descriptor_t){FAT_FREE, 0};
    else if (raw < FAT32_CLUSTER_FIRST_VALID)
        return (fat_entry_descriptor_t){FAT_RESERVED, raw};
    else if (raw == FAT32_CLUSTER_BAD)
        return (fat_entry_descriptor_t){FAT_BAD_CLUSTER, 0};
    else if (raw >= FAT32_CLUSTER_EOF_START && raw <= FAT32_CLUSTER_EOF_END)
        return (fat_entry_descriptor_t){FAT_EOF, 0};
    else
        return (fat_entry_descriptor_t){FAT_DATA_CLUSTER, raw};
}