/*
******************************************************************************
*
*   Copyright (C) 1998-2003, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
******************************************************************************
*
* File uprntf_p.c
*
* Modification History:
*
*   Date        Name        Description
*   11/23/98    stephen     Creation.
*   03/12/99    stephen     Modified for new C API.
*   08/07/2003  george      Reunify printf implementations
******************************************************************************
*/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/ustring.h"

#include "uprntf_p.h"
#include "ufmt_cmn.h"
#include "cmemory.h"

/* ANSI style formatting */
/* Use US-ASCII characters only for formatting */

/* % */
#define UFMT_SIMPLE_PERCENT {ufmt_simple_percent, u_printf_simple_percent_handler}
/* s */
#define UFMT_STRING         {ufmt_string, u_printf_string_handler}
/* c */
#define UFMT_CHAR           {ufmt_char, u_printf_char_handler}
/* d, i */
#define UFMT_INT            {ufmt_int, u_printf_integer_handler}
/* u */
#define UFMT_UINT           {ufmt_int, u_printf_uinteger_handler}
/* o */
#define UFMT_OCTAL          {ufmt_int, u_printf_octal_handler}
/* x, X */
#define UFMT_HEX            {ufmt_int, u_printf_hex_handler}
/* f */
#define UFMT_DOUBLE         {ufmt_double, u_printf_double_handler}
/* e, E */
#define UFMT_SCIENTIFIC     {ufmt_double, u_printf_scientific_handler}
/* g, G */
#define UFMT_SCIDBL         {ufmt_double, u_printf_scidbl_handler}
/* n */
#define UFMT_COUNT          {ufmt_count, u_printf_count_handler}

/* non-ANSI extensions */
/* Use US-ASCII characters only for formatting */

/* p */
#define UFMT_POINTER        {ufmt_pointer, u_printf_pointer_handler}
/* V */
#define UFMT_SPELLOUT       {ufmt_double, u_printf_spellout_handler}
/* P */
#define UFMT_PERCENT        {ufmt_double, u_printf_percent_handler}
/* C  K is old format */
#define UFMT_UCHAR          {ufmt_uchar, u_printf_uchar_handler}
/* S  U is old format */
#define UFMT_USTRING        {ufmt_ustring, u_printf_ustring_handler}


#define UFMT_EMPTY {ufmt_empty, NULL}

typedef struct u_printf_info {
    ufmt_type_info info;
    u_printf_handler *handler;
} u_printf_info;

/**
 * Struct encapsulating a single uprintf format specification.
 */
typedef struct u_printf_spec {
  u_printf_spec_info    fInfo;        /* Information on this spec */
  int32_t        fWidthPos;     /* Position of width in arg list */
  int32_t        fPrecisionPos;    /* Position of precision in arg list */
  int32_t        fArgPos;    /* Position of data in arg list */
} u_printf_spec;

#define UPRINTF_NUM_FMT_HANDLERS 108

/* We do not use handlers for 0-0x1f */
#define UPRINTF_BASE_FMT_HANDLERS 0x20

/* buffer size for formatting */
#define UPRINTF_BUFFER_SIZE 1024
#define UPRINTF_SYMBOL_BUFFER_SIZE 8

static const UChar gNullStr[] = {0x28, 0x6E, 0x75, 0x6C, 0x6C, 0x29, 0}; /* "(null)" */
static const UChar gSpaceStr[] = {0x20, 0}; /* " " */

/* Sets the sign of a format based on u_printf_spec_info */
/* TODO: Is setting the prefix symbol to a positive sign a good idea in all locales? */
static void
u_printf_set_sign(UNumberFormat        *format,
                   const u_printf_spec_info     *info,
                   UChar *prefixBuffer,
                   int32_t *prefixBufLen,
                   UErrorCode *status)
{
    if(info->fShowSign) {
        *prefixBufLen = unum_getTextAttribute(format,
                                              UNUM_POSITIVE_PREFIX,
                                              prefixBuffer,
                                              *prefixBufLen,
                                              status);
        if (info->fSpace) {
            /* Setting UNUM_PLUS_SIGN_SYMBOL affects the exponent too. */
            /* unum_setSymbol(format, UNUM_PLUS_SIGN_SYMBOL, gSpaceStr, 1, &status); */
            unum_setTextAttribute(format, UNUM_POSITIVE_PREFIX, gSpaceStr, 1, status);
        }
        else {
            UChar plusSymbol[UPRINTF_SYMBOL_BUFFER_SIZE];
            int32_t symbolLen;

            symbolLen = unum_getSymbol(format,
                UNUM_PLUS_SIGN_SYMBOL,
                plusSymbol,
                sizeof(plusSymbol)/sizeof(*plusSymbol),
                status);
            unum_setTextAttribute(format,
                UNUM_POSITIVE_PREFIX,
                plusSymbol,
                symbolLen,
                status);
        }
    }
    else {
        *prefixBufLen = 0;
    }
}

static void
u_printf_reset_sign(UNumberFormat        *format,
                   const u_printf_spec_info     *info,
                   UChar *prefixBuffer,
                   int32_t *prefixBufLen,
                   UErrorCode *status)
{
    if(info->fShowSign) {
        unum_setTextAttribute(format,
                              UNUM_POSITIVE_PREFIX,
                              prefixBuffer,
                              *prefixBufLen,
                              status);
    }
}


/* handle a '%' */
static int32_t
u_printf_simple_percent_handler(const u_printf_stream_handler  *handler,
                                void                           *context,
                                ULocaleBundle                  *formatBundle,
                                const u_printf_spec_info       *info,
                                const ufmt_args                *args)
{
    static const UChar PERCENT[] = { UP_PERCENT };

    /* put a single '%' onto the output */
    return handler->write(context, PERCENT, 1);
}

