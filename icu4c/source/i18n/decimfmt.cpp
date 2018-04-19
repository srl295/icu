// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT

// Allow implicit conversion from char16_t* to UnicodeString for this file:
// Helpful in toString methods and elsewhere.
#define UNISTR_FROM_STRING_EXPLICIT

#include <cmath>
#include <stdlib.h>
#include "unicode/errorcode.h"
#include "unicode/decimfmt.h"
#include "number_decimalquantity.h"
#include "number_types.h"
#include "numparse_impl.h"
#include "number_mapper.h"
#include "number_patternstring.h"
#include "putilimp.h"
#include "number_utils.h"
#include "number_utypes.h"

using namespace icu;
using namespace icu::number;
using namespace icu::number::impl;
using namespace icu::numparse;
using namespace icu::numparse::impl;
using ERoundingMode = icu::DecimalFormat::ERoundingMode;
using EPadPosition = icu::DecimalFormat::EPadPosition;


UOBJECT_DEFINE_RTTI_IMPLEMENTATION(DecimalFormat)


DecimalFormat::DecimalFormat(UErrorCode& status)
        : DecimalFormat(nullptr, status) {
    // Use the default locale and decimal pattern.
    const char* localeName = Locale::getDefault().getName();
    LocalPointer<NumberingSystem> ns(NumberingSystem::createInstance(status));
    UnicodeString patternString = utils::getPatternForStyle(
            localeName,
            ns->getName(),
            CLDR_PATTERN_STYLE_DECIMAL,
            status);
    setPropertiesFromPattern(patternString, IGNORE_ROUNDING_IF_CURRENCY, status);
    touch(status);
}

DecimalFormat::DecimalFormat(const UnicodeString& pattern, UErrorCode& status)
        : DecimalFormat(nullptr, status) {
    setPropertiesFromPattern(pattern, IGNORE_ROUNDING_IF_CURRENCY, status);
    touch(status);
}

DecimalFormat::DecimalFormat(const UnicodeString& pattern, DecimalFormatSymbols* symbolsToAdopt,
                             UErrorCode& status)
        : DecimalFormat(symbolsToAdopt, status) {
    setPropertiesFromPattern(pattern, IGNORE_ROUNDING_IF_CURRENCY, status);
    touch(status);
}

DecimalFormat::DecimalFormat(const UnicodeString& pattern, DecimalFormatSymbols* symbolsToAdopt,
                             UNumberFormatStyle style, UErrorCode& status)
        : DecimalFormat(symbolsToAdopt, status) {
    // If choice is a currency type, ignore the rounding information.
    if (style == UNumberFormatStyle::UNUM_CURRENCY || style == UNumberFormatStyle::UNUM_CURRENCY_ISO ||
        style == UNumberFormatStyle::UNUM_CURRENCY_ACCOUNTING ||
        style == UNumberFormatStyle::UNUM_CASH_CURRENCY ||
        style == UNumberFormatStyle::UNUM_CURRENCY_STANDARD ||
        style == UNumberFormatStyle::UNUM_CURRENCY_PLURAL) {
        setPropertiesFromPattern(pattern, IGNORE_ROUNDING_ALWAYS, status);
    } else {
        setPropertiesFromPattern(pattern, IGNORE_ROUNDING_IF_CURRENCY, status);
    }
    // Note: in Java, CurrencyPluralInfo is set in NumberFormat.java, but in C++, it is not set there,
    // so we have to set it here.
    if (style == UNumberFormatStyle::UNUM_CURRENCY_PLURAL) {
        LocalPointer<CurrencyPluralInfo> cpi(
                new CurrencyPluralInfo(fSymbols->getLocale(), status),
                status);
        if (U_FAILURE(status)) { return; }
        fProperties->currencyPluralInfo.fPtr.adoptInstead(cpi.orphan());
    }
    touch(status);
}

DecimalFormat::DecimalFormat(const DecimalFormatSymbols* symbolsToAdopt, UErrorCode& status) {
    fProperties.adoptInsteadAndCheckErrorCode(new DecimalFormatProperties(), status);
    fExportedProperties.adoptInsteadAndCheckErrorCode(new DecimalFormatProperties(), status);
    fWarehouse.adoptInsteadAndCheckErrorCode(new DecimalFormatWarehouse(), status);
    if (symbolsToAdopt == nullptr) {
        fSymbols.adoptInsteadAndCheckErrorCode(new DecimalFormatSymbols(status), status);
    } else {
        fSymbols.adoptInsteadAndCheckErrorCode(symbolsToAdopt, status);
    }
}

#if UCONFIG_HAVE_PARSEALLINPUT

void DecimalFormat::setParseAllInput(UNumberFormatAttributeValue value) {
    fProperties->parseAllInput = value;
}

#endif

