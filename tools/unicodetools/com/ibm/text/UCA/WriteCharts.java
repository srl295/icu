/**
*******************************************************************************
* Copyright (C) 1996-2001, International Business Machines Corporation and    *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
* $Source: /xsrl/Nsvn/icu/unicodetools/com/ibm/text/UCA/WriteCharts.java,v $
* $Date: 2004/02/07 01:01:12 $
* $Revision: 1.19 $
*
*******************************************************************************
*/

package com.ibm.text.UCA;

import java.util.*;

import java.io.*;
import com.ibm.text.UCD.*;
import com.ibm.text.utility.*;
import com.ibm.icu.text.UTF16;
import com.ibm.icu.text.UnicodeSetIterator;
import com.ibm.icu.text.Transliterator;
import com.ibm.icu.text.UnicodeSet;
import java.text.NumberFormat;


import java.text.SimpleDateFormat;

public class WriteCharts implements UCD_Types {

    static boolean HACK_KANA = false;

    static public void special() {
    	
    	for (int i = 0xE000; i < 0x10000; ++i) {
    		if (!Default.ucd().isRepresented(i)) continue;
    		if (!Default.nfkc().isNormalized(i)) continue;
    		System.out.println(Default.ucd().getCodeAndName(i));
    	}
    }

    static public void collationChart(UCA uca) throws IOException {
    	Default.setUCD(uca.getUCDVersion());
    	HACK_KANA = true;

        uca.setAlternate(UCA.NON_IGNORABLE);

        //Normalizer nfd = new Normalizer(Normalizer.NFD);
        //Normalizer nfc = new Normalizer(Normalizer.NFC);

        UCA.UCAContents cc = uca.getContents(UCA.FIXED_CE, null); // nfd instead of null if skipping decomps
        cc.enableSamples();

        Set set = new TreeSet();

        while (true) {
            String x = cc.next();
            if (x == null) break;
            if (x.equals("\u2F00")) {
            	System.out.println("debug");
            }

            set.add(new Pair(uca.getSortKey(x), x));
        }

        PrintWriter output = null;

        Iterator it = set.iterator();

        byte oldScript = -127;

        int[] scriptCount = new int[128];

        int counter = 0;

        String lastSortKey = "\u0000";

        int high = uca.getSortKey("a").charAt(0);
        int variable = UCA.getPrimary(uca.getVariableHigh());

        int columnCount = 0;

        String[] replacement = new String[] {"%%%", "Collation Charts"};
        String folder = "charts\\uca\\";

        Utility.copyTextFile("index.html", Utility.UTF8, folder + "index.html", replacement);
        Utility.copyTextFile("charts.css", Utility.LATIN1, folder + "charts.css");
        Utility.copyTextFile("help.html", Utility.UTF8, folder + "help.html");

        indexFile = Utility.openPrintWriter(folder + "index_list.html", Utility.UTF8_WINDOWS);
        Utility.appendFile("index_header.html", Utility.UTF8, indexFile, replacement);

        /*
        indexFile.println("<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'>");
        indexFile.println("<title>UCA Default Collation Table</title>");
        indexFile.println("<base target='main'>");
        indexFile.println("<style><!-- p { font-size: 90% } --></style>");
        indexFile.println("</head><body><h2 align='center'>UCA Default Collation Table</h2>");
        indexFile.println("<p align='center'><a href = 'help.html'>Help</a>");
        */

        while (it.hasNext()) {
            Utility.dot(counter);

            Pair p = (Pair) it.next();
            String sortKey = (String) p.first;
            String s = (String) p.second;

            int cp = UTF16.charAt(s,0);

            byte script = Default.ucd().getScript(cp);

            // get first non-zero primary
            int currentPrimary = getFirstPrimary(sortKey);
            int primary = currentPrimary >>> 16;

            if (sortKey.length() < 4) script = NULL_ORDER;
            else if (primary == 0) script = IGNORABLE_ORDER;
            else if (primary <= variable) script = VARIABLE_ORDER;
            else if (primary < high) script = COMMON_SCRIPT;
            else if (UCA.isImplicitLeadPrimary(primary)) {
                if (primary < UCA_Types.UNSUPPORTED_CJK_AB_BASE) script = CJK;
                else if (primary < UCA_Types.UNSUPPORTED_OTHER_BASE) script = CJK_AB;
                else script = UNSUPPORTED;
            }

            if (script == KATAKANA_SCRIPT) script = HIRAGANA_SCRIPT;
            else if ((script == INHERITED_SCRIPT || script == COMMON_SCRIPT) && oldScript >= 0) script = oldScript;

            if (script != oldScript
                    // && (script != COMMON_SCRIPT && script != INHERITED_SCRIPT)
                    ) {
                closeFile(output);
                output = null;
                oldScript = script;
            }

            if (output == null) {
                ++scriptCount[script+3];
                if (scriptCount[script+3] > 1) {
                    System.out.println("\t\tFAIL: " + scriptCount[script+3] + ", " +
                        getChunkName(script, LONG) + ", " + Default.ucd().getCodeAndName(s));
                }
                output = openFile(scriptCount[script+3], folder, script);
            }

            boolean firstPrimaryEquals = currentPrimary == getFirstPrimary(lastSortKey);

            int strength = uca.strengthDifference(sortKey, lastSortKey);
            if (strength < 0) strength = -strength;
            lastSortKey = sortKey;

            // find out if this is an expansion: more than one primary weight

            int primaryCount = 0;
            for (int i = 0; i < sortKey.length(); ++i) {
                char w = sortKey.charAt(i);
                if (w == 0) break;
				if (UCA.isImplicitLeadPrimary(w)) {
					++i; // skip next
				}
                ++ primaryCount;
            }

            String breaker = "";
            if (columnCount > 10 || !firstPrimaryEquals) {
                columnCount = 0;
                if (!firstPrimaryEquals || script == UNSUPPORTED) breaker = "</tr><tr>";
                else {
                	breaker = "</tr><tr><td></td>"; // indent 1 cell
                	++columnCount;
                }
            }

            String classname = primaryCount > 1 ? XCLASSNAME[strength] : CLASSNAME[strength];

            String outline = showCell2(sortKey, s, script, classname);

            output.println(breaker + outline);
            ++columnCount;
        }

        closeFile(output);
        closeIndexFile(indexFile, "<br>UCA: " + uca.getDataVersion(), COLLATION);
    }

