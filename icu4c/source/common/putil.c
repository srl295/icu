/*
*******************************************************************************
*
*   Copyright (C) 1997-1999, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*
*  FILE NAME : putil.c (previously putil.cpp and ptypes.cpp)
*
*   Date        Name        Description
*   04/14/97    aliu        Creation.
*   04/24/97    aliu        Added getDefaultDataDirectory() and
*                            getDefaultLocaleID().
*   04/28/97    aliu        Rewritten to assume Unix and apply general methods 
*                            for assumed case.  Non-UNIX platforms must be
*                            special-cased.  Rewrote numeric methods dealing
*                            with NaN and Infinity to be platform independent
*                             over all IEEE 754 platforms.
*   05/13/97    aliu        Restored sign of timezone 
*                            (semantics are hours West of GMT)
*   06/16/98    erm         Added IEEE_754 stuff, cleaned up isInfinite, isNan,
*                             nextDouble..
*   07/22/98    stephen     Added remainder, max, min, trunc
*   08/13/98    stephen     Added isNegativeInfinity, isPositiveInfinity
*   08/24/98    stephen     Added longBitsFromDouble
*   09/08/98    stephen     Minor changes for Mac Port
*   03/02/99    stephen     Removed openFile().  Added AS400 support.  
*                            Fixed EBCDIC tables
*   04/15/99    stephen     Converted to C.
*   06/28/99    stephen     Removed mutex locking in u_isBigEndian().
*   08/04/99    jeffrey R.  Added OS/2 changes
*   11/15/99    helena      Integrated S/390 IEEE support.
*******************************************************************************
*/

#ifdef _AIX
#    include<sys/types.h>
#endif

/* Define _XOPEN_SOURCE for Solaris and friends. */
#ifndef PTX
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

/* Define __USE_POSIX and __USE_XOPEN for Linux and glibc. */
#ifndef __USE_POSIX
#define __USE_POSIX
#endif
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#endif

/* Include standard headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <time.h>

/* include ICU headers */
#include "unicode/utypes.h"
#include "umutex.h"
#include "cmemory.h"
#include "cstring.h"
#include "filestrm.h"

/* include system headers */
#ifdef WIN32
#   include <wtypes.h>
#   include <winnls.h>
#   include "locmap.h"
#elif defined(OS2)
#   define INCL_DOSMISC
#   define INCL_DOSERRORS
#   define INCL_DOSMODULEMGR
#   include <os2.h>
#elif defined(OS400)
#   include <float.h>
#elif defined(XP_MAC)
#   include <Files.h>
#   include <IntlResources.h>
#   include <Script.h>
#   include <Folders.h>
#   include <Errors.h>
#   include <TextUtils.h>
#elif defined(AIX)
#   include <sys/ldr.h>
#elif defined(U_SOLARIS) || defined(U_LINUX)
#   include <dlfcn.h>
#   include <link.h>
#elif defined(HPUX)
#   include <dl.h>
#endif 

/* floating point implementations ------------------------------------------- */

/* We return QNAN rather than SNAN*/
#if IEEE_754
#define NAN_TOP ((int16_t)0x7FF8)
#define INF_TOP ((int16_t)0x7FF0)
#elif defined(OS390)
#define NAN_TOP ((int16_t)0x7F08)
#define INF_TOP ((int16_t)0x3F00)
#endif

#define SIGN 0x80000000L

/* statics */
static UBool fgNaNInitialized = FALSE;
static double fgNan;
static UBool fgInfInitialized = FALSE;
static double fgInf;

/* protos */
static char* u_topNBytesOfDouble(double* d, int n);
static char* u_bottomNBytesOfDouble(double* d, int n);


/*---------------------------------------------------------------------------
  Platform utilities
  Our general strategy is to assume we're on a POSIX platform.  Platforms which
  are non-POSIX must declare themselves so.  The default POSIX implementation
  will sometimes work for non-POSIX platforms as well (e.g., the NaN-related
  functions).
  ---------------------------------------------------------------------------*/

#if defined(_WIN32) || defined(XP_MAC) || defined(OS400) || defined(OS2)
#   undef U_POSIX_LOCALE
#else
#   define U_POSIX_LOCALE	1
#endif

/*
 * Only include langinfo.h if we have a way to get the codeset. If we later
 * depend on more feature, we can test on U_HAVE_NL_LANGINFO.
 *
 */

#if U_HAVE_NL_LANGINFO_CODESET
#include <langinfo.h>
#endif

/*---------------------------------------------------------------------------
  Universal Implementations
  These are designed to work on all platforms.  Try these, and if they don't
  work on your platform, then special case your platform with new
  implementations.
  ---------------------------------------------------------------------------*/

/* Get UTC (GMT) time measured in seconds since 0:00 on 1/1/70.*/
int32_t
uprv_getUTCtime()
{
#ifdef XP_MAC
    time_t t, t1, t2;
    struct tm tmrec;

    memset( &tmrec, 0, sizeof(tmrec) );
    tmrec.tm_year = 70;
    tmrec.tm_mon = 0;
    tmrec.tm_mday = 1;
    t1 = mktime(&tmrec);    /* seconds of 1/1/1970*/

    time(&t);
    memcpy( &tmrec, gmtime(&t), sizeof(tmrec) );
    t2 = mktime(&tmrec);    /* seconds of current GMT*/
    return t2 - t1;         /* GMT (or UTC) in seconds since 1970*/
#else
    time_t epochtime;
    time(&epochtime);
    return epochtime;
#endif
}

/*-----------------------------------------------------------------------------
  IEEE 754
  These methods detect and return NaN and infinity values for doubles
  conforming to IEEE 754.  Platforms which support this standard include X86,
  Mac 680x0, Mac PowerPC, AIX RS/6000, and most others.
  If this doesn't work on your platform, you have non-IEEE floating-point, and
  will need to code your own versions.  A naive implementation is to return 0.0
  for getNaN and getInfinity, and false for isNaN and isInfinite.
  ---------------------------------------------------------------------------*/

UBool 
uprv_isNaN(double number)
{
#if IEEE_754
    /* This should work in theory, but it doesn't, so we resort to the more*/
    /* complicated method below.*/
    /*  return number != number;*/

    /* You can't return number == getNaN() because, by definition, NaN != x for*/
    /* all x, including NaN (that is, NaN != NaN).  So instead, we compare*/
    /* against the known bit pattern.  We must be careful of endianism here.*/
    /* The pattern we are looking for id:*/

    /*   7FFy yyyy yyyy yyyy  (some y non-zero)*/

    /* There are two different kinds of NaN, but we ignore the distinction*/
    /* here.  Note that the y value must be non-zero; if it is zero, then we*/
    /* have infinity.*/

    uint32_t highBits = *(uint32_t*)u_topNBytesOfDouble(&number, 
                              sizeof(uint32_t));
    uint32_t lowBits  = *(uint32_t*)u_bottomNBytesOfDouble(&number, 
                             sizeof(uint32_t));

    return (UBool)(((highBits & 0x7FF00000L) == 0x7FF00000L) && 
      (((highBits & 0x000FFFFFL) != 0) || (lowBits != 0)));

#elif defined(OS390)
    uint32_t highBits = *(uint32_t*)u_topNBytesOfDouble(&number,
                        sizeof(uint32_t));
    uint32_t lowBits  = *(uint32_t*)u_bottomNBytesOfDouble(&number,
                        sizeof(uint32_t));

    return ((highBits & 0x7F080000L) == 0x7F080000L) &&
      (lowBits == 0x00000000L);

#else
    /* If your platform doesn't support IEEE 754 but *does* have an NaN value,*/
    /* you'll need to replace this default implementation with what's correct*/
    /* for your platform.*/
    return number != number;
#endif
}

