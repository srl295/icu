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
#include "ucol_tok.h"
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

static const char* keywords[] = {
/* 00 */ "",
/* 01 */ "direct",
/* 02 */ "phonebook",
/* 03 */ "pinyin",
/* 04 */ "standard",
/* 05 */ "stroke",
/* 06 */ "traditional"
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

static const char *collationKeyword  = "@collation=";

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

static const AttributeConversion conversions[12] = {
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


static char 
ucol_sit_attributeValueToLetter(UColAttributeValue value, UErrorCode *status) {
    uint32_t i = 0;
    for(i = 0; i < sizeof(conversions)/sizeof(conversions[0]); i++) {
        if(conversions[i].value == value) {
            return conversions[i].letter;
        }
    }
    *status = U_ILLEGAL_ARGUMENT_ERROR;
    return 0;
}

static UColAttributeValue 
ucol_sit_letterToAttributeValue(char letter, UErrorCode *status) {
    uint32_t i = 0;
    for(i = 0; i < sizeof(conversions)/sizeof(conversions[0]); i++) {
        if(conversions[i].letter == letter) {
            return conversions[i].value;
        }
    }
    *status = U_ILLEGAL_ARGUMENT_ERROR;
    return UCOL_DEFAULT;
}

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
    // don't skip the underscore at the end
    return string;
}
U_CDECL_END

U_CDECL_BEGIN
static const char* U_CALLCONV
_processRFC3066Locale(CollatorSpec *spec, uint32_t, const char* string, 
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
        return end+1;
    }
}

U_CDECL_END

U_CDECL_BEGIN
static const char* U_CALLCONV
_processCollatorOption(CollatorSpec *spec, uint32_t option, const char* string, 
                       UErrorCode *status) 
{
    spec->options[option] = ucol_sit_letterToAttributeValue(*string, status);
    if((*(++string) != '_' && *string) || U_FAILURE(*status)) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
    }
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
        result = (result << 4) | (UChar)value;
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
    } 
    return string;
}
U_CDECL_END


/* Table for parsing short strings */
struct ShortStringOptions {
    char optionStart;
    ActionFunction *action;
    uint32_t attr;
};

