#include "string.h"

void uitoa(uint64_t num, char *buffer)
{
    int i = 0;
    if (num == 0)
    {
        buffer[0] = '0';
        buffer[1] = 0;
        return;
    }
    while (num > 0)
    {
        int rem = num % 10;
        num /= 10;
        buffer[i] = rem + '0';
        ++i;
    }
    buffer[i] = 0;
    reverse_str(buffer);
}

void itoa(int64_t num, char *buffer)
{

    int i = 0;
    if (num == 0)
    {
        buffer[0] = '0';
        buffer[1] = 0;
        return;
    }
    if (num < 0)
    {
        num *= -1;
        buffer[0] = '-';
        ++i;
    }
    while (num > 0)
    {
        int rem = num % 10;
        num /= 10;
        buffer[i] = rem + '0';
        ++i;
    }
    buffer[i] = 0;
    reverse_str(buffer);
}

void reverse_str(char *str)
{
    if (!str)
        return;

    size_t start = 0;
    size_t end = 0;

    while (str[end] != '\0')
    {
        end++;
    }

    if (end == 0)
        return;

    end--;

    while (start < end)
    {
        char tmp = str[start];
        str[start] = str[end];
        str[end] = tmp;
        start++;
        end--;
    }
}

size_t strlen(char *str)
{
    size_t s = 0;
    while (*str)
    {
        ++s;
        ++str;
    }
    return s;
}

void strcpy(char *dest, char *src)
{
    size_t i = 0;
    while (*src)
    {
        dest[i] = *src;
        ++src;
        ++i;
    }
    dest[i] = 0;
}

void ftoa(float num, char *buffer, int places)
{
    char integer_component[33];
    itoa((int)num, integer_component);
    int len = strlen(integer_component);
    if (num < 0)
    {
        num *= -1;
    }
    float decimal = num;
    decimal -= ((int)num);
    strcpy(buffer, integer_component);
    buffer[len] = '.';
    int idx = len + 1;
    for (int i = 0; i < places; ++i)
    {
        decimal *= 10;
    }
    int idecimal = (int)decimal;
    char decimal_component[33];
    itoa(idecimal, decimal_component);
    strcpy(buffer + idx, decimal_component);
}

void itoa_hex(uint64_t num, char *buffer)
{
    uint64_t *valptr = &num;
    uint8_t *ptr;
    uint8_t temp;
    uint8_t size = 8 * 2 - 1;
    for (uint8_t i = 0; i < size; i++)
    {
        ptr = ((uint8_t *)valptr + i);
        temp = ((*ptr & 0xF0) >> 4);
        buffer[size - (i * 2 + 1)] = temp > 9 ? 'A' + (temp - 10) : '0' + temp;
        temp = ((*ptr & 0x0F));
        buffer[size - (i * 2)] = temp > 9 ? 'A' + (temp - 10) : '0' + temp;
    }
    buffer[size + 1] = 0;
}

void memset(void *start, uint8_t value, uint64_t num)
{
    for (uint64_t i = 0; i < num; i++)
    {
        *(uint8_t *)((uint64_t)start + i) = value;
    }
}
int vsprintf(char *out, char *fmt, va_list args)
{
    char *out_ptr = out;
    for (char *p = fmt; *p; ++p)
    {
        if (*p != '%')
        {
            *out_ptr = *p;
            ++out_ptr;
        }
        else
        {
            ++p;
            switch (*p)
            {
            case 'd':
            {
                int value = va_arg(args, int);
                char buf[32];
                itoa(value, buf);
                for (char *b = buf; *b; b++)
                {
                    *out_ptr = *b;
                    ++out_ptr;
                    ++b;
                }
            }
            break;
            case 's':
            {
                char *str = va_arg(args, char *);
                while (*str)
                {
                    *out_ptr = *str;
                    ++out_ptr;
                    ++str;
                }
            }
            break;
            case 'c':
            {
                char ch = (char)va_arg(args, int);
                *out_ptr = ch;
                ++out_ptr;
            }
            break;
            }
        }
    }
    *out_ptr = 0;
    return (int)(out_ptr - out);
}

int sprintf(char *out, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int written = vsprintf(out, fmt, args);
    va_end(args);
    return written;
}