/* handle 's' */
static int32_t
u_printf_string_handler(const u_printf_stream_handler  *handler,
                        void                           *context,
                        ULocaleBundle                  *formatBundle,
                        const u_printf_spec_info       *info,
                        const ufmt_args                *args)
{
    UChar *s;
    UChar buffer[UFMT_DEFAULT_BUFFER_SIZE];
    int32_t len, written;
    int32_t argSize;
    const char *arg = (const char*)(args[0].ptrValue);

    /* convert from the default codepage to Unicode */
    if (arg) {
        argSize = (int32_t)strlen(arg) + 1;
        if (argSize >= MAX_UCHAR_BUFFER_SIZE(buffer)) {
            s = ufmt_defaultCPToUnicode(arg, argSize,
                    (UChar *)uprv_malloc(MAX_UCHAR_BUFFER_NEEDED(argSize)),
                    MAX_UCHAR_BUFFER_NEEDED(argSize));
            if(s == NULL) {
                return 0;
            }
        }
        else {
            s = ufmt_defaultCPToUnicode(arg, argSize, buffer,
                    sizeof(buffer)/sizeof(UChar));
        }
    }
    else {
        s = (UChar *)gNullStr;
    }
    len = u_strlen(s);

    /* width = minimum # of characters to write */
    /* precision = maximum # of characters to write */

    /* precision takes precedence over width */
    /* determine if the string should be truncated */
    if(info->fPrecision != -1 && len > info->fPrecision) {
        written = handler->write(context, s, info->fPrecision);
    }
    /* determine if the string should be padded */
    else {
        written = handler->pad_and_justify(context, info, s, len);
    }

    /* clean up */
    if (gNullStr != s && buffer != s) {
        uprv_free(s);
    }

    return written;
}

static int32_t
u_printf_char_handler(const u_printf_stream_handler  *handler,
                      void                           *context,
                      ULocaleBundle                  *formatBundle,
                      const u_printf_spec_info       *info,
                      const ufmt_args                *args)
{
    UChar s[UTF_MAX_CHAR_LENGTH+1];
    int32_t len = 1, written;
    unsigned char arg = (unsigned char)(args[0].int64Value);

    /* convert from default codepage to Unicode */
    ufmt_defaultCPToUnicode((const char *)&arg, 2, s, sizeof(s)/sizeof(UChar));

    /* Remember that this may be an MBCS character */
    if (arg != 0) {
        len = u_strlen(s);
    }

    /* width = minimum # of characters to write */
    /* precision = maximum # of characters to write */

    /* precision takes precedence over width */
    /* determine if the string should be truncated */
    if(info->fPrecision != -1 && len > info->fPrecision) {
        written = handler->write(context, s, info->fPrecision);
    }
    else {
        /* determine if the string should be padded */
        written = handler->pad_and_justify(context, info, s, len);
    }

    return written;
}

static int32_t
u_printf_double_handler(const u_printf_stream_handler  *handler,
                        void                           *context,
                        ULocaleBundle                  *formatBundle,
                        const u_printf_spec_info       *info,
                        const ufmt_args                *args)
{
    double        num         = (double) (args[0].doubleValue);
    UNumberFormat  *format;
    UChar          result[UPRINTF_BUFFER_SIZE];
    UChar          prefixBuffer[UPRINTF_BUFFER_SIZE];
    int32_t        prefixBufferLen = sizeof(prefixBuffer);
    int32_t        minDecimalDigits;
    int32_t        maxDecimalDigits;
    UErrorCode     status        = U_ZERO_ERROR;

    prefixBuffer[0] = 0;

    /* mask off any necessary bits */
    /*  if(! info->fIsLongDouble)
    num &= DBL_MAX;*/

    /* get the formatter */
    format = u_locbund_getNumberFormat(formatBundle, UNUM_DECIMAL);

    /* handle error */
    if(format == 0)
        return 0;

    /* save the formatter's state */
    minDecimalDigits = unum_getAttribute(format, UNUM_MIN_FRACTION_DIGITS);
    maxDecimalDigits = unum_getAttribute(format, UNUM_MAX_FRACTION_DIGITS);

    /* set the appropriate flags and number of decimal digits on the formatter */
    if(info->fPrecision != -1) {
        /* set the # of decimal digits */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, info->fPrecision);
    }
    else if(info->fPrecision == 0 && ! info->fAlt) {
        /* no decimal point in this case */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 0);
    }
    else if(info->fAlt) {
        /* '#' means always show decimal point */
        /* copy of printf behavior on Solaris - '#' shows 6 digits */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 6);
    }
    else {
        /* # of decimal digits is 6 if precision not specified regardless of locale */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 6);
    }

    /* set whether to show the sign */
    if (info->fShowSign) {
        u_printf_set_sign(format, info, prefixBuffer, &prefixBufferLen, &status);
    }

    /* format the number */
    unum_formatDouble(format, num, result, UPRINTF_BUFFER_SIZE, 0, &status);

    /* restore the number format */
    /* TODO: Is this needed? */
    unum_setAttribute(format, UNUM_MIN_FRACTION_DIGITS, minDecimalDigits);
    unum_setAttribute(format, UNUM_MAX_FRACTION_DIGITS, maxDecimalDigits);

    if (info->fShowSign) {
        /* Reset back to original value regardless of what the error was */
        UErrorCode localStatus = U_ZERO_ERROR;
        u_printf_reset_sign(format, info, prefixBuffer, &prefixBufferLen, &localStatus);
    }

    return handler->pad_and_justify(context, info, result, u_strlen(result));
}

