/*
**********************************************************************
*   Copyright (C) 1999-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*   Date        Name        Description
*   11/17/99    aliu        Creation.
**********************************************************************
*/
#include "cmemory.h"
#include "cstring.h"
#include "hash.h"
#include "mutex.h"
#include "rbt_data.h"
#include "rbt_pars.h"
#include "unicode/cpdtrans.h"
#include "unicode/hangjamo.h"
#include "unicode/hextouni.h"
#include "unicode/jamohang.h"
#include "unicode/locid.h"
#include "unicode/msgfmt.h"
#include "unicode/name2uni.h"
#include "unicode/nortrans.h"
#include "unicode/nultrans.h"
#include "unicode/putil.h"
#include "unicode/rep.h"
#include "unicode/remtrans.h"
#include "unicode/resbund.h"
#include "unicode/titletrn.h"
#include "unicode/tolowtrn.h"
#include "unicode/toupptrn.h"
#include "unicode/translit.h"
#include "unicode/uni2name.h"
#include "unicode/unicode.h"
#include "unicode/unifilt.h"
#include "unicode/uniset.h"
#include "unicode/unitohex.h"

const UChar Transliterator::ID_SEP   = 0x002D; /*-*/
const UChar Transliterator::ID_DELIM = 0x003B; /*;*/

static Hashtable _cache(TRUE); // TRUE = keys are case insensitive
static Hashtable _internalCache(TRUE); // TRUE = keys are case insensitive

/**
 * Cache of public system transliterators.  Keys are UnicodeString
 * names, values are CacheEntry objects.
 */
Hashtable* Transliterator::cache = &_cache;

/**
 * Like 'cache', but IDs are not public.  Internal transliterators are
 * combined together and aliased to public IDs.
 */
Hashtable* Transliterator::internalCache = &_internalCache;

/**
 * The mutex controlling access to the cache.
 */
UMTX Transliterator::cacheMutex = NULL;

/**
 * When set to TRUE, the cache has been initialized.  Any code must
 * check this boolean before accessing the cache, and if the boolean
 * is FALSE, it must call initializeCache().  We do this form of lazy
 * evaluation for two reasons: (1) so we don't initialize if we don't
 * have to (i.e., if no one is using Transliterator, but has included
 * the code as part of a shared library, and (2) to avoid static
 * intialization problems.
 */
UBool Transliterator::cacheInitialized = FALSE;

/**
 * Vector of registered IDs.
 */
UVector Transliterator::cacheIDs;

/**
 * Prefix for resource bundle key for the display name for a
 * transliterator.  The ID is appended to this to form the key.
 * The resource bundle value should be a String.
 */
const char* Transliterator::RB_DISPLAY_NAME_PREFIX = "%Translit%%";

/**
 * Prefix for resource bundle key for the display name for a
 * transliterator SCRIPT.  The ID is appended to this to form the key.
 * The resource bundle value should be a String.
 */
const char* Transliterator::RB_SCRIPT_DISPLAY_NAME_PREFIX = "%Translit%";

/**
 * Resource bundle key for display name pattern.
 * The resource bundle value should be a String forming a
 * MessageFormat pattern, e.g.:
 * "{0,choice,0#|1#{1} Transliterator|2#{1} to {2} Transliterator}".
 */
const char* Transliterator::RB_DISPLAY_NAME_PATTERN =
    "TransliteratorNamePattern";

/**
 * Resource bundle key for the list of RuleBasedTransliterator IDs.
 * The resource bundle value should be a String[] with each element
 * being a valid ID.  The ID will be appended to RB_RULE_BASED_PREFIX
 * to obtain the class name in which the RB_RULE key will be sought.
 */
const char* Transliterator::RB_RULE_BASED_IDS =
    "RuleBasedTransliteratorIDs";

/**
 * Resource bundle key for the RuleBasedTransliterator rule.
 */
const char* Transliterator::RB_RULE = "Rule";

/**
 * Class identifier for subclasses of Transliterator that do not
 * define their class (anonymous subclasses).
 */
char Transliterator::fgClassID = 0; // Value is irrelevant

/**
 * Default constructor.
 * @param theID the string identifier for this transliterator
 * @param theFilter the filter.  Any character for which
 * <tt>filter.contains()</tt> returns <tt>FALSE</tt> will not be
 * altered by this transliterator.  If <tt>filter</tt> is
 * <tt>null</tt> then no filtering is applied.
 */
Transliterator::Transliterator(const UnicodeString& theID,
                               UnicodeFilter* adoptedFilter) :
    ID(theID), filter(adoptedFilter),
    maximumContextLength(0) {}

/**
 * Destructor.
 */
Transliterator::~Transliterator() {
    delete filter;
}

/**
 * Copy constructor.
 */
Transliterator::Transliterator(const Transliterator& other) :
    ID(other.ID), filter(0),
    maximumContextLength(other.maximumContextLength) {
    if (other.filter != 0) {
        // We own the filter, so we must have our own copy
        filter = other.filter->clone();
    }
}

/**
 * Assignment operator.
 */
Transliterator& Transliterator::operator=(const Transliterator& other) {
    ID = other.ID;
    maximumContextLength = other.maximumContextLength;
    // MUST go through adoptFilter in case latter is overridden
    adoptFilter((other.filter == 0) ? 0 : other.filter->clone());
    return *this;
}

/**
 * Transliterates a segment of a string.  <code>Transliterator</code> API.
 * @param text the string to be transliterated
 * @param start the beginning index, inclusive; <code>0 <= start
 * <= limit</code>.
 * @param limit the ending index, exclusive; <code>start <= limit
 * <= text.length()</code>.
 * @return the new limit index
 */
