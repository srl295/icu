/**
*******************************************************************************
* Copyright (C) 1996-2001, International Business Machines Corporation and    *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
* $Source: /xsrl/Nsvn/icu/unicodetools/com/ibm/text/UCD/DerivedProperty.java,v $
* $Date: 2003/02/25 23:38:23 $
* $Revision: 1.18 $
*
*******************************************************************************
*/

package com.ibm.text.UCD;
import com.ibm.text.utility.*;
import com.ibm.icu.text.UTF16;
import com.ibm.icu.text.UnicodeSet;
import java.util.*;
import java.io.PrintWriter;

public final class DerivedProperty implements UCD_Types {
  
    UCD ucdData;
    Normalizer nfc;
    Normalizer nfd;
    Normalizer nfkc;
    Normalizer nfkd;
    Normalizer[] nf = new Normalizer[4];
    UnicodeSet XID_Start_Set = new UnicodeSet();
    UnicodeSet XID_Continue_Set = new UnicodeSet();
    
    // ADD CONSTANT to UCD_TYPES
    
    static public UnicodeProperty make(int derivedPropertyID) {
        return make(derivedPropertyID, Default.ucd);
    }
    
    static public UnicodeProperty make(int derivedPropertyID, UCD ucd) {
        if (derivedPropertyID < 0 || derivedPropertyID >= DERIVED_PROPERTY_LIMIT) return null;
        DerivedProperty dp = getCached(ucd);
        return dp.dprops[derivedPropertyID];
    }
    
    ///////////////////////////////////////////////////////////
    
    static Map cache = new HashMap();
    static UCD lastUCD = null;
    static DerivedProperty lastValue = null;
    
    private static DerivedProperty getCached(UCD ucd) {
        if (ucd.equals(lastUCD)) return lastValue;
        DerivedProperty dp = (DerivedProperty) cache.get(ucd);
        if (dp == null) {
            dp = new DerivedProperty(ucd);
            cache.put(ucd, dp);
        }
        lastUCD = ucd;
        lastValue = dp;
        return dp;
    }
    
    /*
    public String getHeader(int propNumber) {
        UnicodeProperty dp = dprops[propNumber];
        if (dp != null) return dp.getHeader();
        else return "Unimplemented!!";
    }

    public String getName(int propNumber, byte style) {
        UnicodeProperty dp = dprops[propNumber];
        if (dp != null) return dp.getName(style);
        else return "Unimplemented!!";
    }

    public String getValue(int cp, int propNumber) {
        UnicodeProperty dp = dprops[propNumber];
        if (dp != null) return dp.getValue(cp);
        else return "Unimplemented!!";
    }
    
    public boolean isTest(int propNumber) {
        if (!isDefined(propNumber)) return false;
        return dprops[propNumber].isTest();
    }
    
    public boolean hasProperty(int cp, int propNumber) {
        if (!isDefined(propNumber)) return false;
        return dprops[propNumber].hasProperty(cp);
    }
    
    public boolean valueVaries(int propNumber) {
        return dprops[propNumber].valueVaries();
    }
    /*
    public String getValue(int cp, int propNumber) {
        return dprops[propNumber].getValue(int cp);
    }
    */
    private UnicodeProperty[] dprops = new UnicodeProperty[50];

    static final String[] CaseNames = {
                "Uppercase", 
                "Lowercase", 
                "Mixedcase"};
    
    class ExDProp extends UnicodeProperty {
        Normalizer nfx;
        ExDProp(int i) {
            type = DERIVED_NORMALIZATION;
            nfx = nf[i];
            name = "Expands_On_" + nfx.getName();
            shortName = "XO_" + nfx.getName();
            header = "# Derived Property: " + name
                + "\r\n#   Generated according to UAX #15."
                + "\r\n#   Characters whose normalized length is not one."
                + "\r\n#   WARNING: Normalization of STRINGS must use the algorithm in UAX #15 because characters may interact."
                + "\r\n#            The length of a normalized string is not necessarily the sum of the lengths of the normalized characters!";
        }
        public boolean hasValue(int cp) {
            if (ucdData.getDecompositionType(cp) == NONE) return false;
            String norm = nfx.normalize(cp);
            if (UTF16.countCodePoint(norm) != 1) return true;
            return false;
        }
    };
    
    class NF_UnsafeStartProp extends UnicodeProperty {
        Normalizer nfx;
        //int prop;
        