    private static String showCell2(
        String sortKey,
        String s,
        byte script,
        String classname) {
        String name = Default.ucd().getName(s);
        
        
        if (s.equals("\u1eaf")) {
        	System.out.println("debug");
        }
        
        String comp = Default.nfc().normalize(s);
        int cat = Default.ucd().getCategory(UTF16.charAt(comp,0));
        if (cat == Mn || cat == Mc || cat == Me) {
            comp = '\u25CC' + comp;
            if (s.equals("\u0300")) {
                System.out.println(Default.ucd().getCodeAndName(comp));
            } 
        } 
        // TODO: merge with showCell
        
        String outline = classname
            + " title='"
            + (script != UNSUPPORTED
                ? Utility.quoteXML(name, true) + ": "
                : "")
            + UCA.toString(sortKey) + "'>"
            + Utility.quoteXML(comp, true)
            + "<br><tt>"
            + Utility.hex(s)
            //+ "<br>" + script
            + "</tt></td>"
            + (script == UNSUPPORTED
                ? "<td class='name'><tt>" + Utility.quoteXML(name, true) + "</td>"
                : "")
            ;
        return outline;
    }

    static public void normalizationChart() throws IOException {
    	HACK_KANA = false;

        Set set = new TreeSet();

        for (int i = 0; i <= 0x10FFFF; ++i) {
        	if (!Default.ucd().isRepresented(i)) {
        		if (i < 0xAC00) continue;
        		if (i > 0xD7A3) continue;
        		if (i > 0xACFF && i < 0xD700) continue;
        	}
        	byte cat = Default.ucd().getCategory(i);
        	if (cat == Cs || cat == Co) continue;

        	if (Default.nfkd().isNormalized(i)) continue;
        	String decomp = Default.nfkd().normalize(i);

        	byte script = getBestScript(decomp);

            set.add(new Pair(new Integer(script == COMMON_SCRIPT ? cat + CAT_OFFSET : script),
            		new Pair(Default.ucd().getCase(decomp, FULL, FOLD),
            				 new Integer(i))));
        }

        PrintWriter output = null;

        Iterator it = set.iterator();

        int oldScript = -127;

        int counter = 0;

        String[] replacement = new String[] {"%%%", "Normalization Charts"};
        String folder = "charts\\normalization\\";

        Utility.copyTextFile("index.html", Utility.UTF8, folder + "index.html", replacement);
        Utility.copyTextFile("charts.css", Utility.LATIN1, folder + "charts.css");
        Utility.copyTextFile("norm_help.html", Utility.UTF8, folder + "help.html");

        indexFile = Utility.openPrintWriter(folder + "index_list.html", Utility.UTF8_WINDOWS);
        Utility.appendFile("index_header.html", Utility.UTF8, indexFile, replacement);

        /*
        indexFile.println("<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'>");
        indexFile.println("<title>UCA Default Collation Table</title>");
        indexFile.println("<base target='main'>");
        indexFile.println("<style><!-- p { font-size: 90% } --></style>");
        indexFile.println("</head><body><h2 align='center'>UCA Default Collation Table</h2>");
        indexFile.println("<p align='center'><a href = 'help.html'>Help</a>");
        */

        while (it.hasNext()) {
            Utility.dot(counter);

            Pair p = (Pair) it.next();
            int script = ((Integer) p.first).intValue();
            int cp = ((Integer)((Pair) p.second).second).intValue();

            if (script != oldScript
                    // && (script != COMMON_SCRIPT && script != INHERITED_SCRIPT)
                    ) {
                closeFile(output);
                output = null;
                oldScript = script;
            }

            if (output == null) {
                output = openFile(0, folder, script);
                output.println("<tr><td class='z'>Code</td><td class='z'>C</td><td class='z'>D</td><td class='z'>KC</td><td class='z'>KD</td></tr>");

            }

            output.println("<tr>");

            String prefix;
            String code = UTF16.valueOf(cp);
            String c = Default.nfc().normalize(cp);
            String d = Default.nfd().normalize(cp);
            String kc = Default.nfkc().normalize(cp);
            String kd = Default.nfkd().normalize(cp);

            showCell(output, code, "<td class='z' ", "", false);

            prefix = c.equals(code) ? "<td class='g' " : "<td class='n' ";
            showCell(output, c, prefix, "", c.equals(code));

            prefix = d.equals(c) ? "<td class='g' " : "<td class='n' ";
            showCell(output, d, prefix, "", d.equals(c));

            prefix = kc.equals(c) ? "<td class='g' " : "<td class='n' ";
            showCell(output, kc, prefix, "", kc.equals(c));

            prefix = (kd.equals(d) || kd.equals(kc)) ? "<td class='g' " : "<td class='n' ";
            showCell(output, kd, prefix, "", (kd.equals(d) || kd.equals(kc)));

            output.println("</tr>");

        }

        closeFile(output);
        closeIndexFile(indexFile, "", NORMALIZATION);
    }

