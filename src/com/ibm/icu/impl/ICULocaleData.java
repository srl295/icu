/*
 * (C) Copyright IBM Corp. 2002 - All Rights Reserved
 */

package com.ibm.icu.impl;

import java.io.InputStream;
import java.io.IOException;
import java.net.JarURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;
import java.util.jar.JarFile;

/**
 * Provides information about and access to resource bundles in the
 * com.ibm.text.resources package.  Unlike the java version, this does
 * not include resources from any other location.  In particular, it
 * does not look in the boot or system class path.
 */
public class ICULocaleData {
	private static Locale[] localeList;
	private static final String PACKAGE1 = "com.ibm.icu.impl.data";
	private static final String[] packageNames = { PACKAGE1 };
	private static boolean debug = ICUDebug.enabled("localedata");

    /**
     * Returns a list of the installed locales.
     * @param key A resource tag.  Currently, this parameter is ignored.  The obvious
     * intent, however,  is for getAvailableLocales() to return a list of only those
     * locales that contain a resource with the specified resource tag.
     *
     * <p>Before we implement this function this way, however, some thought should be
     * given to whether this is really the right thing to do.  Because of the lookup
     * algorithm, a NumberFormat, for example, is "installed" for all locales.  But if
     * we're trying to put up a list of NumberFormats to choose from, we may want to see
     * only a list of those locales that uniquely define a NumberFormat rather than
     * inheriting one from another locale.  Thus, if fr and fr_CA uniquely define
     * NumberFormat data, but fr_BE doesn't, the user wouldn't see "French (Belgium)" in
     * the list and would go for "French (default)" instead.  Of course, this means
     * "English (United States)" would not be in the list, since it is the default locale.
     * This might be okay, but might be confusing to some users.
     *
     * <p>In addition, the other functions that call getAvailableLocales() don't currently
     * all pass the right thing for "key," meaning that all of these functions should be
     * looked at before anything is done to this function.
     *
     * <p>We recommend that someone take some careful consideration of these issues before
     * modifying this function to pay attention to the "key" parameter.  --rtg 1/26/98
     */
    public static Locale[] getAvailableLocales(String key) {
		// ignore key, just return all locales
		return getAvailableLocales();
    }
    
	/**
	 * Return an array of all the locales for which we have resource information.
	 */
	public static Locale[] getAvailableLocales() {
        // creating the locale list is expensive, so be careful to do it
        // only once
        if (localeList == null) {
            synchronized(ICULocaleData.class) {
                if (localeList == null) {
                    localeList = createLocaleList();
                }
            }
        }

		return (Locale[])localeList.clone();
	}

    /**
     * Gets a LocaleElements resource bundle.
     */
    public static ResourceBundle getLocaleElements(Locale locale) {
        return getResourceBundle("LocaleElements", locale);
    }

	/**
	 * Still need permissions to use our own class loader, is there no way
	 * to load class resources from new locations that aren't already on the
	 * class path?
	 */
	private static ResourceBundle instantiateBundle(String name, Locale l) {
		return ResourceBundle.getBundle(name, l);
	}

	/**
	 * Get a resource bundle from the lookup chain.
	 */
    public static ResourceBundle getResourceBundle(String bundleName, Locale locale) {
		if (locale == null) {
			locale = Locale.getDefault();
		}
		ResourceBundle rb = null;
		for (int i = 0; i < packageNames.length && rb == null; ++i) {
			try {
				String path = packageNames[i] + "." + bundleName;
				if (debug) System.out.println("calling getBundle: " + path + "_" + locale);
				rb = instantiateBundle(path, locale);
			} 
			catch (MissingResourceException e) {
				if (debug) System.out.println(bundleName + "_" + locale + " not found in " + packageNames[i]);
			}
		}

		return rb;
	}

    // ========== privates ==========


    private static Locale[] createLocaleList() {
		try {
			ResourceBundle index = getLocaleElements(LocaleUtility.getLocaleFromName("index"));
			String[] localeNames = index.getStringArray("InstalledLocales");
			Locale[] locales = new Locale[localeNames.length];
			for (int i = 0; i < localeNames.length; ++i) {
				locales[i] = LocaleUtility.getLocaleFromName(localeNames[i]);
			}
			return locales;
		}
		catch (MissingResourceException e) {
		}
		
		return new Locale[0];
    }
}
