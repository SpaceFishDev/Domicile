#ifndef KERNEL_TRACE_H

#define KERNEL_TRACE_H
#include "../basic_renderer/basic_renderer.h"
#include "../utils/kernel_util.h"

typedef struct trace_token_proto
{
    struct trace_token_proto *prev;
    int line;
    char *file;
    char *msg;
    struct trace_token_proto *next;
} trace_token_t;

typedef struct
{
    trace_token_t *trace_tokens;
    trace_token_t *tail;
    uint64_t num_token;
    bool logging;
} trace_manager_t;

void kernel_trace(char *msg, char *file, int line);
void init_kernel_trace();
void free_kernel_trace();
void dump_trace();
void kernel_trace_clear();

#define KERN_TRACE(msg) (kernel_trace(msg, __FILE__, __LINE__))
#define KERN_TRACE_FUNC (KERN_TRACE((char *)__func__))

extern trace_manager_t *global_trace_manager;

#endif