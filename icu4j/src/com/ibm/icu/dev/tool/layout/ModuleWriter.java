/*
 *******************************************************************************
 * Copyright (C) 1998-2008, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 *
 * Created on Dec 3, 2003
 *
 *******************************************************************************
 */

package com.ibm.icu.dev.tool.layout;

import java.io.PrintStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Date;
import java.io.*;
import java.util.Scanner;
import java.util.ArrayList;

import com.ibm.icu.text.MessageFormat;

public class ModuleWriter
{
    private static final String BUILDER_FILE_PATH="src/com/ibm/icu/dev/tool/layout/"; 
        
    public ModuleWriter()
    {
        wroteDefine = false;
        output = null;
    }

    public void openFile(String outputFileName) {
        try
        {
            output = new PrintStream(
                new FileOutputStream(BUILDER_FILE_PATH+outputFileName));
        } catch (IOException e) {
            System.out.println("? Could not open " + outputFileName + " for writing.");
            return;
        }
    
        wroteDefine = false;
        System.out.println("Writing module " + outputFileName + "...");
    }

    public void writeHeader(String define, String[] includeFiles)
    {
        writeHeader(define, includeFiles, null);
    }
    
    public void writeHeader(String define, String[] includeFiles, String brief)
    {
        MessageFormat format = new MessageFormat(moduleHeader);
        Object args[] = {new Date(System.currentTimeMillis())};

        output.print(format.format(args));
        
        if (define != null) {
            wroteDefine = true;
            output.print("#ifndef ");
            output.println(define);
            
            output.print("#define ");
            output.println(define);
            
            output.println();
        }
        
        if (includeFiles != null) {
            for (int i = 0; i < includeFiles.length; i += 1) {
                output.print("#include \"");
                output.print(includeFiles[i]);
                output.println("\"");
            }
            
            output.println();
        }
        
        if (brief != null) {
            output.print(brief);
        }
        
        output.println(moduleBegin);
    }

    public void writeTrailer() {
        output.print(moduleTrailer);
        
        if (wroteDefine) {
            output.println("#endif");
            
        }
    }

    public void closeFile() {
        System.out.println("Done.");
        output.close();
    }

    protected boolean wroteDefine;
    
    protected PrintStream output;
    
    protected BufferedReader reader;
    protected Scanner sc;
    protected PrintStream updateFile;
    protected int previousTotalScripts;
    protected int previousTotalLanguages;
    protected ArrayList scriptVersionNumber = new ArrayList();
    protected ArrayList languageVersionNumber = new ArrayList();
    
    public void openScriptAndLanguages(String name){
        try
        {
            updateFile = new PrintStream(new FileOutputStream(BUILDER_FILE_PATH+name));
        } catch (IOException e) {
            System.out.println("? Could not open " + name + " for writing.");
            return;
        }
    }
    
    public void readFile(String file, String what){
        try
        {
           reader = new BufferedReader(new FileReader(BUILDER_FILE_PATH+file));
           String inputText = "";
           String versionToAdd = "";
           while((inputText=reader.readLine())!=null){
               if(what.equals("script") && inputText.contains("Script=")){
                   previousTotalScripts = Integer.parseInt(inputText.substring(inputText.indexOf("=")+1));
               }else if(what.equals("languages") && inputText.contains("Language=")){
                   previousTotalLanguages = Integer.parseInt(inputText.substring(inputText.indexOf("=")+1));
               }else if(what.equals("script") && inputText.contains("Scripts={")){
                   while(!(versionToAdd=reader.readLine()).contains("}")){
                       scriptVersionNumber.add(versionToAdd);
                   }
               }else if(what.equals("languages") && inputText.contains("Languages={")){
                   while(!(versionToAdd=reader.readLine()).contains("}")){
                       languageVersionNumber.add(versionToAdd);
                   }
               }
           }
           reader.close();
           
        } catch (IOException e) {
            System.out.println("? Could not open " + file + " for reading.");
            return;
        }
    }
    
    
    
    protected static final String moduleHeader =
        "/*\n" +
        " *\n" +
        " * (C) Copyright IBM Corp. 1998-{0,date,yyyy}. All Rights Reserved.\n" +
        " *\n" +
        " * WARNING: THIS FILE IS MACHINE GENERATED. DO NOT HAND EDIT IT UNLESS\n" +
        " * YOU REALLY KNOW WHAT YOU''RE DOING.\n" +
        " *\n" +
        " * Generated on: {0,date,MM/dd/yyyy hh:mm:ss a z}\n" +
        " */\n" +
        "\n";

    protected static final String moduleBegin = "U_NAMESPACE_BEGIN\n";

    protected static final String moduleTrailer = "U_NAMESPACE_END\n";

}