DecimalFormat&
DecimalFormat::setAttribute(UNumberFormatAttribute attr, int32_t newValue, UErrorCode& status) {
    if (U_FAILURE(status)) { return *this; }

    switch (attr) {
        case UNUM_LENIENT_PARSE:
            setLenient(newValue != 0);
            break;

        case UNUM_PARSE_INT_ONLY:
            setParseIntegerOnly(newValue != 0);
            break;

        case UNUM_GROUPING_USED:
            setGroupingUsed(newValue != 0);
            break;

        case UNUM_DECIMAL_ALWAYS_SHOWN:
            setDecimalSeparatorAlwaysShown(newValue != 0);
            break;

        case UNUM_MAX_INTEGER_DIGITS:
            setMaximumIntegerDigits(newValue);
            break;

        case UNUM_MIN_INTEGER_DIGITS:
            setMinimumIntegerDigits(newValue);
            break;

        case UNUM_INTEGER_DIGITS:
            setMinimumIntegerDigits(newValue);
            setMaximumIntegerDigits(newValue);
            break;

        case UNUM_MAX_FRACTION_DIGITS:
            setMaximumFractionDigits(newValue);
            break;

        case UNUM_MIN_FRACTION_DIGITS:
            setMinimumFractionDigits(newValue);
            break;

        case UNUM_FRACTION_DIGITS:
            setMinimumFractionDigits(newValue);
            setMaximumFractionDigits(newValue);
            break;

        case UNUM_SIGNIFICANT_DIGITS_USED:
            setSignificantDigitsUsed(newValue != 0);
            break;

        case UNUM_MAX_SIGNIFICANT_DIGITS:
            setMaximumSignificantDigits(newValue);
            break;

        case UNUM_MIN_SIGNIFICANT_DIGITS:
            setMinimumSignificantDigits(newValue);
            break;

        case UNUM_MULTIPLIER:
            setMultiplier(newValue);
            break;

        case UNUM_SCALE:
            setMultiplierScale(newValue);
            break;

        case UNUM_GROUPING_SIZE:
            setGroupingSize(newValue);
            break;

        case UNUM_ROUNDING_MODE:
            setRoundingMode((DecimalFormat::ERoundingMode) newValue);
            break;

        case UNUM_FORMAT_WIDTH:
            setFormatWidth(newValue);
            break;

        case UNUM_PADDING_POSITION:
            /** The position at which padding will take place. */
            setPadPosition((DecimalFormat::EPadPosition) newValue);
            break;

        case UNUM_SECONDARY_GROUPING_SIZE:
            setSecondaryGroupingSize(newValue);
            break;

#if UCONFIG_HAVE_PARSEALLINPUT
        case UNUM_PARSE_ALL_INPUT:
            setParseAllInput((UNumberFormatAttributeValue) newValue);
            break;
#endif

        case UNUM_PARSE_NO_EXPONENT:
            setParseNoExponent((UBool) newValue);
            break;

        case UNUM_PARSE_DECIMAL_MARK_REQUIRED:
            setDecimalPatternMatchRequired((UBool) newValue);
            break;

        case UNUM_CURRENCY_USAGE:
            setCurrencyUsage((UCurrencyUsage) newValue, &status);
            break;

        case UNUM_MINIMUM_GROUPING_DIGITS:
            setMinimumGroupingDigits(newValue);
            break;

        case UNUM_PARSE_CASE_SENSITIVE:
            setParseCaseSensitive(static_cast<UBool>(newValue));
            break;

        case UNUM_SIGN_ALWAYS_SHOWN:
            setSignAlwaysShown(static_cast<UBool>(newValue));
            break;

        case UNUM_FORMAT_FAIL_IF_MORE_THAN_MAX_DIGITS:
            setFormatFailIfMoreThanMaxDigits(static_cast<UBool>(newValue));
            break;

        default:
            status = U_UNSUPPORTED_ERROR;
            break;
    }
    return *this;
}

int32_t DecimalFormat::getAttribute(UNumberFormatAttribute attr, UErrorCode& status) const {
    if (U_FAILURE(status)) { return -1; }
    switch (attr) {
        case UNUM_LENIENT_PARSE:
            return isLenient();

        case UNUM_PARSE_INT_ONLY:
            return isParseIntegerOnly();

        case UNUM_GROUPING_USED:
            return isGroupingUsed();

        case UNUM_DECIMAL_ALWAYS_SHOWN:
            return isDecimalSeparatorAlwaysShown();

        case UNUM_MAX_INTEGER_DIGITS:
            return getMaximumIntegerDigits();

        case UNUM_MIN_INTEGER_DIGITS:
            return getMinimumIntegerDigits();

        case UNUM_INTEGER_DIGITS:
            // TBD: what should this return?
            return getMinimumIntegerDigits();

        case UNUM_MAX_FRACTION_DIGITS:
            return getMaximumFractionDigits();

        case UNUM_MIN_FRACTION_DIGITS:
            return getMinimumFractionDigits();

        case UNUM_FRACTION_DIGITS:
            // TBD: what should this return?
            return getMinimumFractionDigits();

        case UNUM_SIGNIFICANT_DIGITS_USED:
            return areSignificantDigitsUsed();

        case UNUM_MAX_SIGNIFICANT_DIGITS:
            return getMaximumSignificantDigits();

        case UNUM_MIN_SIGNIFICANT_DIGITS:
            return getMinimumSignificantDigits();

        case UNUM_MULTIPLIER:
            return getMultiplier();

        case UNUM_SCALE:
            return getMultiplierScale();

        case UNUM_GROUPING_SIZE:
            return getGroupingSize();

        case UNUM_ROUNDING_MODE:
            return getRoundingMode();

        case UNUM_FORMAT_WIDTH:
            return getFormatWidth();

        case UNUM_PADDING_POSITION:
            return getPadPosition();

        case UNUM_SECONDARY_GROUPING_SIZE:
            return getSecondaryGroupingSize();

        case UNUM_PARSE_NO_EXPONENT:
            return isParseNoExponent();

        case UNUM_PARSE_DECIMAL_MARK_REQUIRED:
            return isDecimalPatternMatchRequired();

        case UNUM_CURRENCY_USAGE:
            return getCurrencyUsage();

        case UNUM_MINIMUM_GROUPING_DIGITS:
            return getMinimumGroupingDigits();

        case UNUM_PARSE_CASE_SENSITIVE:
            return isParseCaseSensitive();

        case UNUM_SIGN_ALWAYS_SHOWN:
            return isSignAlwaysShown();

        case UNUM_FORMAT_FAIL_IF_MORE_THAN_MAX_DIGITS:
            return isFormatFailIfMoreThanMaxDigits();

        default:
            status = U_UNSUPPORTED_ERROR;
            break;
    }
    // TODO: UNUM_FORMAT_FAIL_IF_MORE_THAN_MAX_DIGITS?

    return -1; /* undefined */
}

void DecimalFormat::setGroupingUsed(UBool enabled) {
    NumberFormat::setGroupingUsed(enabled); // to set field for compatibility
    fProperties->groupingUsed = enabled;
    touchNoError();
}

