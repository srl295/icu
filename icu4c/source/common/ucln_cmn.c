/*
******************************************************************************
*                                                                            *
* Copyright (C) 2001-2001, International Business Machines                   *
*                Corporation and others. All Rights Reserved.                *
*                                                                            *
******************************************************************************
*   file name:  ucln_cmn.c
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2001July05
*   created by: George Rhoten
*/

#include "unicode/uclean.h"
#include "ucln_cmn.h"
#include "umutex.h"
#include "ucln.h"

static cleanupFunc *gCleanupFunctions[UCLN_COMMON] = {
    NULL,
    NULL,
    NULL
};

U_CAPI void U_EXPORT2
ucln_registerCleanup(ECleanupLibraryType type,
                     cleanupFunc *func)
{
    if (UCLN_START < type && type < UCLN_COMMON)
    {
        gCleanupFunctions[type] = func;
    }
}

U_CAPI void U_EXPORT2
u_cleanup(void)
{

    ECleanupLibraryType libType = UCLN_START;
    while (++libType < UCLN_COMMON)
    {
        if (gCleanupFunctions[libType])
        {
            gCleanupFunctions[libType]();
        }

    }
    uset_cleanup();
    unorm_cleanup();
    unames_cleanup();
    uchar_cleanup();
    pname_cleanup();
    locale_cleanup();
    uloc_cleanup();
    ustring_cleanup();
#if !UCONFIG_NO_BREAK_ITERATION
	breakiterator_cleanup();
#endif
#if !UCONFIG_NO_SERVICE
    service_cleanup();
#endif
    ucnv_cleanup();
    ucnv_io_cleanup();
    ures_cleanup();
    udata_cleanup();
    putil_cleanup();
    ustrprep_cleanup();
    /*
     * WARNING! Destroying the global mutex can cause synchronization
     * problems.  ICU must be reinitialized from a single thread
     * before the library is used again.  You never want two
     * threads trying to initialize the global mutex at the same
     * time. The global mutex is being destroyed so that heap and
     * resource checkers don't complain. [grhoten]
     */
    umtx_destroy(NULL);
}