int32_t Transliterator::transliterate(Replaceable& text,
                                      int32_t start, int32_t limit) const {

    UTransPosition offsets;
    offsets.contextStart= start;
    offsets.contextLimit = limit;
    offsets.start = start;
    offsets.limit = limit;
    handleTransliterate(text, offsets, FALSE);
    return offsets.limit;
}

/**
 * Transliterates an entire string in place. Convenience method.
 * @param text the string to be transliterated
 */
void Transliterator::transliterate(Replaceable& text) const {
    transliterate(text, 0, text.length());
}

/**
 * Transliterates the portion of the text buffer that can be
 * transliterated unambiguosly after new text has been inserted,
 * typically as a result of a keyboard event.  The new text in
 * <code>insertion</code> will be inserted into <code>text</code>
 * at <code>index.contextLimit</code>, advancing
 * <code>index.contextLimit</code> by <code>insertion.length()</code>.
 * Then the transliterator will try to transliterate characters of
 * <code>text</code> between <code>index.start</code> and
 * <code>index.contextLimit</code>.  Characters before
 * <code>index.start</code> will not be changed.
 *
 * <p>Upon return, values in <code>index</code> will be updated.
 * <code>index.contextStart</code> will be advanced to the first
 * character that future calls to this method will read.
 * <code>index.start</code> and <code>index.contextLimit</code> will
 * be adjusted to delimit the range of text that future calls to
 * this method may change.
 *
 * <p>Typical usage of this method begins with an initial call
 * with <code>index.contextStart</code> and <code>index.contextLimit</code>
 * set to indicate the portion of <code>text</code> to be
 * transliterated, and <code>index.start == index.contextStart</code>.
 * Thereafter, <code>index</code> can be used without
 * modification in future calls, provided that all changes to
 * <code>text</code> are made via this method.
 *
 * <p>This method assumes that future calls may be made that will
 * insert new text into the buffer.  As a result, it only performs
 * unambiguous transliterations.  After the last call to this
 * method, there may be untransliterated text that is waiting for
 * more input to resolve an ambiguity.  In order to perform these
 * pending transliterations, clients should call {@link
 * #finishKeyboardTransliteration} after the last call to this
 * method has been made.
 * 
 * @param text the buffer holding transliterated and untransliterated text
 * @param index an array of three integers.
 *
 * <ul><li><code>index.contextStart</code>: the beginning index,
 * inclusive; <code>0 <= index.contextStart <= index.contextLimit</code>.
 *
 * <li><code>index.contextLimit</code>: the ending index, exclusive;
 * <code>index.contextStart <= index.contextLimit <= text.length()</code>.
 * <code>insertion</code> is inserted at
 * <code>index.contextLimit</code>.
 *
 * <li><code>index.start</code>: the next character to be
 * considered for transliteration; <code>index.contextStart <=
 * index.start <= index.contextLimit</code>.  Characters before
 * <code>index.start</code> will not be changed by future calls
 * to this method.</ul>
 *
 * @param insertion text to be inserted and possibly
 * transliterated into the translation buffer at
 * <code>index.contextLimit</code>.  If <code>null</code> then no text
 * is inserted.
 * @see #START
 * @see #LIMIT
 * @see #CURSOR
 * @see #handleTransliterate
 * @exception IllegalArgumentException if <code>index</code>
 * is invalid
 */
void Transliterator::transliterate(Replaceable& text,
                                   UTransPosition& index,
                                   const UnicodeString& insertion,
                                   UErrorCode &status) const {
    _transliterate(text, index, &insertion, status);
}

/**
 * Transliterates the portion of the text buffer that can be
 * transliterated unambiguosly after a new character has been
 * inserted, typically as a result of a keyboard event.  This is a
 * convenience method; see {@link
 * #transliterate(Replaceable, int[], String)} for details.
 * @param text the buffer holding transliterated and
 * untransliterated text
 * @param index an array of three integers.  See {@link
 * #transliterate(Replaceable, int[], String)}.
 * @param insertion text to be inserted and possibly
 * transliterated into the translation buffer at
 * <code>index.contextLimit</code>.
 * @see #transliterate(Replaceable, int[], String)
 */
void Transliterator::transliterate(Replaceable& text,
                                   UTransPosition& index,
                                   UChar insertion,
                                   UErrorCode& status) const {
    UnicodeString str(insertion);
    _transliterate(text, index, &str, status);
}

/**
 * Transliterates the portion of the text buffer that can be
 * transliterated unambiguosly.  This is a convenience method; see
 * {@link #transliterate(Replaceable, int[], String)} for
 * details.
 * @param text the buffer holding transliterated and
 * untransliterated text
 * @param index an array of three integers.  See {@link
 * #transliterate(Replaceable, int[], String)}.
 * @see #transliterate(Replaceable, int[], String)
 */
void Transliterator::transliterate(Replaceable& text,
                                   UTransPosition& index,
                                   UErrorCode& status) const {
    _transliterate(text, index, 0, status);
}

/**
 * Finishes any pending transliterations that were waiting for
 * more characters.  Clients should call this method as the last
 * call after a sequence of one or more calls to
 * <code>transliterate()</code>.
 * @param text the buffer holding transliterated and
 * untransliterated text.
 * @param index the array of indices previously passed to {@link
 * #transliterate}
 */
void Transliterator::finishTransliteration(Replaceable& text,
                                           UTransPosition& index) const {
    handleTransliterate(text, index, FALSE);
}

/**
 * This internal method does keyboard transliteration.  If the
 * 'insertion' is non-null then we append it to 'text' before
 * proceeding.  This method calls through to the pure virtual
 * framework method handleTransliterate() to do the actual
 * work.
 */