/* HSYS */
static int32_t
u_printf_integer_handler(const u_printf_stream_handler  *handler,
                         void                           *context,
                         ULocaleBundle                  *formatBundle,
                         const u_printf_spec_info       *info,
                         const ufmt_args                *args)
{
    int64_t         num        = args[0].int64Value;
    UNumberFormat   *format;
    UChar           result[UPRINTF_BUFFER_SIZE];
    UChar           prefixBuffer[UPRINTF_BUFFER_SIZE];
    int32_t         prefixBufferLen = sizeof(prefixBuffer);
    int32_t         minDigits     = -1;
    UErrorCode      status        = U_ZERO_ERROR;

    prefixBuffer[0] = 0;

    /* mask off any necessary bits */
    if (info->fIsShort)
        num = (int16_t)num;
    else if (!info->fIsLongLong)
        num = (int32_t)num;

    /* get the formatter */
    format = u_locbund_getNumberFormat(formatBundle, UNUM_DECIMAL);

    /* handle error */
    if(format == 0)
        return 0;

    /* set the appropriate flags on the formatter */

    /* set the minimum integer digits */
    if(info->fPrecision != -1) {
        /* set the minimum # of digits */
        minDigits = unum_getAttribute(format, UNUM_MIN_INTEGER_DIGITS);
        unum_setAttribute(format, UNUM_MIN_INTEGER_DIGITS, info->fPrecision);
    }

    /* set whether to show the sign */
    if(info->fShowSign) {
        u_printf_set_sign(format, info, prefixBuffer, &prefixBufferLen, &status);
    }

    /* format the number */
    unum_formatInt64(format, num, result, UPRINTF_BUFFER_SIZE, 0, &status);

    /* restore the number format */
    if (minDigits != -1) {
        unum_setAttribute(format, UNUM_MIN_INTEGER_DIGITS, minDigits);
    }

    if (info->fShowSign) {
        /* Reset back to original value regardless of what the error was */
        UErrorCode localStatus = U_ZERO_ERROR;
        u_printf_reset_sign(format, info, prefixBuffer, &prefixBufferLen, &localStatus);
    }

    return handler->pad_and_justify(context, info, result, u_strlen(result));
}

static int32_t
u_printf_hex_handler(const u_printf_stream_handler  *handler,
                     void                           *context,
                     ULocaleBundle                  *formatBundle,
                     const u_printf_spec_info       *info,
                     const ufmt_args                *args)
{
    int64_t         num        = args[0].int64Value;
    UChar           result[UPRINTF_BUFFER_SIZE];
    int32_t         len        = UPRINTF_BUFFER_SIZE;


    /* mask off any necessary bits */
    if (info->fIsShort)
        num &= UINT16_MAX;
    else if (!info->fIsLongLong)
        num &= UINT32_MAX;

    /* format the number, preserving the minimum # of digits */
    ufmt_64tou(result, &len, num, 16,
        (UBool)(info->fSpec == 0x0078),
        (info->fPrecision == -1 && info->fZero) ? info->fWidth : info->fPrecision);

    /* convert to alt form, if desired */
    if(num != 0 && info->fAlt && len < UPRINTF_BUFFER_SIZE - 2) {
        /* shift the formatted string right by 2 chars */
        memmove(result + 2, result, len * sizeof(UChar));
        result[0] = 0x0030;
        result[1] = info->fSpec;
        len += 2;
    }

    return handler->pad_and_justify(context, info, result, len);
}

static int32_t
u_printf_octal_handler(const u_printf_stream_handler  *handler,
                       void                           *context,
                       ULocaleBundle                  *formatBundle,
                       const u_printf_spec_info       *info,
                       const ufmt_args                *args)
{
    int64_t         num        = args[0].int64Value;
    UChar           result[UPRINTF_BUFFER_SIZE];
    int32_t         len        = UPRINTF_BUFFER_SIZE;


    /* mask off any necessary bits */
    if (info->fIsShort)
        num &= UINT16_MAX;
    else if (!info->fIsLongLong)
        num &= UINT32_MAX;

    /* format the number, preserving the minimum # of digits */
    ufmt_64tou(result, &len, num, 8,
        FALSE, /* doesn't matter for octal */
        info->fPrecision == -1 && info->fZero ? info->fWidth : info->fPrecision);

    /* convert to alt form, if desired */
    if(info->fAlt && result[0] != 0x0030 && len < UPRINTF_BUFFER_SIZE - 1) {
        /* shift the formatted string right by 1 char */
        memmove(result + 1, result, len * sizeof(UChar));
        result[0] = 0x0030;
        len += 1;
    }

    return handler->pad_and_justify(context, info, result, len);
}