        NF_UnsafeStartProp(int i) {
            isStandard = false;
            type = DERIVED_NORMALIZATION;
            nfx = nf[i];
            name = nfx.getName() + "_UnsafeStart";
            shortName = nfx.getName() + "_SS";
            header = "# Derived Property: " + name
                + "\r\n#   Generated according to UAX #15."
                + "\r\n#   Characters that are cc==0, BUT which may interact with previous characters."
                ;
        }
        public boolean hasValue(int cp) {
            if (ucdData.getCombiningClass(cp) != 0) return false;
            String norm = nfx.normalize(cp);
            int first = UTF16.charAt(norm, 0);
            if (ucdData.getCombiningClass(first) != 0) return true;
            if (nfx.isComposition()
                && dprops[NFC_TrailingZero].hasValue(first)) return true; // 1,3 == composing
            return false;
        }
    };
    

    /*
    class HangulSyllableType extends UnicodeProperty {
        Normalizer nfx;
        //int prop;
        
        HangulSyllableType(int i) {
            isStandard = false;
            type = DERIVED_NORMALIZATION;
            nfx = nf[i];
            name = nfx.getName() + "_UnsafeStart";
            shortName = nfx.getName() + "_SS";
            header = "# Derived Property: " + name
                + "\r\n#   Generated according to UAX #15."
                + "\r\n#   Characters that are cc==0, BUT which may interact with previous characters."
                ;
        }
        public boolean hasValue(int cp) {
            if (ucdData.getCombiningClass(cp) != 0) return false;
            String norm = nfx.normalize(cp);
            int first = UTF16.charAt(norm, 0);
            if (ucdData.getCombiningClass(first) != 0) return true;
            if (nfx.isComposition()
                && dprops[NFC_TrailingZero].hasValue(first)) return true; // 1,3 == composing
            return false;
        }
    };
    */
    
    
    class NFC_Prop extends UnicodeProperty {
        BitSet bitset;
        boolean filter = false;
        boolean keepNonZero = true;
        
        NFC_Prop(int i) {
            isStandard = false;
            type = DERIVED_NORMALIZATION;
            BitSet[] bitsets = new BitSet[3];
            switch(i) {
                case NFC_Leading: bitsets[0] = bitset = new BitSet(); break;
                case NFC_Resulting: bitsets[2] = bitset = new BitSet(); break;
                case NFC_TrailingZero: keepNonZero = false; // FALL THRU
                case NFC_TrailingNonZero: bitsets[1] = bitset = new BitSet(); break;
            }
            filter = bitsets[1] != null;
            nfc.getCompositionStatus(bitsets[0], bitsets[1], bitsets[2]);
            
            name = Names[i-NFC_Leading];
            shortName = SNames[i-NFC_Leading];
            header = "# Derived Property: " + name
                + "\r\n#   " + Description[i-NFC_Leading]
                + "\r\n#   NFKC characters are the same, after subtracting the NFKD = NO values."
                + "\r\n#   Generated according to UAX #15."
                + "\r\n#   WARNING: Normalization of STRINGS must use the algorithm in UAX #15 because characters may interact."
                + "\r\n#            The length of a normalized string is not necessarily the sum of the lengths of the normalized characters!";
        }
        public boolean hasValue(int cp) {
            boolean result = bitset.get(cp);
            if (result && filter) {
                result = (ucdData.getCombiningClass(cp) != 0) == keepNonZero;
            }
            return result;
        }
        final String[] Names = {"NFC_Leading", "NFC_TrailingNonZero", "NFC_TrailingZero", "NFC_Resulting"};
        final String[] SNames = {"NFC_L", "NFC_TNZ", "NFC_TZ", "NFC_R"};
        final String[] Description = {
            "Characters that can combine with following characters in NFC",
            "Characters that can combine with previous characters in NFC, and have non-zero combining class",
            "Characters that can combine with previous characters in NFC, and have zero combining class",
            "Characters that can result from a combination of other characters in NFC",
        };
    };
    
    class GenDProp extends UnicodeProperty {
        Normalizer nfx;
        Normalizer nfComp = null;
        
        GenDProp (int i) {
            isStandard = false;
            setValueType(NON_ENUMERATED);
            type = DERIVED_NORMALIZATION;
            nfx = nf[i];
            name = nfx.getName();
            String compName = "the character itself";
            
            if (i == NFKC || i == NFD) {
                name += "-NFC";
                nfComp = nfc;
                compName = "NFC for the character";
            } else if (i == NFKD) {
                name += "-NFD";
                nfComp = nfd;
                compName = "NFD for the character";
            }
            header = "# Derived Property: " + name              
                + "\r\n#   Lists characters in normalized form " + nfx.getName() + "."
                + "\r\n#   Only those characters whith normalized forms are DIFFERENT from " + compName + " are listed!"
                + "\r\n#   WARNING: Normalization of STRINGS must use the algorithm in UAX #15 because characters may interact."
                + "\r\n#            It is NOT sufficient to replace characters one-by-one with these results!";
        }
        