void Transliterator::_transliterate(Replaceable& text,
                                    UTransPosition& index,
                                    const UnicodeString* insertion,
                                    UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return;
    }

    if (index.contextStart < 0 ||
        index.contextLimit > text.length() ||
        index.start < index.contextStart ||
        index.start > index.contextLimit) {
        status = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }

    int32_t originalStart = index.contextStart;
    if (insertion != 0) {
        text.handleReplaceBetween(index.limit, index.limit, *insertion);
        index.limit += insertion->length();
        index.contextLimit += insertion->length();
    }

    handleTransliterate(text, index, TRUE);

    index.contextStart = uprv_max(index.start - getMaximumContextLength(),
                           originalStart);
}

/**
 * Method for subclasses to use to set the maximum context length.
 * @see #getMaximumContextLength
 */
void Transliterator::setMaximumContextLength(int32_t maxContextLength) {
    maximumContextLength = maxContextLength;
}

/**
 * Returns a programmatic identifier for this transliterator.
 * If this identifier is passed to <code>getInstance()</code>, it
 * will return this object, if it has been registered.
 * @see #registerInstance
 * @see #getAvailableIDs
 */
const UnicodeString& Transliterator::getID(void) const {
    return ID;
}

/**
 * Returns a name for this transliterator that is appropriate for
 * display to the user in the default locale.  See {@link
 * #getDisplayName(Locale)} for details.
 */
UnicodeString& Transliterator::getDisplayName(const UnicodeString& ID,
                                              UnicodeString& result) {
    return getDisplayName(ID, Locale::getDefault(), result);
}

/**
 * Returns a name for this transliterator that is appropriate for
 * display to the user in the given locale.  This name is taken
 * from the locale resource data in the standard manner of the
 * <code>java.text</code> package.
 *
 * <p>If no localized names exist in the system resource bundles,
 * a name is synthesized using a localized
 * <code>MessageFormat</code> pattern from the resource data.  The
 * arguments to this pattern are an integer followed by one or two
 * strings.  The integer is the number of strings, either 1 or 2.
 * The strings are formed by splitting the ID for this
 * transliterator at the first ID_SEP.  If there is no ID_SEP, then the
 * entire ID forms the only string.
 * @param inLocale the Locale in which the display name should be
 * localized.
 * @see java.text.MessageFormat
 */
UnicodeString& Transliterator::getDisplayName(const UnicodeString& ID,
                                              const Locale& inLocale,
                                              UnicodeString& result) {
    UErrorCode status = U_ZERO_ERROR;

    ResourceBundle bundle(u_getDataDirectory(), inLocale, status);

    // Suspend checking status until later...

    // build the char* key
    char key[100];
    uprv_strcpy(key, RB_DISPLAY_NAME_PREFIX);
    int32_t length=(int32_t)uprv_strlen(RB_DISPLAY_NAME_PREFIX);
    key[length + ID.extract(0, (int32_t)(sizeof(key)-length-1), key+length, "")]=0;

    // Try to retrieve a UnicodeString* from the bundle.  The result,
    // if any, should NOT be deleted.
    /*const UnicodeString* resString = bundle.getString(key, status);*/
    UnicodeString resString = bundle.getStringEx(key, status);

    /*if (U_SUCCESS(status) && resString != 0) {*/
    if (U_SUCCESS(status) && resString.length() != 0) {
        /*return result = *resString; // [sic] assign & return*/
        return result = resString; // [sic] assign & return
    }

    // We have failed to get a name from the locale data.  This is
    // typical, since most transliterators will not have localized
    // name data.  The next step is to retrieve the MessageFormat
    // pattern from the locale data and to use it to synthesize the
    // name from the ID.

    status = U_ZERO_ERROR;
    /*resString = bundle.getString(RB_DISPLAY_NAME_PATTERN, status);*/
    resString = bundle.getStringEx(RB_DISPLAY_NAME_PATTERN, status);

    /*if (U_SUCCESS(status) && resString != 0) {*/
    if (U_SUCCESS(status) && resString.length() != 0) {
        /*MessageFormat msg(*resString, inLocale, status);*/
        MessageFormat msg(resString, inLocale, status);
        // Suspend checking status until later...

        // We pass either 2 or 3 Formattable objects to msg.
        Formattable args[3];
        int32_t i = ID.indexOf(ID_SEP);
        int32_t nargs;
        if (i < 0) {
            args[0].setLong(1); // # of args to follow
            args[1].setString(ID);
            nargs = 2;
        } else {
            UnicodeString left, right;
            ID.extractBetween(0, i, left);
            ID.extractBetween(i+1, ID.length(), right);
            args[0].setLong(2); // # of args to follow
            args[1].setString(left);
            args[2].setString(right);
            nargs = 3;
        }

        // Use display names for the scripts, if they exist
        UnicodeString s;
        length=(int32_t)uprv_strlen(RB_SCRIPT_DISPLAY_NAME_PREFIX);
        for (int j=1; j<=((i<0)?1:2); ++j) {
            status = U_ZERO_ERROR;
            uprv_strcpy(key, RB_SCRIPT_DISPLAY_NAME_PREFIX);
            args[j].getString(s);
            key[length + s.extract(0, sizeof(key)-length-1, key+length, "")]=0;

            /*resString = bundle.getString(key, status);*/
            resString = bundle.getStringEx(key, status);

            if (U_SUCCESS(status)) {
                /*args[j] = *resString;*/
                args[j] = resString;
            }
        }
        
        status = U_ZERO_ERROR;
        FieldPosition pos; // ignored by msg
        msg.format(args, nargs, result, pos, status);
        if (U_SUCCESS(status)) {
            return result;
        }
    }

    // We should not reach this point unless there is something
    // wrong with the build or the RB_DISPLAY_NAME_PATTERN has
    // been deleted from the root RB_LOCALE_ELEMENTS resource.
    result = ID;
    return result;
}

