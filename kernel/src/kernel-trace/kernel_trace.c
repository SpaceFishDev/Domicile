#include "kernel_trace.h"

trace_manager_t *global_trace_manager;

void init_kernel_trace()
{
    global_trace_manager = malloc(sizeof(trace_manager_t));
    global_trace_manager->num_token = 0;
    global_trace_manager->logging = 0;
    KERN_TRACE("Init Trace Manager");
}

void add_trace_token(trace_token_t token);

void kernel_trace(char *msg, char *file, int line)
{
    trace_token_t token = (trace_token_t){0, line, file, msg, 0};
    if (global_trace_manager->logging != 0)
    {
        printf("KERNEL TRACE [  %s : %u  ]: '%s'\n", token.file, token.line, token.msg);
    }
    add_trace_token(token);
}

void add_trace_token(trace_token_t token)
{
    trace_token_t *ptr = malloc(sizeof(trace_token_t));
    *ptr = token;
    ptr->next = 0;
    ptr->prev = 0;
    if (!global_trace_manager->tail)
    {
        global_trace_manager->trace_tokens = ptr;
        global_trace_manager->tail = ptr;
        global_trace_manager->num_token = 1;
        return;
    }
    global_trace_manager->tail->next = ptr;
    ptr->prev = global_trace_manager->tail;
    global_trace_manager->tail = ptr;
    global_trace_manager->num_token += 1;
}

void free_kernel_trace()
{
    trace_token_t *ptr = global_trace_manager->trace_tokens;
    while (ptr)
    {
        trace_token_t *old = ptr;
        ptr = ptr->next;
        free(old);
    }
}

void dump_trace()
{
    trace_token_t *ptr = global_trace_manager->trace_tokens;
    while (ptr)
    {
        trace_token_t *old = ptr;
        ptr = ptr->next;
        printf("KERNEL TRACE [  %s : %u  ]: '%s'\n", old->file, old->line, old->msg);
    }
}

void kernel_trace_clear()
{
    free_kernel_trace();
    global_trace_manager->num_token = 0;
    global_trace_manager->tail = 0;
    global_trace_manager->trace_tokens = 0;
}