void DecimalFormat::setParseIntegerOnly(UBool value) {
    NumberFormat::setParseIntegerOnly(value); // to set field for compatibility
    fProperties->parseIntegerOnly = value;
    touchNoError();
}

void DecimalFormat::setLenient(UBool enable) {
    NumberFormat::setLenient(enable); // to set field for compatibility
    fProperties->parseMode = enable ? PARSE_MODE_LENIENT : PARSE_MODE_STRICT;
    touchNoError();
}

DecimalFormat::DecimalFormat(const UnicodeString& pattern, DecimalFormatSymbols* symbolsToAdopt,
                             UParseError&, UErrorCode& status)
        : DecimalFormat(symbolsToAdopt, status) {
    // TODO: What is parseError for?
    setPropertiesFromPattern(pattern, IGNORE_ROUNDING_IF_CURRENCY, status);
    touch(status);
}

DecimalFormat::DecimalFormat(const UnicodeString& pattern, const DecimalFormatSymbols& symbols,
                             UErrorCode& status)
        : DecimalFormat(new DecimalFormatSymbols(symbols), status) {
    setPropertiesFromPattern(pattern, IGNORE_ROUNDING_IF_CURRENCY, status);
    touch(status);
}

DecimalFormat::DecimalFormat(const DecimalFormat& source) : NumberFormat(source) {
    fProperties.adoptInstead(new DecimalFormatProperties(*source.fProperties));
    fExportedProperties.adoptInstead(new DecimalFormatProperties());
    fWarehouse.adoptInstead(new DecimalFormatWarehouse());
    fSymbols.adoptInstead(new DecimalFormatSymbols(*source.fSymbols));
    if (fProperties == nullptr || fExportedProperties == nullptr || fWarehouse == nullptr ||
        fSymbols == nullptr) {
        return;
    }
    touchNoError();
}

DecimalFormat& DecimalFormat::operator=(const DecimalFormat& rhs) {
    *fProperties = *rhs.fProperties;
    fExportedProperties->clear();
    fSymbols.adoptInstead(new DecimalFormatSymbols(*rhs.fSymbols));
    touchNoError();
    return *this;
}

DecimalFormat::~DecimalFormat() {
    delete fAtomicParser.load();
    delete fAtomicCurrencyParser.load();
};

Format* DecimalFormat::clone() const {
    return new DecimalFormat(*this);
}

UBool DecimalFormat::operator==(const Format& other) const {
    auto* otherDF = dynamic_cast<const DecimalFormat*>(&other);
    if (otherDF == nullptr) {
        return false;
    }
    return *fProperties == *otherDF->fProperties && *fSymbols == *otherDF->fSymbols;
}

UnicodeString& DecimalFormat::format(double number, UnicodeString& appendTo, FieldPosition& pos) const {
    if (pos.getField() == FieldPosition::DONT_CARE && fastFormatDouble(number, appendTo)) {
        return appendTo;
    }
    UErrorCode localStatus = U_ZERO_ERROR;
    FormattedNumber output = fFormatter->formatDouble(number, localStatus);
    output.populateFieldPosition(pos, localStatus);
    auto appendable = UnicodeStringAppendable(appendTo);
    output.appendTo(appendable);
    return appendTo;
}

UnicodeString& DecimalFormat::format(double number, UnicodeString& appendTo, FieldPosition& pos,
                                     UErrorCode& status) const {
    if (pos.getField() == FieldPosition::DONT_CARE && fastFormatDouble(number, appendTo)) {
        return appendTo;
    }
    FormattedNumber output = fFormatter->formatDouble(number, status);
    output.populateFieldPosition(pos, status);
    auto appendable = UnicodeStringAppendable(appendTo);
    output.appendTo(appendable);
    return appendTo;
}

UnicodeString&
DecimalFormat::format(double number, UnicodeString& appendTo, FieldPositionIterator* posIter,
                      UErrorCode& status) const {
    if (posIter == nullptr && fastFormatDouble(number, appendTo)) {
        return appendTo;
    }
    FormattedNumber output = fFormatter->formatDouble(number, status);
    if (posIter != nullptr) {
        output.populateFieldPositionIterator(*posIter, status);
    }
    auto appendable = UnicodeStringAppendable(appendTo);
    output.appendTo(appendable);
    return appendTo;
}

UnicodeString& DecimalFormat::format(int32_t number, UnicodeString& appendTo, FieldPosition& pos) const {
    return format(static_cast<int64_t> (number), appendTo, pos);
}

UnicodeString& DecimalFormat::format(int32_t number, UnicodeString& appendTo, FieldPosition& pos,
                                     UErrorCode& status) const {
    return format(static_cast<int64_t> (number), appendTo, pos, status);
}

UnicodeString&
DecimalFormat::format(int32_t number, UnicodeString& appendTo, FieldPositionIterator* posIter,
                      UErrorCode& status) const {
    return format(static_cast<int64_t> (number), appendTo, posIter, status);
}

UnicodeString& DecimalFormat::format(int64_t number, UnicodeString& appendTo, FieldPosition& pos) const {
    if (pos.getField() == FieldPosition::DONT_CARE && fastFormatInt64(number, appendTo)) {
        return appendTo;
    }
    UErrorCode localStatus = U_ZERO_ERROR;
    FormattedNumber output = fFormatter->formatInt(number, localStatus);
    output.populateFieldPosition(pos, localStatus);
    auto appendable = UnicodeStringAppendable(appendTo);
    output.appendTo(appendable);
    return appendTo;
}

UnicodeString& DecimalFormat::format(int64_t number, UnicodeString& appendTo, FieldPosition& pos,
                                     UErrorCode& status) const {
    if (pos.getField() == FieldPosition::DONT_CARE && fastFormatInt64(number, appendTo)) {
        return appendTo;
    }
    FormattedNumber output = fFormatter->formatInt(number, status);
    output.populateFieldPosition(pos, status);
    auto appendable = UnicodeStringAppendable(appendTo);
    output.appendTo(appendable);
    return appendTo;
}