/**
 * Returns the filter used by this transliterator, or <tt>null</tt>
 * if this transliterator uses no filter.  Caller musn't delete
 * the result!
 */
const UnicodeFilter* Transliterator::getFilter(void) const {
    return filter;
}

/**
 * Returns the filter used by this transliterator, or
 * <tt>NULL</tt> if this transliterator uses no filter.  The
 * caller must eventually delete the result.  After this call,
 * this transliterator's filter is set to <tt>NULL</tt>.
 */
UnicodeFilter* Transliterator::orphanFilter(void) {
    UnicodeFilter *result = filter;
    // MUST go through adoptFilter in case latter is overridden
    adoptFilter(0);
    return result;
}

/**
 * Changes the filter used by this transliterator.  If the filter
 * is set to <tt>null</tt> then no filtering will occur.
 *
 * <p>Callers must take care if a transliterator is in use by
 * multiple threads.  The filter should not be changed by one
 * thread while another thread may be transliterating.
 */
void Transliterator::adoptFilter(UnicodeFilter* filterToAdopt) {
    delete filter;
    filter = filterToAdopt;
}

/**
 * Returns this transliterator's inverse.  See the class
 * documentation for details.  This implementation simply inverts
 * the two entities in the ID and attempts to retrieve the
 * resulting transliterator.  That is, if <code>getID()</code>
 * returns "A-B", then this method will return the result of
 * <code>getInstance("B-A")</code>, or <code>null</code> if that
 * call fails.
 *
 * <p>This method does not take filtering into account.  The
 * returned transliterator will have no filter.
 *
 * <p>Subclasses with knowledge of their inverse may wish to
 * override this method.
 *
 * @return a transliterator that is an inverse, not necessarily
 * exact, of this transliterator, or <code>null</code> if no such
 * transliterator is registered.
 * @see #registerInstance
 */
Transliterator* Transliterator::createInverse(void) const {
    return Transliterator::createInstance(ID, UTRANS_REVERSE);
}

/**
 * Returns a <code>Transliterator</code> object given its ID.
 * The ID must be either a system transliterator ID or a ID registered
 * using <code>registerInstance()</code>.
 *
 * @param ID a valid ID, as enumerated by <code>getAvailableIDs()</code>
 * @return A <code>Transliterator</code> object with the given ID
 * @see #registerInstance
 * @see #getAvailableIDs
 * @see #getID
 */
Transliterator* Transliterator::createInstance(const UnicodeString& ID,
                                               UTransDirection dir,
                                               UParseError* parseError) {
    UErrorCode status = U_ZERO_ERROR;
    return createInstance(ID, dir, -1, NULL, parseError, status);
}

/**
 * Create a transliterator given a compound ID (possibly degenerate,
 * with no ID_DELIM).  If idSplitPoint >= 0 and adoptedSplitTrans !=
 * 0, then insert adoptedSplitTrans in the compound ID at offset
 * idSplitPoint.  Otherwise idSplitPoint should be -1 and
 * adoptedSplitTrans should be 0.  The resultant transliterator will
 * be an atomic (non-compound) transliterator if this is indicated by
 * ID.  Otherwise it will be a compound translitertor.
 */
Transliterator* Transliterator::createInstance(const UnicodeString& ID,
                                               UTransDirection dir,
                                               int32_t idSplitPoint,
                                               Transliterator *adoptedSplitTrans,
                                               UParseError* parseError,
                                               UErrorCode& status) {
    if (U_FAILURE(status)) {
        return 0;
    }

    UVector list;
    int32_t ignored;
    parseCompoundID(ID, dir, idSplitPoint, adoptedSplitTrans,
                    list, ignored, parseError, status);

    if (U_FAILURE(status)) {
        return 0;
    }
    
    switch (list.size()) {
    case 0:
        return new NullTransliterator();

    case 1:
        return (Transliterator*) list.elementAt(0);

    default:
        return new CompoundTransliterator(ID, dir, list, status);
    }
}

/**
 * Returns a <code>Transliterator</code> object constructed from
 * the given rule string.  This will be a RuleBasedTransliterator,
 * if the rule string contains only rules, or a
 * CompoundTransliterator, if it contains ID blocks, or a
 * NullTransliterator, if it contains ID blocks which parse as
 * empty for the given direction.
 */
Transliterator* Transliterator::createFromRules(const UnicodeString& ID,
                                                const UnicodeString& rules,
                                                UTransDirection dir,
                                                UParseError* parseError) {
    UnicodeString idBlock;
    int32_t idSplitPoint = -1;
    TransliterationRuleData *data = 0;
    UErrorCode status = U_ZERO_ERROR;

    TransliteratorParser::parse(rules, dir, data,
                                idBlock, idSplitPoint,
                                parseError, status);

    if (U_FAILURE(status)) {
        delete data;
        return 0;
    }

    // NOTE: The logic here matches that in _createInstance().
    if (idBlock.length() == 0) {
        if (data == 0) {
            // No idBlock, no data -- this is just an
            // alias for Null
            return new NullTransliterator();
        } else {
            // No idBlock, data != 0 -- this is an
            // ordinary RBT_DATA.
            return new RuleBasedTransliterator(ID, data, TRUE); // TRUE == adopt data object
        }
    } else {
        if (data == 0) {
            // idBlock, no data -- this is an alias
            Transliterator *t = createInstance(idBlock, dir, parseError);
            if (t != 0) {
                t->setID(ID);
            }
            return t;
        } else {
            // idBlock and data -- this is a compound
            // RBT
            UnicodeString id("_", "");
            Transliterator *t = new RuleBasedTransliterator(id, data, TRUE); // TRUE == adopt data object
            t = new CompoundTransliterator(ID, idBlock, idSplitPoint,
                                           t, status);
            if (U_FAILURE(status)) {
                delete t;
                t = 0;
            }
            return t;
        }
    }
}

