/*
**********************************************************************
*   Copyright (C) 1997-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*
* File URES.H (formerly CRESBUND.H)
*
* Modification History:
*
*   Date        Name        Description
*   04/01/97    aliu        Creation.
*   02/22/99    damiba      overhaul.
*   04/04/99    helena      Fixed internal header inclusion.
*   04/15/99    Madhu       Updated Javadoc  
*   06/14/99    stephen     Removed functions taking a filename suffix.
*   07/20/99    stephen     Language-independent ypedef to void*
*   11/09/99    weiv        Added ures_getLocale()
******************************************************************************
*/

#ifndef URES_H
#define URES_H

#include "unicode/utypes.h"
#include "unicode/uloc.h"

/**
 * \file
 * \brief C API: Resource Bundle 
 *
 * <h2>C API: Resource Bundle</h2>
 *
 * C API representing a collection of resource information pertaining to a given
 * locale. A resource bundle provides a way of accessing locale- specific information in
 * a data file. You create a resource bundle that manages the resources for a given
 * locale and then ask it for individual resources.
 * <P>
 * The resource bundle file is a text (ASCII or Unicode) file with the format:
 * <pre>
 * \code
 *    locale {
 *       tag1 {...}
 *       tag2 {...}
 *    }
 * \endcode
 * </pre>
 * The tags are used to retrieve the data later. You may not have multiple instances of
 * the same tag.
 * <P>
 * Four data types are supported. These are solitary strings, comma-delimited lists of
 * strings, 2-dimensional arrays of strings, and tagged lists of strings.
 * <P>
 * Note that all data is textual. Adjacent strings are merged by the low-level
 * tokenizer, so that the following effects occur: foo bar, baz // 2 elements, "foo
 * bar", and "baz" "foo" "bar", baz // 2 elements, "foobar", and "baz" Note that a
 * single intervening space is added between merged strings, unless they are both double
 * quoted. This extends to more than two strings in a row.
 * <P>
 * Whitespace is ignored, as in a C source file.
 * <P>
 * Solitary strings have the format:
 * <pre>
 * \code
 *    Tag { Data }
 * \endcode
 * </pre>
 * This is indistinguishable from a comma-delimited list with only one element, and in
 * fact may be retrieved as such (as an array, or as element 0 or an array).
 * <P>
 * Comma-delimited lists have the format:
 * <pre>
 * \code
 *    Tag { Data, Data, Data }
 * \endcode
 * </pre>
 * Parsing is lenient; a final string, after the last element, is allowed.
 * <P>
 * Tagged lists have the format:
 * <pre>
 * \code
 *    Tag { Subtag { Data } Subtag {Data} }
 * \endcode
 * </pre>
 * Data is retrieved by specifying the subtag.
 * <P>
 * Two-dimensional arrays have the format:
 * <pre>
 * \code
 *    TwoD {
 *        { r1c1, r1c2, ..., r1cm },
 *        { r2c1, r2c2, ..., r2cm },
 *        ...
 *        { rnc1, rnc2, ..., rncm }
 *    }
 * \endcode
 * </pre>
 * where n is the number of rows, and m is the number of columns. Parsing is lenient (as
 * in other data types). A final comma is always allowed after the last element; either
 * the last string in a row, or the last row itself. Furthermore, since there is no
 * ambiguity, the commas between the rows are entirely optional. (However, if a comma is
 * present, there can only be one comma, no more.) It is possible to have zero columns,
 * as follows:
 * <pre>
 * \code
 *    Odd { {} {} {} } // 3 x 0 array
 * \endcode
 * </pre>
 * But it is impossible to have zero rows. The smallest array is thus a 1 x 0 array,
 * which looks like this:
 * <pre>
 * \code
 *   Smallest { {} } // 1 x 0 array
 * \endcode
 * </pre>
 * The array must be strictly rectangular; that is, each row must have the same number
 * of elements.
 * <P>
 * <H2>Usage model:</H2>
 * Resource bundles contain resources. In code, both types of entities are treated the
 * same and are represented with a same data structure <pre>UResourceBundle</pre>. 
 * Resource bundle has a tree structure, where leaf nodes can be strings, binaries 
 * and integers while non-leaf nodes (including the root node) can be tables and arrays.
 * One or more resource bundles are used to represent data needed by the application
 * for running in the particular locale. Complete set of resource bundles for an application
 * would contain all the data needed to run in intended locales. <P>
 * If the data for the requested locale is missing, an effort will be made to obtain most
 * usable data. This process is called fallback. Also, fallback happens when a resource 
 * is not present in the given bundle. Then, the other bundles in the fallback chain are
 * also searched for the requested resource.<P>
 * Retrieving data from resources is possible in several ways, depending on the type of
 * the resources:<P>
 * 1) Access by a key: this approach works only for table resources<P>
 * 2) Access by an index: tables and arrays can be addressed by an index<P>
 * 3) Iteration: works for tables and arrays<P>
 * To use data in resource bundles, following steps are needed:<P>
 * 1) opening a bundle for a particular locale:
 * <pre>
 * \code
 *      UErrorCode status = U_ZERO_ERROR;
 *      UResourceBundle* resB = ures_open("/datadir/resources/GUI", "de_AT_EURO", &status);
 * \endcode
 * </pre>
 * Status allows, besides testing for plain error, to see whether fallback occured. There
 * are two extra non error values for status after this operation: U_USING_FALLBACK_ERROR,
 * which implies that the bundle for the requested locale was not found, but that one of
 * the bundles in the fallback chain was used (de_AT and de in this case) and
 * U_USING_DEFAULT_ERROR which implies that not one bundle in the fallback chain was found
 * and that default locale was used. In any case, 'root' locale is always at the end of the
 * chain.
 *
 * This is an example for using a possible custom resource:
 * <pre>
 * \code
 *     const char *currentLocale;
 *     UErrorCode success = U_ZERO_ERROR;
 *     UResourceBundle* myResources=ures_open("MyResources", currentLocale, &success );
 * 
 *     UChar *button1Title, *button2Title;
 *     button1Title= ures_get(myResources, "OkKey", &success );
 *     button2Title= ures_get(myResources, "CancelKey", &success );
 * \endcode
 * </pre>
 */

