package com.ibm.text.resources;

import java.util.ListResourceBundle;

/**
 * Default RuleBasedNumberFormat data for Thai.
 *
 * @author Suwit
 */
public class NumberFormatRules_th extends ListResourceBundle {
    /**
     * Puts a copyright in the .class file
     */
    private static final String copyrightNotice
        = "Copyright \u00a92001 IBM Corp.  All rights reserved.";

    public Object[][] getContents() {
        return contents;
    }

    Object[][] contents = {
        { "SpelloutRules",
          "%default:\n"
        + "    -x: \u0e25\u0e1a>>;\n"
        + "    x.x: <<\u0e08\u0e38\u0e14>>>;\n"
        + "    \u0e28\u0e39\u0e19\u0e22\u0e4c; \u0e2b\u0e19\u0e36\u0e48\u0e07; \u0e2a\u0e2d\u0e07; \u0e2a\u0e32\u0e21;\n"
        + "    \u0e2a\u0e35\u0e48; \u0e2b\u0e49\u0e32; \u0e2b\u0e01; \u0e40\u0e08\u0e47\u0e14; \u0e41\u0e1b\u0e14;\n"
        + "    \u0e40\u0e01\u0e49\u0e32; \u0e2a\u0e34\u0e1a; \u0e2a\u0e34\u0e1a\u0e40\u0e2d\u0e47\u0e14;\n"
        + "    \u0e2a\u0e34\u0e1a\u0e2a\u0e2d\u0e07; \u0e2a\u0e34\u0e1a\u0e2a\u0e32\u0e21;\n"
        + "    \u0e2a\u0e34\u0e1a\u0e2a\u0e35\u0e48; \u0e2a\u0e34\u0e1a\u0e2b\u0e49\u0e32;\n"
        + "    \u0e2a\u0e34\u0e1a\u0e2b\u0e01; \u0e2a\u0e34\u0e1a\u0e40\u0e08\u0e47\u0e14;\n"
        + "    \u0e2a\u0e34\u0e1a\u0e41\u0e1b\u0e14; \u0e2a\u0e34\u0e1a\u0e40\u0e01\u0e49\u0e32;\n"
        + "    20: \u0e22\u0e35\u0e48\u0e2a\u0e34\u0e1a[>%%alt-ones>];\n"
        + "    30: \u0e2a\u0e32\u0e21\u0e2a\u0e34\u0e1a[>%%alt-ones>];\n"
        + "    40: \u0e2a\u0e35\u0e48\u0e2a\u0e34\u0e1a[>%%alt-ones>];\n"
        + "    50: \u0e2b\u0e49\u0e32\u0e2a\u0e34\u0e1a[>%%alt-ones>];\n"
        + "    60: \u0e2b\u0e01\u0e2a\u0e34\u0e1a[>%%alt-ones>];\n"
        + "    70: \u0e40\u0e08\u0e47\u0e14\u0e2a\u0e34\u0e1a[>%%alt-ones>];\n"
        + "    80: \u0e41\u0e1b\u0e14\u0e2a\u0e34\u0e1a[>%%alt-ones>];\n"
        + "    90: \u0e40\u0e01\u0e49\u0e32\u0e2a\u0e34\u0e1a[>%%alt-ones>];\n"
        + "    100: <<\u0e23\u0e49\u0e2d\u0e22[>>];\n"
        + "    1000: <<\u0e1e\u0e31\u0e19[>>];\n"
        + "    10000: <<\u0e2b\u0e21\u0e37\u0e48\u0e19[>>];\n"
        + "    100000: <<\u0e41\u0e2a\u0e19[>>];\n"
        + "    1,000,000: <<\u0e25\u0e49\u0e32\u0e19[>>];\n"
        + "    1,000,000,000: <<\u0e1e\u0e31\u0e19\u0e25\u0e49\u0e32\u0e19[>>];\n"
        + "    1,000,000,000,000: <<\u0e25\u0e49\u0e32\u0e19\u0e25\u0e49\u0e32\u0e19[>>];\n"
        + "    1,000,000,000,000,000: =#,##0=;\n"
        + "%%alt-ones:\n"
        + "    \u0e28\u0e39\u0e19\u0e22\u0e4c;\n"
        + "    \u0e40\u0e2d\u0e47\u0e14;\n"
        + "    =%default=;\n"
        }
    };
}
