/**
*******************************************************************************
* Copyright (C) 1996-2001, International Business Machines Corporation and    *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
* $Source: /xsrl/Nsvn/icu/unicodetools/com/ibm/text/UCD/UCD.java,v $
* $Date: 2004/11/13 23:10:32 $
* $Revision: 1.36 $
*
*******************************************************************************
*/

package com.ibm.text.UCD;

import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.BitSet;
import java.util.Map;

import java.io.IOException;
import java.io.DataInputStream;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.BufferedReader;

import com.ibm.text.utility.*;
import com.ibm.icu.dev.test.util.BagFormatter;
import com.ibm.icu.dev.test.util.UnicodeMap;
import com.ibm.icu.dev.test.util.UnicodeProperty;
import com.ibm.icu.text.UnicodeSet;

public final class UCD implements UCD_Types {
    
    private static int SPOT_CHECK = 0x20AC;

    static final boolean DEBUG = false;
    
    /**
     * Used for the default version.
     */
    public static final String latestVersion = "4.1.0";

    /**
     * Create singleton instance for default (latest) version
     */
    public static UCD make() {
        return make("");
    }

    /**
     * Create singleton instance for the specific version
     */
    public static UCD make(String version) {
        if (version == null || version.length() == 0) version = latestVersion;
        if (version.indexOf('.') < 0) throw new IllegalArgumentException("Version must be of form 3.1.1");
        UCD result = (UCD)versionCache.get(version);
        if (result == null) {
            //System.out.println(Utility.getStack());
            result = new UCD();
            result.fillFromFile(version);
            versionCache.put(version, result);
        }
        return result;
    }

    /**
     * Get the version of the UCD
     */
    public String getVersion() {
        return version;
    }

    /**
     * Get the date that the data was parsed
     */
    public long getDate() {
        return date;
    }

    /**
     * Is the code point allocated?
     */
    public boolean isAllocated(int codePoint) {
        if (getCategory(codePoint) != Cn) return true;
        if (isNoncharacter(codePoint)) return true;
        return false;
    }
    
    public boolean isNoncharacter(int codePoint) {
        if ((codePoint & 0xFFFE) == 0xFFFE) {
            if (compositeVersion < 0x20000 && codePoint > 0xFFFF) return false;
            return true;
        }
        if (codePoint >= 0xFDD0 && codePoint <= 0xFDEF && compositeVersion >= 0x30100) return true;
        return false;
    }

    /**
     * Is the code point assigned to a character (or surrogate)
     */
    public boolean isAssigned(int codePoint) {
        return getCategory(codePoint) != Cn;
    }

    /**
     * Is the code point a PUA character (fast check)
     */
    public boolean isPUA(int codePoint) {
        if (codePoint >= 0xE000 && codePoint < 0xF900) return true;
        if (compositeVersion < 0x20000) return false;
        return (codePoint >= 0xF0000 && codePoint < 0xFFFFE
             || codePoint >= 0x100000 && codePoint < 0x10FFFE);
    }

    /**
     * Many ranges are elided in the UCD. All but the first are not actually
     * represented in the data internally. This detects such cases.
     */
    public boolean isRepresented(int codePoint) {
        return getRaw(codePoint) != null;
    }

    /**
     * Return XML version of the data associated with the code point.
     */
    public String toString(int codePoint) {
        return get(codePoint, true).toString(this,FULL);
    }

    /**
     * Get the character name.
     */
    public String getName(int codePoint) {
        return getName(codePoint, NORMAL);
    }

    /**
     * Get the character name.
     */
    public String getName(String s) {
        return getName(s, NORMAL);
    }

    /**
     * Get the character name.
     */
    public String getName(int codePoint, byte style) {
        if (style == SHORT) return get(codePoint, true).shortName;
        return get(codePoint, true).name;
    }

    /**
     * Get the character names for the code points in a string, separated by ", "
     */
    public String getName(String s, byte style) {
        if (s.length() == 1) return getName(s.charAt(0), style);
        StringBuffer result = new StringBuffer();
        int cp;
        for (int i = 0; i < s.length(); i += UTF32.count16(cp)) {
            cp = UTF32.char32At(s, i);
            if (i > 0) result.append(", ");
            result.append(getName(cp, style));
        }
        return result.toString();
    }

    /**
     * Get the code in U+ notation
     */
    public static String getCode(int codePoint) {
        return "U+" + Utility.hex(codePoint);
    }

    /**
     * Get the code in U+ notation
     */
    public static String getCode(String s) {
        if (s.length() == 1) return getCode(s.charAt(0)); // fast path
        StringBuffer result = new StringBuffer();
        int cp;
        for (int i = 0; i < s.length(); i += UTF32.count16(cp)) {
            cp = UTF32.char32At(s, i);
            if (i > 0) result.append(", ");
            result.append(getCode(cp));
        }
        return result.toString();
    }

    /**
     * Get the name and number (U+xxxx NAME) for a code point
     */
    public String getCodeAndName(int codePoint, byte type) {
        return getCode(codePoint) + " " + getName(codePoint, type);
    }

    /**
     * Get the name and number (U+xxxx NAME) for the code points in a string,
     * separated by ", "
     */
    public String getCodeAndName(String s, byte type) {
        if (s == null || s.length() == 0) return "NULL";
        if (s.length() == 1) return getCodeAndName(s.charAt(0)); // fast path
        StringBuffer result = new StringBuffer();
        int cp;
        for (int i = 0; i < s.length(); i += UTF32.count16(cp)) {
            cp = UTF32.char32At(s, i);
            if (i > 0) result.append(", ");
            result.append(getCodeAndName(cp));
        }
        return result.toString();
    }

    /**
     * Get the name and number (U+xxxx NAME) for a code point
     */
    public String getCodeAndName(int codePoint) {
        return getCodeAndName(codePoint, NORMAL);
    }

    /**
     * Get the name and number (U+xxxx NAME) for a code point
     */
    public String getCodeAndName(String s) {
        return getCodeAndName(s, NORMAL);
    }

    /**
     * Get the general category
     */
    public byte getCategory(int codePoint) {
        return get(codePoint, false).generalCategory;
    }
    
    private static final byte FAKE_SYMBOL = 57; // fake category for comparison
    private static final byte FAKE_PUNCTUATION = 58; // fake category for comparison
    private static final byte FAKE_SEPERATOR = 59; // fake category for comparison
    private static final byte FAKE_NUMBER = 60; // fake category for comparison
    private static final byte FAKE_MARK = 61; // fake category for comparison
    private static final byte FAKE_LETTER = 62; // fake category for comparison
    private static final byte FAKE_OTHER = 63; // fake category for comparison
    private static final byte FAKENC = 31; // fake category for comparison
    