/** A UResourceBundle.
 *  For usage in C programs.
 */
struct UResourceBundle;

typedef struct UResourceBundle UResourceBundle;

typedef enum {
    RES_NONE=-1,
    RES_STRING=0,
    RES_BINARY=1,
    RES_TABLE=2,

    RES_INT=7,
    RES_ARRAY=8,

    RES_INT_VECTOR=14,
    RES_RESERVED=15
} UResType;

/**
 * Functions to create and destroy resource bundles.
 */

/**
 * Opens a UResourceBundle, from which users can extract strings by using
 * their corresponding keys.
 * Note that the caller is responsible of calling <TT>ures_close</TT> on each succesfully
 * opened resource bundle.
 * @param path  : string containing the full path pointing to the directory
 *                where the resources reside followed by the package name
 *                e.g. "/usr/resource/my_app/resources/guimessages" on a Unix system.
 *                if NULL, ICU default data files will be used.
 * @param locale: specifies the locale for which we want to open the resource
 *                if NULL, the default locale will be used. If strlen(locale) == 0
 *                root locale will be used.
 *                
 * @param status : fills in the outgoing error code.
 * The UErrorCode err parameter is used to return status information to the user. To
 * check whether the construction succeeded or not, you should check the value of
 * U_SUCCESS(err). If you wish more detailed information, you can check for
 * informational error results which still indicate success. U_USING_FALLBACK_ERROR
 * indicates that a fall back locale was used. For example, 'de_CH' was requested,
 * but nothing was found there, so 'de' was used. U_USING_DEFAULT_ERROR indicates that
 * the default locale data or root locale data was used; neither the requested locale 
 * nor any of its fall back locales could be found.
 * @return      a newly allocated resource bundle.
 * @see ures_close
 * @draft
 */