        int cacheCp = 0;
        String cacheStr = "";
        
        public String getValue(int cp, byte style) {
            if (cacheCp == cp) return cacheStr;
            cacheCp = cp;
            cacheStr = "";
            
            if (ucdData.getDecompositionType(cp) != NONE) {
                String cps = UTF32.valueOf32(cp);
                String comp = cps;
                if (nfComp != null) {
                    comp = nfComp.normalize(comp);
                }
                String normal = nfx.normalize(cps);
                if (!comp.equals(normal)) {
                    String norm = Utility.hex(normal);
                    String pad = Utility.repeat(" ", 14-norm.length());
                    cacheStr = name + "; " + norm + pad;
                }
            }
            
            return cacheStr;
            //if (cp >= 0xAC00 && cp <= 0xD7A3) return true;
            //System.out.println(Utility.hex(cps) + " => " + Utility.hex(nf[i-4].normalize(cps)));
        } // default
        public boolean hasValue(int cp) { return getValue(cp).length() != 0; }
    };
    
    class CaseDProp extends UnicodeProperty {
        byte val;
        CaseDProp (int i) {
            type = DERIVED_CORE;
                isStandard = false;
            val = (i == Missing_Uppercase ? Lu : i == Missing_Lowercase ? Ll : Lt);
            name = "Possible_Missing_" + CaseNames[i-Missing_Uppercase];
            header = "# Derived Property: " + name
            + "\r\n#  Generated from: NFKD has >0 " + CaseNames[i-Missing_Uppercase] + ", no other cases";
        }
        public boolean hasValue(int cp) {
            byte cat = ucdData.getCategory(cp);
            if (cat == val
            || val != Lt && ucdData.getBinaryProperty(cp, Other_Uppercase)) return false;
            byte xCat = getDecompCat(cp);
            if (xCat == val) return true;
            return false;
        }
    };
    
    class QuickDProp extends UnicodeProperty {
        String NO;
        String MAYBE;
        Normalizer nfx;
        QuickDProp (int i) {
            setValueType((i == NFC || i == NFKC) ? ENUMERATED : BINARY);
            type = DERIVED_NORMALIZATION;
            nfx = nf[i];
            NO = nfx.getName() + "_NO";
            MAYBE = nfx.getName() + "_MAYBE";
            name = nfx.getName() + "_QuickCheck";
            shortName = nfx.getName() + "_QC";
            header = "# Derived Property: " + name
            + "\r\n#  Generated from computing decomposibles"
            + ((i == NFC || i == NFKC)
                ? " (and characters that may compose with previous ones)" : "");
        }
                
        public String getValue(int cp, byte style) { 
            if (!nfx.isNormalized(cp)) return NO;
            else if (nfx.isTrailing(cp)) return MAYBE;
            else return "";
        }
        
		public String getListingValue(int cp) {
    		return getValue(cp, LONG);
    	}
        
        public boolean hasValue(int cp) { return getValue(cp).length() != 0; }
    };

