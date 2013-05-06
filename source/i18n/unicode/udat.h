/*
 *******************************************************************************
 * Copyright (C) 1996-2013, International Business Machines
 * Corporation and others. All Rights Reserved.
 *******************************************************************************
*/

#ifndef UDAT_H
#define UDAT_H

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/localpointer.h"
#include "unicode/ucal.h"
#include "unicode/unum.h"
#include "unicode/udisplaycontext.h"
/**
 * \file
 * \brief C API: DateFormat
 *
 * <h2> Date Format C API</h2>
 *
 * Date Format C API  consists of functions that convert dates and
 * times from their internal representations to textual form and back again in a
 * language-independent manner. Converting from the internal representation (milliseconds
 * since midnight, January 1, 1970) to text is known as "formatting," and converting
 * from text to millis is known as "parsing."  We currently define only one concrete
 * structure UDateFormat, which can handle pretty much all normal
 * date formatting and parsing actions.
 * <P>
 * Date Format helps you to format and parse dates for any locale. Your code can
 * be completely independent of the locale conventions for months, days of the
 * week, or even the calendar format: lunar vs. solar.
 * <P>
 * To format a date for the current Locale with default time and date style,
 * use one of the static factory methods:
 * <pre>
 * \code
 *  UErrorCode status = U_ZERO_ERROR;
 *  UChar *myString;
 *  int32_t myStrlen = 0;
 *  UDateFormat* dfmt = udat_open(UDAT_DEFAULT, UDAT_DEFAULT, NULL, NULL, -1, NULL, -1, &status);
 *  myStrlen = udat_format(dfmt, myDate, NULL, myStrlen, NULL, &status);
 *  if (status==U_BUFFER_OVERFLOW_ERROR){
 *      status=U_ZERO_ERROR;
 *      myString=(UChar*)malloc(sizeof(UChar) * (myStrlen+1) );
 *      udat_format(dfmt, myDate, myString, myStrlen+1, NULL, &status);
 *  }
 * \endcode
 * </pre>
 * If you are formatting multiple numbers, it is more efficient to get the
 * format and use it multiple times so that the system doesn't have to fetch the
 * information about the local language and country conventions multiple times.
 * <pre>
 * \code
 *  UErrorCode status = U_ZERO_ERROR;
 *  int32_t i, myStrlen = 0;
 *  UChar* myString;
 *  char buffer[1024];
 *  UDate myDateArr[] = { 0.0, 100000000.0, 2000000000.0 }; // test values
 *  UDateFormat* df = udat_open(UDAT_DEFAULT, UDAT_DEFAULT, NULL, NULL, -1, NULL, 0, &status);
 *  for (i = 0; i < 3; i++) {
 *      myStrlen = udat_format(df, myDateArr[i], NULL, myStrlen, NULL, &status);
 *      if(status == U_BUFFER_OVERFLOW_ERROR){
 *          status = U_ZERO_ERROR;
 *          myString = (UChar*)malloc(sizeof(UChar) * (myStrlen+1) );
 *          udat_format(df, myDateArr[i], myString, myStrlen+1, NULL, &status);
 *          printf("%s\n", u_austrcpy(buffer, myString) );
 *          free(myString);
 *      }
 *  }
 * \endcode
 * </pre>
 * To get specific fields of a date, you can use UFieldPosition to
 * get specific fields.
 * <pre>
 * \code
 *  UErrorCode status = U_ZERO_ERROR;
 *  UFieldPosition pos;
 *  UChar *myString;
 *  int32_t myStrlen = 0;
 *  char buffer[1024];
 *
 *  pos.field = 1;  // Same as the DateFormat::EField enum
 *  UDateFormat* dfmt = udat_open(UDAT_DEFAULT, UDAT_DEFAULT, NULL, -1, NULL, 0, &status);
 *  myStrlen = udat_format(dfmt, myDate, NULL, myStrlen, &pos, &status);
 *  if (status==U_BUFFER_OVERFLOW_ERROR){
 *      status=U_ZERO_ERROR;
 *      myString=(UChar*)malloc(sizeof(UChar) * (myStrlen+1) );
 *      udat_format(dfmt, myDate, myString, myStrlen+1, &pos, &status);
 *  }
 *  printf("date format: %s\n", u_austrcpy(buffer, myString));
 *  buffer[pos.endIndex] = 0;   // NULL terminate the string.
 *  printf("UFieldPosition position equals %s\n", &buffer[pos.beginIndex]);
 * \endcode
 * </pre>
 * To format a date for a different Locale, specify it in the call to
 * udat_open()
 * <pre>
 * \code
 *        UDateFormat* df = udat_open(UDAT_SHORT, UDAT_SHORT, "fr_FR", NULL, -1, NULL, 0, &status);
 * \endcode
 * </pre>
 * You can use a DateFormat API udat_parse() to parse.
 * <pre>
 * \code
 *  UErrorCode status = U_ZERO_ERROR;
 *  int32_t parsepos=0;
 *  UDate myDate = udat_parse(df, myString, u_strlen(myString), &parsepos, &status);
 * \endcode
 * </pre>
 *  You can pass in different options for the arguments for date and time style
 *  to control the length of the result; from SHORT to MEDIUM to LONG to FULL.
 *  The exact result depends on the locale, but generally:
 *  see UDateFormatStyle for more details
 * <ul type=round>
 *   <li>   UDAT_SHORT is completely numeric, such as 12/13/52 or 3:30pm
 *   <li>   UDAT_MEDIUM is longer, such as Jan 12, 1952
 *   <li>   UDAT_LONG is longer, such as January 12, 1952 or 3:30:32pm
 *   <li>   UDAT_FULL is pretty completely specified, such as
 *          Tuesday, April 12, 1952 AD or 3:30:42pm PST.
 * </ul>
 * You can also set the time zone on the format if you wish.
 * <P>
 * You can also use forms of the parse and format methods with Parse Position and
 * UFieldPosition to allow you to
 * <ul type=round>
 *   <li>   Progressively parse through pieces of a string.
 *   <li>   Align any particular field, or find out where it is for selection
 *          on the screen.
 * </ul>
 */

/** A date formatter.
 *  For usage in C programs.
 *  @stable ICU 2.6
 */
typedef void* UDateFormat;

/** The possible date/time format styles 
 *  @stable ICU 2.6
 */