    public byte getModCat(int cp, int collapseBits) {
        byte cat = getCategory(cp);
        if (cat == UNASSIGNED && isNoncharacter(cp)) {
            cat = FAKENC;
        } else if (((1<<cat) & collapseBits) != 0) {
        	switch (cat) {
				case UNASSIGNED: cat = FAKE_OTHER; break;
				case FAKENC: cat = FAKE_OTHER; break;
				
				case UPPERCASE_LETTER: cat = FAKE_LETTER; break;
				case LOWERCASE_LETTER: cat = FAKE_LETTER; break;
				case TITLECASE_LETTER: cat = FAKE_LETTER; break;
				case MODIFIER_LETTER: cat = FAKE_LETTER; break;
				case OTHER_LETTER: cat = FAKE_LETTER; break;
				
				case NON_SPACING_MARK: cat = FAKE_MARK; break;
				case ENCLOSING_MARK: cat = FAKE_MARK; break;
				case COMBINING_SPACING_MARK: cat = FAKE_MARK; break;
				
				case DECIMAL_DIGIT_NUMBER: cat = FAKE_NUMBER; break;
				case LETTER_NUMBER: cat = FAKE_NUMBER; break;
				case OTHER_NUMBER: cat = FAKE_NUMBER; break;
				
				case SPACE_SEPARATOR: cat = FAKE_SEPERATOR; break;
				case LINE_SEPARATOR: cat = FAKE_SEPERATOR; break;
				case PARAGRAPH_SEPARATOR: cat = FAKE_SEPERATOR; break;
				
				case CONTROL: cat = FAKE_OTHER; break;
				case FORMAT: cat = FAKE_OTHER; break;
				case UNUSED_CATEGORY: cat = FAKE_OTHER; break;
				case PRIVATE_USE: cat = FAKE_OTHER; break;
				case SURROGATE: cat = FAKE_OTHER; break;
				
				case DASH_PUNCTUATION: cat = FAKE_PUNCTUATION; break;
				case START_PUNCTUATION: cat = FAKE_PUNCTUATION; break;
				case END_PUNCTUATION: cat = FAKE_PUNCTUATION; break;
				case CONNECTOR_PUNCTUATION: cat = FAKE_PUNCTUATION; break;
				case OTHER_PUNCTUATION: cat = FAKE_PUNCTUATION; break;
				case INITIAL_PUNCTUATION: cat = FAKE_PUNCTUATION; break;
				case FINAL_PUNCTUATION: cat = FAKE_PUNCTUATION; break;
				
				case MATH_SYMBOL: cat = FAKE_SYMBOL; break;
				case CURRENCY_SYMBOL: cat = FAKE_SYMBOL; break;
				case MODIFIER_SYMBOL: cat = FAKE_SYMBOL; break;
				case OTHER_SYMBOL: cat = FAKE_SYMBOL; break;
			}
            if (collapseBits == -1) {
                switch (cat) {
                    case FAKE_MARK: 
                    case FAKE_NUMBER:
                    case FAKE_SEPERATOR:
                    case FAKE_PUNCTUATION:
                    case FAKE_SYMBOL:
                        cat = FAKE_LETTER;
                        break;
                }
			}
        }
        return cat;
    }

    public String getModCatID_fromIndex(byte cat) {
    	switch (cat) {
			case FAKE_SYMBOL: return "S&";
			case FAKE_PUNCTUATION: return "P&";
			case FAKE_SEPERATOR: return "Z&";
			case FAKE_NUMBER: return "N&";
			case FAKE_MARK: return "M&";
			case FAKE_LETTER: return "L&";
			case FAKE_OTHER: return "C&";
			case FAKENC: return "NC";
        }
        return getCategoryID_fromIndex(cat);
    }

    /**
     * Get the main category, as a mask
     */
    public static int mainCategoryMask(byte cat) {
        switch (cat) {
          case Lu: case Ll: case Lt: case Lm: case Lo: return LETTER_MASK;
          case Mn: case Me: case Mc: return MARK_MASK;
          case Nd: case Nl: case No: return NUMBER_MASK;
          case Zs: case Zl: case Zp: return SEPARATOR_MASK;
          case Cc: case Cf: case Cs: case Co: return CONTROL_MASK;
          case Pc: case Pd: case Ps: case Pe: case Po: case Pi: case Pf: return PUNCTUATION_MASK;
          case Sm: case Sc: case Sk: case So: return SYMBOL_MASK;
          case Cn: return UNASSIGNED_MASK;
        }
        throw new IllegalArgumentException ("Illegal General Category " + cat);
    }

    /**
     * Get the combining class, a number between zero and 255. Returned
     * as a short to avoid the signed-byte problem in Java
     */
    public short getCombiningClass(int codePoint) {
        return (short)(get(codePoint, false).combiningClass & 0xFF);
    }

    /**
     * Does this combining class actually occur in this version of the data.
     */
    public boolean isCombiningClassUsed(byte value) {
        return combiningClassSet.get(0xFF & value);
    }

    static UnicodeSet BIDI_R_SET, BIDI_AL_SET, BIDI_BN_SET;
    
    /**
     * Get the bidi class
     */
    public byte getBidiClass(int codePoint) {
        if (getCategory(codePoint) != Cn) return get(codePoint, false).bidiClass;
        
        if (BIDI_R_SET == null) { // build it
            
            BIDI_R_SET = new UnicodeSet();
            BIDI_AL_SET = new UnicodeSet();
            
            blockData.getSet("Hebrew",BIDI_R_SET);
            blockData.getSet("Cypriot_Syllabary",BIDI_R_SET);
            
            blockData.getSet("Arabic",BIDI_AL_SET);
            blockData.getSet("Syriac",BIDI_AL_SET);
            blockData.getSet("Thaana",BIDI_AL_SET);
            blockData.getSet("Arabic_Presentation_Forms-A",BIDI_AL_SET);
            blockData.getSet("Arabic_Presentation_Forms-B",BIDI_AL_SET);
            /*
            int blockId = 0;
            BlockData blockData = new BlockData();
            UnicodeSet s = blockData.get
            while (getBlockData(blockId++, blockData)) {
                if (blockData.name.equals("Hebrew")
                 || blockData.name.equals("Cypriot_Syllabary")
                ) {
                    System.out.println("R:  Adding " + blockData.name + ": " 
                        + Utility.hex(blockData.start) 
                        + ".." + Utility.hex(blockData.end));
                    BIDI_R_SET.add(blockData.start, blockData.end);
                } else if (blockData.name.equals("Arabic")
                 || blockData.name.equals("Syriac")
                 || blockData.name.equals("Thaana")
                 || blockData.name.equals("Arabic_Presentation_Forms-A")
                 || blockData.name.equals("Arabic_Presentation_Forms-B")
                ) {
                    System.out.println("AL: Adding " + blockData.name + ": " 
                        + Utility.hex(blockData.start) 
                        + ".." + Utility.hex(blockData.end));
                    BIDI_AL_SET.add(blockData.start, blockData.end);
                } else {
                    if (false) System.out.println("SKIPPING: " + blockData.name + ": " 
                        + Utility.hex(blockData.start) 
                        + ".." + Utility.hex(blockData.end));
                }
            }
            */
            
            System.out.println("BIDI_R_SET: " + BIDI_R_SET);
            System.out.println("BIDI_AL_SET: " + BIDI_AL_SET);

            UnicodeSet BIDI_R_Delta = new UnicodeSet(0xFB1D, 0xFB4F).add(0x10800, 0x10FFF).add(0x07C0,0x8FF);
            BIDI_R_Delta.removeAll(BIDI_R_SET);
            System.out.println("R: Adding " + BIDI_R_Delta);
            BIDI_R_SET.addAll(BIDI_R_Delta);
            
            UnicodeSet BIDI_AL_Delta = new UnicodeSet(0x0750, 0x077F);
            BIDI_AL_Delta.removeAll(BIDI_AL_SET);
            System.out.println("AL: Adding " + BIDI_AL_Delta);
            BIDI_AL_SET.addAll(BIDI_AL_Delta);
            
            UnicodeSet noncharacters = UnifiedBinaryProperty.make(BINARY_PROPERTIES + Noncharacter_Code_Point, this).getSet();
            noncharacters.remove(Utility.BOM);
            
            System.out.println("Removing Noncharacters/BOM  " + noncharacters);
            BIDI_R_SET.removeAll(noncharacters);
            BIDI_AL_SET.removeAll(noncharacters);
            
            BIDI_BN_SET = new UnicodeSet();
            if (compositeVersion >= 0x40001) {
                BIDI_BN_SET.addAll(noncharacters);
                UnicodeSet DefaultIg = DerivedProperty.make(DefaultIgnorable, this).getSet();
                System.out.println("DefaultIg: " + DefaultIg);
                BIDI_BN_SET.addAll(DefaultIg);
            }                       
            
            System.out.println("BIDI_R_SET: " + BIDI_R_SET);
            System.out.println("BIDI_AL_SET: " + BIDI_AL_SET);
            System.out.println("BIDI_BN_SET: " + BIDI_BN_SET);
            
            if (BIDI_R_SET.containsSome(BIDI_AL_SET)) {
                throw new ChainException("BIDI values for Cf characters overlap!!", null);
            }
            
        }

        if (BIDI_BN_SET.contains(codePoint)) {
            return BIDI_BN;
        }
        if (BIDI_R_SET.contains(codePoint)) {
            return BIDI_R;
        }
        if (BIDI_AL_SET.contains(codePoint)) {
            return BIDI_AL;
        }
        return BIDI_L;
    }

