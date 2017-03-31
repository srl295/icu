// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
package com.ibm.icu.impl.number;

import com.ibm.icu.text.DecimalFormatSymbols;
import com.ibm.icu.text.NumberFormat.Field;

/**
 * Performs manipulations on affix patterns: the prefix and suffix strings associated with a decimal
 * format pattern. For example:
 *
 * <table>
 * <tr><th>Affix Pattern</th><th>Example Unescaped (Formatted) String</th></tr>
 * <tr><td>abc</td><td>abc</td></tr>
 * <tr><td>ab-</td><td>ab−</td></tr>
 * <tr><td>ab'-'</td><td>ab-</td></tr>
 * <tr><td>ab''</td><td>ab'</td></tr>
 * </table>
 *
 * To manually iterate over tokens in a literal string, use the following pattern, which is designed
 * to be efficient.
 *
 * <pre>
 * long tag = 0L;
 * while (AffixPatternUtils.hasNext(tag, patternString)) {
 *   tag = AffixPatternUtils.nextToken(tag, patternString);
 *   int typeOrCp = AffixPatternUtils.getTypeOrCp(tag);
 *   switch (typeOrCp) {
 *     case AffixPatternUtils.TYPE_MINUS_SIGN:
 *       // Current token is a minus sign.
 *       break;
 *     case AffixPatternUtils.TYPE_PLUS_SIGN:
 *       // Current token is a plus sign.
 *       break;
 *     case AffixPatternUtils.TYPE_PERCENT:
 *       // Current token is a percent sign.
 *       break;
 *     case AffixPatternUtils.TYPE_PERMILLE:
 *       // Current token is a permille sign.
 *       break;
 *     case AffixPatternUtils.TYPE_CURRENCY_SINGLE:
 *       // Current token is a single currency sign.
 *       break;
 *     case AffixPatternUtils.TYPE_CURRENCY_DOUBLE:
 *       // Current token is a double currency sign.
 *       break;
 *     case AffixPatternUtils.TYPE_CURRENCY_TRIPLE:
 *       // Current token is a triple currency sign.
 *       break;
 *     case AffixPatternUtils.TYPE_CURRENCY_OVERFLOW:
 *       // Current token has four or more currency signs.
 *       break;
 *     default:
 *       // Current token is an arbitrary code point.
 *       // The variable typeOrCp is the code point.
 *       break;
 *   }
 * }
 * </pre>
 */
public class AffixPatternUtils {

  private static final int STATE_BASE = 0;
  private static final int STATE_FIRST_QUOTE = 1;
  private static final int STATE_INSIDE_QUOTE = 2;
  private static final int STATE_AFTER_QUOTE = 3;
  private static final int STATE_FIRST_CURR = 4;
  private static final int STATE_SECOND_CURR = 5;
  private static final int STATE_THIRD_CURR = 6;
  private static final int STATE_OVERFLOW_CURR = 7;

  private static final int TYPE_CODEPOINT = 0;

  /** Represents a minus sign symbol '-'. */
  public static final int TYPE_MINUS_SIGN = -1;

  /** Represents a plus sign symbol '+'. */
  public static final int TYPE_PLUS_SIGN = -2;

  /** Represents a percent sign symbol '%'. */
  public static final int TYPE_PERCENT = -3;

  /** Represents a permille sign symbol '‰'. */
  public static final int TYPE_PERMILLE = -4;

  /** Represents a single currency symbol '¤'. */
  public static final int TYPE_CURRENCY_SINGLE = -5;

  /** Represents a double currency symbol '¤¤'. */
  public static final int TYPE_CURRENCY_DOUBLE = -6;

  /** Represents a triple currency symbol '¤¤¤'. */
  public static final int TYPE_CURRENCY_TRIPLE = -7;

  /** Represents a sequence of four or more currency symbols. */
  public static final int TYPE_CURRENCY_OVERFLOW = -15;

