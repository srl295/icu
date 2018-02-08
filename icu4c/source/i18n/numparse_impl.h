// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT
#ifndef __NUMPARSE_IMPL_H__
#define __NUMPARSE_IMPL_H__

#include "numparse_types.h"
#include "unicode/uniset.h"

U_NAMESPACE_BEGIN namespace numparse {
namespace impl {

class NumberParserImpl {
  public:
    static NumberParserImpl* createSimpleParser(const Locale& locale, const UnicodeString& patternString,
                                                parse_flags_t parseFlags, UErrorCode& status);

    void addAndAdoptMatcher(const NumberParseMatcher* matcher);

    void freeze();

    void parse(const UnicodeString& input, bool greedy, ParsedNumber& result, UErrorCode& status) const;

    void parse(const UnicodeString& input, int32_t start, bool greedy, ParsedNumber& result,
               UErrorCode& status) const;

    UnicodeString toString() const;

  private:
    parse_flags_t fParseFlags;
    int32_t fNumMatchers = 0;
    // NOTE: The stack capacity for fMatchers and fLeads should be the same
    MaybeStackArray<const NumberParseMatcher*, 10> fMatchers;
    MaybeStackArray<const UnicodeSet*, 10> fLeads;
    bool fComputeLeads;
    bool fFrozen = false;

    NumberParserImpl(parse_flags_t parseFlags, bool computeLeads);

    ~NumberParserImpl();

    void parseGreedyRecursive(StringSegment& segment, ParsedNumber& result, UErrorCode& status) const;

    void parseLongestRecursive(StringSegment& segment, ParsedNumber& result, UErrorCode& status) const;
};


} // namespace impl
} // namespace numparse
U_NAMESPACE_END

#endif //__NUMPARSE_IMPL_H__
#endif /* #if !UCONFIG_NO_FORMATTING */