    /**
     * Get the RAW decomposition mapping. Must be used recursively for the full mapping!
     */
    public String getDecompositionMapping(int codePoint) {
        return get(codePoint, true).decompositionMapping;
    }

    /**
     * Get BIDI mirroring character, if there is one.
     */
    public String getBidiMirror(int codePoint) {
        return get(codePoint, true).bidiMirror;
    }

    /**
     * Get the RAW decomposition type: the <...> field in the UCD data.
     */
    public byte getDecompositionType(int codePoint) {
        return get(codePoint, false).decompositionType;
    }
    
    IntMap hanExceptions = null;
    
    static class HanException {
        double numericValue;
        byte numericType;
    }
    
    public UnicodeMap getHanValue(String propertyName) {
        UnicodeMap result = new UnicodeMap();
        try {
            BufferedReader in = Utility.openUnicodeFile("Unihan", version, true, Utility.UTF8); 
            int lineCounter = 0;
            while (true) {
                Utility.dot(++lineCounter);
                
                String line = in.readLine();
                if (line == null) break;
                if (line.length() < 6) continue;
                if (line.charAt(0) == '#') continue;
                line = line.trim();
                
                int tabPos = line.indexOf('\t');
                int tabPos2 = line.indexOf('\t', tabPos+1);
                
                String property = line.substring(tabPos+1, tabPos2).trim();
                if (!property.equalsIgnoreCase(propertyName)) continue;
                
                String scode = line.substring(2, tabPos).trim();
                int code = Integer.parseInt(scode, 16);
                String propertyValue = line.substring(tabPos2+1).trim();
                result.put(code, propertyValue);
            }
            in.close();
        } catch (Exception e) {
            throw new ChainException("Han File Processing Exception", null, e);
        } finally {
            Utility.fixDot();
        }
        return result;
    }

    
    void populateHanExceptions() {
        hanExceptions = new IntMap();
        BufferedReader in = null;
        try {
            in = Utility.openUnicodeFile("Unihan", version, true, Utility.UTF8); 
            int lineCounter = 0;
            while (true) {
                Utility.dot(++lineCounter);
                
                String line = in.readLine();
                if (line == null) break;
                if (line.length() < 6) continue;
                if (line.charAt(0) == '#') continue;
                line = line.trim();
                
                int tabPos = line.indexOf('\t');
                int tabPos2 = line.indexOf('\t', tabPos+1);
                
                String property = line.substring(tabPos+1, tabPos2).trim();
                if (!property.endsWith("Numeric")) continue;
                
                String propertyValue = line.substring(tabPos2+1).trim();
                propertyValue = Utility.replace(propertyValue, ",", "");
                int hack = propertyValue.indexOf(' ');
                if (hack >= 0) {
                    Utility.fixDot();
                    System.out.println("BAD NUMBER: " + line);
                    propertyValue = propertyValue.substring(0,hack);
                }
                
                String scode = line.substring(2, tabPos).trim();
                int code = Integer.parseInt(scode, 16);
                
                if (code == 0x5793 || code == 0x4EAC) continue; // two exceptions!!
                
                //kAccountingNumeric
                //kOtherNumeric
                //kPrimaryNumeric
                
                HanException except = (HanException) hanExceptions.get(code);
                if (except != null) throw new Exception("Duplicate Numeric Value for " + line);
                except = new HanException();
                hanExceptions.put(code, except);
                except.numericValue = Double.parseDouble(propertyValue);
                except.numericType = property.equals("kAccountingNumeric") ? NUMERIC
                : property.equals("kOtherNumeric") ? NUMERIC
                : property.equals("kPrimaryNumeric") ? NUMERIC
                : NONE;
                if (except.numericType == NONE) throw new Exception("Unknown Numeric Type for " + line);
                
                if (false) {
                    Utility.fixDot();
                    System.out.println(line);
                    System.out.println(getNumericValue(code));
                    System.out.println(getNumericTypeID(code));
                }
            }
            in.close();
        } catch (Exception e) {
            throw new ChainException("Han File Processing Exception", null, e);
        } finally {
            Utility.fixDot();
            System.out.println("****Size: " + hanExceptions.size());
        }
    }

    public double getNumericValue(int codePoint) {
        if (hanExceptions == null) populateHanExceptions();
        Object except = hanExceptions.get(codePoint);
        if (except != null) {
            return ((HanException)except).numericValue;
        }
        return get(codePoint, false).numericValue;
    }

    public byte getNumericType(int codePoint) {
        if (hanExceptions == null) populateHanExceptions();
        Object except = hanExceptions.get(codePoint);
        if (except != null) {
            return ((HanException)except).numericType;
        }
        return get(codePoint, false).numericType;
    }

    public String getCase(int codePoint, byte simpleVsFull, byte caseType) {
        return getCase(codePoint, simpleVsFull, caseType, "");
    }

    public String getCase(String s, byte simpleVsFull, byte caseType) {
        return getCase(s, simpleVsFull, caseType, "");
    }

    public String getCase(int codePoint, byte simpleVsFull, byte caseType, String condition) {
        UData udata = get(codePoint, true);
        if (caseType < LOWER || caseType > FOLD
          || (simpleVsFull != SIMPLE && simpleVsFull != FULL)) {
            throw new IllegalArgumentException("simpleVsFull or caseType out of bounds");
        }
        if (caseType < FOLD) {
            if (simpleVsFull == FULL && udata.specialCasing.length() != 0) {
                if (condition.length() == 0
                || udata.specialCasing.indexOf(condition) < 0) {
                    simpleVsFull = SIMPLE;
                }
            }
        } else {
            // special case. For these characters alone, use "I" as option meaning collapse to "i"
            //if (codePoint == 0x0131 || codePoint == 0x0130) { // special case turkish i
            if (getBinaryProperty(codePoint, CaseFoldTurkishI)) {
                if (!udata.specialCasing.equals("I")) simpleVsFull = SIMPLE;
                else simpleVsFull = FULL;
            }
        }

        switch (caseType + simpleVsFull) {
            case SIMPLE + UPPER: return udata.simpleUppercase;
            case SIMPLE + LOWER: return udata.simpleLowercase;
            case SIMPLE + TITLE: return udata.simpleTitlecase;
            case SIMPLE + FOLD: return udata.simpleCaseFolding;
            case FULL + UPPER: return udata.fullUppercase;
            case FULL + LOWER: return udata.fullLowercase;
            case FULL + TITLE: return udata.fullTitlecase;
            case FULL + FOLD: return udata.fullCaseFolding;
        }
        throw new IllegalArgumentException("getCase: " + caseType + ", " + simpleVsFull);
    }
    
    static final char SHY = '\u00AD';
    
    static final char APOSTROPHE = '\u2019';
    
    public String getCase(String s, byte simpleVsFull, byte caseType, String condition) {
        if (UTF32.length32(s) == 1) return getCase(UTF32.char32At(s, 0), simpleVsFull, caseType);
        StringBuffer result = new StringBuffer();
        int cp;
        byte currentCaseType = caseType;
        UCDProperty defaultIgnorable = DerivedProperty.make(DerivedProperty.DefaultIgnorable, this);
        
        for (int i = 0; i < s.length(); i += UTF32.count16(cp)) {
            cp = UTF32.char32At(s, i);
            String mappedVersion = getCase(cp, simpleVsFull, currentCaseType, condition);
            result.append(mappedVersion);
            if (caseType == TITLE) {    // set the case type for the next character
            
                // certain characters are ignored
                if (cp == SHY || cp == '\'' || cp == APOSTROPHE) continue;
                byte cat = getCategory(cp);
                if (cat == Mn || cat == Me || cat == Cf || cat == Lm) continue;
                if (defaultIgnorable.hasValue(cp)) continue;
                // if DefaultIgnorable is not supported, then 
                // check for (Cf + Cc + Cs) - White_Space
                // if (cat == Cs && cp != 0x85 && (cp < 9 || cp > 0xD)) continue;                
                
                // if letter is cased, change next to lowercase, otherwise revert to TITLE
                if (cat == Lu || cat == Ll || cat == Lt
                  || getBinaryProperty(cp, Other_Lowercase) // skip if not supported
                  || getBinaryProperty(cp, Other_Uppercase) // skip if not supported
                ) {
                    currentCaseType = LOWER;
                } else {
                    currentCaseType = TITLE;
                }
            }
        }
        return result.toString();
    }