typedef enum UDateFormatStyle {
    /** Full style */
    UDAT_FULL,
    /** Long style */
    UDAT_LONG,
    /** Medium style */
    UDAT_MEDIUM,
    /** Short style */
    UDAT_SHORT,
    /** Default style */
    UDAT_DEFAULT = UDAT_MEDIUM,

    /** Bitfield for relative date */
    UDAT_RELATIVE = (1 << 7),
    
    UDAT_FULL_RELATIVE = UDAT_FULL | UDAT_RELATIVE,
        
    UDAT_LONG_RELATIVE = UDAT_LONG | UDAT_RELATIVE,
    
    UDAT_MEDIUM_RELATIVE = UDAT_MEDIUM | UDAT_RELATIVE,
    
    UDAT_SHORT_RELATIVE = UDAT_SHORT | UDAT_RELATIVE,
    
    
    /** No style */
    UDAT_NONE = -1,

#ifndef U_HIDE_DRAFT_API
    /**
     * Use the pattern given in the parameter to udat_open
     * @see udat_open
     * @draft ICU 50
     */
    UDAT_PATTERN = -2,

    /** @internal alias to UDAT_PATTERN */
    UDAT_IGNORE = UDAT_PATTERN
#endif  /* U_HIDE_DRAFT_API */
} UDateFormatStyle;

// Skeletons for dates.

/**
 * Constant for date skeleton with year.
 * @stable ICU 4.0
 */
#define UDAT_YEAR                       "y"
#ifndef U_HIDE_DRAFT_API
/**
 * Constant for date skeleton with quarter.
 * @draft ICU 51
 */
#define UDAT_QUARTER                    "QQQQ"
/**
 * Constant for date skeleton with abbreviated quarter.
 * @draft ICU 51
 */
#define UDAT_ABBR_QUARTER               "QQQ"
#endif  /* U_HIDE_DRAFT_API */
/**
 * Constant for date skeleton with year and quarter.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_QUARTER               "yQQQQ"
/**
 * Constant for date skeleton with year and abbreviated quarter.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_ABBR_QUARTER          "yQQQ"
/**
 * Constant for date skeleton with month.
 * @stable ICU 4.0
 */
#define UDAT_MONTH                      "MMMM"
/**
 * Constant for date skeleton with abbreviated month.
 * @stable ICU 4.0
 */
#define UDAT_ABBR_MONTH                 "MMM"
/**
 * Constant for date skeleton with numeric month.
 * @stable ICU 4.0
 */
#define UDAT_NUM_MONTH                  "M"
/**
 * Constant for date skeleton with year and month.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_MONTH                 "yMMMM"
/**
 * Constant for date skeleton with year and abbreviated month.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_ABBR_MONTH            "yMMM"
/**
 * Constant for date skeleton with year and numeric month.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_NUM_MONTH             "yM"
/**
 * Constant for date skeleton with day.
 * @stable ICU 4.0
 */
#define UDAT_DAY                        "d"
/**
 * Constant for date skeleton with year, month, and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_MONTH_DAY             "yMMMMd"
/**
 * Constant for date skeleton with year, abbreviated month, and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_ABBR_MONTH_DAY        "yMMMd"
/**
 * Constant for date skeleton with year, numeric month, and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_NUM_MONTH_DAY         "yMd"
#ifndef U_HIDE_DRAFT_API
/**
 * Constant for date skeleton with weekday.
 * @draft ICU 51
 */
#define UDAT_WEEKDAY                    "EEEE"
/**
 * Constant for date skeleton with abbreviated weekday.
 * @draft ICU 51
 */
#define UDAT_ABBR_WEEKDAY               "E"
#endif  /* U_HIDE_DRAFT_API */
/**
 * Constant for date skeleton with year, month, weekday, and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_MONTH_WEEKDAY_DAY     "yMMMMEEEEd"
/**
 * Constant for date skeleton with year, abbreviated month, weekday, and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_ABBR_MONTH_WEEKDAY_DAY "yMMMEd" 
/**
 * Constant for date skeleton with year, numeric month, weekday, and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_YEAR_NUM_MONTH_WEEKDAY_DAY "yMEd"
/**
 * Constant for date skeleton with long month and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_MONTH_DAY                  "MMMMd"
/**
 * Constant for date skeleton with abbreviated month and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_ABBR_MONTH_DAY             "MMMd"
/**
 * Constant for date skeleton with numeric month and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_NUM_MONTH_DAY              "Md"
/**
 * Constant for date skeleton with month, weekday, and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_MONTH_WEEKDAY_DAY          "MMMMEEEEd"
/**
 * Constant for date skeleton with abbreviated month, weekday, and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_ABBR_MONTH_WEEKDAY_DAY     "MMMEd"
/**
 * Constant for date skeleton with numeric month, weekday, and day.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_NUM_MONTH_WEEKDAY_DAY      "MEd"

// Skeletons for times.

/**
 * Constant for date skeleton with hour, with the locale's preferred hour format (12 or 24).
 * @stable ICU 4.0
 */
#define UDAT_HOUR                       "j"
#ifndef U_HIDE_DRAFT_API
/**
 * Constant for date skeleton with hour in 24-hour presentation.
 * @draft ICU 51
 */
#define UDAT_HOUR24                     "H"
/**
 * Constant for date skeleton with minute.
 * @draft ICU 51
 */
#define UDAT_MINUTE                     "m"
#endif  /* U_HIDE_DRAFT_API */
/**
 * Constant for date skeleton with hour and minute, with the locale's preferred hour format (12 or 24).
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_HOUR_MINUTE                "jm"
/**
 * Constant for date skeleton with hour and minute in 24-hour presentation.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_HOUR24_MINUTE              "Hm"
#ifndef U_HIDE_DRAFT_API
/**
 * Constant for date skeleton with second.
 * @draft ICU 51
 */
#define UDAT_SECOND                     "s"
#endif  /* U_HIDE_DRAFT_API */
/**
 * Constant for date skeleton with hour, minute, and second,
 * with the locale's preferred hour format (12 or 24).
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_HOUR_MINUTE_SECOND         "jms"
/**
 * Constant for date skeleton with hour, minute, and second in
 * 24-hour presentation.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_HOUR24_MINUTE_SECOND       "Hms"
/**
 * Constant for date skeleton with minute and second.
 * Used in combinations date + time, date + time + zone, or time + zone.
 * @stable ICU 4.0
 */
#define UDAT_MINUTE_SECOND              "ms"

// Skeletons for time zones.

#ifndef U_HIDE_DRAFT_API
/**
 * Constant for <i>generic location format</i>, such as Los Angeles Time;
 * used in combinations date + time + zone, or time + zone.
 * @see <a href="http://unicode.org/reports/tr35/#Date_Format_Patterns">LDML Date Format Patterns</a>
 * @see <a href="http://unicode.org/reports/tr35/#Time_Zone_Fallback">LDML Time Zone Fallback</a>
 * @draft ICU 51
 */
