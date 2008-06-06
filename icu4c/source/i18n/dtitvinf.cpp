/*******************************************************************************
* Copyright (C) 2008, International Business Machines Corporation and
* others. All Rights Reserved.
*******************************************************************************
*
* File DTITVINF.CPP 
*
*******************************************************************************
*/


//FIXME: how to define it in compiler time
//#define DTITVINF_DEBUG 0


#ifdef DTITVINF_DEBUG 
#include <iostream>
#endif

#include "cstring.h"
#include "unicode/msgfmt.h"
#include "unicode/dtitvinf.h"
#include "dtitv_impl.h"

#if !UCONFIG_NO_FORMATTING


U_NAMESPACE_BEGIN


#ifdef DTITVINF_DEBUG 
#define PRINTMESG(msg) { std::cout << "(" << __FILE__ << ":" << __LINE__ << ") " << msg << "\n"; }
#endif

UOBJECT_DEFINE_RTTI_IMPLEMENTATION(DateIntervalInfo)

static const char gIntervalDateTimePatternTag[]="IntervalDateTimePatterns";
static const char gFallbackPatternTag[]="Fallback";

// {0}
static const UChar gFirstPattern[] = {LEFT_CURLY_BRACKET, DIGIT_ZERO, RIGHT_CURLY_BRACKET, 0};
// {1}
static const UChar gSecondPattern[] = {LEFT_CURLY_BRACKET, DIGIT_ONE, RIGHT_CURLY_BRACKET, 0};

// default fall-back
static const UChar gDefaultFallbackPattern[] = {LEFT_CURLY_BRACKET, DIGIT_ZERO, RIGHT_CURLY_BRACKET, SPACE, EN_DASH, SPACE, LEFT_CURLY_BRACKET, DIGIT_ONE, RIGHT_CURLY_BRACKET, 0};



DateIntervalInfo::DateIntervalInfo(UErrorCode& status) 
:   fFallbackIntervalPattern(gDefaultFallbackPattern),
    fFirstDateInPtnIsLaterDate(false),
    fIntervalPatterns(NULL)
{
    if ( U_FAILURE(status) ) {
        return;
    }
    fIntervalPatterns = initHash(status);
}



DateIntervalInfo::DateIntervalInfo(const Locale& locale, UErrorCode& status)
:   fFallbackIntervalPattern(gDefaultFallbackPattern),
    fFirstDateInPtnIsLaterDate(false),
    fIntervalPatterns(NULL)
{
    if ( U_FAILURE(status) ) {
        return;
    }
    initializeData(locale, status);
}



void
DateIntervalInfo::setIntervalPattern(const UnicodeString& skeleton,
                                     UCalendarDateFields lrgDiffCalUnit,
                                     const UnicodeString& intervalPattern,
                                     UErrorCode& status) {
    if ( U_FAILURE(status) ) {
        return;
    }
    
    if ( lrgDiffCalUnit == UCAL_HOUR_OF_DAY ) {
        setIntervalPatternInternally(skeleton, UCAL_AM_PM, intervalPattern, status);
        setIntervalPatternInternally(skeleton, UCAL_HOUR, intervalPattern, status);
    } else if ( lrgDiffCalUnit == UCAL_DAY_OF_MONTH ||
                lrgDiffCalUnit == UCAL_DAY_OF_WEEK ) {
        setIntervalPatternInternally(skeleton, UCAL_DATE, intervalPattern, status);
    } else {
        setIntervalPatternInternally(skeleton, lrgDiffCalUnit, intervalPattern, status);
    }
}


void
DateIntervalInfo::setFallbackIntervalPattern(
                                    const UnicodeString& fallbackPattern) {
    int32_t firstPatternIndex = fallbackPattern.indexOf(gFirstPattern, 
                        sizeof(gFirstPattern)/sizeof(gFirstPattern[0]), 0);
    int32_t secondPatternIndex = fallbackPattern.indexOf(gSecondPattern, 
                        sizeof(gSecondPattern)/sizeof(gSecondPattern[0]), 0);
    if ( firstPatternIndex > secondPatternIndex ) { 
        fFirstDateInPtnIsLaterDate = true;
    }
    fFallbackIntervalPattern = fallbackPattern;
}