    /*
    public String getSimpleLowercase(int codePoint) {
        return get(codePoint, true).simpleLowercase;
    }

    public String getSimpleUppercase(int codePoint) {
        return get(codePoint, true).simpleUppercase;
    }

    public String getSimpleTitlecase(int codePoint) {
        return get(codePoint, true).simpleTitlecase;
    }

    public String getSimpleCaseFolding(int codePoint) {
        return get(codePoint, true).simpleCaseFolding;
    }

    public String getFullLowercase(int codePoint) {
        return get(codePoint, true).fullLowercase;
    }

    public String getFullUppercase(int codePoint) {
        return get(codePoint, true).fullUppercase;
    }

    public String getFullTitlecase(int codePoint) {
        return get(codePoint, true).fullTitlecase;
    }

    public String getFullCaseFolding(int codePoint) {
        return get(codePoint, true).simpleCaseFolding;
    }

    public String getLowercase(int codePoint, boolean full) {
        if (full) return getFullLowercase(codePoint);
        return getSimpleLowercase(codePoint);
    }

    public String getUppercase(int codePoint, boolean full) {
        if (full) return getFullUppercase(codePoint);
        return getSimpleLowercase(codePoint);
    }

    public String getTitlecase(int codePoint, boolean full) {
        if (full) return getFullTitlecase(codePoint);
        return getSimpleTitlecase(codePoint);
    }

    public String getCaseFolding(int codePoint, boolean full) {
        if (full) return getFullCaseFolding(codePoint);
        return getSimpleCaseFolding(codePoint);
    }

    public String getLowercase(String s, boolean full) {
        if (s.length() == 1) return getLowercase(s.charAt(0), true);
        StringBuffer result = new StringBuffer();
        int cp;
        for (int i = 0; i < s.length(); i += UTF32.count16(cp)) {
            cp = UTF32.char32At(s, i);
            if (i > 0) result.append(", ");
            result.append(getLowercase(cp, true));
        }
        return result.toString();
    }

    public String getUppercase(String s, boolean full) {
        if (s.length() == 1) return getUppercase(s.charAt(0), true);
        StringBuffer result = new StringBuffer();
        int cp;
        for (int i = 0; i < s.length(); i += UTF32.count16(cp)) {
            cp = UTF32.char32At(s, i);
            if (i > 0) result.append(", ");
            result.append(getUppercase(cp, true));
        }
        return result.toString();
    }

    public String getTitlecase(String s, boolean full) {
        if (s.length() == 1) return getTitlecase(s.charAt(0), true);
        StringBuffer result = new StringBuffer();
        int cp;
        for (int i = 0; i < s.length(); i += UTF32.count16(cp)) {
            cp = UTF32.char32At(s, i);
            if (i > 0) result.append(", ");
            result.append(getTitlecase(cp, true));
        }
        return result.toString();
    }

    public String getCaseFolding(String s, boolean full) {
        if (s.length() == 1) return getCaseFolding(s.charAt(0), true);
        StringBuffer result = new StringBuffer();
        int cp;
        for (int i = 0; i < s.length(); i += UTF32.count16(cp)) {
            cp = UTF32.char32At(s, i);
            if (i > 0) result.append(", ");
            result.append(getCaseFolding(cp, true));
        }
        return result.toString();
    }
    */

    public String getSpecialCase(int codePoint) {
        return get(codePoint, true).specialCasing;
    }

    public byte getEastAsianWidth(int codePoint) {
//      if (0x30000 <= codepoint && codepoint <= 0x3FFFD) return EAW;
        return get(codePoint, false).eastAsianWidth;
    }

    public byte getLineBreak(int codePoint) {
        return get(codePoint, false).lineBreak;
    }

    public byte getScript(int codePoint) {
        return get(codePoint, false).script;
    }
    
    
    public byte getScript(String s) {
        byte result = COMMON_SCRIPT;
        if (s == null || s.length() == 0) return result;
        int cp;
        for (int i = 0; i < s.length(); i += UTF32.count16(cp)) {
            cp = UTF32.char32At(s, i);
            byte script = getScript(cp);
            if (script == INHERITED_SCRIPT) continue;
            result = script;
        }
        return result;
    }
    

    public byte getAge(int codePoint) {
        return get(codePoint, false).age;
    }

    public byte getJoiningType(int codePoint) {
        return get(codePoint, false).joiningType;
    }

    public byte getJoiningGroup(int codePoint) {
        return get(codePoint, false).joiningGroup;
    }

    public long getBinaryProperties(int codePoint) {
        return get(codePoint, false).binaryProperties;
    }

    public boolean getBinaryProperty(int codePoint, int bit) {
        return (get(codePoint, false).binaryProperties & (1L<<bit)) != 0;
    }

    // ENUM Mask Utilties

    public int getCategoryMask(int codePoint) {
        return 1<<get(codePoint, false).generalCategory;
    }

    public int getBidiClassMask(int codePoint) {
        return 1<<get(codePoint, false).bidiClass;
    }

    public int getNumericTypeMask(int codePoint) {
        return 1<<getNumericType(codePoint);
    }

    public int getDecompositionTypeMask(int codePoint) {
        return 1<<get(codePoint, false).decompositionType;
    }

    public int getEastAsianWidthMask(int codePoint) {
        return 1<<get(codePoint, false).eastAsianWidth;
    }

    public int getLineBreakMask(int codePoint) {
        return 1<<get(codePoint, false).lineBreak;
    }

    public int getScriptMask(int codePoint) {
        return 1<<get(codePoint, false).script;
    }

    public int getAgeMask(int codePoint) {
        return 1<<get(codePoint, false).age;
    }

    public int getJoiningTypeMask(int codePoint) {
        return 1<<get(codePoint, false).joiningType;
    }

    public int getJoiningGroupMask(int codePoint) {
        return 1<<get(codePoint, false).joiningGroup;
    }


    // VERSIONS WITH NAMES

    public String getCategoryID(int codePoint) {
        return getCategoryID_fromIndex(getCategory(codePoint));
    }

    public static String getCategoryID_fromIndex(byte prop) {
        return getCategoryID_fromIndex(prop, NORMAL);
    }
    
    public static String getCategoryID_fromIndex(byte prop, byte style) {
        return prop < 0 || prop >= UCD_Names.GENERAL_CATEGORY.length ? null
            : (style != LONG) ? UCD_Names.GENERAL_CATEGORY[prop] : UCD_Names.LONG_GENERAL_CATEGORY[prop];
    }
    
    
    public String getCombiningClassID(int codePoint) {
        return getCombiningClassID(codePoint, NORMAL);
    }

    public String getCombiningClassID(int codePoint, byte style) {
        return getCombiningClassID_fromIndex(getCombiningClass(codePoint), style);
    }
    
    public static String getCombiningClassID_fromIndex(short cc) {
        return getCombiningClassID_fromIndex(cc, NORMAL);
    }

