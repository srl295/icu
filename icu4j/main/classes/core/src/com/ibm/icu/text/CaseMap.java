// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
package com.ibm.icu.text;

import java.util.Locale;

import com.ibm.icu.impl.UCaseProps;
import com.ibm.icu.lang.UCharacter;

// TODO: issues/questions
// - optimizing strategies for unstyled text: stop after number of changes or length of replacement?

/**
 * Low-level case mapping options and methods. Immutable.
 *
 * @draft ICU 59
 * @provisional This API might change or be removed in a future release.
 */
public abstract class CaseMap {
    /**
     * @internal
     * @deprecated This API is ICU internal only.
     */
    @Deprecated
    protected int internalOptions;

    private CaseMap(int opt) { internalOptions = opt; }

    /**
     * @return Lowercasing object with default options.
     * @draft ICU 59
     * @provisional This API might change or be removed in a future release.
     */
    public static Lower toLower() { return Lower.DEFAULT; }
    /**
     * @return Uppercasing object with default options.
     * @draft ICU 59
     * @provisional This API might change or be removed in a future release.
     */
    public static Upper toUpper() { return Upper.DEFAULT; }
    /**
     * @return Titlecasing object with default options.
     * @draft ICU 59
     * @provisional This API might change or be removed in a future release.
     */
    public static Title toTitle() { return Title.DEFAULT; }
    /**
     * @return Case folding object with default options.
     * @draft ICU 59
     * @provisional This API might change or be removed in a future release.
     */
    public static Fold fold() { return Fold.DEFAULT; }

    /**
     * Omit unchanged text when case-mapping with {@link Edits}.
     *
     * @return an options object with this option.
     * @draft ICU 59
     * @provisional This API might change or be removed in a future release.
     */
    public abstract CaseMap omitUnchangedText();

    /**
     * Lowercasing options and methods. Immutable.
     *
     * @see #toLower()
     * @draft ICU 59
     * @provisional This API might change or be removed in a future release.
     */
    public static final class Lower extends CaseMap {
        private static final Lower DEFAULT = new Lower(0);
        private static final Lower OMIT_UNCHANGED = new Lower(OMIT_UNCHANGED_TEXT);
        private Lower(int opt) { super(opt); }

        /**
         * {@inheritDoc}
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
        @Override
        public Lower omitUnchangedText() {
            return OMIT_UNCHANGED;
        }

        /**
         * Lowercases a string and optionally records edits (see {@link #omitUnchangedText}).
         * Casing is locale-dependent and context-sensitive.
         * The result may be longer or shorter than the original.
         *
         * @param locale    The locale ID. Can be null for {@link Locale#getDefault}.
         * @param src       The original string.
         * @param dest      A buffer for the result string. Must not be null.
         * @param edits     Records edits for index mapping, working with styled text,
         *                  and getting only changes (if any).
         *                  This function calls edits.reset() first. edits can be null.
         * @return dest with the result string (or only changes) appended.
         *
         * @see UCharacter#toLowerCase(Locale, String)
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
         public <A extends Appendable> A apply(
                 Locale locale, CharSequence src, A dest, Edits edits) {
             if (locale == null) {
                 locale = Locale.getDefault();
             }
             int caseLocale = UCaseProps.getCaseLocale(locale);
             // TODO: remove package path
             return com.ibm.icu.impl.CaseMap.toLower(
                     caseLocale, internalOptions, src, dest, edits);
         }
    }

    /**
     * Uppercasing options and methods. Immutable.
     *
     * @see #toUpper()
     * @draft ICU 59
     * @provisional This API might change or be removed in a future release.
     */
    public static final class Upper extends CaseMap {
        private static final Upper DEFAULT = new Upper(0);
        private static final Upper OMIT_UNCHANGED = new Upper(OMIT_UNCHANGED_TEXT);
        private Upper(int opt) { super(opt); }

        /**
         * {@inheritDoc}
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
        @Override
        public Upper omitUnchangedText() {
            return OMIT_UNCHANGED;
        }

        /**
         * Uppercases a string and optionally records edits (see {@link #omitUnchangedText}).
         * Casing is locale-dependent and context-sensitive.
         * The result may be longer or shorter than the original.
         *
         * @param locale    The locale ID. Can be null for {@link Locale#getDefault}.
         * @param src       The original string.
         * @param dest      A buffer for the result string. Must not be null.
         * @param edits     Records edits for index mapping, working with styled text,
         *                  and getting only changes (if any).
         *                  This function calls edits.reset() first. edits can be null.
         * @return dest with the result string (or only changes) appended.
         *
         * @see UCharacter#toUpperCase(Locale, String)
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
         public <A extends Appendable> A apply(
                 Locale locale, CharSequence src, A dest, Edits edits) {
             return null;
         }
    }

    /**
     * Titlecasing options and methods. Immutable.
     *
     * @see #toTitle()
     * @draft ICU 59
     * @provisional This API might change or be removed in a future release.
     */
    public static final class Title extends CaseMap {
        private static final Title DEFAULT = new Title(0);
        private static final Title OMIT_UNCHANGED = new Title(OMIT_UNCHANGED_TEXT);
        private Title(int opt) { super(opt); }

