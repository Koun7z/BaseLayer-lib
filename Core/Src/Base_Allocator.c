#include "Base_Allocator.h"

#include "stdalign.h"
#include "stdlib.h"

const Allocator_t DefaultAllocator = {
    .alloc   = &malloc,
    .realloc = &realloc,
    .free    = &free,
};

const KV_Allocator_t DefaultKVAllocator = {
    .key_alignement   = alignof(max_align_t),
    .value_alignement = alignof(max_align_t),
    .alloc            = &malloc,
    .realloc          = &realloc,
    .free             = &free,
};


const Allocator_t* Alloc_GetDefaultAllocator()
{
    return &DefaultAllocator;
}

const KV_Allocator_t* Alloc_GetDefaultKVAllocator()
{
    return &DefaultKVAllocator;
}