// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

// ucasemap_imp.h
// created: 2017feb08 Markus W. Scherer

#ifndef __UCASEMAP_IMP_H__
#define __UCASEMAP_IMP_H__

#include "unicode/utypes.h"
#include "unicode/ucasemap.h"
#include "ucase.h"

/**
 * Internal API, used by u_strcasecmp() etc.
 * Compare strings case-insensitively,
 * in code point order or code unit order.
 */
U_CFUNC int32_t
u_strcmpFold(const UChar *s1, int32_t length1,
             const UChar *s2, int32_t length2,
             uint32_t options,
             UErrorCode *pErrorCode);

/**
 * Interanl API, used for detecting length of
 * shared prefix case-insensitively.
 * @param s1            input string 1
 * @param length1       length of string 1, or -1 (NULL terminated)
 * @param s2            input string 2
 * @param length2       length of string 2, or -1 (NULL terminated)
 * @param options       compare options
 * @param matchLen1     (output) length of partial prefix match in s1
 * @param matchLen2     (output) length of partial prefix match in s2
 * @param pErrorCode    receives error status
 */
U_CAPI void
u_caseInsensitivePrefixMatch(const UChar *s1, int32_t length1,
                             const UChar *s2, int32_t length2,
                             uint32_t options,
                             int32_t *matchLen1, int32_t *matchLen2,
                             UErrorCode *pErrorCode);

/**
 * Are the Unicode properties loaded?
 * This must be used before internal functions are called that do
 * not perform this check.
 * Generate a debug assertion failure if data is not loaded.
 */
U_CFUNC UBool
uprv_haveProperties(UErrorCode *pErrorCode);

#ifdef __cplusplus

#include "unicode/unistr.h"  // for UStringCaseMapper

/*
 * Internal string casing functions implementing
 * ustring.h/ustrcase.cpp and UnicodeString case mapping functions.
 */

struct UCaseMap : public icu::UMemory {
    /** Implements most of ucasemap_open(). */
    UCaseMap(const char *localeID, uint32_t opts, UErrorCode *pErrorCode);
    ~UCaseMap();

#if !UCONFIG_NO_BREAK_ITERATION
    icu::BreakIterator *iter;  /* We adopt the iterator, so we own it. */
#endif
    char locale[32];
    int32_t caseLocale;
    uint32_t options;
};

#if UCONFIG_NO_BREAK_ITERATION
#   define UCASEMAP_BREAK_ITERATOR_PARAM
#   define UCASEMAP_BREAK_ITERATOR_UNUSED
#   define UCASEMAP_BREAK_ITERATOR
#   define UCASEMAP_BREAK_ITERATOR_NULL
#else
#   define UCASEMAP_BREAK_ITERATOR_PARAM icu::BreakIterator *iter,
#   define UCASEMAP_BREAK_ITERATOR_UNUSED icu::BreakIterator *,
#   define UCASEMAP_BREAK_ITERATOR iter,
#   define UCASEMAP_BREAK_ITERATOR_NULL NULL,
#endif

U_CFUNC int32_t
ustrcase_getCaseLocale(const char *locale);

// TODO: swap src / dest if approved for new public api
/** Implements UStringCaseMapper. */
U_CFUNC int32_t U_CALLCONV
ustrcase_internalToLower(int32_t caseLocale, uint32_t options, UCASEMAP_BREAK_ITERATOR_PARAM
                         UChar *dest, int32_t destCapacity,
                         const UChar *src, int32_t srcLength,
                         icu::Edits *edits,
                         UErrorCode &errorCode);

/** Implements UStringCaseMapper. */
U_CFUNC int32_t U_CALLCONV
ustrcase_internalToUpper(int32_t caseLocale, uint32_t options, UCASEMAP_BREAK_ITERATOR_PARAM
                         UChar *dest, int32_t destCapacity,
                         const UChar *src, int32_t srcLength,
                         icu::Edits *edits,
                         UErrorCode &errorCode);

#if !UCONFIG_NO_BREAK_ITERATION

/** Implements UStringCaseMapper. */
U_CFUNC int32_t U_CALLCONV
ustrcase_internalToTitle(int32_t caseLocale, uint32_t options,
                         icu::BreakIterator *iter,
                         UChar *dest, int32_t destCapacity,
                         const UChar *src, int32_t srcLength,
                         icu::Edits *edits,
                         UErrorCode &errorCode);

#endif

/** Implements UStringCaseMapper. */
U_CFUNC int32_t U_CALLCONV
ustrcase_internalFold(int32_t caseLocale, uint32_t options, UCASEMAP_BREAK_ITERATOR_PARAM
                      UChar *dest, int32_t destCapacity,
                      const UChar *src, int32_t srcLength,
                      icu::Edits *edits,
                      UErrorCode &errorCode);

