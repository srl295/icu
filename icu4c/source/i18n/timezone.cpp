/*
*******************************************************************************
* Copyright (C) 1997-2001, International Business Machines Corporation and    *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
* File TIMEZONE.CPP
*
* Modification History:
*
*   Date        Name        Description
*   12/05/96    clhuang     Creation.
*   04/21/97    aliu        General clean-up and bug fixing.
*   05/08/97    aliu        Fixed Hashtable code per code review.
*   07/09/97    helena      Changed createInstance to createDefault.
*   07/29/97    aliu        Updated with all-new list of 96 UNIX-derived
*                           TimeZones.  Changed mechanism to load from static
*                           array rather than resource bundle.
*   07/07/1998  srl         Bugfixes from the Java side: UTC GMT CAT NST
*                           Added getDisplayName API
*                           going to add custom parsing.
*
*                           ISSUES:
*                               - should getDisplayName cache something?
*                               - should custom time zones be cached? [probably]
*  08/10/98     stephen     Brought getDisplayName() API in-line w/ conventions
*  08/19/98     stephen     Changed createTimeZone() to never return 0
*  09/02/98     stephen     Added getOffset(monthLen) and hasSameRules()
*  09/15/98     stephen     Added getStaticClassID()
*  02/22/99     stephen     Removed character literals for EBCDIC safety
*  05/04/99     stephen     Changed initDefault() for Mutex issues
*  07/12/99     helena      HPUX 11 CC Port.
*  12/03/99     aliu        Moved data out of static table into icudata.dll.
*                           Substantial rewrite of zone lookup, default zone, and
*                           available IDs code.  Misc. cleanup.
*********************************************************************************/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/simpletz.h"
#include "unicode/smpdtfmt.h"
#include "unicode/calendar.h"
#include "mutex.h"
#include "unicode/udata.h"
#include "tzdat.h"
#include "ucln_in.h"
#include "cstring.h"
#include "cmemory.h"
#include "unicode/strenum.h"

// static initialization


static const UChar         GMT_ID[] = {0x47, 0x4D, 0x54, 0x00}; /* "GMT" */
static const int32_t       GMT_ID_LENGTH = 3;
static const UChar         CUSTOM_ID[] = 
{
    0x43, 0x75, 0x73, 0x74, 0x6F, 0x6D, 0x00 /* "Custom" */
};

#ifdef ICU_TIMEZONE_USE_DEPRECATES
const TimeZone*            TimeZone::GMT = getGMT();
#endif

// See header file for documentation of the following
static const TZHeader *    DATA = NULL;
static const uint32_t*     INDEX_BY_ID = 0;
static const OffsetIndex*  INDEX_BY_OFFSET = 0;
static const CountryIndex* INDEX_BY_COUNTRY = 0;
static UDataMemory*        UDATA_POINTER = 0;
static UMTX                LOCK;
static UBool               DATA_LOADED = FALSE;
static void                loadZoneData(void);

U_NAMESPACE_BEGIN
static TimeZone*           DEFAULT_ZONE = NULL;
static TimeZone*           GMT = NULL;
static UnicodeString*      ZONE_IDS = 0;
const char                 TimeZone::fgClassID = 0; // Value is irrelevant

static const TZEquivalencyGroup* lookupEquivalencyGroup(const UnicodeString& id);
U_NAMESPACE_END
/**
 * udata callback to verify the zone data.
 */
U_CDECL_BEGIN
static UBool U_CALLCONV
isTimeZoneDataAcceptable(void * /*context*/,
                           const char * /*type*/, const char * /*name*/,
                           const UDataInfo *pInfo) {
    return
        pInfo->size >= sizeof(UDataInfo) &&
        pInfo->isBigEndian == U_IS_BIG_ENDIAN &&
        pInfo->charsetFamily == U_CHARSET_FAMILY &&
        pInfo->dataFormat[0] == TZ_SIG_0 &&
        pInfo->dataFormat[1] == TZ_SIG_1 &&
        pInfo->dataFormat[2] == TZ_SIG_2 &&
        pInfo->dataFormat[3] == TZ_SIG_3 &&
        pInfo->formatVersion[0] == TZ_FORMAT_VERSION;
}
U_CDECL_END

