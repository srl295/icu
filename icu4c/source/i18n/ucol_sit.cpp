/*
*******************************************************************************
*   Copyright (C) 2004, International Business Machines
*   Corporation and others.  All Rights Reserved.
*******************************************************************************
*   file name:  ucol_sit.cpp
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
* Modification history
* Date        Name      Comments
* 03/12/2004  weiv      Creation
*/

#include "utracimp.h"
#include "ucol_imp.h"
#include "unormimp.h"
#include "cmemory.h"
#include "cstring.h"



enum OptionsList {
    UCOL_SIT_LANGUAGE = 0,
    UCOL_SIT_SCRIPT,
    UCOL_SIT_REGION,
    UCOL_SIT_VARIANT,
    UCOL_SIT_KEYWORD,
    UCOL_SIT_RFC3166BIS,
    UCOL_SIT_STRENGTH,
    UCOL_SIT_CASE_LEVEL,
    UCOL_SIT_CASE_FIRST,
    UCOL_SIT_NUMERIC_COLLATION,
    UCOL_SIT_ALTERNATE_HANDLING,
    UCOL_SIT_NORMALIZATION_MODE,
    UCOL_SIT_FRENCH_COLLATION,
    UCOL_SIT_HIRAGANA_QUATERNARY,
    UCOL_SIT_VARIABLE_TOP,
    UCOL_SIT_VARIABLE_TOP_VALUE,
    UCOL_SIT_ITEMS_COUNT
};

/* list of locales for packing of a collator to an integer.
 * This list corresponds to ICU 3.0. If more collation bearing
 * locales are added in the future, this won't be a simple array
 * but a mapping allowing forward and reverse lookup would have to 
 * be established. Currently, the mapping is from locale name to 
 * index.
 */
static const char* locales[] = {
/* 00 - 09 */ "ar", "be", "bg", "ca", "cs", "da", "de", "de__PHONEBOOK", "el", "en",
/* 10 - 19 */ "en_BE", "eo", "es", "es__TRADITIONAL", "et", "fa", "fa_AF", "fi", "fo", "fr",
/* 20 - 29 */ "gu", "he", "hi", "hi__DIRECT", "hr", "hu", "is", "it", "ja", "kk",
/* 30 - 39 */ "kl", "kn", "ko", "lt", "lv", "mk", "mr", "mt", "nb", "nn",
/* 40 - 49 */ "om", "pa", "pl", "ps", "ro", "root", "ru", "sh", "sk", "sl",
/* 50 - 59 */ "sq", "sr", "sv", "ta", "te", "th", "tr", "uk", "vi", "zh",
/* 60 - 64 */ "zh_HK", "zh_MO", "zh_TW", "zh_TW_STROKE", "zh__PINYIN"
};

/* option starters chars. */
static const char alternateHArg     = 'A';
static const char variableTopValArg = 'B';
static const char caseFirstArg      = 'C';
static const char numericCollArg    = 'D';
static const char caseLevelArg      = 'E';
static const char frenchCollArg     = 'F';
static const char hiraganaQArg      = 'H';
static const char keywordArg        = 'K';
static const char languageArg       = 'L';
static const char normArg           = 'N';
static const char regionArg         = 'R';
static const char strengthArg       = 'S';
static const char variableTopArg    = 'T';
static const char variantArg        = 'V';
static const char RFC3066Arg        = 'X';
static const char scriptArg         = 'Z';

static const int32_t locElementCount = 5;
static const int32_t locElementCapacity = 32;
static const int32_t loc3066Capacity = 256;
static const int32_t internalBufferSize = 512;

/* structure containing specification of a collator. Initialized
 * from a short string. Also used to construct a short string from a
 * collator instance
 */
struct CollatorSpec {
    char locElements[locElementCount][locElementCapacity];
    char locale[loc3066Capacity];
    UColAttributeValue options[UCOL_ATTRIBUTE_COUNT];
    uint32_t variableTopValue;
    UChar variableTopString[locElementCapacity];
    UBool variableTopSet;
    struct {
        const char *start;
        int32_t len;
    } entries[UCOL_SIT_ITEMS_COUNT];
};


