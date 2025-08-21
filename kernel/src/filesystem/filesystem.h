#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_GLOBAL_FILES 4096
#define MAX_GLOBAL_FS_MANAGERS 256
#define stdout 0
#define stdin 1
#define stderr 2

typedef enum
{
    FAT32,
    NOT_IMPLIMENTED,
} manager_type;

typedef struct file_s
{
    char *name;
    char *buffer;
    char *path;
    uint64_t size;
    void *internal_file_info;
    void (*clean_up)(struct file_s *file);
} file_t;

typedef struct
{
    manager_type type;
    uint64_t drive_no;
    void *manager;
    file_t *(*load_file)(void *manager, char *path, bool is_dir); // Should create the file if it does not exist.
    void (*write_buffer)(void *manager, file_t *file, char *buffer, uint64_t size);
    void (*make_dir)(void *manager, file_t *file);
    void (*read_buffer)(void *manager, file_t *file, char *buffer);
    void (*delete_file)(void *manager, file_t *file);
    char *current_working_directory;
} file_system_manager_t;

typedef struct
{
    file_system_manager_t *managers[MAX_GLOBAL_FS_MANAGERS];
    uint64_t num_manager;
    file_t *files[MAX_GLOBAL_FILES];
    uint64_t current_manager;
    uint64_t primary_drive;
} file_system_t;

void init_file_system(file_system_t *fs, uint64_t primary_drive);
void register_fs_manager(file_system_manager_t *manager);
uint64_t fs_open(char *path, bool directory);
void fs_write(void *buffer, uint64_t size, uint64_t f_id);
void fs_read(void *buffer, uint64_t f_id);
void fs_create_dir(uint64_t f_id);
void fs_close(uint64_t f_id);
void fs_delete(uint64_t f_id);
void fs_set_working_dir(char *path);
uint64_t fs_get_file_size(uint64_t f_id);

extern file_system_t *global_file_system;

#endif