#include "Base_Allocator.h"

#include "stdalign.h"
#include "stdlib.h"

const Allocator_t DefaultAllocator = {.type_id = ALLOCATOR_TYPE_ID_ANY, .alloc = malloc, .free = free};

const Allocator_t* Alloc_GetDefaultAllocator()
{
    return &DefaultAllocator;
}