UnicodeString& Transliterator::toRules(UnicodeString& rulesSource,
                                       UBool escapeUnprintable) const {
    // The base class implementation of toRules munges the ID into
    // the correct format.  That is: foo => ::foo
    rulesSource = getID();
    // KEEP in sync with rbt_pars
    rulesSource.insert(0, UnicodeString("::", ""));
    return rulesSource;
}

/**
 * Parse a compound ID (possibly a degenerate one, containing no
 * ID_DELIM).  If idSplitPoint >= 0 and adoptedSplitTrans != 0, then
 * insert adoptedSplitTrans in the compound ID at offset idSplitPoint.
 * Otherwise idSplitPoint should be -1 and adoptedSplitTrans should be
 * 0.  Return in the result vector the instantiated transliterator
 * objects (one of these will be adoptedSplitTrans, if the latter was
 * specified).  These will be in order of id, so if dir is REVERSE,
 * then the caller will have to reverse the order.
 * 
 * @param splitTransIndex output parameter to receive the index in
 * 'result' at which the adoptedSplitTrans is stored, or -1 if
 * adoptedSplitTrans == 0
 */
void Transliterator::parseCompoundID(const UnicodeString& id,
                                     UTransDirection dir,
                                     int32_t idSplitPoint,
                                     Transliterator *adoptedSplitTrans,
                                     UVector& result,
                                     int32_t& splitTransIndex,
                                     UParseError* parseError,
                                     UErrorCode& status) {
    if (U_FAILURE(status)) {
        return;
    }
    
    splitTransIndex = -1;
    int32_t pos = 0;
    int32_t i;
    while (pos < id.length()) {
        // We compare (pos >= split), not (pos == split), so we can
        // skip over whitespace (see below).
        if (pos >= idSplitPoint && adoptedSplitTrans != 0) {
            splitTransIndex = result.size();
            result.addElement(adoptedSplitTrans);
            adoptedSplitTrans = 0;
        }
        int32_t p = pos;
        UBool sawDelimiter; // We ignore this
        Transliterator *t =
            parseID(id, p, sawDelimiter, dir, parseError, TRUE);
        if (p == pos) {
            delete t;
            status = U_ILLEGAL_ARGUMENT_ERROR;
            break;
        }
		pos = p;
        // The return value may be NULL when, for instance, creating a
        // REVERSE transliterator of ID "Latin-Greek()".
        if (t != 0) {
            result.addElement(t);
        }
    }

    // Handle case of idSplitPoint == id.length()
    if (pos >= idSplitPoint && adoptedSplitTrans != 0) {
        splitTransIndex = result.size();
        result.addElement(adoptedSplitTrans);
        adoptedSplitTrans = 0;
    }

    if (U_FAILURE(status)) {
        for (i=0; i<result.size(); ++i) {
            delete (Transliterator*)result.elementAt(i);
        }
        result.removeAllElements();
        delete adoptedSplitTrans;
    }
}

/**
 * Parse a single ID, possibly including an inline filter, and return
 * the resultant transliterator object.  NOTE: If 'create' is FALSE,
 * then the amount of syntax checking is limited.  However, the 'pos'
 * parameter will be updated correctly, assuming the input string is
 * valid.
 *
 * A trailing /;? \s* / is skipped.  The parameter sawDelimiter
 * indicates whether the ';' was seen or not.  Upon return, if pos is
 * advanced, it will either point to a non-whitespace character past
 * the trailing ';', if any, or be equal to length().
 *
 * On return one of the following will be true:
 *  pos unchanged: sawDelimiter meaningless
 *  pos == ID.length(): sawDelimiter TRUE or FALSE
 *  pos < ID.length(): sawDelimiter always TRUE
 *
 * @param ID the ID string
 * @param pos INPUT-OUTPUT parameter.  On input, the position of the
 * first character to parse.  On output, the position after the last
 * character parsed.  This will be a semicolon or ID.length().  In the
 * case of an error this value will be unchanged.
 * @param create if TRUE, create and return the result.  If FALSE,
 * only scan the ID, and return NULL.
 * @return a newly created transliterator, or NULL.  NULL is returned
 * in all cases if create is FALSE.  If create is TRUE, then NULL is
 * returned on error, or if the ID is effectively empty.
 * E.g. "Latin-Greek()" with dir == REVERSE.  Do NOT check for NULL to
 * determine if there was an error.  Instead, check to see if pos
 * moved.
 */
