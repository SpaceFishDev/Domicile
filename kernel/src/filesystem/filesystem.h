#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>

typedef enum
{
    FAT32,
    NOT_IMPLIMENTED,
} manager_type;

typedef struct
{
    manager_type type;
    void *manager;
    uint64_t (*read_from_path)(void *manager, char *path, char *buffer);
    uint64_t (*get_file_size)(void *manager, char *path);
    uint64_t (*get_files)(void *manager, char *path);
} file_system_manager_t;

typedef struct
{
} file_system_t;

#endif