    static public void caseChart() throws IOException {
    	HACK_KANA = false;

        Set set = new TreeSet();

        for (int i = 0; i <= 0x10FFFF; ++i) {
        	if (!Default.ucd().isRepresented(i)) continue;
        	byte cat = Default.ucd().getCategory(i);
        	if (cat == Cs || cat == Co) continue;

            String code = UTF16.valueOf(i);
            String lower = Default.ucd().getCase(i, FULL, LOWER);
            String title = Default.ucd().getCase(i, FULL, TITLE);
            String upper = Default.ucd().getCase(i, FULL, UPPER);
            String fold = Default.ucd().getCase(i, FULL, FOLD);

        	String decomp = Default.nfkd().normalize(i);
        	int script = 0;
            if (lower.equals(code) && upper.equals(code) && fold.equals(code) && title.equals(code)) {
            	if (!containsCase(decomp)) continue;
            	script = NO_CASE_MAPPING;
        	}

        	if (script == 0) script = getBestScript(decomp);

            set.add(new Pair(new Integer(script == COMMON_SCRIPT ? cat + CAT_OFFSET : script),
            		new Pair(Default.ucd().getCase(decomp, FULL, FOLD),
            				 new Integer(i))));
        }

        PrintWriter output = null;

        Iterator it = set.iterator();

        int oldScript = -127;

        int counter = 0;
        String[] replacement = new String[] {"%%%", "Case Charts"};
        String folder = "charts\\case\\";

        Utility.copyTextFile("index.html", Utility.UTF8, folder + "index.html", replacement);
        Utility.copyTextFile("charts.css", Utility.LATIN1, folder + "charts.css");
        Utility.copyTextFile("case_help.html", Utility.UTF8, folder + "help.html");

        indexFile = Utility.openPrintWriter(folder + "index_list.html", Utility.UTF8_WINDOWS);
        Utility.appendFile("index_header.html", Utility.UTF8, indexFile, replacement);

        /*
        indexFile.println("<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'>");
        indexFile.println("<title>UCA Default Collation Table</title>");
        indexFile.println("<base target='main'>");
        indexFile.println("<style><!-- p { font-size: 90% } --></style>");
        indexFile.println("</head><body><h2 align='center'>UCA Default Collation Table</h2>");
        indexFile.println("<p align='center'><a href = 'help.html'>Help</a>");
        */

        int columnCount = 0;

        while (it.hasNext()) {
            Utility.dot(counter);

            Pair p = (Pair) it.next();
            int script = ((Integer) p.first).intValue();
            int cp = ((Integer)((Pair) p.second).second).intValue();

            if (script != oldScript
                    // && (script != COMMON_SCRIPT && script != INHERITED_SCRIPT)
                    ) {
                closeFile(output);
                output = null;
                oldScript = script;
            }

            if (output == null) {
                output = openFile(0, folder, script);
                if (script == NO_CASE_MAPPING) output.println("<tr>");
                else output.println("<tr><td class='z'>Code</td><td class='z'>Lower</td><td class='z'>Title</td>"
                	+"<td class='z'>Upper</td><td class='z'>Fold</td></tr>");

            }

            if (script == NO_CASE_MAPPING) {
            	if (columnCount > 10) {
            		output.println("</tr><tr>");
            		columnCount = 0;
            	}
            	showCell(output, UTF16.valueOf(cp), "<td ", "", false);
            	++columnCount;
            	continue;
            }

            output.println("<tr>");

            String prefix;
            String code = UTF16.valueOf(cp);
            String lower = Default.ucd().getCase(cp, FULL, LOWER);
            String title = Default.ucd().getCase(cp, FULL, TITLE);
            String upper = Default.ucd().getCase(cp, FULL, UPPER);
            String fold = Default.ucd().getCase(cp, FULL, FOLD);

            showCell(output, code, "<td class='z' ", "", false);

            prefix = lower.equals(code) ? "<td class='g' " : "<td class='n' ";
            showCell(output, lower, prefix, "", lower.equals(code));

            prefix = title.equals(upper) ? "<td class='g' " : "<td class='n' ";
            showCell(output, title, prefix, "", title.equals(upper));

            prefix = upper.equals(code) ? "<td class='g' " : "<td class='n' ";
            showCell(output, upper, prefix, "", upper.equals(code));

            prefix = fold.equals(lower) ? "<td class='g' " : "<td class='n' ";
            showCell(output, fold, prefix, "", fold.equals(lower));

            output.println("</tr>");

        }

        closeFile(output);
        closeIndexFile(indexFile, "", CASE);
    }
    
