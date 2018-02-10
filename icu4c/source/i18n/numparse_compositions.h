// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT
#ifndef __SOURCE_NUMPARSE_COMPOSITIONS__
#define __SOURCE_NUMPARSE_COMPOSITIONS__

#include "numparse_types.h"

U_NAMESPACE_BEGIN namespace numparse {
namespace impl {


/**
 * Base class for AnyMatcher and SeriesMatcher.
 */
class CompositionMatcher : public NumberParseMatcher {
  protected:
    // No construction except by subclasses!
    CompositionMatcher() = default;

    // To be overridden by subclasses (used for iteration):
    virtual const NumberParseMatcher* const* begin() const = 0;

    // To be overridden by subclasses (used for iteration):
    virtual const NumberParseMatcher* const* end() const = 0;
};


/**
 * Composes a number of matchers, and succeeds if any of the matchers succeed. Always greedily chooses
 * the first matcher in the list to succeed.
 *
 * NOTE: In C++, this is a base class, unlike ICU4J, which uses a factory-style interface.
 *
 * @author sffc
 * @see SeriesMatcher
 */
class AnyMatcher : public CompositionMatcher {
  public:
    bool match(StringSegment& segment, ParsedNumber& result, UErrorCode& status) const override;

    void postProcess(ParsedNumber& result) const override;

  protected:
    // No construction except by subclasses!
    AnyMatcher() = default;
};


/**
 * Composes a number of matchers, running one after another. Matches the input string only if all of the
 * matchers in the series succeed. Performs greedy matches within the context of the series.
 *
 * @author sffc
 * @see AnyMatcher
 */
class SeriesMatcher : public CompositionMatcher {
  public:
    bool match(StringSegment& segment, ParsedNumber& result, UErrorCode& status) const override;

    void postProcess(ParsedNumber& result) const override;

  protected:
    // No construction except by subclasses!
    SeriesMatcher() = default;
};


/**
 * An implementation of SeriesMatcher that references an array of matchers.
 *
 * The object adopts the array, but NOT the matchers contained inside the array.
 */
class ArraySeriesMatcher : public SeriesMatcher {
  public:
    /** The array is adopted, but NOT the matchers inside the array. */
    ArraySeriesMatcher(NumberParseMatcher** matchers, int32_t matchersLen);

    const UnicodeSet& getLeadCodePoints() override;

  protected:
    const NumberParseMatcher* const* begin() const override;

    const NumberParseMatcher* const* end() const override;

  private:
    LocalArray<NumberParseMatcher*> fMatchers;
    int32_t fMatchersLen;
};


} // namespace impl
} // namespace numparse
U_NAMESPACE_END

#endif //__SOURCE_NUMPARSE_COMPOSITIONS__
#endif /* #if !UCONFIG_NO_FORMATTING */