static const ShortStringOptions options[UCOL_SIT_ITEMS_COUNT] =
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
  int32_t i = 0;

  for(i = 0; i < UCOL_SIT_ITEMS_COUNT; i++) {
      if(*start == options[i].optionStart) {
          spec->entries[i].start = start;
          const char* end = options[i].action(spec, options[i].attr, start+1, status);
          spec->entries[i].len = end - start;
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

static const char* 
ucol_sit_readSpecs(CollatorSpec *s, const char *string, 
                        UParseError *parseError, UErrorCode *status)
{
    const char *definition = string;
    while(U_SUCCESS(*status) && *string) { 
        string = ucol_sit_readOption(string, s, status);
        // advance over '_'
        while(*string && *string == '_') {
            string++;
        }
    }
    if(U_FAILURE(*status)) {
        parseError->offset = string - definition;
    }
    return string;
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
                if(optName == languageArg || optName == regionArg || optName == variantArg || optName == keywordArg) {
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

static void
ucol_sit_calculateWholeLocale(CollatorSpec *s) {
    // put the locale together, unless we have a done
    // locale
    if(s->locale[0] == 0) {
        // first the language
        uprv_strcat(s->locale, s->locElements[0]);
        // then the script, if present
        if(*(s->locElements[1])) {
            uprv_strcat(s->locale, "_");
            uprv_strcat(s->locale, s->locElements[1]);
        }
        // then the region, if present
        if(*(s->locElements[2])) {
            uprv_strcat(s->locale, "_");
            uprv_strcat(s->locale, s->locElements[2]);
        } else if(*(s->locElements[3])) { // if there is a variant, we need an underscore
            uprv_strcat(s->locale, "_");
        }
        // add variant, if there
        if(*(s->locElements[3])) {
            uprv_strcat(s->locale, "_");
            uprv_strcat(s->locale, s->locElements[3]);
        }

        // if there is a collation keyword, add that too
        if(*(s->locElements[4])) {
            uprv_strcat(s->locale, collationKeyword);
            uprv_strcat(s->locale, s->locElements[4]);
        }
    }
}

U_CAPI UCollator* U_EXPORT2
ucol_openFromShortString( const char *definition,
                          UBool forceDefaults,
                          UParseError *parseError,
                          UErrorCode *status)
{
    UTRACE_ENTRY_OC(UTRACE_UCOL_OPEN_FROM_SHORT_STRING);
    UTRACE_DATA1(UTRACE_INFO, "short string = \"%s\"", definition);

    if(U_FAILURE(*status)) return 0;

    UParseError internalParseError;

    if(!parseError) {
        parseError = &internalParseError;
    }
    parseError->line = 0;
    parseError->offset = 0;
    parseError->preContext[0] = 0;
    parseError->postContext[0] = 0;


    // first we want to pick stuff out of short string.
    // we'll end up with an UCA version, locale and a bunch of
    // settings

    // analyse the string in order to get everything we need.
    const char *string = definition;
    CollatorSpec s;
    ucol_sit_initCollatorSpecs(&s);
    string = ucol_sit_readSpecs(&s, definition, parseError, status);
    ucol_sit_calculateWholeLocale(&s);
    
    char buffer[internalBufferSize];
    uprv_memset(buffer, 0, internalBufferSize);
    uloc_canonicalize(s.locale, buffer, internalBufferSize, status);

    UCollator *result = ucol_open(s.locale, status);
    int32_t i = 0;

    for(i = 0; i < UCOL_ATTRIBUTE_COUNT; i++) {
        if(s.options[i] != UCOL_DEFAULT) {
            if(ucol_getAttribute(result, (UColAttribute)i, status) != s.options[i] || forceDefaults) {
                ucol_setAttribute(result, (UColAttribute)i, s.options[i], status);
            }

            if(U_FAILURE(*status)) {
                parseError->offset = string - definition;
                ucol_close(result);
                return NULL;
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
        uprv_strncat(result, src, len);
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

    int32_t i = 0;
    UColAttributeValue attribute = UCOL_DEFAULT;
    for(i = 0; i < UCOL_SIT_ITEMS_COUNT; i++) {
        if(options[i].action == _processCollatorOption) {
            attribute = ucol_getAttributeOrDefault(coll, (UColAttribute)options[i].attr, status);
            if(attribute != UCOL_DEFAULT) {
                char letter = ucol_sit_attributeValueToLetter(attribute, status);
                appendShortStringElement(&letter, 1, 
                    buffer, &resultSize, options[i].optionStart);
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
    char letter;
    uint32_t offset;
    uint32_t width;
    UColAttribute attribute;
    UColAttributeValue values[6];
};

static const bitPacking attributesToBits[UCOL_ATTRIBUTE_COUNT] = {
    /* french */        { frenchCollArg,    29, 2, UCOL_FRENCH_COLLATION,         { UCOL_DEFAULT, UCOL_OFF, UCOL_ON }},
    /* alternate */     { alternateHArg,    27, 2, UCOL_ALTERNATE_HANDLING,       { UCOL_DEFAULT, UCOL_NON_IGNORABLE, UCOL_SHIFTED }}, 
    /* case first */    { caseFirstArg,     25, 2, UCOL_CASE_FIRST,               { UCOL_DEFAULT, UCOL_OFF, UCOL_LOWER_FIRST, UCOL_UPPER_FIRST }},
    /* case level */    { caseLevelArg,     23, 2, UCOL_CASE_LEVEL,               { UCOL_DEFAULT, UCOL_OFF, UCOL_ON }},
    /* normalization */ { normArg,          21, 2, UCOL_NORMALIZATION_MODE,       { UCOL_DEFAULT, UCOL_OFF, UCOL_ON }},
    /* strength */      { strengthArg,      18, 3, UCOL_STRENGTH,                 { UCOL_DEFAULT, UCOL_PRIMARY, UCOL_SECONDARY, UCOL_TERTIARY, UCOL_QUATERNARY, UCOL_IDENTICAL }},
    /* hiragana */      { hiraganaQArg,     16, 2, UCOL_HIRAGANA_QUATERNARY_MODE, { UCOL_DEFAULT, UCOL_OFF, UCOL_ON }},
    /* numeric coll */  { numericCollArg,   14, 2, UCOL_NUMERIC_COLLATION,        { UCOL_DEFAULT, UCOL_OFF, UCOL_ON }}
};

static const uint32_t keywordShift =   9;
static const uint32_t keywordWidth =   5;
static const uint32_t localeShift =    0;
static const uint32_t localeWidth =    7;

static const uint32_t needExpansion = 0xC0000000;


static uint32_t ucol_sit_putLocaleInIdentifier(uint32_t result, const char* locale, UErrorCode* status) {
    char buffer[internalBufferSize], keywordBuffer[internalBufferSize], 
        baseName[internalBufferSize], localeBuffer[internalBufferSize];
    int32_t len = 0, keywordLen = 0,
        baseNameLen = 0, localeLen = 0;
    uint32_t i = 0;
    UBool isAvailable = FALSE;
    if(locale) {
        len = uloc_canonicalize(locale, buffer, internalBufferSize, status);
        localeLen = ucol_getFunctionalEquivalent(localeBuffer, internalBufferSize, "collation", buffer, &isAvailable, status);
        keywordLen = uloc_getKeywordValue(buffer, "collation", keywordBuffer, internalBufferSize, status);
        baseNameLen = uloc_getBaseName(buffer, baseName, internalBufferSize, status);

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

            compVal = uprv_strcmp(baseName, locales[mid]);
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
    }

    if(keywordLen) {
        for(i = 1; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
            if(uprv_strcmp(keywords[i], keywordBuffer) == 0) {
                result |= (i & ((1 << keywordWidth) - 1)) << keywordShift;
                break;
            }
        }
    }
    return result;
}

U_CAPI uint32_t U_EXPORT2
ucol_collatorToIdentifier(const UCollator *coll,
                          const char *locale,
                          UErrorCode *status) 
{
    uint32_t result = 0;
    uint32_t i = 0, j = 0;
    UColAttributeValue attrValue = UCOL_DEFAULT;

    // if variable top is not default, we need to use strings
    if(coll->variableTopValueisDefault != TRUE) {
        return needExpansion;
    }

    if(locale == NULL) {
        locale = ucol_getLocale(coll, ULOC_VALID_LOCALE, status);
    }

    result = ucol_sit_putLocaleInIdentifier(result, locale, status);

    for(i = 0; i < sizeof(attributesToBits)/sizeof(attributesToBits[0]); i++) {
        attrValue = ucol_getAttributeOrDefault(coll, attributesToBits[i].attribute, status);
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
                        UBool forceDefaults,
                        UErrorCode *status) 
{
    uint32_t i = 0;
    int32_t value = 0, keyword = 0;
    char locale[internalBufferSize];

    value = (identifier >> localeShift) & ((1 << localeWidth) - 1);
    keyword = (identifier >> keywordShift) & ((1 << keywordWidth) - 1);
    
    uprv_strcpy(locale, locales[value]);

    if(keyword) {
        uprv_strcat(locale, collationKeyword);
        uprv_strcat(locale, keywords[keyword]);
    }

    UColAttributeValue attrValue = UCOL_DEFAULT;

    UCollator *result = ucol_open(locale, status);

    // variable top is not set in the identifier, so we can easily skip that on

    for(i = 0; i < sizeof(attributesToBits)/sizeof(attributesToBits[0]); i++) {
        value = (identifier >> attributesToBits[i].offset) & ((1 << attributesToBits[i].width) - 1);
        attrValue = attributesToBits[i].values[value];
        // the collator is all default, so we will set only the values that will differ from 
        // the default values.
        if(attrValue != UCOL_DEFAULT) {
            if(ucol_getAttribute(result, attributesToBits[i].attribute, status) != attrValue 
                || forceDefaults) {
                ucol_setAttribute(result, attributesToBits[i].attribute, attrValue, status);
            }
        }
    }

    return result;
}

U_CAPI int32_t U_EXPORT2
ucol_identifierToShortString(uint32_t identifier,
                             char *buffer,
                             int32_t capacity,
                             UBool forceDefaults,
                             UErrorCode *status) 
{
    int32_t locIndex = (identifier >> localeShift) & ((1 << localeWidth) - 1);
    int32_t keywordIndex = (identifier >> keywordShift) & ((1 << keywordWidth) - 1);
    CollatorSpec s;
    ucol_sit_initCollatorSpecs(&s);
    uprv_strcpy(s.locale, locales[locIndex]);
    if(keywordIndex) {
        uprv_strcat(s.locale, collationKeyword);
        uprv_strcat(s.locale, keywords[keywordIndex]);
    }
    UCollator *coll = ucol_openFromIdentifier(identifier, forceDefaults, status);
    int32_t resultLen = ucol_getShortDefinitionString(coll, s.locale, buffer, capacity, status);
    ucol_close(coll);
    return resultLen;

#if 0
    // TODO: Crumy, crumy, crumy... Very hard to currently go algorithmically from 
    // identifier to short string. Do rethink
    if(forceDefaults == FALSE) {
        UCollator *coll = ucol_openFromIdentifier(identifier, FALSE, status);
        int32_t resultLen = ucol_getShortDefinitionString(coll, s.locale, buffer, capacity, status);
        ucol_close(coll);
        return resultLen;
    } else { // forceDefaults == TRUE
        char letter;
        UColAttributeValue value;
        int32_t i = 0;
        for(i = 0; i < sizeof(attributesToBits)/sizeof(attributesToBits[0]); i++) {
            value = attributesToBits[i].values[(identifier >> attributesToBits[i].offset) & ((1 << attributesToBits[i].width) - 1)];
            if(value != UCOL_DEFAULT) {
                uprv_strcat(buffer, "_");
                uprv_strncat(buffer, &attributesToBits[i].letter, 1);
                letter = ucol_sit_attributeValueToLetter(value, status);
                uprv_strncat(buffer, &letter, 1);
            }
        }
        return ucol_sit_dumpSpecs(&s, buffer, status);
    }
#endif
}

U_CAPI uint32_t U_EXPORT2
ucol_shortStringToIdentifier(const char *definition,
                             UBool forceDefaults,
                             UErrorCode *status) 
{
    UParseError parseError;
    CollatorSpec s;
    uint32_t result = 0;
    uint32_t i = 0, j = 0;
    ucol_sit_initCollatorSpecs(&s);

    ucol_sit_readSpecs(&s, definition, &parseError, status);
    ucol_sit_calculateWholeLocale(&s);

    char locBuffer[internalBufferSize];
    UBool isAvailable = FALSE;
    UColAttributeValue attrValue = UCOL_DEFAULT;

    ucol_getFunctionalEquivalent(locBuffer, internalBufferSize, "collation", s.locale, &isAvailable, status);

    if(forceDefaults == FALSE) {
        UCollator *coll = ucol_openFromShortString(definition, FALSE, &parseError, status);
        result = ucol_collatorToIdentifier(coll, locBuffer, status);
        ucol_close(coll);
    } else { // forceDefaults == TRUE
        result = ucol_sit_putLocaleInIdentifier(result, locBuffer, status);

        for(i = 0; i < sizeof(attributesToBits)/sizeof(attributesToBits[0]); i++) {
            attrValue = s.options[i];
            j = 0;
            while(attributesToBits[i].values[j] != attrValue) {
                j++;
            }
            result |= (j & ((1 << attributesToBits[i].width) - 1)) << attributesToBits[i].offset;
        }

    }
    return result;
        
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


struct contContext {
    const UCollator *coll;
    USet            *conts;
    USet            *removedContractions;
    UErrorCode      *status;
};



static void
addContraction(const UCollator *coll, USet *contractions, UChar *buffer, int32_t bufLen, 
               uint32_t CE, int32_t rightIndex, UErrorCode *status) 
{
    if(rightIndex == bufLen-1) {
        *status = U_INTERNAL_PROGRAM_ERROR;
        return;
    }
    const UChar *UCharOffset = (UChar *)coll->image+getContractOffset(CE);
    uint32_t newCE = *(coll->contractionCEs + (UCharOffset - coll->contractionIndex));
    // we might have a contraction that ends from previous level
    if(newCE != UCOL_NOT_FOUND && rightIndex > 1) {
            uset_addString(contractions, buffer, rightIndex + 1);
    }

    UCharOffset++;
    while(*UCharOffset != 0xFFFF) {
        newCE = *(coll->contractionCEs + (UCharOffset - coll->contractionIndex));
        buffer[rightIndex] = *UCharOffset;
        if(isSpecial(newCE) && getCETag(newCE) == CONTRACTION_TAG) {
            addContraction(coll, contractions, buffer, bufLen, newCE, rightIndex + 1, status);
        } else {
            uset_addString(contractions, buffer, rightIndex + 1);
        }
        UCharOffset++;
    }
}

U_CDECL_BEGIN
static UBool U_CALLCONV
_processContractions(const void *context, UChar32 start, UChar32 limit, uint32_t CE) 
{
    UErrorCode *status = ((contContext *)context)->status;
    USet *unsafe = ((contContext *)context)->conts;
    USet *removed = ((contContext *)context)->removedContractions;
    const UCollator *coll = ((contContext *)context)->coll;
    UChar contraction[internalBufferSize];
    if(isSpecial(CE) && getCETag(CE) == CONTRACTION_TAG) {
        while(start < limit && U_SUCCESS(*status)) {
            // if there are suppressed contractions, we don't 
            // want to add them.
            if(removed && uset_contains(removed, start)) {
                start++;
                continue;
            }
            // we start our contraction from middle, since we don't know if it
            // will grow toward right or left
            contraction[0] = (UChar)start;
            addContraction(coll, unsafe, contraction, internalBufferSize, CE, 1, status);
            start++;
        }
    }
    if(U_FAILURE(*status)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static int32_t U_CALLCONV
_getTrieFoldingOffset(uint32_t data) 
{
    return (int32_t)(data&0xFFFFFF);
}
U_CDECL_END



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
    uset_clear(contractions);
    int32_t rulesLen = 0;
    const UChar* rules = ucol_getRules(coll, &rulesLen);
    UColTokenParser src;
    ucol_tok_initTokenList(&src, rules, rulesLen, coll->UCA, status);

    contContext c = { NULL, contractions, src.removeSet, status };

    coll->mapping->getFoldingOffset = _getTrieFoldingOffset;

    // TODO: if you're supressing contractions in the tailoring
    // you want to remove (or rather not include) contractions
    // from the UCA.
    // Probably want to pass a set of contraction starters that
    // are suppressed. However, we don't want a dependency on 
    // the builder, so this is going to be hard to pull off.

    // Add the UCA contractions
    c.coll = coll->UCA;
    utrie_enum(coll->UCA->mapping, NULL, _processContractions, &c);
    
    // This is collator specific. Add contractions from a collator
    c.coll = coll;
    c.removedContractions =  NULL;
    utrie_enum(coll->mapping, NULL, _processContractions, &c);

    return uset_getItemCount(contractions);

}

U_CAPI int32_t U_EXPORT2
ucol_getUnsafeSet( const UCollator *coll,
                  USet *unsafe,
                  UErrorCode *status)
{
    uset_clear(unsafe);

    // add chars that fail the fcd check
    UChar buffer[internalBufferSize];
    static const char* fcdUnsafes = "[[:^tccc=0:][:^lccc=0:]]";
    int32_t len = uprv_strlen(fcdUnsafes);   
    u_charsToUChars(fcdUnsafes, buffer, internalBufferSize);
    uset_applyPattern(unsafe, buffer, len, USET_IGNORE_SPACE, status);

    // add Thai/Lao prevowels
    uset_addRange(unsafe, 0xe40, 0xe44);
    uset_addRange(unsafe, 0xec0, 0xec4);
    // add lead/trail surrogates
    uset_addRange(unsafe, 0xd800, 0xdfff);

    USet *contractions = uset_open(0,0);

    int32_t i = 0, j = 0;
    int32_t contsSize = ucol_getContractions(coll, contractions, status);
    // Contraction set consists only of strings
    // to get unsafe code points, we need to 
    // break the strings apart and add them to the unsafe set
    for(i = 0; i < contsSize; i++) {
        len = uset_getItem(contractions, i, NULL, NULL, buffer, internalBufferSize, status);
        if(len > 0) {
            for(j = 1; j < len; j++) {
                uset_add(unsafe, buffer[j]);
            }
        }
    }

    uset_close(contractions);

    return uset_size(unsafe);
}
