// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT

#include "numparse_types.h"
#include "numparse_decimal.h"
#include "numparse_unisets.h"
#include "numparse_utils.h"
#include "unicode/uchar.h"

using namespace icu;
using namespace icu::numparse;
using namespace icu::numparse::impl;


DecimalMatcher::DecimalMatcher(const DecimalFormatSymbols& symbols, const Grouper& grouper,
                               parse_flags_t parseFlags) {
    if (0 != (parseFlags & PARSE_FLAG_MONETARY_SEPARATORS)) {
        groupingSeparator = symbols.getConstSymbol(DecimalFormatSymbols::kMonetaryGroupingSeparatorSymbol);
        decimalSeparator = symbols.getConstSymbol(DecimalFormatSymbols::kMonetarySeparatorSymbol);
    } else {
        groupingSeparator = symbols.getConstSymbol(DecimalFormatSymbols::kGroupingSeparatorSymbol);
        decimalSeparator = symbols.getConstSymbol(DecimalFormatSymbols::kDecimalSeparatorSymbol);
    }
    bool strictSeparators = 0 != (parseFlags & PARSE_FLAG_STRICT_SEPARATORS);
    unisets::Key groupingKey = strictSeparators ? unisets::STRICT_ALL_SEPARATORS
                                                : unisets::ALL_SEPARATORS;

    // Attempt to find separators in the static cache

    groupingUniSet = unisets::get(groupingKey);
    unisets::Key decimalKey = unisets::chooseFrom(
            decimalSeparator,
            strictSeparators ? unisets::STRICT_COMMA : unisets::COMMA,
            strictSeparators ? unisets::STRICT_PERIOD : unisets::PERIOD);
    if (decimalKey != unisets::COUNT) {
        decimalUniSet = unisets::get(decimalKey);
    } else {
        auto* set = new UnicodeSet();
        set->add(decimalSeparator.char32At(0));
        set->freeze();
        decimalUniSet = set;
        fLocalDecimalUniSet.adoptInstead(set);
    }

    if (groupingKey != unisets::COUNT && decimalKey != unisets::COUNT) {
        // Everything is available in the static cache
        separatorSet = groupingUniSet;
        leadSet = unisets::get(
                strictSeparators ? unisets::DIGITS_OR_ALL_SEPARATORS
                                 : unisets::DIGITS_OR_STRICT_ALL_SEPARATORS);
    } else {
        auto* set = new UnicodeSet();
        set->addAll(*groupingUniSet);
        set->addAll(*decimalUniSet);
        set->freeze();
        separatorSet = set;
        fLocalSeparatorSet.adoptInstead(set);
        leadSet = nullptr;
    }

    int cpZero = symbols.getCodePointZero();
    if (cpZero == -1 || !u_isdigit(cpZero) || u_digit(cpZero, 10) != 0) {
        // Uncommon case: okay to allocate.
        auto digitStrings = new UnicodeString[10];
        fLocalDigitStrings.adoptInstead(digitStrings);
        for (int32_t i = 0; i <= 9; i++) {
            digitStrings[i] = symbols.getConstDigitSymbol(i);
        }
    }

    requireGroupingMatch = 0 != (parseFlags & PARSE_FLAG_STRICT_GROUPING_SIZE);
    groupingDisabled = 0 != (parseFlags & PARSE_FLAG_GROUPING_DISABLED);
    fractionGroupingDisabled = 0 != (
            parseFlags & PARSE_FLAG_FRACTION_GROUPING_DISABLED);
    integerOnly = 0 != (parseFlags & PARSE_FLAG_INTEGER_ONLY);
    grouping1 = grouper.getPrimary();
    grouping2 = grouper.getSecondary();
}

bool DecimalMatcher::match(StringSegment& segment, ParsedNumber& result, UErrorCode& status) const {
    return match(segment, result, 0, status);
}

