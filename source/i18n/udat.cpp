/*
*******************************************************************************
*   Copyright (C) 1996-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
*******************************************************************************
*/

#include "unicode/udat.h"

#include "unicode/uloc.h"
#include "unicode/datefmt.h"
#include "unicode/timezone.h"
#include "unicode/smpdtfmt.h"
#include "unicode/fieldpos.h"
#include "cpputils.h"
#include "unicode/parsepos.h"
#include "unicode/calendar.h"
#include "unicode/numfmt.h"
#include "unicode/dtfmtsym.h"
#include "unicode/ustring.h"

U_CAPI UDateFormat*
udat_open(            UDateFormatStyle        timeStyle, 
                UDateFormatStyle        dateStyle,
                const   char*                locale,
                const   UChar                    *tzID,
		      int32_t tzIDLength,
                UErrorCode*             status)
{
  if(U_FAILURE(*status)) return 0;
  
  DateFormat *fmt;
  if(locale == 0)
    fmt = DateFormat::createDateTimeInstance((DateFormat::EStyle)dateStyle,
                         (DateFormat::EStyle)timeStyle);
  else
    fmt = DateFormat::createDateTimeInstance((DateFormat::EStyle)dateStyle,
                         (DateFormat::EStyle)timeStyle,
                                             Locale(locale));
  
  if(fmt == 0) {
    *status = U_MEMORY_ALLOCATION_ERROR;
    return 0;
  }

  if(tzID != 0) {
    TimeZone *zone = 0;
    int32_t length = (tzIDLength == -1 ? u_strlen(tzID) : tzIDLength);
    zone = TimeZone::createTimeZone(UnicodeString((UChar*)tzID,
						  length, length));
    if(zone == 0) {
      *status = U_MEMORY_ALLOCATION_ERROR;
      delete fmt;
      return 0;
    }
    fmt->adoptTimeZone(zone);
  }
  
  return (UDateFormat*)fmt;
}

U_CAPI UDateFormat*
udat_openPattern(    const   UChar           *pattern, 
            int32_t         patternLength,
            const   char         *locale,
            UErrorCode      *status)
{
  if(U_FAILURE(*status)) return 0;

  int32_t len = (patternLength == -1 ? u_strlen(pattern) : patternLength);
  UDateFormat *retVal = 0;

  if(locale == 0)
    retVal = (UDateFormat*)new SimpleDateFormat(UnicodeString((UChar*)pattern,
                                  len, len),
                        *status);
  else
    retVal = (UDateFormat*)new SimpleDateFormat(UnicodeString((UChar*)pattern,
                                  len, len),
                        Locale(locale),
                        *status);

  if(retVal == 0) {
    *status = U_MEMORY_ALLOCATION_ERROR;
    return 0;
  }
  return retVal;
}

U_CAPI void
udat_close(UDateFormat* format)
{
  delete (DateFormat*)format;
}

U_CAPI UDateFormat*
udat_clone(const UDateFormat *fmt,
       UErrorCode *status)
{
  if(U_FAILURE(*status)) return 0;

  Format *res = ((SimpleDateFormat*)fmt)->clone();
  
  if(res == 0) {
    *status = U_MEMORY_ALLOCATION_ERROR;
    return 0;
  }

  return (UDateFormat*) res;
}

U_CAPI int32_t
udat_format(    const    UDateFormat*    format,
        UDate           dateToFormat,
        UChar*          result,
        int32_t         resultLength,
        UFieldPosition* position,
        UErrorCode*     status)
{
  if(U_FAILURE(*status)) return -1;

  UnicodeString res(result, 0, resultLength);
  FieldPosition fp;
  
  if(position != 0)
    fp.setField(position->field);
  
  ((DateFormat*)format)->format(dateToFormat, res, fp);
  
  if(position != 0) {
    position->beginIndex = fp.getBeginIndex();
    position->endIndex = fp.getEndIndex();
  }
  
  return uprv_fillOutputString(res, result, resultLength, status);
}

U_CAPI UDate
udat_parse(    const    UDateFormat*        format,
        const    UChar*          text,
        int32_t         textLength,
        int32_t         *parsePos,
        UErrorCode      *status)
{
  if(U_FAILURE(*status)) return (UDate)0;

  int32_t len = (textLength == -1 ? u_strlen(text) : textLength);
  const UnicodeString src((UChar*)text, len, len);
  ParsePosition pp;
  UDate res;

  if(parsePos != 0)
    pp.setIndex(*parsePos);

  res = ((DateFormat*)format)->parse(src, pp);

  if(parsePos != 0) {
    if(pp.getErrorIndex() == -1)
      *parsePos = pp.getIndex();
    else {
      *parsePos = pp.getErrorIndex();
      *status = U_PARSE_ERROR;
    }
  }
  
  return res;
}

