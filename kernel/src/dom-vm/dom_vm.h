#ifndef DOM_VM_H
#define DOM_VM_H

#include <stdint.h>

enum {
	DOM_VM_NOP,
	DOM_VM_PUSH,
	DOM_VM_POP,
	DOM_VM_MOV,
	DOM_VM_ADD,
	DOM_VM_SUB,
	DOM_VM_MUL,
	DOM_VM_DIV,
	DOM_VM_CMP,
	DOM_VM_JMP,
	DOM_VM_JE,
	DOM_VM_JNE,
	DOM_VM_JG,
	DOM_VM_JL,
	DOM_VM_SYSCALL
};

typedef struct
{
	void* head;
	void* ptr;
	size_t len;	
} vm_mem_list;

typedef struct
{
	vm_mem_list* kernel_pages;
	vm_mem_list* virtual_mappings;
}; vm_mem_map

typedef struct
{
	uint64_t reg_a,reg_b,reg_c,reg_d,reg_e,reg_f,reg_g;
	void* stack_ptr;
	void* stack_bottom;
	uint64_t ip;
	uint8_t* instructions;
	uint64_t num_ins;
} dom_vm_t;


void execute_instruction(dom_vm_t* vm);
void init_vm(uint8_t* instructions, uint64_t num_ins);

#endif