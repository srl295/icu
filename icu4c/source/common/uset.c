/*
*******************************************************************************
*
*   Copyright (C) 2002, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  uset.c
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2002mar07
*   created by: Markus W. Scherer
*
*   Poor man's C version of UnicodeSet, with only basic functions.
*   The main data structure, the array of range limits, is
*   the same as in UnicodeSet, except that the HIGH value is not stored.
*
*   There are functions to efficiently serialize a USet into an array of uint16_t
*   and functions to use such a serialized form efficiently without
*   instantiating a new USet.
*
*   If we needed more of UnicodeSet's functionality, then we should
*   move UnicodeSet from the i18n to the common library and
*   use it directly.
*   The only part of this code that would still be useful is the serialization
*   and the functions that use the serialized form directly.
*/

#include "unicode/utypes.h"
#include "cmemory.h"
#include "uset.h"

#define USET_STATIC_CAPACITY 12
#define USET_GROW_DELTA 20

struct USet {
    UChar32 *array;
    int32_t length, capacity;
    UChar32 staticBuffer[USET_STATIC_CAPACITY];
};

U_CAPI USet * U_EXPORT2
uset_open(UChar32 start, UChar32 limit) {
    USet *set;

    set=(USet *)uprv_malloc(sizeof(USet));
    if(set!=NULL) {
        /* initialize to an empty set */
        set->array=set->staticBuffer;
        set->length=0;
        set->capacity=USET_STATIC_CAPACITY;

        /* set initial range */
        if(start<=0) {
            start=0; /* UChar32 may be signed! */
        }
        if(limit>0x110000) {
            limit=0x110000;
        }
        if(start<limit) {
            set->array[0]=start;
            if(limit<0x110000) {
                set->array[1]=limit;
                set->length=2;
            } else {
                set->length=1;
            }
        }
    }
    return set;
}

U_CAPI void U_EXPORT2
uset_close(USet *set) {
    if(set!=NULL) {
        if(set->array!=set->staticBuffer) {
            uprv_free(set->array);
        }
        uprv_free(set);
    }
}

static U_INLINE int32_t
findChar(const UChar32 *array, int32_t length, UChar32 c) {
    int32_t i;

    /* check the last range limit first for more efficient appending */
    if(length>0) {
        if(c>=array[length-1]) {
            return length;
        }

        /* do not check the last range limit again in the loop below */
        --length;
    }

    for(i=0; i<length && c>=array[i]; ++i) {}
    return i;
}

static UBool
addRemove(USet *set, UChar32 c, int32_t doRemove) {
    int32_t i, length, more;

    if(set==NULL || (uint32_t)c>0x10ffff) {
        return FALSE;
    }

    length=set->length;
    i=findChar(set->array, length, c);
    if((i&1)^doRemove) {
        /* c is already in the set */
        return TRUE;
    }

    /* how many more array items do we need? */
    if(i<length && (c+1)==set->array[i]) {
        /* c is just before the following range, extend that in-place by one */
        set->array[i]=c;
        if(i>0) {
            --i;
            if(c==set->array[i]) {
                /* the previous range collapsed, remove it */
                set->length=length-=2;
                if(i<length) {
                    uprv_memmove(set->array+i, set->array+i+2, (length-i)*4);
                }
            }
        }
        return TRUE;
    } else if(i>0 && c==set->array[i-1]) {
        /* c is just after the previous range, extend that in-place by one */
        if(++c<=0x10ffff) {
            set->array[i-1]=c;
            if(i<length && c==set->array[i]) {
                /* the following range collapsed, remove it */
                --i;
                set->length=length-=2;
                if(i<length) {
                    uprv_memmove(set->array+i, set->array+i+2, (length-i)*4);
                }
            }
        } else {
            /* extend the previous range (had limit 0x10ffff) to the end of Unicode */
            set->length=i-1;
        }
        return TRUE;
    } else if(i==length && c==0x10ffff) {
        /* insert one range limit c */
        more=1;
    } else {
        /* insert two range limits c, c+1 */
        more=2;
    }

    /* insert <more> range limits */
    if(length+more>set->capacity) {
        /* reallocate */
        int32_t newCapacity=set->capacity+set->capacity/2+USET_GROW_DELTA;
        UChar32 *newArray=(UChar32 *)uprv_malloc(newCapacity*4);
        if(newArray==NULL) {
            return FALSE;
        }
        set->capacity=newCapacity;
        uprv_memcpy(newArray, set->array, length*4);

        if(set->array!=set->staticBuffer) {
            uprv_free(set->array);
        }
        set->array=newArray;
    }

    if(i<length) {
        uprv_memmove(set->array+i+more, set->array+i, (length-i)*4);
    }
    set->array[i]=c;
    if(more==2) {
        set->array[i+1]=c+1;
    }
    set->length+=more;

    return TRUE;
}

