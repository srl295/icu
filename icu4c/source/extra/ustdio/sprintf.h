/*
******************************************************************************
*
*   Copyright (C) 2000-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
******************************************************************************
*
* File sprintf.h
*
* Modification History:
*
*   Date        Name        Description
*   02/08/00    george      Creation. Copied from uprintf.h
******************************************************************************
*/

#ifndef USPRINTF_H
#define USPRINTF_H

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "uprintf.h"
#include "locbund.h"
#include "uprintf.h"

typedef struct u_localized_string {
    UChar     *str;     /* Place to write the string */
    int32_t   available;/* Number of codeunits available to write to */
    int32_t   len;      /* Maximum number of code units that can be written to output */

    ULocaleBundle  fBundle;     /* formatters */
} u_localized_string;

#endif /* #if !UCONFIG_NO_FORMATTING */

#endif