/* structure for converting between character attribute
 * representation and real collation attribute value.
 */
struct AttributeConversion {
    char letter;
    UColAttributeValue value;
};

static AttributeConversion conversions[12] = {
    { '1', UCOL_PRIMARY },
    { '2', UCOL_SECONDARY },
    { '3', UCOL_TERTIARY },
    { '4', UCOL_QUATERNARY },
    { 'D', UCOL_DEFAULT },
    { 'I', UCOL_IDENTICAL },
    { 'L', UCOL_LOWER_FIRST },
    { 'N', UCOL_NON_IGNORABLE },
    { 'O', UCOL_ON },
    { 'S', UCOL_SHIFTED },
    { 'U', UCOL_UPPER_FIRST },
    { 'X', UCOL_OFF }
};

/* function prototype for functions used to parse a short string */
U_CDECL_BEGIN
typedef const char* U_CALLCONV
ActionFunction(CollatorSpec *spec, uint32_t value1, const char* string,
               UErrorCode *status);
U_CDECL_END

U_CDECL_BEGIN
static const char* U_CALLCONV
_processLocaleElement(CollatorSpec *spec, uint32_t value, const char* string, 
                      UErrorCode *status) 
{
    int32_t len = 0;
    do {
        if(value == 0 || value == 4) {
            spec->locElements[value][len++] = uprv_tolower(*string);
        } else {
            spec->locElements[value][len++] = *string;
        }
    } while(*(++string) != '_' && *string && len < locElementCapacity);
    if(len >= locElementCapacity) {
        *status = U_BUFFER_OVERFLOW_ERROR;
        return string;
    }
    // skip the underscore at the end
    return ++string;
}
U_CDECL_END

U_CDECL_BEGIN
static const char* U_CALLCONV
_processRFC3066Locale(CollatorSpec *spec, uint32_t value1, const char* string, 
                      UErrorCode *status) 
{
    char terminator = *string;
    string++;
    const char *end = uprv_strchr(string+1, terminator);
    if(end - string > loc3066Capacity) {
        *status = U_BUFFER_OVERFLOW_ERROR;
        return string;
    } else {
        uprv_strncpy(spec->locale, string, end-string);
        return end+2;
    }
}

U_CDECL_END

U_CDECL_BEGIN
static const char* U_CALLCONV
_processCollatorOption(CollatorSpec *spec, uint32_t option, const char* string, 
                       UErrorCode *status) 
{
    int32_t i = 0;
    for(i = 0; i < (int32_t)(sizeof(conversions)/sizeof(conversions[0])); i++) {
        if(*string == conversions[i].letter) {
            spec->options[option] = conversions[i].value;
            if(*(++string) != '_' && *string) {
                *status = U_ILLEGAL_ARGUMENT_ERROR;
                return string;
            }
            return ++string;
        }
    }
    *status = U_ILLEGAL_ARGUMENT_ERROR;
    return string;
}
U_CDECL_END


static UChar 
readHexCodeUnit(const char **string, UErrorCode *status) 
{
    UChar result = 0;
    int32_t value = 0;
    char c;
    int32_t noDigits = 0;
    while((c = **string) != 0 && noDigits < 4) {
        if( c >= '0' && c <= '9') {
            value = c - '0';
        } else if ( c >= 'a' && c <= 'f') {
            value = c - 'a' + 10;
        } else if ( c >= 'A' && c <= 'F') {
            value = c - 'A' + 10;
        } else {
            *status = U_ILLEGAL_ARGUMENT_ERROR;
            return 0;
        }
        result = (result << 4) | value;
        noDigits++;
        (*string)++;
    }
    return result;
}

