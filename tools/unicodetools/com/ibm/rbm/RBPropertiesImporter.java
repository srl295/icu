/*
 *****************************************************************************
 * Copyright (C) 2000-2004, International Business Machines Corporation and  *
 * others. All Rights Reserved.                                              *
 *****************************************************************************
 */
package com.ibm.rbm;

import java.io.*;
import javax.swing.filechooser.*;

import com.ibm.rbm.gui.RBManagerGUI;

import java.util.*;

/**
 * This is the super class for all importer plug-in classes. As of yet, there
 * is little contained in this class.
 * 
 * @author Jared Jackson
 * @see com.ibm.rbm.RBManager
 */
public class RBPropertiesImporter extends RBImporter {
	
    boolean isRBMFile = true;
    
    /**
     * Constructs the importer given the parent data classes and a Dialog title.
     */
	
    public RBPropertiesImporter(String title, RBManager rbm, RBManagerGUI gui) {
        super(title, rbm, gui);
    }
	
    protected void setupFileChooser() {
        chooser.setFileFilter(new javax.swing.filechooser.FileFilter(){
            public boolean accept(File f) {
                if (f.isDirectory()) return true;
                if (f.getName().toLowerCase().endsWith(".properties") && f.getName().indexOf("_") < 0) return true;
                return false;
            }
            
            public String getDescription() {
                return Resources.getTranslation("import_properties_file_description");
            }
        });
    }
	
    protected void beginImport() throws IOException {
        super.beginImport();
        File baseFile = getChosenFile();
        FileReader fr = new FileReader(baseFile);
        BufferedReader br = new BufferedReader(fr);
		
        // Test if this is an RBManager generated file or not
        int count = 0;
        String line = null;
        isRBMFile = true;
        while ((line = br.readLine()) != null) {
            if (!line.trim().equals("")) count++;
            if (count == 1 && !line.startsWith("# @file")) {
                // Not generated by RBManager
                isRBMFile = false;
            }
        } // end while
        if (isRBMFile) {
            // Treat the file as generated by RBManager
            // Parse the resource bundle through RBManager
            RBManager import_rbm = new RBManager(baseFile);
            // Merge the two resource bundles
            Vector bundles = import_rbm.getBundles();
            Vector encodings = new Vector();
            for (int i=0; i < bundles.size(); i++) {
                Bundle b = (Bundle)bundles.elementAt(i);
                encodings.addElement(b.encoding);
            }
            resolveEncodings(encodings);
            for (int i=0; i < bundles.size(); i++) {
                Bundle b = (Bundle)bundles.elementAt(i);
                Enumeration enum = b.allItems.keys();
                while (enum.hasMoreElements()) {
                    String key = (String)enum.nextElement();
                    BundleItem item = (BundleItem)b.allItems.get(key);
                    importResource(item, b.encoding, (item.getParentGroup() == null ? getDefaultGroup(): item.getParentGroup().getName())); 
                }
            }
        } else {
            // Just treat it as a regular properties file
            // Check if there are any missing target locale files
            String baseName = baseFile.getName().substring(0,baseFile.getName().length()-11); // |'.properties'| == 11
            File baseDir = new File(baseFile.getParent());
            String allChildren[] = baseDir.list();
            Vector children_v = new Vector();
            for (int i=0; i < allChildren.length; i++) {
                if (allChildren[i].startsWith(baseName) && allChildren[i].toLowerCase().endsWith(".properties")) {
                    if (allChildren[i].length() == (baseName + ".properties").length()) children_v.addElement("");
                    else children_v.addElement(allChildren[i].substring(baseName.length()+1, allChildren[i].indexOf(".properties")));
                }
            }
            showProgressBar(children_v.size());
            resolveEncodings(children_v);
            // Run through each source locale file importing as necessary
            for (int i=0; i < children_v.size(); i++) {
                Properties p = new Properties();
                FileInputStream fis = new FileInputStream(new File(baseDir, baseName + 
                                        (children_v.elementAt(i).toString().equals("") ? "" : "_" + children_v.elementAt(i).toString()) +
                                        ".properties"));
                p.load(fis);
                Enumeration enum = p.keys();
                while (enum.hasMoreElements()) {
                    String key = (String)enum.nextElement();
                    BundleItem item = new BundleItem(null, key, p.getProperty(key));
                    item.setTranslated(this.getDefaultTranslated());
                    importResource(item, children_v.elementAt(i).toString(), getDefaultGroup());
                }
                incrementProgressBar();
            }
            hideProgressBar();
        }
    }
}