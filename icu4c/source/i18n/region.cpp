/*
*******************************************************************************
* Copyright (C) 2013, International Business Machines Corporation and
* others. All Rights Reserved.
*******************************************************************************
*
*
* File REGION.CPP
*
* Modification History:*
*   Date        Name        Description
* 01/15/13      Emmons      Original Port from ICU4J
********************************************************************************
*/

/**
 * \file 
 * \brief C++ API: Region classes (territory containment)
 */

#include "unicode/region.h"
#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "unicode/unistr.h"
#include "unicode/ures.h"
#include "unicode/decimfmt.h"
#include "cstring.h"
#include "uhash.h"
#include "uresimp.h"
#include "region_impl.h"

#if !UCONFIG_NO_FORMATTING

U_NAMESPACE_BEGIN

static UBool regionDataIsLoaded = false;
static UVector* regions = NULL;
static UVector* availableRegions[URGN_LIMIT];

static UHashtable *regionAliases;
static UHashtable *regionIDMap;
static UHashtable *numericCodeMap;

static UnicodeString UNKNOWN_REGION_ID = UNICODE_STRING_SIMPLE("ZZ");
static UnicodeString OUTLYING_OCEANIA_REGION_ID = UNICODE_STRING_SIMPLE("QO");
static UnicodeString WORLD_ID = UNICODE_STRING_SIMPLE("001");

UOBJECT_DEFINE_RTTI_IMPLEMENTATION(Region)
UOBJECT_DEFINE_RTTI_IMPLEMENTATION(RegionNameEnumeration)

void addRegion( Region *r ) {

    UErrorCode status = U_ZERO_ERROR;
    if ( regions == NULL ) {
        regions = new UVector(NULL, NULL, status);
    }
    regions->addElement(r,status);
}

void addAvailableRegion( const Region *r , URegionType type) {

    UErrorCode status = U_ZERO_ERROR;
    if ( availableRegions[type] == NULL ) {
        availableRegions[type] = new UVector(NULL, uhash_compareChars, status);
    }

    availableRegions[type]->addElement((void *)r->getRegionCode(),status);
}
/*
 * Initializes the region data from the ICU resource bundles.  The region data
 * contains the basic relationships such as which regions are known, what the numeric
 * codes are, any known aliases, and the territory containment data.
 * 
 * If the region data has already loaded, then this method simply returns without doing
 * anything meaningful.
 */
