/* Stateless memory helpers for the resident bootstrap. No CRT or allocator. */
#include <stddef.h>
#include <string.h>
#pragma function(memcpy, memset)
void *memcpy(void *destination, const void *source, size_t count)
{
    volatile unsigned char *out = (volatile unsigned char *)destination;
    const unsigned char *in = (const unsigned char *)source;
    while (count--)
        *out++ = *in++;
    return destination;
}
void *memset(void *destination, int value, size_t count)
{
    volatile unsigned char *out = (volatile unsigned char *)destination;
    while (count--)
        *out++ = (unsigned char)value;
    return destination;
}
