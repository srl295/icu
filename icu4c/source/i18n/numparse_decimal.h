// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT
#ifndef __NUMPARSE_DECIMAL_H__
#define __NUMPARSE_DECIMAL_H__

#include "unicode/uniset.h"
#include "numparse_types.h"

U_NAMESPACE_BEGIN namespace numparse {
namespace impl {

using ::icu::number::impl::Grouper;

class DecimalMatcher : public NumberParseMatcher, public UMemory {
  public:
    DecimalMatcher() = default;  // WARNING: Leaves the object in an unusable state

    DecimalMatcher(const DecimalFormatSymbols& symbols, const Grouper& grouper,
                   parse_flags_t parseFlags);

    bool match(StringSegment& segment, ParsedNumber& result, UErrorCode& status) const override;

    bool
    match(StringSegment& segment, ParsedNumber& result, int8_t exponentSign, UErrorCode& status) const;

    const UnicodeSet* getLeadCodePoints() const override;

  private:
    /** If true, only accept strings whose grouping sizes match the locale */
    bool requireGroupingMatch;

    /** If true, do not accept grouping separators at all */
    bool groupingDisabled;

    /** If true, do not accept fraction grouping separators */
    bool fractionGroupingDisabled;

    /** If true, do not accept numbers in the fraction */
    bool integerOnly;

    int16_t grouping1;
    int16_t grouping2;

    UnicodeString groupingSeparator;
    UnicodeString decimalSeparator;

    // Assumption: these sets all consist of single code points. If this assumption needs to be broken,
    // fix getLeadCodePoints() as well as matching logic. Be careful of the performance impact.
    const UnicodeSet* groupingUniSet;
    const UnicodeSet* decimalUniSet;
    const UnicodeSet* separatorSet;
    const UnicodeSet* leadSet;

    // Make this class the owner of a few objects that could be allocated.
    // The first two LocalPointers are used for assigning ownership only.
    LocalPointer<const UnicodeSet> fLocalDecimalUniSet;
    LocalPointer<const UnicodeSet> fLocalSeparatorSet;
    LocalArray<const UnicodeString> fLocalDigitStrings;
};


} // namespace impl
} // namespace numparse
U_NAMESPACE_END

#endif //__NUMPARSE_DECIMAL_H__
#endif /* #if !UCONFIG_NO_FORMATTING */