static int32_t
u_printf_uinteger_handler(const u_printf_stream_handler *handler,
                          void                          *context,
                          ULocaleBundle                 *formatBundle,
                          const u_printf_spec_info      *info,
                          const ufmt_args               *args)
{
    int64_t         num        = args[0].int64Value;
    UNumberFormat   *format;
    UChar           result[UPRINTF_BUFFER_SIZE];
    UChar           prefixBuffer[UPRINTF_BUFFER_SIZE];
    int32_t         prefixBufferLen = sizeof(prefixBuffer);
    int32_t         minDigits     = -1;
    UErrorCode      status        = U_ZERO_ERROR;

    prefixBuffer[0] = 0;

    /* TODO: Fix this once uint64_t can be formatted. */
    if (info->fIsShort)
        num &= UINT16_MAX;
    else if (!info->fIsLongLong)
        num &= UINT32_MAX;

    /* get the formatter */
    format = u_locbund_getNumberFormat(formatBundle, UNUM_DECIMAL);

    /* handle error */
    if(format == 0)
        return 0;

    /* set the appropriate flags on the formatter */

    /* set the minimum integer digits */
    if(info->fPrecision != -1) {
        /* set the minimum # of digits */
        minDigits = unum_getAttribute(format, UNUM_MIN_INTEGER_DIGITS);
        unum_setAttribute(format, UNUM_MIN_INTEGER_DIGITS, info->fPrecision);
    }

    /* set whether to show the sign */
    if(info->fShowSign) {
        u_printf_set_sign(format, info, prefixBuffer, &prefixBufferLen, &status);
    }

    /* format the number */
    unum_formatInt64(format, num, result, UPRINTF_BUFFER_SIZE, 0, &status);

    /* restore the number format */
    if (minDigits != -1) {
        unum_setAttribute(format, UNUM_MIN_INTEGER_DIGITS, minDigits);
    }

    if (info->fShowSign) {
        /* Reset back to original value regardless of what the error was */
        UErrorCode localStatus = U_ZERO_ERROR;
        u_printf_reset_sign(format, info, prefixBuffer, &prefixBufferLen, &localStatus);
    }

    return handler->pad_and_justify(context, info, result, u_strlen(result));
}

static int32_t
u_printf_pointer_handler(const u_printf_stream_handler  *handler,
                         void                           *context,
                         ULocaleBundle                  *formatBundle,
                         const u_printf_spec_info       *info,
                         const ufmt_args                *args)
{
    uint64_t        num = (uint64_t)args[0].ptrValue;
    UChar           result[UPRINTF_BUFFER_SIZE];
    int32_t         len        = UPRINTF_BUFFER_SIZE;


    if (sizeof(void*)==sizeof(int32_t)) {
        num &= UINT32_MAX;
    }

    /* format the pointer in hex */
    ufmt_64tou(result, &len, num, 16, TRUE, info->fPrecision);

    return handler->pad_and_justify(context, info, result, len);
}

static int32_t
u_printf_scientific_handler(const u_printf_stream_handler  *handler,
                            void                           *context,
                            ULocaleBundle                  *formatBundle,
                            const u_printf_spec_info       *info,
                            const ufmt_args                *args)
{
    double          num         = (double) (args[0].doubleValue);
    UNumberFormat   *format;
    UChar           result[UPRINTF_BUFFER_SIZE];
    UChar           prefixBuffer[UPRINTF_BUFFER_SIZE];
    int32_t         prefixBufferLen = sizeof(prefixBuffer);
    int32_t         minDecimalDigits;
    int32_t         maxDecimalDigits;
    UErrorCode      status        = U_ZERO_ERROR;
    UChar srcExpBuf[UPRINTF_SYMBOL_BUFFER_SIZE];
    int32_t srcLen, expLen;
    UChar expBuf[UPRINTF_SYMBOL_BUFFER_SIZE];

    prefixBuffer[0] = 0;

    /* mask off any necessary bits */
    /*  if(! info->fIsLongDouble)
    num &= DBL_MAX;*/

    /* get the formatter */
    format = u_locbund_getNumberFormat(formatBundle, UNUM_SCIENTIFIC);

    /* handle error */
    if(format == 0)
        return 0;

    /* set the appropriate flags on the formatter */

    srcLen = unum_getSymbol(format,
        UNUM_EXPONENTIAL_SYMBOL,
        srcExpBuf,
        sizeof(srcExpBuf),
        &status);

    /* Upper/lower case the e */
    if (info->fSpec == (UChar)0x65 /* e */) {
        expLen = u_strToLower(expBuf, (int32_t)sizeof(expBuf),
            srcExpBuf, srcLen,
            formatBundle->fLocale,
            &status);
    }
    else {
        expLen = u_strToUpper(expBuf, (int32_t)sizeof(expBuf),
            srcExpBuf, srcLen,
            formatBundle->fLocale,
            &status);
    }

    unum_setSymbol(format,
        UNUM_EXPONENTIAL_SYMBOL,
        expBuf,
        expLen,
        &status);

    /* save the formatter's state */
    minDecimalDigits = unum_getAttribute(format, UNUM_MIN_FRACTION_DIGITS);
    maxDecimalDigits = unum_getAttribute(format, UNUM_MAX_FRACTION_DIGITS);

    /* set the appropriate flags and number of decimal digits on the formatter */
    if(info->fPrecision != -1) {
        /* set the # of decimal digits */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, info->fPrecision);
    }
    else if(info->fPrecision == 0 && ! info->fAlt) {
        /* no decimal point in this case */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 0);
    }
    else if(info->fAlt) {
        /* '#' means always show decimal point */
        /* copy of printf behavior on Solaris - '#' shows 6 digits */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 6);
    }
    else {
        /* # of decimal digits is 6 if precision not specified */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 6);
    }

    /* set whether to show the sign */
    if (info->fShowSign) {
        u_printf_set_sign(format, info, prefixBuffer, &prefixBufferLen, &status);
    }

    /* format the number */
    unum_formatDouble(format, num, result, UPRINTF_BUFFER_SIZE, 0, &status);

    /* restore the number format */
    /* TODO: Is this needed? */
    unum_setAttribute(format, UNUM_MIN_FRACTION_DIGITS, minDecimalDigits);
    unum_setAttribute(format, UNUM_MAX_FRACTION_DIGITS, maxDecimalDigits);

    /* Since we're the only one using the scientific
       format, we don't need to save the old exponent value. */
    /*unum_setSymbol(format,
        UNUM_EXPONENTIAL_SYMBOL,
        srcExpBuf,
        srcLen,
        &status);*/

    if (info->fShowSign) {
        /* Reset back to original value regardless of what the error was */
        UErrorCode localStatus = U_ZERO_ERROR;
        u_printf_reset_sign(format, info, prefixBuffer, &prefixBufferLen, &localStatus);
    }

    return handler->pad_and_justify(context, info, result, u_strlen(result));
}