Transliterator* Transliterator::parseID(const UnicodeString& ID,
                                        int32_t& pos,
                                        UBool& sawDelimiter,
                                        UTransDirection dir,
                                        UParseError* parseError,
                                        UBool create) {
    Transliterator* t = 0;
    UnicodeString str; // scratch

    // Look for embedded filter pattern by looking for ';' and
    // '[' and seeing which comes first.
    UnicodeSet *filter = 0;
    int32_t limit = ID.indexOf(ID_DELIM, pos);
    sawDelimiter = limit >= 0;
    if (!sawDelimiter) {
        limit = ID.length();
    }
    int32_t setStart = ID.indexOf((UChar)0x005B /*[*/, pos);
    int32_t setLimit;
    if (setStart >= 0 && setStart < limit) {
        UErrorCode status = U_ZERO_ERROR;
        ParsePosition ppos(setStart);
        filter = new UnicodeSet();
        filter->applyPattern(ID, ppos, 0, status);
        if (U_FAILURE(status)) {
            // There was a parse failure in the filter pattern
            delete filter;
            return 0;
        }
        setLimit = ppos.getIndex();
        if (limit < setLimit) {
            limit = ID.indexOf(ID_DELIM, setLimit);
            sawDelimiter = limit >= 0;
            if (!sawDelimiter) {
                limit = ID.length();
            }
        }
    } else {
        setStart = setLimit = pos;
    }
    
    // Advance limit past /;?\s*/
    int32_t idLimit = limit; // limit before separator
    if (sawDelimiter) {
        // assert(limit < ID.length() && ID.charAt(limit) == ID_DELIM);
        ++limit;
    }
    while (limit < ID.length() && u_isspace(ID.charAt(limit))) {
        ++limit;
    }

    if (!create) {
        // TODO Improve performance by scanning the UnicodeSet pattern
        // without actually constructing it, if create is FALSE.  That
        // is, create a method like this one for UnicodeSet.
        delete filter;
        pos = limit;
        return 0;
    }

    // 'id' is the ID with the filter pattern removed and with
    // whitespace deleted.
    UnicodeString id;
    ID.extractBetween(pos, setStart, id);
    ID.extractBetween(setLimit, idLimit, str);
    id.append(str);

    // Delete whitespace
    int32_t i;
    for (i=0; i<id.length(); ++i) {
        if (Unicode::isWhitespace(id.charAt(i))) {
            id.remove(i, 1);
            --i;
        }
    }

    // Fix the id, if necessary, by reversing it (A-B => B-A).
    // Record the position of the separator.  Detect the special
    // case of Null, whose inverse is itself.  Given an ID with no
    // separator "Foo", an abbreviation for "Any-Foo", consider
    // the inverse to be "Foo-Any".
    int32_t sep = id.indexOf(ID_SEP);
    if (sep < 0 && id.caseCompare(NullTransliterator::ID,
                                  U_FOLD_CASE_DEFAULT) == 0) {
        sep = id.length();
    } else if (dir == UTRANS_REVERSE) {
        if (sep >= 0) {
            id.extractBetween(0, sep, str);
            id.removeBetween(0, sep+1);
        } else {
            str = UnicodeString("Any", "");
        }
        sep = id.length();
        id.append(ID_SEP).append(str);
    } else if (sep < 0) {
        str = UnicodeString("Any-", "");
        sep = str.length();
        id.insert(0, str);
    }

    // The 'alias' parameter is non-empty if _createInstance()
    // finds that the given ID refers to an alias.  The reason
    // _createInstance() doesn't call createInstance() (this
    // method) directly is to avoid deadlock.  There are other
    // ways to do this but this is one of the more efficient ways.
    str.truncate(0);
    t = _createInstance(id, str /*alias*/, parseError);

    if (str.length() > 0) {
        // assert(t==0);
        t = createInstance(str, UTRANS_FORWARD, parseError);
    }

    if (t != 0) {
        if (filter != 0) {
            t->adoptFilter(filter);
            id.insert(sep, ID, setStart, setLimit-setStart);
        }
        t->setID(id);
        pos = limit;
    }

    return t;
}
                                        

/**
 * Returns a transliterator object given its ID.  Unlike getInstance(),
 * this method returns null if it cannot make use of the given ID.
 * @param aliasReturn if ID is an alias transliterator this is set
 * the the parameter to be passed to createInstance() and 0 is
 * returned; otherwise, this is unchanged
 */
Transliterator* Transliterator::_createInstance(const UnicodeString& ID,
                                                UnicodeString& aliasReturn,
                                                UParseError* parseError) {
    if (!cacheInitialized) {
        initializeCache();
    }

    Mutex lock(&cacheMutex);

    CacheEntry* entry = (CacheEntry*) cache->get(ID);
    if (entry == 0) {
        entry = (CacheEntry*) internalCache->get(ID);
        if (entry == 0) {
            return 0; // out of memory
        }
    }

    UErrorCode status = U_ZERO_ERROR;

    for (;;) {
        if (entry->entryType == CacheEntry::RBT_DATA) {
            return new RuleBasedTransliterator(ID, entry->u.data);
        } else if (entry->entryType == CacheEntry::PROTOTYPE) {
            return entry->u.prototype->clone();
        } else if (entry->entryType == CacheEntry::ALIAS) {
            // We can't call createInstance() here because of deadlock.
            aliasReturn = entry->stringArg;
            return 0;
        } else if (entry->entryType == CacheEntry::FACTORY) {
            return entry->u.factory();
        } else if (entry->entryType == CacheEntry::COMPOUND_RBT) {
            UnicodeString id("_", "");
            Transliterator *t = new RuleBasedTransliterator(id, entry->u.data);
            t = new CompoundTransliterator(ID, entry->stringArg,
                                           entry->intArg, t, status);
            if (U_FAILURE(status)) {
                delete t;
                t = 0;
                _unregister(ID);
            }
            return t;
        }

        // At this point entry type must be either RULES_FORWARD or
        // RULES_REVERSE.  We process the rule data into a
        // TransliteratorRuleData object, and possibly also into an
        // ::id header and/or footer.  Then we modify the cache with
        // the parsed data and retry.
        UBool isReverse = (entry->entryType == CacheEntry::RULES_REVERSE);

        // We use the file name, taken from another resource bundle
        // 2-d array at static init time, as a locale language.  We're
        // just using the locale mechanism to map through to a file
        // name; this in no way represents an actual locale.

        char *ch = new char[entry->stringArg.length() + 1];
        ch[entry->stringArg.extract(0, 0x7fffffff, ch, "")] = 0;
        Locale fakeLocale(ch);
        delete [] ch;

        ResourceBundle bundle((char *)0, fakeLocale, status);
        UnicodeString rules = bundle.getStringEx(RB_RULE, status);

        // If the status indicates a failure, then we don't have any
        // rules -- there is probably an installation error.  The list
        // in the root locale should correspond to all the installed
        // transliterators; if it lists something that's not
        // installed, we'll get an error from ResourceBundle.

        TransliteratorParser::parse(rules, isReverse ?
                                    UTRANS_REVERSE : UTRANS_FORWARD,
                                    entry->u.data,
                                    entry->stringArg,
                                    entry->intArg,
                                    parseError,
                                    status);

        if (U_FAILURE(status)) {
            // We have a failure of some kind.  Remove the ID from the
            // cache so we don't keep trying.  NOTE: This will throw off
            // anyone who is, at the moment, trying to iterate over the
            // available IDs.  That's acceptable since we should never
            // really get here except under installation, configuration,
            // or unrecoverable run time memory failures.
            _unregister(ID);
            break;
        }

        // Reset entry->entryType to something that we process at the
        // top of the loop, then loop back to the top.  As long as we
        // do this, we only loop through twice at most.
        // NOTE: The logic here matches that in createFromRules().
        if (entry->stringArg.length() == 0) {
            if (entry->u.data == 0) {
                // No idBlock, no data -- this is just an
                // alias for Null
                entry->entryType = CacheEntry::ALIAS;
                entry->stringArg = NullTransliterator::ID;
            } else {
                // No idBlock, data != 0 -- this is an
                // ordinary RBT_DATA
                entry->entryType = CacheEntry::RBT_DATA;
            }
        } else {
            if (entry->u.data == 0) {
                // idBlock, no data -- this is an alias
                entry->entryType = CacheEntry::ALIAS;
            } else {
                // idBlock and data -- this is a compound
                // RBT
                entry->entryType = CacheEntry::COMPOUND_RBT;
            }
        }
    }

    return 0; // failed
}