UBool
uprv_isInfinite(double number)
{
#if IEEE_754
    /* We know the top bit is the sign bit, so we mask that off in a copy of */
    /* the number and compare against infinity. [LIU]*/
    /* The following approach doesn't work for some reason, so we go ahead and */
    /* scrutinize the pattern itself. */
    /*  double a = number; */
    /*  *(int8_t*)u_topNBytesOfDouble(&a, 1) &= 0x7F;*/
    /*  return a == uprv_getInfinity();*/
    /* Instead, We want to see either:*/

    /*   7FF0 0000 0000 0000*/
    /*   FFF0 0000 0000 0000*/
  
    uint32_t highBits = *(uint32_t*)u_topNBytesOfDouble(&number, 
                        sizeof(uint32_t));
    uint32_t lowBits  = *(uint32_t*)u_bottomNBytesOfDouble(&number, 
                        sizeof(uint32_t));

    return (UBool)(((highBits  & ~SIGN) == 0x7FF00000L) &&
      (lowBits == 0x00000000L));

#elif defined(OS390)
    uint32_t highBits = *(uint32_t*)u_topNBytesOfDouble(&number,
                        sizeof(uint32_t));
    uint32_t lowBits  = *(uint32_t*)u_bottomNBytesOfDouble(&number,
                        sizeof(uint32_t));

    return ((highBits  & ~SIGN) == 0x70FF0000L) && (lowBits == 0x00000000L);

#else
    /* If your platform doesn't support IEEE 754 but *does* have an infinity*/
    /* value, you'll need to replace this default implementation with what's*/
    /* correct for your platform.*/
    return number == (2.0 * number);
#endif
}

UBool   
uprv_isPositiveInfinity(double number)
{
#if IEEE_754 || defined(OS390)
    return (UBool)(number > 0 && uprv_isInfinite(number));
#else
    return uprv_isInfinite(number);
#endif
}

UBool   
uprv_isNegativeInfinity(double number)
{
#if IEEE_754 || defined(OS390)
    return (UBool)(number < 0 && uprv_isInfinite(number));

#else
    uint32_t highBits = *(uint32_t*)u_topNBytesOfDouble(&number,
                        sizeof(uint32_t));
    return((highBits & SIGN) && uprv_isInfinite(number));

#endif
}

double 
uprv_getNaN()
{
#if IEEE_754 || defined(OS390)
    if( !fgNaNInitialized) {
        umtx_lock(NULL);
        if( ! fgNaNInitialized) {
            int i;
            int8_t* p = (int8_t*)&fgNan;
            for(i = 0; i < sizeof(double); ++i) 
                *p++ = 0;
            *(int16_t*)u_topNBytesOfDouble(&fgNan, sizeof(NAN_TOP)) = NAN_TOP;
            fgNaNInitialized = TRUE;
        }
        umtx_unlock(NULL);
    }
    return fgNan;
#else
    /* If your platform doesn't support IEEE 754 but *does* have an NaN value,*/
    /* you'll need to replace this default implementation with what's correct*/
    /* for your platform.*/
    return 0.0;
#endif
}

double 
uprv_getInfinity()
{
#if IEEE_754 || defined(OS390)
    if (!fgInfInitialized)
    {
        int i;
        int8_t* p = (int8_t*)&fgInf;
        for(i = 0; i < sizeof(double); ++i) 
            *p++ = 0;
        *(int16_t*)u_topNBytesOfDouble(&fgInf, sizeof(INF_TOP)) = INF_TOP;
        fgInfInitialized = TRUE;
    }
    return fgInf;
#else
    /* If your platform doesn't support IEEE 754 but *does* have an infinity*/
    /* value, you'll need to replace this default implementation with what's*/
    /* correct for your platform.*/
    return 0.0;
#endif
}

double 
uprv_floor(double x)
{
    return floor(x);
}

double 
uprv_ceil(double x)
{
    return ceil(x);
}

double 
uprv_fabs(double x)
{
    return fabs(x);
}

double 
uprv_modf(double x, double* y)
{
    return modf(x, y);
}

double 
uprv_fmod(double x, double y)
{
    return fmod(x, y);
}

double
uprv_pow10(int32_t x)
{
    /* This is declared as "double pow(double x, double y)" */
    return pow(10.0, (double)x);
}

/**
 * Computes the remainder of an implied division of its operands, as
 * defined by the IEEE 754 standard.  Commonly used to bring a value
 * into range without losing accuracy; e.g., bringing a large argument
 * to sin() into range.
 *
 * Returns r, where x = n * p + r.  Here n is the integer nearest to
 * x / p.  If two integers are equidistant from x / p, n is the even
 * integer.  If r is zero, then it should have the same sign as the
 * dividend x.
 *
 * The IEEE remainder may be negative or positive.
 * IEEEremainder(5,3) = -1.  IEEEremainder(4,3) = 1.
 *
 * The IEEE remainder r is always less than or equal to p/2 in
 * absolute value.  That is, |r| <= |p/2|.  By comparison, fmod()
 * returns a remainder r such that |r| <= |p|.
 *
 * Some floating point processors can compute this value in hardware.
 * We provide two implementations here, one that manipulates the IEEE
 * bit pattern directly, and one that is built upon other floating
 * point operations.  The former implementation has superior accuracy
 * and is preferred; the latter may work on platforms where the former
 * fails, but will introduce inaccuracies.
 */
double 
uprv_IEEEremainder(double x, double p)
{
#if IEEE_754
    int32_t hx, hp;
    uint32_t sx, lx, lp;
    double p_half;

    hx = *(int32_t*)u_topNBytesOfDouble(&x, sizeof(int32_t));
    lx = *(uint32_t*)u_bottomNBytesOfDouble(&x, sizeof(uint32_t));

    hp = *(int32_t*)u_topNBytesOfDouble(&p, sizeof(int32_t));
    lp = *(uint32_t*)u_bottomNBytesOfDouble(&p, sizeof(uint32_t));

    sx = hx & SIGN;

    hp &= 0x7fffffff;
    hx &= 0x7fffffff;

    /* purge off exception values */
    if((hp|lp) == 0) 
        return (x*p) / (x*p);     /* p = 0 */
    if((hx >= 0x7ff00000)||        /* x not finite */
      ((hp>=0x7ff00000) &&    /* p is NaN */
      (((hp-0x7ff00000)|lp) != 0)))
        return (x*p) / (x*p);

    if(hp <= 0x7fdfffff) 
        x = uprv_fmod(x, p + p);    /* now x < 2p */
    if(((hx-hp)|(lx-lp)) == 0) 
        return 0.0 * x;
    x = uprv_fabs(x);
    p = uprv_fabs(p);
    if (hp < 0x00200000) {
        if(x + x > p) {
            x -= p;
            if(x + x >= p) 
                x -= p;
        }
    }
    else {
        p_half = 0.5 * p;
        if(x > p_half) {
            x -= p;
            if(x >= p_half) 
                x -= p;
        }
    }

    *(int32_t*)u_topNBytesOfDouble(&x, sizeof(int32_t)) ^= sx;

    return x;

#else
    /* INACCURATE but portable implementation of IEEEremainder.  This
     * implementation should work on platforms that do not have IEEE
     * bit layouts.  Deficiencies of this implementation are its
     * inaccuracy and that it does not attempt to handle NaN or
     * infinite parameters and it returns the dividend if the divisor
     * is zero.  This is probably not an issue on non-IEEE
     * platforms. - aliu
     */
    if (p != 0.0) { /* exclude zero divisor */
        double a = x / p;
        double aint = uprv_floor(a);
        double afrac = a - aint;
        if (afrac > 0.5) {
            aint += 1.0;
        } else if (!(afrac < 0.5)) { /* avoid == comparison */
            if (uprv_modf(aint / 2.0, &a) > 0.0) {
                aint += 1.0;
            }
        }
        x -= (p * aint);
    }
    return x;
#endif
}

double 
uprv_fmax(double x, double y)
{
#if IEEE_754
    int32_t lowBits;

    /* first handle NaN*/
    if(uprv_isNaN(x) || uprv_isNaN(y))
        return uprv_getNaN();

    /* check for -0 and 0*/
    lowBits = *(uint32_t*) u_bottomNBytesOfDouble(&x, sizeof(uint32_t));
    if(x == 0.0 && y == 0.0 && (lowBits & SIGN))
        return y; 

    return (x > y ? x : y);
#else

    /* this should work for all flt point w/o NaN and Infpecial cases */
    return (x > y ? x : y);
#endif
}

