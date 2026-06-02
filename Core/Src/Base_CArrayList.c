#include "Base_CArrayList.h"

#include "Base_Utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define STD_ARRAYLIST_MIN_EXPAND_SIZE 4

void* CArrayList_Create(size_t elemSize, size_t elemAlignement, size_t size)
{
    assert(elemAlignement != 0 && (elemAlignement & (elemAlignement - 1)) == 0);

    // Alight the header with the array elements alignement. The elemAlignement field should always be
    // a power of 2, at least when using the std_arraylist macro.
    const size_t header_size = sizeof(CArrayList_Header_t)
                             + (elemAlignement - (sizeof(CArrayList_Header_t) & (elemAlignement - 1)));

    char* array = NULL;
    if(elemAlignement > _Alignof(max_align_t))
    {
        array = (char*) aligned_alloc(elemAlignement, header_size + elemSize * size);
    }
    else
    {
        array = (char*) malloc(header_size + elemSize * size);
    }

    if(array)
    {
        CArrayList_Header_t* header = (CArrayList_Header_t*) (array + (header_size - sizeof(CArrayList_Header_t)));

        header->count        = 0;
        header->size         = size;
        header->element_size = elemSize;
        header->header_size  = header_size;

        return (void*) (array + header_size);
    }

    return NULL;
}

void CArrayList_Destroy(void** array)
{
    if(*array == NULL)
    {
        return;
    }

    CArrayList_Header_t* header = CArrayList_GetHeader(*array);
    free((char*) *array - header->header_size);
    *array = NULL;
}

int CArrayList_Reserve(void** array, size_t numElems)
{
    CArrayList_Header_t* header = CArrayList_GetHeader(*array);

    char* new_array = (char*) realloc((char*) *array - header->header_size,
                                      header->header_size + header->element_size * (header->size + numElems));
    if(new_array)
    {
        *array        = (void*) (new_array + header->header_size);
        header        = (CArrayList_Header_t*) (*array - sizeof(CArrayList_Header_t));
        header->size += numElems;
        return 0;
    }
    return -1;
}

int CArrayList_Realloc(void** array, size_t newSize)
{
    CArrayList_Header_t* header = CArrayList_GetHeader(*array);

    char* new_array = (char*) realloc((char*) *array - header->header_size,
                                      header->header_size + header->element_size * newSize);
    if(new_array)
    {
        *array       = (void*) (new_array + header->header_size);
        header       = (CArrayList_Header_t*) (*array - sizeof(CArrayList_Header_t));
        header->size = newSize;
        return 0;
    }
    return -1;
}

static inline int expand_if_needed(void** array, CArrayList_Header_t** header, size_t n)
{
    if((*header)->size < n || (*header)->count >= ((*header)->size - n))
    {
        const size_t min_expand = Utils_Max(STD_ARRAYLIST_MIN_EXPAND_SIZE, n);

        // Expand the array by 12.5% (1/8) when it's full. TODO: find good expansion factor.
        int status = CArrayList_Reserve(array, Utils_Max((*header)->size >> 3, min_expand));
        if(status == 0)
        {
            *header = CArrayList_GetHeader(*array);
        }
        return status;
    }
    return 0;
}

int CArrayList_AppendElement(void** array, const void* element)
{
    CArrayList_Header_t* header = CArrayList_GetHeader(*array);

    if(expand_if_needed(array, &header, 1))
    {
        return -1;
    }

    memcpy((char*) *array + header->count * header->element_size, element, header->element_size);
    header->count++;

    return 0;
}

int CArrayList_AppendRange(void** array, const void* element, size_t numElems)
{
    CArrayList_Header_t* header = CArrayList_GetHeader(*array);

    if(expand_if_needed(array, &header, numElems))
    {
        return -1;
    }

    memcpy((char*) *array + header->count * header->element_size, element, header->element_size * numElems);
    header->count += numElems;

    return 0;
}

int CArrayList_InsertElement(void** array, size_t index, const void* element)
{
    CArrayList_Header_t* header = CArrayList_GetHeader(*array);

    if(index > header->count)
    {
        return -1;
    }

    if(expand_if_needed(array, &header, 1))
    {
        return -1;
    }

    memmove((char*) *array + (index + 1) * header->element_size, (char*) *array + index * header->element_size,
            (header->count - index) * header->element_size);
    memcpy((char*) *array + index * header->element_size, element, header->element_size);
    header->count++;
    return 0;
}

int CArrayList_InsertRange(void** array, size_t index, const void* elements, size_t numElems)
{
    CArrayList_Header_t* header = CArrayList_GetHeader(*array);

    if(index > header->count)
    {
        return -1;
    }

    if(expand_if_needed(array, &header, numElems))
    {
        return -1;
    }

    memmove((char*) *array + (index + numElems) * header->element_size, (char*) *array + index * header->element_size,
            (header->count - index) * header->element_size);
    memcpy((char*) *array + index * header->element_size, elements, header->element_size * numElems);
    header->count += numElems;
    return 0;
}

void CArrayList_PopElement(void* array)
{
    CArrayList_Header_t* header = CArrayList_GetHeader(array);

    if(header->count > 0)
    {
        header->count--;
    }
}

void CArrayList_RemoveElement(void* array, size_t index)
{
    CArrayList_Header_t* header = CArrayList_GetHeader(array);

    if(index >= header->count)
    {
        return;
    }

    memmove((char*) array + index * header->element_size, (char*) array + (index + 1) * header->element_size,
            (header->count - index - 1) * header->element_size);
    header->count--;
}

void CArrayList_ClearList(void* array)
{
    CArrayList_Header_t* header = CArrayList_GetHeader(array);
    header->count               = 0;
}

int CArrayList_TrimList(void** array)
{
    const CArrayList_Header_t* header = CArrayList_GetHeader(*array);
    return CArrayList_Realloc(array, header->count);
}