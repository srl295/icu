/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

#include "cmemory.h"
#include "unicode/coll.h"
#include "unicode/tblcoll.h"
#include "unicode/unistr.h"
#include "unicode/sortkey.h"
#include "itercoll.h"
#include "unicode/schriter.h"
#include "unicode/chariter.h"

#define ARRAY_LENGTH(array) (sizeof array / sizeof array[0])

static UErrorCode status = U_ZERO_ERROR;

const UnicodeString CollationIteratorTest::test1 = "What subset of all possible test cases?";
const UnicodeString CollationIteratorTest::test2 = "has the highest probability of detecting";

CollationIteratorTest::CollationIteratorTest()
{
    en_us = (RuleBasedCollator *)Collator::createInstance(Locale::US, status);
}

CollationIteratorTest::~CollationIteratorTest()
{
    delete en_us;
}

/**
 * Test for CollationElementIterator previous and next for the whole set of
 * unicode characters.
 */
void CollationIteratorTest::TestUnicodeChar()
{
    CollationElementIterator *iter;
    UChar codepoint;
    UnicodeString source;
    
    for (codepoint = 1; codepoint < 0xFFFE;)
    {
      source.remove();

      while (codepoint % 0xFF != 0) 
      {
        if (u_isdefined(codepoint))
          source += codepoint;
        codepoint ++;
      }

      if (u_isdefined(codepoint))
        source += codepoint;
      
      if (codepoint != 0xFFFF)
        codepoint ++;

      iter = en_us->createCollationElementIterator(source);
      /* A basic test to see if it's working at all */
      backAndForth(*iter);
      delete iter;
    }
}

/**
 * Test for CollationElementIterator.previous()
 *
 * @bug 4108758 - Make sure it works with contracting characters
 * 
 */
void CollationIteratorTest::TestPrevious(/* char* par */)
{
    UErrorCode status = U_ZERO_ERROR;
    CollationElementIterator *iter = en_us->createCollationElementIterator(test1);

    // A basic test to see if it's working at all
    backAndForth(*iter);
    delete iter;

    // Test with a contracting character sequence
    UnicodeString source;
    RuleBasedCollator *c1 = NULL;
    c1 = new RuleBasedCollator(
        (UnicodeString)"&a,A < b,B < c,C, d,D < z,Z < ch,cH,Ch,CH", status);

    if (c1 == NULL || U_FAILURE(status))
    {
        errln("Couldn't create a RuleBasedCollator with a contracting sequence.");
        delete c1;
        return;
    }

    source = "abchdcba";
    iter = c1->createCollationElementIterator(source);
    backAndForth(*iter);
    delete iter;
    delete c1;

    // Test with an expanding character sequence
    RuleBasedCollator *c2 = NULL;
    c2 = new RuleBasedCollator((UnicodeString)"&a < b < c/abd < d", status);

    if (c2 == NULL || U_FAILURE(status))
    {
        errln("Couldn't create a RuleBasedCollator with an expanding sequence.");
        delete c2;
        return;
    }

    source = "abcd";
    iter = c2->createCollationElementIterator(source);
    backAndForth(*iter);
    delete iter;
    delete c2;

    // Now try both
    RuleBasedCollator *c3 = NULL;
    c3 = new RuleBasedCollator((UnicodeString)"&a < b < c/aba < d < z < ch", status);

    if (c3 == NULL || U_FAILURE(status))
    {
        errln("Couldn't create a RuleBasedCollator with both an expanding and a contracting sequence.");
        delete c3;
        return;
    }

    source = "abcdbchdc";
    iter = c3->createCollationElementIterator(source);
    backAndForth(*iter);
    delete iter;
    delete c3;

    status=U_ZERO_ERROR;
    source= CharsToUnicodeString("\\u0e41\\u0e02\\u0e41\\u0e02\\u0e27abc");
    
    Collator *c4 = Collator::createInstance(Locale("th", "TH", ""), status);
    if(U_FAILURE(status)){
        errln("Couldn't create a collator");
    }
    iter = ((RuleBasedCollator*)c4)->createCollationElementIterator(source);
    backAndForth(*iter);
    delete iter;
    delete c4;
   
    source= CharsToUnicodeString("\\u0061\\u30CF\\u3099\\u30FC");
    Collator *c5 = Collator::createInstance(Locale("ja", "JP", ""), status);

    iter = ((RuleBasedCollator*)c5)->createCollationElementIterator(source);
    if(U_FAILURE(status)){
        errln("Couldn't create Japanese collator\n");
    }
    backAndForth(*iter);
    delete iter;
    delete c5;
}

/**
 * Test for getOffset() and setOffset()
 */