  /**
   * Estimates the number of code points present in an unescaped version of the affix pattern string
   * (one that would be returned by {@link #unescape}), assuming that all interpolated symbols
   * consume one code point and that currencies consume as many code points as their symbol width.
   * Used for computing padding width.
   *
   * @param patternString The original string whose width will be estimated.
   * @return The length of the unescaped string.
   */
  public static int unescapedLength(CharSequence patternString) {
    if (patternString == null) return 0;
    int state = STATE_BASE;
    int offset = 0;
    int length = 0;
    for (; offset < patternString.length(); ) {
      int cp = Character.codePointAt(patternString, offset);

      switch (state) {
        case STATE_BASE:
          if (cp == '\'') {
            // First quote
            state = STATE_FIRST_QUOTE;
          } else {
            // Unquoted symbol
            length++;
          }
          break;
        case STATE_FIRST_QUOTE:
          if (cp == '\'') {
            // Repeated quote
            length++;
            state = STATE_BASE;
          } else {
            // Quoted code point
            length++;
            state = STATE_INSIDE_QUOTE;
          }
          break;
        case STATE_INSIDE_QUOTE:
          if (cp == '\'') {
            // End of quoted sequence
            state = STATE_AFTER_QUOTE;
          } else {
            // Quoted code point
            length++;
          }
          break;
        case STATE_AFTER_QUOTE:
          if (cp == '\'') {
            // Double quote inside of quoted sequence
            length++;
            state = STATE_INSIDE_QUOTE;
          } else {
            // Unquoted symbol
            length++;
          }
          break;
        default:
          throw new AssertionError();
      }

      offset += Character.charCount(cp);
    }

    switch (state) {
      case STATE_FIRST_QUOTE:
      case STATE_INSIDE_QUOTE:
        throw new IllegalArgumentException("Unterminated quote: \"" + patternString + "\"");
      default:
        break;
    }

    return length;
  }

  /**
   * Takes a string and escapes (quotes) characters that have special meaning in the affix pattern
   * syntax. This function does not reverse-lookup symbols.
   *
   * <p>Example input: "-$x"; example output: "'-'$x"
   *
   * @param input The string to be escaped.
   * @param output The string builder to which to append the escaped string.
   * @return The number of chars (UTF-16 code units) appended to the output.
   */
  public static int escape(CharSequence input, StringBuilder output) {
    if (input == null) return 0;
    int state = STATE_BASE;
    int offset = 0;
    int startLength = output.length();
    for (; offset < input.length(); ) {
      int cp = Character.codePointAt(input, offset);

      switch (cp) {
        case '\'':
          output.append("''");
          break;

        case '-':
        case '+':
        case '%':
        case '‰':
        case '¤':
          if (state == STATE_BASE) {
            output.append('\'');
            output.appendCodePoint(cp);
            state = STATE_INSIDE_QUOTE;
          } else {
            output.appendCodePoint(cp);
          }
          break;

        default:
          if (state == STATE_INSIDE_QUOTE) {
            output.append('\'');
            output.appendCodePoint(cp);
            state = STATE_BASE;
          } else {
            output.appendCodePoint(cp);
          }
          break;
      }
      offset += Character.charCount(cp);
    }

    if (state == STATE_INSIDE_QUOTE) {
      output.append('\'');
    }

    return output.length() - startLength;
  }

  /**
   * Executes the unescape state machine. Replaces the unquoted characters "-", "+", "%", and "‰"
   * with their localized equivalents. Replaces "¤", "¤¤", and "¤¤¤" with the three argument
   * strings.
   *
   * <p>Example input: "'-'¤x"; example output: "-$x"
   *
   * @param affixPattern The original string to be unescaped.
   * @param symbols An instance of {@link DecimalFormatSymbols} for the locale of interest.
   * @param currency1 The string to replace "¤".
   * @param currency2 The string to replace "¤¤".
   * @param currency3 The string to replace "¤¤¤".
   * @param minusSign The string to replace "-". If null, symbols.getMinusSignString() is used.
   * @param output The {@link NumberStringBuilder} to which the result will be appended.
   */
  public static void unescape(
      CharSequence affixPattern,
      DecimalFormatSymbols symbols,
      String currency1,
      String currency2,
      String currency3,
      String minusSign,
      NumberStringBuilder output) {
    if (affixPattern == null || affixPattern.length() == 0) return;
    if (minusSign == null) minusSign = symbols.getMinusSignString();
    long tag = 0L;
    while (hasNext(tag, affixPattern)) {
      tag = nextToken(tag, affixPattern);
      int typeOrCp = getTypeOrCp(tag);
      switch (typeOrCp) {
        case TYPE_MINUS_SIGN:
          output.append(minusSign, Field.SIGN);
          break;
        case TYPE_PLUS_SIGN:
          output.append(symbols.getPlusSignString(), Field.SIGN);
          break;
        case TYPE_PERCENT:
          output.append(symbols.getPercentString(), Field.PERCENT);
          break;
        case TYPE_PERMILLE:
          output.append(symbols.getPerMillString(), Field.PERMILLE);
          break;
        case TYPE_CURRENCY_SINGLE:
          output.append(currency1, Field.CURRENCY);
          break;
        case TYPE_CURRENCY_DOUBLE:
          output.append(currency2, Field.CURRENCY);
          break;
        case TYPE_CURRENCY_TRIPLE:
          output.append(currency3, Field.CURRENCY);
          break;
        case TYPE_CURRENCY_OVERFLOW:
          output.append("\uFFFD", Field.CURRENCY);
          break;
        default:
          output.appendCodePoint(typeOrCp, null);
          break;
      }
    }
  }