U_CAPI UResourceBundle*  U_EXPORT2 ures_open(const char*    path,   /* NULL if none */
                                           const char*  locale, /* NULL if none */
                                           UErrorCode*     status);


/**
*Opens a UResourceBundle, from which users can extract strings by using
*their corresponding keys. This version of open requires the path 
*string to be of type <TT>const wchar_t*</TT>.
*Note that the caller is responsible of calling <TT>ures_close</TT> on each succesfully
*opened resource bundle.
*@param path: string containing the full path pointing to the directory
*             where the resources reside (should end with a directory
*             separator.
*                e.g. "/usr/resource/my_app/resources/" on a Unix system
*             if NULL will use the system's current data directory
*@param locale: specifies the locale for which we want to open the resource
*                if NULL will use the default locale
*                
*@param status: fills in the outgoing error code.
*@see ures_close
*@return : a newly allocated resource bundle.
*@draft
*/
U_CAPI UResourceBundle* U_EXPORT2 ures_openW(const wchar_t* path, 
                  const char* locale, 
                  UErrorCode* status);

U_CAPI UResourceBundle* U_EXPORT2 ures_openU(const UChar* path, 
                  const char* locale, 
                  UErrorCode* status);

/**
 * Returns the number of strings/arrays in resource bundles.
 *
 *@param resourceBundle: resource bundle containing the desired strings
 *@param resourceKey: key tagging the resource
 *@param err: fills in the outgoing error code
 *                could be <TT>U_MISSING_RESOURCE_ERROR</T> if the key is not found
 *                could be a non-failing error 
 *                e.g.: <TT>U_USING_FALLBACK_ERROR</TT>,<TT>U_USING_DEFAULT_ERROR </TT>
 *@return: for    <STRONG>Arrays</STRONG>: returns the number of strings in the array
 *                <STRONG>2d Arrays</STRONG>: returns the number of 1d arrays
 *                <STRONG>taggedArrays</STRONG>: returns the number of strings in the array
 *                <STRONG>single string</STRONG>: returns 1
 *@see ures_get
 *@draft
 */
U_CAPI int32_t U_EXPORT2 ures_countArrayItems(const UResourceBundle* resourceBundle,
                  const char* resourceKey,
                  UErrorCode* err);
/**
 * close a resource bundle, all pointers returned from the various ures_getXXX calls
 * on this particular bundle are INVALID henceforth.
 *
 * @param resourceBundle: a succesfully opened resourceBundle.
 * @param status: fills in the outgoing error code
 *                could be <TT>U_MISSING_RESOURCE_ERROR</T> if the key is not found
 *                could be a non-failing error 
 *                e.g.: <TT>U_USING_FALLBACK_ERROR</TT>,<TT>U_USING_DEFAULT_ERROR </TT>
 * @see ures_open
 * @see ures_openW
 * @draft
 */
U_CAPI void U_EXPORT2 ures_close(UResourceBundle*    resourceBundle);

/**
 * Return the version number associated with this ResourceBundle. This version
 * number is a string of the form MAJOR.MINOR, where MAJOR is the version number of
 * the current analytic code package, and MINOR is the version number contained in
 * the resource file as the value of the tag "Version". A change in the MINOR
 * version indicated an updated data file. A change in the MAJOR version indicates a
 * new version of the code which is not binary-compatible with the previous version.
 * If no "Version" tag is present in a resource file, the MINOR version "0" is assigned.
 * For example, if the Collation sort key algorithm changes, the MAJOR version
 * increments. If the collation data in a resource file changes, the MINOR version
 * for that file increments.
 * @param resourceBundle: resource bundle in question
 * @return  A string of the form N.n, where N is the major version number,
 *          representing the code version, and n is the minor version number,
 *          representing the resource data file. The caller does not own this
 *          string.
 * @draft
 */
U_CAPI const char* U_EXPORT2 ures_getVersionNumber(const UResourceBundle*   resourceBundle);