void CollationIteratorTest::TestOffset(/* char* par */)
{
    CollationElementIterator *iter = en_us->createCollationElementIterator(test1);

    // Run all the way through the iterator, then get the offset
    int32_t orderLength = 0;
    int32_t *orders = getOrders(*iter, orderLength);

    int32_t offset = iter->getOffset();

    if (offset != test1.length())
    {
        UnicodeString msg1("offset at end != length: ");
        UnicodeString msg2(" vs ");

        errln(msg1 + offset + msg2 + test1.length());
    }

    // Now set the offset back to the beginning and see if it works
    CollationElementIterator *pristine = en_us->createCollationElementIterator(test1);
    UErrorCode status = U_ZERO_ERROR;

    iter->setOffset(0, status);

    if (U_FAILURE(status))
    {
        errln("setOffset failed.");
    }
    else
    {
        assertEqual(*iter, *pristine);
    }

    // TODO: try iterating halfway through a messy string.

    delete pristine;
    delete[] orders;
    delete iter;
}

/**
 * Test for setText()
 */
void CollationIteratorTest::TestSetText(/* char* par */)
{
    CollationElementIterator *iter1 = en_us->createCollationElementIterator(test1);
    CollationElementIterator *iter2 = en_us->createCollationElementIterator(test2);
    UErrorCode status = U_ZERO_ERROR;

    // Run through the second iterator just to exercise it
    int32_t c = iter2->next(status);
    int32_t i = 0;

    while ( ++i < 10 && c != CollationElementIterator::NULLORDER)
    {
        if (U_FAILURE(status))
        {
            errln("iter2->next() returned an error.");
            delete iter2;
            delete iter1;
        }

        c = iter2->next(status);
    }

    // Now set it to point to the same string as the first iterator
    iter2->setText(test1, status);

    if (U_FAILURE(status))
    {
        errln("call to iter2->setText(test1) failed.");
    }
    else
    {
        assertEqual(*iter1, *iter2);
    }
    iter1->reset();
    //now use the overloaded setText(ChracterIterator&, UErrorCode) function to set the text
    CharacterIterator* chariter = new StringCharacterIterator(test1);
    iter2->setText(*chariter, status);
    if (U_FAILURE(status))
    {
        errln("call to iter2->setText(chariter(test1)) failed.");
    }
    else
    {
        assertEqual(*iter1, *iter2);
    }
   
    delete iter2;
    delete iter1;
}

/** @bug 4108762
 * Test for getMaxExpansion()
 */
void CollationIteratorTest::TestMaxExpansion(/* char* par */)
{
    UErrorCode          status = U_ZERO_ERROR; 
    UnicodeString rule("&a < ab < c/aba < d < z < ch");
    RuleBasedCollator  *coll   = new RuleBasedCollator(rule, status);
    UChar               ch     = 0;
    UnicodeString       str(ch);

    CollationElementIterator *iter   = coll->createCollationElementIterator(str);

    while (ch < 0xFFFF && U_SUCCESS(status)) {
        int      count = 1;
        uint32_t order;
        ch ++;
        UnicodeString str(ch);
        iter->setText(str, status);
        order = iter->previous(status);

        /* thai management */
        if (order == 0)
            order = iter->previous(status);

        while (U_SUCCESS(status)
            && iter->previous(status) != (int32_t)UCOL_NULLORDER)
        {
            count ++; 
        }

        if (U_FAILURE(status) && iter->getMaxExpansion(order) < count) {
            errln("Failure at codepoint %d, maximum expansion count < %d\n",
                ch, count);
        }
    }

    delete iter;
    delete coll;
}

/*
 * @bug 4157299
 */
void CollationIteratorTest::TestClearBuffers(/* char* par */)
{
    UErrorCode status = U_ZERO_ERROR;
    RuleBasedCollator *c = NULL;
    c = new RuleBasedCollator((UnicodeString)"&a < b < c & ab = d", status);

    if (c == NULL || U_FAILURE(status))
    {
        errln("Couldn't create a RuleBasedCollator.");
        delete c;
        return;
    }

    UnicodeString source("abcd");
    CollationElementIterator *i = c->createCollationElementIterator(source);
    int32_t e0 = i->next(status);    // save the first collation element

    if (U_FAILURE(status))
    {
        errln("call to i->next() failed");
        goto bail;
    }

    i->setOffset(3, status);        // go to the expanding character

    if (U_FAILURE(status))
    {
        errln("call to i->setOffset(3) failed");
        goto bail;
    }

    i->next(status);                // but only use up half of it

    if (U_FAILURE(status))
    {
        errln("call to i->next() failed");
        goto bail;
    }

    i->setOffset(0, status);        // go back to the beginning

    if (U_FAILURE(status))
    {
        errln("call to i->setOffset(0) failed");
        goto bail;
    }

    {
        // This is in it's own block to stop a stupid compiler
        // error about the goto's skipping the initialization
        // of e...
        int32_t e = i->next(status);    // and get this one again

        if (U_FAILURE(status))
        {
            errln("call to i->next() failed.");
            goto bail;
        }

        if (e != e0)
        {
            UnicodeString msg;
            
            msg += "got 0x";
            appendHex(e, 8, msg);
            msg += ", expected 0x";
            appendHex(e0, 8, msg);

            errln(msg);
        }
    }

bail:
    delete i;
    delete c;
}

