/*
******************************************************************************
*                                                                            *
* Copyright (C) 2001-2001, International Business Machines                   *
*                Corporation and others. All Rights Reserved.                *
*                                                                            *
******************************************************************************
*   file name:  ucln_cmn.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2001July05
*   created by: George Rhoten
*/

#ifndef __UCLN_CMN_H__
#define __UCLN_CMN_H__

#include "unicode/utypes.h"

/* These are the cleanup functions for various APIs. */

U_CFUNC UBool unames_cleanup(void);

U_CFUNC UBool unorm_cleanup(void);

U_CFUNC UBool uchar_cleanup(void);

U_CFUNC UBool locale_cleanup(void);

U_CFUNC UBool uloc_cleanup(void);

U_CFUNC UBool ustring_cleanup(void);

/* @deprecated this functionality is going away */
U_CFUNC UBool UnicodeConverter_cleanup(void);

U_CAPI UBool U_EXPORT2 ucnv_cleanup(void);

U_CFUNC UBool ucnv_io_cleanup(void);

U_CFUNC UBool ures_cleanup(void);

U_CFUNC UBool udata_cleanup(void);

U_CFUNC UBool putil_cleanup(void);

U_CFUNC UBool upropset_cleanup(void);

#endif
