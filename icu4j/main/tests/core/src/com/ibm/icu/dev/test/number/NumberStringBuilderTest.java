// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
package com.ibm.icu.dev.test.number;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import java.text.FieldPosition;
import java.text.Format.Field;

import org.junit.Test;

import com.ibm.icu.impl.number.NumberStringBuilder;
import com.ibm.icu.text.NumberFormat;

/** @author sffc */
public class NumberStringBuilderTest {
  private static final String[] EXAMPLE_STRINGS = {
    "",
    "xyz",
    "The quick brown fox jumps over the lazy dog",
    "😁",
    "mixed 😇 and ASCII",
    "with combining characters like 🇦🇧🇨🇩",
    "A very very very very very very very very very very long string to force heap"
  };

  @Test
  public void testInsertAppendCharSequence() {

    StringBuilder sb1 = new StringBuilder();
    NumberStringBuilder sb2 = new NumberStringBuilder();
    for (String str : EXAMPLE_STRINGS) {
      NumberStringBuilder sb3 = new NumberStringBuilder();
      sb1.append(str);
      sb2.append(str, null);
      sb3.append(str, null);
      assertCharSequenceEquals(sb1, sb2);
      assertCharSequenceEquals(sb3, str);

      StringBuilder sb4 = new StringBuilder();
      NumberStringBuilder sb5 = new NumberStringBuilder();
      sb4.append("😇");
      sb4.append(str);
      sb4.append("xx");
      sb5.append("😇xx", null);
      sb5.insert(2, str, null);
      assertCharSequenceEquals(sb4, sb5);

      int start = Math.min(1, str.length());
      int end = Math.min(10, str.length());
      sb4.insert(3, str, start, end);
      sb5.insert(3, str, start, end, null);
      assertCharSequenceEquals(sb4, sb5);

      sb4.append(str.toCharArray());
      sb5.append(str.toCharArray(), null);
      assertCharSequenceEquals(sb4, sb5);

      sb4.insert(4, str.toCharArray());
      sb5.insert(4, str.toCharArray(), null);
      assertCharSequenceEquals(sb4, sb5);

      sb4.append(sb4.toString());
      sb5.append(new NumberStringBuilder(sb5));
      assertCharSequenceEquals(sb4, sb5);
    }
  }

  @Test
  public void testInsertAppendCodePoint() {
    int[] cases = {0, 1, 60, 127, 128, 0x7fff, 0x8000, 0xffff, 0x10000, 0x1f000, 0x10ffff};

    StringBuilder sb1 = new StringBuilder();
    NumberStringBuilder sb2 = new NumberStringBuilder();
    for (int cas : cases) {
      NumberStringBuilder sb3 = new NumberStringBuilder();
      sb1.appendCodePoint(cas);
      sb2.appendCodePoint(cas, null);
      sb3.appendCodePoint(cas, null);
      assertCharSequenceEquals(sb1, sb2);
      assertEquals(Character.codePointAt(sb3, 0), cas);

      StringBuilder sb4 = new StringBuilder();
      NumberStringBuilder sb5 = new NumberStringBuilder();
      sb4.append("😇");
      sb4.appendCodePoint(cas); // Java StringBuilder has no insertCodePoint()
      sb4.append("xx");
      sb5.append("😇xx", null);
      sb5.insertCodePoint(2, cas, null);
      assertCharSequenceEquals(sb4, sb5);
    }
  }

  @Test
  public void testCopy() {
    for (String str : EXAMPLE_STRINGS) {
      NumberStringBuilder sb1 = new NumberStringBuilder();
      sb1.append(str, null);
      NumberStringBuilder sb2 = new NumberStringBuilder(sb1);
      assertCharSequenceEquals(sb1, sb2);
      assertTrue(sb1.contentEquals(sb2));

      sb1.append("12345", null);
      assertNotEquals(sb1.length(), sb2.length());
      assertFalse(sb1.contentEquals(sb2));
    }
  }

  @Test
  public void testFields() {
    for (String str : EXAMPLE_STRINGS) {
      NumberStringBuilder sb = new NumberStringBuilder();
      sb.append(str, null);
      sb.append(str, NumberFormat.Field.CURRENCY);
      Field[] fields = sb.toFieldArray();
      assertEquals(str.length() * 2, fields.length);
      for (int i = 0; i < str.length(); i++) {
        assertEquals(null, fields[i]);
        assertEquals(null, sb.fieldAt(i));
        assertEquals(NumberFormat.Field.CURRENCY, fields[i + str.length()]);
        assertEquals(NumberFormat.Field.CURRENCY, sb.fieldAt(i + str.length()));
      }

      // Very basic FieldPosition test. More robust tests happen in NumberFormatTest.
      // Let NumberFormatTest also take care of AttributedCharacterIterator material.
      FieldPosition fp = new FieldPosition(NumberFormat.Field.CURRENCY);
      sb.populateFieldPosition(fp, 0);
      assertEquals(str.length(), fp.getBeginIndex());
      assertEquals(str.length() * 2, fp.getEndIndex());

      if (str.length() > 0) {
        sb.insertCodePoint(2, 100, NumberFormat.Field.INTEGER);
        fields = sb.toFieldArray();
        assertEquals(str.length() * 2 + 1, fields.length);
        assertEquals(fields[2], NumberFormat.Field.INTEGER);
      }

      sb.append(new NumberStringBuilder(sb));
      sb.append(sb.toCharArray(), sb.toFieldArray());
      int numNull = 0;
      int numCurr = 0;
      int numInt = 0;
      Field[] oldFields = fields;
      fields = sb.toFieldArray();
      for (int i = 0; i < sb.length(); i++) {
        assertEquals(oldFields[i % oldFields.length], fields[i]);
        if (fields[i] == null) {
          numNull++;
        } else if (fields[i] == NumberFormat.Field.CURRENCY) {
          numCurr++;
        } else if (fields[i] == NumberFormat.Field.INTEGER) {
          numInt++;
        } else {
          throw new AssertionError("Encountered unknown field in " + str);
        }
      }
      assertEquals(str.length() * 4, numNull);
      assertEquals(numNull, numCurr);
      assertEquals(str.length() > 0 ? 4 : 0, numInt);

      NumberStringBuilder sb2 = new NumberStringBuilder();
      sb2.append(sb);
      assertTrue(sb.contentEquals(sb2));
      assertTrue(sb.contentEquals(sb2.toCharArray(), sb2.toFieldArray()));

      sb2.insertCodePoint(0, 50, NumberFormat.Field.FRACTION);
      assertTrue(!sb.contentEquals(sb2));
      assertTrue(!sb.contentEquals(sb2.toCharArray(), sb2.toFieldArray()));
    }
  }

  @Test
  public void testUnlimitedCapacity() {
    NumberStringBuilder builder = new NumberStringBuilder();
    // The builder should never fail upon repeated appends.
    for (int i = 0; i < 1000; i++) {
      assertEquals(builder.length(), i);
      builder.appendCodePoint('x', null);
      assertEquals(builder.length(), i + 1);
    }
  }

  private static void assertCharSequenceEquals(CharSequence a, CharSequence b) {
    assertEquals(a.toString(), b.toString());

    assertEquals(a.length(), b.length());
    for (int i = 0; i < a.length(); i++) {
      assertEquals(a.charAt(i), b.charAt(i));
    }

    int start = Math.min(2, a.length());
    int end = Math.min(12, a.length());
    if (start != end) {
      assertCharSequenceEquals(a.subSequence(start, end), b.subSequence(start, end));
    }
  }
}
