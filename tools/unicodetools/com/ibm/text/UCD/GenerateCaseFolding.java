/**
*******************************************************************************
* Copyright (C) 1996-2001, International Business Machines Corporation and    *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
* $Source: /xsrl/Nsvn/icu/unicodetools/com/ibm/text/UCD/GenerateCaseFolding.java,v $
* $Date: 2002/03/15 01:57:01 $
* $Revision: 1.6 $
*
*******************************************************************************
*/

package com.ibm.text.UCD;

import java.util.*;
import java.io.*;
import com.ibm.icu.text.UTF16;

import com.ibm.text.utility.*;

public class GenerateCaseFolding implements UCD_Types {
    public static boolean DEBUG = false;
    public static boolean COMMENT_DIFFS = false; // ON if we want a comment on mappings != lowercase
    public static boolean PICK_SHORT = false; // picks short value for SIMPLE if in FULL, changes weighting
    public static boolean NF_CLOSURE = false; // picks short value for SIMPLE if in FULL, changes weighting
    // PICK_SHORT & NF_CLOSURE = false for old style
    
    
    /*public static void main(String[] args) throws java.io.IOException {
        makeCaseFold(arg[0]);
        //getAge();
    }
    */
    
    static PrintWriter log;
    
    public static void makeCaseFold(boolean normalized) throws java.io.IOException {
        PICK_SHORT = NF_CLOSURE = normalized;
        
        Main.setUCD();
        log = Utility.openPrintWriter("CaseFoldingLog" + GenerateData.getFileSuffix(true));
        System.out.println("Writing Log: " + "CaseFoldingLog" + GenerateData.getFileSuffix(true));
        
        System.out.println("Making Full Data");
        Map fullData = getCaseFolding(true, NF_CLOSURE);
        Utility.fixDot();
        System.out.println("Making Simple Data");
        Map simpleData = getCaseFolding(false, NF_CLOSURE);
        // write the data

        Utility.fixDot();
        System.out.println("Writing");
        String filename = "CaseFolding";
        if (normalized) filename += "-Normalized";
        String directory = "DerivedData/";
        String newFile = directory + filename + GenerateData.getFileSuffix(true);
        PrintWriter out = Utility.openPrintWriter(newFile);
        String mostRecent = GenerateData.generateBat(directory, filename, GenerateData.getFileSuffix(true));
        
        out.println("# CaseFolding" + GenerateData.getFileSuffix(false));
        out.println(GenerateData.generateDateLine());
        out.println("#");
        Utility.appendFile("CaseFoldingHeader.txt", false, out);
        
        /*
        PrintWriter out = new PrintWriter(
            new BufferedWriter(
            new OutputStreamWriter(
                new FileOutputStream(directory + fileRoot + GenerateData.getFileSuffix()),
                "UTF8"),
            4*1024));
        */
        
        for (int ch = 0; ch <= 0x10FFFF; ++ch) {
            Utility.dot(ch);

            if (!charsUsed.get(ch)) continue;
            
            String rFull = (String)fullData.get(UTF32.valueOf32(ch));
            String rSimple = (String)simpleData.get(UTF32.valueOf32(ch));
            if (rFull == null && rSimple == null) continue;
            if (rFull != null && rFull.equals(rSimple) 
              || (PICK_SHORT && UTF16.countCodePoint(rFull) == 1)) {
                String type = "C";
                if (ch == 0x130 || ch == 0x131) type = "I";
                drawLine(out, ch, type, rFull);
            } else {
                if (rFull != null) {
                    drawLine(out, ch, "F", rFull);
                }
                if (rSimple != null) {
                    drawLine(out, ch, "S", rSimple);
                }
            }
        }
        out.close();
        log.close();
        Utility.renameIdentical(mostRecent, Utility.getOutputName(newFile));
    }

    static void drawLine(PrintWriter out, int ch, String type, String result) {
        String comment = "";
        if (COMMENT_DIFFS) {
            String lower = Main.ucd.getCase(UTF16.valueOf(ch), FULL, LOWER);
            if (!lower.equals(result)) {
                String upper = Main.ucd.getCase(UTF16.valueOf(ch), FULL, UPPER);
                String lower2 = Main.ucd.getCase(UTF16.valueOf(ch), FULL, LOWER);
                if (lower.equals(lower2)) {
                    comment = "[Diff " + Utility.hex(lower, " ") + "] ";
                } else {
                    Utility.fixDot();
                    System.out.println("PROBLEM WITH: " + Main.ucd.getCodeAndName(ch));
                    comment = "[DIFF " + Utility.hex(lower, " ") + ", " + Utility.hex(lower2, " ") + "] ";
                }
            }
        }
        
        out.println(Utility.hex(ch)
            + "; " + type
            + "; " + Utility.hex(result, " ")
            + "; # " + comment + Main.ucd.getName(ch));
    }