#define UDAT_LOCATION_TZ "VVVV"
/**
 * Constant for <i>generic non-location format</i>, such as Pacific Time;
 * used in combinations date + time + zone, or time + zone.
 * @see <a href="http://unicode.org/reports/tr35/#Date_Format_Patterns">LDML Date Format Patterns</a>
 * @see <a href="http://unicode.org/reports/tr35/#Time_Zone_Fallback">LDML Time Zone Fallback</a>
 * @draft ICU 51
 */
#define UDAT_GENERIC_TZ "vvvv"
/**
 * Constant for <i>generic non-location format</i>, abbreviated if possible, such as PT;
 * used in combinations date + time + zone, or time + zone.
 * @see <a href="http://unicode.org/reports/tr35/#Date_Format_Patterns">LDML Date Format Patterns</a>
 * @see <a href="http://unicode.org/reports/tr35/#Time_Zone_Fallback">LDML Time Zone Fallback</a>
 * @draft ICU 51
 */
#define UDAT_ABBR_GENERIC_TZ "v"
/**
 * Constant for <i>specific non-location format</i>, such as Pacific Daylight Time;
 * used in combinations date + time + zone, or time + zone.
 * @see <a href="http://unicode.org/reports/tr35/#Date_Format_Patterns">LDML Date Format Patterns</a>
 * @see <a href="http://unicode.org/reports/tr35/#Time_Zone_Fallback">LDML Time Zone Fallback</a>
 * @draft ICU 51
 */
#define UDAT_SPECIFIC_TZ "zzzz"
/**
 * Constant for <i>specific non-location format</i>, abbreviated if possible, such as PDT;
 * used in combinations date + time + zone, or time + zone.
 * @see <a href="http://unicode.org/reports/tr35/#Date_Format_Patterns">LDML Date Format Patterns</a>
 * @see <a href="http://unicode.org/reports/tr35/#Time_Zone_Fallback">LDML Time Zone Fallback</a>
 * @draft ICU 51
 */
#define UDAT_ABBR_SPECIFIC_TZ "z"
/**
 * Constant for <i>localized GMT/UTC format</i>, such as GMT+8:00 or HPG-8:00;
 * used in combinations date + time + zone, or time + zone.
 * @see <a href="http://unicode.org/reports/tr35/#Date_Format_Patterns">LDML Date Format Patterns</a>
 * @see <a href="http://unicode.org/reports/tr35/#Time_Zone_Fallback">LDML Time Zone Fallback</a>
 * @draft ICU 51
 */
#define UDAT_ABBR_UTC_TZ "ZZZZ"
#endif  /* U_HIDE_DRAFT_API */

// deprecated skeleton constants

#ifndef U_HIDE_DEPRECATED_API 
/**
 * Constant for date skeleton with standalone month.
 * @deprecated ICU 50 Use UDAT_MONTH instead.
 */
#define UDAT_STANDALONE_MONTH           "LLLL"
/**
 * Constant for date skeleton with standalone abbreviated month.
 * @deprecated ICU 50 Use UDAT_ABBR_MONTH instead.
 */
#define UDAT_ABBR_STANDALONE_MONTH      "LLL"

/**
 * Constant for date skeleton with hour, minute, and generic timezone.
 * @deprecated ICU 50 Use instead UDAT_HOUR_MINUTE UDAT_ABBR_GENERIC_TZ or some other timezone presentation.
 */
#define UDAT_HOUR_MINUTE_GENERIC_TZ     "jmv"
/**
 * Constant for date skeleton with hour, minute, and timezone.
 * @deprecated ICU 50 Use instead UDAT_HOUR_MINUTE UDAT_ABBR_SPECIFIC_TZ or some other timezone presentation.
 */
#define UDAT_HOUR_MINUTE_TZ             "jmz"
/**
 * Constant for date skeleton with hour and generic timezone.
 * @deprecated ICU 50 Use instead UDAT_HOUR UDAT_ABBR_GENERIC_TZ or some other timezone presentation.
 */
#define UDAT_HOUR_GENERIC_TZ            "jv"
/**
 * Constant for date skeleton with hour and timezone.
 * @deprecated ICU 50 Use instead UDAT_HOUR UDAT_ABBR_SPECIFIC_TZ or some other timezone presentation.
 */
#define UDAT_HOUR_TZ                    "jz"
#endif  /* U_HIDE_DEPRECATED_API */

/**
 * FieldPosition and UFieldPosition selectors for format fields
 * defined by DateFormat and UDateFormat.
 * @stable ICU 3.0
 */
