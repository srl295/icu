/*
**********************************************************************
*   Copyright (C) 2000-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*/

#ifndef URESIMP_H
#define URESIMP_H

#include "unicode/ures.h"

#include "unicode/uloc.h"
#include "unicode/ustring.h"
#include "cmemory.h"
#include "cstring.h"
#include "uresdata.h"
#include "uhash.h"
#include "umutex.h"

#define kRootLocaleName         "root"
#define kIndexLocaleName        "index"
#define kIndexTag               "InstalledLocales"

/*
 The default minor version and the version separator must be exactly one
 character long.
*/

#define kDefaultMinorVersion    "0"
#define kVersionSeparator       "."
#define kVersionTag             "Version"

#define MAGIC1 19700503
#define MAGIC2 19641227


enum UResEntryType {
    ENTRY_OK = 0,
    ENTRY_GOTO_ROOT = 1,
    ENTRY_GOTO_DEFAULT = 2,
    ENTRY_INVALID = 3
};

typedef enum UResEntryType UResEntryType;

struct UResourceDataEntry;
typedef struct UResourceDataEntry UResourceDataEntry;

struct UResourceDataEntry {
    char *fName; /* name of the locale for bundle - still to decide whether it is original or fallback */
    char *fPath; /* path to bundle - used for distinguishing between resources with the same name */
    uint32_t fCountExisting; /* how much is this resource used */
    ResourceData fData; /* data for low level access */
    UResourceDataEntry *fParent; /*next resource in fallback chain*/
    UResEntryType fStatus;
    UErrorCode fBogus;
    int32_t fHashKey; /* for faster access in the hashtable */
};

struct UResourceBundle {
    const char *fKey; /*tag*/
    char *fVersion;
    UBool fHasFallback;
    UBool fIsTopLevel;
    uint32_t fMagic1;
    uint32_t fMagic2;
    /*UBool fIsStackObject;*/
    UResourceDataEntry *fData; /*for low-level access*/
    int32_t fIndex;
    int32_t fSize;
    ResourceData fResData;
    Resource fRes;
};

U_CFUNC const char* ures_getRealLocale(const UResourceBundle* resourceBundle, UErrorCode* status);
U_CAPI void ures_setIsStackObject( UResourceBundle* resB, UBool state);
U_CAPI UBool ures_isStackObject( UResourceBundle* resB, UErrorCode *status);

U_CFUNC const ResourceData *getFallbackData(const UResourceBundle* resBundle, const char* * resTag, UResourceDataEntry* *realData, Resource *res, UErrorCode *status);
U_CFUNC int32_t hashBundle(const void *parm);
U_CFUNC UBool compareBundles(const void *p1, const void *p2);

/* Candidates for export */
U_CFUNC UResourceBundle *copyResb(UResourceBundle *r, const UResourceBundle *original, UErrorCode *status);
U_CFUNC const ResourceData * ures_getResData(const UResourceBundle* resB);
#endif /*URESIMP_H*/
