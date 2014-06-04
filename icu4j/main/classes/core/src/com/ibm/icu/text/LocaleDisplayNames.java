/*
 *******************************************************************************
 * Copyright (C) 2009-2014, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 */
package com.ibm.icu.text;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Locale;

import com.ibm.icu.impl.ICUConfig;
import com.ibm.icu.lang.UScript;
import com.ibm.icu.text.DisplayContext.Type;
import com.ibm.icu.util.ULocale;

/**
 * Returns display names of ULocales and components of ULocales. For
 * more information on language, script, region, variant, key, and
 * values, see {@link com.ibm.icu.util.ULocale}.
 * @stable ICU 4.4
 */
public abstract class LocaleDisplayNames {
    /**
     * Enum used in {@link #getInstance(ULocale, DialectHandling)}.
     * @stable ICU 4.4
     */
    public enum DialectHandling {
        /**
         * Use standard names when generating a locale name,
         * e.g. en_GB displays as 'English (United Kingdom)'.
         * @stable ICU 4.4
         */
        STANDARD_NAMES,
        /**
         * Use dialect names when generating a locale name,
         * e.g. en_GB displays as 'British English'.
         * @stable ICU 4.4
         */
        DIALECT_NAMES
    }

    // factory methods
    /**
     * Convenience overload of {@link #getInstance(ULocale, DialectHandling)} that specifies
     * STANDARD dialect handling.
     * @param locale the display locale
     * @return a LocaleDisplayNames instance
     * @stable ICU 4.4
     */
    public static LocaleDisplayNames getInstance(ULocale locale) {
        return getInstance(locale, DialectHandling.STANDARD_NAMES);
    };

    /**
     * Returns an instance of LocaleDisplayNames that returns names formatted for the provided locale,
     * using the provided dialectHandling.
     * @param locale the display locale
     * @param dialectHandling how to select names for locales
     * @return a LocaleDisplayNames instance
     * @stable ICU 4.4
     */
    public static LocaleDisplayNames getInstance(ULocale locale, DialectHandling dialectHandling) {
        LocaleDisplayNames result = null;
        if (FACTORY_DIALECTHANDLING != null) {
            try {
                result = (LocaleDisplayNames) FACTORY_DIALECTHANDLING.invoke(null,
                        locale, dialectHandling);
            } catch (InvocationTargetException e) {
                // fall through
            } catch (IllegalAccessException e) {
                // fall through
            }
        }
        if (result == null) {
            result = new LastResortLocaleDisplayNames(locale, dialectHandling);
        }
        return result;
    }

    /**
     * Returns an instance of LocaleDisplayNames that returns names formatted for the provided locale,
     * using the provided DisplayContext settings
     * @param locale the display locale
     * @param contexts one or more context settings (e.g. for dialect
     *              handling, capitalization, etc.
     * @return a LocaleDisplayNames instance
     * @stable ICU 51
     */
    public static LocaleDisplayNames getInstance(ULocale locale, DisplayContext... contexts) {
        LocaleDisplayNames result = null;
        if (FACTORY_DISPLAYCONTEXT != null) {
            try {
                result = (LocaleDisplayNames) FACTORY_DISPLAYCONTEXT.invoke(null,
                        locale, (Object[])contexts);
            } catch (InvocationTargetException e) {
                // fall through
            } catch (IllegalAccessException e) {
                // fall through
            }
        }
        if (result == null) {
            result = new LastResortLocaleDisplayNames(locale, contexts);
        }
        return result;
    }

    // getters for state
    /**
     * Returns the locale used to determine the display names. This is not necessarily the same
     * locale passed to {@link #getInstance}.
     * @return the display locale
     * @stable ICU 4.4
     */
    public abstract ULocale getLocale();

    /**
     * Returns the dialect handling used in the display names.
     * @return the dialect handling enum
     * @stable ICU 4.4
     */
    public abstract DialectHandling getDialectHandling();

    /**
     * Returns the current value for a specified DisplayContext.Type.
     * @param type the DisplayContext.Type whose value to return
     * @return the current DisplayContext setting for the specified type
     * @stable ICU 51
     */
    public abstract DisplayContext getContext(DisplayContext.Type type);

    // names for entire locales
    /**
     * Returns the display name of the provided ulocale.
     * @param locale the locale whose display name to return
     * @return the display name of the provided locale
     * @stable ICU 4.4
     */
    public abstract String localeDisplayName(ULocale locale);

    /**
     * Returns the display name of the provided locale.
     * @param locale the locale whose display name to return
     * @return the display name of the provided locale
     * @stable ICU 4.4
     */
    public abstract String localeDisplayName(Locale locale);

    /**
     * Returns the display name of the provided locale id.
     * @param localeId the id of the locale whose display name to return
     * @return the display name of the provided locale
     * @stable ICU 4.4
     */
    public abstract String localeDisplayName(String localeId);

    // names for components of a locale id
    /**
     * Returns the display name of the provided language code.
     * @param lang the language code
     * @return the display name of the provided language code
     * @stable ICU 4.4
     */
    public abstract String languageDisplayName(String lang);

    /**
     * Returns the display name of the provided script code.
     * @param script the script code
     * @return the display name of the provided script code
     * @stable ICU 4.4
     */
    public abstract String scriptDisplayName(String script);
 