typedef enum UDateFormatField {
    /**
     * FieldPosition and UFieldPosition selector for 'G' field alignment,
     * corresponding to the UCAL_ERA field.
     * @stable ICU 3.0
     */
    UDAT_ERA_FIELD = 0,

    /**
     * FieldPosition and UFieldPosition selector for 'y' field alignment,
     * corresponding to the UCAL_YEAR field.
     * @stable ICU 3.0
     */
    UDAT_YEAR_FIELD = 1,

    /**
     * FieldPosition and UFieldPosition selector for 'M' field alignment,
     * corresponding to the UCAL_MONTH field.
     * @stable ICU 3.0
     */
    UDAT_MONTH_FIELD = 2,

    /**
     * FieldPosition and UFieldPosition selector for 'd' field alignment,
     * corresponding to the UCAL_DATE field.
     * @stable ICU 3.0
     */
    UDAT_DATE_FIELD = 3,

    /**
     * FieldPosition and UFieldPosition selector for 'k' field alignment,
     * corresponding to the UCAL_HOUR_OF_DAY field.
     * UDAT_HOUR_OF_DAY1_FIELD is used for the one-based 24-hour clock.
     * For example, 23:59 + 01:00 results in 24:59.
     * @stable ICU 3.0
     */
    UDAT_HOUR_OF_DAY1_FIELD = 4,

    /**
     * FieldPosition and UFieldPosition selector for 'H' field alignment,
     * corresponding to the UCAL_HOUR_OF_DAY field.
     * UDAT_HOUR_OF_DAY0_FIELD is used for the zero-based 24-hour clock.
     * For example, 23:59 + 01:00 results in 00:59.
     * @stable ICU 3.0
     */
    UDAT_HOUR_OF_DAY0_FIELD = 5,

    /**
     * FieldPosition and UFieldPosition selector for 'm' field alignment,
     * corresponding to the UCAL_MINUTE field.
     * @stable ICU 3.0
     */
    UDAT_MINUTE_FIELD = 6,

    /**
     * FieldPosition and UFieldPosition selector for 's' field alignment,
     * corresponding to the UCAL_SECOND field.
     * @stable ICU 3.0
     */
    UDAT_SECOND_FIELD = 7,

    /**
     * FieldPosition and UFieldPosition selector for 'S' field alignment,
     * corresponding to the UCAL_MILLISECOND field.
     *
     * Note: Time formats that use 'S' can display a maximum of three
     * significant digits for fractional seconds, corresponding to millisecond
     * resolution and a fractional seconds sub-pattern of SSS. If the
     * sub-pattern is S or SS, the fractional seconds value will be truncated
     * (not rounded) to the number of display places specified. If the
     * fractional seconds sub-pattern is longer than SSS, the additional
     * display places will be filled with zeros.
     * @stable ICU 3.0
     */
    UDAT_FRACTIONAL_SECOND_FIELD = 8,

    /**
     * FieldPosition and UFieldPosition selector for 'E' field alignment,
     * corresponding to the UCAL_DAY_OF_WEEK field.
     * @stable ICU 3.0
     */
    UDAT_DAY_OF_WEEK_FIELD = 9,

    /**
     * FieldPosition and UFieldPosition selector for 'D' field alignment,
     * corresponding to the UCAL_DAY_OF_YEAR field.
     * @stable ICU 3.0
     */
    UDAT_DAY_OF_YEAR_FIELD = 10,

    /**
     * FieldPosition and UFieldPosition selector for 'F' field alignment,
     * corresponding to the UCAL_DAY_OF_WEEK_IN_MONTH field.
     * @stable ICU 3.0
     */
    UDAT_DAY_OF_WEEK_IN_MONTH_FIELD = 11,

    /**
     * FieldPosition and UFieldPosition selector for 'w' field alignment,
     * corresponding to the UCAL_WEEK_OF_YEAR field.
     * @stable ICU 3.0
     */
    UDAT_WEEK_OF_YEAR_FIELD = 12,

    /**
     * FieldPosition and UFieldPosition selector for 'W' field alignment,
     * corresponding to the UCAL_WEEK_OF_MONTH field.
     * @stable ICU 3.0
     */
    UDAT_WEEK_OF_MONTH_FIELD = 13,

    /**
     * FieldPosition and UFieldPosition selector for 'a' field alignment,
     * corresponding to the UCAL_AM_PM field.
     * @stable ICU 3.0
     */
    UDAT_AM_PM_FIELD = 14,

    /**
     * FieldPosition and UFieldPosition selector for 'h' field alignment,
     * corresponding to the UCAL_HOUR field.
     * UDAT_HOUR1_FIELD is used for the one-based 12-hour clock.
     * For example, 11:30 PM + 1 hour results in 12:30 AM.
     * @stable ICU 3.0
     */
    UDAT_HOUR1_FIELD = 15,

    /**
     * FieldPosition and UFieldPosition selector for 'K' field alignment,
     * corresponding to the UCAL_HOUR field.
     * UDAT_HOUR0_FIELD is used for the zero-based 12-hour clock.
     * For example, 11:30 PM + 1 hour results in 00:30 AM.
     * @stable ICU 3.0
     */
    UDAT_HOUR0_FIELD = 16,

    /**
     * FieldPosition and UFieldPosition selector for 'z' field alignment,
     * corresponding to the UCAL_ZONE_OFFSET and
     * UCAL_DST_OFFSET fields.
     * @stable ICU 3.0
     */
    UDAT_TIMEZONE_FIELD = 17,

    /**
     * FieldPosition and UFieldPosition selector for 'Y' field alignment,
     * corresponding to the UCAL_YEAR_WOY field.
     * @stable ICU 3.0
     */
    UDAT_YEAR_WOY_FIELD = 18,

    /**
     * FieldPosition and UFieldPosition selector for 'e' field alignment,
     * corresponding to the UCAL_DOW_LOCAL field.
     * @stable ICU 3.0
     */
    UDAT_DOW_LOCAL_FIELD = 19,

    /**
     * FieldPosition and UFieldPosition selector for 'u' field alignment,
     * corresponding to the UCAL_EXTENDED_YEAR field.
     * @stable ICU 3.0
     */
    UDAT_EXTENDED_YEAR_FIELD = 20,

    /**
     * FieldPosition and UFieldPosition selector for 'g' field alignment,
     * corresponding to the UCAL_JULIAN_DAY field.
     * @stable ICU 3.0
     */
    UDAT_JULIAN_DAY_FIELD = 21,

    /**
     * FieldPosition and UFieldPosition selector for 'A' field alignment,
     * corresponding to the UCAL_MILLISECONDS_IN_DAY field.
     * @stable ICU 3.0
     */
    UDAT_MILLISECONDS_IN_DAY_FIELD = 22,

    /**
     * FieldPosition and UFieldPosition selector for 'Z' field alignment,
     * corresponding to the UCAL_ZONE_OFFSET and
     * UCAL_DST_OFFSET fields.
     * @stable ICU 3.0
     */
    UDAT_TIMEZONE_RFC_FIELD = 23,

    /**
     * FieldPosition and UFieldPosition selector for 'v' field alignment,
     * corresponding to the UCAL_ZONE_OFFSET field.
     * @stable ICU 3.4
     */
    UDAT_TIMEZONE_GENERIC_FIELD = 24,
    /**
     * FieldPosition selector for 'c' field alignment,
     * corresponding to the {@link #UCAL_DOW_LOCAL} field.
     * This displays the stand alone day name, if available.
     * @stable ICU 3.4
     */
    UDAT_STANDALONE_DAY_FIELD = 25,

    /**
     * FieldPosition selector for 'L' field alignment,
     * corresponding to the {@link #UCAL_MONTH} field.
     * This displays the stand alone month name, if available.
     * @stable ICU 3.4
     */
    UDAT_STANDALONE_MONTH_FIELD = 26,

    /**
     * FieldPosition selector for "Q" field alignment,
     * corresponding to quarters. This is implemented
     * using the {@link #UCAL_MONTH} field. This
     * displays the quarter.
     * @stable ICU 3.6
     */
    UDAT_QUARTER_FIELD = 27,

    /**
     * FieldPosition selector for the "q" field alignment,
     * corresponding to stand-alone quarters. This is
     * implemented using the {@link #UCAL_MONTH} field.
     * This displays the stand-alone quarter.
     * @stable ICU 3.6
     */
    UDAT_STANDALONE_QUARTER_FIELD = 28,

    /**
     * FieldPosition and UFieldPosition selector for 'V' field alignment,
     * corresponding to the UCAL_ZONE_OFFSET field.
     * @stable ICU 3.8
     */
    UDAT_TIMEZONE_SPECIAL_FIELD = 29,

    /**
     * FieldPosition selector for "U" field alignment,
     * corresponding to cyclic year names. This is implemented
     * using the {@link #UCAL_YEAR} field. This displays
     * the cyclic year name, if available.
     * @stable ICU 49
     */
    UDAT_YEAR_NAME_FIELD = 30,

#ifndef U_HIDE_DRAFT_API
    /**
     * FieldPosition selector for 'O' field alignment,
     * corresponding to the UCAL_ZONE_OFFSET and UCAL_DST_OFFSETfields.
     * This displays the localized GMT format.
     * @draft ICU 51
     */
    UDAT_TIMEZONE_LOCALIZED_GMT_OFFSET_FIELD = 31,

    /**
     * FieldPosition selector for 'X' field alignment,
     * corresponding to the UCAL_ZONE_OFFSET and UCAL_DST_OFFSETfields.
     * This displays the ISO 8601 local time offset format or UTC indicator ("Z").
     * @draft ICU 51
     */
    UDAT_TIMEZONE_ISO_FIELD = 32,

    /**
     * FieldPosition selector for 'x' field alignment,
     * corresponding to the UCAL_ZONE_OFFSET and UCAL_DST_OFFSETfields.
     * This displays the ISO 8601 local time offset format.
     * @draft ICU 51
     */
    UDAT_TIMEZONE_ISO_LOCAL_FIELD = 33,
#endif  /* U_HIDE_DRAFT_API */

    /**
     * Number of FieldPosition and UFieldPosition selectors for
     * DateFormat and UDateFormat.
     * Valid selectors range from 0 to UDAT_FIELD_COUNT-1.
     * This value is subject to change if new fields are defined
     * in the future.
     * @stable ICU 3.0
     */
    UDAT_FIELD_COUNT = 34

} UDateFormatField;