UnicodeString&
DecimalFormat::format(int64_t number, UnicodeString& appendTo, FieldPositionIterator* posIter,
                      UErrorCode& status) const {
    if (posIter == nullptr && fastFormatInt64(number, appendTo)) {
        return appendTo;
    }
    FormattedNumber output = fFormatter->formatInt(number, status);
    if (posIter != nullptr) {
        output.populateFieldPositionIterator(*posIter, status);
    }
    auto appendable = UnicodeStringAppendable(appendTo);
    output.appendTo(appendable);
    return appendTo;
}

UnicodeString&
DecimalFormat::format(StringPiece number, UnicodeString& appendTo, FieldPositionIterator* posIter,
                      UErrorCode& status) const {
    FormattedNumber output = fFormatter->formatDecimal(number, status);
    if (posIter != nullptr) {
        output.populateFieldPositionIterator(*posIter, status);
    }
    auto appendable = UnicodeStringAppendable(appendTo);
    output.appendTo(appendable);
    return appendTo;
}

UnicodeString& DecimalFormat::format(const DecimalQuantity& number, UnicodeString& appendTo,
                                     FieldPositionIterator* posIter, UErrorCode& status) const {
    FormattedNumber output = fFormatter->formatDecimalQuantity(number, status);
    if (posIter != nullptr) {
        output.populateFieldPositionIterator(*posIter, status);
    }
    auto appendable = UnicodeStringAppendable(appendTo);
    output.appendTo(appendable);
    return appendTo;
}

UnicodeString&
DecimalFormat::format(const DecimalQuantity& number, UnicodeString& appendTo, FieldPosition& pos,
                      UErrorCode& status) const {
    FormattedNumber output = fFormatter->formatDecimalQuantity(number, status);
    output.populateFieldPosition(pos, status);
    auto appendable = UnicodeStringAppendable(appendTo);
    output.appendTo(appendable);
    return appendTo;
}

void DecimalFormat::parse(const UnicodeString& text, Formattable& output,
                          ParsePosition& parsePosition) const {
    if (parsePosition.getIndex() < 0 || parsePosition.getIndex() >= text.length()) {
        return;
    }

    ErrorCode status;
    ParsedNumber result;
    // Note: if this is a currency instance, currencies will be matched despite the fact that we are not in the
    // parseCurrency method (backwards compatibility)
    int32_t startIndex = parsePosition.getIndex();
    const NumberParserImpl& parser = getParser(status);
    if (U_FAILURE(status)) { return; }
    parser.parse(text, startIndex, true, result, status);
    // TODO: Do we need to check for fProperties->parseAllInput (UCONFIG_HAVE_PARSEALLINPUT) here?
    if (result.success()) {
        parsePosition.setIndex(result.charEnd);
        result.populateFormattable(output, parser.getParseFlags());
    } else {
        parsePosition.setErrorIndex(startIndex + result.charEnd);
    }
}

CurrencyAmount* DecimalFormat::parseCurrency(const UnicodeString& text, ParsePosition& parsePosition) const {
    if (parsePosition.getIndex() < 0 || parsePosition.getIndex() >= text.length()) {
        return nullptr;
    }

    ErrorCode status;
    ParsedNumber result;
    // Note: if this is a currency instance, currencies will be matched despite the fact that we are not in the
    // parseCurrency method (backwards compatibility)
    int32_t startIndex = parsePosition.getIndex();
    const NumberParserImpl& parser = getCurrencyParser(status);
    if (U_FAILURE(status)) { return nullptr; }
    parser.parse(text, startIndex, true, result, status);
    // TODO: Do we need to check for fProperties->parseAllInput (UCONFIG_HAVE_PARSEALLINPUT) here?
    if (result.success()) {
        parsePosition.setIndex(result.charEnd);
        Formattable formattable;
        result.populateFormattable(formattable, parser.getParseFlags());
        return new CurrencyAmount(formattable, result.currencyCode, status);
    } else {
        parsePosition.setErrorIndex(startIndex + result.charEnd);
        return nullptr;
    }
}

const DecimalFormatSymbols* DecimalFormat::getDecimalFormatSymbols(void) const {
    return fSymbols.getAlias();
}

void DecimalFormat::adoptDecimalFormatSymbols(DecimalFormatSymbols* symbolsToAdopt) {
    if (symbolsToAdopt == nullptr) {
        return; // do not allow caller to set fSymbols to NULL
    }
    fSymbols.adoptInstead(symbolsToAdopt);
    touchNoError();
}

void DecimalFormat::setDecimalFormatSymbols(const DecimalFormatSymbols& symbols) {
    fSymbols.adoptInstead(new DecimalFormatSymbols(symbols));
    touchNoError();
}

const CurrencyPluralInfo* DecimalFormat::getCurrencyPluralInfo(void) const {
    return fProperties->currencyPluralInfo.fPtr.getAlias();
}

void DecimalFormat::adoptCurrencyPluralInfo(CurrencyPluralInfo* toAdopt) {
    fProperties->currencyPluralInfo.fPtr.adoptInstead(toAdopt);
    touchNoError();
}

void DecimalFormat::setCurrencyPluralInfo(const CurrencyPluralInfo& info) {
    *fProperties->currencyPluralInfo.fPtr = info; // copy-assignment operator
    touchNoError();
}

UnicodeString& DecimalFormat::getPositivePrefix(UnicodeString& result) const {
    ErrorCode localStatus;
    fFormatter->getAffix(true, false, result, localStatus);
    return result;
}

void DecimalFormat::setPositivePrefix(const UnicodeString& newValue) {
    fProperties->positivePrefix = newValue;
    touchNoError();
}

UnicodeString& DecimalFormat::getNegativePrefix(UnicodeString& result) const {
    ErrorCode localStatus;
    fFormatter->getAffix(true, true, result, localStatus);
    return result;
}