UBool timeZone_cleanup()
{
    U_NAMESPACE_USE

    DATA = NULL;
    INDEX_BY_ID = NULL;
    INDEX_BY_OFFSET = NULL;
    INDEX_BY_COUNTRY = NULL;
    if (ZONE_IDS) {
        delete []ZONE_IDS;
        ZONE_IDS = NULL;
    }
    if (UDATA_POINTER) {
        udata_close(UDATA_POINTER);
        UDATA_POINTER = NULL;
    }
    if (LOCK) {
        umtx_destroy(&LOCK);
        LOCK = NULL;
    }
    if (U_NAMESPACE_QUALIFIER GMT) {
        delete U_NAMESPACE_QUALIFIER GMT;
        U_NAMESPACE_QUALIFIER GMT = NULL;
    }
    if (DEFAULT_ZONE) {
        delete DEFAULT_ZONE;
        DEFAULT_ZONE = NULL;
    }
    DATA_LOADED = FALSE;
    return TRUE;
}

/**
 * Attempt to load the system zone data from icudata.dll (or its
 * equivalent).  After this call returns DATA_LOADED will be true.
 * DATA itself will be non-null if the load succeeded; otherwise it
 * will be null.  This call does nothing if the load has already
 * happened or or if it happens in another thread concurrently before
 * we can get there.
 *
 * After this call, we are guaranteed that DATA_LOADED is true.  We
 * are _not_ guaranteed that DATA will be nonzero.  If it is nonzero,
 * we are guaranteed that all associated data structures are
 * initialized.
 */
static void loadZoneData() {
    U_NAMESPACE_USE

    if (!DATA_LOADED) {
        Mutex lock(&LOCK);
        if (!DATA_LOADED) {
            UErrorCode status = U_ZERO_ERROR;
            UDATA_POINTER = udata_openChoice(0, TZ_DATA_TYPE, TZ_DATA_NAME, // THIS IS NOT A LEAK!
                   (UDataMemoryIsAcceptable*)isTimeZoneDataAcceptable, 0, &status); // see the comment on udata_close line
            UDataMemory *data = UDATA_POINTER;
            if (U_SUCCESS(status)) {
                DATA = (TZHeader*)udata_getMemory(data);
                // Result guaranteed to be nonzero if data is nonzero

                INDEX_BY_ID =
                    (const uint32_t*)((int8_t*)DATA + DATA->nameIndexDelta);
                INDEX_BY_OFFSET =
                    (const OffsetIndex*)((int8_t*)DATA + DATA->offsetIndexDelta);
                INDEX_BY_COUNTRY =
                    (const CountryIndex*)((int8_t*)DATA + DATA->countryIndexDelta);
                
                // Construct the available IDs array. The ordering
                // of this array conforms to the ordering of the
                // index by name table.
                ZONE_IDS = new UnicodeString[DATA->count];
                // Find start of name table, and walk through it
                // linearly.  If you're wondering why we don't use
                // the INDEX_BY_ID, it's because that indexes the
                // zone objects, not the name table.  The name
                // table is unindexed.
                const char* name = (const char*)DATA + DATA->nameTableDelta;
                int32_t length;
                for (uint32_t i=0; i<DATA->count; ++i) {
                    ZONE_IDS[i] = UnicodeString(name, ""); // invariant converter
                    length = ZONE_IDS[i].length();  // add a NUL but don't count it so that
                    ZONE_IDS[i].append((UChar)0);   // getBuffer() gets a terminated string
                    ZONE_IDS[i].truncate(length);
                    name += uprv_strlen(name) + 1;
                }

                //udata_close(data);    // Without udata_close purify will report a leak. However, DATA_LOADED is 
                                        // static, and udata_openChoice will be called only once, and data from
                                        // udata_openChoice needs to stick around.
                                        
            }

            // Whether we succeed or fail, stop future attempts
            DATA_LOADED = TRUE;
            U_NAMESPACE_QUALIFIER GMT = new SimpleTimeZone(0, UnicodeString(GMT_ID, GMT_ID_LENGTH));
            ucln_i18n_registerCleanup();
        }
    }
}

// -------------------------------------
U_NAMESPACE_BEGIN
const TimeZone*
TimeZone::getGMT(void)
{
    if (!DATA_LOADED) {
        loadZoneData();
    }
    return GMT;
}

// *****************************************************************************
// class TimeZone
// *****************************************************************************