void Region::loadRegionData() {

    if (regionDataIsLoaded) {
        return;
    }
    
    UErrorCode status = U_ZERO_ERROR;

    UResourceBundle* regionCodes = NULL;
    UResourceBundle* territoryAlias = NULL;
    UResourceBundle* codeMappings = NULL;
    UResourceBundle* worldContainment = NULL;
    UResourceBundle* territoryContainment = NULL;
    UResourceBundle* groupingContainment = NULL;

    DecimalFormat *df = new DecimalFormat(status);
    df->setParseIntegerOnly(TRUE);

    regionAliases = uhash_open(uhash_hashUnicodeString,uhash_compareUnicodeString,NULL,&status);
    regionIDMap = uhash_open(uhash_hashUnicodeString,uhash_compareUnicodeString,NULL,&status);
    numericCodeMap = uhash_open(uhash_hashLong,uhash_compareLong,NULL,&status);

    UResourceBundle *rb = ures_openDirect(NULL,"metadata",&status);
    regionCodes = ures_getByKey(rb,"regionCodes",NULL,&status);
    territoryAlias = ures_getByKey(rb,"territoryAlias",NULL,&status);
    
    UResourceBundle *rb2 = ures_openDirect(NULL,"supplementalData",&status);
    codeMappings = ures_getByKey(rb2,"codeMappings",NULL,&status);

    territoryContainment = ures_getByKey(rb2,"territoryContainment",NULL,&status);
    worldContainment = ures_getByKey(territoryContainment,"001",NULL,&status);
    groupingContainment = ures_getByKey(territoryContainment,"grouping",NULL,&status);

    UVector *continents = new UVector(uprv_deleteUObject, uhash_compareUnicodeString, status);

    while ( ures_hasNext(worldContainment) ) {
        UnicodeString *continentName = new UnicodeString(ures_getNextUnicodeString(worldContainment,NULL,&status));
        continents->addElement(continentName,status);
    }

    UVector *groupings = new UVector(uprv_deleteUObject, uhash_compareUnicodeString, status);
    while ( ures_hasNext(groupingContainment) ) {
        UnicodeString *groupingName = new UnicodeString(ures_getNextUnicodeString(groupingContainment,NULL,&status));
        groupings->addElement(groupingName,status);
    }

    while ( ures_hasNext(regionCodes) ) {
        UnicodeString regionID = ures_getNextUnicodeString(regionCodes,NULL,&status);
        Region *r = new Region();
        r->idStr = regionID;
        r->idStr.extract(0,r->idStr.length(),r->id,sizeof(r->id),US_INV);
        r->type = URGN_TERRITORY; // Only temporary - figure out the real type later once the aliases are known.

        uhash_put(regionIDMap,(void *)&(r->idStr),(void *)r,&status);
        Formattable result;
        UErrorCode ps = U_ZERO_ERROR;
        df->parse(r->idStr,result,ps);
        if ( U_SUCCESS(ps) ) {
            r->code = result.getLong(); // Convert string to number
            uhash_iput(numericCodeMap,r->code,(void *)r,&status);
            r->type = URGN_SUBCONTINENT;
        } else {
            r->code = Region::UNDEFINED_NUMERIC_CODE;
        }
        addRegion(r);
    }


    // Process the territory aliases
    while ( ures_hasNext(territoryAlias) ) {
        UResourceBundle *res = ures_getNextResource(territoryAlias,NULL,&status);
        const char *aliasFrom = ures_getKey(res);
        UnicodeString* aliasFromStr = new UnicodeString(aliasFrom);
        UnicodeString aliasTo = ures_getUnicodeString(res,&status);

        Region *aliasToRegion = (Region *) uhash_get(regionIDMap,&aliasTo);
        Region *aliasFromRegion = (Region *)uhash_get(regionIDMap,aliasFromStr);

        if ( aliasToRegion != NULL && aliasFromRegion == NULL ) { // This is just an alias from some string to a region
            uhash_put(regionAliases,(void *)aliasFromStr, (void *)aliasToRegion,&status);
        } else {
           if ( aliasFromRegion == NULL ) { // Deprecated region code not in the master codes list - so need to create a deprecated region for it.
                aliasFromRegion = new Region();
                aliasFromRegion->idStr.setTo(*aliasFromStr);
                aliasFromRegion->idStr.extract(0,aliasFromRegion->idStr.length(),aliasFromRegion->id,sizeof(aliasFromRegion->id),US_INV);
                uhash_put(regionIDMap,(void *)&(aliasFromRegion->idStr),(void *)aliasFromRegion,&status);
                Formattable result;
                UErrorCode ps = U_ZERO_ERROR;
                df->parse(aliasFromRegion->idStr,result,ps);
                if ( U_SUCCESS(ps) ) {
                    aliasFromRegion->code = result.getLong(); // Convert string to number
                    uhash_iput(numericCodeMap,aliasFromRegion->code,(void *)aliasFromRegion,&status);
                } else {
                    aliasFromRegion->code = Region::UNDEFINED_NUMERIC_CODE;
                }
                aliasFromRegion->type = URGN_DEPRECATED;
                addRegion(aliasFromRegion);
            } else {
                aliasFromRegion->type = URGN_DEPRECATED;
            }
            delete aliasFromStr;

            aliasFromRegion->preferredValues = new UVector(NULL, uhash_compareChars, status);
            UnicodeString currentRegion;
            currentRegion.remove();
            for (int32_t i = 0 ; i < aliasTo.length() ; i++ ) {
                if ( aliasTo.charAt(i) != 0x0020 ) {
                    currentRegion.append(aliasTo.charAt(i));
                }
                if ( aliasTo.charAt(i) == 0x0020 || i+1 == aliasTo.length() ) {
                    Region *target = (Region *)uhash_get(regionIDMap,(void *)&currentRegion);
                    if (target) {
                        aliasFromRegion->preferredValues->addElement((void *)target->id,status);
                    }
                    currentRegion.remove();
                }
            }
        }
    }

    // Process the code mappings - This will allow us to assign numeric codes to most of the territories.
    while ( ures_hasNext(codeMappings) ) {
        UResourceBundle *mapping = ures_getNextResource(codeMappings,NULL,&status);
        if ( ures_getType(mapping) == URES_ARRAY && ures_getSize(mapping) == 3) {
            UnicodeString codeMappingID = ures_getUnicodeStringByIndex(mapping,0,&status);
            UnicodeString codeMappingNumber = ures_getUnicodeStringByIndex(mapping,1,&status);
            UnicodeString codeMapping3Letter = ures_getUnicodeStringByIndex(mapping,2,&status);

            Region *r = (Region *)uhash_get(regionIDMap,(void *)&codeMappingID);
            if ( r ) {
                Formattable result;
                UErrorCode ps = U_ZERO_ERROR;
                df->parse(codeMappingNumber,result,ps);
                if ( U_SUCCESS(ps) ) {
                    r->code = result.getLong(); // Convert string to number
                    uhash_iput(numericCodeMap,r->code,(void *)r,&status);
                }
                UnicodeString *code3 = new UnicodeString(codeMapping3Letter);
                uhash_put(regionAliases,(void *)code3, (void *)r,&status);
            }                    
        }
    }

    // Now fill in the special cases for WORLD, UNKNOWN, CONTINENTS, and GROUPINGS
    Region *r;
    r = (Region *) uhash_get(regionIDMap,(void *)&WORLD_ID);
    if ( r ) {
        r->type = URGN_WORLD;
    }

    r = (Region *) uhash_get(regionIDMap,(void *)&UNKNOWN_REGION_ID);
    if ( r ) {
        r->type = URGN_UNKNOWN;
    }

    for ( int32_t i = 0 ; i < continents->size() ; i++ ) {
        r = (Region *) uhash_get(regionIDMap,(void *)continents->elementAt(i));
        if ( r ) {
            r->type = URGN_CONTINENT;
        }
    }
    delete continents;

    for ( int32_t i = 0 ; i < groupings->size() ; i++ ) {
        r = (Region *) uhash_get(regionIDMap,(void *)groupings->elementAt(i));
        if ( r ) {
            r->type = URGN_GROUPING;
        }
    }
    delete groupings;

    // Special case: The region code "QO" (Outlying Oceania) is a subcontinent code added by CLDR
    // even though it looks like a territory code.  Need to handle it here.

    r = (Region *) uhash_get(regionIDMap,(void *)&OUTLYING_OCEANIA_REGION_ID);
    if ( r ) {
        r->type = URGN_SUBCONTINENT;
    }

    // Load territory containment info from the supplemental data.
    while ( ures_hasNext(territoryContainment) ) {
        UResourceBundle *mapping = ures_getNextResource(territoryContainment,NULL,&status);
        const char *parent = ures_getKey(mapping);
        UnicodeString parentStr = UnicodeString(parent);
        Region *parentRegion = (Region *) uhash_get(regionIDMap,(void *)&parentStr);

        for ( int j = 0 ; j < ures_getSize(mapping); j++ ) {
            UnicodeString child = ures_getUnicodeStringByIndex(mapping,j,&status);
            Region *childRegion = (Region *) uhash_get(regionIDMap,(void *)&child);
            if ( parentRegion != NULL && childRegion != NULL ) {                    

                // Add the child region to the set of regions contained by the parent
                if (parentRegion->containedRegions == NULL) {
                    parentRegion->containedRegions = new UVector(NULL, uhash_compareChars, status);
                }
                parentRegion->containedRegions->addElement((void *)childRegion->id,status);

                // Set the parent region to be the containing region of the child.
                // Regions of type GROUPING can't be set as the parent, since another region
                // such as a SUBCONTINENT, CONTINENT, or WORLD must always be the parent.
                if ( parentRegion->type != URGN_GROUPING) {
                    childRegion->containingRegion = parentRegion;
                }
            }
        }
    }     

    // Create the availableRegions lists

    for ( int32_t i = 0 ; i < regions->size() ; i++ ) {
        Region *ar = (Region *)regions->elementAt(i);
        addAvailableRegion(ar,ar->type);
    }

    regionDataIsLoaded = true;

    ures_close(territoryContainment);
    ures_close(worldContainment);
    ures_close(groupingContainment);

    ures_close(codeMappings);
    ures_close(rb2);
    ures_close(territoryAlias);
    ures_close(regionCodes);
    ures_close(rb);

    delete df;
}