    static String getCombiningClassID_fromIndex (short index, byte style) {
        return index < 0 
            || index >= UCD_Names.COMBINING_CLASS.length 
            ? null
            : style == SHORT 
            ? UCD_Names.COMBINING_CLASS[index] 
            : UCD_Names.LONG_COMBINING_CLASS[index];
/*
        if (index > 255) return null;
        index &= 0xFF;
        if (style == NORMAL || style == NUMBER) return String.valueOf(index);
        String s = "";
        switch (index) {
            case 0: s = style < LONG ? "NR" : "NotReordered"; break;
            case 1: s = style < LONG ? "OV" :  "Overlay"; break;
            case 7: s = style < LONG ? "NK" :  "Nukta"; break;
            case 8: s = style < LONG ? "KV" :  "KanaVoicing"; break;
            case 9: s = style < LONG ? "VR" :  "Virama"; break;
            case 200: s = style < LONG ? "ATBL" :  "AttachedBelowLeft"; break;
            case 202: s = style < LONG ? "ATB" :  "AttachedBelow"; break;
            case 204: s = style < LONG ? "ATBR" :  "AttachedBelowRight"; break;
            case 208: s = style < LONG ? "ATL" :  "AttachedLeft"; break;
            case 210: s = style < LONG ? "ATR" :  "AttachedRight"; break;
            case 212: s = style < LONG ? "ATAL" :  "AttachedAboveLeft"; break;
            case 214: s = style < LONG ? "ATA" :  "AttachedAbove"; break;
            case 216: s = style < LONG ? "ATAR" :   "AttachedAboveRight"; break;
            case 218: s = style < LONG ? "BL" :   "BelowLeft"; break;
            case 220: s = style < LONG ? "B" :   "Below"; break;
            case 222: s = style < LONG ? "BR" :   "BelowRight"; break;
            case 224: s = style < LONG ? "L" :   "Left"; break;
            case 226: s = style < LONG ? "R" :   "Right"; break;
            case 228: s = style < LONG ? "AL" :   "AboveLeft"; break;
            case 230: s = style < LONG ? "A" :   "Above"; break;
            case 232: s = style < LONG ? "AR" :   "AboveRight"; break;
            case 233: s = style < LONG ? "DB" :   "DoubleBelow"; break;
            case 234: s = style < LONG ? "DA" :   "DoubleAbove"; break;
            case 240: s = style < LONG ? "IS" :   "IotaSubscript"; break;
            default: s += "" + index;
        }
        return s;
        */
    }
    

    public String getBidiClassID(int codePoint) {
        return getBidiClassID_fromIndex(getBidiClass(codePoint));
    }

    public static String getBidiClassID_fromIndex(byte prop) {
        return getBidiClassID_fromIndex(prop, NORMAL);
    }
    
    public static String getBidiClassID_fromIndex(byte prop, byte style) {
        return prop < 0 
            || prop >= UCD_Names.BIDI_CLASS.length 
            ? null
            : style == SHORT 
            ? UCD_Names.BIDI_CLASS[prop] 
            : UCD_Names.LONG_BIDI_CLASS[prop];
    }

    public String getDecompositionTypeID(int codePoint) {
        return getDecompositionTypeID_fromIndex(getDecompositionType(codePoint));
    }

    public static String getDecompositionTypeID_fromIndex(byte prop) {
        return getDecompositionTypeID_fromIndex(prop, NORMAL);
    }
    public static String getDecompositionTypeID_fromIndex(byte prop, byte style) {
        return prop < 0 || prop >= UCD_Names.LONG_DECOMPOSITION_TYPE.length ? null
        : style == SHORT ? UCD_Names.DECOMPOSITION_TYPE[prop] : UCD_Names.LONG_DECOMPOSITION_TYPE[prop];
    }

    public String getNumericTypeID(int codePoint) {
        return getNumericTypeID_fromIndex(getNumericType(codePoint));
    }

    public static String getNumericTypeID_fromIndex(byte prop) {
        return getNumericTypeID_fromIndex(prop, NORMAL);
    }

    public static String getNumericTypeID_fromIndex(byte prop, byte style) {
        return prop < 0 || prop >= UCD_Names.LONG_NUMERIC_TYPE.length ? null
        : style == SHORT ? UCD_Names.NUMERIC_TYPE[prop] : UCD_Names.LONG_NUMERIC_TYPE[prop];
    }

    public String getEastAsianWidthID(int codePoint) {
        return getEastAsianWidthID_fromIndex(getEastAsianWidth(codePoint));
    }

    public static String getEastAsianWidthID_fromIndex(byte prop) {
        return getEastAsianWidthID_fromIndex(prop, NORMAL);
    }

    public static String getEastAsianWidthID_fromIndex(byte prop, byte style) {
        return prop < 0 || prop >= UCD_Names.LONG_EAST_ASIAN_WIDTH.length ? null
        : style != LONG ? UCD_Names.EAST_ASIAN_WIDTH[prop] : UCD_Names.LONG_EAST_ASIAN_WIDTH[prop];
    }

    public String getLineBreakID(int codePoint) {
        return getLineBreakID_fromIndex(getLineBreak(codePoint));
    }

    public static String getLineBreakID_fromIndex(byte prop) {
        return getLineBreakID_fromIndex(prop, NORMAL);
    }

    public static String getLineBreakID_fromIndex(byte prop, byte style) {
        return prop < 0 || prop >= UCD_Names.LINE_BREAK.length ? null
        : style != LONG ? UCD_Names.LINE_BREAK[prop] : UCD_Names.LONG_LINE_BREAK[prop];
    }

    public String getJoiningTypeID(int codePoint) {
        return getJoiningTypeID_fromIndex(getJoiningType(codePoint));
    }

    public static String getJoiningTypeID_fromIndex(byte prop) {
        return getJoiningTypeID_fromIndex(prop, NORMAL);
    }

    public static String getJoiningTypeID_fromIndex(byte prop, byte style) {
        return prop < 0 || prop >= UCD_Names.JOINING_TYPE.length ? null
        : style != LONG ? UCD_Names.JOINING_TYPE[prop] : UCD_Names.LONG_JOINING_TYPE[prop];
    }

    public String getJoiningGroupID(int codePoint) {
        return getJoiningGroupID_fromIndex(getJoiningGroup(codePoint));
    }

    public static String getJoiningGroupID_fromIndex(byte prop) {
        return getJoiningGroupID_fromIndex(prop, NORMAL);
    }

    public static String getJoiningGroupID_fromIndex(byte prop, byte style) {
        // no short version
        return prop < 0 || prop >= UCD_Names.JOINING_GROUP.length ? null
        : UCD_Names.JOINING_GROUP[prop];
    }

    public String getScriptID(int codePoint) {
        return getScriptID_fromIndex(getScript(codePoint));
    }

    public static String getScriptID_fromIndex(byte prop) {
        return getScriptID_fromIndex(prop, NORMAL);
    }

    public static String getScriptID_fromIndex(byte prop, byte length) {
        return prop < 0 || prop >= UCD_Names.SCRIPT.length ? null
        : (length == SHORT) ? UCD_Names.SCRIPT[prop] : UCD_Names.LONG_SCRIPT[prop];
    }

    public String getAgeID(int codePoint) {
        return getAgeID_fromIndex(getAge(codePoint));
    }

    public static String getAgeID_fromIndex(byte prop) {
        return getAgeID_fromIndex(prop, NORMAL);
    }

    public static String getAgeID_fromIndex(byte prop, byte style) {
        // no short for
        return prop < 0 || prop >= UCD_Names.AGE.length ? null
        : UCD_Names.AGE[prop];
    }

    public String getBinaryPropertiesID(int codePoint, byte bit) {
        return getBinaryProperty(codePoint, bit) ? UCD_Names.YN_TABLE[1] : UCD_Names.YN_TABLE[0];
    }

    public static String getBinaryPropertiesID_fromIndex(byte bit) {
        return getBinaryPropertiesID_fromIndex(bit, NORMAL);
    }

    public static String getBinaryPropertiesID_fromIndex(byte bit, byte style) {
        return bit < 0 || bit >= UCD_Names.BP.length ? null
        : style == SHORT ? UCD_Names.SHORT_BP[bit] : UCD_Names.BP[bit];
    }