DateIntervalInfo::DateIntervalInfo(const DateIntervalInfo& dtitvinf)
:   UObject(dtitvinf),
    fIntervalPatterns(NULL)
{
    *this = dtitvinf;
}
    


DateIntervalInfo&
DateIntervalInfo::operator=(const DateIntervalInfo& dtitvinf) {
    if ( this == &dtitvinf ) {
        return *this;
    }
    
    UErrorCode status = U_ZERO_ERROR;
    deleteHash(fIntervalPatterns);
    fIntervalPatterns = initHash(status);
    if ( U_FAILURE(status) ) {
        return *this;
    } 
    copyHash(dtitvinf.fIntervalPatterns, fIntervalPatterns, status);
    if ( U_FAILURE(status) ) {
        return *this;
    } 

    fFallbackIntervalPattern = dtitvinf.fFallbackIntervalPattern;
    fFirstDateInPtnIsLaterDate = dtitvinf.fFirstDateInPtnIsLaterDate;
    return *this;
}


DateIntervalInfo*
DateIntervalInfo::clone() const {
    return new DateIntervalInfo(*this);
}


DateIntervalInfo::~DateIntervalInfo() {
    deleteHash(fIntervalPatterns);
    fIntervalPatterns = NULL;
}


UBool
DateIntervalInfo::operator==(const DateIntervalInfo& other) const {
    UBool equal = ( 
      fFallbackIntervalPattern == other.fFallbackIntervalPattern &&
      fFirstDateInPtnIsLaterDate == other.fFirstDateInPtnIsLaterDate );

    if ( equal == TRUE ) {
        equal = fIntervalPatterns->equals(*(other.fIntervalPatterns));
    }

    return equal;
}


const UnicodeString*
DateIntervalInfo::getIntervalPattern(const UnicodeString& skeleton,
                                     UCalendarDateFields field,
                                     UErrorCode& status) const {
    if ( U_FAILURE(status) ) {
        return NULL;
    }
   
    const UnicodeString* patternsOfOneSkeleton = (UnicodeString*) fIntervalPatterns->get(skeleton);
    if ( patternsOfOneSkeleton != NULL ) {
        int8_t index = (int8_t)calendarFieldToIntervalIndex(field, status);
        if ( U_FAILURE(status) ) {
            return NULL;
        }
        const UnicodeString& intervalPattern =  patternsOfOneSkeleton[index];
        if ( !intervalPattern.isEmpty() ) {
            return &intervalPattern;
        }
    }
    return NULL;
}