void CollationIteratorTest::backAndForth(CollationElementIterator &iter)
{
    // Run through the iterator forwards and stick it into an array
    int32_t orderLength = 0;
    int32_t *orders = getOrders(iter, orderLength);
    UErrorCode status = U_ZERO_ERROR;

    // Now go through it backwards and make sure we get the same values
    int32_t index = orderLength;
    int32_t o;

    // reset the iterator
    iter.reset();

    while ((o = iter.previous(status)) != CollationElementIterator::NULLORDER)
    {
        if (o != orders[--index])
        {
          if (o == 0)
            index ++;
          else
          {
            while (index > 0 && orders[-- index] == 0)
            {
            }
            if (o != orders[index])
            {
              UnicodeString msg1("Mismatch at index ");
              UnicodeString msg2(": 0x");
              appendHex(orders[index], 8, msg2);
              msg2 += " vs 0x";
              appendHex(o, 8, msg2);

              errln(msg1 + index + msg2);
              break;
            }
          }
        }
    }

    while (index != 0 && orders[index - 1] == 0)
    {
      index --;
    }

    if (index != 0)
    {
        UnicodeString msg("Didn't get back to beginning - index is ");
        errln(msg + index);

        iter.reset();
        err("next: ");
        while ((o = iter.next(status)) != CollationElementIterator::NULLORDER)
        {
            UnicodeString hexString("0x");

            appendHex(o, 8, hexString);
            hexString += " ";
            err(hexString);
        }
        errln("");

        err("prev: ");
        while ((o = iter.previous(status)) != CollationElementIterator::NULLORDER)
        {
            UnicodeString hexString("0x");

            appendHex(o, 8, hexString);
            hexString += " ";
             err(hexString);
        }
        errln("");
    }

    delete[] orders;
}

/**
 * Return an integer array containing all of the collation orders
 * returned by calls to next on the specified iterator
 */
int32_t *CollationIteratorTest::getOrders(CollationElementIterator &iter, int32_t &orderLength)
{
    int32_t maxSize = 100;
    int32_t size = 0;
    int32_t *orders = new int32_t[maxSize];
    UErrorCode status = U_ZERO_ERROR;

    int32_t order;
    while ((order = iter.next(status)) != CollationElementIterator::NULLORDER)
    {
        if (size == maxSize)
        {
            maxSize *= 2;
            int32_t *temp = new int32_t[maxSize];

            uprv_memcpy(temp, orders, size * sizeof(int32_t));
            delete[] orders;
            orders = temp;
        }

        orders[size++] = order;
    }

    if (maxSize > size)
    {
        int32_t *temp = new int32_t[size];

        memcpy(temp, orders, size * sizeof(int32_t));
        delete[] orders;
        orders = temp;
    }

    orderLength = size;
    return orders;
}

/**
 * Return a string containing all of the collation orders
 * returned by calls to next on the specified iterator
 */
UnicodeString &CollationIteratorTest::orderString(CollationElementIterator &iter, UnicodeString &target)
{
    int32_t order;
    UErrorCode status = U_ZERO_ERROR;

    while ((order = iter.next(status)) != CollationElementIterator::NULLORDER)
    {
        target += "0x";
        appendHex(order, 8, target);
        target += " ";
    }

    return target;
}

void CollationIteratorTest::assertEqual(CollationElementIterator &i1, CollationElementIterator &i2)
{
    int32_t c1, c2, count = 0;
    UErrorCode status = U_ZERO_ERROR;

    do
    {
        c1 = i1.next(status);
        c2 = i2.next(status);

        if (c1 != c2)
        {
            UnicodeString msg, msg1("    ");
            
            msg += msg1 + count;
            msg += ": strength(0x";
            appendHex(c1, 8, msg);
            msg += ") != strength(0x";
            appendHex(c2, 8, msg);
            msg += ")";

            errln(msg);
            break;
        }

        count += 1;
    }
    while (c1 != CollationElementIterator::NULLORDER);
}

void CollationIteratorTest::runIndexedTest(int32_t index, UBool exec, const char* &name, char* /*par*/)
{
    if (exec)
    {
        logln("Collation Iteration Tests: ");
    }

    switch (index)
    {
        case  0: name = "TestPrevious";        if (exec) TestPrevious(/* par */);     break;
        case  1: name = "TestOffset";        if (exec) TestOffset(/* par */);         break;
        case  2: name = "TestSetText";        if (exec) TestSetText(/* par */);         break;
        case  3: name = "TestMaxExpansion";    if (exec) TestMaxExpansion(/* par */); break;
        case  4: name = "TestClearBuffers"; if (exec) TestClearBuffers(/* par */); break;
        case  5: name = "TestUnicodeChar"; if (exec) TestUnicodeChar(/* par */); break;
        default: name = ""; break;
    }
}

