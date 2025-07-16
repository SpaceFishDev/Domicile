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

uint32_t get_cluster(uint16_t cluster_low, uint16_t cluster_high);
fat_entry_descriptor_t parse_fat_entry(uint32_t raw);

fat_entry_descriptor_t get_fat_descriptor(fat32_manager_t *manager, int fat_index)
{
    return parse_fat_entry((*manager->FAT)[fat_index]);
}

fat32_directory_t *get_root_dir(fat32_manager_t *manager)
{
    fat32_directory_t *dir = malloc(512);
    uint32_t root_sector = cluster_to_sector(manager->root_cluster, manager);
    ahci_read(global_ahci_manager, manager->drive_no, root_sector, 1, (uint8_t *)dir);
    return dir;
}

bool str_eq(uint8_t *strA, uint8_t *strB, int len)
{
    for (int i = 0; i < len; ++i)
    {
        if (strA[i] != strB[i])
        {
            return false;
        }
    }
    return true;
}

char *parse_file_name(char *name)
{
    int len = strlen(name);
    int i = 0;
    int dot = 0;
    while (i < len)
    {
        if (name[i] == '.')
        {
            dot = i + 1;
            break;
        }
        ++i;
    }
    char *temp = malloc(11);
    for (i = 0; i < 11; ++i)
    {
        temp[i] = ' ';
    }
    for (i = 0; i < dot - 1; ++i)
    {
        temp[i] = name[i];
    }
    int n = 8;
    for (i = dot; i < (dot + 3); ++i)
    {
        temp[n] = name[i];
        ++n;
    }
    str_to_upper(temp, 11);
    return temp;
}

file_type get_file_type(uint8_t attributes)
{
    if (attributes & FAT_DIRECTORY)
        return F_DIRECTORY;
    if (attributes & FAT_READ_ONLY)
        return F_READ_ONLY;
    return F_READ_WRITE;
}

fs_file_t get_file(char *name, fat32_directory_t *dir)
{
    uint64_t max_size = 512;
    uint8_t *buf = (uint8_t *)dir;
    uint64_t current_byte = 0;
    name = parse_file_name(name);
    fs_file_t file;
    while (current_byte < max_size)
    {
        fat32_directory_t dir = *((fat32_directory_t *)(buf + current_byte));
        if (str_eq(dir.file_name, (uint8_t *)name, 11))
        {
            file.base_cluster = get_cluster(dir.cluster_low, dir.cluster_high);
            for (int i = 8; i < 11; ++i)
            {
                file.extension[i - 8] = name[i];
            }
            for (int i = 0; i < 8; ++i)
            {
                file.name[i] = name[i];
            }
            file.type = get_file_type(dir.attributes);
            file.file_size = dir.size;
            file.f32_dir_entry = dir;
            break;
        }
        current_byte += sizeof(fat32_directory_t);
    }
    free(name);
    return file;
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
    manager->FAT = malloc(sizeof(uint32_t *) * manager->bpb->no_fat);
    uint64_t sector = manager->bpb->reserved_sectors;
    for (int i = 0; i < manager->bpb->no_fat; ++i)
    {
        manager->FAT[i] = malloc(manager->sectors_per_fat * 512);
        ahci_read(global_ahci_manager, drive_no, sector, manager->sectors_per_fat, manager->FAT[i]);
        sector += manager->sectors_per_fat;
    }
    manager->root_dir = get_root_dir(manager);
}

uint32_t cluster_to_sector(uint32_t cluster, fat32_manager_t *manager)
{
    return manager->bpb->reserved_sectors +
           (manager->bpb->no_fat * manager->sectors_per_fat) +
           ((cluster - 2) * manager->sectors_per_cluster);
}

inline uint32_t get_cluster(uint16_t cluster_low, uint16_t cluster_high)
{
    uint32_t high = cluster_high;
    uint32_t low = cluster_low;
    return (high << 16) | low;
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