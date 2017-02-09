// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
*****************************************************************************************
* Copyright (C) 2010-2012, International Business Machines
* Corporation and others. All Rights Reserved.
*****************************************************************************************
*/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/upluralrules.h"
#include "unicode/plurrule.h"
#include "unicode/locid.h"
#include "unicode/unistr.h"
#include "unicode/unum.h"
#include "unicode/numfmt.h"

U_NAMESPACE_USE


U_CAPI UPluralRules* U_EXPORT2
uplrules_open(const char *locale, UErrorCode *status)
{
    return uplrules_openForType(locale, UPLURAL_TYPE_CARDINAL, status);
}

U_CAPI UPluralRules* U_EXPORT2
uplrules_openForType(const char *locale, UPluralType type, UErrorCode *status)
{
    return (UPluralRules*)PluralRules::forLocale(Locale(locale), type, *status);
}

U_CAPI void U_EXPORT2
uplrules_close(UPluralRules *uplrules)
{
    delete (PluralRules*)uplrules;
}

U_CAPI int32_t U_EXPORT2
uplrules_select(const UPluralRules *uplrules,
                double number,
                UChar *keyword, int32_t capacity,
                UErrorCode *status)
{
    if (U_FAILURE(*status)) {
        return 0;
    }
    if (keyword == NULL ? capacity != 0 : capacity < 0) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    UnicodeString result = ((PluralRules*)uplrules)->select(number);
    return result.extract(keyword, capacity, *status);
}

U_CAPI int32_t U_EXPORT2
uplrules_selectWithFormat(const UPluralRules *uplrules,
                          double number,
                          const UNumberFormat *fmt,
                          UChar *keyword, int32_t capacity,
                          UErrorCode *status)
{
    if (U_FAILURE(*status)) {
        return 0;
    }
    const PluralRules* plrules = reinterpret_cast<const PluralRules*>(uplrules);
    const NumberFormat* nf = reinterpret_cast<const NumberFormat*>(fmt);
    if (plrules == NULL || nf == NULL || ((keyword == NULL)? capacity != 0 : capacity < 0)) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    Formattable obj(number);
    UnicodeString result = plrules->select(obj, *nf, *status);
    return result.extract(keyword, capacity, *status);
}

#endif /* #if !UCONFIG_NO_FORMATTING */