U_CDECL_BEGIN
static const char* U_CALLCONV
_processVariableTop(CollatorSpec *spec, uint32_t value1, const char* string, UErrorCode *status) 
{
    // get four digits
    int32_t i = 0;
    if(!value1) {
        while(U_SUCCESS(*status) && *string != 0 && *string != '_') {
            spec->variableTopString[i++] = readHexCodeUnit(&string, status);
        }
    } else {
        spec->variableTopValue = readHexCodeUnit(&string, status);
    }
    if(U_SUCCESS(*status)) {
        spec->variableTopSet = TRUE;
        return string+1; // to move over the '_'
    } else {
        return string;
    }
}
U_CDECL_END


/* Table for parsing short strings */
struct ShortStringOptions {
    char optionStart;
    ActionFunction *action;
    uint32_t attr;
};

static ShortStringOptions options[UCOL_SIT_ITEMS_COUNT] =
{
/* 10 ALTERNATE_HANDLING */   {alternateHArg,     _processCollatorOption, UCOL_ALTERNATE_HANDLING }, // alternate  N, S, D 
/* 15 VARIABLE_TOP_VALUE */   {variableTopValArg, _processVariableTop,    1 },
/* 08 CASE_FIRST */           {caseFirstArg,      _processCollatorOption, UCOL_CASE_FIRST }, // case first L, U, X, D
/* 09 NUMERIC_COLLATION */    {numericCollArg,    _processCollatorOption, UCOL_NUMERIC_COLLATION }, // codan      O, X, D
/* 07 CASE_LEVEL */           {caseLevelArg,      _processCollatorOption, UCOL_CASE_LEVEL }, // case level O, X, D
/* 12 FRENCH_COLLATION */     {frenchCollArg,     _processCollatorOption, UCOL_FRENCH_COLLATION }, // french     O, X, D
/* 13 HIRAGANA_QUATERNARY] */ {hiraganaQArg,      _processCollatorOption, UCOL_HIRAGANA_QUATERNARY_MODE }, // hiragana   O, X, D
/* 04 KEYWORD */              {keywordArg,        _processLocaleElement,  4 }, // keyword
/* 00 LANGUAGE */             {languageArg,       _processLocaleElement,  0 }, // language
/* 11 NORMALIZATION_MODE */   {normArg,           _processCollatorOption, UCOL_NORMALIZATION_MODE }, // norm       O, X, D
/* 02 REGION */               {regionArg,         _processLocaleElement,  2 }, // region
/* 06 STRENGTH */             {strengthArg,       _processCollatorOption, UCOL_STRENGTH }, // strength   1, 2, 3, 4, I, D
/* 14 VARIABLE_TOP */         {variableTopArg,    _processVariableTop,    0 },
/* 03 VARIANT */              {variantArg,        _processLocaleElement,  3 }, // variant
/* 05 RFC3066BIS */           {RFC3066Arg,        _processRFC3066Locale,  0 }, // rfc3066bis locale name
/* 01 SCRIPT */               {scriptArg,         _processLocaleElement,  1 }  // script
};


static
const char* ucol_sit_readOption(const char *start, CollatorSpec *spec, 
                            UErrorCode *status) 
{
  int32_t i = 0, j = 0, k = 0;

  for(i = 0; i < UCOL_SIT_ITEMS_COUNT; i++) {
      if(*start == options[i].optionStart) {
          spec->entries[i].start = start;
          const char* end = options[i].action(spec, options[i].attr, start+1, status);
          spec->entries[i].len = end - start - 1;
          return end;
      }
  }
  *status = U_ILLEGAL_ARGUMENT_ERROR;
  return start;
}

static
void ucol_sit_initCollatorSpecs(CollatorSpec *spec) 
{
    // reset everything
    uprv_memset(spec, 0, sizeof(CollatorSpec));
    // set collation options to default
    int32_t i = 0;
    for(i = 0; i < UCOL_ATTRIBUTE_COUNT; i++) {
        spec->options[i] = UCOL_DEFAULT;
    }
}

static
void ucol_sit_readSpecs(CollatorSpec *s, const char *string, 
                        UParseError *parseError, UErrorCode *status)
{
    const char *definition = string;
    while(U_SUCCESS(*status) && *string) { 
        string = ucol_sit_readOption(string, s, status);
    }
    if(U_FAILURE(*status)) {
        parseError->line = 0;
        parseError->offset = string - definition;
        // perhaps just stuff chars in UChar[]?
        parseError->preContext[0] = 0;
        parseError->postContext[0] = 0;
    }
}