	static public void scriptChart() throws IOException {
			HACK_KANA = false;

			Set set = new TreeSet();

			for (int i = 0; i <= 0x10FFFF; ++i) {
				if (!Default.ucd().isRepresented(i)) continue;
				byte cat = Default.ucd().getCategory(i);
				if (cat == Cs || cat == Co || cat == Cn) continue;

				String code = UTF16.valueOf(i);

				String decomp = Default.nfkd().normalize(i);
				int script = getBestScript(decomp);

				set.add(new Pair(new Integer(script == COMMON_SCRIPT ? cat + CAT_OFFSET : script),
						new Pair(decomp,
								 new Integer(i))));
			}

			PrintWriter output = null;

			Iterator it = set.iterator();

			int oldScript = -127;

			int counter = 0;
			String[] replacement = new String[] {"%%%", "Script Charts"};
			String folder = "charts\\script\\";

			Utility.copyTextFile("index.html", Utility.UTF8, folder + "index.html", replacement);
			Utility.copyTextFile("charts.css", Utility.LATIN1, folder + "charts.css");
			Utility.copyTextFile("script_help.html", Utility.UTF8, folder + "help.html");

			indexFile = Utility.openPrintWriter(folder + "index_list.html", Utility.UTF8_WINDOWS);
			Utility.appendFile("script_index_header.html", Utility.UTF8, indexFile, replacement);

			/*
			indexFile.println("<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'>");
			indexFile.println("<title>UCA Default Collation Table</title>");
			indexFile.println("<base target='main'>");
			indexFile.println("<style><!-- p { font-size: 90% } --></style>");
			indexFile.println("</head><body><h2 align='center'>UCA Default Collation Table</h2>");
			indexFile.println("<p align='center'><a href = 'help.html'>Help</a>");
			*/

			int columnCount = 0;

			while (it.hasNext()) {
				Utility.dot(counter);

				Pair p = (Pair) it.next();
				int script = ((Integer) p.first).intValue();
				int cp = ((Integer)((Pair)p.second).second).intValue();

				if (script != oldScript
						// && (script != COMMON_SCRIPT && script != INHERITED_SCRIPT)
						) {
					closeFile(output);
					output = null;
					oldScript = script;
					columnCount = 0;
				}

				if (output == null) {
					output = openFile(0, folder, script);
				}

				if (columnCount > 10) {
					output.println("</tr><tr>");
					columnCount = 0;
				}
				showCell(output, UTF16.valueOf(cp), "<td ", "", false);
				++columnCount;
			}

			closeFile(output);
			closeIndexFile(indexFile, "", CASE);
		}

    static public void addMapChar(Map m, Set stoplist, String key, String ch) {
    	if (stoplist.contains(key)) return;
    	for (int i = 0; i < key.length(); ++i) {
    		char c = key.charAt(i);
    		if ('0' <= c && c <= '9') return;
    	}
    	Set result = (Set)m.get(key);
    	if (result == null) {
    		result = new TreeSet();
    		m.put(key, result);
    	}
    	result.add(ch);
    }

    static public void indexChart() throws IOException {
    	HACK_KANA = false;

        Map map = new TreeMap();
        Set stoplist = new TreeSet();

        String[] stops = {"LETTER", "CHARACTER", "AND", "CAPITAL", "SMALL", "COMPATIBILITY", "WITH"};
        stoplist.addAll(Arrays.asList(stops));
        System.out.println("Stop-list: " + stoplist);

        for (int i = 0; i < LIMIT_SCRIPT; ++i) {
        	stoplist.add(Default.ucd().getScriptID_fromIndex((byte)i));
        }
        System.out.println("Stop-list: " + stoplist);

        for (int i = 0; i <= 0x10FFFF; ++i) {
			if (!Default.ucd().isRepresented(i)) continue;
			if (!Default.ucd().isAssigned(i)) continue;
        	if (0xAC00 <= i && i <= 0xD7A3) continue;
        	if (Default.ucd().hasComputableName(i)) continue;

        	String s = Default.ucd().getName(i);
        	if (s == null) continue;

        	if (s.startsWith("<")) {
        		System.out.println("Weird character at " + Default.ucd().getCodeAndName(i));
        	}
        	String ch = UTF16.valueOf(i);
        	int last = -1;
        	int j;
        	for (j = 0; j < s.length(); ++j) {
        		char c = s.charAt(j);
        		if ('A' <= c && c <= 'Z' || '0' <= c && c <= '9') {
        			if (last == -1) last = j;
        		} else {
        			if (last != -1) {
        				String word = s.substring(last, j);
        				addMapChar(map, stoplist, word, ch);
        				last = -1;
        			}
        		}
        	}
        	if (last != -1) {
        				String word = s.substring(last, j);
        				addMapChar(map, stoplist, word, ch);
        	}
        }

        PrintWriter output = null;

        Iterator it = map.keySet().iterator();

        int oldScript = -127;

        int counter = 0;
        String[] replacement = new String[] {"%%%", "Name Charts"};
        String folder = "charts\\name\\";

        Utility.copyTextFile("index.html", Utility.UTF8, folder + "index.html", replacement);
        Utility.copyTextFile("charts.css", Utility.LATIN1, folder + "charts.css");
        Utility.copyTextFile("name_help.html", Utility.UTF8, folder + "help.html");

        indexFile = Utility.openPrintWriter(folder + "index_list.html", Utility.UTF8_WINDOWS);
        Utility.appendFile("index_header.html", Utility.UTF8, indexFile, replacement);

        int columnCount = 0;
        char lastInitial = 0;

        while (it.hasNext()) {
            Utility.dot(counter);

            String key = (String) it.next();

            Set chars = (Set) map.get(key);

            char initial = key.charAt(0);

            if (initial != lastInitial) {
                closeFile(output);
                output = null;
                lastInitial = initial;
            }

            if (output == null) {
                output = openFile2(0, folder, String.valueOf(initial));
            }

            output.println("<tr><td class='h'>" + key + "</td>");
            columnCount = 1;

            Iterator sublist = chars.iterator();
            while (sublist.hasNext()) {

            	String ch = (String) sublist.next();
            	if (columnCount > 10) {
            		output.println("</tr><tr><td></td>");
            		columnCount = 1;
            	}
            	showCell(output, ch, "<td ", "", true);
            	++columnCount;
            	continue;
            }

            output.println("</tr>");

        }

        closeFile(output);
        closeIndexFile(indexFile, "", CASE);
    }

