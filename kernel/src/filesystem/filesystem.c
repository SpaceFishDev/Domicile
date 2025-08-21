#include "filesystem.h"
#include "../basic_renderer/basic_renderer.h"
#include "../memory/malloc.h"

file_system_t *global_file_system;

void init_file_system(file_system_t *fs, uint64_t primary_drive)
{
    fs->num_manager = 0;
    for (int i = 0; i < MAX_GLOBAL_FILES; ++i)
    {
        fs->files[i] = 0;
    }
    fs->files[stdout] = (file_t *)1;
    fs->files[stdin] = (file_t *)1;
    fs->files[stderr] = (file_t *)1;
    fs->current_manager = -1;
    fs->primary_drive = primary_drive;
}

void register_fs_manager(file_system_manager_t *manager)
{
    if (global_file_system->num_manager >= MAX_GLOBAL_FS_MANAGERS)
    {
        printf("Could Not Register Filesystem 0x%x\n", manager);
        return;
    }
    global_file_system->managers[global_file_system->num_manager] = manager;
    global_file_system->num_manager++;
    if (global_file_system->current_manager == -1)
    {
        global_file_system->current_manager = 0;
    }
    if (manager->drive_no == global_file_system->primary_drive)
    {
        global_file_system->current_manager = global_file_system->num_manager - 1;
    }
}

uint64_t fs_open(char *p, bool directory)
{
    uint64_t manager = global_file_system->current_manager;
    file_system_manager_t *man = global_file_system->managers[manager];
    char *path = malloc(strlen(man->current_working_directory) + strlen(p) + 2);
    for (uint64_t i = 0; i < strlen(man->current_working_directory); ++i)
    {
        path[i] = man->current_working_directory[i];
    }
    uint64_t i = strlen(man->current_working_directory) + 1;
    path[i - 1] = '/';
    for (uint64_t j = 0; j < strlen(p); ++j)
    {
        path[i] = p[j];
        ++i;
    }

    file_t *file = man->load_file(man->manager, path, directory);

    for (i = 0; i < MAX_GLOBAL_FILES; ++i)
    {
        if (global_file_system->files[i] == 0)
        {
            global_file_system->files[i] = file;
            return i;
        }
    }
    return -1;
}

void fs_write(void *buffer, uint64_t size, uint64_t f_id)
{
    if (f_id == stdout || f_id == stdin || f_id == stderr)
    {
        // handle this later when proper console is implimented
        return;
    }
    if (f_id > MAX_GLOBAL_FILES)
    {
        return;
    }
    file_t *file = global_file_system->files[f_id];
    if (file == 0)
    {
        return;
    }
    file->buffer = buffer;
    uint64_t current_manager = global_file_system->current_manager;
    file_system_manager_t *manager = global_file_system->managers[current_manager];
    manager->write_buffer(manager->manager, file, (char *)buffer, size);
}

void fs_read(void *buffer, uint64_t f_id)
{
    if (f_id == stdout || f_id == stdin || f_id == stderr)
    {
        return;
    }
    if (f_id > MAX_GLOBAL_FILES)
    {
        return;
    }
    file_t *file = global_file_system->files[f_id];
    if (file == 0)
    {
        return;
    }
    uint64_t current_manager = global_file_system->current_manager;
    file_system_manager_t *manager = global_file_system->managers[current_manager];
    manager->read_buffer(manager->manager, file, (char *)buffer);
}

void fs_close(uint64_t f_id)
{
    if (f_id == stdout || f_id == stdin || f_id == stderr)
    {
        return;
    }
    if (f_id > MAX_GLOBAL_FILES)
    {
        return;
    }
    file_t *file = global_file_system->files[f_id];
    if (file == 0)
    {
        return;
    }
    free(file);
    global_file_system->files[f_id] = 0;
}

void fs_set_working_dir(char *path)
{
    global_file_system->managers[global_file_system->current_manager]->current_working_directory = path;
}

void fs_delete(uint64_t f_id)
{
    if (f_id == stdout || f_id == stdin || f_id == stderr)
    {
        // handle this later when proper console is implimented
        return;
    }
    if (f_id > MAX_GLOBAL_FILES)
    {
        return;
    }
    file_t *file = global_file_system->files[f_id];
    if (file == 0)
    {
        return;
    }
    uint64_t current_manager = global_file_system->current_manager;
    file_system_manager_t *manager = global_file_system->managers[current_manager];
    manager->delete_file(manager->manager, file);
}
uint64_t fs_get_file_size(uint64_t f_id)
{
    if (f_id == stdout || f_id == stdin || f_id == stderr)
    {
        // handle this later when proper console is implimented
        return 0;
    }
    if (f_id > MAX_GLOBAL_FILES)
    {
        return 0;
    }
    file_t *file = global_file_system->files[f_id];
    if (file == 0)
    {
        return 0;
    }
    return file->size;
}
void fs_create_dir(uint64_t f_id)
{
    file_system_manager_t *man = global_file_system->managers[global_file_system->current_manager];
    if (f_id == stdout || f_id == stdin || f_id == stderr)
    {
        return;
    }
    if (f_id > MAX_GLOBAL_FILES)
    {
        return;
    }
    file_t *file = global_file_system->files[f_id];
    man->make_dir(man, file);
}