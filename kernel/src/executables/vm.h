#ifndef VM_H
#define VM_H
#include "bytecode.h"
#include <stdint.h>

typedef struct
{
    bytecode_command cmd;
    int64_t arg0;
} vm_instruction_t;

typedef struct
{
    uint64_t pos;
    uint64_t sp;
    uint64_t id;
    uint64_t *stack;
    vm_instruction_t *instructions;
    uint64_t num_ins;
} vm_t;

typedef void (*instruction_handler)(vm_t *vm);

void execute_instruction(vm_t *vm);

#endif