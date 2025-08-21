#ifndef MEMDEBUG_H
#define MEMDEBUG_H
#define free(ptr) \
    memdebug_free(ptr, __FILE__, __LINE__)
#define malloc(sz) \
    memdebug_malloc(sz, __FILE__, __LINE__)
#define realloc(ptr, sz) \
    memdebug_realloc(ptr, sz, __FILE__, __LINE__)

extern void memdebug_free(void *, char *, int);
extern void *memdebug_malloc(int, char *, int);
extern void *memdebug_realloc(void *, int, char *, int);
extern void init_memdebug();
extern void cleanup_memdebug();
extern void memdebug_view();

#endif