void 
DateIntervalInfo::initializeData(const Locale& locale, UErrorCode& status)
{
    fIntervalPatterns = initHash(status);
    if ( U_FAILURE(status) ) {
        return;
    }
    CalendarData* calData = new CalendarData(locale, NULL, status);
    if ( calData == NULL ) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return;
    }

    const UResourceBundle* itvDtPtnResource = calData->getByKey(
                                       gIntervalDateTimePatternTag, status);

    // look for fallback first, since it establishes the default order
    const UChar* resStr;
    int32_t resStrLen = 0;
    resStr = ures_getStringByKeyWithFallback(itvDtPtnResource, 
                                             gFallbackPatternTag, 
                                             &resStrLen, &status);
    if ( U_FAILURE(status) ) {
        delete calData;
        return;
    }

    UnicodeString pattern = UnicodeString(TRUE, resStr, resStrLen);
    setFallbackIntervalPattern(pattern);

    int32_t size = ures_getSize(itvDtPtnResource);
    int32_t index;
    for ( index = 0; index < size; ++index ) {
        UResourceBundle* oneRes = ures_getByIndex(itvDtPtnResource, index, 
                                                 NULL, &status);
        if ( U_FAILURE(status) ) {
            delete calData;
            return;
        }

        const char* skeleton = ures_getKey(oneRes);
        ures_close(oneRes);
        if ( skeleton == NULL ) {
            status = U_MISSING_RESOURCE_ERROR;
            delete calData;
            return;
        }
        if ( uprv_strcmp(skeleton, gFallbackPatternTag) == 0 ) {
            continue;  // fallback
        }

        UResourceBundle* intervalPatterns = ures_getByKey(itvDtPtnResource,
                                                      skeleton, NULL, &status);

        if ( U_FAILURE(status) ) {
            delete calData;
            return;
        }

        // return if interval patterns for skeleton not found
        if ( intervalPatterns == NULL ) {
            status = U_MISSING_RESOURCE_ERROR;
            delete calData;
            return;
        }

        const UChar* pattern;
        const char* key;
        int32_t ptLength;
        int32_t ptnNum = ures_getSize(intervalPatterns);
        int32_t ptnIndex;
        for ( ptnIndex = 0; ptnIndex < ptnNum; ++ptnIndex ) {
            pattern = ures_getNextString(intervalPatterns, &ptLength, &key,
                                         &status);
            if ( U_FAILURE(status) ) {
                delete calData;
                return;
            }

            UCalendarDateFields calendarField = UCAL_FIELD_COUNT;
            if ( !uprv_strcmp(key, "y") ) {
                calendarField = UCAL_YEAR;
            } else if ( !uprv_strcmp(key, "M") ) {
                calendarField = UCAL_MONTH;
            } else if ( !uprv_strcmp(key, "d") ) {
                calendarField = UCAL_DATE;
            } else if ( !uprv_strcmp(key, "a") ) {
                calendarField = UCAL_AM_PM;
            } else if ( !uprv_strcmp(key, "h") ) {
                calendarField = UCAL_HOUR;
            } else if ( !uprv_strcmp(key, "m") ) {
                calendarField = UCAL_MINUTE;
            }
            if ( calendarField != UCAL_FIELD_COUNT ) {
                setIntervalPatternInternally(skeleton, calendarField, pattern,status);
            }
        }
        ures_close(intervalPatterns);
    }
    delete calData;
}



void
DateIntervalInfo::setIntervalPatternInternally(const UnicodeString& skeleton,
                                      UCalendarDateFields lrgDiffCalUnit,
                                      const UnicodeString& intervalPattern,
                                      UErrorCode& status) {
    int8_t index = (int8_t)calendarFieldToIntervalIndex(lrgDiffCalUnit,status);
    if ( U_FAILURE(status) ) {
        return;
    }
    UnicodeString* patternsOfOneSkeleton = (UnicodeString*)(fIntervalPatterns->get(skeleton));
    UBool emptyHash = false;
    if ( patternsOfOneSkeleton == NULL ) {
        patternsOfOneSkeleton = new UnicodeString[kIPI_MAX_INDEX];
        emptyHash = true;
    }
    
    patternsOfOneSkeleton[index] = intervalPattern;
    if ( emptyHash == TRUE ) {
        fIntervalPatterns->put(skeleton, patternsOfOneSkeleton, status);
    }
}



void 
DateIntervalInfo::parseSkeleton(const UnicodeString& skeleton, 
                                int32_t* skeletonFieldWidth) {
    const int8_t PATTERN_CHAR_BASE = 0x41;
    int32_t i;
    for ( i = 0; i < skeleton.length(); ++i ) {
        // it is an ASCII char in skeleton
        int8_t ch = (int8_t)skeleton.charAt(i);  
        ++skeletonFieldWidth[ch - PATTERN_CHAR_BASE];
    }
}



UBool 
DateIntervalInfo::stringNumeric(int32_t fieldWidth, int32_t anotherFieldWidth,
                                char patternLetter) {
    if ( patternLetter == 'M' ) {
        if ( fieldWidth <= 2 && anotherFieldWidth > 2 ||
             fieldWidth > 2 && anotherFieldWidth <= 2 ) {
            return true;
        }
    }
    return false;
}



