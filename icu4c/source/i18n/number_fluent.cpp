// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#if !UCONFIG_NO_FORMATTING

#include "uassert.h"
#include "unicode/numberformatter.h"
#include "number_decimalquantity.h"
#include "number_formatimpl.h"

using namespace icu;
using namespace icu::number;
using namespace icu::number::impl;

template<typename Derived>
Derived NumberFormatterSettings<Derived>::notation(const Notation &notation) const {
    Derived copy(*this);
    // NOTE: Slicing is OK.
    copy.fMacros.notation = notation;
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::unit(const icu::MeasureUnit &unit) const {
    Derived copy(*this);
    // NOTE: Slicing occurs here. However, CurrencyUnit can be restored from MeasureUnit.
    // TimeUnit may be affected, but TimeUnit is not as relevant to number formatting.
    copy.fMacros.unit = unit;
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::adoptUnit(const icu::MeasureUnit *unit) const {
    Derived copy(*this);
    // Just copy the unit into the MacroProps by value, and delete it since we have ownership.
    // NOTE: Slicing occurs here. However, CurrencyUnit can be restored from MeasureUnit.
    // TimeUnit may be affected, but TimeUnit is not as relevant to number formatting.
    if (unit != nullptr) {
        copy.fMacros.unit = *unit;
        delete unit;
    }
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::rounding(const Rounder &rounder) const {
    Derived copy(*this);
    // NOTE: Slicing is OK.
    copy.fMacros.rounder = rounder;
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::grouping(const Grouper &grouper) const {
    Derived copy(*this);
    copy.fMacros.grouper = grouper;
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::integerWidth(const IntegerWidth &style) const {
    Derived copy(*this);
    copy.fMacros.integerWidth = style;
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::symbols(const DecimalFormatSymbols &symbols) const {
    Derived copy(*this);
    copy.fMacros.symbols.setTo(symbols);
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::adoptSymbols(const NumberingSystem *ns) const {
    Derived copy(*this);
    copy.fMacros.symbols.setTo(ns);
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::unitWidth(const UNumberUnitWidth &width) const {
    Derived copy(*this);
    copy.fMacros.unitWidth = width;
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::sign(const UNumberSignDisplay &style) const {
    Derived copy(*this);
    copy.fMacros.sign = style;
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::decimal(const UNumberDecimalSeparatorDisplay &style) const {
    Derived copy(*this);
    copy.fMacros.decimal = style;
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::padding(const Padder &padder) const {
    Derived copy(*this);
    copy.fMacros.padder = padder;
    return copy;
}

template<typename Derived>
Derived NumberFormatterSettings<Derived>::threshold(uint32_t threshold) const {
    Derived copy(*this);
    copy.fMacros.threshold = threshold;
    return copy;
}

// Declare all classes that implement NumberFormatterSettings
// See https://stackoverflow.com/a/495056/1407170
template
class icu::number::NumberFormatterSettings<icu::number::UnlocalizedNumberFormatter>;
template
class icu::number::NumberFormatterSettings<icu::number::LocalizedNumberFormatter>;


UnlocalizedNumberFormatter NumberFormatter::with() {
    UnlocalizedNumberFormatter result;
    return result;
}

LocalizedNumberFormatter NumberFormatter::withLocale(const Locale &locale) {
    return with().locale(locale);
}

// Make the child class constructor that takes the parent class call the parent class's copy constructor
UnlocalizedNumberFormatter::UnlocalizedNumberFormatter(
        const NumberFormatterSettings <UnlocalizedNumberFormatter> &other)
        : NumberFormatterSettings<UnlocalizedNumberFormatter>(other) {
}

// Make the child class constructor that takes the parent class call the parent class's copy constructor
// For LocalizedNumberFormatter, also copy over the extra fields
LocalizedNumberFormatter::LocalizedNumberFormatter(
        const NumberFormatterSettings <LocalizedNumberFormatter> &other)
        : NumberFormatterSettings<LocalizedNumberFormatter>(other) {
    // No additional copies required
}

LocalizedNumberFormatter::LocalizedNumberFormatter(const MacroProps &macros, const Locale &locale) {
    fMacros = macros;
    fMacros.locale = locale;
}

LocalizedNumberFormatter UnlocalizedNumberFormatter::locale(const Locale &locale) const {
    return LocalizedNumberFormatter(fMacros, locale);
}

SymbolsWrapper::SymbolsWrapper(const SymbolsWrapper &other) {
    doCopyFrom(other);
}

SymbolsWrapper &SymbolsWrapper::operator=(const SymbolsWrapper &other) {
    if (this == &other) {
        return *this;
    }
    doCleanup();
    doCopyFrom(other);
    return *this;
}

SymbolsWrapper::~SymbolsWrapper() {
    doCleanup();
}

void SymbolsWrapper::setTo(const DecimalFormatSymbols &dfs) {
    doCleanup();
    fType = SYMPTR_DFS;
    fPtr.dfs = new DecimalFormatSymbols(dfs);
}

void SymbolsWrapper::setTo(const NumberingSystem *ns) {
    doCleanup();
    fType = SYMPTR_NS;
    fPtr.ns = ns;
}

void SymbolsWrapper::doCopyFrom(const SymbolsWrapper &other) {
    fType = other.fType;
    switch (fType) {
        case SYMPTR_NONE:
            // No action necessary
            break;
        case SYMPTR_DFS:
            // Memory allocation failures are exposed in copyErrorTo()
            if (other.fPtr.dfs != nullptr) {
                fPtr.dfs = new DecimalFormatSymbols(*other.fPtr.dfs);
            } else {
                fPtr.dfs = nullptr;
            }
            break;
        case SYMPTR_NS:
            // Memory allocation failures are exposed in copyErrorTo()
            if (other.fPtr.ns != nullptr) {
                fPtr.ns = new NumberingSystem(*other.fPtr.ns);
            } else {
                fPtr.ns = nullptr;
            }
            break;
    }
}

void SymbolsWrapper::doCleanup() {
    switch (fType) {
        case SYMPTR_NONE:
            // No action necessary
            break;
        case SYMPTR_DFS:
            delete fPtr.dfs;
            break;
        case SYMPTR_NS:
            delete fPtr.ns;
            break;
    }
}

bool SymbolsWrapper::isDecimalFormatSymbols() const {
    return fType == SYMPTR_DFS;
}

bool SymbolsWrapper::isNumberingSystem() const {
    return fType == SYMPTR_NS;
}

const DecimalFormatSymbols* SymbolsWrapper::getDecimalFormatSymbols() const {
    U_ASSERT(fType == SYMPTR_DFS);
    return fPtr.dfs;
}

const NumberingSystem* SymbolsWrapper::getNumberingSystem() const {
    U_ASSERT(fType == SYMPTR_NS);
    return fPtr.ns;
}

LocalizedNumberFormatter::~LocalizedNumberFormatter() {
    delete fCompiled.load();
}

FormattedNumber LocalizedNumberFormatter::formatInt(int64_t value, UErrorCode &status) const {
    if (U_FAILURE(status)) { return FormattedNumber(); }
    auto results = new NumberFormatterResults();
    if (results == nullptr) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return FormattedNumber();
    }
    results->quantity.setToLong(value);
    return formatImpl(results, status);
}

FormattedNumber LocalizedNumberFormatter::formatDouble(double value, UErrorCode &status) const {
    if (U_FAILURE(status)) { return FormattedNumber(); }
    auto results = new NumberFormatterResults();
    if (results == nullptr) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return FormattedNumber();
    }
    results->quantity.setToDouble(value);
    return formatImpl(results, status);
}

FormattedNumber LocalizedNumberFormatter::formatDecimal(StringPiece value, UErrorCode &status) const {
    if (U_FAILURE(status)) { return FormattedNumber(); }
    auto results = new NumberFormatterResults();
    if (results == nullptr) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return FormattedNumber();
    }
    results->quantity.setToDecNumber(value);
    return formatImpl(results, status);
}

FormattedNumber
LocalizedNumberFormatter::formatImpl(impl::NumberFormatterResults *results, UErrorCode &status) const {
    uint32_t currentCount = fCallCount.load();
    if (currentCount <= fMacros.threshold && fMacros.threshold > 0) {
        currentCount = const_cast<LocalizedNumberFormatter *>(this)->fCallCount.fetch_add(1) + 1;
    }
    const NumberFormatterImpl *compiled;
    if (currentCount == fMacros.threshold && fMacros.threshold > 0) {
        compiled = NumberFormatterImpl::fromMacros(fMacros, status);
        U_ASSERT(fCompiled.load() == nullptr);
        const_cast<LocalizedNumberFormatter *>(this)->fCompiled.store(compiled);
        compiled->apply(results->quantity, results->string, status);
    } else if ((compiled = fCompiled.load()) != nullptr) {
        compiled->apply(results->quantity, results->string, status);
    } else {
        NumberFormatterImpl::applyStatic(fMacros, results->quantity, results->string, status);
    }

    return FormattedNumber(results);
}

UnicodeString FormattedNumber::toString() const {
    return fResults->string.toUnicodeString();
}

Appendable &FormattedNumber::appendTo(Appendable &appendable) {
    appendable.appendString(fResults->string.chars(), fResults->string.length());
    return appendable;
}

void FormattedNumber::populateFieldPosition(FieldPosition &fieldPosition, UErrorCode &status) {
    fResults->string.populateFieldPosition(fieldPosition, 0, status);
}

void
FormattedNumber::populateFieldPositionIterator(FieldPositionIterator &iterator, UErrorCode &status) {
    fResults->string.populateFieldPositionIterator(iterator, status);
}

FormattedNumber::~FormattedNumber() {
    delete fResults;
}

#endif /* #if !UCONFIG_NO_FORMATTING */
