// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT
#ifndef __NUMPARSE_SCIENTIFIC_H__
#define __NUMPARSE_SCIENTIFIC_H__

#include "numparse_types.h"
#include "numparse_decimal.h"
#include "unicode/numberformatter.h"

using icu::number::impl::Grouper;

U_NAMESPACE_BEGIN namespace numparse {
namespace impl {


class ScientificMatcher : public NumberParseMatcher, public UMemory {
  public:
    ScientificMatcher() = default;  // WARNING: Leaves the object in an unusable state

    ScientificMatcher(const DecimalFormatSymbols& dfs, const Grouper& grouper);

    bool match(StringSegment& segment, ParsedNumber& result, UErrorCode& status) const override;

    const UnicodeSet& getLeadCodePoints() override;

    UnicodeString toString() const override;

  private:
    UnicodeString fExponentSeparatorString;
    DecimalMatcher fExponentMatcher;
};


} // namespace impl
} // namespace numparse
U_NAMESPACE_END

#endif //__NUMPARSE_SCIENTIFIC_H__
#endif /* #if !UCONFIG_NO_FORMATTING */