bool DecimalMatcher::match(StringSegment& segment, ParsedNumber& result, int8_t exponentSign,
                           UErrorCode&) const {
    if (result.seenNumber() && exponentSign == 0) {
        // A number has already been consumed.
        return false;
    } else if (exponentSign != 0) {
        // scientific notation always comes after the number
        U_ASSERT(!result.quantity.bogus);
    }

    ParsedNumber backupResult(result);

    // strict parsing
    bool strictFail = false; // did we exit with a strict parse failure?
    UnicodeString actualGroupingString = groupingSeparator;
    UnicodeString actualDecimalString = decimalSeparator;
    int32_t groupedDigitCount = 0; // tracking count of digits delimited by grouping separator
    int32_t backupOffset = -1; // used for preserving the last confirmed position
    bool afterFirstGrouping = false;
    bool seenGrouping = false;
    bool seenDecimal = false;
    int32_t digitsAfterDecimal = 0;
    int32_t initialOffset = segment.getOffset();
    int32_t exponent = 0;
    bool hasPartialPrefix = false;
    while (segment.length() > 0) {
        hasPartialPrefix = false;

        // Attempt to match a digit.
        int8_t digit = -1;

        // Try by code point digit value.
        int cp = segment.getCodePoint();
        if (u_isdigit(cp)) {
            segment.adjustOffset(U16_LENGTH(cp));
            digit = static_cast<int8_t>(u_digit(cp, 10));
        }

        // Try by digit string.
        if (digit == -1 && !fLocalDigitStrings.isNull()) {
            for (int i = 0; i < 10; i++) {
                const UnicodeString& str = fLocalDigitStrings[i];
                int overlap = segment.getCommonPrefixLength(str);
                if (overlap == str.length()) {
                    segment.adjustOffset(overlap);
                    digit = static_cast<int8_t>(i);
                    break;
                } else if (overlap == segment.length()) {
                    hasPartialPrefix = true;
                }
            }
        }

        if (digit >= 0) {
            // Digit was found.
            // Check for grouping size violation
            if (backupOffset != -1) {
                if (requireGroupingMatch) {
                    // comma followed by digit, so group before comma is a secondary
                    // group. If there was a group separator before that, the group
                    // must == the secondary group length, else it can be <= the the
                    // secondary group length.
                    if ((afterFirstGrouping && groupedDigitCount != grouping2) ||
                        (!afterFirstGrouping && groupedDigitCount > grouping2)) {
                        strictFail = true;
                        break;
                    }
                }
                afterFirstGrouping = true;
                backupOffset = -1;
                groupedDigitCount = 0;
            }

            // Save the digit in the DecimalQuantity or scientific adjustment.
            if (exponentSign != 0) {
                int nextExponent = digit + exponent * 10;
                if (nextExponent < exponent) {
                    // Overflow
                    exponent = INT32_MAX;
                } else {
                    exponent = nextExponent;
                }
            } else {
                if (result.quantity.bogus) {
                    result.quantity.bogus = false;
                }
                result.quantity.appendDigit(digit, 0, true);
            }
            result.setCharsConsumed(segment);
            groupedDigitCount++;
            if (seenDecimal) {
                digitsAfterDecimal++;
            }
            continue;
        }

        // Attempt to match a literal grouping or decimal separator
        int32_t decimalOverlap = segment.getCommonPrefixLength(actualDecimalString);
        bool decimalStringMatch = decimalOverlap == actualDecimalString.length();
        int32_t groupingOverlap = segment.getCommonPrefixLength(actualGroupingString);
        bool groupingStringMatch = groupingOverlap == actualGroupingString.length();

        hasPartialPrefix = (decimalOverlap == segment.length()) || (groupingOverlap == segment.length());

        if (!seenDecimal && !groupingStringMatch &&
            (decimalStringMatch || (!seenDecimal && decimalUniSet->contains(cp)))) {
            // matched a decimal separator
            if (requireGroupingMatch) {
                if (backupOffset != -1 || (seenGrouping && groupedDigitCount != grouping1)) {
                    strictFail = true;
                    break;
                }
            }

            // If we're only parsing integers, then don't parse this one.
            if (integerOnly) {
                break;
            }

            seenDecimal = true;
            if (!decimalStringMatch) {
                actualDecimalString = UnicodeString(cp);
            }
            segment.adjustOffset(actualDecimalString.length());
            result.setCharsConsumed(segment);
            result.flags |= FLAG_HAS_DECIMAL_SEPARATOR;
            continue;
        }

        if (!groupingDisabled && !decimalStringMatch &&
            (groupingStringMatch || (!seenGrouping && groupingUniSet->contains(cp)))) {
            // matched a grouping separator
            if (requireGroupingMatch) {
                if (groupedDigitCount == 0) {
                    // leading group
                    strictFail = true;
                    break;
                } else if (backupOffset != -1) {
                    // two group separators in a row
                    break;
                }
            }

            if (fractionGroupingDisabled && seenDecimal) {
                // Stop parsing here.
                break;
            }

            seenGrouping = true;
            if (!groupingStringMatch) {
                actualGroupingString = UnicodeString(cp);
            }
            backupOffset = segment.getOffset();
            segment.adjustOffset(actualGroupingString.length());
            // Note: do NOT set charsConsumed
            continue;
        }

        // Not a digit and not a separator
        break;
    }

    // Check the final grouping for validity
    if (requireGroupingMatch && !seenDecimal && seenGrouping && afterFirstGrouping &&
        groupedDigitCount != grouping1) {
        strictFail = true;
    }

    if (requireGroupingMatch && strictFail) {
        result = backupResult;
        segment.setOffset(initialOffset);
    }

    if (result.quantity.bogus && segment.getOffset() != initialOffset) {
        // Strings that start with a separator but have no digits.
        // We don't need a backup of ParsedNumber because no changes could have been made to it.
        segment.setOffset(initialOffset);
        hasPartialPrefix = true;
    }

    if (!result.quantity.bogus) {
        // The final separator was a decimal separator.
        result.quantity.adjustMagnitude(-digitsAfterDecimal);
    }

    if (exponentSign != 0 && segment.getOffset() != initialOffset) {
        U_ASSERT(!result.quantity.bogus);
        bool overflow = (exponent == INT32_MAX);
        if (!overflow) {
            result.quantity.adjustMagnitude(exponentSign * exponent);
        }
        if (overflow) {
            if (exponentSign == -1) {
                // Set to zero
                result.quantity.clear();
            } else {
                // Set to infinity
                result.quantity.bogus = true;
                result.flags |= FLAG_INFINITY;
            }
        }
    }

    return segment.length() == 0 || hasPartialPrefix;
}

const UnicodeSet* DecimalMatcher::getLeadCodePoints() const {
    if (fLocalDigitStrings.isNull() && leadSet != nullptr) {
        return new UnicodeSet(*leadSet);
    }

    auto* leadCodePoints = new UnicodeSet();
    // Assumption: the sets are all single code points.
    leadCodePoints->addAll(*unisets::get(unisets::DIGITS));
    leadCodePoints->addAll(*separatorSet);
    if (!fLocalDigitStrings.isNull()) {
        for (int i = 0; i < 10; i++) {
            utils::putLeadCodePoint(fLocalDigitStrings[i], leadCodePoints);
        }
    }
    leadCodePoints->freeze();
    return leadCodePoints;
}


#endif /* #if !UCONFIG_NO_FORMATTING */
