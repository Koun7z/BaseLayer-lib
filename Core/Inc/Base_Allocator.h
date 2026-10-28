#ifndef BASE_ALLOCATOR_H__
#define BASE_ALLOCATOR_H__

#include "stddef.h"

typedef struct
{
    void* (*const alloc)(size_t size);
    void* (*const realloc)(void* ptr, size_t new_size);
    void (*const free)(void* ptr);
} Allocator_t;

typedef struct
{
    const size_t key_alignement;
    const size_t value_alignement;
    void* (*const alloc)(
      size_t size);  // The larger alignement of the two should be hardcoded in the alloc implementation
    void* (*const realloc)(void* ptr, size_t new_size);
    void (*const free)(void* ptr);
} KV_Allocator_t;

const Allocator_t* Alloc_GetDefaultAllocator();

const KV_Allocator_t* Alloc_GetDefaultKVAllocator();

#endif /* BASE_ALLOCATOR_H__ */