U_CAPI void U_EXPORT2 ures_getVersion(const UResourceBundle* resB, UVersionInfo versionInfo);

/**
 * Return the name of the Locale associated with this ResourceBundle.
 * @param resourceBundle: resource bundle in question
 * @param status: just for catching illegal arguments
 * @return  A Locale name
 * @draft
 */
U_CAPI const char* ures_getLocale(const UResourceBundle* resourceBundle, UErrorCode* status);


/** New API */
U_CAPI void ures_openFillIn(UResourceBundle *r, const char* path,
                    const char* localeID, UErrorCode* status);

/**
 * returns a string from a string resource type
 *
 * @param resourceBundle: a string resource
 * @param len:    fills in the length of resulting string
 * @param status: fills in the outgoing error code
 *                could be <TT>U_MISSING_RESOURCE_ERROR</T> if the key is not found
 *                could be a non-failing error 
 *                e.g.: <TT>U_USING_FALLBACK_ERROR</TT>,<TT>U_USING_DEFAULT_ERROR </TT>
 * @return a pointer to a zero-terminated UChar array which lives in a memory mapped/DLL file.
 * @draft
 */
U_CAPI const UChar* U_EXPORT2 ures_getString(const UResourceBundle* resourceBundle, int32_t* len, 
               UErrorCode*               status);

/**
 * returns a binary data from a resource. Can be used at most primitive resource types (binaries,
 * strings, ints)
 *
 * @param resourceBundle: a string resource
 * @param len:    fills in the length of resulting byte chunk
 * @param status: fills in the outgoing error code
 *                could be <TT>U_MISSING_RESOURCE_ERROR</T> if the key is not found
 *                could be a non-failing error 
 *                e.g.: <TT>U_USING_FALLBACK_ERROR</TT>,<TT>U_USING_DEFAULT_ERROR </TT>
 * @return a pointer to a chuck of unsigned bytes which live in a memory mapped/DLL file.
 * @draft
 */
U_CAPI const uint8_t* U_EXPORT2 ures_getBinary(const UResourceBundle* resourceBundle, int32_t* len, 
               UErrorCode*               status);

/**
 * returns an integer from a resource. 
 *
 * @param resourceBundle: a string resource
 * @param status: fills in the outgoing error code
 *                could be <TT>U_MISSING_RESOURCE_ERROR</T> if the key is not found
 *                could be a non-failing error 
 *                e.g.: <TT>U_USING_FALLBACK_ERROR</TT>,<TT>U_USING_DEFAULT_ERROR </TT>
 * @return an integer value
 * @draft
 */
U_CAPI uint32_t U_EXPORT2 ures_getInt(const UResourceBundle* resourceBundle, UErrorCode *status);

/**
 * Returns the size of a resource. Size for scalar types is always 1, and for vector/table types is
 * the number of child resources.
 *
 * @param resourceBundle: a resource
 * @return number of resources in a given resource.
 * @draft
 */
U_CAPI int32_t U_EXPORT2 ures_getSize(UResourceBundle *resourceBundle);

/**
 * Returns the type of a resource. Available types are defined in enum UResType
 *
 * @param resourceBundle: a resource
 * @return type of the given resource.
 * @draft
 */
U_CAPI UResType U_EXPORT2 ures_getType(UResourceBundle *resourceBundle);

/**
 * Returns the key associated with a given resource. Not all the resources have a key - only 
 * those that are members of a table.
 *
 * @param resourceBundle: a resource
 * @return a key associated to this resource, or NULL if it doesn't have a key
 * @draft
 */
U_CAPI const char * U_EXPORT2 ures_getKey(UResourceBundle *resB);

/* ITERATION API 
    This API provides means for iterating through a resource
*/

/**
 * Resets the internal context of a resource so that iteration starts from the first element.
 *
 * @param resourceBundle: a resource
 * @draft
 */
U_CAPI void U_EXPORT2 ures_resetIterator(UResourceBundle *resourceBundle);

