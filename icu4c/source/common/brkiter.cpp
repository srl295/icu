/*
*******************************************************************************
* Copyright (C) 1997-2001, International Business Machines Corporation and    *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
* File TXTBDRY.CPP
*
* Modification History:
*
*   Date        Name        Description
*   02/18/97    aliu        Converted from OpenClass.  Added DONE.
*   01/13/2000  helena      Added UErrorCode parameter to createXXXInstance methods.
*****************************************************************************************
*/

// *****************************************************************************
// This file was generated from the java source file BreakIterator.java
// *****************************************************************************

#include "unicode/dbbi.h"
#include "unicode/brkiter.h"
#include "unicode/udata.h"
#include "unicode/resbund.h"
#include "cstring.h"
#include <string.h>

// *****************************************************************************
// class BreakIterator
// This class implements methods for finding the location of boundaries in text. 
// Instances of BreakIterator maintain a current position and scan over text
// returning the index of characters where boundaries occur.
// *****************************************************************************

U_NAMESPACE_BEGIN

const int32_t BreakIterator::DONE = (int32_t)-1;

// -------------------------------------

// Creates a simple text boundary for word breaks.
BreakIterator*
BreakIterator::createWordInstance(const Locale& key, UErrorCode& status)
{
    // WARNING: This routine is currently written specifically to handle only the
    // default rules files and the alternate rules files for Thai.  This function
    // will have to be made fully general at some time in the future!
    BreakIterator* result = NULL;
    const char* filename = "word";

    if (U_FAILURE(status))
        return NULL;
    if (!uprv_strcmp(key.getLanguage(), "th"))
    {
        filename = "word_th";
    }

    UDataMemory* file = udata_open(NULL, "brk", filename, &status);

    if (U_SUCCESS(status)) {
        if(!uprv_strcmp(filename, "word_th")) {
            filename = "thaidict.brk";
            result = new DictionaryBasedBreakIterator(file, filename, status);
            /* test for NULL */
            if(result == 0) {
                status = U_MEMORY_ALLOCATION_ERROR;
                return 0;
            }
        }
        else {
            result = new RuleBasedBreakIterator(file, status);
            /* test for NULL */
            if(result == 0) {
                status = U_MEMORY_ALLOCATION_ERROR;
                return 0;
            }
        }
    }

    return result;
}

// -------------------------------------

// Creates a simple text boundary for line breaks.
BreakIterator*
BreakIterator::createLineInstance(const Locale& key, UErrorCode& status)
{
    // WARNING: This routine is currently written specifically to handle only the
    // default rules files and the alternate rules files for Thai.  This function
    // will have to be made fully general at some time in the future!
    BreakIterator* result = NULL;
    const char* filename = "line";

    if (U_FAILURE(status))
        return NULL;
    if (!uprv_strcmp(key.getLanguage(), "th"))
    {
        filename = "line_th";
    }

    UDataMemory* file = udata_open(NULL, "brk", filename, &status);

    if (!U_FAILURE(status)) {
        if (!uprv_strcmp(key.getLanguage(), "th")) {
            filename = "thaidict.brk";
            result = new DictionaryBasedBreakIterator(file, filename, status);
            /* test for NULL */
            if(result == 0) {
                status = U_MEMORY_ALLOCATION_ERROR;
                return 0;
            }
        }
        else {
            result = new RuleBasedBreakIterator(file, status);
            /* test for NULL */
            if(result == 0) {
                status = U_MEMORY_ALLOCATION_ERROR;
                return 0;
            }
        }
    }

    return result;
}

// -------------------------------------

// Creates a simple text boundary for character breaks.
BreakIterator*
BreakIterator::createCharacterInstance(const Locale& key, UErrorCode& status)
{
    // WARNING: This routine is currently written specifically to handle only the
    // default rules files and the alternate rules files for Thai.  This function
    // will have to be made fully general at some time in the future!
    BreakIterator* result = NULL;
    static const char filename[] = "char";

    if (U_FAILURE(status))
        return NULL;
    UDataMemory* file = udata_open(NULL, "brk", filename, &status);

    if (!U_FAILURE(status)) {
        result = new RuleBasedBreakIterator(file, status);
        /* test for NULL */
        if(result == 0) {
            status = U_MEMORY_ALLOCATION_ERROR;
            return 0;
        }
    }

    return result;
}

// -------------------------------------

// Creates a simple text boundary for sentence breaks.
BreakIterator*
BreakIterator::createSentenceInstance(const Locale& key, UErrorCode& status)
{
    // WARNING: This routine is currently written specifically to handle only the
    // default rules files and the alternate rules files for Thai.  This function
    // will have to be made fully general at some time in the future!
    BreakIterator* result = NULL;
    static const char filename[] = "sent";

    if (U_FAILURE(status))
        return NULL;
    UDataMemory* file = udata_open(NULL, "brk", filename, &status);

    if (!U_FAILURE(status)) {
        result = new RuleBasedBreakIterator(file, status);
        /* test for NULL */
            if(result == 0) {
                status = U_MEMORY_ALLOCATION_ERROR;
                return 0;
            }
    }

    return result;
}

// -------------------------------------

// Creates a simple text boundary for title casing breaks.
BreakIterator*
BreakIterator::createTitleInstance(const Locale& key, UErrorCode& status)
{
    // WARNING: This routine is currently written specifically to handle only the
    // default rules files.  This function will have to be made fully general 
    // at some time in the future!
    BreakIterator* result = NULL;
    static const char filename[] = "title";

    if (U_FAILURE(status))
        return NULL;
    UDataMemory* file = udata_open(NULL, "brk", filename, &status);

    if (!U_FAILURE(status)) {
        result = new RuleBasedBreakIterator(file, status);
        /* test for NULL */
        if(result == 0) {
            status = U_MEMORY_ALLOCATION_ERROR;
            return 0;
        }
    }

    return result;
}
// -------------------------------------

// Gets all the available locales that has localized text boundary data.
const Locale*
BreakIterator::getAvailableLocales(int32_t& count)
{
    return Locale::getAvailableLocales(count);
}

// -------------------------------------
// Gets the objectLocale display name in the default locale language.
UnicodeString&
BreakIterator::getDisplayName(const Locale& objectLocale,
                             UnicodeString& name)
{
    return objectLocale.getDisplayName(name);
}

// -------------------------------------
// Gets the objectLocale display name in the displayLocale language.
UnicodeString&
BreakIterator::getDisplayName(const Locale& objectLocale,
                             const Locale& displayLocale,
                             UnicodeString& name)
{
    return objectLocale.getDisplayName(displayLocale, name);
}

// -------------------------------------

// Needed because we declare the copy constructor (in order to prevent synthesizing one) and
// so the default constructor is no longer synthesized.

BreakIterator::BreakIterator()
{
    fBufferClone = FALSE;
}

BreakIterator::~BreakIterator()
{
}

U_NAMESPACE_END

//eof