static
int32_t ucol_sit_dumpSpecs(CollatorSpec *s, char *destination, UErrorCode *status)
{
    int32_t i = 0, j = 0;
    int32_t len = 0;
    char optName; 
    if(U_SUCCESS(*status)) {
        for(i = 0; i < UCOL_SIT_ITEMS_COUNT; i++) {
            if(s->entries[i].start) {
                if(len) {
                    uprv_strcat(destination, "_");
                    len++;
                } 
                optName = *(s->entries[i].start);
                if(optName == languageArg || optName == regionArg || optName == variantArg) {
                    for(j = 0; j < s->entries[i].len; j++) {
                        destination[len++] = uprv_toupper(*(s->entries[i].start+j));
                    }
                } else {
                    uprv_strncat(destination,s->entries[i].start, s->entries[i].len);
                    len += s->entries[i].len;
                }
            }
        }
        return len;
    } else {
        return 0;
    }
}

/** 
 * Open a collator defined by a short form string.
 * The structure and the syntax of the string is defined in the "Naming collators"
 * section of the users guide:
 * http://oss.software.ibm.com/icu/userguide/Collate_Concepts.html#Naming_Collators
 * The call to this function is equivalent to a call to ucol_open, followed by a
 * series of calls to ucol_setAttribute and ucol_setVariableTop.
 * Attributes are overriden by the subsequent attributes. So, for "S2_S3", final
 * strength will be 3. 3066bis locale overrides individual locale parts.
 * @param definition A short string containing a locale and a set of attributes.
 *                   Attributes not explicitly mentioned are left at the default
 *                   state for a locale.
 * @param parseError if not NULL, structure that will get filled with error's pre
 *                   and post context in case of error.
 * @param status     Error code. Apart from regular error conditions connected to
 *                   instantiating collators (like out of memory or similar), this
 *                   API will return an error if an invalid attribute or attribute/value
 *                   combination is specified.
 * @return           A pointer to a UCollator or 0 if an error occured (including an
 *                   invalid attribute).
 * @see ucol_open
 * @see ucol_setAttribute
 * @see ucol_setVariableTop
 * @draft ICU 3.0
 *
 */
U_CAPI UCollator* U_EXPORT2
ucol_openFromShortString( const char *definition,
                          UParseError *parseError,
                          UErrorCode *status)
{
    UTRACE_ENTRY_OC(UTRACE_UCOL_OPEN_FROM_SHORT_STRING);
    UTRACE_DATA1(UTRACE_INFO, "short string = \"%s\"", definition);

    if(U_FAILURE(*status)) return 0;

    // first we want to pick stuff out of short string.
    // we'll end up with an UCA version, locale and a bunch of
    // settings

    // analyse the string in order to get everything we need.
    int32_t definitionLen = uprv_strlen(definition);
    const char *string = definition;
    CollatorSpec s;
    ucol_sit_initCollatorSpecs(&s);
    ucol_sit_readSpecs(&s, definition, parseError, status);

    
    // put the locale together, unless we have a done
    // locale
    int32_t i = 0;
    if(s.locale[0] == 0) {
        // first the language
        uprv_strcat(s.locale, s.locElements[0]);
        // then the script, if present
        if(*(s.locElements[1])) {
            uprv_strcat(s.locale, "_");
            uprv_strcat(s.locale, s.locElements[1]);
        }
        // then the region, if present
        if(*(s.locElements[2])) {
            uprv_strcat(s.locale, "_");
            uprv_strcat(s.locale, s.locElements[2]);
        } else if(*(s.locElements[3])) { // if there is a variant, we need an underscore
            uprv_strcat(s.locale, "_");
        }
        // add variant, if there
        if(*(s.locElements[3])) {
            uprv_strcat(s.locale, "_");
            uprv_strcat(s.locale, s.locElements[3]);
        }

        // if there is a collation keyword, add that too
        if(*(s.locElements[4])) {
            uprv_strcat(s.locale, "@collation=");
            uprv_strcat(s.locale, s.locElements[4]);
        }
    }
    char buffer[internalBufferSize];
    uprv_memset(buffer, 0, internalBufferSize);
    uloc_canonicalize(s.locale, buffer, internalBufferSize, status);

    UCollator *result = ucol_open(s.locale, status);

    for(i = 0; i < UCOL_ATTRIBUTE_COUNT; i++) {
        if(s.options[i] != UCOL_DEFAULT) {
            if(ucol_getAttribute(result, (UColAttribute)i, status) != s.options[i]) {
                ucol_setAttribute(result, (UColAttribute)i, s.options[i], status);
            }
        }
    }
    if(s.variableTopSet) {
        if(s.variableTopString[0]) {
            ucol_setVariableTop(result, s.variableTopString, u_strlen(s.variableTopString), status);
        } else { // we set by value, using 'B'
            ucol_restoreVariableTop(result, s.variableTopValue, status);
        }
    }


    if(U_FAILURE(*status)) { // here it can only be a bogus value
        ucol_close(result);
        result = NULL;
    }

    UTRACE_EXIT_PTR_STATUS(result, *status);
    return result;
}