int32_t 
uprv_max(int32_t x, int32_t y)
{
    return (x > y ? x : y);
}

double 
uprv_fmin(double x, double y)
{
#if IEEE_754
    int32_t lowBits;

    /* first handle NaN*/
    if(uprv_isNaN(x) || uprv_isNaN(y))
        return uprv_getNaN();

    /* check for -0 and 0*/
    lowBits = *(uint32_t*) u_bottomNBytesOfDouble(&y, sizeof(uint32_t));
    if(x == 0.0 && y == 0.0 && (lowBits & SIGN))
        return y; 

    return (x > y ? y : x);
#else

    /* this should work for all flt point w/o NaN and Inf special cases */
    return (x > y ? y : x);
#endif
}

int32_t 
uprv_min(int32_t x, int32_t y)
{
    return (x > y ? y : x);
}

/**
 * Truncates the given double.
 * trunc(3.3) = 3.0, trunc (-3.3) = -3.0
 * This is different than calling floor() or ceil():
 * floor(3.3) = 3, floor(-3.3) = -4
 * ceil(3.3) = 4, ceil(-3.3) = -3
 */
double 
uprv_trunc(double d)
{
#if IEEE_754
    int32_t lowBits;

    /* handle error cases*/
    if(uprv_isNaN(d))
        return uprv_getNaN();
    if(uprv_isInfinite(d))
        return uprv_getInfinity();

    lowBits = *(uint32_t*) u_bottomNBytesOfDouble(&d, sizeof(uint32_t));
    if( (d == 0.0 && (lowBits & SIGN)) || d < 0)
        return ceil(d);
    else
        return floor(d);

#else
  return d >= 0 ? floor(d) : ceil(d);

#endif
}

void 
uprv_longBitsFromDouble(double d, int32_t *hi, uint32_t *lo)
{
    *hi = *(int32_t*)u_topNBytesOfDouble(&d, sizeof(int32_t));
    *lo = *(uint32_t*)u_bottomNBytesOfDouble(&d, sizeof(uint32_t));
}


/**
 * Return the floor of the log base 10 of a given double.
 * This method compensates for inaccuracies which arise naturally when
 * computing logs, and always give the correct value.  The parameter
 * must be positive and finite.
 * (Thanks to Alan Liu for supplying this function.)
 */
int16_t 
uprv_log10(double d)
{
    /* The reason this routine is needed is that simply taking the*/
    /* log and dividing by log10 yields a result which may be off*/
    /* by 1 due to rounding errors.  For example, the naive log10*/
    /* of 1.0e300 taken this way is 299, rather than 300.*/
    double alog10 = log(d) / log(10.0);
    int16_t ailog10 = (int16_t) floor(alog10);

    /* Positive logs could be too small, e.g. 0.99 instead of 1.0*/
    if (alog10 > 0 && d >= pow(10.0, ailog10 + 1))
        ++ailog10;

    /* Negative logs could be too big, e.g. -0.99 instead of -1.0*/
    else if (alog10 < 0 && d < pow(10.0, ailog10))
        --ailog10;

    return ailog10;
}

int32_t 
uprv_digitsAfterDecimal(double x)
{
    char buffer[20];
    int32_t numDigits;
    char *p;
    int32_t ptPos, exponent;

    /* negative numbers throw off the calculations*/
    x = fabs(x);

    /* cheat and use the string-format routine to get a string representation*/
    /* (it handles mathematical inaccuracy better than we can), then find out */
    /* many characters are to the right of the decimal point */
    sprintf(buffer, "%.9g", x);
    p = uprv_strchr(buffer, '.');
    if (p == 0)
        return 0;

    ptPos = (int16_t)(p - buffer);
    numDigits = (int16_t)(strlen(buffer) - ptPos - 1);

    /* if the number's string representation is in scientific notation, find */
    /* the exponent and take it into account*/
    exponent = 0;
    p = uprv_strchr(buffer, 'e');
    if (p != 0) {
        int16_t expPos = (int16_t)(p - buffer);
        numDigits -= strlen(buffer) - expPos;
        exponent = (int16_t)(atoi(p + 1));
    }

    /* the string representation may still have spurious decimal digits in it, */
    /* so we cut off at the ninth digit to the right of the decimal, and have */
    /* to search backward from there to the first non-zero digit*/
    if (numDigits > 9) {
        numDigits = 9;
        while (numDigits > 0 && buffer[ptPos + numDigits] == '0')
            --numDigits;
    }
    numDigits -= exponent;
    return numDigits;
}

/*---------------------------------------------------------------------------
  Platform-specific Implementations
  Try these, and if they don't work on your platform, then special case your
  platform with new implementations.
  ---------------------------------------------------------------------------*/

/* Time zone utilities */
void 
uprv_tzset()
{
#ifdef U_TZSET
    U_TZSET();
#else
    /* no initialization*/
#endif
}

int32_t 
uprv_timezone()
{
#ifdef U_TIMEZONE
    return U_TIMEZONE;
#else
    time_t t, t1, t2;
    struct tm tmrec;
    UBool dst_checked;
    int32_t tdiff = 0;

    time(&t);
    memcpy( &tmrec, localtime(&t), sizeof(tmrec) );
    dst_checked = (tmrec.tm_isdst != 0); /* daylight savings time is checked*/
    t1 = mktime(&tmrec);                 /* local time in seconds*/
    memcpy( &tmrec, gmtime(&t), sizeof(tmrec) );
    t2 = mktime(&tmrec);                 /* GMT (or UTC) in seconds*/
    tdiff = t2 - t1;
    /* imitate NT behaviour, which returns same timezone offset to GMT for 
       winter and summer*/
    if (dst_checked)
        tdiff += 3600;
    return tdiff;
#endif
}

char* 
uprv_tzname(int n)
{
#ifdef U_TZNAME
    return U_TZNAME[n];
#else
    return "";
#endif
}

/* Get and set the ICU data directory --------------------------------------- */

static UBool
gHaveDataDirectory=FALSE;

static char
gDataDirectory[1024];

/*
 * Here, we use a mutex to make sure that setting the data directory
 * is thread-safe; however, reading it after calling u_getDataDirectory()
 * may still occur while it is (re)set and is therefore not thread-safe.
 * The best is to not call it after the initialization.
 */
U_CAPI void U_EXPORT2
u_setDataDirectory(const char *directory) {
    if(directory!=NULL) {
        int length=uprv_strlen(directory);

        if(length<sizeof(gDataDirectory)-1) {
            umtx_lock(NULL);
            if(length==0) {
                *gDataDirectory=0;
            } else {
                uprv_memcpy(gDataDirectory, directory, length);

                /* terminate the directory with a separator (/ or \) */
                if(gDataDirectory[length-1]!=U_FILE_SEP_CHAR) {
                    gDataDirectory[length++]=U_FILE_SEP_CHAR;
                }

                /* zero-terminate it */
                gDataDirectory[length]=0;
            }
            gHaveDataDirectory=TRUE;
            umtx_unlock(NULL);
        }
    }
}


#ifndef ICU_DATA_DIR

# if defined(XP_MAC)
/*
  We need FSpGetFullPath() from Apple Sample Code's "MoreFiles". Look in:
    http://developer.apple.com/samplecode/Sample_Code/Files/MoreFiles.htm
  and replace the following function..
*/

 /* Dummy FSpGetFullPath */
pascal	OSErr	FSpGetFullPath(const FSSpec *spec,
							   short *fullPathLength,
							   Handle *fullPath)
{
    *fullPath = NULL;
    *fullPathLength = 0;
    return fnfErr; 
}
# endif /* XP_MAC */

/*
 * get the system drive or volume path
 * (Windows: e.g. "C:" or "D:")
 * do not terminate with a U_FILE_SEP_CHAR separator
 * return the length of the path, or 0 if none
 */
