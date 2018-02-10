// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT

// Allow implicit conversion from char16_t* to UnicodeString for this file
#define UNISTR_FROM_STRING_EXPLICIT

#include "number_types.h"
#include "number_patternstring.h"
#include "numparse_types.h"
#include "numparse_impl.h"
#include "numparse_symbols.h"
#include "numparse_decimal.h"
#include "unicode/numberformatter.h"

#include <typeinfo>

using namespace icu;
using namespace icu::number;
using namespace icu::number::impl;
using namespace icu::numparse;
using namespace icu::numparse::impl;


NumberParserImpl*
NumberParserImpl::createSimpleParser(const Locale& locale, const UnicodeString& patternString,
                                     parse_flags_t parseFlags, UErrorCode& status) {

    auto* parser = new NumberParserImpl(parseFlags, true);
    DecimalFormatSymbols symbols(locale, status);

    parser->fLocalMatchers.ignorables = std::move(IgnorablesMatcher(unisets::DEFAULT_IGNORABLES));

//    MatcherFactory factory = new MatcherFactory();
//    factory.currency = Currency.getInstance("USD");
//    factory.symbols = symbols;
//    factory.ignorables = ignorables;
//    factory.locale = locale;
//    factory.parseFlags = parseFlags;

    ParsedPatternInfo patternInfo;
    PatternParser::parseToPatternInfo(patternString, patternInfo, status);
//    AffixMatcher.createMatchers(patternInfo, parser, factory, ignorables, parseFlags);

    Grouper grouper = Grouper::forStrategy(UNUM_GROUPING_AUTO);
    grouper.setLocaleData(patternInfo, locale);

    parser->addMatcher(parser->fLocalMatchers.ignorables);
    parser->addMatcher(parser->fLocalMatchers.decimal = {symbols, grouper, parseFlags});
    parser->addMatcher(parser->fLocalMatchers.minusSign = {symbols, false});
    parser->addMatcher(parser->fLocalMatchers.plusSign = {symbols, false});
    parser->addMatcher(parser->fLocalMatchers.percent = {symbols});
    parser->addMatcher(parser->fLocalMatchers.permille = {symbols});
    parser->addMatcher(parser->fLocalMatchers.nan = {symbols});
    parser->addMatcher(parser->fLocalMatchers.infinity = {symbols});
    parser->addMatcher(parser->fLocalMatchers.padding = {u"@"});
    parser->addMatcher(parser->fLocalMatchers.scientific = {symbols, grouper});
    parser->addMatcher(parser->fLocalMatchers.currencyNames = {locale, status});
//    parser.addMatcher(new RequireNumberMatcher());

    parser->freeze();
    return parser;
}

NumberParserImpl::NumberParserImpl(parse_flags_t parseFlags, bool computeLeads)
        : fParseFlags(parseFlags), fComputeLeads(computeLeads) {
}

NumberParserImpl::~NumberParserImpl() {
    if (fComputeLeads) {
        for (int32_t i = 0; i < fNumMatchers; i++) {
            delete (fLeads[i]);
        }
    }
    fNumMatchers = 0;
}

void NumberParserImpl::addMatcher(NumberParseMatcher& matcher) {
    if (fNumMatchers + 1 > fMatchers.getCapacity()) {
        fMatchers.resize(fNumMatchers * 2, fNumMatchers);
        if (fComputeLeads) {
            // The two arrays should grow in tandem:
            U_ASSERT(fNumMatchers >= fLeads.getCapacity());
            fLeads.resize(fNumMatchers * 2, fNumMatchers);
        }
    }

    fMatchers[fNumMatchers] = &matcher;

    if (fComputeLeads) {
        addLeadCodePointsForMatcher(matcher);
    }

    fNumMatchers++;
}

void NumberParserImpl::addLeadCodePointsForMatcher(NumberParseMatcher& matcher) {
    const UnicodeSet& leadCodePoints = matcher.getLeadCodePoints();
    // TODO: Avoid the clone operation here.
    if (0 != (fParseFlags & PARSE_FLAG_IGNORE_CASE)) {
        auto* copy = dynamic_cast<UnicodeSet*>(leadCodePoints.cloneAsThawed());
        copy->closeOver(USET_ADD_CASE_MAPPINGS);
        copy->freeze();
        fLeads[fNumMatchers] = copy;
    } else {
        // FIXME: new here because we still take ownership
        fLeads[fNumMatchers] = new UnicodeSet(leadCodePoints);
    }
}

