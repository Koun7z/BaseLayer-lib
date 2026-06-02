#ifndef BASE_ALLOCATOR_H__
#define BASE_ALLOCATOR_H__

#include "stddef.h"

#define ALLOCATOR_TYPE_ID_ANY 0

typedef struct
{
    int type_id;
    void* (*alloc)(size_t size);
    void (*free)(void* ptr);
} Allocator_t;

const Allocator_t* Alloc_GetDefaultAllocator();

#endif /* BASE_ALLOCATOR_H__ */