    static int probeCh = 0x01f0;
    static String shower = UTF16.valueOf(probeCh);

    static Map getCaseFolding(boolean full, boolean nfClose) throws java.io.IOException {
        Map data = new TreeMap();
        Map repChar = new TreeMap();
        //String option = "";

        // get the equivalence classes

        for (int ch = 0; ch <= 0x10FFFF; ++ch) {
            Utility.dot(ch);
            //if ((ch & 0x3FF) == 0) System.out.println(Utility.hex(ch));
            if (!Main.ucd.isRepresented(ch)) continue;
            getClosure(ch, data, full, nfClose);
        }

        // get the representative characters
        
        Iterator it = data.keySet().iterator();
        while (it.hasNext()) {
            String s = (String) it.next();
            Set set = (Set) data.get(s);
            show = set.contains(shower);
            if (show) {
                Utility.fixDot();
                System.out.println(toString(set));
            }
            
        // Pick the best available representative
            
            String rep = null;
            int repGood = 0;
            String dup = null;
            Iterator it2 = set.iterator();
            while (it2.hasNext()) {
                String s2 = (String)it2.next();
                int s2Good = goodness(s2, full);
                if (s2Good > repGood) {
                    rep = s2;
                    repGood = s2Good;
                    dup = null;
                } else if (s2Good == repGood) {
                    dup = s2;
                }
            }
            if (rep == null) {
                Utility.fixDot();
                System.err.println("No representative for: " + toString(set));
            } else if ((repGood & (NFC_FORMAT | ISLOWER)) != (NFC_FORMAT | ISLOWER)) {
                String message = "";
                if ((repGood & NFC_FORMAT) == 0) {
                    message += " [NOT NFC FORMAT]";
                }
                if ((repGood & ISLOWER) == 0) {
                    message += " [NOT LOWERCASE]";
                }
                Utility.fixDot();
                log.println("Non-Optimal Representative " + message);
                log.println(" Rep:\t" + Main.ucd.getCodeAndName(rep));
                log.println(" Set:\t" + toString(set,true, true));
            }
            
        // Add it for all the elements of the set
        
            it2 = set.iterator();
            while (it2.hasNext()) {
                String s2 = (String)it2.next();
                if (UTF16.countCodePoint(s2) == 1 && !s2.equals(rep)) {
                    repChar.put(UTF32.getCodePointSubstring(s2,0), rep);
                    charsUsed.set(UTF16.charAt(s2, 0));
                }
            }
        }
        return repChar;
    }
    
    static BitSet charsUsed = new BitSet();
    static boolean show = false;
    static final int NFC_FORMAT = 64;
    static final int ISLOWER = 128;

    static int goodness(String s, boolean full) {
        if (s == null) return 0;
        int result = 32-s.length();
        if (!PICK_SHORT) {
            result = s.length();
        }
        if (!full) result <<= 8;
        String low = lower(upper(s, full), full);
        if (s.equals(low)) result |= ISLOWER;
        else if (PICK_SHORT && Main.nfd.normalize(s).equals(Main.nfd.normalize(low))) result |= ISLOWER;
        
        if (s.equals(Main.nfc.normalize(s))) result |= NFC_FORMAT;
        
        if (show) {
            Utility.fixDot();
            System.out.println(Utility.hex(result) + ", " + Main.ucd.getCodeAndName(s));
        }
        return result;
    }


    /*
    static HashSet temp = new HashSet();
    static void normalize(HashSet set) {
        temp.clear();
        temp.addAll(set);
        set.clear();
        Iterator it = temp.iterator();
        while (it.hasNext()) {
            String s = (String) it.next();
            String s2 = KC.normalize(s);
            set.add(s);
            data2.put(s,set);
            if (!s.equals(s2)) {
                set.add(s2);
                data2.put(s2,set);
                System.err.println("Adding " + Utility.hex(s) + " by " + Utility.hex(s2));
            }
        }
    }
    */