static int32_t
u_printf_percent_handler(const u_printf_stream_handler  *handler,
                         void                           *context,
                         ULocaleBundle                  *formatBundle,
                         const u_printf_spec_info       *info,
                         const ufmt_args                *args)
{
    double          num         = (double) (args[0].doubleValue);
    UNumberFormat   *format;
    UChar           result[UPRINTF_BUFFER_SIZE];
    UChar           prefixBuffer[UPRINTF_BUFFER_SIZE];
    int32_t         prefixBufferLen = sizeof(prefixBuffer);
    int32_t         minDecimalDigits;
    int32_t         maxDecimalDigits;
    UErrorCode      status        = U_ZERO_ERROR;

    prefixBuffer[0] = 0;

    /* mask off any necessary bits */
    /*  if(! info->fIsLongDouble)
    num &= DBL_MAX;*/

    /* get the formatter */
    format = u_locbund_getNumberFormat(formatBundle, UNUM_PERCENT);

    /* handle error */
    if(format == 0)
        return 0;

    /* save the formatter's state */
    minDecimalDigits = unum_getAttribute(format, UNUM_MIN_FRACTION_DIGITS);
    maxDecimalDigits = unum_getAttribute(format, UNUM_MAX_FRACTION_DIGITS);

    /* set the appropriate flags and number of decimal digits on the formatter */
    if(info->fPrecision != -1) {
        /* set the # of decimal digits */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, info->fPrecision);
    }
    else if(info->fPrecision == 0 && ! info->fAlt) {
        /* no decimal point in this case */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 0);
    }
    else if(info->fAlt) {
        /* '#' means always show decimal point */
        /* copy of printf behavior on Solaris - '#' shows 6 digits */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 6);
    }
    else {
        /* # of decimal digits is 6 if precision not specified */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 6);
    }

    /* set whether to show the sign */
    if (info->fShowSign) {
        u_printf_set_sign(format, info, prefixBuffer, &prefixBufferLen, &status);
    }

    /* format the number */
    unum_formatDouble(format, num, result, UPRINTF_BUFFER_SIZE, 0, &status);

    /* restore the number format */
    /* TODO: Is this needed? */
    unum_setAttribute(format, UNUM_MIN_FRACTION_DIGITS, minDecimalDigits);
    unum_setAttribute(format, UNUM_MAX_FRACTION_DIGITS, maxDecimalDigits);

    if (info->fShowSign) {
        /* Reset back to original value regardless of what the error was */
        UErrorCode localStatus = U_ZERO_ERROR;
        u_printf_reset_sign(format, info, prefixBuffer, &prefixBufferLen, &localStatus);
    }

    return handler->pad_and_justify(context, info, result, u_strlen(result));
}

static int32_t
u_printf_ustring_handler(const u_printf_stream_handler  *handler,
                         void                           *context,
                         ULocaleBundle                  *formatBundle,
                         const u_printf_spec_info       *info,
                         const ufmt_args                *args)
{
    int32_t len, written;
    const UChar *arg = (const UChar*)(args[0].ptrValue);

    /* allocate enough space for the buffer */
    if (arg == NULL) {
        arg = gNullStr;
    }
    len = u_strlen(arg);

    /* width = minimum # of characters to write */
    /* precision = maximum # of characters to write */

    /* precision takes precedence over width */
    /* determine if the string should be truncated */
    if(info->fPrecision != -1 && len > info->fPrecision) {
        written = handler->write(context, arg, info->fPrecision);
    }
    else {
        /* determine if the string should be padded */
        written = handler->pad_and_justify(context, info, arg, len);
    }

    return written;
}

static int32_t
u_printf_uchar_handler(const u_printf_stream_handler  *handler,
                       void                           *context,
                       ULocaleBundle                  *formatBundle,
                       const u_printf_spec_info       *info,
                       const ufmt_args                *args)
{
    int32_t written = 0;
    UChar arg = (UChar)(args[0].int64Value);


    /* width = minimum # of characters to write */
    /* precision = maximum # of characters to write */

    /* precision takes precedence over width */
    /* determine if the char should be printed */
    if(info->fPrecision != -1 && info->fPrecision < 1) {
        /* write nothing */
        written = 0;
    }
    else {
        /* determine if the string should be padded */
        written = handler->pad_and_justify(context, info, &arg, 1);
    }

    return written;
}

