/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

#include "intltest.h"

/** 
 * Test API and functionality of class Unicode
 **/
class UnicodeTest: public IntlTest {
public:
    UnicodeTest();
    virtual ~UnicodeTest();
    
    void runIndexedTest( int32_t index, UBool exec, const char* &name, char* par = NULL );

    /** 
     * Test methods toUpperCase() and toLowerCase()
     **/
    void TestUpperLower(void);
    /** 
     * Test methods isLetter() and isDigit() 
     **/
    void TestLetterNumber(void);
    /** 
     * Tests methods isControl() and isPrintable()
     **/
    void TestControlPrint(void);
    /** 
     * Tests methods isJavaIdentifierStart(), isJavaIdentifierPart(),
     * isUnicodeIdentifierStart() and isUnicodeIdentifierPart()
     **/
    void TestIdentifier(void);
    /** 
     * Tests methods isDefined(), isBaseForm(), isSpaceChar() and getCellWidth()
     **/
    void TestMisc(void);

    /** Helper function for TestUnicodeData */
    static void U_CALLCONV unicodeDataLineFn(void *context,
                                  char *fields[][2], int32_t fieldCount,
                                  UErrorCode *pErrorCode);
    /** 
     * tests methods getType(), isTitleCase(), and toTitleCase() 
     * as well as characterDirection()
     **/
    void TestUnicodeData(void);
    /** 
      * tests methods isSingle(), isLead(), and isTrail() 
      **/
    void TestCodeUnit(void);
    /**
      * Tests for isSurrogate(), isUnicodeChar(), isError(), isValid() 
      **/
    void TestCodePoint();
    /**
      * Tests for needMultipleChar(), charLength()
      **/
    void TestCharLength();
    /**
      * Tests for getScript()
      **/
    void TestScript();

private:
    /**
     * internal utility used by TestUnicodeData
     **/
    int32_t MakeProp(char* str);
    /**
     * internal utility used by TestUnicodeData
     **/
    int32_t MakeDir(char* str);
};

