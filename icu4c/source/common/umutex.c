/*
******************************************************************************
*
*   Copyright (C) 1997-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
******************************************************************************
*
* File CMUTEX.C
*
* Modification History:
*
*   Date        Name        Description
*   04/02/97    aliu        Creation.
*   04/07/99    srl         updated
*   05/13/99    stephen     Changed to umutex (from cmutex).
*   11/22/99    aliu        Make non-global mutex autoinitialize [j151]
******************************************************************************
*/

/* Assume POSIX, and modify as necessary below */
#define POSIX

#if defined(_WIN32)
#undef POSIX
#endif
#if defined(macintosh)
#undef POSIX
#endif
#if defined(OS2)
#undef POSIX
#endif


/* Check our settings... */
#include "unicode/utypes.h"


#if defined(POSIX) && (ICU_USE_THREADS==1)
  /* Usage: uncomment the following, and breakpoint WeAreDeadlocked to
     find reentrant issues. */
/* # define POSIX_DEBUG_REENTRANCY 1 */
# include <pthread.h> /* must be first, so that we get the multithread versions of things. */

# ifdef POSIX_DEBUG_REENTRANCY
 pthread_t      gLastThread;
 UBool         gInMutex;

 U_EXPORT void WeAreDeadlocked();

 void WeAreDeadlocked()
 {
    puts("ARGH!! We're deadlocked.. break on WeAreDeadlocked() next time.");
 }
# endif /* POSIX_DEBUG_REENTRANCY */
#endif /* POSIX && (ICU_USE_THREADS==1) */

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# define NOGDI
# define NOUSER
# define NOSERVICE
# define NOIME
# define NOMCX
# include <windows.h>
#endif

#include "umutex.h"
#include "cmemory.h"

#if (ICU_USE_THREADS == 1)

/* the global mutex. Use it proudly and wash it often. */
static UMTX    gGlobalMutex = NULL;

#if defined(WIN32)
static CRITICAL_SECTION gPlatformMutex;

#elif defined(POSIX)
static pthread_mutex_t gPlatformMutex;

#endif
#endif /* ICU_USE_THREADS==1 */

U_CAPI void  U_EXPORT2
umtx_lock(UMTX *mutex)
{
#if (ICU_USE_THREADS == 1)
    if (mutex == NULL)
    {
        mutex = &gGlobalMutex;
    }

    if (*mutex == NULL)
    {
        if (mutex != &gGlobalMutex) {umtx_lock(NULL);};
        if (*mutex == NULL) {
            umtx_init(mutex);
        }
        if (mutex != &gGlobalMutex) {umtx_unlock(NULL);};
    }

#if defined(WIN32)

    EnterCriticalSection((CRITICAL_SECTION*) *mutex);

#elif defined(POSIX)

#  ifdef POSIX_DEBUG_REENTRANCY
    if (gInMutex == TRUE && mutex == &gGlobalMutex) /* in the mutex -- possible deadlock*/
        if(pthread_equal(gLastThread, pthread_self()))
            WeAreDeadlocked();
#  endif
    pthread_mutex_lock((pthread_mutex_t*) *mutex);

#  ifdef POSIX_DEBUG_REENTRANCY
    if (mutex == &gGlobalMutex) {
        gLastThread = pthread_self();
        gInMutex = TRUE;
    }
#  endif
#endif
#endif /* ICU_USE_THREADS==1 */
}

U_CAPI void  U_EXPORT2
umtx_unlock(UMTX* mutex)
{
#if (ICU_USE_THREADS==1)
    if(mutex == NULL)
    {
        mutex = &gGlobalMutex;
    }

    if(*mutex == NULL)
    {
        return; /* jitterbug 135, fix for multiprocessor machines */
    }

#if defined (WIN32)
    LeaveCriticalSection((CRITICAL_SECTION*)*mutex);

#elif defined (POSIX)
    pthread_mutex_unlock((pthread_mutex_t*)*mutex);

#ifdef POSIX_DEBUG_REENTRANCY
    if (mutex == &gGlobalMutex) {
        gInMutex = FALSE;
    }
#endif

#endif
#endif /* ICU_USE_THREADS == 1 */
}