/**
 * Maps from a UDateFormatField to the corresponding UCalendarDateFields.
 * Note: since the mapping is many-to-one, there is no inverse mapping.
 * @param field the UDateFormatField.
 * @return the UCalendarDateField.  This will be UCAL_FIELD_COUNT in case
 * of error (e.g., the input field is UDAT_FIELD_COUNT).
 * @stable ICU 4.4
 */
U_STABLE UCalendarDateFields U_EXPORT2
udat_toCalendarDateField(UDateFormatField field);


/**
 * Open a new UDateFormat for formatting and parsing dates and times.
 * A UDateFormat may be used to format dates in calls to {@link #udat_format },
 * and to parse dates in calls to {@link #udat_parse }.
 * @param timeStyle The style used to format times; one of UDAT_FULL, UDAT_LONG,
 * UDAT_MEDIUM, UDAT_SHORT, UDAT_DEFAULT, or UDAT_NONE (relative time styles
 * are not currently supported).
 * When the pattern parameter is used, pass in UDAT_PATTERN for both timeStyle and dateStyle.
 * @param dateStyle The style used to format dates; one of UDAT_FULL, UDAT_LONG,
 * UDAT_MEDIUM, UDAT_SHORT, UDAT_DEFAULT, UDAT_FULL_RELATIVE, UDAT_LONG_RELATIVE,
 * UDAT_MEDIUM_RELATIVE, UDAT_SHORT_RELATIVE, or UDAT_NONE.
 * When the pattern parameter is used, pass in UDAT_PATTERN for both timeStyle and dateStyle.
 * As currently implemented,
 * relative date formatting only affects a limited range of calendar days before or
 * after the current date, based on the CLDR &lt;field type="day"&gt;/&lt;relative&gt; data: For
 * example, in English, "Yesterday", "Today", and "Tomorrow". Outside of this range,
 * dates are formatted using the corresponding non-relative style.
 * @param locale The locale specifying the formatting conventions
 * @param tzID A timezone ID specifying the timezone to use.  If 0, use
 * the default timezone.
 * @param tzIDLength The length of tzID, or -1 if null-terminated.
 * @param pattern A pattern specifying the format to use.
 * @param patternLength The number of characters in the pattern, or -1 if null-terminated.
 * @param status A pointer to an UErrorCode to receive any errors
 * @return A pointer to a UDateFormat to use for formatting dates and times, or 0 if
 * an error occurred.
 * @stable ICU 2.0
 */
U_STABLE UDateFormat* U_EXPORT2 
udat_open(UDateFormatStyle  timeStyle,
          UDateFormatStyle  dateStyle,
          const char        *locale,
          const UChar       *tzID,
          int32_t           tzIDLength,
          const UChar       *pattern,
          int32_t           patternLength,
          UErrorCode        *status);


/**
* Close a UDateFormat.
* Once closed, a UDateFormat may no longer be used.
* @param format The formatter to close.
* @stable ICU 2.0
*/
U_STABLE void U_EXPORT2 
udat_close(UDateFormat* format);

#if U_SHOW_CPLUSPLUS_API

U_NAMESPACE_BEGIN

/**
 * \class LocalUDateFormatPointer
 * "Smart pointer" class, closes a UDateFormat via udat_close().
 * For most methods see the LocalPointerBase base class.
 *
 * @see LocalPointerBase
 * @see LocalPointer
 * @stable ICU 4.4
 */
U_DEFINE_LOCAL_OPEN_POINTER(LocalUDateFormatPointer, UDateFormat, udat_close);

U_NAMESPACE_END

#endif

/**
 * Open a copy of a UDateFormat.
 * This function performs a deep copy.
 * @param fmt The format to copy
 * @param status A pointer to an UErrorCode to receive any errors.
 * @return A pointer to a UDateFormat identical to fmt.
 * @stable ICU 2.0
 */
U_STABLE UDateFormat* U_EXPORT2 
udat_clone(const UDateFormat *fmt,
       UErrorCode *status);

/**
* Format a date using an UDateFormat.
* The date will be formatted using the conventions specified in {@link #udat_open }
* @param format The formatter to use
* @param dateToFormat The date to format
* @param result A pointer to a buffer to receive the formatted number.
* @param resultLength The maximum size of result.
* @param position A pointer to a UFieldPosition.  On input, position->field
* is read.  On output, position->beginIndex and position->endIndex indicate
* the beginning and ending indices of field number position->field, if such
* a field exists.  This parameter may be NULL, in which case no field
* position data is returned.
* @param status A pointer to an UErrorCode to receive any errors
* @return The total buffer size needed; if greater than resultLength, the output was truncated.
* @see udat_parse
* @see UFieldPosition
* @stable ICU 2.0
*/
U_STABLE int32_t U_EXPORT2 
udat_format(    const    UDateFormat*    format,
                        UDate           dateToFormat,
                        UChar*          result,
                        int32_t         resultLength,
                        UFieldPosition* position,
                        UErrorCode*     status);