const UnicodeString* 
DateIntervalInfo::getBestSkeleton(const UnicodeString& skeleton,
                                  int8_t& bestMatchDistanceInfo) const {
#ifdef DTITVINF_DEBUG
    char result[1000];
    char result_1[1000];
    char mesg[2000];
    skeleton.extract(0,  skeleton.length(), result, "UTF-8");
    sprintf(mesg, "in getBestSkeleton: skeleton: %s; \n", result);
    PRINTMESG(mesg)
#endif


    int32_t inputSkeletonFieldWidth[] =
    {
    //       A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    //   P   Q   R   S   T   U   V   W   X   Y   Z
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,  0, 0, 0,
    //       a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    //   p   q   r   s   t   u   v   w   x   y   z
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    };

    int32_t skeletonFieldWidth[] =
    {
    //       A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
             0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    //   P   Q   R   S   T   U   V   W   X   Y   Z
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,  0, 0, 0,
    //       a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    //   p   q   r   s   t   u   v   w   x   y   z
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    };

    const int32_t DIFFERENT_FIELD = 0x1000;
    const int32_t STRING_NUMERIC_DIFFERENCE = 0x100;
    const int32_t BASE = 0x41;
    const UChar CHAR_V = 0x0076;
    const UChar CHAR_Z = 0x007A;

    // hack for 'v' and 'z'.
    // resource bundle only have time skeletons ending with 'v',
    // but not for time skeletons ending with 'z'.
    UBool replaceZWithV = false;
    const UnicodeString* inputSkeleton = &skeleton; 
    UnicodeString copySkeleton;
    if ( skeleton.indexOf(CHAR_Z) != -1 ) {
        UChar zstr[2];
        UChar vstr[2]; 
        zstr[0]=CHAR_Z;
        vstr[0]=CHAR_V;
        zstr[1]=0;
        vstr[1]=0;
        copySkeleton = skeleton;
        copySkeleton.findAndReplace(zstr, vstr);
        inputSkeleton = &copySkeleton;
        replaceZWithV = true;
    }

    parseSkeleton(*inputSkeleton, inputSkeletonFieldWidth);
    int32_t bestDistance = MAX_POSITIVE_INT;
    const UnicodeString* bestSkeleton = NULL;

    // 0 means exact the same skeletons;
    // 1 means having the same field, but with different length,
    // 2 means only z/v differs
    // -1 means having different field.
    bestMatchDistanceInfo = 0;
    int8_t fieldLength = sizeof(skeletonFieldWidth)/sizeof(skeletonFieldWidth[0]);

    int32_t pos = -1;
    const UHashElement* elem = NULL;
    while ( (elem = fIntervalPatterns->nextElement(pos)) != NULL ) {
        const UHashTok keyTok = elem->key;
        UnicodeString* skeleton = (UnicodeString*)keyTok.pointer;
#ifdef DTITVINF_DEBUG
    skeleton->extract(0,  skeleton->length(), result, "UTF-8");
    sprintf(mesg, "available skeletons: skeleton: %s; \n", result);
    PRINTMESG(mesg)
#endif

        // clear skeleton field width
        int8_t i;
        for ( i = 0; i < fieldLength; ++i ) {
            skeletonFieldWidth[i] = 0;    
        }
        parseSkeleton(*skeleton, skeletonFieldWidth);
        // calculate distance
        int32_t distance = 0;
        int8_t fieldDifference = 1;
        for ( i = 0; i < fieldLength; ++i ) {
            int32_t inputFieldWidth = inputSkeletonFieldWidth[i];
            int32_t fieldWidth = skeletonFieldWidth[i];
            if ( inputFieldWidth == fieldWidth ) {
                continue;
            }
            if ( inputFieldWidth == 0 ) {
                fieldDifference = -1;
                distance += DIFFERENT_FIELD;
            } else if ( fieldWidth == 0 ) {
                fieldDifference = -1;
                distance += DIFFERENT_FIELD;
            } else if (stringNumeric(inputFieldWidth, fieldWidth, 
                                     (char)(i+BASE) ) ) {
                distance += STRING_NUMERIC_DIFFERENCE;
            } else {
                distance += (inputFieldWidth > fieldWidth) ? 
                            (inputFieldWidth - fieldWidth) : 
                            (fieldWidth - inputFieldWidth);
            }
        }
        if ( distance < bestDistance ) {
            bestSkeleton = skeleton;
            bestDistance = distance;
            bestMatchDistanceInfo = fieldDifference;
        }
        if ( distance == 0 ) {
            bestMatchDistanceInfo = 0;
            break;
        }
    }
    if ( replaceZWithV && bestMatchDistanceInfo != -1 ) {
        bestMatchDistanceInfo = 2;
    }
    return bestSkeleton;
}



