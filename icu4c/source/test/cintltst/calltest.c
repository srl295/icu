/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1996-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/********************************************************************************
*
* File CALLTEST.C
*
* Modification History:
*   Creation:   Madhu Katragadda 
*********************************************************************************
*/
/* THE FILE WHERE ALL C API TESTS ARE ADDED TO THE ROOT */


#include "cintltst.h"

void addSetup(TestNode** root);
void addUtility(TestNode** root);
void addBreakIter(TestNode** root);
void addStandardNamesTest(TestNode **root);
void addFormatTest(TestNode** root);
void addConvert(TestNode** root);
void addCollTest(TestNode** root);
void addComplexTest(TestNode** root);
void addUDataTest(TestNode** root);
void addUTF16Test(TestNode** root);
void addUTF8Test(TestNode** root);
void addUTransTest(TestNode** root);
void addPUtilTest(TestNode** root);
void addCompactArrayTest(TestNode** root);
void addTestDeprecatedAPI(TestNode** root);
void addUCharTransformTest(TestNode** root);

void addAllTests(TestNode** root)
{
    addSetup(root);  /* Leave this test first! */
    addUDataTest(root);
    addPUtilTest(root);
    addUTF16Test(root);
    addUTF8Test(root);
    addUtility(root);
    addConvert(root);
    addUCharTransformTest(root);
    addCompactArrayTest(root);
    addFormatTest(root);
    addStandardNamesTest(root);
    addBreakIter(root);
    addCollTest(root);
    addComplexTest(root);
    addUTransTest(root);
    addTestDeprecatedAPI(root);
}