    static void showCell(PrintWriter output, String s, 
      String prefix, String extra, boolean skipName) {
        if (s.equals("\u0300")) {
            System.out.println();
        }
        String name = Default.ucd().getName(s);
        String comp = Default.nfc().normalize(s);
        int cat = Default.ucd().getCategory(UTF16.charAt(comp,0));
        if (cat == Mn || cat == Mc || cat == Me) {
            comp = '\u25CC' + comp;
            if (s.equals("\u0300")) {
                System.out.println(Default.ucd().getCodeAndName(comp));
            } 
        } 

        String outline = prefix
            + (skipName ? "" : " title='" + Utility.quoteXML(name, true) + "'")
            + extra + ">"
            + Utility.quoteXML(comp, true)
            + "<br><tt>"
            + Utility.hex(s)
            //+ "<br>" + script
            + "</tt></td>";

        output.println(outline);
    }

    static byte getBestScript(String s) {
    	int cp;
    	byte result = COMMON_SCRIPT;
    	for (int i = 0; i < s.length(); i += UTF16.getCharCount(cp)) {
    		cp = UTF16.charAt(s, i);
    		result = Default.ucd().getScript(cp);
    		if (result != COMMON_SCRIPT && result != INHERITED_SCRIPT) return result;
    	}
    	return COMMON_SCRIPT;
    }

    static int getFirstPrimary(String sortKey) {
        int result = sortKey.charAt(0);
		if (UCA.isImplicitLeadPrimary(result)) {
			return (result << 16) | sortKey.charAt(1);
		}
		return (result << 16);
    }

    static final String[] CLASSNAME = {
        "<td class='q'",
        "<td class='q'",
        "<td class='q'",
        "<td class='t'",
        "<td class='s'",
        "<td class='p'"};

    static final String[] XCLASSNAME = {
        "<td class='eq'",
        "<td class='eq'",
        "<td class='eq'",
        "<td class='et'",
        "<td class='es'",
        "<td class='ep'"};


    static PrintWriter indexFile;

    static PrintWriter openFile(int count, String directory, int script) throws IOException {
        String scriptName = getChunkName(script, LONG);
        String shortScriptName = getChunkName(script, SHORT);
        String hover = scriptName.equals(shortScriptName) ? "" : "' title='" + shortScriptName;

        String fileName = "chart_" + scriptName + (count > 1 ? count + "" : "") + ".html";
        PrintWriter output = Utility.openPrintWriter(directory + fileName, Utility.UTF8_WINDOWS);
        Utility.fixDot();
        System.out.println("Writing: " + scriptName);
        indexFile.println(" <a href = '" + fileName + hover + "'>" + scriptName + "</a>");
        String title = "UCA: " + scriptName;
        output.println("<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'>");
        output.println("<title>" + title + "</title>");
        output.println("<link rel='stylesheet' href='charts.css' type='text/css'>");
        output.println("</head><body><h2>" + scriptName + "</h2>");
        output.println("<table>");
        return output;
    }

    static PrintWriter openFile2(int count, String directory, String name) throws IOException {
        String fileName = "chart_" + name + (count > 1 ? count + "" : "") + ".html";
        PrintWriter output = Utility.openPrintWriter(directory + fileName, Utility.UTF8_WINDOWS);
        Utility.fixDot();
        System.out.println("Writing: " + name);
        indexFile.println(" <a href = '" + fileName + "'>" + name + "</a>");
        String title = name;
        output.println("<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'>");
        output.println("<title>" + title + "</title>");
        output.println("<link rel='stylesheet' href='charts.css' type='text/css'>");
        output.println("</head><body>");
        output.println("<table>");
        return output;
    }

    static final int
    	NULL_ORDER = -3,
    	IGNORABLE_ORDER = -2,
    	VARIABLE_ORDER = -1,
    	// scripts in here
    	CJK = 120,
    	CJK_AB = 121,
    	UNSUPPORTED = 122,
    	CAT_OFFSET = 128,
    	// categories in here
    	NO_CASE_MAPPING = 200;

    static String getChunkName(int script, byte length) {
    	switch(script) {
    		case NO_CASE_MAPPING: return "NoCaseMapping";
        	case NULL_ORDER: return "Null";
        	case IGNORABLE_ORDER: return "Ignorable";
        	case VARIABLE_ORDER: return "Variable";
        	case CJK: return "CJK";
        	case CJK_AB: return "CJK-Extensions";
        	case UNSUPPORTED: return "Unsupported";
        	default:
    		if (script >= CAT_OFFSET) return Default.ucd().getCategoryID_fromIndex((byte)(script - CAT_OFFSET), length);
        	else if (script == HIRAGANA_SCRIPT && HACK_KANA) return length == SHORT ? "Kata-Hira" : "Katakana-Hiragana";
        	else return Default.ucd().getCase(Default.ucd().getScriptID_fromIndex((byte)script, length), FULL, TITLE);
    	}
    }