// For public consumption
void Transliterator::registerFactory(const UnicodeString& id,
                                     Transliterator::Factory factory,
                                     UErrorCode &status) {
    if (U_FAILURE(status)) {
        return;
    }
    if (!cacheInitialized) {
        initializeCache();
    }
    Mutex lock(&cacheMutex);
    _registerFactory(id, factory, status);
}

// To be called only by Transliterator subclasses that are called
// to register themselves by initializeCache().
void Transliterator::_registerFactory(const UnicodeString& id,
                                      Transliterator::Factory factory,
                                      UErrorCode &status) {
    if (U_FAILURE(status)) {
        return;
    }

    CacheEntry* entry = (CacheEntry*) cache->get(id);
    if (entry == 0) {
        cacheIDs.addElement((void*) new UnicodeString(id));
        entry = new CacheEntry();
    }
    entry->setFactory(factory);

    cache->put(id, entry, status);
}

/**
 * Registers a instance <tt>obj</tt> of a subclass of
 * <code>Transliterator</code> with the system.  This object must
 * implement the <tt>clone()</tt> method.  When
 * <tt>getInstance()</tt> is called with an ID string that is
 * equal to <tt>obj.getID()</tt>, then <tt>obj.clone()</tt> is
 * returned.
 *
 * @param obj an instance of subclass of
 * <code>Transliterator</code> that defines <tt>clone()</tt>
 * @see #getInstance
 * @see #unregister
 */
void Transliterator::registerInstance(Transliterator* adoptedPrototype,
                                      UErrorCode &status) {    
    if (!cacheInitialized) {
        initializeCache();
    }

    Mutex lock(&cacheMutex);
    _registerInstance(adoptedPrototype, status);
}

/**
 * This internal method registers a prototype instance in the cache.
 * The CALLER MUST MUTEX using cacheMutex before calling this method.
 */
void Transliterator::_registerInstance(Transliterator* adoptedPrototype,
                                       UErrorCode &status) {
    if (U_FAILURE(status)) {
        delete adoptedPrototype;
        return;
    }

    const UnicodeString& id = adoptedPrototype->getID();

    CacheEntry* entry = (CacheEntry*) cache->get(id);
    if (entry == 0) {
        cacheIDs.addElement((void*) new UnicodeString(id));
        entry = new CacheEntry();
    }

    entry->adoptPrototype(adoptedPrototype);

    cache->put(id, entry, status);
}

/**
 * Unregisters a transliterator or class.  This may be either
 * a system transliterator or a user transliterator or class.
 * 
 * @param ID the ID of the transliterator or class
 * @see #registerInstance

 */
void Transliterator::unregister(const UnicodeString& ID) {
    if (!cacheInitialized) {
        initializeCache();
    }
    Mutex lock(&cacheMutex);
    _unregister(ID);
}

/**
 * Unregisters a transliterator or class.  Internal method.
 * Prerequisites: The cache must be initialized, and the
 * caller must own the cacheMutex.
 */
void Transliterator::_unregister(const UnicodeString& ID) {
    cacheIDs.removeElement((void*) &ID);
    CacheEntry* entry = (CacheEntry*) cache->get(ID);
    if (entry != 0) {
        cache->remove(ID);
        delete entry;
    }
}

/**
 * Return the number of IDs currently registered with the system.
 * To retrieve the actual IDs, call getAvailableID(i) with
 * i from 0 to countAvailableIDs() - 1.
 */
int32_t Transliterator::countAvailableIDs(void) {
    if (!cacheInitialized) {
        initializeCache();
    }
    Mutex lock(&cacheMutex);
    return cacheIDs.size();
}

/**
 * Return the index-th available ID.  index must be between 0
 * and countAvailableIDs() - 1, inclusive.  If index is out of
 * range, the result of getAvailableID(0) is returned.
 */
const UnicodeString& Transliterator::getAvailableID(int32_t index) {
    if (index < 0 || index >= cacheIDs.size()) {
        index = 0;
    }
    if (!cacheInitialized) {
        initializeCache();
    }
    Mutex lock(&cacheMutex);
    return *(const UnicodeString*) cacheIDs[index];
}

/**
 * Method for subclasses to use to obtain a character in the given
 * string, with filtering.
 */