static int32_t
u_printf_scidbl_handler(const u_printf_stream_handler  *handler,
                        void                           *context,
                        ULocaleBundle                  *formatBundle,
                        const u_printf_spec_info       *info,
                        const ufmt_args                *args)
{
    u_printf_spec_info scidbl_info;
    double      num = args[0].doubleValue;

    memcpy(&scidbl_info, info, sizeof(u_printf_spec_info));

    /* determine whether to use 'd', 'e' or 'f' notation */
    if (scidbl_info.fPrecision == -1 && num == uprv_trunc(num))
    {
        /* use 'f' notation */
        scidbl_info.fSpec = 0x0066;
        scidbl_info.fPrecision = 0;
        /* call the double handler */
        return u_printf_double_handler(handler, context, formatBundle, &scidbl_info, args);
    }
    else if(num < 0.0001 || (scidbl_info.fPrecision < 1 && 1000000.0 <= num)
        || (scidbl_info.fPrecision != -1 && num > uprv_pow10(scidbl_info.fPrecision)))
    {
        /* use 'e' or 'E' notation */
        scidbl_info.fSpec = scidbl_info.fSpec - 2;
        if (scidbl_info.fPrecision == -1) {
            scidbl_info.fPrecision = 5;
        }
        /* call the scientific handler */
        return u_printf_scientific_handler(handler, context, formatBundle, &scidbl_info, args);
    }
    else {
        /* use 'f' notation */
        scidbl_info.fSpec = 0x0066;
        /* call the double handler */
        return u_printf_double_handler(handler, context, formatBundle, &scidbl_info, args);
    }
}

static int32_t
u_printf_count_handler(const u_printf_stream_handler  *handler,
                       void                           *context,
                       ULocaleBundle                  *formatBundle,
                       const u_printf_spec_info       *info,
                       const ufmt_args                *args)
{
    int32_t *count = (int32_t*)(args[0].ptrValue);

    /* in the special case of count, the u_printf_spec_info's width */
    /* will contain the # of chars written thus far */
    *count = info->fWidth;

    return 0;
}

static int32_t
u_printf_spellout_handler(const u_printf_stream_handler *handler,
                          void                          *context,
                          ULocaleBundle                 *formatBundle,
                          const u_printf_spec_info      *info,
                          const ufmt_args               *args)
{
    double          num         = (double) (args[0].doubleValue);
    UNumberFormat   *format;
    UChar           result[UPRINTF_BUFFER_SIZE];
    UChar           prefixBuffer[UPRINTF_BUFFER_SIZE];
    int32_t         prefixBufferLen = sizeof(prefixBuffer);
    int32_t         minDecimalDigits;
    int32_t         maxDecimalDigits;
    UErrorCode      status        = U_ZERO_ERROR;

    prefixBuffer[0] = 0;

    /* mask off any necessary bits */
    /*  if(! info->fIsLongDouble)
    num &= DBL_MAX;*/

    /* get the formatter */
    format = u_locbund_getNumberFormat(formatBundle, UNUM_SPELLOUT);

    /* handle error */
    if(format == 0)
        return 0;

    /* save the formatter's state */
    minDecimalDigits = unum_getAttribute(format, UNUM_MIN_FRACTION_DIGITS);
    maxDecimalDigits = unum_getAttribute(format, UNUM_MAX_FRACTION_DIGITS);

    /* set the appropriate flags and number of decimal digits on the formatter */
    if(info->fPrecision != -1) {
        /* set the # of decimal digits */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, info->fPrecision);
    }
    else if(info->fPrecision == 0 && ! info->fAlt) {
        /* no decimal point in this case */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 0);
    }
    else if(info->fAlt) {
        /* '#' means always show decimal point */
        /* copy of printf behavior on Solaris - '#' shows 6 digits */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 6);
    }
    else {
        /* # of decimal digits is 6 if precision not specified */
        unum_setAttribute(format, UNUM_FRACTION_DIGITS, 6);
    }

    /* set whether to show the sign */
    if (info->fShowSign) {
        u_printf_set_sign(format, info, prefixBuffer, &prefixBufferLen, &status);
    }

    /* format the number */
    unum_formatDouble(format, num, result, UPRINTF_BUFFER_SIZE, 0, &status);

    /* restore the number format */
    /* TODO: Is this needed? */
    unum_setAttribute(format, UNUM_MIN_FRACTION_DIGITS, minDecimalDigits);
    unum_setAttribute(format, UNUM_MAX_FRACTION_DIGITS, maxDecimalDigits);

    if (info->fShowSign) {
        /* Reset back to original value regardless of what the error was */
        UErrorCode localStatus = U_ZERO_ERROR;
        u_printf_reset_sign(format, info, prefixBuffer, &prefixBufferLen, &localStatus);
    }

    return handler->pad_and_justify(context, info, result, u_strlen(result));
}

/* Use US-ASCII characters only for formatting. Most codepages have
 characters 20-7F from Unicode. Using any other codepage specific
 characters will make it very difficult to format the string on
 non-Unicode machines */
static const u_printf_info g_u_printf_infos[UPRINTF_NUM_FMT_HANDLERS] = {
/* 0x20 */
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,
    UFMT_EMPTY,         UFMT_SIMPLE_PERCENT,UFMT_EMPTY,         UFMT_EMPTY,
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,

/* 0x30 */
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,

/* 0x40 */
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_UCHAR,
    UFMT_EMPTY,         UFMT_SCIENTIFIC,    UFMT_EMPTY,         UFMT_SCIDBL,
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_UCHAR/*deprecated*/,
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,

/* 0x50 */
    UFMT_PERCENT,       UFMT_EMPTY,         UFMT_EMPTY,         UFMT_USTRING,
    UFMT_EMPTY,         UFMT_USTRING/*deprecated*/,UFMT_SPELLOUT,      UFMT_EMPTY,
    UFMT_HEX,           UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,

/* 0x60 */
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_CHAR,
    UFMT_INT,           UFMT_SCIENTIFIC,    UFMT_DOUBLE,        UFMT_SCIDBL,
    UFMT_EMPTY,         UFMT_INT,           UFMT_EMPTY,         UFMT_EMPTY,
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_COUNT,         UFMT_OCTAL,

/* 0x70 */
    UFMT_POINTER,       UFMT_EMPTY,         UFMT_EMPTY,         UFMT_STRING,
    UFMT_EMPTY,         UFMT_UINT,          UFMT_EMPTY,         UFMT_EMPTY,
    UFMT_HEX,           UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,
    UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,         UFMT_EMPTY,
};