  /**
   * Checks whether the given affix pattern contains at least one token of the given type, which is
   * one of the constants "TYPE_" in {@link AffixPatternUtils}.
   *
   * @param affixPattern The affix pattern to check.
   * @param type The token type.
   * @return true if the affix pattern contains the given token type; false otherwise.
   */
  public static boolean containsType(CharSequence affixPattern, int type) {
    if (affixPattern == null || affixPattern.length() == 0) return false;
    long tag = 0L;
    while (hasNext(tag, affixPattern)) {
      tag = nextToken(tag, affixPattern);
      if (getTypeOrCp(tag) == type) {
        return true;
      }
    }
    return false;
  }

  /**
   * Checks whether the specified affix pattern has any unquoted currency symbols ("¤").
   *
   * @param affixPattern The string to check for currency symbols.
   * @return true if the literal has at least one unquoted currency symbol; false otherwise.
   */
  public static boolean hasCurrencySymbols(CharSequence affixPattern) {
    if (affixPattern == null || affixPattern.length() == 0) return false;
    long tag = 0L;
    while (hasNext(tag, affixPattern)) {
      tag = nextToken(tag, affixPattern);
      int typeOrCp = getTypeOrCp(tag);
      if (typeOrCp == AffixPatternUtils.TYPE_CURRENCY_SINGLE
          || typeOrCp == AffixPatternUtils.TYPE_CURRENCY_DOUBLE
          || typeOrCp == AffixPatternUtils.TYPE_CURRENCY_TRIPLE
          || typeOrCp == AffixPatternUtils.TYPE_CURRENCY_OVERFLOW) {
        return true;
      }
    }
    return false;
  }

  /**
   * Replaces all occurrences of tokens with the given type with the given replacement char.
   *
   * @param affixPattern The source affix pattern (does not get modified).
   * @param type The token type.
   * @param replacementChar The char to substitute in place of chars of the given token type.
   * @return A string containing the new affix pattern.
   */
  public static String replaceType(CharSequence affixPattern, int type, char replacementChar) {
    if (affixPattern == null || affixPattern.length() == 0) return "";
    char[] chars = affixPattern.toString().toCharArray();
    long tag = 0L;
    while (hasNext(tag, affixPattern)) {
      tag = nextToken(tag, affixPattern);
      if (getTypeOrCp(tag) == type) {
        int offset = getOffset(tag);
        chars[offset - 1] = replacementChar;
      }
    }
    return new String(chars);
  }