static void appendShortStringElement(const char *src, int32_t len, char *result, int32_t *resultSize, char arg)
{
    if(len) {
        if(*resultSize) {
            uprv_strcat(result, "_");
            (*resultSize)++;
        }
        *resultSize += len + 1;
        uprv_strncat(result, &arg, 1);
        uprv_strcat(result, src);
    }
}

U_CAPI int32_t U_EXPORT2
ucol_getShortDefinitionString(const UCollator *coll,
                              const char *locale,
                              char *dst,
                              int32_t capacity,
                              UErrorCode *status)
{
    if(U_FAILURE(*status)) return 0;
    char buffer[internalBufferSize];
    uprv_memset(buffer, 0, internalBufferSize*sizeof(char));
    int32_t resultSize = 0;
    char tempbuff[internalBufferSize];
    char locBuff[internalBufferSize];
    uprv_memset(buffer, 0, internalBufferSize*sizeof(char));
    int32_t elementSize = 0;
    UBool isAvailable = 0;
    CollatorSpec s;
    ucol_sit_initCollatorSpecs(&s);

    if(!locale) {
        locale = ucol_getLocale(coll, ULOC_VALID_LOCALE, status);
    }
    elementSize = ucol_getFunctionalEquivalent(locBuff, internalBufferSize, "collation", locale, &isAvailable, status);

    if(elementSize) {
        // we should probably canonicalize here...
        elementSize = uloc_getLanguage(locBuff, tempbuff, internalBufferSize, status);
        appendShortStringElement(tempbuff, elementSize, buffer, &resultSize, languageArg);
        elementSize = uloc_getCountry(locBuff, tempbuff, internalBufferSize, status);
        appendShortStringElement(tempbuff, elementSize, buffer, &resultSize, regionArg);
        elementSize = uloc_getScript(locBuff, tempbuff, internalBufferSize, status);
        appendShortStringElement(tempbuff, elementSize, buffer, &resultSize, scriptArg);
        elementSize = uloc_getVariant(locBuff, tempbuff, internalBufferSize, status);
        appendShortStringElement(tempbuff, elementSize, buffer, &resultSize, variantArg);
        elementSize = uloc_getKeywordValue(locBuff, "collation", tempbuff, internalBufferSize, status);
        appendShortStringElement(tempbuff, elementSize, buffer, &resultSize, keywordArg);
    } 

    int32_t i = 0, j = 0;
    UColAttributeValue attribute = UCOL_DEFAULT;
    for(i = 0; i < UCOL_SIT_ITEMS_COUNT; i++) {
        if(options[i].action == _processCollatorOption) {
            attribute = ucol_getAttributeOrDefault(coll, (UColAttribute)options[i].attr, status);
            if(attribute != UCOL_DEFAULT) {
                for(j = 0; j < sizeof(conversions)/sizeof(conversions[0]); j++) {
                    if(attribute == conversions[j].value) {
                        appendShortStringElement(&(conversions[j].letter), 1, 
                            buffer, &resultSize, options[i].optionStart);
                        break;
                    }
                }
            }
        }
    }
    if(coll->variableTopValueisDefault == FALSE) {
        //s.variableTopValue = ucol_getVariableTop(coll, status);
        elementSize = T_CString_integerToString(tempbuff, coll->variableTopValue, 16);
        appendShortStringElement(tempbuff, elementSize, buffer, &resultSize, variableTopValArg);
    }

    UParseError parseError;
    return ucol_normalizeShortDefinitionString(buffer, dst, capacity, &parseError, status);
}