    private DerivedProperty(UCD ucd) {
        ucdData = ucd;
    
        nfd = nf[NFD] = new Normalizer(Normalizer.NFD, ucdData.getVersion());
        nfc = nf[NFC] = new Normalizer(Normalizer.NFC, ucdData.getVersion());
        nfkd = nf[NFKD] = new Normalizer(Normalizer.NFKD, ucdData.getVersion());
        nfkc = nf[NFKC] = new Normalizer(Normalizer.NFKC, ucdData.getVersion());

        for (int i = ExpandsOnNFD; i <= ExpandsOnNFKC; ++i) {
            dprops[i] = new ExDProp(i-ExpandsOnNFD);
        }
        
        for (int i = GenNFD; i <= GenNFKC; ++i) {
            dprops[i] = new GenDProp(i-GenNFD);
        }
        
        for (int i = NFC_Leading; i <= NFC_Resulting; ++i) {
            dprops[i] = new NFC_Prop(i);
        }
        
        for (int i = NFD_UnsafeStart; i <= NFKC_UnsafeStart; ++i) {
            dprops[i] = new NF_UnsafeStartProp(i-NFD_UnsafeStart);
        }
        
        dprops[ID_Start] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                name = "ID_Start";
                shortName = "IDS";
                header = "# Derived Property: " + name
                    + "\r\n#  Characters that can start an identifier."
                    + "\r\n#  Generated from Lu+Ll+Lt+Lm+Lo+Nl+ID_Start_Exceptions";
            }
            public boolean hasValue(int cp) {
                return ucdData.isIdentifierStart(cp);
            }
        };
        
        dprops[ID_Continue_NO_Cf] = new UnicodeProperty() {
            {
                name = "ID_Continue";
                type = DERIVED_CORE;
                shortName = "IDC";
                header = "# Derived Property: " + name
                    + "\r\n#  Characters that can continue an identifier."
                    + "\r\n#  Generated from: ID_Start + Mn+Mc+Nd+Pc"
                    + "\r\n#  NOTE: Cf characters should be filtered out.";
            }
            public boolean hasValue(int cp) {
                return ucdData.isIdentifierContinue_NO_Cf(cp);
            }
        };
        
        StringBuffer tempBuf = new StringBuffer();
        
        //System.out.println("Deriving data for XID");
        // special hack for middle dot
        XID_Continue_Set.add(0x00B7);
        //System.out.println("Adding (2)" + ucdData.getCodeAndName(0x00B7));
        
        
        for (int cp = 0; cp < 0x10FFFF; ++cp) {
            // skip cases that can't matter
            if (!ucdData.isAssigned(cp)) continue;
            
            // find out normal status
            int status = 0;
            if (ucdData.isIdentifierStart(cp)) status = 1;
            else if (ucdData.isIdentifierContinue_NO_Cf(cp)) status = 2;
            
            if (status != 0 && !nfkd.isNormalized(cp)) {
                // now find out NFKD status
                // if it is <start><extend>*, then it is start
                // else if it is <extend>*, then it is extend
                // else it is nothing
                int status2 = 0;
                tempBuf.setLength(0);
                nfkd.normalize(UTF32.valueOf32(cp), tempBuf);
                for (int i = 0; i < tempBuf.length(); i += UTF32.count16(cp)) {
                    int cp2 = UTF32.char32At(tempBuf, i);
                    if (i == 0) {
                        if (ucdData.isIdentifierStart(cp2)) status2 = 1;
                        else if (ucdData.isIdentifierContinue_NO_Cf(cp2)) status2 = 2;
                        else {
                            status2 = 0;
                            break;
                        }
                    } else if (!ucdData.isIdentifierContinue_NO_Cf(cp2) && cp2 != 0xB7) {
                        status2 = 0;
                        break;
                    }
                }
                
                // Now see if the statuses are compatible.
                if (status != status2) {
                    //System.out.println("Need to do something with:");
                    //System.out.println("  " + status + ": " + ucdData.getCodeAndName(cp));
                    //System.out.println("  " + status2 + ": " + ucdData.getCodeAndName(tempBuf.toString()));
                    if (status2 == 0) status = 0;
                    else if (status2 > status) status = status2;
                    //System.out.println("  " + status + ": " + ucdData.getCodeAndName(cp));
                }
            }
            
            if (status == 1) XID_Start_Set.add(cp);
            if (status != 0) XID_Continue_Set.add(cp);
        }
        
        dprops[Mod_ID_Start] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                name = "XID_Start";
                shortName = "XIDS";
                header = "# Derived Property: " + name
                    + "\r\n#  ID_Start modified for closure under NFKx"
                    + "\r\n#  Modified as described in UAX #15"
                    + "\r\n#  NOTE: Does NOT remove the non-NFKx characters."
                    + "\r\n#        Merely ensures that if isIdentifer(string) then isIdentifier(NFKx(string))";
            }
            public boolean hasValue(int cp) {
                return XID_Start_Set.contains(cp);
            }
        };
        
        dprops[Mod_ID_Continue_NO_Cf] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                name = "XID_Continue";
                shortName = "XIDC";
                header = "# Derived Property: " + name
                    + "\r\n#  Mod_ID_Continue modified for closure under NFKx"
                    + "\r\n#  Modified as described in UAX #15"
                    + "\r\n#  NOTE: Cf characters should be filtered out."
                    + "\r\n#  NOTE: Does NOT remove the non-NFKx characters."
                    + "\r\n#        Merely ensures that if isIdentifer(string) then isIdentifier(NFKx(string))";
            }
            public boolean hasValue(int cp) {
                return XID_Continue_Set.contains(cp);
            }
        };
        
        dprops[PropMath] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                name = "Math";
                shortName = name;
                header = "# Derived Property: " + name
                    + "\r\n#  Generated from: Sm + Other_Math";
            }
            public boolean hasValue(int cp) {
                byte cat = ucdData.getCategory(cp);
                if (cat == Sm
                || ucdData.getBinaryProperty(cp,Math_Property)) return true;
                return false;
            }
        };
        
        dprops[PropAlphabetic] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
               name = "Alphabetic";
                shortName = "Alpha";
                header = "# Derived Property: " + name
                    + "\r\n#  Generated from: Lu+Ll+Lt+Lm+Lo+Nl + Other_Alphabetic";
            }
            public boolean hasValue(int cp) {
                byte cat = ucdData.getCategory(cp);
                if (cat == Lu || cat == Ll || cat == Lt || cat == Lm || cat == Lo || cat == Nl
                || ucdData.getBinaryProperty(cp, Alphabetic)) return true;
                return false;
            }
        };
        
        dprops[PropLowercase] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                name = "Lowercase";
                shortName = "Lower";
                header = "# Derived Property: " + name
                    + "\r\n#  Generated from: Ll + Other_Lowercase";
            }
            public boolean hasValue(int cp) {
                byte cat = ucdData.getCategory(cp);
                if (cat == Ll
                || ucdData.getBinaryProperty(cp, Other_Lowercase)) return true;
                return false;
            }
        };
        
        dprops[PropUppercase] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                name = "Uppercase";
                shortName = "Upper";
                header = "# Derived Property: " + name
                    + "\r\n#  Generated from: Lu + Other_Uppercase";
            }
            public boolean hasValue(int cp) {
                byte cat = ucdData.getCategory(cp);
                if (cat == Lu
                || ucdData.getBinaryProperty(cp, Other_Uppercase)) return true;
                return false;
            }
        };
        
        for (int i = Missing_Uppercase; i <= Missing_Mixedcase; ++i) {
            dprops[i] = new CaseDProp(i);
        }
        