        /**
         * {@inheritDoc}
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
        @Override
        public Title omitUnchangedText() {
            if (internalOptions == 0 || internalOptions == OMIT_UNCHANGED_TEXT) {
                return OMIT_UNCHANGED;
            }
            return new Title(internalOptions | OMIT_UNCHANGED_TEXT);
        }

        /**
         * Do not lowercase non-initial parts of words when titlecasing.
         *
         * <p>By default, titlecasing will titlecase the first cased character
         * of a word and lowercase all other characters.
         * With this option, the other characters will not be modified.
         *
         * @return an options object with this option.
         * @see UCharacter#TITLECASE_NO_LOWERCASE
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
        public Title noLowercase() {
            return new Title(internalOptions | UCharacter.TITLECASE_NO_LOWERCASE);
        }

        // TODO: update references to the Unicode Standard for recent version
        /**
         * Do not adjust the titlecasing indexes from BreakIterator::next() indexes;
         * titlecase exactly the characters at breaks from the iterator.
         *
         * <p>By default, titlecasing will take each break iterator index,
         * adjust it by looking for the next cased character, and titlecase that one.
         * Other characters are lowercased.
         *
         * <p>This follows Unicode 4 &amp; 5 section 3.13 Default Case Operations:
         *
         * R3  toTitlecase(X): Find the word boundaries based on Unicode Standard Annex
         * #29, "Text Boundaries." Between each pair of word boundaries, find the first
         * cased character F. If F exists, map F to default_title(F); then map each
         * subsequent character C to default_lower(C).
         *
         * @return an options object with this option.
         * @see UCharacter#TITLECASE_NO_BREAK_ADJUSTMENT
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
        public Title noBreakAdjustment() {
            return new Title(internalOptions | UCharacter.TITLECASE_NO_BREAK_ADJUSTMENT);
        }

        /**
         * Titlecases a string and optionally records edits (see {@link #omitUnchangedText}).
         * Casing is locale-dependent and context-sensitive.
         * The result may be longer or shorter than the original.
         *
         * Titlecasing uses a break iterator to find the first characters of words
         * that are to be titlecased. It titlecases those characters and lowercases
         * all others. (This can be modified with options bits.)
         *
         * @param locale    The locale ID. Can be null for {@link Locale#getDefault}.
         * @param iter      A break iterator to find the first characters of words that are to be titlecased.
         *                  It is set to the source string (setText())
         *                  and used one or more times for iteration (first() and next()).
         *                  If null, then a word break iterator for the locale is used
         *                  (or something equivalent).
         * @param src       The original string.
         * @param dest      A buffer for the result string. Must not be null.
         * @param edits     Records edits for index mapping, working with styled text,
         *                  and getting only changes (if any).
         *                  This function calls edits.reset() first. edits can be null.
         * @return dest with the result string (or only changes) appended.
         *
         * @see UCharacter#toTitleCase(Locale, String, BreakIterator, int)
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
         public <A extends Appendable> A apply(
                 Locale locale, BreakIterator iter, CharSequence src, A dest, Edits edits) {
             return null;
         }
    }

    /**
     * Case folding options and methods. Immutable.
     *
     * @see #fold()
     * @draft ICU 59
     * @provisional This API might change or be removed in a future release.
     */
    public static final class Fold extends CaseMap {
        private static final Fold DEFAULT = new Fold(0);
        private static final Fold TURKIC = new Fold(UCharacter.FOLD_CASE_EXCLUDE_SPECIAL_I);
        private static final Fold OMIT_UNCHANGED = new Fold(OMIT_UNCHANGED_TEXT);
        private static final Fold TURKIC_OMIT_UNCHANGED = new Fold(
                UCharacter.FOLD_CASE_EXCLUDE_SPECIAL_I | OMIT_UNCHANGED_TEXT);
        private Fold(int opt) { super(opt); }

        /**
         * {@inheritDoc}
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
        @Override
        public Fold omitUnchangedText() {
            return (internalOptions & UCharacter.FOLD_CASE_EXCLUDE_SPECIAL_I) == 0 ?
                    OMIT_UNCHANGED : TURKIC_OMIT_UNCHANGED;
        }

        /**
         * Handle dotted I and dotless i appropriately for Turkic languages (tr, az).
         *
         * <p>Uses the Unicode CaseFolding.txt mappings marked with 'T' that
         * are to be excluded for default mappings and
         * included for the Turkic-specific mappings.
         *
         * @return an options object with this option.
         * @see UCharacter#FOLD_CASE_EXCLUDE_SPECIAL_I
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
        public Fold turkic() {
            return (internalOptions & OMIT_UNCHANGED_TEXT) == 0 ?
                    TURKIC : TURKIC_OMIT_UNCHANGED;
        }

        /**
         * Case-folds a string and optionally records edits (see {@link #omitUnchangedText}).
         *
         * Case-folding is locale-independent and not context-sensitive,
         * but there is an option for whether to include or exclude mappings for dotted I
         * and dotless i that are marked with 'T' in CaseFolding.txt.
         *
         * The result may be longer or shorter than the original.
         *
         * @param src       The original string.
         * @param dest      A buffer for the result string. Must not be null.
         * @param edits     Records edits for index mapping, working with styled text,
         *                  and getting only changes (if any).
         *                  This function calls edits.reset() first. edits can be null.
         * @return dest with the result string (or only changes) appended.
         *
         * @see UCharacter#foldCase(String, int)
         * @draft ICU 59
         * @provisional This API might change or be removed in a future release.
         */
         public <A extends Appendable> A apply(CharSequence src, A dest, Edits edits) {
             return null;
         }
    }

    /**
     * Omit unchanged text when case-mapping with Edits.
     *
     * @internal
     * @deprecated This API is ICU internal only.
     */
    @Deprecated
    public static final int OMIT_UNCHANGED_TEXT = 0x4000;  // TODO: move to a non-public class
}