/**
* Parse a string into an date/time using a UDateFormat.
* The date will be parsed using the conventions specified in {@link #udat_open }.
* <P>
* Note that the normal date formats associated with some calendars - such
* as the Chinese lunar calendar - do not specify enough fields to enable
* dates to be parsed unambiguously. In the case of the Chinese lunar
* calendar, while the year within the current 60-year cycle is specified,
* the number of such cycles since the start date of the calendar (in the
* UCAL_ERA field of the UCalendar object) is not normally part of the format,
* and parsing may assume the wrong era. For cases such as this it is
* recommended that clients parse using udat_parseCalendar with the UCalendar
* passed in set to the current date, or to a date within the era/cycle that
* should be assumed if absent in the format.
*
* @param format The formatter to use.
* @param text The text to parse.
* @param textLength The length of text, or -1 if null-terminated.
* @param parsePos If not 0, on input a pointer to an integer specifying the offset at which
* to begin parsing.  If not 0, on output the offset at which parsing ended.
* @param status A pointer to an UErrorCode to receive any errors
* @return The value of the parsed date/time
* @see udat_format
* @stable ICU 2.0
*/
U_STABLE UDate U_EXPORT2 
udat_parse(const    UDateFormat*    format,
           const    UChar*          text,
                    int32_t         textLength,
                    int32_t         *parsePos,
                    UErrorCode      *status);

/**
* Parse a string into an date/time using a UDateFormat.
* The date will be parsed using the conventions specified in {@link #udat_open }.
* @param format The formatter to use.
* @param calendar A calendar set on input to the date and time to be used for
*                 missing values in the date/time string being parsed, and set
*                 on output to the parsed date/time. When the calendar type is
*                 different from the internal calendar held by the UDateFormat
*                 instance, the internal calendar will be cloned to a work
*                 calendar set to the same milliseconds and time zone as this
*                 calendar parameter, field values will be parsed based on the
*                 work calendar, then the result (milliseconds and time zone)
*                 will be set in this calendar.
* @param text The text to parse.
* @param textLength The length of text, or -1 if null-terminated.
* @param parsePos If not 0, on input a pointer to an integer specifying the offset at which
* to begin parsing.  If not 0, on output the offset at which parsing ended.
* @param status A pointer to an UErrorCode to receive any errors
* @see udat_format
* @stable ICU 2.0
*/
U_STABLE void U_EXPORT2 
udat_parseCalendar(const    UDateFormat*    format,
                            UCalendar*      calendar,
                   const    UChar*          text,
                            int32_t         textLength,
                            int32_t         *parsePos,
                            UErrorCode      *status);

/**
* Determine if an UDateFormat will perform lenient parsing.
* With lenient parsing, the parser may use heuristics to interpret inputs that do not
* precisely match the pattern. With strict parsing, inputs must match the pattern.
* @param fmt The formatter to query
* @return TRUE if fmt is set to perform lenient parsing, FALSE otherwise.
* @see udat_setLenient
* @stable ICU 2.0
*/
U_STABLE UBool U_EXPORT2 
udat_isLenient(const UDateFormat* fmt);

/**
* Specify whether an UDateFormat will perform lenient parsing.
* With lenient parsing, the parser may use heuristics to interpret inputs that do not
* precisely match the pattern. With strict parsing, inputs must match the pattern.
* @param fmt The formatter to set
* @param isLenient TRUE if fmt should perform lenient parsing, FALSE otherwise.
* @see dat_isLenient
* @stable ICU 2.0
*/
U_STABLE void U_EXPORT2 
udat_setLenient(    UDateFormat*    fmt,
                    UBool          isLenient);

/**
* Get the UCalendar associated with an UDateFormat.
* A UDateFormat uses a UCalendar to convert a raw value to, for example,
* the day of the week.
* @param fmt The formatter to query.
* @return A pointer to the UCalendar used by fmt.
* @see udat_setCalendar
* @stable ICU 2.0
*/
U_STABLE const UCalendar* U_EXPORT2 
udat_getCalendar(const UDateFormat* fmt);

/**
* Set the UCalendar associated with an UDateFormat.
* A UDateFormat uses a UCalendar to convert a raw value to, for example,
* the day of the week.
* @param fmt The formatter to set.
* @param calendarToSet A pointer to an UCalendar to be used by fmt.
* @see udat_setCalendar
* @stable ICU 2.0
*/
U_STABLE void U_EXPORT2 
udat_setCalendar(            UDateFormat*    fmt,
                    const   UCalendar*      calendarToSet);

/**
* Get the UNumberFormat associated with an UDateFormat.
* A UDateFormat uses a UNumberFormat to format numbers within a date,
* for example the day number.
* @param fmt The formatter to query.
* @return A pointer to the UNumberFormat used by fmt to format numbers.
* @see udat_setNumberFormat
* @stable ICU 2.0
*/
U_STABLE const UNumberFormat* U_EXPORT2 
udat_getNumberFormat(const UDateFormat* fmt);

/**
* Set the UNumberFormat associated with an UDateFormat.
* A UDateFormat uses a UNumberFormat to format numbers within a date,
* for example the day number.
* @param fmt The formatter to set.
* @param numberFormatToSet A pointer to the UNumberFormat to be used by fmt to format numbers.
* @see udat_getNumberFormat
* @stable ICU 2.0
*/
U_STABLE void U_EXPORT2 
udat_setNumberFormat(            UDateFormat*    fmt,
                        const   UNumberFormat*  numberFormatToSet);

/**
* Get a locale for which date/time formatting patterns are available.
* A UDateFormat in a locale returned by this function will perform the correct
* formatting and parsing for the locale.
* @param localeIndex The index of the desired locale.
* @return A locale for which date/time formatting patterns are available, or 0 if none.
* @see udat_countAvailable
* @stable ICU 2.0
*/
U_STABLE const char* U_EXPORT2 
udat_getAvailable(int32_t localeIndex);

/**
* Determine how many locales have date/time  formatting patterns available.
* This function is most useful as determining the loop ending condition for
* calls to {@link #udat_getAvailable }.
* @return The number of locales for which date/time formatting patterns are available.
* @see udat_getAvailable
* @stable ICU 2.0
*/
U_STABLE int32_t U_EXPORT2 
udat_countAvailable(void);

