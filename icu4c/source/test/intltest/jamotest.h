/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************
************************************************************************
*   Date        Name        Description
*   02/28/2001  aliu        Creation
*   03/01/2001  George      port to HP/UX
************************************************************************/

#ifndef JAMOTEST_H
#define JAMOTEST_H

#include "transtst.h"

/**
 * @test
 * @summary Test of Latin-Jamo and Jamo-Latin rules
 */
class JamoTest : public TransliteratorTest {

    void runIndexedTest(int32_t index, UBool exec, const char* &name,
                        char* par=NULL);

    void TestJamo(void);
    
    void TestRealText(void);

    void TestPiecemeal(void);

    //======================================================================
    // Support methods
    //======================================================================

    // Override TransliteratorTest
    virtual void expectAux(const UnicodeString& tag,
                           const UnicodeString& summary, UBool pass,
                           const UnicodeString& expectedResult);
    
    // Methods to convert Jamo to/from readable short names,
    // e.g. (Gi) <> U+1100
    static const char* JAMO_NAMES_RULES;
    static Transliterator* JAMO_NAME;
    static Transliterator* NAME_JAMO;
    static UnicodeString nameToJamo(const UnicodeString& input);
    static UnicodeString jamoToName(const UnicodeString& input);
};

#endif
