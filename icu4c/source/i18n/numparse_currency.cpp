// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT

#include "numparse_types.h"
#include "numparse_currency.h"
#include "ucurrimp.h"
#include "unicode/errorcode.h"
#include "numparse_utils.h"

using namespace icu;
using namespace icu::numparse;
using namespace icu::numparse::impl;


CurrencyNamesMatcher::CurrencyNamesMatcher(const Locale& locale, UErrorCode& status)
        : fLocaleName(locale.getName(), -1, status) {}

bool CurrencyNamesMatcher::match(StringSegment& segment, ParsedNumber& result, UErrorCode& status) const {
    if (result.currencyCode[0] != 0) {
        return false;
    }

    // NOTE: This requires a new UnicodeString to be allocated, instead of using the StringSegment.
    // This should be fixed with #13584.
    UnicodeString segmentString = segment.toUnicodeString();

    // Try to parse the currency
    ParsePosition ppos(0);
    int32_t partialMatchLen = 0;
    uprv_parseCurrency(
            fLocaleName.data(),
            segmentString,
            ppos,
            UCURR_SYMBOL_NAME, // checks for both UCURR_SYMBOL_NAME and UCURR_LONG_NAME
            &partialMatchLen,
            result.currencyCode,
            status);

    // Possible partial match
    bool partialMatch = partialMatchLen == segment.length();

    if (U_SUCCESS(status) && ppos.getIndex() != 0) {
        // Complete match.
        // NOTE: The currency code should already be saved in the ParsedNumber.
        segment.adjustOffset(ppos.getIndex());
        result.setCharsConsumed(segment);
    }

    return partialMatch;
}

const UnicodeSet& CurrencyNamesMatcher::getLeadCodePoints() {
    if (fLocalLeadCodePoints.isNull()) {
        ErrorCode status;
        auto* leadCodePoints = new UnicodeSet();
        uprv_currencyLeads(fLocaleName.data(), *leadCodePoints, status);
        // Always apply case mapping closure for currencies
        leadCodePoints->closeOver(USET_ADD_CASE_MAPPINGS);
        leadCodePoints->freeze();
        fLocalLeadCodePoints.adoptInstead(leadCodePoints);
    }
    return *fLocalLeadCodePoints;
}


CurrencyCustomMatcher::CurrencyCustomMatcher(const char16_t* currencyCode, const UnicodeString& currency1,
                                             const UnicodeString& currency2)
        : fCurrency1(currency1), fCurrency2(currency2) {
    utils::copyCurrencyCode(fCurrencyCode, currencyCode);
}

bool CurrencyCustomMatcher::match(StringSegment& segment, ParsedNumber& result, UErrorCode&) const {
    if (result.currencyCode[0] != 0) {
        return false;
    }

    int overlap1 = segment.getCommonPrefixLength(fCurrency1);
    if (overlap1 == fCurrency1.length()) {
        utils::copyCurrencyCode(result.currencyCode, fCurrencyCode);
        segment.adjustOffset(overlap1);
        result.setCharsConsumed(segment);
    }

    int overlap2 = segment.getCommonPrefixLength(fCurrency2);
    if (overlap2 == fCurrency2.length()) {
        utils::copyCurrencyCode(result.currencyCode, fCurrencyCode);
        segment.adjustOffset(overlap2);
        result.setCharsConsumed(segment);
    }

    return overlap1 == segment.length() || overlap2 == segment.length();
}

const UnicodeSet& CurrencyCustomMatcher::getLeadCodePoints() {
    if (fLocalLeadCodePoints.isNull()) {
        auto* leadCodePoints = new UnicodeSet();
        utils::putLeadCodePoint(fCurrency1, leadCodePoints);
        utils::putLeadCodePoint(fCurrency2, leadCodePoints);
        leadCodePoints->freeze();
        fLocalLeadCodePoints.adoptInstead(leadCodePoints);
    }
    return *fLocalLeadCodePoints;
}


CurrencyAnyMatcher::CurrencyAnyMatcher() {
    fMatcherArray[0] = &fNamesMatcher;
    fMatcherArray[1] = &fCustomMatcher;
}

CurrencyAnyMatcher::CurrencyAnyMatcher(CurrencyNamesMatcher namesMatcher,
                                       CurrencyCustomMatcher customMatcher)
        : fNamesMatcher(std::move(namesMatcher)), fCustomMatcher(std::move(customMatcher)) {
    fMatcherArray[0] = &fNamesMatcher;
    fMatcherArray[1] = &fCustomMatcher;
}

const UnicodeSet& CurrencyAnyMatcher::getLeadCodePoints() {
    if (fLocalLeadCodePoints.isNull()) {
        auto* leadCodePoints = new UnicodeSet();
        leadCodePoints->addAll(fNamesMatcher.getLeadCodePoints());
        leadCodePoints->addAll(fCustomMatcher.getLeadCodePoints());
        leadCodePoints->freeze();
        fLocalLeadCodePoints.adoptInstead(leadCodePoints);
    }
    return *fLocalLeadCodePoints;
}

const NumberParseMatcher* const* CurrencyAnyMatcher::begin() const {
    return fMatcherArray;
}

const NumberParseMatcher* const* CurrencyAnyMatcher::end() const {
    return fMatcherArray + 2;
}


#endif /* #if !UCONFIG_NO_FORMATTING */