U_CAPI UBool
udat_isLenient(const UDateFormat* fmt)
{
  return ((DateFormat*)fmt)->isLenient();
}

U_CAPI void
udat_setLenient(    UDateFormat*    fmt,
            UBool          isLenient)
{
  ((DateFormat*)fmt)->setLenient(isLenient);
}

U_CAPI const UCalendar*
udat_getCalendar(const UDateFormat* fmt)
{
  return (const UCalendar*) ((DateFormat*)fmt)->getCalendar();
}

U_CAPI void
udat_setCalendar(            UDateFormat*    fmt,
                    const   UCalendar*      calendarToSet)
{
  ((DateFormat*)fmt)->setCalendar(*((Calendar*)calendarToSet));
}

U_CAPI const UNumberFormat*
udat_getNumberFormat(const UDateFormat* fmt)
{
  return (const UNumberFormat*) ((DateFormat*)fmt)->getNumberFormat();
}

U_CAPI void
udat_setNumberFormat(            UDateFormat*    fmt,
                    const   UNumberFormat*  numberFormatToSet)
{
  ((DateFormat*)fmt)->setNumberFormat(*((NumberFormat*)numberFormatToSet));
}

U_CAPI const char*
udat_getAvailable(int32_t index)
{
  return uloc_getAvailable(index);
}

U_CAPI int32_t
udat_countAvailable()
{
  return uloc_countAvailable();
}

U_CAPI UDate
udat_get2DigitYearStart(    const   UDateFormat     *fmt,
                UErrorCode      *status)
{
  if(U_FAILURE(*status)) return (UDate)0;
  return ((SimpleDateFormat*)fmt)->get2DigitYearStart(*status);
}

U_CAPI void
udat_set2DigitYearStart(    UDateFormat     *fmt,
                UDate           d,
                UErrorCode      *status)
{
  if(U_FAILURE(*status)) return;
  ((SimpleDateFormat*)fmt)->set2DigitYearStart(d, *status);
}

U_CAPI int32_t
udat_toPattern(    const   UDateFormat     *fmt,
        UBool          localized,
        UChar           *result,
        int32_t         resultLength,
        UErrorCode      *status)
{
  if(U_FAILURE(*status)) return -1;

  UnicodeString res(result, 0, resultLength);

  if(localized)
    ((SimpleDateFormat*)fmt)->toLocalizedPattern(res, *status);
  else
    ((SimpleDateFormat*)fmt)->toPattern(res);

  return uprv_fillOutputString(res, result, resultLength, status);
}

// TBD: should this take an UErrorCode?
U_CAPI void
udat_applyPattern(            UDateFormat     *format,
                    UBool          localized,
                    const   UChar           *pattern,
                    int32_t         patternLength)
{
  int32_t len = (patternLength == -1 ? u_strlen(pattern) : patternLength);
  const UnicodeString pat((UChar*)pattern, len, len);
  UErrorCode status = U_ZERO_ERROR;

  if(localized)
    ((SimpleDateFormat*)format)->applyLocalizedPattern(pat, status);
  else
    ((SimpleDateFormat*)format)->applyPattern(pat);
}

U_CAPI int32_t
udat_getSymbols(const   UDateFormat             *fmt,
        UDateFormatSymbolType   type,
        int32_t                 index,
        UChar                   *result,
        int32_t                 resultLength,
        UErrorCode              *status)
{
  if(U_FAILURE(*status)) return -1;

  const DateFormatSymbols *syms = 
    ((SimpleDateFormat*)fmt)->getDateFormatSymbols();
  int32_t count;
  const UnicodeString *res;

  switch(type) {
  case UDAT_ERAS:
    res = syms->getEras(count);
    if(index < count) {
      return uprv_fillOutputString(res[index], result, resultLength, status);
    }
    break;

  case UDAT_MONTHS:
    res = syms->getMonths(count);
    if(index < count) {
      return uprv_fillOutputString(res[index], result, resultLength, status);
    }
    break;

  case UDAT_SHORT_MONTHS:
    res = syms->getShortMonths(count);
    if(index < count) {
      return uprv_fillOutputString(res[index], result, resultLength, status);
    }
    break;

  case UDAT_WEEKDAYS:
    res = syms->getWeekdays(count);
    if(index < count) {
      return uprv_fillOutputString(res[index], result, resultLength, status);
    }
    break;

  case UDAT_SHORT_WEEKDAYS:
    res = syms->getShortWeekdays(count);
    if(index < count) {
      return uprv_fillOutputString(res[index], result, resultLength, status);
    }
    break;

  case UDAT_AM_PMS:
    res = syms->getAmPmStrings(count);
    if(index < count) {
      return uprv_fillOutputString(res[index], result, resultLength, status);
    }
    break;

  case UDAT_LOCALIZED_CHARS:
    {
      UnicodeString res1(result, 0, resultLength);
      syms->getLocalPatternChars(res1);
      return uprv_fillOutputString(res1, result, resultLength, status);
    }
  }
  
  return 0;
}

