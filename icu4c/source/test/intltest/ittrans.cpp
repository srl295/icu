/*
*****************************************************************************************
*                                                                                       *
* COPYRIGHT:                                                                            *
*   (C) Copyright International Business Machines Corporation,  2001                    *
*   Licensed Material - Program-Property of IBM - All Rights Reserved.                  *
*   US Government Users Restricted Rights - Use, duplication, or disclosure             *
*   restricted by GSA ADP Schedule Contract with IBM Corp.                              *
*                                                                                       *
*****************************************************************************************
*/
/***********************************************************************
*   Date        Name        Description
*   01/03/2000  Madhu        Creation.
*   03/2000     Madhu        Added additional tests
***********************************************************************/
/**
 * IntlTestTransliterator is the medium level test class for Transliterator 
 */

#include "unicode/utypes.h"
#include "ittrans.h"
#include "transtst.h"
#include "transapi.h"
#include "cpdtrtst.h"
#include "unhxtrts.h"
#include "hxuntrts.h"
#include "ufltlgts.h"
#include "transrt.h"
#include "usettest.h"
#include "jamotest.h"
#include "trnserr.h"
#include "reptest.h"

#define CASE(id,test) case id:                                \
                          name = #test;                       \
                          if (exec) {                         \
                              logln(#test "---"); logln("");  \
                              test t;                         \
                              callTest(t, par);               \
                          }                                   \
                          break

void IntlTestTransliterator::runIndexedTest( int32_t index, UBool exec, const char* &name, char* par )
{
    if (exec) logln("TestSuite Transliterator");
    switch (index) {
        CASE(0, TransliteratorTest);
        CASE(1, TransliteratorAPITest);
        CASE(2, CompoundTransliteratorTest);
        CASE(3, UniToHexTransliteratorTest);
        CASE(4, HexToUniTransliteratorTest);
        CASE(5, UnicodeFilterLogicTest);
        CASE(6, TransliteratorRoundTripTest);
        CASE(7, UnicodeSetTest);
        CASE(8, JamoTest);
        CASE(9, TransliteratorErrorTest);
        CASE(10, ReplaceableTest);

        default: name=""; break;
    }
}