void DecimalFormat::setNegativePrefix(const UnicodeString& newValue) {
    fProperties->negativePrefix = newValue;
    touchNoError();
}

UnicodeString& DecimalFormat::getPositiveSuffix(UnicodeString& result) const {
    ErrorCode localStatus;
    fFormatter->getAffix(false, false, result, localStatus);
    return result;
}

void DecimalFormat::setPositiveSuffix(const UnicodeString& newValue) {
    fProperties->positiveSuffix = newValue;
    touchNoError();
}

UnicodeString& DecimalFormat::getNegativeSuffix(UnicodeString& result) const {
    ErrorCode localStatus;
    fFormatter->getAffix(false, true, result, localStatus);
    return result;
}

void DecimalFormat::setNegativeSuffix(const UnicodeString& newValue) {
    fProperties->negativeSuffix = newValue;
    touchNoError();
}

UBool DecimalFormat::isSignAlwaysShown() const {
    return fProperties->signAlwaysShown;
}

void DecimalFormat::setSignAlwaysShown(UBool value) {
    fProperties->signAlwaysShown = value;
    touchNoError();
}

int32_t DecimalFormat::getMultiplier(void) const {
    if (fProperties->multiplier != 1) {
        return fProperties->multiplier;
    } else if (fProperties->magnitudeMultiplier != 0) {
        return static_cast<int32_t>(uprv_pow10(fProperties->magnitudeMultiplier));
    } else {
        return 1;
    }
}

void DecimalFormat::setMultiplier(int32_t multiplier) {
    if (multiplier == 0) {
        multiplier = 1;     // one being the benign default value for a multiplier.
    }

    // Try to convert to a magnitude multiplier first
    int delta = 0;
    int value = multiplier;
    while (value != 1) {
        delta++;
        int temp = value / 10;
        if (temp * 10 != value) {
            delta = -1;
            break;
        }
        value = temp;
    }
    if (delta != -1) {
        fProperties->magnitudeMultiplier = delta;
        fProperties->multiplier = 1;
    } else {
        fProperties->magnitudeMultiplier = 0;
        fProperties->multiplier = multiplier;
    }
    touchNoError();
}

int32_t DecimalFormat::getMultiplierScale() const {
    return fProperties->multiplierScale;
}

void DecimalFormat::setMultiplierScale(int32_t newValue) {
    fProperties->multiplierScale = newValue;
    touchNoError();
}

double DecimalFormat::getRoundingIncrement(void) const {
    return fExportedProperties->roundingIncrement;
}

void DecimalFormat::setRoundingIncrement(double newValue) {
    fProperties->roundingIncrement = newValue;
    touchNoError();
}

ERoundingMode DecimalFormat::getRoundingMode(void) const {
    // UNumberFormatRoundingMode and ERoundingMode have the same values.
    return static_cast<ERoundingMode>(fExportedProperties->roundingMode.getNoError());
}

void DecimalFormat::setRoundingMode(ERoundingMode roundingMode) {
    NumberFormat::setMaximumIntegerDigits(roundingMode); // to set field for compatibility
    fProperties->roundingMode = static_cast<UNumberFormatRoundingMode>(roundingMode);
    touchNoError();
}

int32_t DecimalFormat::getFormatWidth(void) const {
    return fProperties->formatWidth;
}

void DecimalFormat::setFormatWidth(int32_t width) {
    fProperties->formatWidth = width;
    touchNoError();
}

UnicodeString DecimalFormat::getPadCharacterString() const {
    if (fProperties->padString.isBogus()) {
        // Readonly-alias the static string kFallbackPaddingString
        return {TRUE, kFallbackPaddingString, -1};
    } else {
        return fProperties->padString;
    }
}

void DecimalFormat::setPadCharacter(const UnicodeString& padChar) {
    if (padChar.length() > 0) {
        fProperties->padString = UnicodeString(padChar.char32At(0));
    } else {
        fProperties->padString.setToBogus();
    }
    touchNoError();
}

EPadPosition DecimalFormat::getPadPosition(void) const {
    if (fProperties->padPosition.isNull()) {
        return EPadPosition::kPadBeforePrefix;
    } else {
        // UNumberFormatPadPosition and EPadPosition have the same values.
        return static_cast<EPadPosition>(fProperties->padPosition.getNoError());
    }
}

void DecimalFormat::setPadPosition(EPadPosition padPos) {
    fProperties->padPosition = static_cast<UNumberFormatPadPosition>(padPos);
    touchNoError();
}

UBool DecimalFormat::isScientificNotation(void) const {
    return fProperties->minimumExponentDigits != -1;
}

void DecimalFormat::setScientificNotation(UBool useScientific) {
    if (useScientific) {
        fProperties->minimumExponentDigits = 1;
    } else {
        fProperties->minimumExponentDigits = -1;
    }
    touchNoError();
}

int8_t DecimalFormat::getMinimumExponentDigits(void) const {
    return static_cast<int8_t>(fProperties->minimumExponentDigits);
}

void DecimalFormat::setMinimumExponentDigits(int8_t minExpDig) {
    fProperties->minimumExponentDigits = minExpDig;
    touchNoError();
}

UBool DecimalFormat::isExponentSignAlwaysShown(void) const {
    return fProperties->exponentSignAlwaysShown;
}

void DecimalFormat::setExponentSignAlwaysShown(UBool expSignAlways) {
    fProperties->exponentSignAlwaysShown = expSignAlways;
    touchNoError();
}

int32_t DecimalFormat::getGroupingSize(void) const {
    if (fProperties->groupingSize < 0) {
        return 0;
    }
    return fProperties->groupingSize;
}

void DecimalFormat::setGroupingSize(int32_t newValue) {
    fProperties->groupingSize = newValue;
    touchNoError();
}

int32_t DecimalFormat::getSecondaryGroupingSize(void) const {
    int grouping2 = fProperties->secondaryGroupingSize;
    if (grouping2 < 0) {
        return 0;
    }
    return grouping2;
}