/*
(3) Singleton Decompositions: characters that  can be derived from the UnicodeData file by
including all characters whose canonical decomposition consists of a single character.
(4) Non-Starter Decompositions: characters that  can be derived from the UnicodeData
file by including all characters whose canonical decomposition consists of a sequence
of characters, the first of which has a non-zero combining class.
*/
        dprops[FullCompExclusion] = new UnicodeProperty() {
            {
                type = DERIVED_NORMALIZATION;
                name = "Full_Composition_Exclusion";
                shortName = "Comp_Ex";
                defaultValueStyle = defaultPropertyStyle = SHORT;
                header = "# Derived Property: " + name
                    + "\r\n#  Generated from: Composition Exclusions + Singletons + Non-Starter Decompositions";
            }
            public boolean hasValue(int cp) {
                if (!ucdData.isRepresented(cp)) return false;
                byte dtype = ucdData.getDecompositionType(cp);
                if (dtype != CANONICAL) return false;

                if (isCompEx(cp)) return true;
                return false;
            }
		    /*public String getListingValue(int cp) {
    		    return "Comp_Ex";
    	    }*/
            /*
			public String getListingValue(int cp) {
        		if (getValueType() != BINARY) return getValue(cp, SHORT);
        		return getProperty(SHORT);
			}
			*/
        };
        
        dprops[FullCompInclusion] = new UnicodeProperty() {
            {
                isStandard = false;
                type = DERIVED_NORMALIZATION;
                name = "Full_Composition_Inclusion";
                shortName = "Comp_In";
                defaultValueStyle = defaultPropertyStyle = SHORT;
                header = "# Derived Property: " + name
                    + ": Full Composition Inclusion"
                    + "\r\n#  characters with Canonical Decompositions MINUS Full Composition Exclusion";
            }
            public boolean hasValue(int cp) {
                if (!ucdData.isRepresented(cp)) return false;
                byte dtype = ucdData.getDecompositionType(cp);
                if (dtype != CANONICAL) return false;

                if (isCompEx(cp)) return true;
                return false;
            }
        };
        
        dprops[FC_NFKC_Closure] = new UnicodeProperty() {
            {
                type = DERIVED_NORMALIZATION;
                setValueType(NON_ENUMERATED);
                name = "FC_NFKC_Closure";
                shortName = "FC_NFKC";
                header = "# Derived Property: " + name
                    + "\r\n#  Generated from computing: b = NFKC(Fold(a)); c = NFKC(Fold(b));"
                    + "\r\n#  Then if (c != b) add the mapping from a to c to the set of"
                    + "\r\n#  mappings that constitute the FC_NFKC_Closure list";
            }
            public String getValue(int cp, byte style) { 
                if (!ucdData.isRepresented(cp)) return "";
                String b = nfkc.normalize(fold(cp));
                String c = nfkc.normalize(fold(b));
                if (c.equals(b)) return "";
                return "FNC; " + Utility.hex(c);
            } // default
            public boolean hasValue(int cp) { return getValue(cp).length() != 0; }
        };
        
        dprops[FC_NFC_Closure] = new UnicodeProperty() {
            {
                type = DERIVED_NORMALIZATION;
                isStandard = false;
                name = "FC_NFC_Closure";
                setValueType(NON_ENUMERATED);
                shortName = "FC_NFC";
                header = "# Derived Property: " + name
                    + "\r\n#  Generated from computing: b = NFC(Fold(a)); c = NFC(Fold(b));"
                    + "\r\n#  Then if (c != b) add the mapping from a to c to the set of"
                    + "\r\n#  mappings that constitute the FC_NFC_Closure list";
            }
            public String getValue(int cp, byte style) { 
                if (!ucdData.isRepresented(cp)) return "";
                String b = nfc.normalize(fold(cp));
                String c = nfc.normalize(fold(b));
                if (c.equals(b)) return "";
                return "FN; " + Utility.hex(c);
            } // default
            public boolean hasValue(int cp) { return getValue(cp).length() != 0; }
        };
        
        for (int i = QuickNFD; i <= QuickNFKC; ++i) {
            dprops[i] = new QuickDProp(i - QuickNFD);
        }        
        
        dprops[DefaultIgnorable] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                name = "Default_Ignorable_Code_Point";
                hasUnassigned = true;
                shortName = "DI";
                header = header = "# Derived Property: " + name
                    + "\r\n#  Generated from <2060..206F, FFF0..FFFB, E0000..E0FFF>"
                    + "\r\n#    + Other_Default_Ignorable_Code_Point + (Cf + Cc + Cs - White_Space)";
            }
            public boolean hasValue(int cp) {
            	if (0x2060 <= cp && cp <= 0x206F || 0xFFF0 <= cp && cp <= 0xFFFB || 0xE0000 <= cp && cp <= 0xE0FFF) return true;
                if (ucdData.getBinaryProperty(cp,Other_Default_Ignorable_Code_Point)) return true;
                if (ucdData.getBinaryProperty(cp, White_space)) return false;
                byte cat = ucdData.getCategory(cp);
                if (cat == Cf || cat == Cs || cat == Cc) return true;
                return false;
            }
        };

        dprops[Case_Sensitive] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                isStandard = false;
                name = "Case_Sensitive";
                hasUnassigned = false;
                shortName = "CS";
                header = header = "# Derived Property: " + name
                    + "\r\n#  Generated from all characters that are either on the right or left side of a case mapping";
            }
            
            UnicodeSet case_sensitive = null;
            UnicodeSet tempSet = new UnicodeSet();
            UnicodeSet cased = null;
            PrintWriter log;
            
            private void addCase(String cps, byte c1, byte c2) {
                String temp = ucdData.getCase(cps, c1, c2);
                if (temp.equals(cps)) return;
                
                //temp = nfc.normalize(temp);
                //if (temp.equals(cps)) return;
                
                tempSet.clear();
                tempSet.addAll(cps);
                tempSet.addAll(temp);
                if (!case_sensitive.containsAll(tempSet)) {
                    tempSet.removeAll(case_sensitive);
                    if (!cased.containsAll(tempSet)) {
                        log.println();
                        log.println("Adding " + tempSet + " because of: ");
                        log.println("\t" + ucdData.getCodeAndName(cps));
                        log.println("=>\t" + ucdData.getCodeAndName(temp));
                    }
                    case_sensitive.addAll(tempSet);
                }
            }
            
            public boolean hasValue(int cp) {
                if (case_sensitive == null) {
                    try {
                        log = Utility.openPrintWriter("Case_Sensitive_Log.txt", Utility.UTF8_UNIX);
                        
                        System.out.println("Building Case-Sensitive cache");
                        case_sensitive = new UnicodeSet();
                        cased = DerivedProperty.make(PropLowercase, ucdData).getSet()
                            .addAll(DerivedProperty.make(PropUppercase, ucdData).getSet())
                            .addAll(UnifiedBinaryProperty.make(CATEGORY | Lt).getSet());
                        for (int c = 0; c < 0x10FFFF; ++c) {
                            Utility.dot(c);
                            // skip cases that can't matter
                            if (!ucdData.isAssigned(c)) continue;
                            
                            String cps = UTF16.valueOf(c);
                            addCase(cps, FULL, LOWER);
                            addCase(cps, FULL, UPPER);
                            addCase(cps, FULL, TITLE);
                            addCase(cps, FULL, FOLD);
                            addCase(cps, SIMPLE, LOWER);
                            addCase(cps, SIMPLE, UPPER);
                            addCase(cps, SIMPLE, TITLE);
                            addCase(cps, SIMPLE, FOLD);
                        }
                        Utility.fixDot();
                        UnicodeSet temp;
                        log.println("Cased, but not Case_Sensitive");
                        temp = new UnicodeSet().addAll(cased).removeAll(case_sensitive);
                        Utility.showSetNames(log, "", temp, false, false, ucdData);
                        
                        log.println("Case_Sensitive, but not Cased");
                        temp = new UnicodeSet().addAll(case_sensitive).removeAll(cased);
                        Utility.showSetNames(log, "", temp, false, false, ucdData);
                        
                        log.println("Both Case_Sensitive, and Cased");
                        temp = new UnicodeSet().addAll(case_sensitive).retainAll(cased);
                        log.println(temp);
                        System.out.println("Done Building Case-Sensitive cache");
                        
                        log.close();
                        
                    } catch (Exception e) {
                        throw new ChainException("internal error", null, e);
                    }
                }
                return case_sensitive.contains(cp);  
            }
        };

        dprops[Other_Case_Ignorable] = new UnicodeProperty() {
            {
                name = "Other_Case_Ignorable";
                shortName = "OCI";
                isStandard = false;
                
                header = header = "# Binary Property";
            }
            public boolean hasValue(int cp) {
                switch(cp) {
                    case 0x27: case 0x2019: case 0xAD: return true;
                    //  case 0x2d: case 0x2010: case 0x2011: 
/*
0027          ; Other_Case_Ignorable # Po       APOSTROPHE
00AD          ; Other_Case_Ignorable # Pd       SOFT HYPHEN
2019          ; Other_Case_Ignorable # Pf       RIGHT SINGLE QUOTATION MARK
*/
                }
                return false;
            }
        };
        
        dprops[Type_i] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                isStandard = false;
                name = "DSoft_Dotted";
                shortName = "DSDot";
                header = header = "# Derived Property: " + name
                    + "\r\n#  Generated from: all characters whose canonical decompositions end with a combining character sequence that"
                    + "\r\n# - starts with i or j"
                    + "\r\n# - has no combining marks above"
                    + "\r\n# - has no combining marks with zero canonical combining class"
                ;
            }
            public boolean hasValue(int cp) {
                if (hasSoftDot(cp)) return true;
                if (nfkd.isNormalized(cp)) return false;
                String decomp = nfd.normalize(cp);
                boolean ok = false;
                for (int i = decomp.length()-1; i >= 0; --i) {
                    int ch = UTF16.charAt(decomp, i);
                    int cc = ucdData.getCombiningClass(ch);
                    if (cc == 230) return false;
                    if (cc == 0) {
                        if (!hasSoftDot(ch)) return false;
                        ok = true;
                    }
                }
                return ok;
            }
            boolean hasSoftDot(int ch) {
                return ch == 'i' || ch == 'j' || ch == 0x0268 || ch == 0x0456 || ch == 0x0458;
            }
        };
        
        dprops[Case_Ignorable] = new UnicodeProperty() {
            {
                name = "Case_Ignorable";
                isStandard = false;
                shortName = "CI";
                header = header = "# Derived Property: " + name
                    + "\r\n#  Generated from: Other_Case_Ignorable + Lm + Mn + Me + Cf";
            }
            public boolean hasValue(int cp) {
                byte cat = ucdData.getCategory(cp);
                if (cat == Lm || cat == Cf || cat == Mn || cat == Me) return true;
                if (dprops[Other_Case_Ignorable].hasValue(cp)) return true;
                return false;
            }
        };
        
