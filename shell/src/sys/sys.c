#include "sys.h"

uint64_t syscall(uint64_t num, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
    __asm__ volatile("movq %0, %%rax\n" : "=r"(num));
    __asm__ volatile("movq %0, %%rdi\n" : "=r"(arg0));
    __asm__ volatile("movq %0, %%rsi\n" : "=r"(arg1));
    __asm__ volatile("movq %0, %%rdx\n" : "=r"(arg2));
    __asm__ volatile("movq %0, %%r10\n" : "=r"(arg3));
    __asm__ volatile("movq %0, %%r8\n" : "=r"(arg4));
    __asm__ volatile("movq %0, %%r9\n" : "=r"(arg5));
    __asm__ volatile("int $0x80\n");
}