            /*
            String
            String lower1 = Main.ucd.getLowercase(ch);
            String lower2 = Main.ucd.toLowercase(ch,option);

            char ch2 = Main.ucd.getLowercase(Main.ucd.getUppercase(ch).charAt(0)).charAt(0);
            //String lower1 = String.valueOf(Main.ucd.getLowercase(ch));
            //String lower = Main.ucd.toLowercase(ch2,option);
            String upper = Main.ucd.toUppercase(ch2,option);
            String lowerUpper = Main.ucd.toLowercase(upper,option);
            //String title = Main.ucd.toTitlecase(ch2,option);
            //String lowerTitle = Main.ucd.toLowercase(upper,option);

            if (ch != ch2 || lowerUpper.length() != 1 || ch != lowerUpper.charAt(0)) { //
                output.println(Utility.hex(ch)
                    + "; " + (lowerUpper.equals(lower1) ? "L" : lowerUpper.equals(lower2) ? "S" : "E")
                    + "; " + Utility.hex(lowerUpper," ")
                    + ";\t#" + Main.ucd.getName(ch)
                    );
                //if (!lowerUpper.equals(lower)) {
                //    output.println("Warning1: " + Utility.hex(lower) + " " + Main.ucd.getName(lower));
                //}
                //if (!lowerUpper.equals(lowerTitle)) {
                //    output.println("Warning2: " + Utility.hex(lowerTitle) + " " + Main.ucd.getName(lowerTitle));
                //}
            }
            */

    static void getClosure(int ch, Map data, boolean full, boolean nfClose) {
        String charStr = UTF32.valueOf32(ch);
        String lowerStr = lower(charStr, full);
        String titleStr = title(charStr, full);
        String upperStr = upper(charStr, full);
        if (charStr.equals(lowerStr) && charStr.equals(upperStr) && charStr.equals(titleStr)) return;
        if (DEBUG) System.err.println("Closure for " + Utility.hex(ch));

        // make new set
        Set set = new TreeSet();
        set.add(charStr);
        data.put(charStr, set);

        // add cases to get started
        add(set, lowerStr, data);
        add(set, upperStr, data);
        add(set, titleStr, data);

        // close it
        main:
        while (true) {
            Iterator it = set.iterator();
            while (it.hasNext()) {
                String s = (String) it.next();
                // do funny stuff since we can't modify set while iterating
                // We don't do this because if the source is not normalized, we don't want to normalize
                if (nfClose) {
                    if (add(set, Main.nfd.normalize(s), data)) continue main;
                    if (add(set, Main.nfc.normalize(s), data)) continue main;
                    if (add(set, Main.nfkd.normalize(s), data)) continue main;
                    if (add(set, Main.nfkc.normalize(s), data)) continue main;
                }
                if (add(set, lower(s, full), data)) continue main;
                if (add(set, title(s, full), data)) continue main;
                if (add(set, upper(s, full), data)) continue main;
            }
            break;
        }
    }

    static String lower(String s, boolean full) {
        String result = lower2(s,full);
        return result.replace('\u03C2', '\u03C3'); // HACK for lower
    }

    // These functions are no longer necessary, since Main.ucd is parameterized,
    // but it's not worth changing

    static String lower2(String s, boolean full) {
        /*if (!full) {
            if (s.length() != 1) return s;
            return Main.ucd.getCase(UTF32.char32At(s,0), SIMPLE, LOWER);
        }
        */
        return Main.ucd.getCase(s, full ? FULL : SIMPLE, LOWER);
    }

    static String upper(String s, boolean full) {
        /* if (!full) {
            if (s.length() != 1) return s;
            return Main.ucd.getCase(UTF32.char32At(s,0), FULL, UPPER);
        }
        */
        return Main.ucd.getCase(s, full ? FULL : SIMPLE, UPPER);
    }

    static String title(String s, boolean full) {
        /*if (!full) {
            if (s.length() != 1) return s;
            return Main.ucd.getCase(UTF32.char32At(s,0), FULL, TITLE);
        }
        */
        return Main.ucd.getCase(s, full ? FULL : SIMPLE, TITLE);
    }

    static boolean add(Set set, String s, Map data) {
        if (set.contains(s)) return false;
        set.add(s);
        if (DEBUG) System.err.println("adding: " + toString(set));
        Set other = (Set) data.get(s);
        if (other != null && other != set) { // merge
            // make all the items in set point to merged set
            Iterator it = other.iterator();
            while (it.hasNext()) {
                data.put(it.next(), set);
            }
            set.addAll(other);
        }
        if (DEBUG) System.err.println("done adding: " + toString(set));
        return true;
    }