  /**
   * Returns the next token from the affix pattern.
   *
   * @param tag A bitmask used for keeping track of state from token to token. The initial value
   *     should be 0L.
   * @param patternString The affix pattern.
   * @return The bitmask tag to pass to the next call of this method to retrieve the following token
   *     (never negative), or -1 if there were no more tokens in the affix pattern.
   * @see #hasNext
   */
  public static long nextToken(long tag, CharSequence patternString) {
    int offset = getOffset(tag);
    int state = getState(tag);
    for (; offset < patternString.length(); ) {
      int cp = Character.codePointAt(patternString, offset);
      int count = Character.charCount(cp);

      switch (state) {
        case STATE_BASE:
          switch (cp) {
            case '\'':
              state = STATE_FIRST_QUOTE;
              offset += count;
              // continue to the next code point
              break;
            case '-':
              return makeTag(offset + count, TYPE_MINUS_SIGN, STATE_BASE, 0);
            case '+':
              return makeTag(offset + count, TYPE_PLUS_SIGN, STATE_BASE, 0);
            case '%':
              return makeTag(offset + count, TYPE_PERCENT, STATE_BASE, 0);
            case '‰':
              return makeTag(offset + count, TYPE_PERMILLE, STATE_BASE, 0);
            case '¤':
              state = STATE_FIRST_CURR;
              offset += count;
              // continue to the next code point
              break;
            default:
              return makeTag(offset + count, TYPE_CODEPOINT, STATE_BASE, cp);
          }
          break;
        case STATE_FIRST_QUOTE:
          if (cp == '\'') {
            return makeTag(offset + count, TYPE_CODEPOINT, STATE_BASE, cp);
          } else {
            return makeTag(offset + count, TYPE_CODEPOINT, STATE_INSIDE_QUOTE, cp);
          }
        case STATE_INSIDE_QUOTE:
          if (cp == '\'') {
            state = STATE_AFTER_QUOTE;
            offset += count;
            // continue to the next code point
            break;
          } else {
            return makeTag(offset + count, TYPE_CODEPOINT, STATE_INSIDE_QUOTE, cp);
          }
        case STATE_AFTER_QUOTE:
          if (cp == '\'') {
            return makeTag(offset + count, TYPE_CODEPOINT, STATE_INSIDE_QUOTE, cp);
          } else {
            state = STATE_BASE;
            // re-evaluate this code point
            break;
          }
        case STATE_FIRST_CURR:
          if (cp == '¤') {
            state = STATE_SECOND_CURR;
            offset += count;
            // continue to the next code point
            break;
          } else {
            return makeTag(offset, TYPE_CURRENCY_SINGLE, STATE_BASE, 0);
          }
        case STATE_SECOND_CURR:
          if (cp == '¤') {
            state = STATE_THIRD_CURR;
            offset += count;
            // continue to the next code point
            break;
          } else {
            return makeTag(offset, TYPE_CURRENCY_DOUBLE, STATE_BASE, 0);
          }
        case STATE_THIRD_CURR:
          if (cp == '¤') {
            state = STATE_OVERFLOW_CURR;
            offset += count;
            // continue to the next code point
            break;
          } else {
            return makeTag(offset, TYPE_CURRENCY_TRIPLE, STATE_BASE, 0);
          }
        case STATE_OVERFLOW_CURR:
          if (cp == '¤') {
            offset += count;
            // continue to the next code point and loop back to this state
            break;
          } else {
            return makeTag(offset, TYPE_CURRENCY_OVERFLOW, STATE_BASE, 0);
          }
        default:
          throw new AssertionError();
      }
    }
    // End of string
    switch (state) {
      case STATE_BASE:
        // No more tokens in string.
        return -1L;
      case STATE_FIRST_QUOTE:
      case STATE_INSIDE_QUOTE:
        // For consistent behavior with the JDK and ICU 58, throw an exception here.
        throw new IllegalArgumentException(
            "Unterminated quote in pattern affix: \"" + patternString + "\"");
      case STATE_AFTER_QUOTE:
        // No more tokens in string.
        return -1L;
      case STATE_FIRST_CURR:
        return makeTag(offset, TYPE_CURRENCY_SINGLE, STATE_BASE, 0);
      case STATE_SECOND_CURR:
        return makeTag(offset, TYPE_CURRENCY_DOUBLE, STATE_BASE, 0);
      case STATE_THIRD_CURR:
        return makeTag(offset, TYPE_CURRENCY_TRIPLE, STATE_BASE, 0);
      case STATE_OVERFLOW_CURR:
        return makeTag(offset, TYPE_CURRENCY_OVERFLOW, STATE_BASE, 0);
      default:
        throw new AssertionError();
    }
  }

  /**
   * Returns whether the affix pattern string has any more tokens to be retrieved from a call to
   * {@link #nextToken}.
   *
   * @param tag The bitmask tag of the previous token, as returned by {@link #nextToken}.
   * @param string The affix pattern.
   * @return true if there are more tokens to consume; false otherwise.
   */
  public static boolean hasNext(long tag, CharSequence string) {
    assert tag >= 0;
    int state = getState(tag);
    int offset = getOffset(tag);
    // Special case: the last character in string is an end quote.
    if (state == STATE_INSIDE_QUOTE
        && offset == string.length() - 1
        && string.charAt(offset) == '\'') {
      return false;
    } else if (state != STATE_BASE) {
      return true;
    } else {
      return offset < string.length();
    }
  }

  /**
   * This function helps determine the identity of the token consumed by {@link #nextToken}.
   * Converts from a bitmask tag, based on a call to {@link #nextToken}, to its corresponding symbol
   * type or code point.
   *
   * @param tag The bitmask tag of the current token, as returned by {@link #nextToken}.
   * @return If less than zero, a symbol type corresponding to one of the <code>TYPE_</code>
   *     constants, such as {@link #TYPE_MINUS_SIGN}. If greater than or equal to zero, a literal
   *     code point.
   */
  public static int getTypeOrCp(long tag) {
    assert tag >= 0;
    int type = getType(tag);
    return (type == 0) ? getCodePoint(tag) : -type;
  }

  private static long makeTag(int offset, int type, int state, int cp) {
    long tag = 0L;
    tag |= offset;
    tag |= (-(long) type) << 32;
    tag |= ((long) state) << 36;
    tag |= ((long) cp) << 40;
    assert tag >= 0;
    return tag;
  }

  static int getOffset(long tag) {
    return (int) (tag & 0xffffffff);
  }

  static int getType(long tag) {
    return (int) ((tag >>> 32) & 0xf);
  }

  static int getState(long tag) {
    return (int) ((tag >>> 36) & 0xf);
  }

  static int getCodePoint(long tag) {
    return (int) (tag >>> 40);
  }
}