/**
* Get the year relative to which all 2-digit years are interpreted.
* For example, if the 2-digit start year is 2100, the year 99 will be
* interpreted as 2199.
* @param fmt The formatter to query.
* @param status A pointer to an UErrorCode to receive any errors
* @return The year relative to which all 2-digit years are interpreted.
* @see udat_Set2DigitYearStart
* @stable ICU 2.0
*/
U_STABLE UDate U_EXPORT2 
udat_get2DigitYearStart(    const   UDateFormat     *fmt,
                                    UErrorCode      *status);

/**
* Set the year relative to which all 2-digit years will be interpreted.
* For example, if the 2-digit start year is 2100, the year 99 will be
* interpreted as 2199.
* @param fmt The formatter to set.
* @param d The year relative to which all 2-digit years will be interpreted.
* @param status A pointer to an UErrorCode to receive any errors
* @see udat_Set2DigitYearStart
* @stable ICU 2.0
*/
U_STABLE void U_EXPORT2 
udat_set2DigitYearStart(    UDateFormat     *fmt,
                            UDate           d,
                            UErrorCode      *status);

/**
* Extract the pattern from a UDateFormat.
* The pattern will follow the pattern syntax rules.
* @param fmt The formatter to query.
* @param localized TRUE if the pattern should be localized, FALSE otherwise.
* @param result A pointer to a buffer to receive the pattern.
* @param resultLength The maximum size of result.
* @param status A pointer to an UErrorCode to receive any errors
* @return The total buffer size needed; if greater than resultLength, the output was truncated.
* @see udat_applyPattern
* @stable ICU 2.0
*/
U_STABLE int32_t U_EXPORT2 
udat_toPattern(    const   UDateFormat     *fmt,
                        UBool          localized,
                        UChar           *result,
                        int32_t         resultLength,
                        UErrorCode      *status);

/**
* Set the pattern used by an UDateFormat.
* The pattern should follow the pattern syntax rules.
* @param format The formatter to set.
* @param localized TRUE if the pattern is localized, FALSE otherwise.
* @param pattern The new pattern
* @param patternLength The length of pattern, or -1 if null-terminated.
* @see udat_toPattern
* @stable ICU 2.0
*/
U_STABLE void U_EXPORT2 
udat_applyPattern(            UDateFormat     *format,
                            UBool          localized,
                    const   UChar           *pattern,
                            int32_t         patternLength);

/** 
 * The possible types of date format symbols 
 * @stable ICU 2.6
 */
typedef enum UDateFormatSymbolType {
    /** The era names, for example AD */
    UDAT_ERAS,
    /** The month names, for example February */
    UDAT_MONTHS,
    /** The short month names, for example Feb. */
    UDAT_SHORT_MONTHS,
    /** The CLDR-style format "wide" weekday names, for example Monday */
    UDAT_WEEKDAYS,
    /**
     * The CLDR-style format "abbreviated" (not "short") weekday names, for example "Mon."
     * For the CLDR-style format "short" weekday names, use UDAT_SHORTER_WEEKDAYS.
     */
    UDAT_SHORT_WEEKDAYS,
    /** The AM/PM names, for example AM */
    UDAT_AM_PMS,
    /** The localized characters */
    UDAT_LOCALIZED_CHARS,
    /** The long era names, for example Anno Domini */
    UDAT_ERA_NAMES,
    /** The narrow month names, for example F */
    UDAT_NARROW_MONTHS,
    /** The CLDR-style format "narrow" weekday names, for example "M" */
    UDAT_NARROW_WEEKDAYS,
    /** Standalone context versions of months */
    UDAT_STANDALONE_MONTHS,
    UDAT_STANDALONE_SHORT_MONTHS,
    UDAT_STANDALONE_NARROW_MONTHS,
    /** The CLDR-style stand-alone "wide" weekday names */
    UDAT_STANDALONE_WEEKDAYS,
    /**
     * The CLDR-style stand-alone "abbreviated" (not "short") weekday names.
     * For the CLDR-style stand-alone "short" weekday names, use UDAT_STANDALONE_SHORTER_WEEKDAYS.
     */
    UDAT_STANDALONE_SHORT_WEEKDAYS,
    /** The CLDR-style stand-alone "narrow" weekday names */
    UDAT_STANDALONE_NARROW_WEEKDAYS,
    /** The quarters, for example 1st Quarter */
    UDAT_QUARTERS,
    /** The short quarter names, for example Q1 */
    UDAT_SHORT_QUARTERS,
    /** Standalone context versions of quarters */
    UDAT_STANDALONE_QUARTERS,
    UDAT_STANDALONE_SHORT_QUARTERS,
#ifndef U_HIDE_DRAFT_API
    /**
     * The CLDR-style short weekday names, e.g. "Su", Mo", etc.
     * These are named "SHORTER" to contrast with the constants using _SHORT_
     * above, which actually get the CLDR-style *abbreviated* versions of the
     * corresponding names.
     * @draft ICU 51
     */
    UDAT_SHORTER_WEEKDAYS,
    /**
     * Standalone version of UDAT_SHORTER_WEEKDAYS.
     * @draft ICU 51
     */
    UDAT_STANDALONE_SHORTER_WEEKDAYS
#endif  /* U_HIDE_DRAFT_API */
} UDateFormatSymbolType;

struct UDateFormatSymbols;
/** Date format symbols.
 *  For usage in C programs.
 *  @stable ICU 2.6
 */
typedef struct UDateFormatSymbols UDateFormatSymbols;

/**
* Get the symbols associated with an UDateFormat.
* The symbols are what a UDateFormat uses to represent locale-specific data,
* for example month or day names.
* @param fmt The formatter to query.
* @param type The type of symbols to get.  One of UDAT_ERAS, UDAT_MONTHS, UDAT_SHORT_MONTHS,
* UDAT_WEEKDAYS, UDAT_SHORT_WEEKDAYS, UDAT_AM_PMS, or UDAT_LOCALIZED_CHARS
* @param symbolIndex The desired symbol of type type.
* @param result A pointer to a buffer to receive the pattern.
* @param resultLength The maximum size of result.
* @param status A pointer to an UErrorCode to receive any errors
* @return The total buffer size needed; if greater than resultLength, the output was truncated.
* @see udat_countSymbols
* @see udat_setSymbols
* @stable ICU 2.0
*/
U_STABLE int32_t U_EXPORT2 
udat_getSymbols(const   UDateFormat             *fmt,
                        UDateFormatSymbolType   type,
                        int32_t                 symbolIndex,
                        UChar                   *result,
                        int32_t                 resultLength,
                        UErrorCode              *status);