    public static int mapToRepresentative(int ch, int rCompositeVersion) {
        if (ch <= 0xFFFD) {
            //if (ch <= 0x2800) return ch;
            //if (ch <= 0x28FF) return 0x2800;    // braille
            if (ch <= 0x3400) return ch;         // CJK Ideograph Extension A
            if (ch <= 0x4DB5) return 0x3400;
            if (ch <= 0x4E00) return ch;         // CJK Ideograph
            if (ch <= 0x9FA5) return 0x4E00;
            if (ch <= 0xAC00) return ch;         // Hangul Syllable
            if (ch <= 0xD7A3) return 0xAC00;
            if (ch <= 0xD800) return ch;         // Non Private Use High Surrogate
            if (ch <= 0xDB7F) return 0xD800;
            if (ch <= 0xDB80) return ch;         // Private Use High Surrogate
            if (ch <= 0xDBFF) return 0xDB80;
            if (ch <= 0xDC00) return ch;         // Low Surrogate
            if (ch <= 0xDFFF) return 0xDC00;
            if (ch <= 0xE000) return ch;         // Private Use
            if (ch <= 0xF8FF) return 0xE000;
            if (rCompositeVersion < 0x20105) {
                if (ch <= 0xF900) return ch;         // CJK Compatibility Ideograp
                if (ch <= 0xFA2D) return 0xF900;
            }
            if (ch <  0xFDD0) return ch;         // Noncharacter
            if (ch <= 0xFDEF) return 0xFFFF;
        } else {
            if ((ch & 0xFFFE) == 0xFFFE) return 0xFFFF;         // Noncharacter
            
            if (ch <= 0x20000) return ch;         // Extension B
            if (ch <= 0x2A6D6) return 0x20000;
            //if (ch <= 0x2F800) return ch;
            //if (ch <= 0x2FA1D) return 0x2F800;      // compat ideographs
            if (ch < 0xF0000) return ch;       // Plane 15 Private Use
            if (rCompositeVersion >= 0x20000) {
                return 0xE000;
            }
            /*
            if (ch <= 0xFFFFD) return 0xF0000;       // Plane 16 Private Use
            if (ch <= 0x100000) return ch;       // Plane 15 Private Use
            if (ch <= 0x10FFFD) return 0x100000;       // Plane 16 Private Use
            */
        }
        return ch;
    }

    public boolean isIdentifierStart(int cp) {
        /*
        if (extended) {
            if (cp == 0x0E33 || cp == 0x0EB3 || cp == 0xFF9E || cp == 0xFF9F) return false;
            if (cp == 0x037A || cp >= 0xFC5E && cp <= 0xFC63 || cp == 0xFDFA || cp == 0xFDFB) return false;
            if (cp >= 0xFE70 && cp <= 0xFE7E && (cp & 1) == 0) return false;
        }
        */
        byte cat = getCategory(cp);
        if (cat == Lu || cat == Ll || cat == Lt || cat == Lm || cat == Lo || cat == Nl) return true;
        if (getBinaryProperty(cp, Other_ID_Start)) return true;
        return false;
    }

    public boolean isIdentifierContinue_NO_Cf(int cp) {
        if (isIdentifierStart(cp)) return true;
        /*
        if (extended) {
            if (cp == 0x00B7) return true;
            if (cp == 0x0E33 || cp == 0x0EB3 || cp == 0xFF9E || cp == 0xFF9F) return true;
        }
        */
        byte cat = getCategory(cp);
        if (cat == Mn || cat == Mc || cat == Nd || cat == Pc) return true;
        if (getBinaryProperty(cp, Other_ID_Start)) return true;
        if (getBinaryProperty(cp, Other_ID_Continue)) return true;
        return false;
    }

    public boolean isIdentifier(String s) {
        if (s.length() == 0) return false; // at least one!
        int cp;
        for (int i = 0; i < s.length(); i += UTF32.count16(cp)) {
            cp = UTF32.char32At(s, i);
            if (i == 0) {
                if (!isIdentifierStart(cp)) return false;
            } else {
                if (!isIdentifierContinue_NO_Cf(cp)) return false;
            }
        }
        return true;
    }
    /*
Middle Dot. Because most Catalan legacy data will be encoded in Latin-1, U+00B7 MIDDLE DOT needs to be
allowed in <identifier_extend>.

In particular, the following four characters should be in <identifier_extend> and not <identifier_start>:
0E33 THAI CHARACTER SARA AM
0EB3 LAO VOWEL SIGN AM
FF9E HALFWIDTH KATAKANA VOICED SOUND MARK
FF9F HALFWIDTH KATAKANA SEMI-VOICED SOUND MARK
Irregularly decomposing characters. U+037A GREEK YPOGEGRAMMENI and certain Arabic presentation
forms have irregular compatibility decompositions, and need to be excluded from both <identifier_start>
and <identifier_extend>. It is recommended that all Arabic presentation forms be excluded from identifiers
in any event, although only a few of them are required to be excluded for normalization
to guarantee identifier closure.
*/

    // *******************
    // PRIVATES
    // *******************

        // cache of singletons
    private static Map versionCache = new HashMap();

    private static final int LIMIT_CODE_POINT = 0x110000;
    private static final UData[] ALL_NULLS = new UData[1024];

    // main data
    private UData[][] data = new UData[LIMIT_CODE_POINT>>10][];

    // extras
    private BitSet combiningClassSet = new BitSet(256);
    private String version;
    private String file;
    private long date = -1;
    private byte format = -1;
    //private byte major = -1;
    //private byte minor = -1;
    //private byte update = -1;
    private int compositeVersion = -1;
    private int size = -1;

    // cache last UData
    private int lastCode = Integer.MIN_VALUE;
    private UData lastResult = UData.UNASSIGNED;
    private boolean lastCodeFixed = false;

    // hide constructor
    private UCD() {
        for (int i = 0; i < data.length; ++i) {
            data[i] = ALL_NULLS;
        }
    }

    private void add(UData uData) {
        int high = uData.codePoint>>10;
        if (data[high] == ALL_NULLS) {
            UData[] temp = new UData[1024];
            data[high] = temp;
        }
        data[high][uData.codePoint & 0x3FF] = uData;
    }

    public boolean hasComputableName(int codePoint) {
        if (codePoint >= 0xF900 && codePoint <= 0xFA2D) return true;
        if (codePoint >= 0x2800 && codePoint <= 0x28FF) return true; 
        if (codePoint >= 0x2F800 && codePoint <= 0x2FA1D) return true;
        
        int rangeStart = mapToRepresentative(codePoint, compositeVersion);
        switch (rangeStart) {
          default:
            return getRaw(codePoint) == null;
          case 0x2800: // braille
          case 0xF900: // compat ideos
          case 0x2F800: // compat ideos
          case 0x3400: // CJK Ideograph Extension A
          case 0x4E00: // CJK Ideograph
          case 0x20000: // Extension B
          case 0xAC00: // Hangul Syllable
          case 0xE000: // Private Use
          case 0xF0000: // Private Use
          case 0x100000: // Private Use
          case 0xD800: // Surrogate
          case 0xDB80: // Private Use
          case 0xDC00: // Private Use
          case 0xFFFF: // Noncharacter
            return true;
        }
    }

    private UData getRaw(int codePoint) {
        return data[codePoint>>10][codePoint & 0x3FF];
    }