static int
getSystemPath(char *path, int size) {
#if defined(XP_MAC)
    int16_t volNum;
    path[0]=0;
    OSErr err=GetVol((unsigned char*)path, &volNum);
    if(err==noErr) {
        int length=(uint8_t)path[0];
        if(length>0) {
            /* convert the Pascal string to a C string */
            uprv_memmove(path, path+1, length);
            path[length]=0;
        }
        return length;
    }

#elif defined(WIN32)
    if(GetSystemDirectory(path, size)>=2 && path[1]==':') {
        /* remove the rest of the path - "\\winnt\\system32" or similar */
        path[2]=0;
        return 2;
    }

#elif defined(OS2)
    APIRET rc;
    ULONG bootDrive=0;  /* 1=A, 2=B, 3=C, ... */

    rc=DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, (PVOID)&bootDrive, sizeof(ULONG));
    if(rc==NO_ERROR) {
        /* convert the numeric boot drive to a string */
        path[0]='A'+bootDrive-1;
        path[1]=':';
        path[2]=0;
        return 2;
    }
#endif

    return 0;
}

#endif /* ICU_DATA_DIR not defined */

/*
 * get the path to the ICU dynamic library
 * do not terminate with a U_FILE_SEP_CHAR separator
 * return the length of the path, or 0 if none
 */
static int
getLibraryPath(char *path, int size) {
#ifdef WIN32
    HINSTANCE mod=GetModuleHandle("icuuc.dll");
    if(mod!=NULL) {
        if(GetModuleFileName(mod, path, size)>0) {
            /* remove the basename and the last file separator */
            char *lastSep=uprv_strrchr(path, U_FILE_SEP_CHAR);
            if(lastSep!=NULL) {
                *lastSep=0;
                return lastSep-path;
            }
        }
    }

#elif defined(OS2)
    HMODULE mod=NULLHANDLE;
    APIRET rc=DosQueryModuleHandle("icuuc.dll", &mod);
    if(rc==NO_ERROR) {
        rc=DosQueryModuleName(mod, (LONG)size, path);
        if(rc==NO_ERROR) {
            /* remove the basename and the last file separator */
            char *lastSep=uprv_strrchr(path, U_FILE_SEP_CHAR);
            if(lastSep!=NULL) {
                *lastSep=0;
                return lastSep-path;
            }
        }
    }

#elif defined(U_SOLARIS)
    void *handle=dlopen(U_COMMON_LIBNAME, RTLD_LAZY); /* "libicu-uc.so" */
    if(handle!=NULL) {
        Link_map *p=NULL;
        char *s;
        int rc, length=0;

        /* get the Link_map list */
        rc=dlinfo(handle, RTLD_DI_LINKMAP, (void *)&p);
        if(rc>=0) {
            /* search for the list item for the library itself */
            while(p!=NULL) {
       	       s=uprv_strstr(p->l_name, U_COMMON_LIBNAME); /* "libicu-uc.so" */
                if(s!=NULL) {
                    if(s>p->l_name) {
                        /* copy the path, without the basename and the last separator */
                        length=(s-p->l_name)-1;
                        if(0<length && length<size) {
                            uprv_memcpy(path, p->l_name, length);
                            path[length]=0;
                        } else {
                            length=0;
                        }
                    }
                    break;
                }
                p=p->l_next;
            }
        }
        dlclose(handle);
        return length;
    }

#elif defined(AIX)
    void *handle=(void*)load(U_COMMON_LIBNAME, L_LIBPATH_EXEC, "."); /* "libicu-uc.a" */
    if(handle!=NULL) {
        uint8_t buffer[4096];
        struct ld_info *p=NULL;
        char *s;
        int rc, length=0;

        /* copy the linked list of loaded libraries into the buffer */
        rc=loadquery(L_GETINFO, buffer, sizeof(buffer));
        if(rc>=0) {
            /* search for the list item for the library itself */
            p=(struct ld_info *)buffer;
            for(;;) {
                /* advance (ignore the first list item) */
                if(p->ldinfo_next==0) {
                    break;
                }
                p=(struct ld_info *)((uint8_t *)p+p->ldinfo_next);

                s=uprv_strstr(p->ldinfo_filename, U_COMMON_LIBNAME); /*  "libicuuc.a"    */
                if(s!=NULL) {
                    if(s>p->ldinfo_filename) {
                        /* copy the path, without the basename and the last separator */
                        length=(s-p->ldinfo_filename)-1;
                        if(0<length && length<size) {
                            uprv_memcpy(path, p->ldinfo_filename, length);
                            path[length]=0;
                        } else {
                            length=0;
                        }
                    }
                    break;
                }
                /* p=p->l_next; */
            }
        }
        unload(handle);
        return length;
    }

#elif defined(HPUX)
    {
        struct shl_descriptor *p=NULL;
        char *s;
        int i=1, rc, length=0;

        /* walk the list of shared libraries */
        /* search for the list item for the library itself */
        for(;;) {
            rc=shl_get(i, &p);
            if(rc<0) {
                break;
            }

            s=uprv_strstr(p->filename, U_COMMON_LIBNAME);
            if(s!=NULL) {
                if(s>p->filename) {
                    /* copy the path, without the basename and the last separator */
                    length=(s-p->filename)-1;
                    if(0<length && length<size) {
                        uprv_memcpy(path, p->filename, length);
                        path[length]=0;
                    } else {
                        length=0;
                    }
                }
                break;
            }
            ++i;
        }
        return length;
    }

#elif defined(OS390)
#elif defined(OS400)
#elif defined(XP_MAC)
#elif defined(U_LINUX)
#elif defined(TANDEM)
#elif defined(U_POSIX)
#endif

    return 0;
}

#ifdef WIN32
#   define LIB_PATH_VAR "PATH"
#   define LIB_FILENAME "icuuc.dll"
#elif defined(U_LINUX)
#   define LIB_PATH_VAR "LD_LIBRARY_PATH"
#   define LIB_FILENAME "libicuuc.so"
#elif defined(OS2)
#   define LIB_PATH_VAR "LIBPATH"
#   define LIB_FILENAME "icuuc.dll"
#elif defined(OS390)
#   define LIB_PATH_VAR "LIBPATH"
#   define LIB_FILENAME "libicuuc.a"
#elif defined(TANDEM)
#   define LIB_PATH_VAR "LIBPATH"
#   define LIB_FILENAME "libicuuc.a"
#elif defined(OS400)
#elif defined(XP_MAC)
#elif defined(U_SOLARIS)
#elif defined(AIX)
#elif defined(HPUX)
#elif defined(U_POSIX)
#   define LIB_PATH_VAR "LIBPATH"
#   define LIB_FILENAME "libicuuc.so"
#endif

/*
 * search for the ICU dynamic library and set the path
 * do not terminate with a U_FILE_SEP_CHAR separator
 * return the length of the path, or 0 if none
 */
static int
findLibraryPath(char *path, int size) {
    /* common implementation for searching the library path */
#ifdef LIB_FILENAME
    const char *libPath=getenv(LIB_PATH_VAR);

    if(libPath!=NULL) {
        /* loop over all paths */
        FileStream *f;
        const char *end;
        int length;

        for(;;) {
            /* find the end of the path */
            end=libPath;
            while(*end!=0 && *end!=U_PATH_SEP_CHAR) {
                ++end;
            }

            if(end!=libPath) {
                /* try this non-empty path */
                length=end-libPath;

                /* do not terminate the path */
                if(*(end-1)==U_FILE_SEP_CHAR) {
                    --length;
                }

                /* copy the path and add the library filename */
                uprv_memcpy(path, libPath, length);
                uprv_strcpy(path+length, U_FILE_SEP_STRING LIB_FILENAME);

                /* does this file exist in this path? */
                f=T_FileStream_open(path, "rb");
                if(f!=NULL) {
                    /* yes, clean up and return */
                    T_FileStream_close(f);
                    path[length]=0;
                    return length;
                }
            }

            if(*end==0) {
                break;  /* no more path */
            }

            /* *end==U_PATH_SEP_CHAR, go to the next path */
            libPath=end+1;
        }
    }
#endif
    return 0;
}