    static String toString(Set set) {
        return toString(set, false, false);
    }

    static String toString(Set set, boolean name, boolean crtab) {
        String result = "{";
        Iterator it2 = set.iterator();
        boolean first = true;
        while (it2.hasNext()) {
            String s2 = (String) it2.next();
            if (!first) {
                if (crtab) {
                    result += ";\r\n\t";
                } else {
                    result += "; ";
                }
            }
            first = false;
            if (name) {
                result += Main.ucd.getCodeAndName(s2);
            } else {
                result += Utility.hex(s2, " ");
            }
        }
        return result + "}";
    }
    
    static boolean specialNormalizationDiffers(int ch) {
        if (ch == 0x00DF) return true;                  // es-zed
        return Main.nfkd.normalizationDiffers(ch);
    }
    
    static String specialNormalization(String s) {
        if (s.equals("\u00DF")) return "ss";
        return Main.nfkd.normalize(s);
    }
    
    static boolean isExcluded(int ch) {
        if (ch == 0x130) return true;                  // skip LATIN CAPITAL LETTER I WITH DOT ABOVE
        if (ch == 0x0132 || ch == 0x0133) return true; // skip IJ, ij
        if (ch == 0x037A) return true;                 // skip GREEK YPOGEGRAMMENI
        if (0x249C <= ch && ch <= 0x24B5) return true; // skip PARENTHESIZED LATIN SMALL LETTER A..
        if (0x20A8 <= ch && ch <= 0x217B) return true; // skip Rupee..
        
        byte type = Main.ucd.getDecompositionType(ch);  
        if (type == COMPAT_SQUARE) return true;
        //if (type == COMPAT_UNSPECIFIED) return true;
        return false;
    }
    