    static void closeFile(PrintWriter output) {
        if (output == null) return;
        output.println("</table></body></html>");
        output.close();
    }


	static final byte COLLATION = 0, NORMALIZATION = 1, CASE = 2;

    static void closeIndexFile(PrintWriter indexFile, String extra, byte choice) {
        SimpleDateFormat df = new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
        df.setTimeZone(TimeZone.getTimeZone("GMT"));

        indexFile.println("</p><hr width='50%'><p>");
        boolean gotOne = false;
        if (choice != COLLATION) {
        	indexFile.println("<a href='..\\uca\\index.html' target='_top'>Collation&nbsp;Charts</a>");
        	gotOne = true;
        }
        if (choice != NORMALIZATION) {
        	if (gotOne) indexFile.println("<br>");
        	indexFile.println("<a href='..\\normalization\\index.html' target='_top'>Normalization&nbsp;Charts</a>");
        	gotOne = true;
        }
        if (choice != CASE) {
        	if (gotOne) indexFile.println("<br>");
        	indexFile.println("<a href='..\\case\\index.html' target='_top'>Case&nbsp;Charts</a>");
        	gotOne = true;
        }
        indexFile.println("</p><hr width='50%'><p style='font-size: 70%'>");
        indexFile.println("UCD: " + Default.ucd().getVersion() + extra);
        indexFile.println("<br>" + Default.getDate() + " <a href='http://www.macchiato.com/' target='_top'>MED</a>");
        indexFile.println("</p></body></html>");
        indexFile.close();
    }

    static boolean containsCase(String s) {
    	int cp;
    	for (int i = 0; i < s.length(); i += UTF16.getCharCount(cp)) {
    		cp = UTF16.charAt(s, i);
			// contains Lu, Lo, Lt, or Lowercase or Uppercase
			byte cat = Default.ucd().getCategory(cp);
			if (cat == Lu || cat == Ll || cat == Lt) return true;
			if (Default.ucd().getBinaryProperty(cp, Other_Lowercase)) return true;
			if (Default.ucd().getBinaryProperty(cp, Other_Uppercase)) return true;
		}
		return false;
	}

    static final Transliterator addCircle = Transliterator.createFromRules(
        "any-addCircle", "([[:Mn:][:Me:]]) > \u25CC $1", Transliterator.FORWARD);

    public static void writeCompositionChart() throws IOException {
        UCA uca = new UCA(null,"");

        Set letters = new TreeSet();
        Set marks = new TreeSet(uca);
        Set totalMarks = new TreeSet(uca);
        Map decomposes = new HashMap();
        Set notPrinted = new TreeSet(new UTF16.StringComparator());
        Set printed = new HashSet();

        // UnicodeSet latin = new UnicodeSet("[:latin:]");

        PrintWriter out = Utility.openPrintWriter("composition_chart.html", Utility.UTF8_WINDOWS);
        try {
            out.println("<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'>");
            out.println("<style>");

            out.println("body { font-family: Arial Unicode MS }");
            out.println("td { text-align: Center ; vertical-align: top; width: 1%; background-color: #EEEEEE }");
            out.println("tt { font-size: 50% }");
            out.println("table { width='1%' }");
            out.println(".w { background-color: #FFFFFF }");
            out.println(".h { background-color: #EEEEFF }");
            out.println(".r { background-color: #FF0000 }");
            out.println("</style>");
            out.println("</head><body bgcolor='#FFFFFF'>");
            out.println("<h1>Composites</h1>");

            UnicodeSetIterator it = new UnicodeSetIterator();

            for (byte script = 0; script < UCD_Types.LIMIT_SCRIPT; ++script) {
                
                String scriptName = "";
                try {
                    scriptName = Default.ucd().getScriptID_fromIndex(script);
                    Utility.fixDot();
                    System.out.println(scriptName);
                } catch (IllegalArgumentException e) {
                    System.out.println("Failed to create transliterator for: " + scriptName + "(" + script + ")");
                    continue;
                }


                letters.clear();
                letters.add(""); // header row
                marks.clear();
                notPrinted.clear();
                printed.clear();

                for (int cp = 0; cp < 0x10FFFF; ++cp) {
                    byte type = Default.ucd().getCategory(cp);
                    if (type == Default.ucd().UNASSIGNED || type == Default.ucd().PRIVATE_USE) continue; // skip chaff
                    Utility.dot(cp);
                    
                    byte newScript = Default.ucd().getScript(cp);
                    if (newScript != script) continue;

                    String source = UTF16.valueOf(cp);
                    String decomp = Default.nfd().normalize(source);
                    if (decomp.equals(source)) continue;

                    // pick up all decompositions
                    int count = UTF16.getCharCount(UTF16.charAt(decomp, 0));

                    if (count == decomp.length()) {
                        notPrinted.add(source);
                        continue; // skip unless marks
                    }

                    if (UCD.isHangulSyllable(cp)) count = 2;
                    String first = decomp.substring(0, count);
                    String second = decomp.substring(count);
                    //if (!markSet.containsAll(second)) continue; // skip unless marks

                    letters.add(first);
                    marks.add(second);
                    Utility.addToSet(decomposes, decomp, source);
                    notPrinted.add(source);
                    if (source.equals("\u212b")) System.out.println("A-RING!");
                }

                if (marks.size() != 0) {

                    totalMarks.addAll(marks);


                    out.println("<table border='1' cellspacing='0'>");
                    out.println("<caption>" + scriptName + "<br>(" + letters.size() + " ? " + marks.size() + ")</caption>");

                    Iterator it2 = letters.iterator();
                    while (it2.hasNext()) {
                        String let = (String)it2.next();
                        out.println("<tr>" + showCell(Default.nfc().normalize(let), "class='h'"));
                        Iterator it3 = marks.iterator();
                        while (it3.hasNext()) {
                            String mark = (String)it3.next();
                            String merge = let + mark;
                            if (let.length() != 0 && decomposes.get(merge) == null) {
                                out.println("<td>&nbsp;</td>");
                                continue;
                            }
                            String comp;
                            try {
                                comp = Default.nfc().normalize(merge);
                            } catch (Exception e) {
                                System.out.println("Failed when trying to compose <" + Utility.hex(e) + ">");
                                continue;
                            }
                            // skip unless single char or header
                            /*if (let.length() != 0
                                && (UTF16.countCodePoint(comp) != 1 || comp.equals(merge))) {
                                    out.println("<td class='x'>&nbsp;</td>");
                                    continue;
                            }
                            */
                            Set decomps = (Set) decomposes.get(merge);
                            if (let.length() == 0) {
                                printed.add(comp);
                                out.println(showCell(comp, "class='h'"));
                            } else if (decomps.contains(comp)) {
                                printed.add(comp);
                                out.println(showCell(comp, "class='w'"));
                            } else {
                                comp = (String) new ArrayList(decomps).get(0);
                                printed.add(comp);
                                out.println(showCell(comp, "class='r'"));
                            }
                        }
                        out.println("</tr>");
                    }
                    out.println("</table><br>");

                    //out.println("<table><tr><th>Other Letters</th><th>Other Marks</th></tr><tr><td>");
                    //tabulate(out, atomics.iterator(),16);
                    //out.println("</td><td>");
                    //out.println("</td></tr></table>");

                }
                notPrinted.removeAll(printed);
                if (notPrinted.size() != 0) {
                    tabulate(out, scriptName + " Excluded", notPrinted.iterator(), 24, "class='r'");
                    out.println("<br>");
                }
            }

            Set otherMarks = new TreeSet(uca);
            UnicodeSet markSet = new UnicodeSet("[[:Me:][:Mn:]]");
            it.reset(markSet);
            while (it.next()) {
                int cp = it.codepoint;
                String source = UTF16.valueOf(cp);
                if (totalMarks.contains(source)) continue; // skip all that we have already
                otherMarks.add(source);
            }
            tabulate(out, "Marks that never combine", otherMarks.iterator(), 24, "class='b'");

            out.println("</body></html>");

        } finally {
            out.close();
        }
    }

