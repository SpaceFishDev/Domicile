#ifndef FAT_H
#define FAT_H

#include <stdint.h>

#include "../device-drivers/ahci/ahci.h"

#define FAT_READ_ONLY 0x01
#define FAT_HIDDEN 0x02
#define FAT_SYSTEM 0x04
#define FAT_VOLUME_ID 0x08
#define FAT_DIRECTORY 0x10
#define FAT_ARCHIVE 0x20
#define LFN (FAT_READ_ONLY | FAT_HIDDEN | FAT_SYSTEM | FAT_VOLUME_ID)

#define FAT32_CLUSTER_FREE 0x00000000
#define FAT32_CLUSTER_RESERVED 0x00000001
#define FAT32_CLUSTER_FIRST_VALID 0x00000002
#define FAT32_CLUSTER_LAST_VALID 0x0FFFFFEF

#define FAT32_CLUSTER_BAD 0x0FFFFFF7
#define FAT32_CLUSTER_EOF_START 0x0FFFFFF8
#define FAT32_CLUSTER_EOF_END 0x0FFFFFFF

#define FAT32_CLUSTER_MASK 0x0FFFFFFF

typedef struct __attribute__((packed))
{
    uint8_t ignored0[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t no_fat;
    uint16_t no_root_dir_entry;
    uint16_t no_logical_sector;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t no_heads;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;
} bios_parameter_block_t;

typedef struct __attribute__((packed))
{
    uint32_t sectors_per_fat;
    uint8_t flags[2];
    uint8_t fat_version_major;
    uint8_t fat_version_minor;
    uint32_t root_cluster;
    uint16_t fsinfo_location;
    uint16_t backup_boot_location;
    uint8_t rsv0[12];
    uint8_t drive_no;
    uint8_t rsv1;
    uint8_t signature;
    uint8_t volume_id[4];
    uint8_t volume_label[11];
    uint8_t system_ident[8];
    uint8_t boot_code[420];
    uint16_t boot_sig;
} fat32_extended_boot_record_t;

typedef struct __attribute__((packed))
{
    uint32_t lead_signature;
    uint8_t rsv0[480];
    uint32_t signature;
    uint32_t last_known_free_cluster_count;
    uint32_t first_free_cluster;
    uint8_t rsv1[12];
    uint32_t trail_sig;
} fs_info_t;

typedef struct __attribute__((packed))
{
    uint8_t order_of_entry;
    uint16_t first_5[5];
    uint8_t attrib;
    uint8_t long_entry_type;
    uint8_t checksum;
    uint16_t next_6[6];
    uint16_t rsv0;
    uint16_t last_two[2];
} lfn_t;

typedef struct __attribute__((packed))
{
    uint8_t hours : 5;
    uint8_t minutes : 6;
    uint8_t seconds : 5;
} fat_time_t;

typedef struct __attribute__((packed))
{
    uint8_t year : 7;
    uint8_t month : 4;
    uint8_t day : 5;
} fat_date_t;

typedef struct __attribute__((packed))
{
    uint8_t file_name[11];
    uint8_t attributes;
    uint8_t rsv0;
    uint8_t creation_time_taken;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t cluster_high;
    uint16_t last_edit_time;
    uint16_t last_edit_date;
    uint16_t cluster_low;
    uint32_t size;
} fat32_directory_t;

typedef enum
{
    FAT_FREE,
    FAT_RESERVED,
    FAT_BAD_CLUSTER,
    FAT_EOF,
    FAT_DATA_CLUSTER,
} fat_entry_type;

typedef struct
{
    fat_entry_type type;
    uint32_t next_cluster;
} fat_entry_descriptor_t;

typedef enum
{
    F_READ_WRITE,
    F_READ_ONLY,
    F_DIRECTORY,
} file_type;

typedef struct fs_file
{
    char name[9];
    char extension[4];
    uint64_t file_size;
    uint64_t base_cluster;
    file_type type;
    struct fs_file *subdirs; // only used if type==DIRECTORY
    uint64_t num_subdir;
    fat32_directory_t f32_dir_entry;
} fs_file_t;

typedef struct
{
    char **dirs;
    uint64_t num_dir;
    char *file_name;
} fs_path_t;

typedef struct
{
    bios_parameter_block_t *bpb;
    fat32_extended_boot_record_t *extended_boot_record;
    fs_info_t *fs_info;
    uint8_t drive_no;
    uint64_t sectors_per_cluster;
    uint64_t sectors_per_fat;
    uint64_t root_cluster;
    uint32_t **FAT;
    fat32_directory_t *root_dir;
} fat32_manager_t;

void fat_read_bpb(bios_parameter_block_t *bpb, int drive_no);
void fat_read_f32_ext_boot_record(fat32_extended_boot_record_t *extended_boot_record, int drive_no);
void fat_read_fs_info(fs_info_t *fs_info, fat32_extended_boot_record_t *extended_boot_record, int drive_no);
void fat_find_root_directory(fat32_manager_t *manager);
void init_fat32_manager(fat32_manager_t *manager, int drive_no);

uint32_t cluster_to_sector(uint32_t cluster, fat32_manager_t *manager);
bool is_lfn(fat32_directory_t *dir);
fs_file_t get_file(char *name, fat32_directory_t *dir);
char *parse_file_name(char *name);

fs_file_t get_dir(fat32_directory_t *dir, char *name);
fs_file_t get_file_from_path(fat32_manager_t *manager, char *path);

uint64_t read_file(fat32_manager_t *manager, fs_file_t *file, uint8_t *buffer, uint64_t size);
void read_file_from_path(fat32_manager_t *manager, char *path, char *buffer);

uint64_t get_files_in_dir(fat32_manager_t *manager, fs_file_t *directory, char **files);

#endif