/*
 * Default constructor.  Use factory methods only.
 * @internal
 */

Region::Region () {
        idStr.remove();
        code = UNDEFINED_NUMERIC_CODE;
        type = URGN_UNKNOWN;
        containingRegion = NULL;
        containedRegions = NULL;
        preferredValues = NULL;
}


/**
 * Returns true if the two regions are equal.
 */
UBool U_EXPORT2
Region::operator==(const Region &that) const {
    return (idStr == that.idStr);
}

/**
 * Returns true if the two regions are NOT equal; that is, if operator ==() returns false.
 */
UBool U_EXPORT2
Region::operator!=(const Region &that) const {
        return (idStr != that.idStr);
}
 
/**
 * Returns a pointer to a Region using the given region code.  The region code can be either 2-letter ISO code,
 * 3-letter ISO code,  UNM.49 numeric code, or other valid Unicode Region Code as defined by the LDML specification.
 * The identifier will be canonicalized internally using the supplemental metadata as defined in the CLDR.
 * If the region code is NULL or not recognized, the appropriate error code will be set ( U_ILLEGAL_ARGUMENT_ERROR )
 */
const Region* U_EXPORT2
Region::getInstance(const char *region_code, UErrorCode &status) {

    if ( !region_code ) {
        status = U_ILLEGAL_ARGUMENT_ERROR;
        return NULL;
    }

    loadRegionData();

    UnicodeString regionCodeString = UnicodeString(region_code);
    Region *r = (Region *)uhash_get(regionIDMap,(void *)&regionCodeString);

    if ( !r ) {
        r = (Region *)uhash_get(regionAliases,(void *)&regionCodeString);
    }

    if ( !r ) { // Unknown region code
        status = U_ILLEGAL_ARGUMENT_ERROR;
        return NULL;
    }

    if ( r->type == URGN_DEPRECATED && r->preferredValues->size() == 1) {
        StringEnumeration *pv = r->getPreferredValues();
        pv->reset(status);
        const UnicodeString *ustr = pv->snext(status);
        r = (Region *)uhash_get(regionIDMap,(void *)ustr);
    }

    return r;

}

