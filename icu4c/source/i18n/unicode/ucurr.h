/*
**********************************************************************
* Copyright (c) 2002, International Business Machines
* Corporation and others.  All Rights Reserved.
**********************************************************************
* $Source: /xsrl/Nsvn/icu/icu/source/i18n/unicode/ucurr.h,v $ 
* $Revision: 1.5 $
**********************************************************************
*/
#ifndef _UCURR_H_
#define _UCURR_H_

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

/**
 * The ucurr API encapsulates information about a currency, as defined by
 * ISO 4217.  A currency is represented by a 3-character string
 * containing its ISO 4217 code.  This API can return various data
 * necessary the proper display of a currency:
 *
 * <ul><li>A display symbol, for a specific locale
 * <li>The number of fraction digits to display
 * <li>A rounding increment
 * </ul>
 *
 * The <tt>DecimalFormat</tt> class uses these data to display
 * currencies.
 * @author Alan Liu
 * @since ICU 2.2
 */

/**
 * Returns a currency code for the default currency in the given
 * locale.
 * @param locale the locale for which to retrieve a currency code
 * @param ec error code
 * @return a pointer to a 3-character ISO 4217 currency code, or
 * NULL if none is found.  The result string may NOT be null
 * terminated.
 * @draft ICU 2.2
 */
U_CAPI const UChar* U_EXPORT2
ucurr_forLocale(const char* locale,
                UErrorCode* ec);

/**
 * Selector constants for ucurr_getName().
 *
 * @see ucurr_getName
 * @draft ICU 2.6
 */
typedef enum UCurrNameStyle {
    /**
     * Selector for ucurr_getName indicating a symbolic name for a
     * currency, such as "$" for USD.
     * @draft ICU 2.6
     */
    UCURR_SYMBOL_NAME,

    /**
     * Selector for ucurr_getName indicating the long name for a
     * currency, such as "US Dollar" for USD.
     * @draft ICU 2.6
     */
    UCURR_LONG_NAME
} UCurrNameStyle;

/**
 * Returns the display name for the given currency in the
 * given locale.  For example, the display name for the USD
 * currency object in the en_US locale is "$".
 * @param currency null-terminated 3-letter ISO 4217 code
 * @param locale locale in which to display currency
 * @param nameStyle selector for which kind of name to return
 * @param isChoiceFormat fill-in set to TRUE if the returned value
 * is a ChoiceFormat pattern; otherwise it is a static string
 * @param len fill-in parameter to receive length of result
 * @param ec error code
 * @return pointer to display string of 'len' UChars.  If the
 * resource data contains no entry for 'currency', then
 * 'currency' itself is returned.  The result string may NOT be
 * null terminated.  If *isChoiceFormat is TRUE, then the result
 * is a ChoiceFormat pattern.  Otherwise it is a static string.
 * @draft ICU 2.6
 */
U_CAPI const UChar* U_EXPORT2
ucurr_getName(const UChar* currency,
              const char* locale,
              UCurrNameStyle nameStyle,
              UBool* isChoiceFormat,
              int32_t* len,
              UErrorCode* ec);

/**
 * Returns the number of the number of fraction digits that should
 * be displayed for the given currency.
 * @param currency null-terminated 3-letter ISO 4217 code
 * @return a non-negative number of fraction digits to be
 * displayed
 * @draft ICU 2.2
 */
U_CAPI int32_t U_EXPORT2
ucurr_getDefaultFractionDigits(const UChar* currency);

/**
 * Returns the rounding increment for the given currency, or 0.0 if no
 * rounding is done by the currency.
 * @param currency null-terminated 3-letter ISO 4217 code
 * @return the non-negative rounding increment, or 0.0 if none
 * @draft ICU 2.2
 */
U_CAPI double U_EXPORT2
ucurr_getRoundingIncrement(const UChar* currency);

#endif /* #if !UCONFIG_NO_FORMATTING */

#endif