U_CAPI int32_t U_EXPORT2
ucol_normalizeShortDefinitionString(const char *definition,
                                    char *destination,
                                    int32_t capacity,
                                    UParseError *parseError,
                                    UErrorCode *status)
{

    if(U_FAILURE(*status)) {
        return 0;
    }
    if(capacity == 0 || destination == NULL) {
        return uprv_strlen(definition);
    }
    uprv_memset(destination, 0, capacity*sizeof(char));

    // validate
    CollatorSpec s;
    ucol_sit_initCollatorSpecs(&s);
    ucol_sit_readSpecs(&s, definition, parseError, status);
    return ucol_sit_dumpSpecs(&s, destination, status);
}

// structure for packing the bits of the attributes in the
// identifier number.
// locale is packed separately
struct bitPacking {
    uint32_t offset;
    uint32_t width;
    UColAttribute attribute;
    UColAttributeValue values[5];
};

bitPacking attributesToBits[UCOL_ATTRIBUTE_COUNT] = {
    /* french */        { 30, 1, UCOL_FRENCH_COLLATION,         { UCOL_OFF, UCOL_ON }},
    /* alternate */     { 29, 1, UCOL_ALTERNATE_HANDLING,       { UCOL_NON_IGNORABLE, UCOL_SHIFTED }}, 
    /* case first */    { 27, 2, UCOL_CASE_FIRST,               { UCOL_OFF, UCOL_LOWER_FIRST, UCOL_UPPER_FIRST }},
    /* case level */    { 26, 1, UCOL_CASE_LEVEL,               { UCOL_OFF, UCOL_ON }},
    /* normalization */ { 25, 1, UCOL_NORMALIZATION_MODE,       { UCOL_OFF, UCOL_ON }},
    /* strength */      { 22, 3, UCOL_STRENGTH,                 { UCOL_PRIMARY, UCOL_SECONDARY, UCOL_TERTIARY, UCOL_QUATERNARY, UCOL_IDENTICAL }},
    /* hiragana */      { 21, 1, UCOL_HIRAGANA_QUATERNARY_MODE, { UCOL_OFF, UCOL_ON }},
    /* numeric coll */  { 20, 1, UCOL_NUMERIC_COLLATION,        { UCOL_OFF, UCOL_ON }}
};

static const uint32_t localeShift =    0;
static const uint32_t localeWidth =    8;

static const uint32_t needExpansion = 0xC0000000;

U_CAPI uint32_t U_EXPORT2
ucol_collatorToIdentifier(const UCollator *coll,
                          const char *locale,
                          UErrorCode *status) 
{
    uint32_t result = 0;
    int32_t i = 0, j = 0;
    int32_t valueIndex = 0;
    UColAttributeValue attrValue = UCOL_DEFAULT;

    // if variable top is not default, we need to use strings
    if(coll->variableTopValueisDefault != TRUE) {
        return needExpansion;
    }

    if(locale == NULL) {
        locale = ucol_getLocale(coll, ULOC_VALID_LOCALE, status);
    }

    /*Binary search for the map entry for normal cases */

    uint32_t   low     = 0;
    uint32_t   high    = sizeof(locales)/sizeof(locales[0]);
    uint32_t   mid     = high;
    uint32_t   oldmid  = 0;
    int32_t    compVal = 0;


    while (high > low)  /*binary search*/{

        mid = (high+low) >> 1; /*Finds median*/

        if (mid == oldmid) 
            return needExpansion; // we didn't find it

        compVal = uprv_strcmp(locale, locales[mid]);
        if (compVal < 0){
            high = mid;
        }
        else if (compVal > 0){
            low = mid;
        }
        else /*we found it*/{
            break;
        }
        oldmid = mid;
    }

    result |= (mid & ((1 << localeWidth) - 1)) << localeShift;

    for(i = 0; i < sizeof(attributesToBits)/sizeof(attributesToBits[0]); i++) {
        attrValue = ucol_getAttribute(coll, attributesToBits[i].attribute, status);
        j = 0;
        while(attributesToBits[i].values[j] != attrValue) {
            j++;
        }
        result |= (j & ((1 << attributesToBits[i].width) - 1)) << attributesToBits[i].offset;
    }
    
    return result;
}

