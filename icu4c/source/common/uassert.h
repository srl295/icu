/*
******************************************************************************
*
*   Copyright (C) 2002, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
******************************************************************************
*
* File uassert.h
*
*  Contains U_ASSERT macro
*
*    By default, U_ASSERT just wraps the C library assert macro.
*    By changing the definition here, the assert behavior for ICU can be changed
*    without affecting other non-ICU uses of the C library assert().
*
******************************************************************************
*/

#ifndef U_ASSERT_H
#define U_ASSERT_H
#include <assert.h>
#define U_ASSERT(exp) assert(exp)
#endif