/**
 * Returns a pointer to a Region using the given numeric region code. If the numeric region code is not recognized,
 * the appropriate error code will be set ( U_ILLEGAL_ARGUMENT_ERROR ).
 */
const Region * U_EXPORT2
Region::getInstance (int32_t code, UErrorCode &status) {

    loadRegionData();

    Region *r = (Region *)uhash_iget(numericCodeMap,code);

    if ( !r ) { // Just in case there's an alias that's numeric, try to find it.
        UErrorCode fs = U_ZERO_ERROR;
        UnicodeString pat = UNICODE_STRING_SIMPLE("00#");
        DecimalFormat *df = new DecimalFormat(pat,fs);
        
        UnicodeString id;
        id.remove();
        df->format(code,id);
        delete df;
        r = (Region *)uhash_get(regionAliases,&id);
    }

    if ( !r ) {
        status = U_ILLEGAL_ARGUMENT_ERROR;
        return NULL;
    }

    if ( r->type == URGN_DEPRECATED && r->preferredValues->size() == 1) {
        StringEnumeration *pv = r->getPreferredValues();
        pv->reset(status);
        const UnicodeString *ustr = pv->snext(status);
        r = (Region *)uhash_get(regionIDMap,(void *)ustr);
    }

    return r;
}


/**
 * Returns an enumeration over the IDs of all known regions that match the given type.
 */
StringEnumeration * U_EXPORT2
Region::getAvailable(URegionType type) {
    
    loadRegionData();
    UErrorCode status = U_ZERO_ERROR;
    return new RegionNameEnumeration(availableRegions[type],status);

    return NULL; 
}
   
/**
 * Returns a pointer to the region that contains this region.  Returns NULL if this region is code "001" (World)
 * or "ZZ" (Unknown region). For example, calling this method with region "IT" (Italy) returns the
 * region "039" (Southern Europe).
 */
const Region * U_EXPORT2
Region::getContainingRegion() const {
    loadRegionData();
    return containingRegion;
}

/**
 * Return a pointer to the region that geographically contains this region and matches the given type,
 * moving multiple steps up the containment chain if necessary.  Returns NULL if no containing region can be found
 * that matches the given type. Note: The URegionTypes = "URGN_GROUPING", "URGN_DEPRECATED", or "URGN_UNKNOWN"
 * are not appropriate for use in this API. NULL will be returned in this case. For example, calling this method
 * with region "IT" (Italy) for type "URGN_CONTINENT" returns the region "150" ( Europe ).
 */
const Region* U_EXPORT2
Region::getContainingRegion(URegionType type) const {
    loadRegionData();
    if ( containingRegion == NULL ) {
        return NULL;
    }

    if ( containingRegion->type == type ) {
        return containingRegion;
    } else {
        return containingRegion->getContainingRegion(type);
    }
}

/**
 * Return an enumeration over the IDs of all the regions that are immediate children of this region in the
 * region hierarchy. These returned regions could be either macro regions, territories, or a mixture of the two,
 * depending on the containment data as defined in CLDR.  This API may return NULL if this region doesn't have
 * any sub-regions. For example, calling this method with region "150" (Europe) returns an enumeration containing
 * the various sub regions of Europe - "039" (Southern Europe) - "151" (Eastern Europe) - "154" (Northern Europe)
 * and "155" (Western Europe).
 */