/* flag characters for uprintf */
#define FLAG_MINUS 0x002D
#define FLAG_PLUS 0x002B
#define FLAG_SPACE 0x0020
#define FLAG_POUND 0x0023
#define FLAG_ZERO  0x0030
#define FLAG_PAREN 0x0028

#define ISFLAG(s)    (s) == FLAG_MINUS || \
            (s) == FLAG_PLUS || \
            (s) == FLAG_SPACE || \
            (s) == FLAG_POUND || \
            (s) == FLAG_ZERO || \
            (s) == FLAG_PAREN

/* special characters for uprintf */
#define SPEC_ASTERISK 0x002A
#define SPEC_DOLLARSIGN 0x0024
#define SPEC_PERIOD 0x002E
#define SPEC_PERCENT 0x0025

/* unicode digits */
#define DIGIT_ZERO 0x0030
#define DIGIT_ONE 0x0031
#define DIGIT_TWO 0x0032
#define DIGIT_THREE 0x0033
#define DIGIT_FOUR 0x0034
#define DIGIT_FIVE 0x0035
#define DIGIT_SIX 0x0036
#define DIGIT_SEVEN 0x0037
#define DIGIT_EIGHT 0x0038
#define DIGIT_NINE 0x0039

#define ISDIGIT(s)    (s) == DIGIT_ZERO || \
            (s) == DIGIT_ONE || \
            (s) == DIGIT_TWO || \
            (s) == DIGIT_THREE || \
            (s) == DIGIT_FOUR || \
            (s) == DIGIT_FIVE || \
            (s) == DIGIT_SIX || \
            (s) == DIGIT_SEVEN || \
            (s) == DIGIT_EIGHT || \
            (s) == DIGIT_NINE

/* u_printf modifiers */
#define MOD_H 0x0068
#define MOD_LOWERL 0x006C
#define MOD_L 0x004C

#define ISMOD(s)    (s) == MOD_H || \
            (s) == MOD_LOWERL || \
            (s) == MOD_L