U_CAPI int32_t
udat_countSymbols(    const    UDateFormat                *fmt,
            UDateFormatSymbolType    type)
{
  const DateFormatSymbols *syms = 
    ((SimpleDateFormat*)fmt)->getDateFormatSymbols();
  int32_t count = 0;

  switch(type) {
  case UDAT_ERAS:
    syms->getEras(count);
    break;

  case UDAT_MONTHS:
    syms->getMonths(count);
    break;

  case UDAT_SHORT_MONTHS:
    syms->getShortMonths(count);
    break;

  case UDAT_WEEKDAYS:
    syms->getWeekdays(count);
    break;

  case UDAT_SHORT_WEEKDAYS:
    syms->getShortWeekdays(count);
    break;

  case UDAT_AM_PMS:
    syms->getAmPmStrings(count);
    break;

  case UDAT_LOCALIZED_CHARS:
    count = 1;
    break;
  }
  
  return count;
}

U_CAPI void
udat_setSymbols(    UDateFormat             *format,
            UDateFormatSymbolType   type,
            int32_t                 index,
            UChar                   *value,
            int32_t                 valueLength,
            UErrorCode              *status)
{
  if(U_FAILURE(*status)) return;

  int32_t count;
  int32_t len = (valueLength == -1 ? u_strlen(value) : valueLength);
  const UnicodeString val((UChar*)value, len, len);
  const UnicodeString *res;
  DateFormatSymbols *syms = new DateFormatSymbols( 
    * ((SimpleDateFormat*)format)->getDateFormatSymbols() );
  UnicodeString *array = 0;

  if(syms == 0) {
    *status = U_MEMORY_ALLOCATION_ERROR;
    return;
  }

  switch(type) {
  case UDAT_ERAS:
    res = syms->getEras(count);
    array = new UnicodeString[count];
    if(array == 0) {
      *status = U_MEMORY_ALLOCATION_ERROR;
      return;
    }
    uprv_arrayCopy(res, array, count);
    if(index < count)
      array[index] = val;
    syms->setEras(array, count);
    break;

  case UDAT_MONTHS:
    res = syms->getMonths(count);
    array = new UnicodeString[count];
    if(array == 0) {
      *status = U_MEMORY_ALLOCATION_ERROR;
      return;
    }
    uprv_arrayCopy(res, array, count);
    if(index < count)
      array[index] = val;
    syms->setMonths(array, count);
    break;

  case UDAT_SHORT_MONTHS:
    res = syms->getShortMonths(count);
    array = new UnicodeString[count];
    if(array == 0) {
      *status = U_MEMORY_ALLOCATION_ERROR;
      return;
    }
    uprv_arrayCopy(res, array, count);
    if(index < count)
      array[index] = val;
    syms->setShortMonths(array, count);
    break;

  case UDAT_WEEKDAYS:
    res = syms->getWeekdays(count);
    array = new UnicodeString[count];
    if(array == 0) {
      *status = U_MEMORY_ALLOCATION_ERROR;
      return;
    }
    uprv_arrayCopy(res, array, count);
    if(index < count)
      array[index] = val;
    syms->setWeekdays(array, count);
    break;

  case UDAT_SHORT_WEEKDAYS:
    res = syms->getShortWeekdays(count);
    array = new UnicodeString[count];
    if(array == 0) {
      *status = U_MEMORY_ALLOCATION_ERROR;
      return;
    }
    uprv_arrayCopy(res, array, count);
    if(index < count)
      array[index] = val;
    syms->setShortWeekdays(array, count);
    break;

  case UDAT_AM_PMS:
    res = syms->getAmPmStrings(count);
    array = new UnicodeString[count];
    if(array == 0) {
      *status = U_MEMORY_ALLOCATION_ERROR;
      return;
    }
    uprv_arrayCopy(res, array, count);
    if(index < count)
      array[index] = val;
    syms->setAmPmStrings(array, count);
    break;

  case UDAT_LOCALIZED_CHARS:
    syms->setLocalPatternChars(val);
    break;
  }
  
  ((SimpleDateFormat*)format)->adoptDateFormatSymbols(syms);
  delete [] array;
}
