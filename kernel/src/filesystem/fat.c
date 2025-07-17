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

bool cmp_file_names(char *a, char *b)
{
    int len_a = 0;
    for (int i = 0; i < 11; ++i)
    {
        ++len_a;
        if (a[i] == ' ')
        {
            break;
        }
    }
    int len_b = 0;
    for (int i = 0; i < 11; ++i)
    {
        ++len_b;
        if (b[i] == ' ')
        {
            break;
        }
    }
    if (len_a != len_b)
    {
        return false;
    }
    int len = len_a;
    for (int i = 0; i < len; ++i)
    {
        if (a[i] != b[i])
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
    temp[11] = 0;
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

void free_path(fs_path_t *path)
{
    for (int i = 0; i < path->num_dir; ++i)
        free(path->dirs[i]);
    free(path->dirs);
    free(path->file_name);
}

fs_file_t get_file(char *name, fat32_directory_t *directory)
{
    char *new_name = parse_file_name(name);
    uint64_t i = 0;
    uint64_t num_entry = 512 / sizeof(fat32_directory_t);
    fs_file_t file;
    file.base_cluster = 0;
    for (; i < num_entry; ++i)
    {
        fat32_directory_t dir = directory[i];
        if (get_file_type(dir.attributes) == F_DIRECTORY)
        {
            continue;
        }
        if (cmp_file_names(dir.file_name, new_name))
        {
            for (int j = 0; j < 8; ++j)
            {
                if (new_name[j] == ' ')
                {
                    file.name[j] = 0;
                    break;
                }
                file.name[j] = new_name[j];
            }
            int start = 8;
            for (int j = 0; j < 3; ++j)
            {
                file.extension[j] = new_name[start + j];
            }
            file.extension[3] = 0;
            file.f32_dir_entry = dir;
            file.file_size = dir.size;
            file.type = get_file_type(dir.attributes);
            file.base_cluster = get_cluster(dir.cluster_low, dir.cluster_high);
            break;
        }
        if (dir.cluster_low == 0)
        {
            break;
        }
    }
    free(new_name);
    return file;
}

fs_path_t parse_path(char *path)
{
    char **split = 0;
    uint64_t num_split = 0;

    int len = strlen(path);
    int segment_start = -1;

    for (int i = 0; i <= len; ++i)
    {
        if (path[i] != '/' && segment_start == -1)
        {
            segment_start = i;
        }
        if ((path[i] == '/' || path[i] == '\0') && segment_start != -1)
        {
            num_split++;
            segment_start = -1;
        }
    }

    split = malloc(num_split * sizeof(char *));
    uint64_t n = 0;
    segment_start = -1;

    for (int i = 0; i <= len; ++i)
    {
        if (path[i] != '/' && segment_start == -1)
        {
            segment_start = i;
        }
        if ((path[i] == '/' || path[i] == '\0') && segment_start != -1)
        {
            int segment_len = i - segment_start;
            char *str = malloc(segment_len + 1);
            memcpy(str, &path[segment_start], segment_len);
            str[segment_len] = 0;
            split[n++] = str;
            segment_start = -1;
        }
    }

    fs_path_t fp;
    fp.dirs = split;
    fp.num_dir = (n > 1) ? n - 1 : 0;
    fp.file_name = (n > 0) ? split[n - 1] : 0;
    return fp;
}

void read_cluster(fat32_manager_t *manager, uint64_t cluster, uint8_t *buffer)
{
    uint64_t sector = cluster_to_sector(cluster, manager);
    ahci_read(global_ahci_manager, manager->drive_no, sector, manager->sectors_per_cluster, buffer);
}

uint64_t read_file(fat32_manager_t *manager, fs_file_t *file, uint8_t *buffer, uint64_t size)
{
    uint64_t cluster_size = manager->sectors_per_cluster * 512;
    uint64_t bytes_read = 0;
    uint32_t cluster = file->base_cluster;

    uint8_t *temp = malloc(cluster_size);
    if (!temp)
        return 0;

    while (1)
    {
        read_cluster(manager, cluster, temp);

        uint64_t remaining_file = file->file_size - bytes_read;
        uint64_t to_copy = (remaining_file < cluster_size) ? remaining_file : cluster_size;

        if (bytes_read + to_copy > size)
            to_copy = size - bytes_read;

        memcpy(buffer + bytes_read, temp, to_copy);
        bytes_read += to_copy;

        if (bytes_read >= file->file_size || bytes_read >= size)
            break;

        fat_entry_descriptor_t desc = get_fat_descriptor(manager, cluster);
        if (desc.type == FAT_EOF)
            break;

        cluster = desc.next_cluster;
    }

    free(temp);
    return bytes_read;
}

fat32_directory_t *read_dir(fat32_manager_t *manager, fs_file_t *file)
{
    uint8_t *buffer = malloc(file->file_size);
    uint64_t size = read_file(manager, file, buffer, file->file_size);
    file->file_size = size;
    fat32_directory_t *dir = (fat32_directory_t *)buffer;
    return dir;
}

void read_file_from_path(fat32_manager_t *manager, char *path, char *buffer)
{
    fs_file_t file = get_file_from_path(manager, path);
    read_file(manager, &file, buffer, file.file_size);
}

fs_file_t get_file_from_path(fat32_manager_t *manager, char *path)
{
    fs_path_t fp = parse_path(path);
    fat32_directory_t *cur_dir_data = manager->root_dir;
    fs_file_t current_dir;
    for (uint64_t i = 0; i < fp.num_dir; ++i)
    {
        current_dir = get_dir(cur_dir_data, fp.dirs[i]);
        if (current_dir.base_cluster == 0)
        {
            free_path(&fp);
            return current_dir; // Not found
        }
        cur_dir_data = read_dir(manager, &current_dir);
    }

    fs_file_t result = get_file(fp.file_name, cur_dir_data);
    free_path(&fp);
    return result;
}

void find_subdirs_recursive(fat32_manager_t *manager, fs_file_t *file)
{
    find_subdirs(manager, file);
    if (file->num_subdir == 0)
    {
        return;
    }
    for (uint64_t i = 0; i < file->num_subdir; ++i)
    {
        find_subdirs_recursive(manager, &file->subdirs[i]);
    }
}

void find_subdirs(fat32_manager_t *manager, fs_file_t *file)
{
    fat32_directory_t *dir = read_dir(manager, file);
    uint64_t num_entry = file->file_size / sizeof(fat32_directory_t);
    uint64_t num_dir = 0;
    for (uint64_t i = 0; i < num_entry; ++i)
    {
        uint8_t file_attr = dir[i].attributes;
        file_type type = get_file_type(file_attr);
        if (type == F_DIRECTORY && dir[i].file_name[0] != '.')
        {
            num_dir++;
        }
    }
    file->subdirs = malloc(sizeof(fs_file_t *) * num_dir);
    file->num_subdir = num_dir;
    uint64_t n = 0;
    for (uint64_t i = 0; i < num_entry; ++i)
    {
        file_type type = get_file_type(dir[i].attributes);
        if (type == F_DIRECTORY)
        {
            if (dir[i].file_name[0] != '.')
            {
                char name_terminated[12];
                char *fn = dir[i].file_name;
                int i = 0;
                while (*fn != ' ')
                {
                    name_terminated[i] = *fn;
                    ++i;
                    ++fn;
                }
                name_terminated[i] = 0;
                fs_file_t directory = get_dir(&file->f32_dir_entry, name_terminated);
                file->subdirs[n] = directory;
                ++n;
            }
        }
    }
}

fs_file_t get_dir(fat32_directory_t *dir, char *name)
{
    int len = 11;
    char *name_cpy = malloc(12);
    name_cpy[11] = 0;
    memset(name_cpy, ' ', 11);
    memcpy(name_cpy, name, strlen(name));
    name = name_cpy;
    str_to_upper(name, len);
    uint64_t max_size = 512;
    uint8_t *buf = (uint8_t *)dir;
    uint64_t current_byte = 0;
    fs_file_t file;
    file.base_cluster = 0;
    int flen = 0;
    while (current_byte < max_size)
    {
        fat32_directory_t dir = *((fat32_directory_t *)(buf + current_byte));
        // if (is_lfn(&dir))
        // {
        //     printf("TODO: handle long file names\n");
        //     current_byte += sizeof(fat32_directory_t);
        //     continue;
        // }
        if (cmp_file_names(dir.file_name, name))
        {
            file.base_cluster = get_cluster(dir.cluster_low, dir.cluster_high);
            for (int i = 0; name[i] != ' '; ++i)
            {
                file.name[i] = name[i];
                ++flen;
            }
            file.type = get_file_type(dir.attributes);
            file.file_size = 512;
            file.f32_dir_entry = dir;
            break;
        }
        current_byte += sizeof(fat32_directory_t);
    }
    file.name[flen] = 0;
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
    KERN_TRACE("Reading FAT table into manager->FAT");
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

void free_file(fs_file_t *file)
{
    for (uint64_t i = 0; i < file->num_subdir; ++i)
    {
        free_file(&file->subdirs[i]);
    }
    free(file->subdirs);
}