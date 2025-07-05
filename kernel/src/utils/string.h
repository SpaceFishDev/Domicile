#include "typdef.h"
#include <stdint.h>
#include <stdarg.h>

void itoa(int64_t num, char *buffer);
void uitoa(uint64_t num, char *buffer);
void ftoa(double num, char *buffer, int places);
void strcpy(char *dest, char *src);
void reverse_str(char *str);
size_t strlen(char *str);
void itoa_hex(uint64_t num, char *buffer);
void memset(void *start, uint8_t value, uint64_t num);
int sprintf(char *str, char *fmt, ...);
int vsprintf(char *out, char *fmt, va_list args);