TimeZone::TimeZone()
    :   UObject(), fID()
{
}

// -------------------------------------

TimeZone::TimeZone(const UnicodeString &id)
    :   UObject(), fID(id)
{
}

// -------------------------------------

TimeZone::~TimeZone()
{
}

// -------------------------------------

TimeZone::TimeZone(const TimeZone &source)
    :   UObject(source), fID(source.fID)
{
}

// -------------------------------------

TimeZone &
TimeZone::operator=(const TimeZone &right)
{
    if (this != &right) fID = right.fID;
    return *this;
}

// -------------------------------------

UBool
TimeZone::operator==(const TimeZone& that) const
{
    return getDynamicClassID() == that.getDynamicClassID() &&
        fID == that.fID;
}

// -------------------------------------

TimeZone*
TimeZone::createTimeZone(const UnicodeString& ID)
{
    /* We first try to lookup the zone ID in our system list.  If this
     * fails, we try to parse it as a custom string GMT[+-]hh:mm.  If
     * all else fails, we return GMT, which is probably not what the
     * user wants, but at least is a functioning TimeZone object.
     */
    TimeZone* result = 0;
    if (!DATA_LOADED) {
        loadZoneData();
    }
    if (DATA != 0) {
        result = createSystemTimeZone(ID);
    }
    if (result == 0) {
        result = createCustomTimeZone(ID);
    }
    if (result == 0) {
        result = getGMT()->clone();
    }
    return result;
}

/**
 * Lookup the given name in our system zone table.  If found,
 * instantiate a new zone of that name and return it.  If not
 * found, return 0.
 */
TimeZone*
TimeZone::createSystemTimeZone(const UnicodeString& name) {
    if (0 == DATA) {
        return 0;
    }
    
    const TZEquivalencyGroup *eg = lookupEquivalencyGroup(name);
    if (eg != 0) {
        return eg->isDST ?
            new SimpleTimeZone(eg->u.d.zone, name) :
            new SimpleTimeZone(eg->u.s.zone, name);                
    }
    return 0;
}

/**
 * Lookup the given ID in the system time zone equivalency group table.
 * Return a pointer to the equivalency group, or NULL if not found.
 * DATA MUST BE INITIALIZED AND NON-NULL.
 */
static const TZEquivalencyGroup*
lookupEquivalencyGroup(const UnicodeString& id) {
    // Perform a binary search.  Possible optimization: Unroll the
    // search.  Not worth it given the small number of zones (416 in
    // 1999j).
    uint32_t low = 0;
    uint32_t high = DATA->count;
    while (high > low) {
        // Invariant: match, if present, must be in the range [low,
        // high).
        uint32_t i = (low + high) / 2;
        int8_t c = id.compare(ZONE_IDS[i]);
        if (c == 0) {
            return (TZEquivalencyGroup*) ((int8_t*)DATA + INDEX_BY_ID[i]);
        } else if (c < 0) {
            high = i;
        } else {
            low = i + 1;
        }
    }
    return 0;
}

// -------------------------------------

