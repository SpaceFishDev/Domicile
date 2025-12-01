#ifndef BYTECODE_H
#define BYETCODE_H

typedef enum
{
    PUSH,
    POP,
    DUP,
    SWAP,
    READ_PTR,
    STORE_AT_PTR,
    MALLOC,
    FREE,
    DIV,
    MUL,
    ADD,
    SUB,
    CMP,
    JNE,
    JE,
    JL,
    JG,
    JMP,
    SYSCALL,
    NOP,
} bytecode_command;

#endif