UChar Transliterator::filteredCharAt(const Replaceable& text, int32_t i) const {
    UChar c;
    const UnicodeFilter* localFilter = getFilter();
    return (localFilter == 0) ? text.charAt(i) :
        (localFilter->contains(c = text.charAt(i)) ? c : (UChar)0xFFFE);
}

/**
 * Comparison function for UVector.
 *
 * Do not make this a class static: This should remain a C function
 * for os/390 and Solaris Workshop [grhoten]
 */
U_CDECL_BEGIN
static UBool U_CALLCONV
_compareCaselessUnicodeString(const void* a, const void* b) {
    UHashKey s, t;
    s.pointer = (void*) a;
    t.pointer = (void*) b;
    return uhash_compareCaselessUnicodeString(s, t);
}
U_CDECL_END

void Transliterator::initializeCache(void) {
    // Lock first, check init boolean second
    Mutex lock(&cacheMutex);
    if (cacheInitialized) {
        return;
    }
        
    UErrorCode status = U_ZERO_ERROR;

    // Before looking for the resource, construct our cache.
    // That way if the resource is absent, we will at least
    // have a valid cache object.
    cacheIDs.setDeleter(uhash_deleteUnicodeString);
    cacheIDs.setComparer(_compareCaselessUnicodeString);

    /* The following code parses the index table located in
     * icu/data/translit_index.txt.  The index is an n x 4 table
     * that follows this format:
     *
     *   <id>:file:<resource>:<direction>
     *   <id>:internal:<resource>:<direction>
     *   <id>:alias:<getInstanceArg>:
     *  
     * <id> is the ID of the system transliterator being defined.  These
     * are public IDs enumerated by Transliterator.getAvailableIDs(),
     * unless the second field is "internal".
     * 
     * <resource> is a ResourceReader resource name.  Currently these refer
     * to file names under com/ibm/text/resources.  This string is passed
     * directly to ResourceReader, together with <encoding>.
     * 
     * <direction> is either "FORWARD" or "REVERSE".
     * 
     * <getInstanceArg> is a string to be passed directly to
     * Transliterator.getInstance().  The returned Transliterator object
     * then has its ID changed to <id> and is returned.
     *
     * The extra blank field on "alias" lines is to make the array square.
     */

    Locale indexLoc("translit_index");

    ResourceBundle bundle((char *)0,
                          indexLoc, status);
    ResourceBundle transIDs(bundle.get(RB_RULE_BASED_IDS, status));

    int32_t row, maxRows;
    if (U_SUCCESS(status)) {
        maxRows = transIDs.getSize();
        for (row = 0; row < maxRows; row++) {
            ResourceBundle colBund(transIDs.get(row, status));

            if (U_SUCCESS(status) && colBund.getSize() == 4) {
                UnicodeString id(colBund.getStringEx((int32_t)0, status));
                UChar type = colBund.getStringEx(1, status).charAt(0);
                UnicodeString resource(colBund.getStringEx(2, status));

                if (U_SUCCESS(status)) {
                    CacheEntry* entry = new CacheEntry();
                    UBool isInternal = FALSE;
                    if (type == 0x0066 || type == 0x0069) { // 'f', 'i'
                        // 'file' or 'internal'; row[2]=resource, row[3]=direction
                        isInternal = (type == 0x0069/*i*/);
                        if ((colBund.getStringEx(3, status).charAt(0)) == 0x0052) {// 'R'
                            entry->entryType = CacheEntry::RULES_REVERSE;
                        } else {
                            entry->entryType = CacheEntry::RULES_FORWARD;
                        }
                    } else { // assert(type == 0x0061 /*a*/)
                        // 'alias'; row[2]=createInstance argument
                        entry->entryType = CacheEntry::ALIAS;
                    }
                    entry->stringArg = resource;

                    // Use internalCache for 'internal' entries
                    Hashtable* c = isInternal ? internalCache : cache;
                    c->put(id, entry, status);

                    // cacheIDs owns & should delete the following string
                    if (!isInternal) {
                        cacheIDs.addElement((void*) new UnicodeString(id));
                    }
                }
            }
        }
    }
    // Manually add prototypes that the system knows about to the
    // cache.  This is how new non-rule-based transliterators are
    // added to the system.

    status = U_ZERO_ERROR; // Reset status for following calls
    _registerInstance(new HexToUnicodeTransliterator(), status);
    _registerInstance(new UnicodeToHexTransliterator(), status);
    _registerInstance(new JamoHangulTransliterator(), status);
    _registerInstance(new HangulJamoTransliterator(), status);
    _registerInstance(new NullTransliterator(), status);
    _registerInstance(new RemoveTransliterator(), status);
    _registerInstance(new LowercaseTransliterator(), status);
    _registerInstance(new UppercaseTransliterator(), status);
    _registerInstance(new TitlecaseTransliterator(), status);
    _registerInstance(new UnicodeNameTransliterator(), status);
    _registerInstance(new NameUnicodeTransliterator(), status);
    NormalizationTransliterator::registerIDs();

    cacheInitialized = TRUE;
}

Transliterator::CacheEntry::CacheEntry() {
    u.prototype = 0;
    entryType = NONE;
}

Transliterator::CacheEntry::~CacheEntry() {
    if (entryType == PROTOTYPE) {
        delete u.prototype;
    }
}

void Transliterator::CacheEntry::adoptPrototype(Transliterator* adopted) {
    if (entryType == PROTOTYPE) {
        delete u.prototype;
    }
    entryType = PROTOTYPE;
    u.prototype = adopted;
}

void Transliterator::CacheEntry::setFactory(Transliterator::Factory factory) {
    if (entryType == PROTOTYPE) {
        delete u.prototype;
    }
    entryType = FACTORY;
    u.factory = factory;
}