/* define a path for fallbacks */
#ifdef WIN32
#   define FALLBACK_PATH U_FILE_SEP_STRING ".." U_FILE_SEP_STRING "data"
#elif defined(XP_MAC)
#   define FALLBACK_PATH U_FILE_SEP_STRING "ICU" U_FILE_SEP_STRING U_ICU_VERSION U_FILE_SEP_STRING
#else
#   define FALLBACK_PATH U_FILE_SEP_STRING "share" U_FILE_SEP_STRING "icu" U_FILE_SEP_STRING U_ICU_VERSION U_FILE_SEP_STRING
#endif

/* #include <stdio.h> */
/* #include <unistd.h> */


U_CAPI const char * U_EXPORT2
u_getDataDirectory(void) {
    /* if we have the directory, then return it immediately */
    if(!gHaveDataDirectory) {
        /* we need to look for it */
        char pathBuffer[1024];
        const char *path = NULL;
        int length;

#       if !defined(XP_MAC)
            /* first try to get the environment variable */
            path=getenv("ICU_DATA");
/* 	    fprintf(stderr, " ******** ICU_DATA=%s ********** \n", path); */
/* 	    { */
/* 	      int i; */
/* 	      fprintf(stderr, "E=%08X\n", __environ); */
/* 	      if(__environ) */
/* 	      for(i=0;__environ[i] && __environ[i][0];i++) */
/* 		puts(__environ[i]); */
/* 	    } */
#else	/* XP_MAC */
		{
			OSErr myErr;
			short vRef;
			long  dir,newDir;
	        int16_t volNum;
			Str255 xpath;
			FSSpec spec;
			short  len;
			Handle full;

	        xpath[0]=0;
	       	
    	    myErr = GetVol(xpath, &volNum);
				
			if(myErr == noErr) {
				myErr = FindFolder(volNum, kApplicationSupportFolderType, TRUE, & vRef, &dir);
				newDir=-1;
				if (myErr == noErr) {
					myErr = 
						DirCreate(volNum,
								 dir,
								 "\pICU",
								 &newDir);	
					if( (myErr == noErr) || (myErr == dupFNErr) ) {
						spec.vRefNum = volNum;
						spec.parID = dir;
						uprv_memcpy(spec.name, "\pICU", 4);
						
						myErr = FSpGetFullPath(&spec, &len, &full);
						if(full != NULL)
						{
							HLock(full);
							uprv_memcpy(pathBuffer,  ((char*)(*full)), len);
							pathBuffer[len] = 0;
							path = pathBuffer;
							DisposeHandle(full);
						}	
					}
				}
			}
		}
#       endif
#       ifdef WIN32
            /* next, try to read the path from the registry */
            if(path==NULL || *path==0) {
                HKEY key;

                if(ERROR_SUCCESS==RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\ICU\\Unicode\\Data", 0, KEY_QUERY_VALUE, &key)) {
                    DWORD type=REG_EXPAND_SZ, size=sizeof(pathBuffer);

                    if(ERROR_SUCCESS==RegQueryValueEx(key, "Path", NULL, &type, (unsigned char *)pathBuffer, &size) && size>1) {
                        if(type==REG_EXPAND_SZ) {
                            /* replace environment variable references by their values */
                            char temporaryPath[1024];

                            /* copy the path with variables to the temporary one */
                            uprv_memcpy(temporaryPath, pathBuffer, size);

                            /* do the replacement and store it in the pathBuffer */
                            size=ExpandEnvironmentStrings(temporaryPath, pathBuffer, sizeof(pathBuffer));
                            if(size>0 && size<sizeof(pathBuffer)) {
                                path=pathBuffer;
                            }
                        } else if(type==REG_SZ) {
                            path=pathBuffer;
                        }
                    }
                    RegCloseKey(key);
                }
            }
#       endif

        /* next, try to get the path to the ICU dynamic library */
        if(path==NULL || *path==0) {
            length=getLibraryPath(pathBuffer, sizeof(pathBuffer));
            if(length>0) {
                uprv_strcpy(pathBuffer+length, U_FILE_SEP_STRING ".." FALLBACK_PATH);
                path=pathBuffer;
            }
        }

        /* next, search for the ICU dynamic library */
        if(path==NULL || *path==0) {
            length=findLibraryPath(pathBuffer, sizeof(pathBuffer));
            if(length>0) {
                uprv_strcpy(pathBuffer+length, U_FILE_SEP_STRING ".." FALLBACK_PATH);
                path=pathBuffer;
            }
        }

        /* last resort: use hardcoded path */
        if(path==NULL || *path==0) {
            /* ICU_DATA_DIR may be set as a compile option */
#           ifdef ICU_DATA_DIR
                path=ICU_DATA_DIR;
#           else
                length=getSystemPath(pathBuffer, sizeof(pathBuffer));
                if(length>0) {
                    uprv_strcpy(pathBuffer+length, FALLBACK_PATH);
                    path=pathBuffer;
                } else {
                    path=FALLBACK_PATH;
                }
#           endif
        }

        u_setDataDirectory(path);
    }

    /* we did set the directory if necessary */
    return gDataDirectory;
}

/* Macintosh-specific locale information ------------------------------------ */
#ifdef XP_MAC

struct mac_lc_rec {
  int32_t script;
  int32_t region;
  int32_t lang;
  int32_t date_region;
  char* posixID;
};
/* To do: This will be updated with a newer version from www.unicode.org web
   page when it's available.*/
#define MAC_LC_MAGIC_NUMBER -5
#define MAC_LC_INIT_NUMBER -9

mac_lc_rec mac_lc_recs[] = {
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 0, "en_US",  
  /* United States*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 1, "fr_FR",  
  /* France*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 2, "en_GB",  
  /* Great Britain*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 3, "de_DE",  
  /* Germany*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 4, "it_IT",  
  /* Italy*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 5, "nl_NL", 
  /* Metherlands*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 6, "fr_BE", 
  /* French for Belgium or Lxembourg*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 7, "sv_SE", 
  /* Sweden*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 9, "da_DK", 
  /* Denmark*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 10, "pt_PT", 
  /* Portugal*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 11, "fr_CA", 
  /* French Canada*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 13, "is_IS", 
  /* Israel*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 14, "ja_JP", 
  /* Japan*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 15, "en_AU", 
  /* Australia*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 16, "ar_AE", 
  /* the Arabic world (?)*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 17, "fi_FI", 
  /* Finland*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 18, "fr_CH", 
  /* French for Switzerland*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 19, "de_CH", 
  /* German for Switzerland*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 20, "EL_GR", 
  /* Greece*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 21, "is_IS", 
  /* Iceland ===*/
  /*MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 22, "", 
    // Malta ===*/
  /*MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 23, "", 
    // Cyprus ===*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 24, "tr_TR", 
  /* Turkey ===*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 25, "sh_YU", 
  /* Croatian system for Yugoslavia*/
  /*MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 33, "", 
    // Hindi system for India*/
  /*MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 34, "", 
    // Pakistan*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 41, "lt_LT", 
  /* Lithuania*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 42, "pl_PL", 
  /* Poland*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 43, "hu_HU", 
  /* Hungary*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 44, "et_EE", 
  /* Estonia*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 45, "lv_LV", 
  /* Latvia*/
  /*MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 46, "", 
    // Lapland  [Ask Rich for the data. HS]*/
  /*MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 47, "", 
    // Faeroe Islands*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 48, "fa_IR", 
  /* Iran*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 49, "ru_RU", 
  /* Russia*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 50, "en_IE", 
  /* Ireland*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 51, "ko_KR", 
  /* Korea*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 52, "zh_CN", 
  /* People's Republic of China*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 53, "zh_TW", 
  /* Taiwan*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 54, "th_TH", 
  /* Thailand*/
  
  /* fallback is en_US*/
  MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, MAC_LC_MAGIC_NUMBER, 
  MAC_LC_MAGIC_NUMBER, "en_US"
};

#endif