void
TimeZone::initDefault()
{ 
    if (!DATA_LOADED) {
        loadZoneData();
    }
    // This function is called by createDefault() to initialize
    // fgDefaultZone from the system default time zone.  If
    // fgDefaultZone is already filled in, we obviously don't have to
    // do anything.
    if (DEFAULT_ZONE == 0) {
        Mutex lock(&LOCK);
        if (DEFAULT_ZONE == 0) {
            // We access system timezone data through TPlatformUtilities,
            // including tzset(), timezone, and tzname[].
            int32_t rawOffset = 0;
            const char *hostID;

            // First, try to create a system timezone, based
            // on the string ID in tzname[0].
            {
                // NOTE: Global mutex here; TimeZone mutex above
                // mutexed to avoid threading issues in the platform fcns.
                // Some of the locale/timezone OS functions may not be thread safe, 
                //  so the intent is that any setting from anywhere within ICU 
                //  happens with the ICU global mutex held.
                Mutex lock; 
                uprv_tzset(); // Initialize tz... system data
                
                // get the timezone ID from the host.
                hostID = uprv_tzname(0);
                
                // Invert sign because UNIX semantics are backwards
                rawOffset = uprv_timezone() * -U_MILLIS_PER_SECOND;
            }

            // Try to create a system zone with the given ID.  This
            // _always fails on Windows_ because Windows returns a
            // non-standard localized zone name, e.g., "Pacific
            // Standard Time" on U.S. systems set to PST.  One way to
            // fix this is to add a Windows-specific mapping table,
            // but that means we'd have to do so for every locale.  A
            // better way is to use the offset and find a
            // corresponding zone, which is what we do below.
            DEFAULT_ZONE = createSystemTimeZone(hostID);

            // If we couldn't get the time zone ID from the host, use
            // the default host timezone offset.  Further refinements
            // to this include querying the host to determine if DST
            // is in use or not and possibly using the host locale to
            // select from multiple zones at a the same offset.  We
            // don't do any of this now, but we could easily add this.
            if (DEFAULT_ZONE == 0 && DATA != 0) {
                // Use the designated default in the time zone list that has the
                // appropriate GMT offset, if there is one.

                const OffsetIndex* index = INDEX_BY_OFFSET;
                
                for (;;) {
                    if (index->gmtOffset > rawOffset) {
                        // Went past our desired offset; no match found
                        break;
                    }
                    if (index->gmtOffset == rawOffset) {
                        // Found our desired offset
                        DEFAULT_ZONE = createTimeZone(ZONE_IDS[index->defaultZone]);
                        break;
                    }
                    // Compute the position of the next entry.  If the delta value
                    // in this entry is zero, then there is no next entry.
                    uint16_t delta = index->nextEntryDelta;
                    if (delta == 0) {
                        break;
                    }
                    index = (const OffsetIndex*)((int8_t*)index + delta);
                }
            }

            // If we _still_ don't have a time zone, use GMT.  This
            // can only happen if the raw offset returned by
            // uprv_timezone() does not correspond to any system zone.
            if (DEFAULT_ZONE == 0) {
                DEFAULT_ZONE = getGMT()->clone();
            }
            ucln_i18n_registerCleanup();
        }
    }
}

// -------------------------------------

TimeZone*
TimeZone::createDefault()
{
    initDefault(); // After this call fgDefaultZone is not NULL
    Mutex lock(&LOCK); // In case adoptDefault is called
    return DEFAULT_ZONE->clone();
}

// -------------------------------------

void
TimeZone::adoptDefault(TimeZone* zone)
{
    if (zone != NULL)
    {
        Mutex mutex(&LOCK);

        if (DEFAULT_ZONE != NULL) {
            delete DEFAULT_ZONE;
        }

        DEFAULT_ZONE = zone;
    }
}
// -------------------------------------

void
TimeZone::setDefault(const TimeZone& zone)
{
    adoptDefault(zone.clone());
}

// -------------------------------------

// New available IDs API as of ICU 2.4.  Uses StringEnumeration API.

class TZEnumeration : public StringEnumeration {
    // Map into to ZONE_IDS.  Our results are ZONE_IDS[map[i]] for
    // i=0..len-1.  If map==NULL then our results are ZONE_IDS[i]
    // for i=0..len-1.  Len will be zero iff the zone data could
    // not be loaded.
    int32_t* map;
    int32_t  len;
    int32_t  pos;
    void* _bufp;
    int32_t _buflen;
    enum { BUFFER_GROW = 8 }; // must be >= 1

public:
    TZEnumeration() {
        map = NULL;
		_bufp = NULL;
        len = pos = _buflen = 0;
        if (!DATA_LOADED) {
            loadZoneData();
        }
        if (DATA != NULL) {
            len = DATA->count;
        }
    }