/**
* Count the number of particular symbols for an UDateFormat.
* This function is most useful as for detemining the loop termination condition
* for calls to {@link #udat_getSymbols }.
* @param fmt The formatter to query.
* @param type The type of symbols to count.  One of UDAT_ERAS, UDAT_MONTHS, UDAT_SHORT_MONTHS,
* UDAT_WEEKDAYS, UDAT_SHORT_WEEKDAYS, UDAT_AM_PMS, or UDAT_LOCALIZED_CHARS
* @return The number of symbols of type type.
* @see udat_getSymbols
* @see udat_setSymbols
* @stable ICU 2.0
*/
U_STABLE int32_t U_EXPORT2 
udat_countSymbols(    const    UDateFormat                *fmt,
                            UDateFormatSymbolType    type);

/**
* Set the symbols associated with an UDateFormat.
* The symbols are what a UDateFormat uses to represent locale-specific data,
* for example month or day names.
* @param format The formatter to set
* @param type The type of symbols to set.  One of UDAT_ERAS, UDAT_MONTHS, UDAT_SHORT_MONTHS,
* UDAT_WEEKDAYS, UDAT_SHORT_WEEKDAYS, UDAT_AM_PMS, or UDAT_LOCALIZED_CHARS
* @param symbolIndex The index of the symbol to set of type type.
* @param value The new value
* @param valueLength The length of value, or -1 if null-terminated
* @param status A pointer to an UErrorCode to receive any errors
* @see udat_getSymbols
* @see udat_countSymbols
* @stable ICU 2.0
*/
U_STABLE void U_EXPORT2 
udat_setSymbols(    UDateFormat             *format,
                    UDateFormatSymbolType   type,
                    int32_t                 symbolIndex,
                    UChar                   *value,
                    int32_t                 valueLength,
                    UErrorCode              *status);

/**
 * Get the locale for this date format object.
 * You can choose between valid and actual locale.
 * @param fmt The formatter to get the locale from
 * @param type type of the locale we're looking for (valid or actual) 
 * @param status error code for the operation
 * @return the locale name
 * @stable ICU 2.8
 */
U_STABLE const char* U_EXPORT2
udat_getLocaleByType(const UDateFormat *fmt,
                     ULocDataLocaleType type,
                     UErrorCode* status); 

#ifndef U_HIDE_DRAFT_API
/**
 * Set a particular UDisplayContext value in the formatter, such as
 * UDISPCTX_CAPITALIZATION_FOR_STANDALONE.
 * @param fmt The formatter for which to set a UDisplayContext value.
 * @param value The UDisplayContext value to set.
 * @param status A pointer to an UErrorCode to receive any errors
 * @draft ICU 51
 */
U_DRAFT void U_EXPORT2
udat_setContext(UDateFormat* fmt, UDisplayContext value, UErrorCode* status);

/**
 * Get the formatter's UDisplayContext value for the specified UDisplayContextType,
 * such as UDISPCTX_TYPE_CAPITALIZATION.
 * @param fmt The formatter to query.
 * @param type The UDisplayContextType whose value to return
 * @param status A pointer to an UErrorCode to receive any errors
 * @return The UDisplayContextValue for the specified type.
 * @draft ICU 51
 */
U_DRAFT UDisplayContext U_EXPORT2
udat_getContext(UDateFormat* fmt, UDisplayContextType type, UErrorCode* status);

#endif  /* U_HIDE_DRAFT_API */

#ifndef U_HIDE_INTERNAL_API
/**
* Extract the date pattern from a UDateFormat set for relative date formatting.
* The pattern will follow the pattern syntax rules.
* @param fmt The formatter to query.
* @param result A pointer to a buffer to receive the pattern.
* @param resultLength The maximum size of result.
* @param status A pointer to a UErrorCode to receive any errors
* @return The total buffer size needed; if greater than resultLength, the output was truncated.
* @see udat_applyPatternRelative
* @internal ICU 4.2 technology preview
*/
U_INTERNAL int32_t U_EXPORT2 
udat_toPatternRelativeDate(const UDateFormat *fmt,
                           UChar             *result,
                           int32_t           resultLength,
                           UErrorCode        *status);

/**
* Extract the time pattern from a UDateFormat set for relative date formatting.
* The pattern will follow the pattern syntax rules.
* @param fmt The formatter to query.
* @param result A pointer to a buffer to receive the pattern.
* @param resultLength The maximum size of result.
* @param status A pointer to a UErrorCode to receive any errors
* @return The total buffer size needed; if greater than resultLength, the output was truncated.
* @see udat_applyPatternRelative
* @internal ICU 4.2 technology preview
*/
U_INTERNAL int32_t U_EXPORT2 
udat_toPatternRelativeTime(const UDateFormat *fmt,
                           UChar             *result,
                           int32_t           resultLength,
                           UErrorCode        *status);

/**
* Set the date & time patterns used by a UDateFormat set for relative date formatting.
* The patterns should follow the pattern syntax rules.
* @param format The formatter to set.
* @param datePattern The new date pattern
* @param datePatternLength The length of datePattern, or -1 if null-terminated.
* @param timePattern The new time pattern
* @param timePatternLength The length of timePattern, or -1 if null-terminated.
* @param status A pointer to a UErrorCode to receive any errors
* @see udat_toPatternRelativeDate, udat_toPatternRelativeTime
* @internal ICU 4.2 technology preview
*/
U_INTERNAL void U_EXPORT2 
udat_applyPatternRelative(UDateFormat *format,
                          const UChar *datePattern,
                          int32_t     datePatternLength,
                          const UChar *timePattern,
                          int32_t     timePatternLength,
                          UErrorCode  *status);

/**
 * @internal
 * @see udat_open
 */
typedef UDateFormat* (U_EXPORT2 *UDateFormatOpener) (UDateFormatStyle  timeStyle,
                                                    UDateFormatStyle  dateStyle,
                                                    const char        *locale,
                                                    const UChar       *tzID,
                                                    int32_t           tzIDLength,
                                                    const UChar       *pattern,
                                                    int32_t           patternLength,
                                                    UErrorCode        *status);

/**
 * Register a provider factory
 * @internal ICU 49
 */
U_INTERNAL void U_EXPORT2
udat_registerOpener(UDateFormatOpener opener, UErrorCode *status);

/**
 * Un-Register a provider factory
 * @internal ICU 49
 */
U_INTERNAL UDateFormatOpener U_EXPORT2
udat_unregisterOpener(UDateFormatOpener opener, UErrorCode *status);
#endif  /* U_HIDE_INTERNAL_API */


#endif /* #if !UCONFIG_NO_FORMATTING */

#endif