U_CAPI void  U_EXPORT2
umtx_init(UMTX *mutex)
{
#if (ICU_USE_THREADS == 1)

    if (mutex == NULL) /* initialize the global mutex */
    {
        mutex = &gGlobalMutex;
    }

    if (*mutex != NULL) /* someone already did it. */
        return;

    if (*mutex == gGlobalMutex)
    {
        *mutex = &gPlatformMutex;
    }
    else
    {
#if defined (WIN32)
        *mutex = uprv_malloc(sizeof(CRITICAL_SECTION));
#elif defined( POSIX )
        *mutex = uprv_malloc(sizeof(pthread_mutex_t));
#endif
    }

#if defined (WIN32)
    InitializeCriticalSection((CRITICAL_SECTION*)*mutex);

#elif defined (POSIX)
# if defined (HPUX_CMA)
    pthread_mutex_init((pthread_mutex_t*)*mutex, pthread_mutexattr_default);
# else
    pthread_mutex_init((pthread_mutex_t*)*mutex, NULL);
# endif

# ifdef POSIX_DEBUG_REENTRANCY
    gInMutex = FALSE;
# endif

#endif
#endif /* ICU_USE_THREADS==1 */
}

U_CAPI void  U_EXPORT2
umtx_destroy(UMTX *mutex) {
#if (ICU_USE_THREADS == 1)
    if (mutex == NULL) /* initialize the global mutex */
    {
        mutex = &gGlobalMutex;
    }

    if (*mutex == NULL) /* someone already did it. */
        return;

#if defined (WIN32)
    DeleteCriticalSection((CRITICAL_SECTION*)*mutex);

#elif defined (POSIX)
    pthread_mutex_destroy((pthread_mutex_t*)*mutex);

#endif

    if (*mutex != gGlobalMutex)
    {
        uprv_free(*mutex);
    }

    *mutex = NULL;
#endif /* ICU_USE_THREADS==1 */
}


#if (ICU_USE_THREADS == 1) 


//
//   umtx_atomic_inc
//   umtx_atomic_dec
//
#if defined (WIN32)
//
//  Win32 - use the Windows API functions for atomic increment and decrement.
//
U_CAPI int32_t U_EXPORT2
umtx_atomic_inc(int32_t *p)
{
    return InterlockedIncrement(p);
}

U_CAPI int32_t U_EXPORT2
umtx_atomic_dec(int32_t *p)
{
    return InterlockedDecrement(p); 
}

#elif defined (POSIX)
//
//  POSIX platforms without specific atomic operations.  Use a posix mutex
//     to protect the increment and decrement.
//     Put the mutex in static storage so we don't have to come back and delete it
//     when the process exits.
//
static pthread_mutex_t gIncDecMutex;
static UBool           gIncDecMutexInitialized = FALSE;

U_CAPI int32_t U_EXPORT2
umtx_atomic_inc(int32_t *p)
{
    int32_t    retVal;

    if (gIncDecMutexInitialized == FALSE) {
        umtx_lock(NULL);
        if (gIncDecMutexInitialized == FALSE) {
# if defined (HPUX_CMA)
            pthread_mutex_init((pthread_mutex_t*)&gIncDecMutex, pthread_mutexattr_default);
# else
            pthread_mutex_init((pthread_mutex_t*)&gIncDecMutex, NULL);
            gIncDecMutexInitialized = TRUE;
        }
# endif
        umtx_unlock(NULL);
    }
   

    pthread_mutex_lock(&gIncDecMutex);
    retVal = ++(*p);
    pthread_mutex_unlock(&gIncDecMutex);
    return retVal;
}


U_CAPI int32_t U_EXPORT2
umtx_atomic_dec(int32_t *p)
{
    int32_t    retVal;

    pthread_mutex_lock(&gIncDecMutex);
    retVal = --(*p);
    pthread_mutex_unlock(&gIncDecMutex);
    return retVal;
}

// TODO:  pthread_mutex_destroy() when the time comes.

#else 
   
// No recognized platform. 
#warning  No atomic increment and decrement defined for this platform.

U_CAPI int32_t U_EXPORT2
umtx_atomic_inc(int32_t *p) {
    return ++(*p);
}

U_CAPI int32_t U_EXPORT2
umtx_atomic_dec(int32_t *p) {
    return --(*p);
}

#endif   // Platform selection for atomic_inc and dec.


#elif  // (ICU_USE_THREADS == 1)

U_CAPI int32_t U_EXPORT2
umtx_atomic_inc(int32_t *p) {
    return ++(*p);
}

U_CAPI int32_t U_EXPORT2
umtx_atomic_dec(int32_t *p) {
    return --(*p);
}

#endif // (ICU_USE_THREADS == 1)




