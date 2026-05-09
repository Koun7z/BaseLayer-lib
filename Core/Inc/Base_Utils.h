#ifndef STD_UTILS_H
#define STD_UTILS_H

#include <stdint.h>
#include <stddef.h>

#include <assert.h>

#define Utils_Max(a, b) ((a) > (b) ? (a) : (b))
#define Utils_Min(a, b) ((a) < (b) ? (a) : (b))

static inline size_t Utils_NextPow2(size_t n)
{
    unsigned int p = 1;
    while(p < n)
    {
        p <<= 1;
    }
    return p;
}


static inline size_t Utils_AlignUp(size_t offset, size_t alignment)
{
    assert(alignment != 0 && (alignment & (alignment - 1)) == 0);  // alignment must be a power of 2

    return (offset + alignment - 1) & ~(alignment - 1);
}


#endif  // STD_UTILS_H