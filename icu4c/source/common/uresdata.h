/*
******************************************************************************
*                                                                            *
* Copyright (C) 1999-2001, International Business Machines                   *
*                Corporation and others. All Rights Reserved.                *
*                                                                            *
******************************************************************************
*   file name:  uresdata.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 1999dec08
*   created by: Markus W. Scherer
*/

#ifndef __RESDATA_H__
#define __RESDATA_H__

#include "unicode/utypes.h"
#include "unicode/udata.h"

/*
 * A Resource is a 32-bit value that has 2 bit fields:
 * 31..28   4-bit type, see enum below
 * 27..0    28-bit four-byte-offset or value according to the type
 */
typedef uint32_t Resource;

#define RES_BOGUS 0xffffffff

#define RES_GET_TYPE(res) ((res)>>28UL)
#define RES_GET_OFFSET(res) ((res)&0x0fffffff)
#define RES_GET_POINTER(pRoot, res) ((pRoot)+RES_GET_OFFSET(res))

/* get signed and unsigned integer values directly from the Resource handle */
#define RES_GET_INT(res) (((int32_t)((res)<<4L))>>4L)
#define RES_GET_UINT(res) ((res)&0x0fffffff)

/*
 * Resource types:
 * Most resources have their values stored at four-byte offsets from the start
 * of the resource data. These values are at least 4-aligned.
 * Some resource values are stored directly in the offset field of the Resource itself.
 *
 * Type Name            Memory layout of values
 *                      (in parentheses: scalar, non-offset values)
 *
 * 0  Unicode String:   int32_t length, UChar[length], (UChar)0, (padding)
 *                  or  (empty string ("") if offset==0)
 * 1  Binary:           int32_t length, uint8_t[length], (padding)
 *                      - this value should be 32-aligned -
 * 2  Table:            uint16_t count, uint16_t keyStringOffsets[count], (uint16_t padding), Resource[count]
 *
 * 7  Integer:          (28-bit offset is integer value)
 * 8  Array:            int32_t count, Resource[count]
 *
 * 14 Integer Vector:   int32_t length, int32_t[length]
 * 15 Reserved:         This value denotes special purpose resources and is for internal use.
 
 */

/*
 * Structure for a single, memory-mapped ResourceBundle.
 */
typedef struct {
    UDataMemory *data;
    Resource *pRoot;
    Resource rootRes;
} ResourceData;

/*
 * Load a resource bundle file.
 * The ResourceData structure must be allocated externally.
 */
U_CFUNC UBool
res_load(ResourceData *pResData,
         const char *path, const char *name, UErrorCode *errorCode);

/*
 * Release a resource bundle file.
 * This does not release the ResourceData structure itself.
 */
U_CFUNC void
res_unload(ResourceData *pResData);

/*
 * Return a pointer to a zero-terminated, const UChar* string
 * and set its length in *pLength.
 * Returns NULL if not found.
 */
U_CFUNC const UChar *
res_getString(const ResourceData *pResData, const Resource res, int32_t *pLength);

U_CFUNC const uint8_t *
res_getBinary(const ResourceData *pResData, const Resource res, int32_t *pLength);

U_CFUNC Resource
res_getResource(const ResourceData *pResData, const char *key);

U_CFUNC int32_t
res_countArrayItems(const ResourceData *pResData, const Resource res);

U_CFUNC int32_t res_getTableSize(const ResourceData *pResData, Resource table);

U_CFUNC Resource res_getArrayItem(const ResourceData *pResData, const Resource array, const int32_t indexS);
U_CFUNC Resource res_getTableItemByIndex(const ResourceData *pResData, const Resource table, int32_t indexS, const char ** key);
U_CFUNC Resource res_getTableItemByKey(const ResourceData *pResData, const Resource table, int32_t *indexS, const char* * key);

#endif
