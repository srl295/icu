/*
******************************************************************************
* Copyright (C) 1999-2001, International Business Machines Corporation and   *
* others. All Rights Reserved.                                               *
******************************************************************************
*   Date        Name        Description
*   10/22/99    alan        Creation.
**********************************************************************
*/

#include "uvector.h"
#include "cmemory.h"

U_NAMESPACE_BEGIN

#define DEFUALT_CAPACITY 8

UVector::UVector(UErrorCode &status) :
    count(0),
    capacity(0),
    elements(0),
    deleter(0),
    comparer(0)
{
    _init(DEFUALT_CAPACITY, status);
}

UVector::UVector(int32_t initialCapacity, UErrorCode &status) :
    count(0),
    capacity(0),
    elements(0),
    deleter(0),
    comparer(0)
{
    // Fix bogus initialCapacity values; avoid malloc(0)
    if (initialCapacity < 1) {
        initialCapacity = 1;
    }
    _init(initialCapacity, status);
}

UVector::UVector(UObjectDeleter d, UKeyComparator c, UErrorCode &status) :
    count(0),
    capacity(0),
    elements(0),
    deleter(d),
    comparer(c)
{
    _init(DEFUALT_CAPACITY, status);
}

UVector::UVector(UObjectDeleter d, UKeyComparator c, int32_t initialCapacity, UErrorCode &status) :
    count(0),
    capacity(0),
    elements(0),
    deleter(d),
    comparer(c)
{
    _init(initialCapacity, status);
}

void UVector::_init(int32_t initialCapacity, UErrorCode &status) {
    elements = (UHashTok *)uprv_malloc(sizeof(UHashTok)*initialCapacity);
    if (elements == 0) {
        status = U_MEMORY_ALLOCATION_ERROR;
    } else {
        capacity = initialCapacity;
    }
}

UVector::~UVector() {
    removeAllElements();
    uprv_free(elements);
    elements = 0;
}

void UVector::addElement(void* obj, UErrorCode &status) {
    if (ensureCapacity(count + 1, status)) {
        elements[count++].pointer = obj;
    }
}

void UVector::addElement(int32_t elem, UErrorCode &status) {
    if (ensureCapacity(count + 1, status)) {
        elements[count++].integer = elem;
    }
}

void UVector::setElementAt(void* obj, int32_t index) {
    if (0 <= index && index < count) {
        if (elements[index].pointer != 0 && deleter != 0) {
            (*deleter)(elements[index].pointer);
        }
        elements[index].pointer = obj;
    }
    /* else index out of range */
}

void UVector::setElementAt(int32_t elem, int32_t index) {
    if (0 <= index && index < count) {
        if (elements[index].pointer != 0 && deleter != 0) {
            (*deleter)(elements[index].pointer);
        }
        elements[index].integer = elem;
    }
    /* else index out of range */
}

void UVector::insertElementAt(void* obj, int32_t index, UErrorCode &status) {
    // must have 0 <= index <= count
    if (0 <= index && index <= count && ensureCapacity(count + 1, status)) {
        for (int32_t i=count; i>index; --i) {
            elements[i] = elements[i-1];
        }
        elements[index].pointer = obj;
        ++count;
    }
    /* else index out of range */
}

void* UVector::elementAt(int32_t index) const {
    return (0 <= index && index < count) ? elements[index].pointer : 0;
}

int32_t UVector::elementAti(int32_t index) const {
    return (0 <= index && index < count) ? elements[index].integer : 0;
}

void UVector::removeElementAt(int32_t index) {
    void* e = orphanElementAt(index);
    if (e != 0 && deleter != 0) {
        (*deleter)(e);
    }
}

UBool UVector::removeElement(void* obj) {
    int32_t i = indexOf(obj);
    if (i >= 0) {
        removeElementAt(i);
        return TRUE;
    }
    return FALSE;
}

void UVector::removeAllElements(void) {
    if (deleter != 0) {
        for (int32_t i=0; i<count; ++i) {
            if (elements[i].pointer != 0) {
                (*deleter)(elements[i].pointer);
            }
        }
    }
    count = 0;
}