    public static void tabulate(PrintWriter out, String caption, Iterator it2, int limit, String classType) {
        int count = 0;
        out.println("<table border='1' cellspacing='0'><tr>");
        if (caption != null && caption.length() != 0) {
            out.println("<caption>" + caption + "</caption>");
        }
        while (it2.hasNext()) {
            if (++count > limit) {
                out.println("</tr><tr>");
                count = 1;
            }

            out.println(showCell((String)it2.next(), classType));
        }
        out.println("</tr></table>");
    }

    public static String showCell(String comp, String classType) {
        if (comp == null) {
            return "<td "
                + classType + (classType.length() != 0 ? " " : "")
                + ">&nbsp;</td>";
        }
        return "<td "
            + classType + (classType.length() != 0 ? " " : "")
            + "title='" + Utility.hex(comp) + " " + Default.ucd().getName(comp) + "'>" + addCircle.transliterate(comp)
            + "<br><tt>" + Utility.hex(comp) + "</tt></td>";
    }

    

    public static void writeAllocation() throws IOException {
        String[] names = new String[300]; // HACK, 300 is plenty for now. Fix if it ever gets larger
        int[] starts = new int[names.length];
        int[] ends = new int[names.length];
        
        UCD.BlockData blockData = new UCD.BlockData();
        
        int counter = 0;
        int blockId = 0;
        while (Default.ucd().getBlockData(blockId++, blockData)) {
            names[counter] = blockData.name;
            starts[counter] = blockData.start;
            ends[counter] = blockData.end;
            //System.out.println(names[counter] + ", " + values[counter]);
            ++counter;
                
            // HACK
            if (blockData.name.equals("Tags")) {
                names[counter] = "<i>reserved default ignorable</i>";
                starts[counter] = 0xE0080;
                ends[counter] = 0xE0FFF;
                ++counter;
            }                   
        }
        
        /*
            Graphic
            Format
            Control
            Private Use
            Surrogate
            Noncharacter
            Reserved (default ignorable)
            Reserved (other)
        */
            
        PrintWriter out = Utility.openPrintWriter("allocation.html", Utility.UTF8_WINDOWS);
        try {
            out.println("<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'>");
            out.println("<title>Unicode Allocation</title></head>");
            out.println("<body bgcolor='#FFFFFF'><h1 align='center'><a href='#Notes'>Unicode Allocation</a></h1>");
  

            for (int textOnly = 0; textOnly < 2; ++textOnly) {
                out.println("<table border='1' cellspacing='0'>"); // width='100%' 
                if (textOnly == 0) {
                    out.println("<caption><b>Graphic Version</b></caption>");
                    out.println("<tr><th>Start</th><th align='left'>Block Name</th><th align='left'>Size</th></tr>");
                } else {
                    out.println("<caption><b>Textual Version (decimal)</b></caption>");
                    out.println("<tr><th>Block Name</th><th>Start</th><th>Total</th><th>Assigned</th></tr>");
                }
                int lastEnd = -1;
                for (int i = 0; i < counter; ++i) {
                    if (starts[i] != lastEnd + 1) {
                        drawAllocation(out, lastEnd + 1, "<i>reserved</i>", starts[i] - lastEnd + 1, 0, "#000000", "#000000", textOnly);
                    }
                    int total = ends[i] - starts[i] + 1;
                    int alloc = 0;
                    for (int j = starts[i]; j <= ends[i]; ++j) {
                        if (Default.ucd().isAllocated(j)) ++alloc;
                    }
                    //System.out.println(names[i] + "\t" + alloc + "\t" + total);
                    String color = names[i].indexOf("Surrogates") >= 0 ? "#FF0000"
                        : names[i].indexOf("Private") >= 0 ? "#0000FF"
                        : "#00FF00";
                    String colorReserved = names[i].indexOf("reserved default ignorable") >= 0 ? "#CCCCCC"
                        : "#000000";
                    drawAllocation(out, starts[i], names[i], total, alloc, color, colorReserved, textOnly);
                    lastEnd = ends[i];
                }
                out.println("</table><p>&nbsp;</p>");
            }
            out.println("<h2>Key</h2><p><a name='Notes'></a>This chart lists all the Unicode blocks and their starting code points. "
                + "The area of each bar is proportional to the total number of code points in each block. "
                + "The colors have the following significance:<br>"
                + "<table border='1' cellspacing='0' cellpadding='4'>"
                + "<tr><td>Green</td><td>Graphic, Control, Format, Noncharacter* code points</td></tr>"
                + "<tr><td>Red</td><td>Surrogate code points</td></tr>"
                + "<tr><td>Blue</td><td>Private Use code points</td></tr>"
                + "<tr><td>Gray</td><td>Reserved (default ignorable) code points</td></tr>"
                + "<tr><td>Black</td><td>Reserved (other) code points</td></tr>"
                + "</table><br>"
                + "* Control, Format, and Noncharacter are not distinguished from Graphic characters by color, since they are mixed into other blocks. "
                + "Tooltips on the bars show the total number of code points and the number assigned. "
                + "(Remember that assigned <i>code points</i> are not necessarily assigned <i>characters</i>.)"
                + "</p>");
            out.println("</body></html>");
        } finally {
            out.close();
        }
    }
    
