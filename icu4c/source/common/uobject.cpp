/*
******************************************************************************
*
*   Copyright (C) 2002-2003, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
******************************************************************************
*   file name:  uobject.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2002jun26
*   created by: Markus W. Scherer
*/

#include "unicode/uobject.h"
#include "cmemory.h"

U_NAMESPACE_BEGIN

#if U_OVERRIDE_CXX_ALLOCATION

/*
 * Default implementation of UMemory::new/delete
 * using uprv_malloc() and uprv_free().
 *
 * For testing, this is used together with a list of imported symbols to verify
 * that ICU is not using the global ::new and ::delete operators.
 *
 * These operators can be implemented like this or any other appropriate way
 * when customizing ICU for certain environments.
 * Whenever ICU is customized in binary incompatible ways please be sure
 * to use library name suffixes to distinguish such libraries from
 * the standard build.
 *
 * Instead of just modifying these C++ new/delete operators, it is usually best
 * to modify the uprv_malloc()/uprv_free()/uprv_realloc() functions in cmemory.c.
 *
 * Memory test on Windows/MSVC 6:
 * The global operators new and delete look as follows:
 *   04F 00000000 UNDEF  notype ()    External     | ??2@YAPAXI@Z (void * __cdecl operator new(unsigned int))
 *   03F 00000000 UNDEF  notype ()    External     | ??3@YAXPAX@Z (void __cdecl operator delete(void *))
 *
 * These lines are from output generated by the MSVC 6 tool dumpbin with
 * dumpbin /symbols *.obj
 *
 * ??2@YAPAXI@Z and ??3@YAXPAX@Z are the linker symbols in the .obj
 * files and are imported from msvcrtd.dll (in a debug build).
 *
 * Make sure that with the UMemory operators new and delete defined these two symbols
 * do not appear in the dumpbin /symbols output for the ICU libraries!
 *
 * If such a symbol appears in the output then look in the preceding lines in the output
 * for which file and function calls the global new or delete operator,
 * and replace with uprv_malloc/uprv_free.
 */

void *UMemory::operator new(size_t size) {
    return uprv_malloc(size);
}

void UMemory::operator delete(void *p) {
    if(p!=NULL) {
        uprv_free(p);
    }
}

void *UMemory::operator new[](size_t size) {
    return uprv_malloc(size);
}

void UMemory::operator delete[](void *p) {
    if(p!=NULL) {
        uprv_free(p);
    }
}

#endif

UObject::~UObject() {}

// Future implementation for RTTI that support subtyping. [alan]
// 
// UClassID UObject::getStaticClassID() {
//     return (UClassID) NULL;
// }
// 
// UBool UObject::instanceOf(UClassID type) const {
//     UClassID c = getDynamicClassID();
//     for (;;) {
//         if (c == type) {
//             return TRUE;
//         } else if (c == (UClassID) NULL) {
//             return FALSE;
//         }
//         c = * (UClassID*) c;
//     }
// }

U_NAMESPACE_END