int32_t UVector::indexOf(void* obj, int32_t startIndex) const {
    if (comparer != 0) {
        UHashTok key;
        key.pointer = obj;
        for (int32_t i=startIndex; i<count; ++i) {
            if ((*comparer)(key, elements[i])) {
                return i;
            }
        }
    }
    return -1;
}

UBool UVector::ensureCapacity(int32_t minimumCapacity, UErrorCode &status) {
    if (capacity >= minimumCapacity) {
        return TRUE;
    } else {
        int32_t newCap = capacity * 2;
        if (newCap < minimumCapacity) {
            newCap = minimumCapacity;
        }
        UHashTok* newElems = (UHashTok *)uprv_malloc(sizeof(UHashTok)*newCap);
        if (newElems == 0) {
            status = U_MEMORY_ALLOCATION_ERROR;
            return FALSE;
        }
        uprv_memcpy(newElems, elements, sizeof(elements[0]) * count);
        uprv_free(elements);
        elements = newElems;
        capacity = newCap;
        return TRUE;
    }
}

/**
 * Change the size of this vector as follows: If newSize is smaller,
 * then truncate the array, possibly deleting held elements for i >=
 * newSize.  If newSize is larger, grow the array, filling in new
 * slots with NULL.
 */
void UVector::setSize(int32_t newSize) {
    int32_t i;
    if (newSize < 0) {
        return;
    }
    if (newSize > count) {
        UErrorCode ec = U_ZERO_ERROR;
        if (!ensureCapacity(newSize, ec)) {
            return;
        }
        UHashTok empty;
        empty.pointer = NULL;
        empty.integer = 0;
        for (i=count; i<newSize; ++i) {
            elements[i] = empty;
        }
    } else {
        /* Most efficient to count down */
        for (i=count-1; i>=newSize; --i) {
            removeElementAt(i);
        }
    }
    count = newSize;
}

/**
 * Fill in the given array with all elements of this vector.
 */
void** UVector::toArray(void** result) const {
    void** a = result;
    for (int i=0; i<count; ++i) {
        *a++ = elements[i].pointer;
    }
    return result;
}

UObjectDeleter UVector::setDeleter(UObjectDeleter d) {
    UObjectDeleter old = deleter;
    deleter = d;
    return old;
}

UKeyComparator UVector::setComparer(UKeyComparator d) {
    UKeyComparator old = comparer;
    comparer = d;
    return old;
}

/**
 * Removes the element at the given index from this vector and
 * transfer ownership of it to the caller.  After this call, the
 * caller owns the result and must delete it and the vector entry
 * at 'index' is removed, shifting all subsequent entries back by
 * one index and shortening the size of the vector by one.  If the
 * index is out of range or if there is no item at the given index
 * then 0 is returned and the vector is unchanged.
 */
void* UVector::orphanElementAt(int32_t index) {
    void* e = 0;
    if (0 <= index && index < count) {
        e = elements[index].pointer;
        for (int32_t i=index; i<count-1; ++i) {
            elements[i] = elements[i+1];
        }
        --count;
    }
    /* else index out of range */
    return e;
}

UStack::UStack(UErrorCode &status) :
    UVector(status)
{
}

UStack::UStack(int32_t initialCapacity, UErrorCode &status) :
    UVector(initialCapacity, status)
{
}

UStack::UStack(UObjectDeleter d, UKeyComparator c, UErrorCode &status) :
    UVector(d, c, status)
{
}

UStack::UStack(UObjectDeleter d, UKeyComparator c, int32_t initialCapacity, UErrorCode &status) :
    UVector(d, c, initialCapacity, status)
{
}

void* UStack::pop(void) {
    int32_t n = size() - 1;
    void* result = 0;
    if (n >= 0) {
        result = elementAt(n);
        removeElementAt(n);
    }
    return result;
}

int32_t UStack::popi(void) {
    int32_t n = size() - 1;
    int32_t result = 0;
    if (n >= 0) {
        result = elementAti(n);
        removeElementAt(n);
    }
    return result;
}

int32_t UStack::search(void* obj) const {
    int32_t i = indexOf(obj);
    return (i >= 0) ? size() - i : i;
}

U_NAMESPACE_END

