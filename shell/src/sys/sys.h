#ifndef SYS_H
#define SYS_H
#include <stdint.h>

enum syscall_table_values
{
    SYSCALL_READ,
    SYSCALL_WRITE,
    SYSCALL_OPEN,
    SYSCALL_FILE_SIZE,
    SYSCALL_CREATE_TEXTURE,
    SYSCALL_ADD_TEXTURE,
    SYSCALL_DELETE_TEXTURE,
    SYSCALL_RENDER_CLEAR,
    SYSCALL_RENDER_DRAW,
};
uint64_t syscall(uint64_t num, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

#endif