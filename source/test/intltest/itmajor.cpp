/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1998-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

/**
 * MajorTestLevel is the top level test class for everything in the directory "IntlWork".
 */

/***********************************************************************
* Modification history
* Date        Name        Description
* 02/14/2001  synwee      Release collation for testing.
***********************************************************************/

#include "unicode/utypes.h"
#include "itmajor.h"

#include "itutil.h"
#include "tscoll.h"
#include "ittxtbd.h"
#include "itformat.h"
#include "itconv.h"
#include "ittrans.h"
#include "itrbbi.h"

void MajorTestLevel::runIndexedTest( int32_t index, UBool exec, const char* &name, char* par )
{
    switch (index) {
        case 0: name = "utility"; 
                if (exec) { 
                    logln("TestSuite Utilities---"); logln();
                    IntlTestUtilities test;
                    callTest( test, par );
                }
                break;

        case 1: name = "collate"; 
                if (exec) {
                    logln("TestSuite Collator----"); logln();
                    IntlTestCollator test;
                    callTest( test, par );
                }
                break;

        case 2: name = "textbounds"; 
                if (exec) {
                    logln("TestSuite TextBoundary----"); logln();
                    IntlTestTextBoundary test;
                    callTest( test, par );
                }
                break;

        case 3: name = "format"; 
                if (exec) {
                    logln("TestSuite Format----"); logln();
                    IntlTestFormat test;
                    callTest( test, par );
                }
                break;

        case 4: name = "convert"; 
                if (exec) {
                    logln("TestSuite Convert----"); logln();
                    IntlTestConvert test;
                    callTest( test, par );
                }
                break;

        case 5: name = "translit"; 
                if (exec) {
                    logln("TestSuite Transliterator----"); logln();
                    IntlTestTransliterator test;
                    callTest( test, par );
                }
                break;
        case 6: name = "rbbi"; 
                if (exec) {
                    logln("TestSuite RuleBasedBreakIterator----"); logln();
                    IntlTestRBBI test;
                    callTest( test, par );
                }
                break;

        default: name = ""; break;
    }
}