    // access data for codepoint
    UData get(int codePoint, boolean fixStrings) {
        /*if (codePoint == 0xF901) {
            System.out.println(version + ", " + Integer.toString(compositeVersion, 16));
            System.out.println("debug: ");
        }
        */
        if (codePoint < 0 || codePoint > 0x10FFFF) {
            throw new IllegalArgumentException("Illegal Code Point: " + Utility.hex(codePoint));
        }
        //if (codePoint == lastCode && fixStrings <= lastCodeFixed) return lastResult;
        /*
        // we play some funny tricks for performance
        // if cp is not represented, it is either in a elided block or missing.
        // elided blocks are either CONTINUE or FFFF

        byte cat;
        if (!ucdData.isRepresented(cp)) {
            int rep = UCD.mapToRepresentative(cp);
            if (rep == 0xFFFF) cat = Cn;
            else if (rep != cp) return CONTINUE;
            else if (!ucdData.isRepresented(rep)) cat = Cn;
            else cat = ucdData.getCategory(rep);
        } else {
            cat = ucdData.getCategory(cp);
        }
        */

        UData result = null;

        // do range stuff
        String constructedName = null;
        int rangeStart = mapToRepresentative(codePoint, compositeVersion);
        boolean isHangul = false;
        boolean isRemapped = false;
        switch (rangeStart) {
          case 0xF900:
            if (compositeVersion < 0x020105) {
                if (fixStrings) constructedName = "CJK COMPATIBILITY IDEOGRAPH-" + Utility.hex(codePoint, 4);
                break;
            }
            //isRemapped = true;
            break;
            // FALL THROUGH!!!!
          //default:
            /*
            result = getRaw(codePoint);
            if (result == null) {
                result = UData.UNASSIGNED;
                result.name = null; // clean this up, since we reuse UNASSIGNED
                result.shortName = null;
                if (fixStrings) {
                    result.name = "<unassigned-" + Utility.hex(codePoint, 4) + ">";
                }
            }
            if (fixStrings) {
                if (result.name == null) {
                    result.name = "<unassigned-" + Utility.hex(codePoint, 4) + ">";
                    // System.out.println("Warning: fixing name for " + result.name);
                }
                if (result.shortName == null) {
                    result.shortName = Utility.replace(result.name, UCD_Names.NAME_ABBREVIATIONS);
                }
            }
            */
            //break;
          case 0x3400: // CJK Ideograph Extension A
          case 0x4E00: // CJK Ideograph
          case 0x20000: // Extension B
            if (fixStrings) constructedName = "CJK UNIFIED IDEOGRAPH-" + Utility.hex(codePoint, 4);
            isRemapped = true;
            break;
          case 0xAC00: // Hangul Syllable
            isHangul = true;
            if (fixStrings) {
                constructedName = "HANGUL SYLLABLE " + getHangulName(codePoint);
            }
            isRemapped = true;
            break;
          case   0xE000: // Private Use
          case  0xF0000: // Private Use
          case 0x100000: // Private Use
            if (fixStrings) constructedName = "<private-use-" + Utility.hex(codePoint, 4) + ">";
            isRemapped = true;
            break;
          case 0xD800: // Surrogate
          case 0xDB80: // Private Use
          case 0xDC00: // Private Use
            if (fixStrings) constructedName = "<surrogate-" + Utility.hex(codePoint, 4) + ">";
            isRemapped = true;
            break;
          case 0xFFFF: // Noncharacter
            if (fixStrings) constructedName = "<noncharacter-" + Utility.hex(codePoint, 4) + ">";
            isRemapped = true;
            break;
        }
        result = getRaw(rangeStart);
        if (result == null) {
            result = UData.UNASSIGNED;
            isRemapped = true;
            result.name = null; // clean this up, since we reuse UNASSIGNED
            result.shortName = null;
            result.decompositionType = NONE;
            if (fixStrings) {
                constructedName = "<reserved-" + Utility.hex(codePoint, 4) + ">";
                //result.shortName = Utility.replace(result.name, UCD_Names.NAME_ABBREVIATIONS);
            }
            //return result;
        }

        result.codePoint = codePoint;
        if (fixStrings) {
            if (result.name == null || isRemapped) result.name = constructedName;
            if (result.shortName == null) result.shortName = Utility.replace(constructedName, UCD_Names.NAME_ABBREVIATIONS);
            if (isRemapped) {
                result.decompositionMapping = result.bidiMirror
                = result.simpleLowercase = result.simpleUppercase = result.simpleTitlecase = result.simpleCaseFolding
                = result.fullLowercase = result.fullUppercase = result.fullTitlecase = result.fullCaseFolding
                = UTF32.valueOf32(codePoint);
            }
        }
        if (isHangul) {
            if (fixStrings) result.decompositionMapping = getHangulDecompositionPair(codePoint);
            result.decompositionType = CANONICAL;
        }
        return result;
    }
    
    // Neither Mapped nor Composite CJK: [\u3400-\u4DB5\u4E00-\u9FA5\U00020000-\U0002A6D6]
    
    public static final boolean isCJK_AB(int bigChar) {
        return (CJK_A_BASE <= bigChar && bigChar < CJK_A_LIMIT
             || CJK_B_BASE <= bigChar && bigChar < CJK_B_LIMIT);
    }
    
    public static boolean isCJK_BASE(int cp) {
        return (CJK_BASE <= cp && cp < CJK_LIMIT 
        || cp == 0xFA0E	// compat characters that don't decompose.
        || cp == 0xFA0F
        || cp == 0xFA11
        || cp == 0xFA13
        || cp == 0xFA14
        || cp == 0xFA1F
        || cp == 0xFA21
        || cp == 0xFA23
        || cp == 0xFA24
        || cp == 0xFA27
        || cp == 0xFA28
        || cp == 0xFA29
        || cp == 0xFA2E
        || cp == 0xFA2F
        );
    }
    
    // Hangul constants

    public static final int
        SBase = 0xAC00, LBase = 0x1100, VBase = 0x1161, TBase = 0x11A7,
        LCount = 19, VCount = 21, TCount = 28,
        NCount = VCount * TCount,   // 588
        SCount = LCount * NCount,   // 11172
        LLimit = LBase + LCount,    // 1113
        VLimit = VBase + VCount,    // 1176
        TLimit = TBase + TCount,    // 11C3
        LLimitFull = 0x1160,
        VLimitFull = TBase,
        TLimitFull = 0x11FF,
        SLimit = SBase + SCount;    // D7A4

    private static String getHangulName(int s) {
        int SIndex = s - SBase;
        if (0 > SIndex || SIndex >= SCount) {
            throw new IllegalArgumentException("Not a Hangul Syllable: " + s);
        }
        int LIndex = SIndex / NCount;
        int VIndex = (SIndex % NCount) / TCount;
        int TIndex = SIndex % TCount;
        // if (true) return "?";
        return UCD_Names.JAMO_L_TABLE[LIndex] + UCD_Names.JAMO_V_TABLE[VIndex] + UCD_Names.JAMO_T_TABLE[TIndex];
    }

    private static final char[] pair = new char[2];
    
    static boolean isDoubleHangul(int s) {
        int SIndex = s - SBase;
        if (0 > SIndex || SIndex >= SCount) {
            throw new IllegalArgumentException("Not a Hangul Syllable: " + s);
        }
        return (SIndex % TCount) == 0;
    }

    static String getHangulDecompositionPair(int ch) {
        int SIndex = ch - SBase;
        if (0 > SIndex || SIndex >= SCount) {
            return "";
        }
        int TIndex = SIndex % TCount;
        if (TIndex != 0) { // triple
            pair[0] = (char)(SBase + SIndex - TIndex);
            pair[1] = (char)(TBase + TIndex);
        } else {
            pair[0] = (char)(LBase + SIndex / NCount);
            pair[1] = (char)(VBase + (SIndex % NCount) / TCount);
        }
        return String.valueOf(pair);
    }

    static int composeHangul(int char1, int char2) {
        if (LBase <= char1 && char1 < LLimit && VBase <= char2 && char2 < VLimit) {
            return (SBase + ((char1 - LBase) * VCount + (char2 - VBase)) * TCount);
        }
        if (SBase <= char1 && char1 < SLimit && TBase <= char2 && char2 < TLimit
                && ((char1 - SBase) % TCount) == 0) {
            return char1 + (char2 - TBase);
        }
        return 0xFFFF; // no composition
    }
    
    static public boolean isHangulSyllable(int char1) {
        return SBase <= char1 && char1 < SLimit;
    }

    static boolean isLeadingJamoComposition(int char1) {
        return isLeadingJamo(char1) || isLV(char1);
    }

    static boolean isLV(int char1) {
        return (SBase <= char1 && char1 < SLimit && ((char1 - SBase) % TCount) == 0);
    }

    static boolean isVowelJamo(int cp) {
        return (VBase <= cp && cp < VLimit);
    }

    static boolean isTrailingJamo(int cp) {
        return (TBase <= cp && cp < TLimit);
    }

    static boolean isLeadingJamo(int cp) {
        return (LBase <= cp && cp < LLimit);
    }

    static boolean isNonLeadJamo(int cp) {
        return (VBase <= cp && cp < VLimit) || (TBase <= cp && cp < TLimit);
    }
    
    byte getHangulSyllableType(int cp) {
        if (!isAssigned(cp)) return NA;
        if (LBase <= cp && cp < LLimitFull) return L;
        if (LLimitFull <= cp && cp < VLimitFull) return V;
        if (VLimitFull <= cp && cp < TLimitFull) return T;
        if (isLV(cp)) return LV;
        if (isHangulSyllable(cp)) return LVT;
        return NA;
    }