    TZEnumeration(int32_t rawOffset) {
        map = NULL;
		_bufp = NULL;
        len = pos = _buflen = 0;
        if (!DATA_LOADED) {
            loadZoneData();
        }
        if (DATA == NULL) return;

        /* The offset index table is a table of variable-sized objects.
         * Each entry has an offset to the next entry; the last entry has
         * a next entry offset of zero.
         *
         * The entries are sorted in ascending numerical order of GMT
         * offset.  Each entry lists all the system zones at that offset,
         * in lexicographic order of ID.  Note that this ordering is
         * somewhat significant in that the _first_ zone in each list is
         * what will be chosen as the default under certain fallback
         * conditions.  We currently just let that be the
         * lexicographically first zone, but we could also adjust the list
         * to pick which zone was first for this situation -- probably not
         * worth the trouble, except for the fact that this fallback is
         * _always_ used to determine the default zone on Windows.
         *
         * The list of zones is actually just a list of integers, from
         * 0..n-1, where n is the total number of system zones.  The
         * numbering corresponds exactly to the ordering of ZONE_IDS.
         */
        const OffsetIndex* index = INDEX_BY_OFFSET;
        
        for (;;) {
            if (index->gmtOffset > rawOffset) {
                // Went past our desired offset; no match found
                break;
            }
            if (index->gmtOffset == rawOffset) {
                // Found our desired offset
                map = (int32_t*)uprv_malloc(sizeof(int32_t) * index->count);
                if (map != NULL) {
                    len = index->count;
                    const uint16_t* zoneNumberArray = &(index->zoneNumber);
                    for (uint16_t i=0; i<len; ++i) {
                        map[i] = zoneNumberArray[i];
                    }
                }
            }
            // Compute the position of the next entry.  If the delta value
            // in this entry is zero, then there is no next entry.
            uint16_t delta = index->nextEntryDelta;
            if (delta == 0) {
                break;
            }
            index = (const OffsetIndex*)((int8_t*)index + delta);
        }
    }

    TZEnumeration(const char* country) {
        map = NULL;
		_bufp = NULL;
        len = pos = _buflen = 0;
        if (!DATA_LOADED) {
            loadZoneData();
        }
        if (DATA == NULL) return;

        /* The country index table is a table of variable-sized objects.
         * Each entry has an offset to the next entry; the last entry has
         * a next entry offset of zero.
         *
         * The entries are sorted in ascending numerical order of intcode.
         * This is an integer representation of the 2-letter ISO 3166
         * country code.  It is computed as (c1-'A')*32 + (c0-'A'), where
         * the country code is c1 c0, with 'A' <= ci <= 'Z'.
         *
         * The list of zones is a list of integers, from 0..n-1, where n
         * is the total number of system zones.  The numbering corresponds
         * exactly to the ordering of ZONE_IDS.
         */
        const CountryIndex* index = INDEX_BY_COUNTRY;

        uint16_t intcode = 0;
        if (country != NULL && *country != 0) {
            intcode = (uint16_t)((U_UPPER_ORDINAL(country[0]) << 5)
                + U_UPPER_ORDINAL(country[1]));
        }

        for (;;) {
            if (index->intcode > intcode) {
                // Went past our desired country; no match found
                break;
            }
            if (index->intcode == intcode) {
                // Found our desired country
                map = (int32_t*)uprv_malloc(sizeof(int32_t) * index->count);
                if (map != NULL) {
                    len = index->count;
                    const uint16_t* zoneNumberArray = &(index->zoneNumber);
                    for (uint16_t i=0; i<len; ++i) {
                        map[i] = zoneNumberArray[i];
                    }
                }
            }
            // Compute the position of the next entry.  If the delta value
            // in this entry is zero, then there is no next entry.
            uint16_t delta = index->nextEntryDelta;
            if (delta == 0) {
                break;
            }
            index = (const CountryIndex*)((int8_t*)index + delta);
        }
    }

    virtual ~TZEnumeration() {
        uprv_free(map);
		uprv_free(_bufp);
    }

    int32_t count(UErrorCode& status) const {
        return U_FAILURE(status) ? 0 : len;
    }

    const char* next(int32_t* resultLength, UErrorCode& status) {
        const UnicodeString* us = snext(status);
        if (us) {
            int newlen;
            for (newlen = us->extract((char*)_bufp, _buflen / sizeof(char), NULL, status);
                 status == U_STRING_NOT_TERMINATED_WARNING || status == U_BUFFER_OVERFLOW_ERROR;
                 ) {
                resizeBuffer((newlen + BUFFER_GROW) * sizeof(char));
                status = U_ZERO_ERROR;
            }
            if (U_SUCCESS(status)) {
                ((char*)_bufp)[newlen] = 0;
                if (resultLength) {
                  resultLength[0] = newlen;
                }
                return (const char*)_bufp;
            }
        }
        return NULL;
    }

    const UChar* unext(int32_t* resultLength, UErrorCode& status) {
        const UnicodeString* us = snext(status);
        if (us != NULL) {
          if (resultLength) {
            resultLength[0] = us->length();
          }
          // TimeZone terminates the ID strings when it builds them
          return us->getBuffer();
        }
        return NULL;
    }

