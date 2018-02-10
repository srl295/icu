// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT

#include "numparse_types.h"
#include "numparse_symbols.h"
#include "numparse_utils.h"

using namespace icu;
using namespace icu::numparse;
using namespace icu::numparse::impl;


SymbolMatcher::SymbolMatcher(const UnicodeString& symbolString, unisets::Key key) {
    fUniSet = unisets::get(key);
    if (fUniSet->contains(symbolString)) {
        fString.setToBogus();
    } else {
        fString = symbolString;
    }
}

const UnicodeSet* SymbolMatcher::getSet() const {
    return fUniSet;
}

bool SymbolMatcher::match(StringSegment& segment, ParsedNumber& result, UErrorCode&) const {
    // Smoke test first; this matcher might be disabled.
    if (isDisabled(result)) {
        return false;
    }

    // Test the string first in order to consume trailing chars greedily.
    int overlap = 0;
    if (!fString.isEmpty()) {
        overlap = segment.getCommonPrefixLength(fString);
        if (overlap == fString.length()) {
            segment.adjustOffset(fString.length());
            accept(segment, result);
            return false;
        }
    }

    int cp = segment.getCodePoint();
    if (cp != -1 && fUniSet->contains(cp)) {
        segment.adjustOffset(U16_LENGTH(cp));
        accept(segment, result);
        return false;
    }

    return overlap == segment.length();
}

const UnicodeSet& SymbolMatcher::getLeadCodePoints() {
    if (fString.isEmpty()) {
        // Assumption: for sets from UnicodeSetStaticCache, uniSet == leadCodePoints.
        return *fUniSet;
    }

    if (fLocalLeadCodePoints.isNull()) {
        auto* leadCodePoints = new UnicodeSet();
        utils::putLeadCodePoints(fUniSet, leadCodePoints);
        utils::putLeadCodePoint(fString, leadCodePoints);
        leadCodePoints->freeze();
        fLocalLeadCodePoints.adoptInstead(leadCodePoints);
    }
    return *fLocalLeadCodePoints;
}


IgnorablesMatcher::IgnorablesMatcher(unisets::Key key)
        : SymbolMatcher({}, key) {
}

bool IgnorablesMatcher::isFlexible() const {
    return true;
}

bool IgnorablesMatcher::isDisabled(const ParsedNumber&) const {
    return false;
}

void IgnorablesMatcher::accept(StringSegment&, ParsedNumber&) const {
    // No-op
}


InfinityMatcher::InfinityMatcher(const DecimalFormatSymbols& dfs)
        : SymbolMatcher(dfs.getConstSymbol(DecimalFormatSymbols::kInfinitySymbol), unisets::INFINITY) {
}

bool InfinityMatcher::isDisabled(const ParsedNumber& result) const {
    return 0 != (result.flags & FLAG_INFINITY);
}

void InfinityMatcher::accept(StringSegment& segment, ParsedNumber& result) const {
    result.flags |= FLAG_INFINITY;
    result.setCharsConsumed(segment);
}


MinusSignMatcher::MinusSignMatcher(const DecimalFormatSymbols& dfs, bool allowTrailing)
        : SymbolMatcher(dfs.getConstSymbol(DecimalFormatSymbols::kMinusSignSymbol), unisets::MINUS_SIGN),
          fAllowTrailing(allowTrailing) {
}

bool MinusSignMatcher::isDisabled(const ParsedNumber& result) const {
    return 0 != (result.flags & FLAG_NEGATIVE) || (fAllowTrailing ? false : result.seenNumber());
}

void MinusSignMatcher::accept(StringSegment& segment, ParsedNumber& result) const {
    result.flags |= FLAG_NEGATIVE;
    result.setCharsConsumed(segment);
}


NanMatcher::NanMatcher(const DecimalFormatSymbols& dfs)
        : SymbolMatcher(dfs.getConstSymbol(DecimalFormatSymbols::kNaNSymbol), unisets::EMPTY) {
}

const UnicodeSet& NanMatcher::getLeadCodePoints() {
    // Overriding this here to allow use of statically allocated sets
    int leadCp = fString.char32At(0);
    const UnicodeSet* s = unisets::get(unisets::NAN_LEAD);
    if (s->contains(leadCp)) {
        return *s;
    }

    return SymbolMatcher::getLeadCodePoints();
}

bool NanMatcher::isDisabled(const ParsedNumber& result) const {
    return result.seenNumber();
}

void NanMatcher::accept(StringSegment& segment, ParsedNumber& result) const {
    result.flags |= FLAG_NAN;
    result.setCharsConsumed(segment);
}


PaddingMatcher::PaddingMatcher(const UnicodeString& padString)
        : SymbolMatcher(padString, unisets::EMPTY) {}

bool PaddingMatcher::isFlexible() const {
    return true;
}

bool PaddingMatcher::isDisabled(const ParsedNumber&) const {
    return false;
}

void PaddingMatcher::accept(StringSegment&, ParsedNumber&) const {
    // No-op
}


PercentMatcher::PercentMatcher(const DecimalFormatSymbols& dfs)
        : SymbolMatcher(dfs.getConstSymbol(DecimalFormatSymbols::kPercentSymbol), unisets::PERCENT_SIGN) {
}

void PercentMatcher::postProcess(ParsedNumber& result) const {
    SymbolMatcher::postProcess(result);
    if (0 != (result.flags & FLAG_PERCENT) && !result.quantity.bogus) {
        result.quantity.adjustMagnitude(-2);
    }
}

bool PercentMatcher::isDisabled(const ParsedNumber& result) const {
    return 0 != (result.flags & FLAG_PERCENT);
}

void PercentMatcher::accept(StringSegment& segment, ParsedNumber& result) const {
    result.flags |= FLAG_PERCENT;
    result.setCharsConsumed(segment);
}


PermilleMatcher::PermilleMatcher(const DecimalFormatSymbols& dfs)
        : SymbolMatcher(dfs.getConstSymbol(DecimalFormatSymbols::kPerMillSymbol), unisets::PERMILLE_SIGN) {
}

void PermilleMatcher::postProcess(ParsedNumber& result) const {
    SymbolMatcher::postProcess(result);
    if (0 != (result.flags & FLAG_PERMILLE) && !result.quantity.bogus) {
        result.quantity.adjustMagnitude(-3);
    }
}

bool PermilleMatcher::isDisabled(const ParsedNumber& result) const {
    return 0 != (result.flags & FLAG_PERMILLE);
}

void PermilleMatcher::accept(StringSegment& segment, ParsedNumber& result) const {
    result.flags |= FLAG_PERMILLE;
    result.setCharsConsumed(segment);
}


PlusSignMatcher::PlusSignMatcher(const DecimalFormatSymbols& dfs, bool allowTrailing)
        : SymbolMatcher(dfs.getConstSymbol(DecimalFormatSymbols::kPlusSignSymbol), unisets::PLUS_SIGN),
          fAllowTrailing(allowTrailing) {
}

bool PlusSignMatcher::isDisabled(const ParsedNumber& result) const {
    return fAllowTrailing ? false : result.seenNumber();
}

void PlusSignMatcher::accept(StringSegment& segment, ParsedNumber& result) const {
    result.setCharsConsumed(segment);
}


#endif /* #if !UCONFIG_NO_FORMATTING */