#if U_POSIX_LOCALE
/* Return just the POSIX id, whatever happens to be in it */
static const char *uprv_getPOSIXID()
{
  const char* posixID = getenv("LC_ALL");
  if (posixID == 0)
    posixID = getenv("LANG");
  if (posixID == 0)
    posixID = setlocale(LC_ALL, NULL);

  if ( (posixID==0) || 
       (uprv_strcmp("C", posixID) == 0) ||
       (uprv_strncmp("C ", posixID, 2) == 0) )
  {  /* HPUX returns 'C C C C C C C' */
    posixID = "en_US";
  }
  return posixID;
}
#endif

const char* 
uprv_getDefaultLocaleID()
{
#if U_POSIX_LOCALE
  char *correctedPOSIXLocale = 0;
  const char* posixID = uprv_getPOSIXID();
  char *p;

  /* Format: (no spaces)
     ll [ _CC ] [ . MM ] [ @ VV]

     l = lang, C = ctry, M = charmap, V = variant
  */

  if((p = uprv_strchr(posixID, '.')) != NULL)
  {
    /* assume new locale can't be larger than old one? */
    correctedPOSIXLocale = uprv_malloc(uprv_strlen(posixID));
    uprv_strncpy(correctedPOSIXLocale, posixID, p-posixID);
    correctedPOSIXLocale[p-posixID] = 0;

    posixID = correctedPOSIXLocale;
  }
  
  if((p = uprv_strchr(posixID, '@')) != NULL)
  {
    if(correctedPOSIXLocale == NULL) {
      correctedPOSIXLocale = uprv_malloc(uprv_strlen(posixID));
      uprv_strncpy(correctedPOSIXLocale, posixID, p-posixID);
      correctedPOSIXLocale[p-posixID] = 0;
    }
    p++;

    /* Take care of any special cases here.. */
    if(!uprv_strcmp(p, "nynorsk"))
      {
        p = "NY";
        
        /*      Should we assume no_NO_NY instead of possible no__NY?
         * if(!uprv_strcmp(correctedPOSIXLocale, "no")) {
         *         uprv_strcpy(correctedPOSIXLocale, "no_NO");
         *         }
         */
      }

    if(uprv_strchr(correctedPOSIXLocale,'_') == NULL)
      uprv_strcat(correctedPOSIXLocale, "__"); /* aa@b -> aa__b */
    else
      uprv_strcat(correctedPOSIXLocale, "_"); /* aa_CC@b -> aa_CC_b */
    uprv_strcat(correctedPOSIXLocale, p);

    /* Should there be a map from 'no@nynorsk' -> no_NO_NY here?
       How about 'russian' -> 'ru'?
    */

    posixID = correctedPOSIXLocale;
  }
  
  return posixID;

#elif defined(WIN32)
    UErrorCode status = U_ZERO_ERROR;
    LCID id = GetThreadLocale();
    const char* locID = T_convertToPosix(id, &status);

    if (U_FAILURE(status)) {
        locID = "en_US";
    }
    return locID;

#elif defined(XP_MAC)
  int32_t script = MAC_LC_INIT_NUMBER; 
  /* = IntlScript(); or GetScriptManagerVariable(smSysScript);*/
  int32_t region = MAC_LC_INIT_NUMBER; 
  /* = GetScriptManagerVariable(smRegionCode);*/
  int32_t lang = MAC_LC_INIT_NUMBER;   
  /* = GetScriptManagerVariable(smScriptLang);*/
  int32_t date_region = MAC_LC_INIT_NUMBER;
  char* posixID = 0;
  Intl1Hndl ih;
  
  ih = (Intl1Hndl) GetIntlResource(1);
  if (ih)
    date_region = ((uint16_t)(*ih)->intl1Vers) >> 8;
  
  int32_t count = sizeof(mac_lc_recs) / sizeof(mac_lc_rec);
  for (int32_t i = 0; i < count; i++) {
    if ( ((mac_lc_recs[i].script == MAC_LC_MAGIC_NUMBER)      
      || (mac_lc_recs[i].script == script))
     && ((mac_lc_recs[i].region == MAC_LC_MAGIC_NUMBER)
         || (mac_lc_recs[i].region == region))
     && ((mac_lc_recs[i].lang == MAC_LC_MAGIC_NUMBER)
         || (mac_lc_recs[i].lang == lang))
     && ((mac_lc_recs[i].date_region == MAC_LC_MAGIC_NUMBER) 
         || (mac_lc_recs[i].date_region == date_region))
     ) {
      posixID = mac_lc_recs[i].posixID;
      break;
    }
  }
  
  return posixID;

#elif defined(OS2)
    char * locID;

    locID = getenv("LC_ALL");
    if (!locID || !*locID)
        locID = getenv("LANG");
    if (!locID || !*locID) {
        locID = "C";
    }
    if (!stricmp(locID, "c") || !stricmp(locID, "posix") ||
        !stricmp(locID, "univ"))
        locID = "en_US";
    return locID;

#elif defined(OS400)
    /* Todo: TBD needs to be implemented */
    return "";

#endif

}

/* end of platform-specific implementation */

double 
uprv_nextDouble(double d, UBool next)
{
#if IEEE_754
  int32_t highBits;
  uint32_t lowBits;
  int32_t highMagnitude;
  uint32_t lowMagnitude;
  double result;
  uint32_t *highResult, *lowResult;
  uint32_t signBit;

  /* filter out NaN's */
  if (uprv_isNaN(d)) {
    return d;
  }
  
  /* zero's are also a special case */
  if (d == 0.0) {
    double smallestPositiveDouble = 0.0;
    uint32_t *plowBits = 
      (uint32_t *)u_bottomNBytesOfDouble(&smallestPositiveDouble, 
                     sizeof(uint32_t));
    
    *plowBits = 1;
    
    if (next) {
      return smallestPositiveDouble;
    } else {
      return -smallestPositiveDouble;
    }
  }
  
  /* if we get here, d is a nonzero value */
  
  /* hold all bits for later use */
  highBits = *(int32_t*)u_topNBytesOfDouble(&d, sizeof(uint32_t));
  lowBits = *(uint32_t*)u_bottomNBytesOfDouble(&d, sizeof(uint32_t));
  
  /* strip off the sign bit */
  highMagnitude = highBits & ~SIGN;
  lowMagnitude = lowBits;
  
  /* if next double away from zero, increase magnitude */
  if ((highBits >= 0) == next) {
    if (highMagnitude != 0x7FF00000L || lowMagnitude != 0x00000000L) {
      lowMagnitude += 1;
      if (lowMagnitude == 0) {
    highMagnitude += 1;
      }
    }
  }
  /* else decrease magnitude */
  else {
    lowMagnitude -= 1;
    if (lowMagnitude > lowBits) {
      highMagnitude -= 1;
    }
  }
  
  /* construct result and return */
  signBit = highBits & SIGN;
  highResult = (uint32_t *)u_topNBytesOfDouble(&result, sizeof(uint32_t));
  lowResult  = (uint32_t *)u_bottomNBytesOfDouble(&result, sizeof(uint32_t));
  
  *highResult = signBit | highMagnitude;
  *lowResult  = lowMagnitude;
  return result;
#else

  /* This is the portable implementation...*/
  /* a small coefficient within the precision of the mantissa*/
  static const double smallValue = 1e-10;  
  double epsilon = ((d<0)?-d:d) * smallValue; /* first approximation*/
  double last_eps, sum;

  if (epsilon == 0)
    epsilon = smallValue; /* for very small d's*/
  if (!next)
    epsilon = -epsilon;
  /* avoid higher precision possibly used for temporay values*/

  last_eps = epsilon * 2.0;
  sum = d + epsilon;

  while ((sum != d) && (epsilon != last_eps)) {
    last_eps = epsilon;
    epsilon /= 2.0;
    sum = d + epsilon;
  }
  return d + last_eps;
#endif
}

static char*
u_topNBytesOfDouble(double* d, int n)
{
  return U_IS_BIG_ENDIAN ? (char*)d : (char*)(d + 1) - n;
}