U_CAPI UCollator* U_EXPORT2
ucol_openFromIdentifier(uint32_t identifier,
                        UErrorCode *status) 
{
    int32_t i = 0, j = 0;
    int32_t value = 0;

    value = (identifier >> localeShift) & ((1 << localeWidth) - 1);
    UColAttributeValue attrValue = UCOL_DEFAULT;

    UCollator *result = ucol_open(locales[value], status);

    // variable top is not set in the identifier, so we can easily skip that on

    for(i = 0; i < sizeof(attributesToBits)/sizeof(attributesToBits[0]); i++) {
        value = (identifier >> attributesToBits[i].offset) & ((1 << attributesToBits[i].width) - 1);
        attrValue = attributesToBits[i].values[value];
        // the collator is all default, so we will set only the values that will differ from 
        // the default values.
        if(ucol_getAttribute(result, attributesToBits[i].attribute, status) != attrValue) {
            ucol_setAttribute(result, attributesToBits[i].attribute, attrValue, status);
        }
    }

    return result;
}

U_CAPI int32_t U_EXPORT2
ucol_identifierToShortString(uint32_t identifier,
                             char *buffer,
                             int32_t capacity,
                             UErrorCode *status) 
{
    UCollator *coll = ucol_openFromIdentifier(identifier, status);
    int32_t locIndex = (identifier >> localeShift) & ((1 << localeWidth) - 1);
    int32_t resultLen = ucol_getShortDefinitionString(coll, locales[locIndex], buffer, capacity, status);
    ucol_close(coll);
    return resultLen;
}

U_CAPI UColAttributeValue  U_EXPORT2
ucol_getAttributeOrDefault(const UCollator *coll, UColAttribute attr, UErrorCode *status) 
{
    if(U_FAILURE(*status) || coll == NULL) {
      return UCOL_DEFAULT;
    }
    switch(attr) {
    case UCOL_NUMERIC_COLLATION:
        return coll->numericCollationisDefault?UCOL_DEFAULT:coll->numericCollation;
    case UCOL_HIRAGANA_QUATERNARY_MODE:
        return coll->hiraganaQisDefault?UCOL_DEFAULT:coll->hiraganaQ;
    case UCOL_FRENCH_COLLATION: /* attribute for direction of secondary weights*/
        return coll->frenchCollationisDefault?UCOL_DEFAULT:coll->frenchCollation;
    case UCOL_ALTERNATE_HANDLING: /* attribute for handling variable elements*/
        return coll->alternateHandlingisDefault?UCOL_DEFAULT:coll->alternateHandling;
    case UCOL_CASE_FIRST: /* who goes first, lower case or uppercase */
        return coll->caseFirstisDefault?UCOL_DEFAULT:coll->caseFirst;
    case UCOL_CASE_LEVEL: /* do we have an extra case level */
        return coll->caseLevelisDefault?UCOL_DEFAULT:coll->caseLevel;
    case UCOL_NORMALIZATION_MODE: /* attribute for normalization */
        return coll->normalizationModeisDefault?UCOL_DEFAULT:coll->normalizationMode;
    case UCOL_STRENGTH:         /* attribute for strength */
        return coll->strengthisDefault?UCOL_DEFAULT:coll->strength;
    case UCOL_ATTRIBUTE_COUNT:
    default:
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        break;
    }
    return UCOL_DEFAULT;
}