    const UnicodeString* snext(UErrorCode& status) {
        if (U_SUCCESS(status) && pos < len) {
            return (map != NULL) ?
                &ZONE_IDS[map[pos++]] : &ZONE_IDS[pos++];
        }
        return NULL;
    }

    void reset(UErrorCode& status) {
        pos = 0;
    }

private:
    void resizeBuffer(int32_t newlen) {
        if (_bufp) {
            _bufp = uprv_realloc(_bufp, newlen);
        } else {
            _bufp = uprv_malloc(newlen);
        }
        _buflen = newlen;
    }
};

StringEnumeration*
TimeZone::createEnumeration() {
    return new TZEnumeration();
}

StringEnumeration*
TimeZone::createEnumeration(int32_t rawOffset) {
    return new TZEnumeration(rawOffset);
}

StringEnumeration*
TimeZone::createEnumeration(const char* country) {
    return new TZEnumeration(country);
}

// -------------------------------------

// TODO: #ifdef out this code after 8-Nov-2003
// #ifdef ICU_TIMEZONE_USE_DEPRECATES

const UnicodeString** 
TimeZone::createAvailableIDs(int32_t rawOffset, int32_t& numIDs)
{
    // We are creating a new array to existing UnicodeString pointers.
    // The caller will delete the array when done, but not the pointers
    // in the array.
    
    if (!DATA_LOADED) {
        loadZoneData();
    }
    if (0 == DATA) {
        numIDs = 0;
        return 0;
    }

    /* The offset index table is a table of variable-sized objects.
     * Each entry has an offset to the next entry; the last entry has
     * a next entry offset of zero.
     *
     * The entries are sorted in ascending numerical order of GMT
     * offset.  Each entry lists all the system zones at that offset,
     * in lexicographic order of ID.  Note that this ordering is
     * somewhat significant in that the _first_ zone in each list is
     * what will be chosen as the default under certain fallback
     * conditions.  We currently just let that be the
     * lexicographically first zone, but we could also adjust the list
     * to pick which zone was first for this situation -- probably not
     * worth the trouble, except for the fact that this fallback is
     * _always_ used to determine the default zone on Windows.
     *
     * The list of zones is actually just a list of integers, from
     * 0..n-1, where n is the total number of system zones.  The
     * numbering corresponds exactly to the ordering of ZONE_IDS.
     */
    const OffsetIndex* index = INDEX_BY_OFFSET;

    for (;;) {
        if (index->gmtOffset > rawOffset) {
            // Went past our desired offset; no match found
            break;
        }
        if (index->gmtOffset == rawOffset) {
            // Found our desired offset
            const UnicodeString** result =
                (const UnicodeString**)uprv_malloc(index->count * sizeof(UnicodeString *));
            const uint16_t* zoneNumberArray = &(index->zoneNumber);
            for (uint16_t i=0; i<index->count; ++i) {
                // Pointer assignment - use existing UnicodeString object!
                // Don't create a new UnicodeString on the heap here!
                result[i] = &ZONE_IDS[zoneNumberArray[i]];
            }
            numIDs = index->count;
            return result;
        }
        // Compute the position of the next entry.  If the delta value
        // in this entry is zero, then there is no next entry.
        uint16_t delta = index->nextEntryDelta;
        if (delta == 0) {
            break;
        }
        index = (const OffsetIndex*)((int8_t*)index + delta);
    }

    numIDs = 0;
    return 0;
}

// -------------------------------------