    static double longestBar = 1000;
    static int longestBlock = 722402;
    static NumberFormat nf = NumberFormat.getNumberInstance(Locale.US);
    static {nf.setMaximumFractionDigits(0);}
    
    static void drawAllocation(PrintWriter out, int start, String title, int total, int alloc, String color, String colorReserved, int textOnly) {
        if (textOnly == 0) {
            int unalloc = total - alloc;
            
            double totalWidth = longestBar*(Math.sqrt(total) / Math.sqrt(longestBlock));
            double allocWidth = alloc * totalWidth / total;
            double unallocWidth = totalWidth - allocWidth;
     
            out.println("<tr><td  align='right'><code>" + Utility.hex(start)
                + "</code></td><td>" + title
                + "</td><td title='total: " + nf.format(total) + ", assigned: " + nf.format(alloc)
                + "'><table border='0' cellspacing='0' cellpadding='0'><tr>");
            
            if (alloc != 0) out.println("<td style='font-size:1;width:" + allocWidth + ";height:" + totalWidth
                + "' bgcolor='" + color + "'>&nbsp;</td>");
            if (unalloc != 0) out.println("<td style='font-size:1;width:" + unallocWidth + ";height:" + totalWidth
                + "' bgcolor='" + colorReserved + "'>&nbsp;</td>");
            out.println("</tr></table></td></tr>");
        } else {
            out.println("<tr><td>" + title + "</td><td align='right'>" + start + "</td><td align='right'>" + total + "</td><td align='right'>" + alloc + "</td></tr>");
        }
    }
    
}



    /*
    static final IntStack p1 = new IntStack(30);
    static final IntStack s1 = new IntStack(30);
    static final IntStack t1 = new IntStack(30);
    static final IntStack p2 = new IntStack(30);
    static final IntStack s2 = new IntStack(30);
    static final IntStack t2 = new IntStack(30);

    static int getStrengthDifference(CEList ceList, CEList lastCEList) {
        extractNonzeros(ceList, p1, s1, t1);
        extractNonzeros(lastCEList, p2, s2, t2);
        int temp = p1.compareTo(p2);
        if (temp != 0) return 3;
        temp = s1.compareTo(s2);
        if (temp != 0) return 2;
        temp = t1.compareTo(t2);
        if (temp != 0) return 1;
        return 0;
    }

    static void extractNonzeros(CEList ceList, IntStack primaries, IntStack secondaries, IntStack tertiaries) {
        primaries.clear();
        secondaries.clear();
        tertiaries.clear();

        for (int i = 0; i < ceList.length(); ++i) {
            int ce = ceList.at(i);
            int temp = UCA.getPrimary(ce);
            if (temp != 0) primaries.push(temp);
            temp = UCA.getSecondary(ce);
            if (temp != 0) secondaries.push(temp);
            temp = UCA.getTertiary(ce);
            if (temp != 0) tertiaries.push(temp);
        }
    }
    */