/*
**********************************************************************
*   Copyright (C) 2000-2002, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*/

#ifndef URESIMP_H
#define URESIMP_H

#include "unicode/ures.h"

#include "uresdata.h"

#define kRootLocaleName         "root"

/*
 The default minor version and the version separator must be exactly one
 character long.
*/

#define kDefaultMinorVersion    "0"
#define kVersionSeparator       "."
#define kVersionTag             "Version"

#define MAGIC1 19700503
#define MAGIC2 19641227

#define URES_MAX_ALIAS_LEVEL 256

/*
enum UResEntryType {
    ENTRY_OK = 0,
    ENTRY_GOTO_ROOT = 1,
    ENTRY_GOTO_DEFAULT = 2,
    ENTRY_INVALID = 3
};

typedef enum UResEntryType UResEntryType;
*/

struct UResourceDataEntry;
typedef struct UResourceDataEntry UResourceDataEntry;

struct UResourceDataEntry {
    char *fName; /* name of the locale for bundle - still to decide whether it is original or fallback */
    char *fPath; /* path to bundle - used for distinguishing between resources with the same name */
    uint32_t fCountExisting; /* how much is this resource used */
    ResourceData fData; /* data for low level access */
    UResourceDataEntry *fParent; /*next resource in fallback chain*/
/*    UResEntryType fStatus;*/
    UErrorCode fBogus;
    int32_t fHashKey; /* for faster access in the hashtable */
};

#define RES_BUFSIZE 256
#define RES_PATH_SEPARATOR   '/'
#define RES_PATH_SEPARATOR_S   "/"

struct UResourceBundle {
    const char *fKey; /*tag*/
    char *fResPath; /* full path to the resource: "zh_TW/CollationElements/Sequence" */
    char fResBuf[RES_BUFSIZE];
    int32_t fResPathLen;
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

    /* parent of this resource - 
     * lives in the same data entry 
     */
    /* This cannot be done right now - need support in genrb */
    /*Resource fParent; */
};

U_CFUNC void ures_initStackObject(UResourceBundle* resB);
U_CFUNC void ures_setIsStackObject( UResourceBundle* resB, UBool state);
U_CFUNC UBool ures_isStackObject( UResourceBundle* resB);

/* Some getters used by the copy constructor */
U_CFUNC const char* ures_getName(const UResourceBundle* resB);
U_CFUNC const char* ures_getPath(const UResourceBundle* resB);
U_CFUNC void ures_appendResPath(UResourceBundle *resB, const char* toAdd);
/*U_CFUNC void ures_setResPath(UResourceBundle *resB, const char* toAdd);*/
U_CFUNC void ures_freeResPath(UResourceBundle *resB);

/* Candidates for export */
U_CFUNC UResourceBundle *ures_copyResb(UResourceBundle *r, const UResourceBundle *original, UErrorCode *status);
#endif /*URESIMP_H*/
