#include "syscall.h"
#include <stdint.h>
#include "../filesystem/filesystem.h"
#include "../memory/malloc.h"
#include "../utils/string.h"
#include "../memdebug/memdebug.h"
#include "../renderer/renderer.h"
#define SYSCALL_TABLE_SIZE 1024

typedef uint64_t (*syscall_t)(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
syscall_t syscall_table[SYSCALL_TABLE_SIZE];

uint64_t read(uint64_t fd, uint64_t buf, uint64_t count, ...)
{
    uint64_t file_size = fs_get_file_size(fd);
    if (file_size == 0)
    {
        return 0;
    }
    void *temp_buffer = malloc(file_size + 1);
    if (!temp_buffer)
    {
        return 0;
    }
    memset(temp_buffer, 0, file_size + 1);
    fs_read(temp_buffer, fd);
    const char *buffer = (const char *)buf;
    memcpy(buffer, temp_buffer, count > file_size ? file_size + 1 : count);
    ((char *)buffer)[count] = 0;
    free(temp_buffer);
    return count > file_size ? file_size + 1 : count;
}

uint64_t write(uint64_t fd, uint64_t buf, uint64_t count, ...)
{
    fs_write((void *)buf, count, fd);
    return count;
}

uint64_t open(uint64_t fp, uint64_t is_dir, ...)
{
    const char *file_path = (const char *)fp;
    return fs_open(file_path, is_dir);
}

uint64_t file_size(uint64_t fd)
{
    return fs_get_file_size(fd);
}

uint64_t mkdir(uint64_t fd)
{
    fs_create_dir(fd);
    return 0;
}

uint64_t sys_create_texture(uint64_t w, uint64_t h, ...)
{
    texture_t *tex = create_texture(w, h);
    return (uint64_t)tex;
}

uint64_t add_texture(uint64_t ptr, ...)
{
    if (ptr)
    {
        return renderer_add_texture((texture_t *)ptr);
    }
    return (uint64_t)-1;
}

uint64_t delete_texture(uint64_t id, ...)
{
    renderer_delete_texture(id);
    return 0;
}

uint64_t clear_renderer()
{
    global_basic_renderer->disabled = false;
    global_basic_renderer->disabled = true;
    renderer_clear();
    return 0;
}

uint64_t draw_renderer()
{
    renderer_draw();
    return 0;
}

void init_syscalls()
{
    memset(syscall_table, 0, SYSCALL_TABLE_SIZE * sizeof(syscall_t));
    syscall_table[SYSCALL_READ] = (syscall_t)read;
    syscall_table[SYSCALL_WRITE] = (syscall_t)write;
    syscall_table[SYSCALL_OPEN] = (syscall_t)open;
    syscall_table[SYSCALL_FILE_SIZE] = (syscall_t)file_size;
    syscall_table[SYSCALL_CREATE_TEXTURE] = (syscall_t)sys_create_texture;
    syscall_table[SYSCALL_ADD_TEXTURE] = (syscall_t)add_texture;
    syscall_table[SYSCALL_DELETE_TEXTURE] = (syscall_t)delete_texture;
    syscall_table[SYSCALL_RENDER_CLEAR] = (syscall_t)clear_renderer;
    syscall_table[SYSCALL_RENDER_DRAW] = (syscall_t)draw_renderer;
}

uint64_t syscall_c_handler(user_registers_t *regs)
{
    uint64_t return_value = (uint64_t)-1;
    if (regs->rax < SYSCALL_TABLE_SIZE && syscall_table[regs->rax] != 0)
    {
        syscall_t syscall = syscall_table[regs->rax];
        return_value = syscall(regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9);
    }
    return return_value;
}