void DecimalFormat::setSecondaryGroupingSize(int32_t newValue) {
    fProperties->secondaryGroupingSize = newValue;
    touchNoError();
}

int32_t DecimalFormat::getMinimumGroupingDigits() const {
    return fProperties->minimumGroupingDigits;
}

void DecimalFormat::setMinimumGroupingDigits(int32_t newValue) {
    fProperties->minimumGroupingDigits = newValue;
    touchNoError();
}

UBool DecimalFormat::isDecimalSeparatorAlwaysShown(void) const {
    return fProperties->decimalSeparatorAlwaysShown;
}

void DecimalFormat::setDecimalSeparatorAlwaysShown(UBool newValue) {
    fProperties->decimalSeparatorAlwaysShown = newValue;
    touchNoError();
}

UBool DecimalFormat::isDecimalPatternMatchRequired(void) const {
    return fProperties->decimalPatternMatchRequired;
}

void DecimalFormat::setDecimalPatternMatchRequired(UBool newValue) {
    fProperties->decimalPatternMatchRequired = newValue;
    touchNoError();
}

UBool DecimalFormat::isParseNoExponent() const {
    return fProperties->parseNoExponent;
}

void DecimalFormat::setParseNoExponent(UBool value) {
    fProperties->parseNoExponent = value;
    touchNoError();
}

UBool DecimalFormat::isParseCaseSensitive() const {
    return fProperties->parseCaseSensitive;
}

void DecimalFormat::setParseCaseSensitive(UBool value) {
    fProperties->parseCaseSensitive = value;
    touchNoError();
}

UBool DecimalFormat::isFormatFailIfMoreThanMaxDigits() const {
    return fProperties->formatFailIfMoreThanMaxDigits;
}

void DecimalFormat::setFormatFailIfMoreThanMaxDigits(UBool value) {
    fProperties->formatFailIfMoreThanMaxDigits = value;
    touchNoError();
}

UnicodeString& DecimalFormat::toPattern(UnicodeString& result) const {
    // Pull some properties from exportedProperties and others from properties
    // to keep affix patterns intact.  In particular, pull rounding properties
    // so that CurrencyUsage is reflected properly.
    // TODO: Consider putting this logic in number_patternstring.cpp instead.
    ErrorCode localStatus;
    DecimalFormatProperties tprops(*fProperties);
    bool useCurrency = ((!tprops.currency.isNull()) || !tprops.currencyPluralInfo.fPtr.isNull() ||
                        !tprops.currencyUsage.isNull() || AffixUtils::hasCurrencySymbols(
            UnicodeStringCharSequence(tprops.positivePrefixPattern), localStatus) ||
                        AffixUtils::hasCurrencySymbols(
                                UnicodeStringCharSequence(tprops.positiveSuffixPattern), localStatus) ||
                        AffixUtils::hasCurrencySymbols(
                                UnicodeStringCharSequence(tprops.negativePrefixPattern), localStatus) ||
                        AffixUtils::hasCurrencySymbols(
                                UnicodeStringCharSequence(tprops.negativeSuffixPattern), localStatus));
    if (useCurrency) {
        tprops.minimumFractionDigits = fExportedProperties->minimumFractionDigits;
        tprops.maximumFractionDigits = fExportedProperties->maximumFractionDigits;
        tprops.roundingIncrement = fExportedProperties->roundingIncrement;
    }
    result = PatternStringUtils::propertiesToPatternString(tprops, localStatus);
    return result;
}

UnicodeString& DecimalFormat::toLocalizedPattern(UnicodeString& result) const {
    ErrorCode localStatus;
    result = toPattern(result);
    result = PatternStringUtils::convertLocalized(result, *fSymbols, true, localStatus);
    return result;
}

void DecimalFormat::applyPattern(const UnicodeString& pattern, UParseError&, UErrorCode& status) {
    // TODO: What is parseError for?
    applyPattern(pattern, status);
}

void DecimalFormat::applyPattern(const UnicodeString& pattern, UErrorCode& status) {
    setPropertiesFromPattern(pattern, IGNORE_ROUNDING_NEVER, status);
    touch(status);
}

void DecimalFormat::applyLocalizedPattern(const UnicodeString& localizedPattern, UParseError&,
                                          UErrorCode& status) {
    // TODO: What is parseError for?
    applyLocalizedPattern(localizedPattern, status);
}

void DecimalFormat::applyLocalizedPattern(const UnicodeString& localizedPattern, UErrorCode& status) {
    UnicodeString pattern = PatternStringUtils::convertLocalized(
            localizedPattern, *fSymbols, false, status);
    applyPattern(pattern, status);
}

void DecimalFormat::setMaximumIntegerDigits(int32_t newValue) {
    // For backwards compatibility, conflicting min/max need to keep the most recent setting.
    int32_t min = fProperties->minimumIntegerDigits;
    if (min >= 0 && min > newValue) {
        fProperties->minimumIntegerDigits = newValue;
    }
    fProperties->maximumIntegerDigits = newValue;
    touchNoError();
}

void DecimalFormat::setMinimumIntegerDigits(int32_t newValue) {
    // For backwards compatibility, conflicting min/max need to keep the most recent setting.
    int32_t max = fProperties->maximumIntegerDigits;
    if (max >= 0 && max < newValue) {
        fProperties->maximumIntegerDigits = newValue;
    }
    fProperties->minimumIntegerDigits = newValue;
    touchNoError();
}

void DecimalFormat::setMaximumFractionDigits(int32_t newValue) {
    // For backwards compatibility, conflicting min/max need to keep the most recent setting.
    int32_t min = fProperties->minimumFractionDigits;
    if (min >= 0 && min > newValue) {
        fProperties->minimumFractionDigits = newValue;
    }
    fProperties->maximumFractionDigits = newValue;
    touchNoError();
}