/**
 * Checks whether the given resource has another element to iterate over.
 *
 * @param resourceBundle a resource
 * @return TRUE if there are more elements, FALSE if there is no more elements
 * @draft
 */
U_CAPI UBool U_EXPORT2 ures_hasNext(UResourceBundle *resourceBundle);

/**
 * Returns the next resource in a given resource or NULL if there are no more resources 
 * to iterate over. Features a fill-in parameter. 
 *
 * @param resourceBundle    a resource
 * @param fillIn            if NULL a new UResourceBundle struct is allocated and must be deleted by the caller.
 *                          Alternatively, you can supply a struct to be filled by this function.
 * @param status            fills in the outgoing error code
 * @return                  a pointer to a UResourceBundle struct. If fill in param was NULL, caller must delete it
 * @draft
 */
U_CAPI UResourceBundle* U_EXPORT2 ures_getNextResource(UResourceBundle *resourceBundle, UResourceBundle *fillIn, UErrorCode *status);

/**
 * Returns the next string in a given resource or NULL if there are no more resources 
 * to iterate over. 
 *
 * @param resourceBundle    a resource
 * @param len               fill in length of the string
 * @param key               fill in for key associated with this string
 * @param status            fills in the outgoing error code
 * @return a pointer to a zero-terminated UChar array which lives in a memory mapped/DLL file.
 * @draft
 */
U_CAPI const UChar* U_EXPORT2 ures_getNextString(UResourceBundle *resourceBundle, int32_t* len, const char ** key, UErrorCode *status);

/**
 * Returns the resource in a given resource at the specified index. Features a fill-in parameter. 
 *
 * @param resourceBundle    a resource
 * @param indexR            an index to the wanted resource.
 * @param fillIn            if NULL a new UResourceBundle struct is allocated and must be deleted by the caller.
 *                          Alternatively, you can supply a struct to be filled by this function.
 * @param status            fills in the outgoing error code
 * @return                  a pointer to a UResourceBundle struct. If fill in param was NULL, caller must delete it
 * @stable
 */
U_CAPI UResourceBundle* U_EXPORT2 ures_getByIndex(const UResourceBundle *resourceBundle, int32_t indexR, UResourceBundle *fillIn, UErrorCode *status);

/**
 * Returns the string in a given resource at the specified index.
 *
 * @param resourceBundle    a resource
 * @param indexS            an index to the wanted string.
 * @param len               fill in length of the string
 * @param status            fills in the outgoing error code
 * @return                  a pointer to a zero-terminated UChar array which lives in a memory mapped/DLL file.
 * @stable
 */
U_CAPI const UChar* U_EXPORT2 ures_getStringByIndex(const UResourceBundle *resB, int32_t indexS, int32_t* len, UErrorCode *status);

/**
 * Returns a resource in a given resource that has a given key. This procedure works only with table
 * resources. Features a fill-in parameter. 
 *
 * @param resourceBundle    a resource
 * @param key               a key associated with the wanted resource
 * @param fillIn            if NULL a new UResourceBundle struct is allocated and must be deleted by the caller.
 *                          Alternatively, you can supply a struct to be filled by this function.
 * @param status            fills in the outgoing error code.
 * @return                  a pointer to a UResourceBundle struct. If fill in param was NULL, caller must delete it
 * @stable
 */
U_CAPI UResourceBundle* U_EXPORT2 ures_getByKey(const UResourceBundle *resourceBundle, const char* key, UResourceBundle *fillIn, UErrorCode *status);

/**
 * Returns a string in a given resource that has a given key. This procedure works only with table
 * resources. 
 *
 * @param resourceBundle    a resource
 * @param key               a key associated with the wanted string
 * @param len               fill in length of the string
 * @param status            fills in the outgoing error code
 * @return                  a pointer to a zero-terminated UChar array which lives in a memory mapped/DLL file.
 * @stable
 */
U_CAPI const UChar* U_EXPORT2 ures_getStringByKey(const UResourceBundle *resB, const char* key, int32_t* len, UErrorCode *status);

#endif /*_URES*/
/*eof*/