U_CAPI UBool U_EXPORT2
uset_add(USet *set, UChar32 c) {
    return addRemove(set, c, 0);
}

U_CAPI void U_EXPORT2
uset_remove(USet *set, UChar32 c) {
    addRemove(set, c, 1);
}

U_CAPI UBool U_EXPORT2
uset_isEmpty(const USet *set) {
    return set==NULL || set->length<=0;
}

U_CAPI UBool U_EXPORT2
uset_contains(const USet *set, UChar32 c) {
    int32_t i;

    if(set==NULL || (uint32_t)c>0x10ffff) {
        return FALSE;
    }

    i=findChar(set->array, set->length, c);
    return (UBool)(i&1);
}

U_CAPI int32_t U_EXPORT2
uset_countRanges(const USet *set) {
    if(set==NULL) {
        return 0;
    } else {
        return (set->length+1)/2;
    }
}

U_CAPI UBool U_EXPORT2
uset_getRange(const USet *set, int32_t rangeIndex,
              UChar32 *pStart, UChar32 *pLimit) {
    if(set==NULL || rangeIndex<0) {
        return FALSE;
    }

    rangeIndex*=2;
    if(rangeIndex<set->length) {
        *pStart=set->array[rangeIndex++];
        if(rangeIndex<set->length) {
            *pLimit=set->array[rangeIndex];
        } else {
            *pLimit=0x110000;
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * Serialize a USet into 16-bit units.
 * Store BMP code points as themselves with one 16-bit unit each.
 *
 * Important: the code points in the array are in ascending order,
 * therefore all BMP code points precede all supplementary code points.
 *
 * Store each supplementary code point in 2 16-bit units,
 * simply with higher-then-lower 16-bit halfs.
 *
 * Precede the entire list with the length.
 * If there are supplementary code points, then set bit 15 in the length
 * and add the bmpLength between it and the array.
 *
 * In other words:
 * - all BMP:            (length=bmpLength) BMP, .., BMP
 * - some supplementary: (length|0x8000) (bmpLength<length) BMP, .., BMP, supp-high, supp-low, ..
 */
U_CAPI int32_t U_EXPORT2
uset_serialize(const USet *set, uint16_t *dest, int32_t destCapacity, UErrorCode *pErrorCode) {
    int32_t bmpLength, length, destLength;

    if(pErrorCode==NULL || U_FAILURE(*pErrorCode)) {
        return 0;
    }

    if(set==NULL || destCapacity<0 || (destCapacity>0 && dest==NULL)) {
        *pErrorCode=U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }

    /* count necessary 16-bit units */
    length=set->length;
    if(length==0) {
        /* empty set */
        if(destCapacity>0) {
            *dest=0;
        }
        return 1;
    }
    /* now length>0 */

    if(set->array[length-1]<=0xffff) {
        /* all BMP */
        bmpLength=length;
    } else if(set->array[0]>=0x10000) {
        /* all supplementary */
        bmpLength=0;
        length*=2;
    } else {
        /* some BMP, some supplementary */
        for(bmpLength=0; bmpLength<length && set->array[bmpLength]<=0xffff; ++bmpLength) {}
        length=bmpLength+2*(length-bmpLength);
    }

    /* length: number of 16-bit array units */
    if(length>0x7fff) {
        /* there are only 15 bits for the length in the first serialized word */
        *pErrorCode=U_INDEX_OUTOFBOUNDS_ERROR;
        return 0;
    }

    /*
     * total serialized length:
     * number of 16-bit array units (length) +
     * 1 length unit (always) +
     * 1 bmpLength unit (if there are supplementary values)
     */
    destLength=length+1+(length>bmpLength);
    if(destLength<=destCapacity) {
        const UChar32 *p;
        int32_t i;

        *dest=(uint16_t)length;
        if(length>bmpLength) {
            *dest|=0x8000;
            *++dest=(uint16_t)bmpLength;
        }
        ++dest;

        /* write the BMP part of the array */
        p=set->array;
        for(i=0; i<bmpLength; ++i) {
            *dest++=(uint16_t)*p++;
        }

        /* write the supplementary part of the array */
        for(; i<length; i+=2) {
            *dest++=(uint16_t)(*p>>16);
            *dest++=(uint16_t)*p++;
        }
    } else {
        *pErrorCode=U_BUFFER_OVERFLOW_ERROR;
    }
    return destLength;
}

U_CAPI UBool U_EXPORT2
uset_getSerializedSet(USerializedSet *fillSet, const uint16_t *src, int32_t srcCapacity) {
    int32_t length;

    if(fillSet==NULL) {
        return FALSE;
    }
    if(src==NULL || srcCapacity<=0) {
        fillSet->length=fillSet->bmpLength=0;
        return FALSE;
    }

    length=*src++;
    if(length&0x8000) {
        /* there are supplementary values */
        length&=0x7fff;
        if(srcCapacity<(2+length)) {
            fillSet->length=fillSet->bmpLength=0;
            return FALSE;
        }
        fillSet->bmpLength=*src++;
    } else {
        /* only BMP values */
        if(srcCapacity<(1+length)) {
            fillSet->length=fillSet->bmpLength=0;
            return FALSE;
        }
        fillSet->bmpLength=length;
    }
    fillSet->array=src;
    fillSet->length=length;
    return TRUE;
}

U_CAPI UBool U_EXPORT2
uset_serializedContains(const USerializedSet *set, UChar32 c) {
    const uint16_t *array;

    if(set==NULL || (uint32_t)c>0x10ffff) {
        return FALSE;
    }

    array=set->array;
    if(c<=0xffff) {
        /* find c in the BMP part */
        int32_t i, bmpLength=set->bmpLength;
        for(i=0; i<bmpLength && (uint16_t)c>=array[i]; ++i) {}
        return (UBool)(i&1);
    } else {
        /* find c in the supplementary part */
        int32_t i, length=set->length;
        uint16_t high=(uint16_t)(c>>16), low=(uint16_t)c;
        for(i=set->bmpLength;
            i<length && (high>array[i] || (high==array[i] && low>=array[i+1]));
            i+=2) {}

        /* count pairs of 16-bit units even per BMP and check if the number of pairs is odd */
        return (UBool)(((i+set->bmpLength)&2)!=0);
    }
}

U_CAPI int32_t U_EXPORT2
uset_countSerializedRanges(const USerializedSet *set) {
    if(set==NULL) {
        return 0;
    }

    return (set->bmpLength+(set->length-set->bmpLength)/2+1)/2;
}

U_CAPI UBool U_EXPORT2
uset_getSerializedRange(const USerializedSet *set, int32_t rangeIndex,
                        UChar32 *pStart, UChar32 *pLimit) {
    const uint16_t *array;
    int32_t bmpLength, length;

    if(set==NULL || rangeIndex<0 || pStart==NULL || pLimit==NULL) {
        return FALSE;
    }

    array=set->array;
    length=set->length;
    bmpLength=set->bmpLength;

    rangeIndex*=2; /* address start/limit pairs */
    if(rangeIndex<bmpLength) {
        *pStart=array[rangeIndex++];
        if(rangeIndex<bmpLength) {
            *pLimit=array[rangeIndex];
        } else if(rangeIndex<length) {
            *pLimit=(((int32_t)array[rangeIndex])<<16)|array[rangeIndex+1];
        } else {
            *pLimit=0x110000;
        }
        return TRUE;
    } else {
        rangeIndex-=bmpLength;
        rangeIndex*=2; /* address pairs of pairs of units */
        length-=bmpLength;
        if(rangeIndex<length) {
            array+=bmpLength;
            *pStart=(((int32_t)array[rangeIndex])<<16)|array[rangeIndex+1];
            rangeIndex+=2;
            if(rangeIndex<length) {
                *pLimit=(((int32_t)array[rangeIndex])<<16)|array[rangeIndex+1];
            } else {
                *pLimit=0x110000;
            }
            return TRUE;
        } else {
            return FALSE;
        }
    }
}
