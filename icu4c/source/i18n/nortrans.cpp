/*
**********************************************************************
*   Copyright (C) 2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*   Date        Name        Description
*   07/03/01    aliu        Creation.
**********************************************************************
*/

#include "unicode/nortrans.h"

U_NAMESPACE_BEGIN

/**
 * System registration hook.
 */
void NormalizationTransliterator::registerIDs() {
    Transliterator::_registerFactory(UnicodeString("Any-NFC", ""),
                                     _create, integerToken(UNORM_NFC));
    Transliterator::_registerFactory(UnicodeString("Any-NFKC", ""),
                                     _create, integerToken(UNORM_NFKC));
    Transliterator::_registerFactory(UnicodeString("Any-NFD", ""),
                                     _create, integerToken(UNORM_NFD));
    Transliterator::_registerFactory(UnicodeString("Any-NFKD", ""),
                                     _create, integerToken(UNORM_NFKD));
}

/**
 * Factory methods
 */
Transliterator* NormalizationTransliterator::_create(const UnicodeString& ID,
                                                     Token context) {
    return new NormalizationTransliterator(ID, (UNormalizationMode) context.integer, 0);
}

/**
 * Constructs a transliterator.
 */
NormalizationTransliterator::NormalizationTransliterator(
                                 const UnicodeString& id,
                                 UNormalizationMode mode, int32_t opt) :
    Transliterator(id, 0) {
    fMode = mode;
    options = opt;
}

/**
 * Destructor.
 */
NormalizationTransliterator::~NormalizationTransliterator() {
}

/**
 * Copy constructor.
 */
NormalizationTransliterator::NormalizationTransliterator(const NormalizationTransliterator& o) :
Transliterator(o) {
    fMode = o.fMode;
    options = o.options;
}

/**
 * Assignment operator.
 */
NormalizationTransliterator& NormalizationTransliterator::operator=(const NormalizationTransliterator& o) {
    Transliterator::operator=(o);
    fMode = o.fMode;
    options = o.options;
    return *this;
}

/**
 * Transliterator API.
 */
Transliterator* NormalizationTransliterator::clone(void) const {
    return new NormalizationTransliterator(*this);
}

// TODO
// TODO
// TODO
// Get rid of this function and use the official Replaceable
// extractBetween() method, when possible
// TODO
// TODO
// TODO
static void _Replaceable_extractBetween(const Replaceable& text,
                                        int32_t start,
                                        int32_t limit,
                                        UChar* buffer) {
    while (start < limit) {
        *buffer++ = text.charAt(start++);
    }
}

/**
 * Implements {@link Transliterator#handleTransliterate}.
 */
void NormalizationTransliterator::handleTransliterate(Replaceable& text, UTransPosition& offsets,
                                                      UBool isIncremental) const {
    int32_t start = offsets.start;
    int32_t limit = offsets.limit;

    // For the non-incremental case normalize right up to
    // offsets.limit.  In the incremental case, find the last base
    // character b, and pass everything from the start up to the
    // character before b to normalizer.
    if (isIncremental) {
        // Wrinkle: Jamo has a combining class of zero, but we
        // don't want to normalize individual Jamo one at a time
        // if we're composing incrementally.  If we are composing
        // in incremental mode then we collect up trailing jamo
        // and save them for next time.
        UBool doStandardBackup = TRUE;
        if (fMode == UNORM_NFC || fMode == UNORM_NFKC) {
            // As a minor optimization, if there are three or more
            // trailing jamo, we let the first three through --
            // these should be handled correctly.
            UChar c;
            while (limit > offsets.start &&
                   (c=text.charAt(limit-1)) >= 0x1100 &&
                   c < 0x1200) {
                --limit;
            }
            // Characters in [limit, offsets.limit) are jamo.
            // If we have at least 3 jamo, then allow them
            // to be transliterated.  If we have zero jamo,
            // then proceed as usual.
            if (limit < offsets.limit) {
                if ((offsets.limit - limit) >= 3) {
                    limit += 3;
                }
                doStandardBackup = FALSE;
            }
        }

        if (doStandardBackup) {
            --limit;
            while (limit > start &&
                   u_getCombiningClass(text.charAt(limit)) != 0) {
                --limit;
            }
        }
    }

    if (limit > start) {

        UChar staticChars[256];
        UChar* chars = staticChars;

        if ((limit - start) > 255) {
            // Allocate extra buffer space if needed
            chars = new UChar[limit-start+1];
            if (chars == NULL) {
                return;
            }
        }

        _Replaceable_extractBetween(text, start, limit, chars);

        UnicodeString input(FALSE, chars, limit-start); // readonly alias
        UnicodeString output;
        UErrorCode status = U_ZERO_ERROR;
        Normalizer::normalize(input, fMode, options, output, status);

        if (chars != staticChars) {
            delete[] chars;
        }

        text.handleReplaceBetween(start, limit, output);

        int32_t delta = output.length() - input.length();
        offsets.contextLimit += delta;
        offsets.limit += delta;
        offsets.start = limit + delta;
    }
}

U_NAMESPACE_END

