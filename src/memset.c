/**
 * @file memset.c
 * @brief Minimal memset implementation for embedded systems
 */

#include <stddef.h>

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