static char* u_bottomNBytesOfDouble(double* d, int n)
{
  return U_IS_BIG_ENDIAN ? (char*)(d + 1) - n : (char*)d;
}

U_CAPI const char *
uprv_defaultCodePageForLocale(const char *locale);

const char* uprv_getDefaultCodepage()
{
#if defined(OS400)
  return "ibm-37";

#elif defined(OS390)
  return "ibm-1047-s390";

#elif defined(XP_MAC)
  return "ibm-1275"; /* Macintosh Roman. There must be a better way. fixme! */

#elif defined(WIN32)
  static char tempString[10] = "";
  static char codepage[12]={ "cp" };
  uprv_strcpy(codepage+2, _itoa(GetACP(), tempString, 10));
  return codepage;

#elif U_POSIX_LOCALE
    static char codesetName[100];
    char *name = NULL;
    char *euro = NULL;
    const char *localeName = NULL;
    const char *defaultTable = NULL;
 
    uprv_memset(codesetName, 0, 100);
    localeName = uprv_getPOSIXID();
    if (localeName != NULL) 
    {
        uprv_strcpy(codesetName, localeName);
        if  ((name = (uprv_strchr(codesetName, (int) '.'))) != NULL) 
        {
            /* strip the locale name and look at the suffix only */
            name++;
            if ((euro  = (uprv_strchr(name, (int)'@'))) != NULL)
            {
               *euro  = 0;
            }
            /* if we can find the codset name from setlocale, return that. */
            if (uprv_strlen(name) != 0) 
            {
                return name;
            }
        } 
    }

    /* otherwise, try CTYPE */

    uprv_memset(codesetName, 0, 100);
    localeName = setlocale(LC_CTYPE, "");
    if (localeName != NULL) 
    {
        uprv_strcpy(codesetName, localeName);
        if  ((name = (uprv_strchr(codesetName, (int) '.'))) != NULL) 
        {
            /* strip the locale name and look at the suffix only */
            name++;
            if ((euro  = (uprv_strchr(name, (int)'@'))) != NULL)
            {
               *euro  = 0;
            }
            /* if we can find the codset name from setlocale, return that. */
            if (uprv_strlen(name) != 0) 
            {
                return name;
            }
        } 
    }
    if (strlen(codesetName) != 0) 
    {
        uprv_memset(codesetName, 0, 100);
    }
#if U_HAVE_NL_LANGINFO_CODESET
    /**/ {
        const char *codeset = nl_langinfo(U_NL_LANGINFO_CODESET);
        if (codeset != NULL) {
            uprv_strcpy(codesetName, codeset);
        }
    }
#endif
    if (uprv_strlen(codesetName) == 0) 
    {
        /* look up in srl's table */
        defaultTable = uprv_defaultCodePageForLocale(localeName);
        if (defaultTable != NULL)
        {
            uprv_strcpy(codesetName, defaultTable);
        } 
        else
        {
            /* if the table lookup failed, return latin1. */
            uprv_strcpy(codesetName, "LATIN_1");
        }
    }
    return codesetName;
#else
  return "LATIN_1";
#endif
}

#if U_CHARSET_FAMILY==U_EBCDIC_FAMILY
#ifdef OS390
/*
 * These maps for ASCII to/from EBCDIC are from
 * "UTF-EBCDIC - EBCDIC-Friendly Unicode (or UCS) Transformation Format"
 * at http://www.unicode.org/unicode/reports/tr16/
 * (which should reflect codepage 1047)
 * but modified to explicitly exclude the variant
 * control and graphical characters that are in ASCII-based
 * codepages at 0x80 and above.
 * Also, unlike in Version 6.0 of the UTR on UTF-EBCDIC,
 * the Line Feed mapping varies according to the environment.
 *
 * These tables do not establish a converter or a codepage.
 */

    /* on S/390 Open Edition, ASCII 0xa (LF) maps to 0x15 and ISO-8 0x85 maps to 0x25 */
#   define E_LF 0x15
#   define A_15 0x0a
#   define A_25 0x00

#   if 0
        /* the CDRA variation of 1047 is not currently used - see tables in #else below */
        /* in standard EBCDIC (CDRA), ASCII 0xa (LF) maps to 0x25 and ISO-8 0x85 maps to 0x15 */
#       define E_LF 0x25
#       define A_15 0x00
#       define A_25 0x0a
#   endif