const UnicodeString** 
TimeZone::createAvailableIDs(const char* country, int32_t& numIDs) {

    // We are creating a new array to existing UnicodeString pointers.
    // The caller will delete the array when done, but not the pointers
    // in the array.
    
    if (!DATA_LOADED) {
        loadZoneData();
    }
    if (0 == DATA) {
        numIDs = 0;
        return 0;
    }

    /* The country index table is a table of variable-sized objects.
     * Each entry has an offset to the next entry; the last entry has
     * a next entry offset of zero.
     *
     * The entries are sorted in ascending numerical order of intcode.
     * This is an integer representation of the 2-letter ISO 3166
     * country code.  It is computed as (c1-'A')*32 + (c0-'A'), where
     * the country code is c1 c0, with 'A' <= ci <= 'Z'.
     *
     * The list of zones is a list of integers, from 0..n-1, where n
     * is the total number of system zones.  The numbering corresponds
     * exactly to the ordering of ZONE_IDS.
     */
    const CountryIndex* index = INDEX_BY_COUNTRY;

    uint16_t intcode = 0;
    if (country != NULL && *country != 0) {
        intcode = (uint16_t)((U_UPPER_ORDINAL(country[0]) << 5)
            + U_UPPER_ORDINAL(country[1]));
    }
    
    for (;;) {
        if (index->intcode > intcode) {
            // Went past our desired country; no match found
            break;
        }
        if (index->intcode == intcode) {
            // Found our desired country
            const UnicodeString** result =
                (const UnicodeString**)uprv_malloc(index->count * sizeof(UnicodeString *));
            const uint16_t* zoneNumberArray = &(index->zoneNumber);
            for (uint16_t i=0; i<index->count; ++i) {
                // Pointer assignment - use existing UnicodeString object!
                // Don't create a new UnicodeString on the heap here!
                result[i] = &ZONE_IDS[zoneNumberArray[i]];
            }
            numIDs = index->count;
            return result;
        }
        // Compute the position of the next entry.  If the delta value
        // in this entry is zero, then there is no next entry.
        uint16_t delta = index->nextEntryDelta;
        if (delta == 0) {
            break;
        }
        index = (const CountryIndex*)((int8_t*)index + delta);
    }

    numIDs = 0;
    return 0;
}

// -------------------------------------

const UnicodeString** 
TimeZone::createAvailableIDs(int32_t& numIDs)
{
    // We are creating a new array to existing UnicodeString pointers.
    // The caller will delete the array when done, but not the pointers
    // in the array.
    //
    // This is really unnecessary, given the fact that we have an
    // array of the IDs already constructed, and we could just return
    // that.  However, that would be a breaking API change, and some
    // callers familiar with the original API might try to delete it.

    if (!DATA_LOADED) {
        loadZoneData();
    }
    if (0 == DATA) {
        numIDs = 0;
        return 0;
    }

    const UnicodeString** result =
        (const UnicodeString** )uprv_malloc(DATA->count * sizeof(UnicodeString *));

    // Create a list of pointers to each and every zone ID
    for (uint32_t i=0; i<DATA->count; ++i) {
        // Pointer assignment - use existing UnicodeString object!
        // Don't create a new UnicodeString on the heap here!
        result[i] = &ZONE_IDS[i];
    }

    numIDs = DATA->count;
    return result;
}

// ICU_TIMEZONE_USE_DEPRECATES
// #endif
// see above

// ---------------------------------------

int32_t
TimeZone::countEquivalentIDs(const UnicodeString& id) {
    if (!DATA_LOADED) {
        loadZoneData();
    }
    if (0 == DATA) {
        return 0;
    }
    const TZEquivalencyGroup *eg = lookupEquivalencyGroup(id);
    return (eg != 0) ? (eg->isDST ? eg->u.d.count : eg->u.s.count) : 0;
}

// ---------------------------------------

const UnicodeString
TimeZone::getEquivalentID(const UnicodeString& id, int32_t index) {
    if (!DATA_LOADED) {
        loadZoneData();
    }
    if (0 != DATA) {
        const TZEquivalencyGroup *eg = lookupEquivalencyGroup(id);
        if (eg != 0) {
            const uint16_t *p = eg->isDST ? &eg->u.d.count : &eg->u.s.count;
            if (index >= 0 && index < *p) {
                return ZONE_IDS[p[index+1]];
            }
        }
    }
    return UnicodeString();
}

// ---------------------------------------


UnicodeString&
TimeZone::getDisplayName(UnicodeString& result) const
{
    return getDisplayName(FALSE,LONG,Locale::getDefault(), result);
}

UnicodeString&
TimeZone::getDisplayName(const Locale& locale, UnicodeString& result) const
{
    return getDisplayName(FALSE, LONG, locale, result);
}

UnicodeString&
TimeZone::getDisplayName(UBool daylight, EDisplayType style, UnicodeString& result)  const
{
    return getDisplayName(daylight,style, Locale::getDefault(), result);
}