    static String getHangulSyllableTypeID_fromIndex(byte prop, byte style) {
        return prop < 0 || prop >= UCD_Names.HANGUL_SYLLABLE_TYPE.length ? null
        : (style == LONG) ? UCD_Names.LONG_HANGUL_SYLLABLE_TYPE[prop] 
        : UCD_Names.HANGUL_SYLLABLE_TYPE[prop];
    }

    String getHangulSyllableTypeID(int char1, byte style) {
        return getHangulSyllableTypeID_fromIndex(getHangulSyllableType(char1),style);
    }

    private void fillFromFile(String version) {
    	try {
    		fillFromFile2(version);
    	} catch (ChainException e) {
    		try {
    			ConvertUCD.main(new String[]{version});
    		} catch (Exception e2) {
            	throw new ChainException("Can't build data file for {0}", new Object[]{version}, e2);
    		}
    		fillFromFile2(version);
    	}
    }
    
    private void fillFromFile2(String version) {
        DataInputStream dataIn = null;
        String fileName = BIN_DIR + "UCD_Data" + version + ".bin";
        int uDataFileCount = 0;
        try {
            dataIn = new DataInputStream(
                new BufferedInputStream(
                    new FileInputStream(fileName),
                    128*1024));
            // header
            format = dataIn.readByte();
            byte major = dataIn.readByte();
            byte minor = dataIn.readByte();
            byte update = dataIn.readByte();
            compositeVersion = (major << 16) | (minor << 8) | update;
            
            String foundVersion = major + "." + minor + "." + update;
            if (format != BINARY_FORMAT || !version.equals(foundVersion)) {
                throw new ChainException("Illegal data file format for {0}: {1}, {2}",
                    new Object[]{version, new Byte(format), foundVersion});
            }
            date = dataIn.readLong();
            size = uDataFileCount = dataIn.readInt();

            boolean didJoiningHack = false;
            System.out.println("Loading UCD " + foundVersion);


            // records
            for (int i = 0; i < uDataFileCount; ++i) {
                UData uData = new UData();
                uData.readBytes(dataIn);

                //T = Mc + (Cf - ZWNJ - ZWJ)
                int cp = uData.codePoint;
                byte old = uData.joiningType;
                byte cat = uData.generalCategory;
                if (cat == Me) {
                	if (compositeVersion >= 0x40100) {
                		uData.joiningType = JT_T; 
                	}
                }
                //if (cp == 0x200D) {
                  //  uData.joiningType = JT_C;
                //} else
                /*
                if (cp != 0x200D && cp != 0x200C && (cat == Mn || cat == Cf)) {
                    uData.joiningType = JT_T;
                }
                */
                if (!didJoiningHack && uData.joiningType != old) {
                    System.out.println("HACK " + foundVersion + ": Setting "
                        + UCD_Names.LONG_JOINING_TYPE[uData.joiningType]
                        + ": " + Utility.hex(cp) + " " + uData.name);
                    didJoiningHack = true;
                }

                combiningClassSet.set(uData.combiningClass & 0xFF);
                add(uData);
            }
            /*
            if (update == -1) {
                throw new ChainException("Data File truncated for ",
                    new Object[]{version}, e);
            }
            if (size != fileSize) {
                throw new ChainException("Counts do not match: file {0}, records {1}",
                    new Object[]{new Integer(fileSize), new Integer(size)});
            }
            */
            // everything is ok!
            this.version = version;
            this.file = fileName;
            //+ " " + new File(fileName).lastModified();
        } catch (IOException e) {
            throw new ChainException("Can't read data file for {0}", new Object[]{version}, e);
        } finally {
            if (dataIn != null) {
                try {
                    dataIn.close();
                } catch (IOException e) {}
            }
        }
    }
    
    UnicodeMap blockData;
    public String getBlock(int codePoint) {
        if (blockData == null) loadBlocks();
        return (String)blockData.getValue(codePoint);
    }
    public List getBlockNames() {
            return getBlockNames(null);
    }
    public List getBlockNames(List result) {
        if (result == null) result = new ArrayList();
        if (blockData == null) loadBlocks();
        return (List)blockData.getAvailableValues(result);
    }
    public UnicodeSet getBlockSet(String value, UnicodeSet result) {
        if (result == null) result = new UnicodeSet();
        if (blockData == null) loadBlocks();
        return blockData.getSet(value, result);
    }
    
    private void loadBlocks() {
        blockData = new UnicodeMap();
        try {
            BufferedReader in = Utility.openUnicodeFile("Blocks", version, true, Utility.LATIN1);
            try {
                while (true) {
                    // 0000..007F; Basic Latin
                    String line = Utility.readDataLine(in);
                    if (line == null) break;
                    if (line.length() == 0) continue;
                    int pos1 = line.indexOf('.');
                    int pos2 = line.indexOf(';', pos1);
                        
                    //lastBlock = new BlockData();
                    int start = Integer.parseInt(line.substring(0, pos1), 16);
                    int end = Integer.parseInt(line.substring(pos1+2, pos2), 16);
                    String name = line.substring(pos2+1).trim().replace(' ', '_');
                    blockData.putAll(start,end, name);
                }
                blockData.setMissing("No_Block");
            } finally {
                in.close();
            }
        } catch (IOException e) {
            throw new IllegalArgumentException("Can't read block file");
        }
    }
    
    /*
    public static class BlockData {
        public int start;
        public int end;
        public String name;
    }
    
    public String NOBLOCK = Utility.getUnskeleton("no block", true);
    private BlockData lastBlock;
    
    public String getBlock(int codePoint) {
        if (blocks == null) loadBlocks();
        if (codePoint >= lastBlock.start && codePoint <= lastBlock.end) return lastBlock.name;
        Iterator it = blocks.iterator();
        while (it.hasNext()) {
            lastBlock = (BlockData) it.next();
            if (codePoint < lastBlock.start) continue;
            if (codePoint > lastBlock.end) break;
            return lastBlock.name;
        }
        return NOBLOCK;
    }
        
    public Collection getBlockNames(Collection result) {
        if (result == null) result = new ArrayList();
        if (blocks == null) loadBlocks();
        Iterator it = blocks.iterator();
        while (it.hasNext()) {
            BlockData data = (BlockData) it.next();
            UnicodeProperty.addUnique(data.name, result);
        }
        UnicodeProperty.addUnique(NOBLOCK, result);
        return result;
    }
    
    public boolean getBlockData(int blockId, BlockData output) {
        if (blocks == null) loadBlocks();
        BlockData temp;
        try {
            temp = (BlockData) blocks.get(blockId);
        } catch (IndexOutOfBoundsException e) {
            return false;
        }
        output.name = temp.name;
        output.start = temp.start;
        output.end = temp.end;
        return true;
    }
    
    private List blocks = null;
    
    private void loadBlocks() {
        blocks = new ArrayList();
        try {
            BufferedReader in = Utility.openUnicodeFile("Blocks", version, true, Utility.LATIN1);
            try {
                while (true) {
                    // 0000..007F; Basic Latin
                    String line = Utility.readDataLine(in);
                    if (line == null) break;
                    if (line.length() == 0) continue;
                    int pos1 = line.indexOf('.');
                    int pos2 = line.indexOf(';', pos1);
                    
                    lastBlock = new BlockData();
                    lastBlock.start = Integer.parseInt(line.substring(0, pos1), 16);
                    lastBlock.end = Integer.parseInt(line.substring(pos1+2, pos2), 16);
                    lastBlock.name = line.substring(pos2+1).trim().replace(' ', '_');
                    blocks.add(lastBlock);
                }
            } finally {
                in.close();
            }
        } catch (IOException e) {
            throw new IllegalArgumentException("Can't read block file");
        }
    }
    */
    /**
     * @return
     */
    public int getCompositeVersion() {
        return compositeVersion;
    }

    /**
     * @param i
     */
    public void setCompositeVersion(int i) {
        compositeVersion = i;
    }

}