    static void generateSpecialCasing(boolean normalize) throws IOException {
        Main.setUCD();
        Map sorted = new TreeMap();
        
        String suffix2 = "";
        if (normalize) suffix2 = "-Normalized";
        
        PrintWriter log = Utility.openPrintWriter("SpecialCasingExceptions" + suffix2 + GenerateData.getFileSuffix(true));
        
        for (int ch = 0; ch <= 0x10FFFF; ++ch) {
            Utility.dot(ch);
            if (!Main.ucd.isRepresented(ch)) continue;
            if (!specialNormalizationDiffers(ch)) continue;

            String lower = Main.nfc.normalize(Main.ucd.getCase(ch, SIMPLE, LOWER));
            String upper = Main.nfc.normalize(Main.ucd.getCase(ch, SIMPLE, UPPER));
            String title = Main.nfc.normalize(Main.ucd.getCase(ch, SIMPLE, TITLE));
            
            String chstr = UTF16.valueOf(ch);
            
            String decomp = specialNormalization(chstr);
            String flower = Main.nfc.normalize(Main.ucd.getCase(decomp, SIMPLE, LOWER));
            String fupper = Main.nfc.normalize(Main.ucd.getCase(decomp, SIMPLE, UPPER));
            String ftitle = Main.nfc.normalize(Main.ucd.getCase(decomp, SIMPLE, TITLE));
            
            String base = decomp;
            String blower = specialNormalization(lower);
            String bupper = specialNormalization(upper);
            String btitle = specialNormalization(title);

            if (true) {
                flower = Main.nfc.normalize(flower);
                fupper = Main.nfc.normalize(fupper);
                ftitle = Main.nfc.normalize(ftitle);
                base = Main.nfc.normalize(base);
                blower = Main.nfc.normalize(blower);
                bupper = Main.nfc.normalize(bupper);
                btitle = Main.nfc.normalize(btitle);
            }
            
            if (ch == -1) {// for debugging, change to actual character
                System.out.println("Code: " + Main.ucd.getCodeAndName(ch));
                System.out.println("Decomp: " + Main.ucd.getCodeAndName(decomp));
                System.out.println("Base: " + Main.ucd.getCodeAndName(base));
                System.out.println("SLower: " + Main.ucd.getCodeAndName(lower));
                System.out.println("FLower: " + Main.ucd.getCodeAndName(flower));
                System.out.println("BLower: " + Main.ucd.getCodeAndName(blower));
                System.out.println("STitle: " + Main.ucd.getCodeAndName(title));
                System.out.println("FTitle: " + Main.ucd.getCodeAndName(ftitle));
                System.out.println("BTitle: " + Main.ucd.getCodeAndName(btitle));
                System.out.println("SUpper: " + Main.ucd.getCodeAndName(upper));
                System.out.println("FUpper: " + Main.ucd.getCodeAndName(fupper));
                System.out.println("BUpper: " + Main.ucd.getCodeAndName(bupper));
            }
            
            // presumably if there is a single code point, it would already be in the simple mappings
            
            if (UTF16.countCodePoint(flower) == 1 && UTF16.countCodePoint(fupper) == 1 
                && UTF16.countCodePoint(title) == 1) continue;
            
            // if there is no change from the base, skip
            
            if (flower.equals(base) && fupper.equals(base) && ftitle.equals(base)) continue;
            
            // fix special cases
            // if (flower.equals(blower) && fupper.equals(bupper) && ftitle.equals(btitle)) continue;
            if (flower.equals(blower)) flower = lower;
            if (fupper.equals(bupper)) fupper = upper;
            if (ftitle.equals(btitle)) ftitle = title;
            
            // if there are no changes from the original, or the expanded original, skip
            
            if (flower.equals(lower) && fupper.equals(upper) && ftitle.equals(title)) continue;
            
            String name = Main.ucd.getName(ch);
            
            int order = name.equals("LATIN SMALL LETTER SHARP S") ? 1
                : name.indexOf("ARMENIAN SMALL LIGATURE") >= 0 ? 3
                : name.indexOf("LIGATURE") >= 0 ? 2
                : name.indexOf("GEGRAMMENI") < 0 ? 4
                : UTF16.countCodePoint(ftitle) == 1 ? 5
                : UTF16.countCodePoint(fupper) == 2 ? 6
                : 7;
            
            // HACK
            boolean denormalize = !normalize && order != 5 && order != 6;
            
            String mapping = Utility.hex(ch)
                + "; " + Utility.hex(flower.equals(base) ? chstr : denormalize ? Main.nfd.normalize(flower) : flower)
                + "; " + Utility.hex(ftitle.equals(base) ? chstr : denormalize ? Main.nfd.normalize(ftitle) : ftitle)
                + "; " + Utility.hex(fupper.equals(base) ? chstr : denormalize ? Main.nfd.normalize(fupper) : fupper)
                + "; # " + Main.ucd.getName(ch);
            
            // special exclusions 
            if (isExcluded(ch)) {
                log.println("# " + mapping);
            } else {
                int x = ch;
                if (ch == 0x01F0) x = 0x03B1; // HACK to reorder the same
                sorted.put(new Integer((order << 24) | x), mapping);
            }
        }
        log.close();
        
        System.out.println("Writing");
        String newFile = "DerivedData/SpecialCasing" + suffix2 + GenerateData.getFileSuffix(true);
        PrintWriter out = Utility.openPrintWriter(newFile);
        String mostRecent = GenerateData.generateBat("DerivedData/", "SpecialCasing", suffix2 + GenerateData.getFileSuffix(true));
        out.println("# SpecialCasing" + GenerateData.getFileSuffix(false));
        out.println(GenerateData.generateDateLine());
        out.println("#");
        Utility.appendFile("SpecialCasingHeader.txt", true, out);

        Iterator it = sorted.keySet().iterator();
        int lastOrder = -1;
        while (it.hasNext()) {
            Integer key = (Integer) it.next();
            String line = (String) sorted.get(key);
            int order = key.intValue() >> 24;
            if (order != lastOrder) {
                lastOrder = order;
                out.println();
                boolean skipLine = false;
                switch(order) {
                case 1: 
                    out.println("# The German es-zed is special--the normal mapping is to SS.");
                    out.println("# Note: the titlecase should never occur in practice. It is equal to titlecase(uppercase(<es-zed>))");
                    break;
                case 2: out.println("# Ligatures"); break;
                case 3: skipLine = true; break;
                case 4: out.println("# No corresponding uppercase precomposed character"); break;
                case 5: Utility.appendFile("SpecialCasingIota.txt", true, out); break;
                case 6: out.println("# Some characters with YPOGEGRAMMENI are also have no corresponding titlecases"); break;
                case 7: skipLine = true; break;
                }
                if (!skipLine) out.println();
            }
            out.println(line);
        }
        Utility.appendFile("SpecialCasingFooter.txt", true, out);
        out.close();
        Utility.renameIdentical(mostRecent, Utility.getOutputName(newFile));
    }
}