void DecimalFormat::setMinimumFractionDigits(int32_t newValue) {
    // For backwards compatibility, conflicting min/max need to keep the most recent setting.
    int32_t max = fProperties->maximumFractionDigits;
    if (max >= 0 && max < newValue) {
        fProperties->maximumFractionDigits = newValue;
    }
    fProperties->minimumFractionDigits = newValue;
    touchNoError();
}

int32_t DecimalFormat::getMinimumSignificantDigits() const {
    return fExportedProperties->minimumSignificantDigits;
}

int32_t DecimalFormat::getMaximumSignificantDigits() const {
    return fExportedProperties->maximumSignificantDigits;
}

void DecimalFormat::setMinimumSignificantDigits(int32_t value) {
    int32_t max = fProperties->maximumSignificantDigits;
    if (max >= 0 && max < value) {
        fProperties->maximumSignificantDigits = value;
    }
    fProperties->minimumSignificantDigits = value;
    touchNoError();
}

void DecimalFormat::setMaximumSignificantDigits(int32_t value) {
    int32_t min = fProperties->minimumSignificantDigits;
    if (min >= 0 && min > value) {
        fProperties->minimumSignificantDigits = value;
    }
    fProperties->maximumSignificantDigits = value;
    touchNoError();
}

UBool DecimalFormat::areSignificantDigitsUsed() const {
    return fProperties->minimumSignificantDigits != -1 || fProperties->maximumSignificantDigits != -1;
}

void DecimalFormat::setSignificantDigitsUsed(UBool useSignificantDigits) {
    if (useSignificantDigits) {
        // These are the default values from the old implementation.
        fProperties->minimumSignificantDigits = 1;
        fProperties->maximumSignificantDigits = 6;
    } else {
        fProperties->minimumSignificantDigits = -1;
        fProperties->maximumSignificantDigits = -1;
    }
    touchNoError();
}

void DecimalFormat::setCurrency(const char16_t* theCurrency, UErrorCode& ec) {
    NumberFormat::setCurrency(theCurrency, ec); // to set field for compatibility
    fProperties->currency = CurrencyUnit(theCurrency, ec);
    // TODO: Set values in fSymbols, too?
    touchNoError();
}

void DecimalFormat::setCurrency(const char16_t* theCurrency) {
    ErrorCode localStatus;
    setCurrency(theCurrency, localStatus);
}

void DecimalFormat::setCurrencyUsage(UCurrencyUsage newUsage, UErrorCode* ec) {
    fProperties->currencyUsage = newUsage;
    touch(*ec);
}

UCurrencyUsage DecimalFormat::getCurrencyUsage() const {
    // CurrencyUsage is not exported, so we have to get it from the input property bag.
    // TODO: Should we export CurrencyUsage instead?
    if (fProperties->currencyUsage.isNull()) {
        return UCURR_USAGE_STANDARD;
    }
    return fProperties->currencyUsage.getNoError();
}

void
DecimalFormat::formatToDecimalQuantity(double number, DecimalQuantity& output, UErrorCode& status) const {
    fFormatter->formatDouble(number, status).getDecimalQuantity(output, status);
}

void DecimalFormat::formatToDecimalQuantity(const Formattable& number, DecimalQuantity& output,
                                            UErrorCode& status) const {
    UFormattedNumberData obj;
    number.populateDecimalQuantity(obj.quantity, status);
    fFormatter->formatImpl(&obj, status);
    output = std::move(obj.quantity);
}

number::LocalizedNumberFormatter&
DecimalFormat::toNumberFormatter(number::LocalizedNumberFormatter& output) const {
    output = *fFormatter; // copy assignment
    return output;
}

/** Rebuilds the formatter object from the property bag. */
void DecimalFormat::touch(UErrorCode& status) {
    if (fExportedProperties == nullptr) {
        // fExportedProperties is null only when the formatter is not ready yet.
        // The only time when this happens is during legacy deserialization.
        return;
    }

    setupFastFormat();

    // In C++, fSymbols is the source of truth for the locale.
    Locale locale = fSymbols->getLocale();

    // Note: The formatter is relatively cheap to create, and we need it to populate fExportedProperties,
    // so automatically compute it here. The parser is a bit more expensive and is not needed until the
    // parse method is called, so defer that until needed.
    fFormatter.adoptInstead(
            new LocalizedNumberFormatter(
                    NumberPropertyMapper::create(
                            *fProperties, *fSymbols, *fWarehouse, *fExportedProperties, status).locale(
                            locale)));

    // Delete the parsers if they were made previously
    delete fAtomicParser.exchange(nullptr, std::memory_order_relaxed);
    delete fAtomicCurrencyParser.exchange(nullptr, std::memory_order_relaxed);

    // In order for the getters to work, we need to populate some fields in NumberFormat.
    NumberFormat::setCurrency(fExportedProperties->currency.get(status).getISOCurrency(), status);
    NumberFormat::setMaximumIntegerDigits(fExportedProperties->maximumIntegerDigits);
    NumberFormat::setMinimumIntegerDigits(fExportedProperties->minimumIntegerDigits);
    NumberFormat::setMaximumFractionDigits(fExportedProperties->maximumFractionDigits);
    NumberFormat::setMinimumFractionDigits(fExportedProperties->minimumFractionDigits);
    // fProperties, not fExportedProperties, since this information comes from the pattern:
    NumberFormat::setGroupingUsed(fProperties->groupingUsed);
}

void DecimalFormat::touchNoError() {
    UErrorCode localStatus = U_ZERO_ERROR;
    touch(localStatus);
}

void DecimalFormat::setPropertiesFromPattern(const UnicodeString& pattern, int32_t ignoreRounding,
                                             UErrorCode& status) {
    // Cast workaround to get around putting the enum in the public header file
    auto actualIgnoreRounding = static_cast<IgnoreRounding>(ignoreRounding);
    PatternParser::parseToExistingProperties(pattern, *fProperties, actualIgnoreRounding, status);
}