void NumberParserImpl::freeze() {
    fFrozen = true;
}

void NumberParserImpl::parse(const UnicodeString& input, bool greedy, ParsedNumber& result,
                             UErrorCode& status) const {
    return parse(input, 0, greedy, result, status);
}

void NumberParserImpl::parse(const UnicodeString& input, int32_t start, bool greedy, ParsedNumber& result,
                             UErrorCode& status) const {
    U_ASSERT(fFrozen);
    // TODO: Check start >= 0 and start < input.length()
    StringSegment segment(input, fParseFlags);
    segment.adjustOffset(start);
    if (greedy) {
        parseGreedyRecursive(segment, result, status);
    } else {
        parseLongestRecursive(segment, result, status);
    }
    for (int32_t i = 0; i < fNumMatchers; i++) {
        fMatchers[i]->postProcess(result);
    }
}

void NumberParserImpl::parseGreedyRecursive(StringSegment& segment, ParsedNumber& result,
                                            UErrorCode& status) const {
    // Base Case
    if (segment.length() == 0) {
        return;
    }

    int initialOffset = segment.getOffset();
    int leadCp = segment.getCodePoint();
    for (int32_t i = 0; i < fNumMatchers; i++) {
        if (fComputeLeads && !fLeads[i]->contains(leadCp)) {
            continue;
        }
        const NumberParseMatcher* matcher = fMatchers[i];
        matcher->match(segment, result, status);
        if (U_FAILURE(status)) {
            return;
        }
        if (segment.getOffset() != initialOffset) {
            // In a greedy parse, recurse on only the first match.
            parseGreedyRecursive(segment, result, status);
            // The following line resets the offset so that the StringSegment says the same across
            // the function
            // call boundary. Since we recurse only once, this line is not strictly necessary.
            segment.setOffset(initialOffset);
            return;
        }
    }

    // NOTE: If we get here, the greedy parse completed without consuming the entire string.
}

void NumberParserImpl::parseLongestRecursive(StringSegment& segment, ParsedNumber& result,
                                             UErrorCode& status) const {
    // Base Case
    if (segment.length() == 0) {
        return;
    }

    // TODO: Give a nice way for the matcher to reset the ParsedNumber?
    ParsedNumber initial(result);
    ParsedNumber candidate;

    int initialOffset = segment.getOffset();
    for (int32_t i = 0; i < fNumMatchers; i++) {
        // TODO: Check leadChars here?
        const NumberParseMatcher* matcher = fMatchers[i];

        // In a non-greedy parse, we attempt all possible matches and pick the best.
        for (int32_t charsToConsume = 0; charsToConsume < segment.length();) {
            charsToConsume += U16_LENGTH(segment.codePointAt(charsToConsume));

            // Run the matcher on a segment of the current length.
            candidate = initial;
            segment.setLength(charsToConsume);
            bool maybeMore = matcher->match(segment, candidate, status);
            segment.resetLength();
            if (U_FAILURE(status)) {
                return;
            }

            // If the entire segment was consumed, recurse.
            if (segment.getOffset() - initialOffset == charsToConsume) {
                parseLongestRecursive(segment, candidate, status);
                if (U_FAILURE(status)) {
                    return;
                }
                if (candidate.isBetterThan(result)) {
                    result = candidate;
                }
            }

            // Since the segment can be re-used, reset the offset.
            // This does not have an effect if the matcher did not consume any chars.
            segment.setOffset(initialOffset);

            // Unless the matcher wants to see the next char, continue to the next matcher.
            if (!maybeMore) {
                break;
            }
        }
    }
}

UnicodeString NumberParserImpl::toString() const {
    UnicodeString result(u"<NumberParserImpl matchers:[");
    for (int32_t i = 0; i < fNumMatchers; i++) {
        result.append(u' ');
        result.append(UnicodeString(typeid(*fMatchers[i]).name()));
    }
    result.append(u" ]>", -1);
    return result;
}


#endif /* #if !UCONFIG_NO_FORMATTING */
