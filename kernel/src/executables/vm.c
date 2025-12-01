#include "vm.h"
#include "../memdebug/memdebug.h"
#include "../filesystem/filesystem.h"
#include "../syscalls/syscall.h"

extern uint64_t open(uint64_t fp, uint64_t is_dir, ...);

extern uint64_t read(uint64_t fd, uint64_t buf, uint64_t count, ...);

extern uint64_t write(uint64_t fd, uint64_t buf, uint64_t count, ...);

instruction_handler push(vm_t *vm)
{
    vm->stack[vm->sp] = vm->instructions[vm->pos].arg0;
    vm->sp++;
}

instruction_handler pop(vm_t *vm)
{
    vm->sp--;
}

instruction_handler add(vm_t *vm)
{
    --vm->sp;
    int64_t b = vm->stack[vm->sp];
    --vm->sp;
    int64_t a = vm->stack[vm->sp];
    int64_t res = a + b;
    vm->stack[vm->sp] = res;
    vm->sp++;
}

instruction_handler sub(vm_t *vm)
{
    --vm->sp;
    int64_t b = vm->stack[vm->sp];
    --vm->sp;
    int64_t a = vm->stack[vm->sp];
    int64_t res = a - b;
    vm->stack[vm->sp] = res;
    vm->sp++;
}

instruction_handler div(vm_t *vm)
{
    --vm->sp;
    int64_t b = vm->stack[vm->sp];
    --vm->sp;
    int64_t a = vm->stack[vm->sp];
    int64_t res = a / b;
    vm->stack[vm->sp] = res;
    vm->sp++;
}

instruction_handler mul(vm_t *vm)
{
    --vm->sp;
    int64_t b = vm->stack[vm->sp];
    --vm->sp;
    int64_t a = vm->stack[vm->sp];
    int64_t res = a * b;
    vm->stack[vm->sp] = res;
    vm->sp++;
}

instruction_handler dup(vm_t *vm)
{
    vm->stack[vm->sp] = vm->stack[vm->sp - 1];
    vm->sp++;
}

instruction_handler swap(vm_t *vm)
{
    int64_t a = vm->stack[vm->sp - 1];
    int64_t b = vm->stack[vm->sp - 2];
    int64_t temp = a;
    a = b;
    b = temp;
    vm->stack[vm->sp - 1] = a;
    vm->stack[vm->sp - 2] = b;
}

instruction_handler read_ptr(vm_t *vm)
{
    --vm->sp;
    int64_t *ptr = (int64_t *)vm->stack[vm->sp];
    vm->stack[vm->sp] = (int64_t)(*ptr);
    vm->sp++;
}

instruction_handler store_at_ptr(vm_t *vm)
{
    --vm->sp;
    int64_t value = vm->stack[vm->sp];
    --vm->sp;
    int64_t *ptr = (int64_t *)vm->stack[vm->sp];
    *ptr = value;
}
instruction_handler allocate(vm_t *vm)
{
    --vm->sp;
    int64_t size = vm->stack[vm->sp];
    size *= 8;
    int64_t ptr = (int64_t)malloc(size);
    vm->stack[vm->sp] = ptr;
    vm->sp++;
}
instruction_handler free_mem(vm_t *vm)
{
    --vm->sp;
    void *ptr = (void *)vm->stack[vm->sp];
    free(ptr);
}

instruction_handler cmp(vm_t *vm)
{
    --vm->sp;
    int64_t b = vm->stack[vm->sp];
    --vm->sp;
    int64_t a = vm->stack[vm->sp];
    int64_t res = a - b;
    vm->stack[vm->sp] = res;
    vm->sp++;
}

instruction_handler jne(vm_t *vm)
{
    --vm->sp;
    int64_t a = vm->stack[vm->sp];
    if (a != 0)
    {
        vm->pos += vm->instructions[vm->pos].arg0 - 1;
    }
}
instruction_handler je(vm_t *vm)
{
    --vm->sp;
    int64_t a = vm->stack[vm->sp];
    if (a == 0)
    {
        vm->pos += vm->instructions[vm->pos].arg0 - 1;
    }
}
instruction_handler jl(vm_t *vm)
{
    --vm->sp;
    int64_t a = vm->stack[vm->sp];
    if (a < 0)
    {
        vm->pos += vm->instructions[vm->pos].arg0 - 1;
    }
}
instruction_handler jg(vm_t *vm)
{
    --vm->sp;
    int64_t a = vm->stack[vm->sp];
    if (a > 0)
    {
        vm->pos += vm->instructions[vm->pos].arg0 - 1;
    }
}
instruction_handler jmp(vm_t *vm)
{
    vm->pos += vm->instructions[vm->pos].arg0 - 1;
}

instruction_handler syscall(vm_t *vm)
{
    int64_t syscall_no = vm->instructions[vm->pos].arg0;
    switch (syscall_no)
    {
    case SYSCALL_READ:
    {
        --vm->sp;
        uint64_t fd = vm->stack[vm->sp];
        --vm->sp;
        uint64_t buf = vm->stack[vm->sp];
        --vm->sp;
        uint64_t count = vm->stack[vm->sp];
        uint64_t res = read(fd, buf, count);
        vm->stack[vm->sp] = res;
        ++vm->sp;
    }
    break;
    case SYSCALL_WRITE:
    {
        --vm->sp;
        uint64_t fd = vm->stack[vm->sp];
        --vm->sp;
        uint64_t buf = vm->stack[vm->sp];
        --vm->sp;
        uint64_t count = vm->stack[vm->sp];
        uint64_t res = write(fd, buf, count);
        vm->stack[vm->sp] = res;
    }
    break;
    case SYSCALL_OPEN:
    {
        --vm->sp;
        uint64_t fp = vm->stack[vm->sp];
        --vm->sp;
        uint64_t is_dir = vm->stack[vm->sp];
        uint64_t fd = open(fp, is_dir);
        vm->stack[vm->sp] = fd;
        vm->sp++;
    }
    break;
    }
}
instruction_handler nop(vm_t *vm)
{
    asm("nop");
}

instruction_handler handlers[] = {
    push,
    pop,
    dup,
    swap,
    read_ptr,
    store_at_ptr,
    allocate,
    free_mem,
    div,
    mul,
    add,
    sub,
    cmp,
    jne,
    je,
    jl,
    jg,
    jmp,
    syscall,
    nop,
};

void execute_instruction(vm_t *vm)
{
    vm_instruction_t ins = vm->instructions[vm->pos];
    handlers[ins.cmd](vm);
    vm->pos++;
}

vm_t *create_vm_from_fp(char *path)
{
    uint64_t fd = fs_open(path, false);
    uint64_t size = fs_get_file_size(fd);
    char *buffer = malloc(size + 1);
    for (int i = 0; i < size + 1; ++i)
    {
        buffer[i] = 0;
    }
    fs_read(buffer, fd);
    fs_close(fd);
    vm_t *vm = malloc(sizeof(vm_t));
    uint64_t num_ins = size / 16;
    vm->instructions = malloc(sizeof(vm_instruction_t) * num_ins);
    vm->num_ins = num_ins;
    vm->pos = 0;
    vm->stack = malloc(16384);
    vm->sp = 0;
    int n = 0;
    uint64_t *buf_new = (uint64_t *)buffer;
    for (int i = 0; i < vm->num_ins; ++i)
    {
        vm_instruction_t ins;
        ins.cmd = buf_new[n++];
        ins.arg0 = buf_new[n++];
        vm->instructions[i] = ins;
    }
    free(buffer);
    return vm;
}

void destroy_vm(vm_t *vm)
{
    free(vm);
}