UnicodeString&
TimeZone::getDisplayName(UBool daylight, EDisplayType style, const Locale& locale, UnicodeString& result) const
{
    // SRL TODO: cache the SDF, just like java.
    UErrorCode status = U_ZERO_ERROR;

    SimpleDateFormat format(style == LONG ? "zzzz" : "z",locale,status);

    if(!U_SUCCESS(status))
    {
        // *** SRL what do I do here?!!
        return result.remove();
    }

    // Create a new SimpleTimeZone as a stand-in for this zone; the
    // stand-in will have no DST, or all DST, but the same ID and offset,
    // and hence the same display name.
    // We don't cache these because they're small and cheap to create.
    UnicodeString tempID;
    SimpleTimeZone *tz =  daylight ?
        // For the pure-DST zone, we use JANUARY and DECEMBER

        new SimpleTimeZone(getRawOffset(), getID(tempID),
                           Calendar::JANUARY , 1, 0, 0,
                           Calendar::DECEMBER , 31, 0, U_MILLIS_PER_DAY, status) :
        new SimpleTimeZone(getRawOffset(), getID(tempID));

    format.applyPattern(style == LONG ? "zzzz" : "z");
    Calendar *myCalendar = (Calendar*)format.getCalendar();
    myCalendar->setTimeZone(*tz); // copy
    
    delete tz;

    FieldPosition pos(FieldPosition::DONT_CARE);
    return format.format(UDate(196262345678.), result, pos); // Must use a valid date here.
}


/**
 * Parse a custom time zone identifier and return a corresponding zone.
 * @param id a string of the form GMT[+-]hh:mm, GMT[+-]hhmm, or
 * GMT[+-]hh.
 * @return a newly created SimpleTimeZone with the given offset and
 * no Daylight Savings Time, or null if the id cannot be parsed.
*/
TimeZone*
TimeZone::createCustomTimeZone(const UnicodeString& id)
{
    static const int32_t         kParseFailed = -99999;

    NumberFormat* numberFormat = 0;
    
    UnicodeString idUppercase = id;
    idUppercase.toUpper();

    if (id.length() > GMT_ID_LENGTH &&
        idUppercase.startsWith(GMT_ID))
    {
        ParsePosition pos(GMT_ID_LENGTH);
        UBool negative = FALSE;
        int32_t offset;

        if (id[pos.getIndex()] == 0x002D /*'-'*/)
            negative = TRUE;
        else if (id[pos.getIndex()] != 0x002B /*'+'*/)
            return 0;
        pos.setIndex(pos.getIndex() + 1);

        UErrorCode success = U_ZERO_ERROR;
        numberFormat = NumberFormat::createInstance(success);
        numberFormat->setParseIntegerOnly(TRUE);

    
        // Look for either hh:mm, hhmm, or hh
        int32_t start = pos.getIndex();
        
        Formattable n(kParseFailed);

        numberFormat->parse(id, n, pos);
        if (pos.getIndex() == start) {
            delete numberFormat;
            return 0;
        }
        offset = n.getLong();

        if (pos.getIndex() < id.length() &&
            id[pos.getIndex()] == 0x003A /*':'*/)
        {
            // hh:mm
            offset *= 60;
            pos.setIndex(pos.getIndex() + 1);
            int32_t oldPos = pos.getIndex();
            n.setLong(kParseFailed);
            numberFormat->parse(id, n, pos);
            if (pos.getIndex() == oldPos) {
                delete numberFormat;
                return 0;
            }
            offset += n.getLong();
        }
        else 
        {
            // hhmm or hh

            // Be strict about interpreting something as hh; it must be
            // an offset < 30, and it must be one or two digits. Thus
            // 0010 is interpreted as 00:10, but 10 is interpreted as
            // 10:00.
            if (offset < 30 && (pos.getIndex() - start) <= 2)
                offset *= 60; // hh, from 00 to 29; 30 is 00:30
            else
                offset = offset % 100 + offset / 100 * 60; // hhmm
        }

        if(negative)
            offset = -offset;

        delete numberFormat;
        return new SimpleTimeZone(offset * 60000, CUSTOM_ID);
    }
    return 0;
}


UBool 
TimeZone::hasSameRules(const TimeZone& other) const
{
    return (getRawOffset() == other.getRawOffset() && 
            useDaylightTime() == other.useDaylightTime());
}

U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_FORMATTING */

//eof
