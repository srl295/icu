/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-1999, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

#ifndef _NUMBERFORMATREGRESSIONTEST_
#define _NUMBERFORMATREGRESSIONTEST_
 
#include "unicode/utypes.h"
#include "unicode/numfmt.h"

#include "intltest.h"

#include "unicode/numfmt.h"

class DecimalFormat;

/** 
 * Performs regression test for MessageFormat
 **/
class NumberFormatRegressionTest: public IntlTest {    
    
    // IntlTest override
    void runIndexedTest( int32_t index, UBool exec, const char* &name, char* par );
public:

    void Test4075713(void);
    void Test4074620(void) ;
    void Test4088161 (void);
    void Test4087245 (void);
    void Test4087535 (void);
    void Test4088503 (void);
    void Test4066646 (void);
    float assignFloatValue(float returnfloat);
    void Test4059870(void);
    void Test4083018 (void);
    void Test4071492 (void);
    void Test4086575(void);
    void Test4068693(void);
    void Test4069754(void);
    void Test4087251 (void);
    void Test4090489 (void);
    void Test4090504 (void);
    void Test4095713 (void);
    void Test4092561 (void);
    void Test4092480 (void);
    void Test4087244 (void);
    void Test4070798 (void);
    void Test4071005 (void);
    void Test4071014 (void);
    void Test4071859 (void);
    void Test4093610(void);
    void roundingTest(DecimalFormat *df, double x, UnicodeString& expected);
    void Test4098741(void);
    void Test4074454(void);
    void Test4099404(void);
    void Test4101481(void);
    void Test4052223(void);
    void Test4061302(void);
    void Test4062486(void);
    void Test4108738(void);
    void Test4106658(void);
    void Test4106662(void);
    void Test4114639(void);
    void Test4106664(void);
    void Test4106667(void);
    void Test4110936(void);
    void Test4122840(void);
    void Test4125885(void);
    void Test4134034(void);
    void Test4134300(void);
    void Test4140009(void);
    void Test4141750(void);
    void Test4145457(void);
    void Test4147295(void);
    void Test4147706(void);

    void Test4162198(void);
    void Test4162852(void);

    void Test4167494(void);
    void Test4170798(void);
    void Test4176114(void);
    void Test4179818(void);
    void Test4212072(void);
    void Test4216742(void);
    void Test4217661(void);
    void Test4161100(void);
    void Test4243011(void);
    void Test4243108(void);

protected:
    UBool failure(UErrorCode status, const UnicodeString& msg);
};

class MyNumberFormatTest : public NumberFormat 
{
public:

    //public StringBuffer format(double number, StringBuffer toAppendTo, FieldPosition pos) 
  //{ return new StringBuffer(""); }
  //public StringBuffer format(long number,StringBuffer toAppendTo, FieldPosition pos) 
  //{ return new StringBuffer(""); }
  
  virtual UnicodeString& format(    double            number, 
                    UnicodeString&        toAppendTo, 
                    FieldPosition&        pos) const
    {
      toAppendTo = "";
      return toAppendTo;
    }
  
  
  //public Number parse(String text, ParsePosition parsePosition) 
  //{ return new Integer(0); }
  
  virtual void parse(    const UnicodeString&    text, 
            Formattable&            result, 
            ParsePosition&            parsePosition) const
    {
      result.setLong(0L);
    }
  
  virtual Format* clone() const 
    { return NULL; }
  
  virtual UnicodeString& format(int32_t, 
                UnicodeString& foo, 
                FieldPosition&) const
    { return foo.remove(); }
};

#endif // _NUMBERFORMATREGRESSIONTEST_
//eof