StringEnumeration * U_EXPORT2
Region::getContainedRegions() const {
    loadRegionData();
    UErrorCode status = U_ZERO_ERROR;
    return new RegionNameEnumeration(containedRegions,status);
}

/**
 * Returns an enumeration over the IDs of all the regions that are children of this region anywhere in the region
 * hierarchy and match the given type.  This API may return an empty enumeration if this region doesn't have any
 * sub-regions that match the given type. For example, calling this method with region "150" (Europe) and type
 * "URGN_TERRITORY" returns a set containing all the territories in Europe ( "FR" (France) - "IT" (Italy) - "DE" (Germany) etc. )
 */
StringEnumeration * U_EXPORT2
Region::getContainedRegions( URegionType type ) const {
    loadRegionData();

    UErrorCode status = U_ZERO_ERROR;
    UVector *result = new UVector(uprv_deleteUObject, uhash_compareChars, status);
 
    StringEnumeration *cr = getContainedRegions();

    for ( int32_t i = 0 ; i < cr->count(status) ; i++ ) {
        const char *id = cr->next(NULL,status);
        const Region *r = Region::getInstance(id,status);
        if ( r->getType() == type ) {
            result->addElement((void *)r->id,status);
        } else {
            StringEnumeration *children = r->getContainedRegions(type);
            for ( int32_t j = 0 ; j < children->count(status) ; j++ ) {
                const char *id2 = children->next(NULL,status);
                const Region *r2 = Region::getInstance(id2,status);
                result->addElement((void *)r2->id,status);
            }
        }
    }
    return new RegionNameEnumeration(result,status);
}
 
/**
 * Returns true if this region contains the supplied other region anywhere in the region hierarchy.
 */
UBool U_EXPORT2
Region::contains(const Region &other) const {
    loadRegionData();

    if (!containedRegions) {
          return FALSE;
    }
    if (containedRegions->contains((void *)other.id)) {
        return TRUE;
    } else {
        for ( int32_t i = 0 ; i < containedRegions->size() ; i++ ) {
            UErrorCode status = U_ZERO_ERROR;
            const Region *cr = Region::getInstance((const char *)containedRegions->elementAt(i),status);
            if ( cr && cr->contains(other) ) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/**
 * For deprecated regions, return an enumeration over the IDs of the regions that are the preferred replacement
 * regions for this region.  Returns NULL for a non-deprecated region.  For example, calling this method with region
 * "SU" (Soviet Union) would return a list of the regions containing "RU" (Russia), "AM" (Armenia), "AZ" (Azerbaijan), etc...
 */
StringEnumeration * U_EXPORT2
Region::getPreferredValues() const {
    loadRegionData();
    UErrorCode status = U_ZERO_ERROR;
    if ( type == URGN_DEPRECATED ) {
        return new RegionNameEnumeration(preferredValues,status);
    } else {
        return NULL;
    }
}
 

/**
 * Return this region's canonical region code.
 */
const char * U_EXPORT2
Region::getRegionCode() const {
    return id;
}

/**
 * Return this region's numeric code. Returns UNDEFINED_NUMERIC_CODE (-1) if the given region does not have a numeric code assigned to it.
 */
int32_t U_EXPORT2
Region::getNumericCode() const {
    return code;
}

/**
 * Returns the region type of this region.
 */
URegionType U_EXPORT2
Region::getType() const {
    return type;
}

RegionNameEnumeration::RegionNameEnumeration(UVector *fNameList, UErrorCode& /*status*/) {
    pos=0;
    fRegionNames = fNameList;
}

const char *
RegionNameEnumeration::next(int32_t *resultLength, UErrorCode& status) {
    if (U_SUCCESS(status) && pos < fRegionNames->size()) {
        if (resultLength != NULL) {
            *resultLength = uprv_strlen((const char *)fRegionNames->elementAt(pos));
        }
        return (const char *)fRegionNames->elementAt(pos++);
    }
    return NULL;
}

const UnicodeString*
RegionNameEnumeration::snext(UErrorCode& status) { 
    int32_t resultLength=0;
    const char *s=next(&resultLength, status);
    return setChars(s, resultLength, status);
}

void
RegionNameEnumeration::reset(UErrorCode& /*status*/) {
    pos=0;
}

int32_t
RegionNameEnumeration::count(UErrorCode& /*status*/) const {
    return (fRegionNames==NULL) ? 0 : fRegionNames->size();
}

RegionNameEnumeration::~RegionNameEnumeration() {
    delete fRegionNames;
}

U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_FORMATTING */

//eof