DateIntervalInfo::IntervalPatternIndex
DateIntervalInfo::calendarFieldToIntervalIndex(UCalendarDateFields field, 
                                               UErrorCode& status) {
    IntervalPatternIndex index = kIPI_ERA;
    switch ( field ) {
      case UCAL_ERA:
        break;
      case UCAL_YEAR:
        index = kIPI_YEAR;
        break;
      case UCAL_MONTH:
        index = kIPI_MONTH;
        break;
      case UCAL_DATE:
      case UCAL_DAY_OF_WEEK:
      //case UCAL_DAY_OF_MONTH:
        index = kIPI_DATE;
        break;
      case UCAL_AM_PM:
        index = kIPI_AM_PM;
        break;
      case UCAL_HOUR:
      case UCAL_HOUR_OF_DAY:
        index = kIPI_HOUR;
        break;
      case UCAL_MINUTE:
        index = kIPI_MINUTE;
        break;
      default:
        status = U_ILLEGAL_ARGUMENT_ERROR;
    }
    return index;
}



void
DateIntervalInfo::deleteHash(Hashtable* hTable) 
{
    if ( hTable == NULL ) {
        return;
    }
    int32_t pos = -1;
    const UHashElement* element = NULL;
    while ( (element = hTable->nextElement(pos)) != NULL ) {
        const UHashTok keyTok = element->key;
        const UHashTok valueTok = element->value;
        const UnicodeString* value = (UnicodeString*)valueTok.pointer;
        delete[] value;
    }
    delete fIntervalPatterns;
}


U_CDECL_BEGIN 

/**
 * set hash table value comparator
 *
 * @param val1  one value in comparison
 * @param val2  the other value in comparison
 * @return      TRUE if 2 values are the same, FALSE otherwise
 */
static UBool U_CALLCONV hashTableValueComparator(UHashTok val1, UHashTok val2);

U_CDECL_END

UBool 
U_CALLCONV hashTableValueComparator(UHashTok val1, UHashTok val2) {
    const UnicodeString* pattern1 = (UnicodeString*)val1.pointer;
    const UnicodeString* pattern2 = (UnicodeString*)val2.pointer;
    UBool ret = TRUE;
    int8_t i;
    for ( i = 0; i < DateIntervalInfo::kIPI_MAX_INDEX && ret == TRUE; ++i ) {
        ret = (pattern1[i] == pattern2[i]);
    }
    return ret;
}



Hashtable*
DateIntervalInfo::initHash(UErrorCode& status) {
    
    Hashtable* hTable;
    if ( (hTable = new Hashtable(TRUE, status)) == NULL ) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return NULL;
    }
    hTable->setValueCompartor(hashTableValueComparator);
    return hTable;
}


void
DateIntervalInfo::copyHash(const Hashtable* source,
                           Hashtable* target,
                           UErrorCode& status) {
    int32_t pos = -1;
    const UHashElement* element = NULL;
    if ( source ) {
        while ( (element = source->nextElement(pos)) != NULL ) {
            const UHashTok keyTok = element->key;
            const UnicodeString* key = (UnicodeString*)keyTok.pointer;
            const UHashTok valueTok = element->value;
            const UnicodeString* value = (UnicodeString*)valueTok.pointer;
            UnicodeString* copy = new UnicodeString[kIPI_MAX_INDEX];
            int8_t i;
            for ( i = 0; i < kIPI_MAX_INDEX; ++i ) {
                copy[i] = value[i];
            }
            target->put(UnicodeString(*key), copy, status);
            if ( U_FAILURE(status) ) {
                return;
            }
        }
    }
}


U_NAMESPACE_END

#endif
