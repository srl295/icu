/*
* Copyright � {1997-1999}, International Business Machines Corporation and others. All Rights Reserved.
*****************************************************************************************
*/
//===============================================================================
//
// File tcoldata.h
//
// Internal file.  Defines TableCollationData, an internal class which is shared
// by TableCollation objects, and which contains all the invariant (and large)
// pieces of data.  Once created, TableCollationData objects do not change.
//
// Created by: Alan Liu
//
// Modification History:
//
//  Date        Name        Description
//  2/5/97      aliu        Creation.
//  3/5/97      aliu        Don't stream rule table in or out.  Added data members
//                          isRuleTableLoaded, desiredLocale, and realLocaleName,
//                          which are required for dynamic reloading of the rule
//                          string.  We still keep rules in this object so that
//                          they can be held in the in-memory cache.
//  3/26/97     helena      Updated with platform independent data types.
//  6/20/97     helena      Java class name change.
//  8/18/97     helena      Added internal API documentation.
//===============================================================================

#ifndef TCOLDATA_H
#define TCOLDATA_H

#include "ucolimp.h"

#include "ucmp32.h"
#include "unicode/utypes.h"
#include "colcache.h"
#include "unicode/unistr.h"
#include "unicode/locid.h"
#include "filestrm.h"
#include "umemstrm.h"

//=============================================================================

class VectorOfPToContractTable;
class VectorOfPToExpandTable;
class RuleBasedCollator;
class CollationElementIterator;

// This is just temporary, for prototyping.
typedef void *  UCollator;

//=============================================================================

/**
 * TableCollationData is an internal class used by TableCollation.  It
 * encapsulates most of the data associated with a TableCollation.
 * This includes the large collation tables, including the contract
 * order and expand order tables, and some small pieces of data that
 * don't change, such as the maximum secondary order.  The general
 * idea is that the TableCollation object should be a lightweight
 * object.  If there are two TableCollation objects, they will each be
 * very small, and users can alter the strength of each object
 * independently.  However, both objects, if they are refering to the
 * same underlying collation, will share pointers to the same
 * TableCollationData object, which doesn't change.  <P>
 * TableCollationData objects are therefore good candidates for
 * caching in memory and potentially for reference counting.  */
class TableCollationData
{
public:
                                TableCollationData();
    virtual                     ~TableCollationData();

    /**
     * Cache interface.  The cache uses UnicodeString objects as keys.  These
     * are completely arbitrary, but are usually something uniquely associated
     * with each collation, while at the same time fairly small, such as a
     * locale identifier string.
     * <P> Adds the collation data object to the cache list.
     * @param key the unique key of the associated collation data object.
     * @param data the collation data object.
     */
    static  void                addToCache(const UnicodeString& key, TableCollationData* data);
    /**
     * Finds and returns the cached collation data.
     * @param key the unique key of the associated collation data object.
     * @returns the found collation data object.
     */
    static  TableCollationData* findInCache(const UnicodeString& key);

    /**
     * The streamIn and streamOut methods read and write objects of this
     * class as binary, platform-dependent data in the iostream.  The stream
     * must be in ios::binary mode for this to work.  These methods are not
     * intended for general public use; they are used by the framework to improve
     * performance by storing certain objects in binary files.
     */
            void                streamIn(FileStream* is);
            void                streamOut(FileStream* os) const;

    /**
     * For internal use only - streaming to memory
     */
            void                streamIn(UMemoryStream* is, UErrorCode& status);
            void                streamOut(UMemoryStream* os) const;

        /**
         * Checks if this object is valid.
         * @return TRUE if the object is valid, FALSE otherwise.
         */
            UBool              isBogus() const;
private:
    /**
     * The following are disallowed operations: not implemented.
     */
    TableCollationData(const TableCollationData&);
    TableCollationData& operator=(const TableCollationData&);

private:
    // Do not access the fgCache object directly; use addToCache and findInCache.
    static  CollationCache      fgCache;

private:
    /**
     * The TableCollation class freely manipulates the data members within a
     * TableCollationData object.  This is because TableCollationData is
     * intended to be an internal, invisible implementation detail.  If
     * TableCollationData every becomes a more public API, then this will have
     * to change, although this is not really advised.
     */
    friend class RuleBasedCollator;
    friend class CollationElementIterator;

// This is just temporary, for prototyping.
    friend int32_t ucol_getNextCE(const UCollator *coll, collIterate *source, UErrorCode *status);
    friend int32_t ucol_getIncrementalCE(const UCollator *coll, incrementalContext *source, UErrorCode *status); 
    friend int32_t getComplicatedCE(const UCollator *coll, collIterate *source, UErrorCode *status);
/*
  friend int32_t ucol_getSortKey(const    UCollator    *coll, const    UChar        *source,
        int32_t        sourceLength, uint8_t        *result, int32_t        resultLength);
*/
  friend int32_t ucol_calcSortKey(const    UCollator    *coll,
        const    UChar        *source,
        int32_t        sourceLength,
        uint8_t        **result,
        int32_t        resultLength,
        UBool allocatePrimary);
  friend UCollationResult ucol_strcoll(    const    UCollator    *coll,
        const    UChar        *source,
        int32_t            sourceLength,
        const    UChar        *target,
        int32_t            targetLength);
/*
  friend UCollationResult ucol_strcollEx(    const    UCollator    *coll,
        const    UChar        *source,
        int32_t            sourceLength,
        const    UChar        *target,
        int32_t            targetLength);
*/
  friend int32_t ucol_getSortKeySize(const UCollator *coll, 
        collIterate *s, 
        int32_t currentSize, 
        UColAttributeValue strength, 
        int32_t len);


    UBool                      isFrenchSec;
    int16_t                     maxSecOrder;
    int16_t                     maxTerOrder;
    CompactIntArray*            mapping;
    VectorOfPToContractTable*   contractTable;
    VectorOfPToExpandTable*     expandTable;
    UBool                      fBogus;

    /**
     * Rule string data is generated dynamically when required by the TableCollation
     * object.  In particular, when a binary file is created, the rule data is
     * not streamed out -- this keeps the binary file small.  However, two pieces
     * of data are kept around (the desiredLocale and the realLocaleName) which
     * allow later loading of the rule string efficiently.  By efficiently, we mean
     * that the usual search procedure is shortened, and the final file which
     * actually led to the successful loading of the collation elements is accessed
     * directly.  The boolean isRuleTableLoaded allows the collation object to
     * know whether rules have been loaded (an alternative is to use an empty
     * ruleTable to indicate this, but this then disallows the empty string as
     * a valid rule string).  Note that when a collation is constructed "from
     * scratch" (not loaded from a binary file), the rule table is already in place,
     * and isRuleTableLoaded is set to true.
     *
     * See RuleBasedCollator::getRules().
     */
    UnicodeString               ruleTable;
    UBool                      isRuleTableLoaded;
    Locale                      desiredLocale;
    UnicodeString               realLocaleName;
};

#endif //_TCOLDATA
//eof