/**
 * Common string case mapping implementation for ucasemap_toXyz() and UnicodeString::toXyz().
 * Implements argument checking.
 */
U_CFUNC int32_t
ustrcase_map(int32_t caseLocale, uint32_t options, UCASEMAP_BREAK_ITERATOR_PARAM
             UChar *dest, int32_t destCapacity,
             const UChar *src, int32_t srcLength,
             UStringCaseMapper *stringCaseMapper,
             icu::Edits *edits,
             UErrorCode &errorCode);

/**
 * Common string case mapping implementation for old-fashioned u_strToXyz() functions
 * that allow the source string to overlap the destination buffer.
 * Implements argument checking and internally works with an intermediate buffer if necessary.
 */
U_CFUNC int32_t
ustrcase_mapWithOverlap(int32_t caseLocale, uint32_t options, UCASEMAP_BREAK_ITERATOR_PARAM
                        UChar *dest, int32_t destCapacity,
                        const UChar *src, int32_t srcLength,
                        UStringCaseMapper *stringCaseMapper,
                        UErrorCode &errorCode);

/**
 * UTF-8 string case mapping function type, used by ucasemap_mapUTF8().
 * UTF-8 version of UStringCaseMapper.
 * All error checking must be done.
 * The UCaseMap must be fully initialized, with locale and/or iter set as needed.
 * src and dest must not overlap.
 */
typedef int32_t U_CALLCONV
UTF8CaseMapper(int32_t caseLocale, uint32_t options,
#if !UCONFIG_NO_BREAK_ITERATION
               icu::BreakIterator *iter,
#endif
               uint8_t *dest, int32_t destCapacity,
               const uint8_t *src, int32_t srcLength,
               icu::Edits *edits,
               UErrorCode &errorCode);

#if !UCONFIG_NO_BREAK_ITERATION

/** Implements UTF8CaseMapper. */
U_CFUNC int32_t U_CALLCONV
ucasemap_internalUTF8ToTitle(int32_t caseLocale, uint32_t options,
        icu::BreakIterator *iter,
        uint8_t *dest, int32_t destCapacity,
        const uint8_t *src, int32_t srcLength,
        icu::Edits *edits,
        UErrorCode &errorCode);

#endif

/**
 * Implements argument checking and buffer handling
 * for UTF-8 string case mapping as a common function.
 */
U_CFUNC int32_t
ucasemap_mapUTF8(int32_t caseLocale, uint32_t options, UCASEMAP_BREAK_ITERATOR_PARAM
                 uint8_t *dest, int32_t destCapacity,
                 const uint8_t *src, int32_t srcLength,
                 UTF8CaseMapper *stringCaseMapper,
                 icu::Edits *edits,
                 UErrorCode &errorCode);

U_NAMESPACE_BEGIN
namespace GreekUpper {

// Data bits.
static const uint32_t UPPER_MASK = 0x3ff;
static const uint32_t HAS_VOWEL = 0x1000;
static const uint32_t HAS_YPOGEGRAMMENI = 0x2000;
static const uint32_t HAS_ACCENT = 0x4000;
static const uint32_t HAS_DIALYTIKA = 0x8000;
// Further bits during data building and processing, not stored in the data map.
static const uint32_t HAS_COMBINING_DIALYTIKA = 0x10000;
static const uint32_t HAS_OTHER_GREEK_DIACRITIC = 0x20000;

static const uint32_t HAS_VOWEL_AND_ACCENT = HAS_VOWEL | HAS_ACCENT;
static const uint32_t HAS_VOWEL_AND_ACCENT_AND_DIALYTIKA =
        HAS_VOWEL_AND_ACCENT | HAS_DIALYTIKA;
static const uint32_t HAS_EITHER_DIALYTIKA = HAS_DIALYTIKA | HAS_COMBINING_DIALYTIKA;

// State bits.
static const uint32_t AFTER_CASED = 1;
static const uint32_t AFTER_VOWEL_WITH_ACCENT = 2;

uint32_t getLetterData(UChar32 c);

/**
 * Returns a non-zero value for each of the Greek combining diacritics
 * listed in The Unicode Standard, version 8, chapter 7.2 Greek,
 * plus some perispomeni look-alikes.
 */
uint32_t getDiacriticData(UChar32 c);

}  // namespace GreekUpper
U_NAMESPACE_END

#endif  // __cplusplus

#endif  // __UCASEMAP_IMP_H__
