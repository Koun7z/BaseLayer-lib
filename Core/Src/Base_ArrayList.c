#include "Base_ArrayList.h"

#include "Base_Utils.h"

#include <stdlib.h>
#include <assert.h>

static inline void* alloc_aligned(size_t size, size_t alignment)
{
    if(alignment <= alignof(max_align_t))
    {
        return malloc(size);
    }

    return aligned_alloc(alignment, Utils_AlignUp(size, alignment));
}

ArrayList_t* ArrayList_Create(size_t elementSize, size_t element_alignment, size_t initialCapacity)
{
    assert((element_alignment & (element_alignment - 1)) == 0);

    ArrayList_t* list = (ArrayList_t*) malloc(sizeof(ArrayList_t));
    if(!list)
    {
        return NULL;
    }

    list->count              = 0;
    list->capacity           = initialCapacity;
    list->_element_size      = elementSize;
    list->_element_alignment = element_alignment;
    list->_data              = alloc_aligned(elementSize * initialCapacity, element_alignment);

    if(list->_data == NULL)
    {
        free(list);
        return NULL;
    }

    return list;
}