static uint8_t asciiFromEbcdic[256]={
    0x00, 0x01, 0x02, 0x03, 0x00, 0x09, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x00, A_15, 0x08, 0x00, 0x18, 0x19, 0x00, 0x00, 0x1C, 0x1D, 0x1E, 0x1F,
    0x00, 0x00, 0x00, 0x00, 0x00, A_25, 0x17, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06, 0x07,
    0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x14, 0x15, 0x00, 0x1A,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
    0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
    0x2D, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
    0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x00, 0x00, 0x00, 0x5B, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5D, 0x00, 0x00,
    0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5C, 0x00, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t ebcdicFromAscii[256]={
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, E_LF, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
    0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
    0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
    0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5F, 0x6D,
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#else
/*
 * These maps for ASCII to/from EBCDIC were generated
 * using the ICU converter for codepage 37 on 2000-may-22.
 * They explicitly exclude the variant
 * control and graphical characters that are in ASCII-based
 * codepages at 0x80 and above.
 *
 * These tables do not establish a converter or a codepage.
 */

static uint8_t asciiFromEbcdic[256]={
    0x00, 0x01, 0x02, 0x03, 0x00, 0x09, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x00, 0x00, 0x08, 0x00, 0x18, 0x19, 0x00, 0x00, 0x1c, 0x1d, 0x1e, 0x1f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x17, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06, 0x07,
    0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x14, 0x15, 0x00, 0x1a,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x3c, 0x28, 0x2b, 0x7c,
    0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0x00,
    0x2d, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x25, 0x5f, 0x3e, 0x3f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22,
    0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5b, 0x5d, 0x00, 0x00, 0x00, 0x00,
    0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5c, 0x00, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t ebcdicFromAscii[256]={
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f, 0x16, 0x05, 0x25, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26, 0x18, 0x19, 0x3f, 0x27, 0x1c, 0x1d, 0x1e, 0x1f,
    0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d, 0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f,
    0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xba, 0xe0, 0xbb, 0xb0, 0x6d,
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0x07,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#endif

#endif

U_CAPI void U_EXPORT2
u_charsToUChars(const char *cs, UChar *us, UTextOffset length) {
    while(length>0) {
#if U_CHARSET_FAMILY==U_ASCII_FAMILY
        *us++=(UChar)(uint8_t)(*cs++);
#elif U_CHARSET_FAMILY==U_EBCDIC_FAMILY
        *us++=(UChar)asciiFromEbcdic[(uint8_t)(*cs++)];
#else
#   error U_CHARSET_FAMILY is not valid
#endif
        --length;
    }
}

U_CAPI void U_EXPORT2
u_UCharsToChars(const UChar *us, char *cs, UTextOffset length) {
    while(length>0) {
#if U_CHARSET_FAMILY==U_ASCII_FAMILY
        *cs++=(char)(*us++);
#elif U_CHARSET_FAMILY==U_EBCDIC_FAMILY
        *cs++=(char)ebcdicFromAscii[(uint8_t)(*us++)];
#else
#   error U_CHARSET_FAMILY is not valid
#endif
        --length;
    }
}

U_CFUNC void
u_versionFromString(UVersionInfo versionArray, const char *versionString) {
    char *end;
    uint16_t part=0;

    if(versionArray==NULL) {
        return;
    }

    if(versionString!=NULL) {
        for(;;) {
            versionArray[part]=(uint8_t)uprv_strtoul(versionString, &end, 10);
            if(end==versionString || ++part==U_MAX_VERSION_LENGTH || *end!=U_VERSION_DELIMITER) {
                break;
            }
            versionString=end+1;
        }
    }

    while(part<U_MAX_VERSION_LENGTH) {
        versionArray[part++]=0;
    }
}

U_CAPI void U_EXPORT2
u_versionToString(UVersionInfo versionArray, char *versionString) {
    uint16_t count, part;
    uint8_t field;

    if(versionString==NULL) {
        return;
    }

    if(versionArray==NULL) {
        versionString[0]=0;
        return;
    }

    /* count how many fields need to be written */
    for(count=4; count>0 && versionArray[count-1]==0; --count) {}

    if(count>0) {
        /* write the first part */
        /* write the decimal field value */
        field=versionArray[0];
        if(field>=100) {
            *versionString++=(char)('0'+field/100);
            field%=100;
        }
        if(field>=10) {
            *versionString++=(char)('0'+field/10);
            field%=10;
        }
        *versionString++=(char)('0'+field);

        /* write the following parts */
        for(part=1; part<count; ++part) {
            /* write a dot first */
            *versionString++=U_VERSION_DELIMITER;

            /* write the decimal field value */
            field=versionArray[part];
            if(field>=100) {
                *versionString++=(char)('0'+field/100);
                field%=100;
            }
            if(field>=10) {
                *versionString++=(char)('0'+field/10);
                field%=10;
            }
            *versionString++=(char)('0'+field);
        }
    }

    /* NUL-terminate */
    *versionString=0;
}

U_CAPI void U_EXPORT2
u_getVersion(UVersionInfo versionArray) {
    u_versionFromString(versionArray, U_ICU_VERSION);
}

/* u_errorName() ------------------------------------------------------------ */

static const char *
_uErrorInfoName[U_ERROR_INFO_LIMIT-U_ERROR_INFO_START]={
    "U_USING_FALLBACK_ERROR",
    "U_USING_DEFAULT_ERROR"
};

static const char *
_uErrorName[U_ERROR_LIMIT]={
    "U_ZERO_ERROR",

    "U_ILLEGAL_ARGUMENT_ERROR",
    "U_MISSING_RESOURCE_ERROR",
    "U_INVALID_FORMAT_ERROR",
    "U_FILE_ACCESS_ERROR",
    "U_INTERNAL_PROGRAM_ERROR",
    "U_MESSAGE_PARSE_ERROR",
    "U_MEMORY_ALLOCATION_ERROR",
    "U_INDEX_OUTOFBOUNDS_ERROR",
    "U_PARSE_ERROR",
    "U_INVALID_CHAR_FOUND",
    "U_TRUNCATED_CHAR_FOUND",
    "U_ILLEGAL_CHAR_FOUND",
    "U_INVALID_TABLE_FORMAT",
    "U_INVALID_TABLE_FILE",
    "U_BUFFER_OVERFLOW_ERROR",
    "U_UNSUPPORTED_ERROR",
    "U_RESOURCE_TYPE_MISMATCH",
    "U_ILLEGAL_ESCAPE_SEQUENCE", 
    "U_UNSUPPORTED_ESCAPE_SEQUENCE"
};

U_CAPI const char * U_EXPORT2
u_errorName(UErrorCode code) {
    if(code>=0 && code<U_ERROR_LIMIT) {
        return _uErrorName[code];
    } else if(code>=U_ERROR_INFO_START && code<U_ERROR_INFO_LIMIT) {
        return _uErrorInfoName[code-U_ERROR_INFO_START];
    } else {
        return "[BOGUS UErrorCode]";
    }
}

struct
{
  char loc[20];
  char charmap[40];
} 
_localeToDefaultCharmapTable [] =
{
/*
  See:         http://czyborra.com/charsets/iso8859.html
*/

/* xx_XX locales first, so they will match: */
 { "zh_CN", "gb2312" },  /* Chinese (Simplified) */
 { "zh_TW", "Big5" },    /* Chinese (Traditional) */

 { "af", "iso-8859-1" },  /* Afrikaans */
 { "ar", "iso-8859-6" },  /* Arabic */
 { "be", "iso-8859-5" },  /* Byelorussian */
 { "bg", "iso-8859-5" },  /* Bulgarian */
 { "ca", "iso-8859-1" },  /* Catalan */
 { "cs", "iso-8859-2" },  /* Czech */
 { "da", "iso-8859-1" },  /* Danish */
 { "de", "iso-8859-1" },  /* German */
 { "el", "iso-8859-7" },  /* Greek */ 
 { "en", "iso-8859-1" },  /* English */
 { "eo", "iso-8859-3" },  /* Esperanto */
 { "es", "iso-8859-1" },  /* Spanish */
 { "et", "iso-8859-4" },  /* Estonian  */
 { "eu", "iso-8859-1" },  /* basque */
 { "fi", "iso-8859-1" },  /* Finnish */
 { "fo", "iso-8859-1" },  /* faroese */
 { "fr", "iso-8859-1" },  /* French */
 { "ga", "iso-8859-1" },  /* Irish (Gaelic) */
 { "gd", "iso-8859-1" },  /* Scottish */
 { "he", "iso-8859-8" },  /* hebrew */
 { "hr", "iso-8859-2" },  /* Croatian */
 { "hu", "iso-8859-2" },  /* Hungarian */
 { "in", "iso-8859-1" },  /* Indonesian */
 { "is", "iso-8859-1" },  /* Icelandic */
 { "it", "iso-8859-1" },  /* Italian  */
 { "iw", "iso-8859-8" },  /* hebrew */
 { "ja", "Shift_JIS"  },  /* Japanese [was: ja_JP ] */
 { "ji", "iso-8859-8" },  /* Yiddish */
 { "kl", "iso-8859-4" },  /* Greenlandic */
 { "ko", "euc-kr"     },  /* korean [was: ko_KR ] */
 { "lt", "iso-8859-4" },  /* Lithuanian */
 { "lv", "iso-8859-4" },  /* latvian (lettish) */
 { "mk", "iso-8859-5" },  /* Macedonian */
 { "mt", "iso-8859-3" },  /* Maltese  */
 { "nl", "iso-8859-1" },  /* dutch */
 { "no", "iso-8859-1" },  /* Norwegian */
 { "pl", "iso-8859-2" },  /* Polish */
 { "pt", "iso-8859-1" },  /* Portugese */
 { "rm", "iso-8859-1" },  /* Rhaeto-romance */
 { "ro", "iso-8859-2" },  /* Romanian */
 { "ru", "iso-8859-5" },  /* Russian */
 { "sk", "iso-8859-2" },  /* Slovak  */
 { "sl", "iso-8859-2" },  /* Slovenian */
 { "sq", "iso-8859-1" },  /* albanian */
 { "sr", "iso-8859-5" },  /* Serbian */
 { "sv", "iso-8859-1" },  /* Swedish */
 { "sw", "iso-8859-1" },  /* Swahili */
 { "th", "tis-620"    },  /* Thai [windows-874] */
 { "tr", "iso-8859-9" },  /* Turkish */
 { "uk", "iso-8859-5" },  /* pre 1990 Ukranian... see: <http://czyborra.com/charsets/cyrillic.html#KOI8-U>  */
 { "zh", "Big-5"      },  /* Chinese (Traditional) */
 {  "",  ""           }
};

/* Not-used list, overridden old data  */
#if 0
 { "ar", "ibm-1256" }, /* arabic */
 { "ko", "ibm-949" }, /* korean  */
 { "ru", "ibm-878" }, /* Russian- koi8-r */
 { "sk", "ibm-912" }, 
#endif

U_CAPI const char *
uprv_defaultCodePageForLocale(const char *locale)
{
  int32_t i;
  int32_t locale_len;

  if (locale == NULL) 
  {
    return NULL;
  }
  locale_len = uprv_strlen(locale);

  if(locale_len < 2)
    {
      return NULL; /* non existent. Not a complete check, but it will
                    * make sure that 'c' doesn't match catalan, etc.
                    **/
    }

  for(i=0; _localeToDefaultCharmapTable[i].loc[0]; i++)
  {
    if(uprv_strncmp(locale, _localeToDefaultCharmapTable[i].loc, 
                    uprv_min(locale_len, 
                             uprv_strlen(_localeToDefaultCharmapTable[i].loc)))
       == 0)
    {
      return _localeToDefaultCharmapTable[i].charmap;
    }
  }

  return NULL;
}

