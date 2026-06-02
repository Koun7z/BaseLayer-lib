#ifndef CArrayList_H__
#define CArrayList_H__

#include <stddef.h>

/**
 * @brief Implementation of a dynamic array list in C.
 *        The whole idea of this implementation is to have a dynamic array for any type while still allowing the user
 *        to access the array elements directly using the "[]" operator.*
 *
 * @note  To have access to the array from multiple places you need to store it an "type**", because the "type*" pointer
 *        will change during list resizing.
 *
 *        I'm quite conflicted on this design, it seems nice at first but gets more complicated the more you use it. On
 *        the other hand it retains at least some of the type safety of having normal C array, compared to having to cast
 *        the void* on access.
 *
 * @details The std_CArrayList macro creates and array of requested size and automatically places
 *          a header before the first element of this array, then "returns" a properly typed
 *          pointer to the first element of the array. The header can always be found at - sizeof(CArrayList_Header_t)
 *          offset from this pointer. Padding may be inserted if the header size is not a multiple of the array element alignement.
 *
 *          |---------|--------|---------|---------|-----|------------- |
 *          | padding | header | elem[0] | elem[1] | ... | elem[size-1] |
 *          |---------|--------|---------|---------|-----|--------------|
 *                             ^
 *                             | pointer returned by CArrayList macro
 */

typedef struct
{
    size_t count;
    size_t size;
    size_t element_size;
    size_t header_size;  // including dynamic padding
    char data[];
} CArrayList_Header_t;

/**
 * @brief Allocate an array of a given type and size, and return a typed pointer to the first element of this array.
 *        Following functions will dynamicly modify this array as needed.
 *        Elements of the array can be accessed directly using the "[]" operator, but you should never modify the
 *        array pointer itself directly.
 *        To change the underlying memory allocation, use the CArrayList_* functions.
 *
 * @param _T   : Any type  - Type of the array elements.
 * @param _size: size_t    - Initial size of the array.
 * @return       *_T       - Pointer to the first element of the array.
 *
 */
#define CArrayList(_T, _size) ((_T*) CArrayList_Create(sizeof(_T), _Alignof(_T), _size))

/**
 * @brief Free the memory previously allocated by CArrayList macro.
 *        _array pointer will be set to NULL after this call.
 *
 * @param _array: any** - Pointer to the array pointer returned by CArrayList macro.
 */
#define CArrayList_Free(_array) CArrayList_Destroy((void**) _array)

#define CArrayList_Append(_array, _element) CArrayList_AppendElement((void**) _array, &_element)
#define CArrayList_AppendMany(_array, _elements, _numElems)       \
    CArrayList_AppendRange((void**) _array, _elements, _numElems)

#define CArrayList_Insert(_array, _index, _element) CArrayList_InsertElement((void**) _array, _index, &_element)
#define CArrayList_InsertMany(_array, _index, _elements, _numElems)       \
    CArrayList_InsertRange((void**) _array, _index, _elements, _numElems)

#define CArrayList_Pop(_array) (CArrayList_get_header(_array)->count--)

#define CArrayList_Remove(_array, _index) CArrayList_RemoveElement(_array, _index, NULL)

#define CArrayList_Clear(_array) CArrayList_ClearList(_array)

#define CArrayList_Trim(_array) CArrayList_TrimList((void**) _array)

static inline CArrayList_Header_t* CArrayList_GetHeader(const void* array)
{
    return (CArrayList_Header_t*) ((char*) array - sizeof(CArrayList_Header_t));
}

static inline size_t CArrayList_Count(const void* array)
{
    return CArrayList_GetHeader(array)->count;
}

static inline size_t CArrayList_Size(const void* array)
{
    return CArrayList_GetHeader(array)->size;
}

static inline size_t CArrayList_ElementSize(const void* array)
{
    return CArrayList_GetHeader(array)->element_size;
}

void* CArrayList_Create(size_t elemSize, size_t elemAlignement, size_t size);

void CArrayList_Destroy(void** array);

void* CArrayList_Copy(const void* array);

int CArrayList_Reserve(void** array, size_t numElems);

int CArrayList_Realloc(void** array, size_t newSize);

int CArrayList_AppendElement(void** array, const void* element);

int CArrayList_AppendRange(void** array, const void* elements, size_t numElems);

void CArrayList_PopElement(void* array);

int CArrayList_InsertElement(void** array, size_t index, const void* element);

int CArrayList_InsertRange(void** array, size_t index, const void* elements, size_t numElems);

void CArrayList_RemoveElement(void* array, size_t index);

void CArrayList_RemoveRange(void* array, size_t index, size_t numElems);

void CArrayList_ClearList(void* array);

int CArrayList_TrimList(void** array);

#endif /* CArrayList_H__ */