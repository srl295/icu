// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
/*
 *******************************************************************************
 * Copyright (C) 2004-2010, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 *
 */

package com.ibm.icu.dev.test.timescale;

import java.util.Date;
import java.util.Locale;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import com.ibm.icu.dev.test.TestFmwk;
import com.ibm.icu.util.GregorianCalendar;
import com.ibm.icu.util.SimpleTimeZone;
import com.ibm.icu.util.TimeZone;
import com.ibm.icu.util.UniversalTimeScale;

/**
 * @author Owner
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
@RunWith(JUnit4.class)
public class TimeScaleDataTest extends TestFmwk
{

    /**
     * Default contstructor.
     */
    public TimeScaleDataTest()
    {
    }

    private void roundTripTest(long value, int scale)
    {
        long rt = UniversalTimeScale.toLong(UniversalTimeScale.from(value, scale), scale);

        if (rt != value) {
            errln("Round-trip error: time scale = " + scale + ", value = " + value + ", round-trip = " + rt);
        }
    }

    private void toLimitTest(long toLimit, long fromLimit, int scale)
    {
        long result = UniversalTimeScale.toLong(toLimit, scale);

        if (result != fromLimit) {
            errln("toLimit failure: scale = " + scale + ", toLimit = " + toLimit +
                  ", toLong(toLimit, scale) = " + result + ", fromLimit = " + fromLimit);
        }
    }

    private void epochOffsetTest(long epochOffset, long units, int scale)
    {
        long universalEpoch = epochOffset * units;
        long local = UniversalTimeScale.toLong(universalEpoch, scale);

        if (local != 0) {
            errln("toLong(epochOffset, scale): scale = " + scale + ", epochOffset = " + universalEpoch +
                  ", result = " + local);
        }

        local = UniversalTimeScale.toLong(0, scale);

        if (local != -epochOffset) {
            errln("toLong(0, scale): scale = " + scale + ", result = " + local);
        }

        long universal = UniversalTimeScale.from(-epochOffset, scale);

        if (universal != 0) {
            errln("from(-epochOffest, scale): scale = " + scale + ", epochOffset = " + epochOffset +
                  ", result = " + universal);
        }

        universal = UniversalTimeScale.from(0, scale);

        if (universal != universalEpoch) {
            errln("from(0, scale): scale = " + scale + ", result = " + universal);
        }
    }

    @Test
    public void TestEpochOffsets()
    {
        for (int scale = 0; scale < UniversalTimeScale.MAX_SCALE; scale += 1) {
            long units       = UniversalTimeScale.getTimeScaleValue(scale, UniversalTimeScale.UNITS_VALUE);
            long epochOffset = UniversalTimeScale.getTimeScaleValue(scale, UniversalTimeScale.EPOCH_OFFSET_VALUE);

            epochOffsetTest(epochOffset, units, scale);
        }
    }

    @Test
    public void TestFromLimits()
    {
        for (int scale = 0; scale < UniversalTimeScale.MAX_SCALE; scale += 1) {
            long fromMin = UniversalTimeScale.getTimeScaleValue(scale, UniversalTimeScale.FROM_MIN_VALUE);
            long fromMax = UniversalTimeScale.getTimeScaleValue(scale, UniversalTimeScale.FROM_MAX_VALUE);

            roundTripTest(fromMin, scale);
            roundTripTest(fromMax, scale);
        }
    }

    @Test
    public void TestToLimits()
    {
        for (int scale = 0; scale < UniversalTimeScale.MAX_SCALE; scale += 1) {
            long fromMin = UniversalTimeScale.getTimeScaleValue(scale, UniversalTimeScale.FROM_MIN_VALUE);
            long fromMax = UniversalTimeScale.getTimeScaleValue(scale, UniversalTimeScale.FROM_MAX_VALUE);
            long toMin   = UniversalTimeScale.getTimeScaleValue(scale, UniversalTimeScale.TO_MIN_VALUE);
            long toMax   = UniversalTimeScale.getTimeScaleValue(scale, UniversalTimeScale.TO_MAX_VALUE);

            toLimitTest(toMin, fromMin, scale);
            toLimitTest(toMax, fromMax, scale);
       }
    }

    // Test with data from .Net System.DateTime ---------------------------- ***

    /*
     * This data was generated by C++.Net code like
     * Console::WriteLine(L"    {{ {0}, 1, 1, INT64_C({1}) }},", year, DateTime(year, 1, 1).Ticks);
     * with the DateTime constructor taking int values for year, month, and date.
     */
    static private final long dotNetDateTimeTicks[] = {
        /* year, month, day, ticks */
        100, 1, 1, 31241376000000000L,
        100, 3, 1, 31292352000000000L,
        200, 1, 1, 62798112000000000L,
        200, 3, 1, 62849088000000000L,
        300, 1, 1, 94354848000000000L,
        300, 3, 1, 94405824000000000L,
        400, 1, 1, 125911584000000000L,
        400, 3, 1, 125963424000000000L,
        500, 1, 1, 157469184000000000L,
        500, 3, 1, 157520160000000000L,
        600, 1, 1, 189025920000000000L,
        600, 3, 1, 189076896000000000L,
        700, 1, 1, 220582656000000000L,
        700, 3, 1, 220633632000000000L,
        800, 1, 1, 252139392000000000L,
        800, 3, 1, 252191232000000000L,
        900, 1, 1, 283696992000000000L,
        900, 3, 1, 283747968000000000L,
        1000, 1, 1, 315253728000000000L,
        1000, 3, 1, 315304704000000000L,
        1100, 1, 1, 346810464000000000L,
        1100, 3, 1, 346861440000000000L,
        1200, 1, 1, 378367200000000000L,
        1200, 3, 1, 378419040000000000L,
        1300, 1, 1, 409924800000000000L,
        1300, 3, 1, 409975776000000000L,
        1400, 1, 1, 441481536000000000L,
        1400, 3, 1, 441532512000000000L,
        1500, 1, 1, 473038272000000000L,
        1500, 3, 1, 473089248000000000L,
        1600, 1, 1, 504595008000000000L,
        1600, 3, 1, 504646848000000000L,
        1700, 1, 1, 536152608000000000L,
        1700, 3, 1, 536203584000000000L,
        1800, 1, 1, 567709344000000000L,
        1800, 3, 1, 567760320000000000L,
        1900, 1, 1, 599266080000000000L,
        1900, 3, 1, 599317056000000000L,
        2000, 1, 1, 630822816000000000L,
        2000, 3, 1, 630874656000000000L,
        2100, 1, 1, 662380416000000000L,
        2100, 3, 1, 662431392000000000L,
        2200, 1, 1, 693937152000000000L,
        2200, 3, 1, 693988128000000000L,
        2300, 1, 1, 725493888000000000L,
        2300, 3, 1, 725544864000000000L,
        2400, 1, 1, 757050624000000000L,
        2400, 3, 1, 757102464000000000L,
        2500, 1, 1, 788608224000000000L,
        2500, 3, 1, 788659200000000000L,
        2600, 1, 1, 820164960000000000L,
        2600, 3, 1, 820215936000000000L,
        2700, 1, 1, 851721696000000000L,
        2700, 3, 1, 851772672000000000L,
        2800, 1, 1, 883278432000000000L,
        2800, 3, 1, 883330272000000000L,
        2900, 1, 1, 914836032000000000L,
        2900, 3, 1, 914887008000000000L,
        3000, 1, 1, 946392768000000000L,
        3000, 3, 1, 946443744000000000L,
        1, 1, 1, 0L,
        1601, 1, 1, 504911232000000000L,
        1899, 12, 31, 599265216000000000L,
        1904, 1, 1, 600527520000000000L,
        1970, 1, 1, 621355968000000000L,
        2001, 1, 1, 631139040000000000L,
        9900, 3, 1, 3123873216000000000L,
        9999, 12, 31, 3155378112000000000L
    };

    /*
     * ICU's Universal Time Scale is designed to be tick-for-tick compatible with
     * .Net System.DateTime. Verify that this is so for the
     * .Net-supported date range (years 1-9999 AD).
     * This requires a proleptic Gregorian calendar because that's what .Net uses.
     * Proleptic: No Julian/Gregorian switchover, or a switchover before
     * any date that we test, that is, before 0001 AD.
     */
    @Test
    public void TestDotNet() {
        TimeZone utc;
        final long dayMillis = 86400 * 1000L;    /* 1 day = 86400 seconds */
        final long dayTicks = 86400 * 10000000L;
        final int kYear = 0;  // offset for dotNetDateTimeTicks[] field
        final int kMonth = 1;
        final int kDay = 2;
        final int kTicks = 3;
        final int kIncrement = 4;
        GregorianCalendar cal;
        long icuDate;
        long ticks, millis;
        int i;

        /* Open a proleptic Gregorian calendar. */
        long before0001AD = -1000000 * dayMillis;
        utc = new SimpleTimeZone(0, "UTC");
        cal = new GregorianCalendar(utc, Locale.ENGLISH);
        cal.setGregorianChange(new Date(before0001AD));
        for(i = 0; i < dotNetDateTimeTicks.length; i += kIncrement) {
            /* Test conversion from .Net/Universal time to ICU time. */
            millis = UniversalTimeScale.toLong(dotNetDateTimeTicks[i + kTicks], UniversalTimeScale.ICU4C_TIME);
            cal.clear();
            cal.set((int)dotNetDateTimeTicks[i + kYear],
                    (int)dotNetDateTimeTicks[i + kMonth] - 1, /* Java & ICU use January = month 0. */
                    (int)dotNetDateTimeTicks[i + kDay]);
            icuDate = cal.getTimeInMillis();
            if(millis != icuDate) {
                /* Print days not millis. */
                errln("UniversalTimeScale.toLong(ticks[" + i + "], ICU4C)=" +
                      (millis/dayMillis) + " != " + (icuDate/dayMillis) +
                      "=ucal_getMillis(" + dotNetDateTimeTicks[i + kYear] +
                      "-" + dotNetDateTimeTicks[i + kMonth] +
                      "-" + dotNetDateTimeTicks[i + kDay] + ")");
            }

            /* Test conversion from ICU time to .Net/Universal time. */
            ticks = UniversalTimeScale.from(icuDate, UniversalTimeScale.ICU4C_TIME);
            if(ticks != dotNetDateTimeTicks[i + kTicks]) {
                /* Print days not ticks. */
                errln("UniversalTimeScale.from(date[" + i + "], ICU4C)=" +
                      (ticks/dayTicks) + " != " + dotNetDateTimeTicks[i + kTicks]/dayTicks +
                      "=.Net System.DateTime(" + dotNetDateTimeTicks[i + kYear] +
                      "-" + dotNetDateTimeTicks[i + kMonth] +
                      "-" + dotNetDateTimeTicks[i + kDay] + ").Ticks");
            }
        }
    }
}