const numparse::impl::NumberParserImpl& DecimalFormat::getParser(UErrorCode& status) const {
    auto* ptr = fAtomicParser.load();
    if (ptr != nullptr) {
        return *ptr;
    }

    ptr = NumberParserImpl::createParserFromProperties(*fProperties, *fSymbols, false, status);

    if (ptr == nullptr) {
        status = U_MEMORY_ALLOCATION_ERROR;
        // although we still dereference, call sites should be guarded
    }

    // Store the new pointer, and delete the old one if it got created.
    auto* nonConstThis = const_cast<DecimalFormat*>(this);
    delete nonConstThis->fAtomicParser.exchange(ptr, std::memory_order_relaxed);
    return *ptr;
}

const numparse::impl::NumberParserImpl& DecimalFormat::getCurrencyParser(UErrorCode& status) const {
    auto* ptr = fAtomicCurrencyParser.load();
    if (ptr != nullptr) {
        return *ptr;
    }

    ptr = NumberParserImpl::createParserFromProperties(*fProperties, *fSymbols, true, status);

    if (ptr == nullptr) {
        status = U_MEMORY_ALLOCATION_ERROR;
        // although we still dereference, call sites should be guarded
    }

    // Store the new pointer, and delete the old one if it got created.
    auto* nonConstThis = const_cast<DecimalFormat*>(this);
    delete nonConstThis->fAtomicCurrencyParser.exchange(ptr, std::memory_order_relaxed);
    return *ptr;
}

void DecimalFormat::setupFastFormat() {
    // Check the majority of properties:
    if (!fProperties->equalsDefaultExceptFastFormat()) {
        fCanUseFastFormat = false;
        return;
    }

    // Now check the remaining properties.
    // Nontrivial affixes:
    UBool trivialPP = fProperties->positivePrefixPattern.isEmpty();
    UBool trivialPS = fProperties->positiveSuffixPattern.isEmpty();
    UBool trivialNP = fProperties->negativePrefixPattern.isBogus() || (
            fProperties->negativePrefixPattern.length() == 1 &&
            fProperties->negativePrefixPattern.charAt(0) == u'-');
    UBool trivialNS = fProperties->negativeSuffixPattern.isEmpty();
    if (!trivialPP || !trivialPS || !trivialNP || !trivialNS) {
        fCanUseFastFormat = false;
        return;
    }

    // Grouping (secondary grouping is forbidden in equalsDefaultExceptFastFormat):
    bool groupingUsed = fProperties->groupingUsed;
    bool unusualGroupingSize = fProperties->groupingSize > 0 && fProperties->groupingSize != 3;
    const UnicodeString& groupingString = fSymbols->getConstSymbol(DecimalFormatSymbols::kGroupingSeparatorSymbol);
    if (groupingUsed && (unusualGroupingSize || groupingString.length() != 1)) {
        fCanUseFastFormat = false;
        return;
    }

    // Integer length:
    int32_t minInt = fProperties->minimumIntegerDigits;
    int32_t maxInt = fProperties->maximumIntegerDigits;
    // Fastpath supports up to only 10 digits (length of INT32_MIN)
    if (minInt > 10) {
        fCanUseFastFormat = false;
        return;
    }

    // Other symbols:
    const UnicodeString& minusSignString = fSymbols->getConstSymbol(DecimalFormatSymbols::kMinusSignSymbol);
    UChar32 codePointZero = fSymbols->getCodePointZero();
    if (minusSignString.length() != 1 || U16_LENGTH(codePointZero) != 1) {
        fCanUseFastFormat = false;
        return;
    }

    // Good to go!
    fCanUseFastFormat = true;
    fFastData.cpZero = static_cast<char16_t>(codePointZero);
    fFastData.cpGroupingSeparator = groupingUsed ? groupingString.charAt(0) : 0;
    fFastData.cpMinusSign = minusSignString.charAt(0);
    fFastData.minInt = static_cast<int8_t>(minInt);
    fFastData.maxInt = static_cast<int8_t>(maxInt < 0 ? 127 : maxInt);
}

bool DecimalFormat::fastFormatDouble(double input, UnicodeString& output) const {
    if (!fCanUseFastFormat) {
        return false;
    }
    auto i32 = static_cast<int32_t>(input);
    if (i32 != input || i32 == INT32_MIN) {
        return false;
    }
    doFastFormatInt32(i32, output);
    return true;
}

bool DecimalFormat::fastFormatInt64(int64_t input, UnicodeString& output) const {
    if (!fCanUseFastFormat) {
        return false;
    }
    auto i32 = static_cast<int32_t>(input);
    if (i32 != input || i32 == INT32_MIN) {
        return false;
    }
    doFastFormatInt32(i32, output);
    return true;
}

void DecimalFormat::doFastFormatInt32(int32_t input, UnicodeString& output) const {
    U_ASSERT(fCanUseFastFormat);
    if (input < 0) {
        output.append(fFastData.cpMinusSign);
        U_ASSERT(input != INT32_MIN);  // handled by callers
        input = -input;
    }
    // Cap at int32_t to make the buffer small and operations fast.
    // Longest string: "2,147,483,648" (13 chars in length)
    static constexpr int32_t localCapacity = 13;
    char16_t localBuffer[localCapacity];
    char16_t* ptr = localBuffer + localCapacity;
    int8_t group = 0;
    for (int8_t i = 0; i < fFastData.maxInt && (input != 0 || i < fFastData.minInt); i++) {
        std::div_t res = std::div(input, 10);
        *(--ptr) = static_cast<char16_t>(fFastData.cpZero + res.rem);
        input = res.quot;
        if (++group == 3 && fFastData.cpGroupingSeparator != 0) {
            *(--ptr) = fFastData.cpGroupingSeparator;
            group = 0;
        }
    }
    int32_t len = localCapacity - static_cast<int32_t>(ptr - localBuffer);
    output.append(ptr, len);
}


#endif /* #if !UCONFIG_NO_FORMATTING */