/*
        GraphemeExtend = 27,
        GraphemeBase = 28,
# GraphemeExtend := Me + Mn + Mc + Other_GraphemeExtend - GraphemeLink
# GraphemeBase := 

*/
        dprops[GraphemeExtend] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                name = "Grapheme_Extend";
                shortName = "GrExt";
                header = header = "# Derived Property: " + name
                    + "\r\n#  Generated from: Me + Mn + Other_Grapheme_Extend"
                    + "\r\n#  Note: depending on an application's interpretation of Co (private use),"
                    + "\r\n#  they may be either in Grapheme_Base, or in Grapheme_Extend, or in neither."                    
                    ;
                     
            }
            public boolean hasValue(int cp) {
            	//if (cp == 0x034F) return false;
                //if (ucdData.getBinaryProperty(cp, GraphemeLink)) return false;
                // || cat == Mc
                byte cat = ucdData.getCategory(cp);
                if (cat == Me || cat == Mn
                        || ucdData.getBinaryProperty(cp,Other_GraphemeExtend)) return true;
                return false;
            }
        };

        dprops[GraphemeBase] = new UnicodeProperty() {
            {
                type = DERIVED_CORE;
                name = "Grapheme_Base";
                shortName = "GrBase";
                
                header = header = "# Derived Property: " + name
                    + "\r\n#  Generated from: [0..10FFFF] - Cc - Cf - Cs - Co - Cn - Zl - Zp - Grapheme_Extend"
                    + "\r\n#  Note: depending on an application's interpretation of Co (private use),"
                    + "\r\n#  they may be either in Grapheme_Base, or in Grapheme_Extend, or in neither."                    
                    ;
            }
            public boolean hasValue(int cp) {
            	//if (cp == 0x034F) return false;
                byte cat = ucdData.getCategory(cp);
                if (cat == Cc || cat == Cf || cat == Cs || cat == Co || cat == Cn || cat == Zl || cat == Zp) return false;
                // || ucdData.getBinaryProperty(cp,GraphemeLink)
                if (dprops[GraphemeExtend].hasValue(cp)) return false;
                return true;
            }
        };
        
        for (int i = 0; i < dprops.length; ++i) {
            UnicodeProperty up = dprops[i];
            if (up == null) continue;
            if (up.getValueType() != BINARY) continue;
            up.setValue(NUMBER, "1");
            up.setValue(SHORT, "Y");
            up.setValue(LONG, "YES");
        }
    }
    
    byte getDecompCat(int cp) {
        byte cat = ucdData.getCategory(cp);
        if (cat == Lu
            || ucdData.getBinaryProperty(cp, Other_Uppercase)) return Lu;
        if (cat == Ll
            || ucdData.getBinaryProperty(cp, Other_Lowercase)) return Ll;
        if (cat == Lt || cat == Lo || cat == Lm || cat == Nl) return cat;
        
       // if (true) throw new IllegalArgumentException("FIX nf[2]");
        
        if (nf[NFKD].isNormalized(cp)) return Lo;

        String norm = nf[NFKD].normalize(cp);
        int cp2;
        boolean gotUpper = false;
        boolean gotLower = false;
        boolean gotTitle = false;
        for (int i = 0; i < norm.length(); i += UTF32.count16(cp2)) {
            cp2 = UTF32.char32At(norm, i);
            byte catx = ucdData.getCategory(cp2);
            boolean upx = ucdData.getBinaryProperty(cp, Other_Uppercase);
            boolean lowx = ucdData.getBinaryProperty(cp, Other_Lowercase);
            if (catx == Ll || lowx || cp2 == 0x345) gotLower = true;
            if (catx == Lu || upx) gotUpper = true;
            if (catx == Lt) gotTitle = true;
        }
        if (gotLower && !gotUpper && !gotTitle) return Ll;
        if (!gotLower && gotUpper && !gotTitle) return Lu;
        if (gotLower || gotUpper || gotTitle) return Lt;
        return cat;
    }
    
    boolean isCompEx(int cp) {
        if (ucdData.getBinaryProperty(cp, CompositionExclusion)) return true;
        String decomp = ucdData.getDecompositionMapping(cp);
        if (UTF32.length32(decomp) == 1) return true;
        int first = UTF32.char32At(decomp,0);
        if (ucdData.getCombiningClass(first) != 0) return true;
        return false;
    }
    
    String fold(int cp) {
        return ucdData.getCase(cp, FULL, FOLD);
    }

    String fold(String s) {
        return ucdData.getCase(s, FULL, FOLD);
    }
    
    public static void test() {
        Default.setUCD();
        /*
        DerivedProperty dprop = new DerivedProperty(Default.ucd);
        for (int j = 0; j < LIMIT; ++j) {
            System.out.println();
            System.out.println(j + "\t" + dprop.getName(j));
            System.out.println(dprop.getHeader(j));
        }
        */
        
        for (int cp = 0xA0; cp < 0xFF; ++cp) {
            System.out.println();
            System.out.println(Default.ucd.getCodeAndName(cp));
            for (int j = 0; j < DERIVED_PROPERTY_LIMIT; ++j) {
                String prop = make(j, Default.ucd).getValue(cp);
                if (prop.length() != 0) System.out.println("\t" + prop);
            }
        }
    }
}