    /**
     * Returns the display name of the provided script code
     * when used in the context of a full locale name.
     * @param script the script code
     * @return the display name of the provided script code
     * @internal ICU 49
     * @deprecated This API is ICU internal only.
     */
    @Deprecated
    public String scriptDisplayNameInContext(String script) {
        return scriptDisplayName(script);
    }

    /**
     * Returns the display name of the provided script code.  See
     * {@link com.ibm.icu.lang.UScript} for recognized script codes.
     * @param scriptCode the script code number
     * @return the display name of the provided script code
     * @stable ICU 4.4
     */
    public abstract String scriptDisplayName(int scriptCode);

    /**
     * Returns the display name of the provided region code.
     * @param region the region code
     * @return the display name of the provided region code
     * @stable ICU 4.4
     */
    public abstract String regionDisplayName(String region);

    /**
     * Returns the display name of the provided variant.
     * @param variant the variant string
     * @return the display name of the provided variant
     * @stable ICU 4.4
     */
    public abstract String variantDisplayName(String variant);

    /**
     * Returns the display name of the provided locale key.
     * @param key the locale key name
     * @return the display name of the provided locale key
     * @stable ICU 4.4
     */
    public abstract String keyDisplayName(String key);

    /**
     * Returns the display name of the provided value (used with the provided key).
     * @param key the locale key name
     * @param value the locale key's value
     * @return the display name of the provided value
     * @stable ICU 4.4
     */
    public abstract String keyValueDisplayName(String key, String value);

    /**
     * Sole constructor.  (For invocation by subclass constructors,
     * typically implicit.)
     * @internal
     * @deprecated This API is ICU internal only.
     */
    @Deprecated
    protected LocaleDisplayNames() {
    }

    private static final Method FACTORY_DIALECTHANDLING;
    private static final Method FACTORY_DISPLAYCONTEXT;

    static {
        String implClassName = ICUConfig.get("com.ibm.icu.text.LocaleDisplayNames.impl", "com.ibm.icu.impl.LocaleDisplayNamesImpl");

        Method factoryDialectHandling = null;
        Method factoryDisplayContext = null;

        try {
            Class<?> implClass = Class.forName(implClassName);
            try {
                factoryDialectHandling = implClass.getMethod("getInstance",
                        ULocale.class, DialectHandling.class);
            } catch (NoSuchMethodException e) {
            }
            try {
                factoryDisplayContext = implClass.getMethod("getInstance",
                        ULocale.class, DisplayContext[].class);
            } catch (NoSuchMethodException e) {
            }

        } catch (ClassNotFoundException e) {
            // fallback to last resort impl
        }

        FACTORY_DIALECTHANDLING = factoryDialectHandling;
        FACTORY_DISPLAYCONTEXT = factoryDisplayContext;
    }

    /**
     * Minimum implementation of LocaleDisplayNames
     */
    private static class LastResortLocaleDisplayNames extends LocaleDisplayNames {

        private ULocale locale;
        private DisplayContext[] contexts;

        private LastResortLocaleDisplayNames(ULocale locale, DialectHandling dialectHandling) {
            this.locale = locale;
            DisplayContext context = (dialectHandling == DialectHandling.DIALECT_NAMES) ?
                    DisplayContext.DIALECT_NAMES : DisplayContext.STANDARD_NAMES;
            this.contexts = new DisplayContext[] {context};
        }

        private LastResortLocaleDisplayNames(ULocale locale, DisplayContext... contexts) {
            this.locale = locale;
            this.contexts = new DisplayContext[contexts.length];
            System.arraycopy(contexts, 0, this.contexts, 0, contexts.length);
        }

        @Override
        public ULocale getLocale() {
            return locale;
        }

        @Override
        public DialectHandling getDialectHandling() {
            DialectHandling result = DialectHandling.STANDARD_NAMES;
            for (DisplayContext context : contexts) {
                if (context.type() == DisplayContext.Type.DIALECT_HANDLING) {
                    if (context.value() == DisplayContext.DIALECT_NAMES.ordinal()) {
                        result = DialectHandling.DIALECT_NAMES;
                        break;
                    }
                }
            }
            return result;
        }

        @Override
        public DisplayContext getContext(Type type) {
            DisplayContext result = DisplayContext.STANDARD_NAMES;  // final fallback
            for (DisplayContext context : contexts) {
                if (context.type() == type) {
                    result = context;
                    break;
                }
            }
            return result;
        }

        @Override
        public String localeDisplayName(ULocale locale) {
            return locale.getName();
        }

        @Override
        public String localeDisplayName(Locale locale) {
            return ULocale.forLocale(locale).getName();
        }

        @Override
        public String localeDisplayName(String localeId) {
            return new ULocale(localeId).getName();
        }

        @Override
        public String languageDisplayName(String lang) {
            return lang;
        }

        @Override
        public String scriptDisplayName(String script) {
            return script;
        }

        @Override
        public String scriptDisplayName(int scriptCode) {
            return UScript.getShortName(scriptCode);
        }

        @Override
        public String regionDisplayName(String region) {
            return region;
        }

        @Override
        public String variantDisplayName(String variant) {
            return variant;
        }

        @Override
        public String keyDisplayName(String key) {
            return key;
        }

        @Override
        public String keyValueDisplayName(String key, String value) {
            return value;
        }
        
    }
}