/* We parse the argument list in Unicode */
int32_t
u_printf_print_spec(const u_printf_stream_handler *streamHandler,
                    const UChar     *fmt,
                    void            *context,
                    ULocaleBundle   *formatBundle,
                    int32_t         patCount,
                    int32_t         *written,
                    va_list         *ap)
{
    uint16_t         handlerNum;
    ufmt_args        args;
    ufmt_type_info   argType;
    u_printf_handler *handler;
    u_printf_spec    spec;

    const UChar *s = fmt;
    const UChar *backup;
    u_printf_spec_info *info = &(spec.fInfo);

    /* initialize spec to default values */
    spec.fWidthPos     = -1;
    spec.fPrecisionPos = -1;
    spec.fArgPos       = -1;

    info->fPrecision    = -1;
    info->fWidth        = -1;
    info->fSpec         = 0x0000;
    info->fPadChar      = 0x0020;
    info->fAlt          = FALSE;
    info->fSpace        = FALSE;
    info->fLeft         = FALSE;
    info->fShowSign     = FALSE;
    info->fZero         = FALSE;
    info->fIsLongDouble = FALSE;
    info->fIsShort      = FALSE;
    info->fIsLong       = FALSE;
    info->fIsLongLong   = FALSE;

    /* skip over the initial '%' */
    s++;

    /* Check for positional argument */
    if(ISDIGIT(*s)) {

        /* Save the current position */
        backup = s;

        /* handle positional parameters */
        if(ISDIGIT(*s)) {
            spec.fArgPos = (int) (*s++ - DIGIT_ZERO);

            while(ISDIGIT(*s)) {
                spec.fArgPos *= 10;
                spec.fArgPos += (int) (*s++ - DIGIT_ZERO);
            }
        }

        /* if there is no '$', don't read anything */
        if(*s != SPEC_DOLLARSIGN) {
            spec.fArgPos = -1;
            s = backup;
        }
        /* munge the '$' */
        else
            s++;
    }

    /* Get any format flags */
    while(ISFLAG(*s)) {
        switch(*s++) {

            /* left justify */
        case FLAG_MINUS:
            info->fLeft = TRUE;
            break;

            /* always show sign */
        case FLAG_PLUS:
            info->fShowSign = TRUE;
            break;

            /* use space if no sign present */
        case FLAG_SPACE:
            info->fShowSign = TRUE;
            info->fSpace = TRUE;
            break;

            /* use alternate form */
        case FLAG_POUND:
            info->fAlt = TRUE;
            break;

            /* pad with leading zeroes */
        case FLAG_ZERO:
            info->fZero = TRUE;
            info->fPadChar = 0x0030;
            break;

            /* pad character specified */
        case FLAG_PAREN:

            /* first four characters are hex values for pad char */
            info->fPadChar = (UChar)ufmt_digitvalue(*s++);
            info->fPadChar = (UChar)((info->fPadChar * 16) + ufmt_digitvalue(*s++));
            info->fPadChar = (UChar)((info->fPadChar * 16) + ufmt_digitvalue(*s++));
            info->fPadChar = (UChar)((info->fPadChar * 16) + ufmt_digitvalue(*s++));

            /* final character is ignored */
            s++;

            break;
        }
    }

    /* Get the width */

    /* width is specified out of line */
    if(*s == SPEC_ASTERISK) {

        info->fWidth = -2;

        /* Skip the '*' */
        s++;

        /* Save the current position */
        backup = s;

        /* handle positional parameters */
        if(ISDIGIT(*s)) {
            spec.fWidthPos = (int) (*s++ - DIGIT_ZERO);

            while(ISDIGIT(*s)) {
                spec.fWidthPos *= 10;
                spec.fWidthPos += (int) (*s++ - DIGIT_ZERO);
            }
        }

        /* if there is no '$', don't read anything */
        if(*s != SPEC_DOLLARSIGN) {
            spec.fWidthPos = -1;
            s = backup;
        }
        /* munge the '$' */
        else
            s++;
    }
    /* read the width, if present */
    else if(ISDIGIT(*s)){
        info->fWidth = (int) (*s++ - DIGIT_ZERO);

        while(ISDIGIT(*s)) {
            info->fWidth *= 10;
            info->fWidth += (int) (*s++ - DIGIT_ZERO);
        }
    }

    /* Get the precision */

    if(*s == SPEC_PERIOD) {

        /* eat up the '.' */
        s++;

        /* precision is specified out of line */
        if(*s == SPEC_ASTERISK) {

            info->fPrecision = -2;

            /* Skip the '*' */
            s++;

            /* save the current position */
            backup = s;

            /* handle positional parameters */
            if(ISDIGIT(*s)) {
                spec.fPrecisionPos = (int) (*s++ - DIGIT_ZERO);

                while(ISDIGIT(*s)) {
                    spec.fPrecisionPos *= 10;
                    spec.fPrecisionPos += (int) (*s++ - DIGIT_ZERO);
                }

                /* if there is no '$', don't read anything */
                if(*s != SPEC_DOLLARSIGN) {
                    spec.fPrecisionPos = -1;
                    s = backup;
                }
                else {
                    /* munge the '$' */
                    s++;
                }
            }
        }
        /* read the precision */
        else if(ISDIGIT(*s)){
            info->fPrecision = (int) (*s++ - DIGIT_ZERO);

            while(ISDIGIT(*s)) {
                info->fPrecision *= 10;
                info->fPrecision += (int) (*s++ - DIGIT_ZERO);
            }
        }
    }

    /* Get any modifiers */
    if(ISMOD(*s)) {
        switch(*s++) {

            /* short */
        case MOD_H:
            info->fIsShort = TRUE;
            break;

            /* long or long long */
        case MOD_LOWERL:
            if(*s == MOD_LOWERL) {
                info->fIsLongLong = TRUE;
                /* skip over the next 'l' */
                s++;
            }
            else
                info->fIsLong = TRUE;
            break;

            /* long double */
        case MOD_L:
            info->fIsLongDouble = TRUE;
            break;
        }
    }

    /* finally, get the specifier letter */
    info->fSpec = *s++;

    /* fill in the precision and width, if specified out of line */

    /* width specified out of line */
    if(spec.fInfo.fWidth == -2) {
        if(spec.fWidthPos == -1) {
            /* read the width from the argument list */
            info->fWidth = va_arg(*ap, int32_t);
        }
        else {
            /* handle positional parameter */
        }

        /* if it's negative, take the absolute value and set left alignment */
        if(info->fWidth < 0) {
            info->fWidth     *= -1;
            info->fLeft     = TRUE;
        }
    }

    /* precision specified out of line */
    if(info->fPrecision == -2) {
        if(spec.fPrecisionPos == -1) {
            /* read the precision from the argument list */
            info->fPrecision = va_arg(*ap, int32_t);
        }
        else {
            /* handle positional parameter */
        }

        /* if it's negative, set it to zero */
        if(info->fPrecision < 0)
            info->fPrecision = 0;
    }

    handlerNum = (uint16_t)(info->fSpec - UPRINTF_BASE_FMT_HANDLERS);
    if (handlerNum < UPRINTF_NUM_FMT_HANDLERS) {
        /* query the info function for argument information */
        argType = g_u_printf_infos[ handlerNum ].info;
        if(argType > ufmt_simple_percent) {
            switch(argType) {
            case ufmt_count:
                /* set the spec's width to the # of chars written */
                info->fWidth = *written;
                /* fall through to set the pointer */
            case ufmt_string:
            case ufmt_ustring:
            case ufmt_pointer:
                args.ptrValue = va_arg(*ap, void*);
                break;
            case ufmt_char:
            case ufmt_uchar:
            case ufmt_int:
                if (info->fIsLongLong) {
                    args.int64Value = va_arg(*ap, int64_t);
                }
                else {
                    args.int64Value = va_arg(*ap, int);
                }
                break;
            case ufmt_float:
                args.floatValue = (float) va_arg(*ap, double);
                break;
            case ufmt_double:
                args.doubleValue = va_arg(*ap, double);
                break;
            default:
                break;  /* Should never get here */
            }
        }

        /* call the handler function */
        handler = g_u_printf_infos[ handlerNum ].handler;
        if(handler != 0) {
            *written += (*handler)(streamHandler, context, formatBundle, info, &args);
        }
        else {
            /* just echo unknown tags */
            *written += (streamHandler->write)(context, fmt, patCount);
        }
    }
    else {
        /* just echo unknown tags */
        *written += (streamHandler->write)(context, fmt, patCount);
    }
    /* return # of characters in this specifier */
    return (int32_t)(s - fmt);
}

#endif /* #if !UCONFIG_NO_FORMATTING */
