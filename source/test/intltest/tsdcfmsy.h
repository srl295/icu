/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

#ifndef _INTLTESTDECIMALFORMATSYMBOLS
#define _INTLTESTDECIMALFORMATSYMBOLS


#include "unicode/utypes.h"
#include "intltest.h"
#include "unicode/dcfmtsym.h"
/**
 * Tests for DecimalFormatSymbols
 **/
class IntlTestDecimalFormatSymbols: public IntlTest {
    void runIndexedTest( int32_t index, UBool exec, const char* &name, char* par = NULL );  

private:
    /**
     * Test the API of DecimalFormatSymbols; primarily a simple get/set set.
     */
    void testSymbols(/*char *par*/);

     /** helper functions**/
    void Verify(double value, const UnicodeString& pattern, DecimalFormatSymbols sym, const UnicodeString& expected);
};

#endif