U_CDECL_BEGIN
static UBool U_CALLCONV
_processContractions(const void *context, UChar32 start, UChar32 limit, uint32_t value) 
{
    UErrorCode status = U_ZERO_ERROR;
    USet *unsafe = (USet *)context;
    UChar contraction[256];
    if(value > UCOL_NOT_FOUND && getCETag(value) == CONTRACTION_TAG) {
        // this is a contraction
        // we want to add the code point for sure
        while(start < limit) {
            //uset_add(unsafe, start);
            contraction[0] = (UChar)start;
            // get the rest of the contraction string from the data structure
            start++;
        }
        // check if there is anything else to add - if these lead
        // to a longer contraction
    }
    if(U_FAILURE(status)) {
        return FALSE;
    } else {
        return TRUE;
    }
}
U_CDECL_END

static int32_t U_CALLCONV
_getTrieFoldingOffset(uint32_t data) 
{
    return (int32_t)(data&0xFFFFFF);
}

U_CAPI int32_t U_EXPORT2
ucol_getUnsafeSet( const UCollator *coll,
                  USet *unsafe,
                  UErrorCode *status)
{
    uset_clear(unsafe);
    // add Thai/Lao prevowels
    uset_addRange(unsafe, 0xe40, 0xe44);
    uset_addRange(unsafe, 0xec0, 0xec4);
    // add lead/trail surrogates
    uset_addRange(unsafe, 0xd800, 0xdfff);


    // add FCD things
    const uint16_t *fcdTrieIndex=unorm_getFCDTrie(status);
    int32_t i = 0;

    // add unsafe BMPs
    uint16_t fcd, leadFCD;
    UChar32 c;
    for(c = 0; c < 0xffff; c++) {
        if(c==0xd800) {
            c=0xe000;
        }
        fcd = unorm_getFCD16(fcdTrieIndex, (UChar)c);
        if (fcd != 0) {
            uset_add(unsafe, c);
        }
    }

    // add unsafe supplementaries
    for(c = 0x10000; c < 0x110000; ) {
        leadFCD=unorm_getFCD16(fcdTrieIndex, U16_LEAD(c));
        if(leadFCD==0) {
            c+=0x400;
        } else {
            for(i=0; i<0x400; ++c, ++i) {
                // either i or U16_TRAIL(c) can be used because only the lower 10 bits are relevant
                fcd = unorm_getFCD16FromSurrogatePair(fcdTrieIndex, U16_LEAD(c), U16_TRAIL(c));
                if (fcd != 0) {
                    uset_add(unsafe, c);
                }
            }
        }
    }



    return uset_size(unsafe);
}


/**
 * Get a set containing the contractions defined by the collator. The set includes
 * both the UCA contractions and the contractions defined by the collator
 * @param coll collator
 * @param conts the set to hold the result
 * @param status to hold the error code
 * @return the size of the contraction set
 *
 * @draft ICU 3.0
 */
U_CAPI int32_t U_EXPORT2
ucol_getContractions( const UCollator *coll,
                  USet *contractions,
                  UErrorCode *status)
{
    // add contractions from the UCA
    int32_t width = coll->UCA->image->contractionUCACombosWidth;
    int32_t size = coll->UCA->image->contractionUCACombosSize;
    UChar *conts = (UChar *)((uint8_t *)coll->UCA->image + coll->UCA->image->contractionUCACombos);
    int32_t i = 0;
    while(i < size * width) {
        if(*(conts + i + 2)) {
            uset_addString(contractions, conts+i, 3);
        } else {
            uset_addString(contractions, conts+i, 2);
        }

        i += 3;
    }
    // This is collator specific. Add contractions from a collator
    coll->mapping->getFoldingOffset = _getTrieFoldingOffset;
    utrie_enum(coll->mapping, NULL, _processContractions, contractions);

    return uset_size(contractions);

}

