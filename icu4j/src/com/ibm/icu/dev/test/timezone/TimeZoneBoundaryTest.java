/*
    @test 1.9 99/06/15
    @summary test Time Zone Boundary
*/

package com.ibm.test.timezone;
import com.ibm.text.*;
import com.ibm.util.*;
import com.ibm.test.*;
import java.util.Date;

/**
 * A test which discovers the boundaries of DST programmatically and verifies
 * that they are correct.
 */
public class TimeZoneBoundaryTest extends TestFmwk
{
    static final int ONE_SECOND = 1000;
    static final int ONE_MINUTE = 60*ONE_SECOND;
    static final int ONE_HOUR = 60*ONE_MINUTE;
    static final long ONE_DAY = 24*ONE_HOUR;
    static final long ONE_YEAR = (long)(365.25 * ONE_DAY);
    static final long SIX_MONTHS = ONE_YEAR / 2;

    static final int MONTH_LENGTH[] = {31,29,31,30,31,30,31,31,30,31,30,31};

    // These values are empirically determined to be correct
    static final long PST_1997_BEG  = 860320800000L;
    static final long PST_1997_END  = 877856400000L;

    // Minimum interval for binary searches in ms; should be no larger
    // than 1000.
    static final long INTERVAL = 10; // Milliseconds

    // When long zone names are supported again, switch this to the
    // long zone name.
    static final String AUSTRALIA = "AET"; // Australia/Adelaide
    static final long AUSTRALIA_1997_BEG = 872524800000L; // 877797000000L
    static final long AUSTRALIA_1997_END = 859651200000L; // 859653000000L
    
    public static void main(String[] args) throws Exception {
        new TimeZoneBoundaryTest().run(args);
    }

    /**
     * Date.toString().substring() Boundary Test
     * Look for a DST changeover to occur within 6 months of the given Date.
     * The initial Date.toString() should yield a string containing the
     * startMode as a SUBSTRING.  The boundary will be tested to be
     * at the expectedBoundary value.
     */
    void findDaylightBoundaryUsingDate(Date d, String startMode, long expectedBoundary)
    {
        // Given a date with a year start, find the Daylight onset
        // and end.  The given date should be 1/1/xx in some year.

        if (d.toString().indexOf(startMode) == -1)
        {
            logln("Error: " + startMode + " not present in " + d);
        }

        // Use a binary search, assuming that we have a Standard
        // time at the midpoint.
        long min = d.getTime();
        long max = min + SIX_MONTHS;

        while ((max - min) >  INTERVAL)
        {
            long mid = (min + max) >> 1;
            String s = new Date(mid).toString();
            // logln(s);
            if (s.indexOf(startMode) != -1)
            {
                min = mid;
            }
            else
            {
                max = mid;
            }
        }

        logln("Date Before: " + showDate(min));
        logln("Date After:  " + showDate(max));
        long mindelta = expectedBoundary - min;
        long maxdelta = max - expectedBoundary;
        if (mindelta >= 0 && mindelta <= INTERVAL &&
            mindelta >= 0 && mindelta <= INTERVAL)
            logln("PASS: Expected boundary at " + expectedBoundary);
        else
            errln("FAIL: Expected boundary at " + expectedBoundary);
    }

    // This test cannot be compiled until the inDaylightTime() method of GregorianCalendar
    // becomes public.
    //    static void findDaylightBoundaryUsingCalendar(Date d, boolean startsInDST)
    //    {
    //  // Given a date with a year start, find the Daylight onset
    //  // and end.  The given date should be 1/1/xx in some year.
    //
    //  GregorianCalendar cal = new GregorianCalendar();
    //  cal.setTime(d);
    //  if (cal.inDaylightTime() != startsInDST)
    //  {
    //      logln("Error: inDaylightTime(" + d + ") != " + startsInDST);
    //  }
    //
    //  // Use a binary search, assuming that we have a Standard
    //  // time at the midpoint.
    //  long min = d.getTime();
    //  long max = min + (long)(365.25 / 2 * 24*60*60*1000);
    //
    //  while ((max - min) >  INTERVAL)
    //  {
    //      long mid = (min + max) >> 1;
    //      cal.setTime(new Date(mid));
    //      if (cal.inDaylightTime() == startsInDST)
    //      {
    //      min = mid;
    //      }
    //      else
    //      {
    //      max = mid;
    //      }
    //  }
    //
    //  logln("Calendar Before: " + showDate(min));
    //  logln("Calendar After:  " + showDate(max));
    //    }

    void findDaylightBoundaryUsingTimeZone(Date d, boolean startsInDST, long expectedBoundary)
    {
        findDaylightBoundaryUsingTimeZone(d, startsInDST, expectedBoundary,
                                          TimeZone.getDefault());
    }

    void findDaylightBoundaryUsingTimeZone(Date d, boolean startsInDST,
                                           long expectedBoundary, TimeZone tz)
    {
        // Given a date with a year start, find the Daylight onset
        // and end.  The given date should be 1/1/xx in some year.

        // Use a binary search, assuming that we have a Standard
        // time at the midpoint.
        long min = d.getTime();
        long max = min + SIX_MONTHS;

        if (tz.inDaylightTime(d) != startsInDST)
        {
            errln("FAIL: " + tz.getID() + " inDaylightTime(" +
                  d + ") != " + startsInDST);
            startsInDST = !startsInDST; // Flip over; find the apparent value
        }

        if (tz.inDaylightTime(new Date(max)) == startsInDST)
        {
            errln("FAIL: " + tz.getID() + " inDaylightTime(" +
                  (new Date(max)) + ") != " + (!startsInDST));
            return;
        }

        while ((max - min) >  INTERVAL)
        {
            long mid = (min + max) >> 1;
            boolean isIn = tz.inDaylightTime(new Date(mid));
            if (isIn == startsInDST)
            {
                min = mid;
            }
            else
            {
                max = mid;
            }
        }

        logln(tz.getID() + " Before: " + showDate(min, tz));
        logln(tz.getID() + " After:  " + showDate(max, tz));

        long mindelta = expectedBoundary - min;
        long maxdelta = max - expectedBoundary;
        if (mindelta >= 0 && mindelta <= INTERVAL &&
            mindelta >= 0 && mindelta <= INTERVAL)
            logln("PASS: Expected boundary at " + expectedBoundary);
        else
            errln("FAIL: Expected boundary at " + expectedBoundary);
    }

    private static String showDate(long l)
    {
        return showDate(new Date(l));
    }

    private static String showDate(Date d)
    {
        return "" + d.getYear() + "/" + showNN(d.getMonth()+1) + "/" + showNN(d.getDate()) +
            " " + showNN(d.getHours()) + ":" + showNN(d.getMinutes()) +
            " \"" + d + "\" = " +
            d.getTime();
    }

    private static String showDate(long l, TimeZone z)
    {
        return showDate(new Date(l), z);
    }

    private static String showDate(Date d, TimeZone zone)
    {
        DateFormat fmt = DateFormat.getDateTimeInstance(DateFormat.LONG, DateFormat.LONG);
        fmt.setTimeZone(zone);
        return "" + d.getYear() + "/" + showNN(d.getMonth()+1) + "/" + showNN(d.getDate()) +
            " " + showNN(d.getHours()) + ":" + showNN(d.getMinutes()) +
            " \"" + d + "\" = " +
            fmt.format(d);
    }

    private static String showNN(int n)
    {
        return ((n < 10) ? "0" : "") + n;
    }

    /**
     * Given a date, a TimeZone, and expected values for inDaylightTime,
     * useDaylightTime, zone and DST offset, verify that this is the case.
     */
    void verifyDST(Date d, TimeZone time_zone,
                   boolean expUseDaylightTime, boolean expInDaylightTime,
                   int expZoneOffset, int expDSTOffset)
    {
        logln("-- Verifying time " + d +
              " in zone " + time_zone.getID());

        if (time_zone.inDaylightTime(d) == expInDaylightTime)
            logln("PASS: inDaylightTime = " + time_zone.inDaylightTime(d));
        else
            errln("FAIL: inDaylightTime = " + time_zone.inDaylightTime(d));

        if (time_zone.useDaylightTime() == expUseDaylightTime)
            logln("PASS: useDaylightTime = " + time_zone.useDaylightTime());
        else
            errln("FAIL: useDaylightTime = " + time_zone.useDaylightTime());

        if (time_zone.getRawOffset() == expZoneOffset)
            logln("PASS: getRawOffset() = " + expZoneOffset/(double)ONE_HOUR);
        else
            errln("FAIL: getRawOffset() = " + time_zone.getRawOffset()/(double)ONE_HOUR +
                  "; expected " + expZoneOffset/(double)ONE_HOUR);

        GregorianCalendar gc = new GregorianCalendar(time_zone);
        gc.setTime(d);
        int offset = time_zone.getOffset(gc.get(gc.ERA), gc.get(gc.YEAR), gc.get(gc.MONTH),
                                         gc.get(gc.DAY_OF_MONTH), gc.get(gc.DAY_OF_WEEK),
                                         ((gc.get(gc.HOUR_OF_DAY) * 60 +
                                           gc.get(gc.MINUTE)) * 60 +
                                          gc.get(gc.SECOND)) * 1000 +
                                         gc.get(gc.MILLISECOND));
        if (offset == expDSTOffset)
            logln("PASS: getOffset() = " + offset/(double)ONE_HOUR);
        else
            errln("FAIL: getOffset() = " + offset/(double)ONE_HOUR +
                  "; expected " + expDSTOffset/(double)ONE_HOUR);
    }

    public void TestBoundaries()
    {
        TimeZone pst = TimeZone.getTimeZone("PST");
        TimeZone save = TimeZone.getDefault();
        try {
            TimeZone.setDefault(pst);

            // DST changeover for PST is 4/6/1997 at 2 hours past midnight
            Date d = new Date(97,Calendar.APRIL,6);

            // i is minutes past midnight standard time
            for (int i=60; i<=180; i+=15)
            {
                boolean inDST = (i >= 120);
                Date e = new Date(d.getTime() + i*60*1000);
                verifyDST(e, pst, true, inDST, -8*ONE_HOUR,
                          inDST ? -7*ONE_HOUR : -8*ONE_HOUR);
            }
        } finally {
            TimeZone.setDefault(save);
        }

        if (true)
        {
            // This only works in PST/PDT
            TimeZone.setDefault(TimeZone.getTimeZone("PST"));
            logln("========================================");
            findDaylightBoundaryUsingDate(new Date(97,0,1), "PST", PST_1997_BEG);
            logln("========================================");
            findDaylightBoundaryUsingDate(new Date(97,6,1), "PDT", PST_1997_END);
        }

        //  if (true)
        //  {
        //      logln("========================================");
        //      findDaylightBoundaryUsingCalendar(new Date(97,0,1), false);
        //      logln("========================================");
        //      findDaylightBoundaryUsingCalendar(new Date(97,6,1), true);
        //  }

        if (true)
        {
            // Southern hemisphere test
            logln("========================================");
            TimeZone z = TimeZone.getTimeZone(AUSTRALIA);
            findDaylightBoundaryUsingTimeZone(new Date(97,0,1), true, AUSTRALIA_1997_END, z);
            logln("========================================");
            findDaylightBoundaryUsingTimeZone(new Date(97,6,1), false, AUSTRALIA_1997_BEG, z);
        }

        if (true)
        {
            logln("========================================");
            findDaylightBoundaryUsingTimeZone(new Date(97,0,1), false, PST_1997_BEG);
            logln("========================================");
            findDaylightBoundaryUsingTimeZone(new Date(97,6,1), true, PST_1997_END);
        }

        // This just shows the offset for April 4-7 in 1997.  This is redundant
        // with a test above, so we disable it.
        if (false)
        {
            TimeZone z = TimeZone.getDefault();
            logln(z.getOffset(1, 97, 3, 4, 6, 0) + " " + new Date(97, 3, 4));
            logln(z.getOffset(1, 97, 3, 5, 7, 0) + " " + new Date(97, 3, 5));
            logln(z.getOffset(1, 97, 3, 6, 1, 0) + " " + new Date(97, 3, 6));
            logln(z.getOffset(1, 97, 3, 7, 2, 0) + " " + new Date(97, 3, 7));
        }
    }


    //----------------------------------------------------------------------
    // Can't do any of these without a public inDaylightTime in GC
    //----------------------------------------------------------------------


    //    static GregorianCalendar cal = new GregorianCalendar();
    //
    //    static void _testUsingBinarySearch(Date d, boolean startsInDST)
    //    {
    //  // Given a date with a year start, find the Daylight onset
    //  // and end.  The given date should be 1/1/xx in some year.
    //
    //  // Use a binary search, assuming that we have a Standard
    //  // time at the midpoint.
    //  long min = d.getTime();
    //  long max = min + (long)(365.25 / 2 * ONE_DAY);
    //
    //  // First check the max
    //  cal.setTime(new Date(max));
    //  if (cal.inDaylightTime() == startsInDST)
    //  {
    //      logln("Error: inDaylightTime(" + (new Date(max)) + ") != " + (!startsInDST));
    //  }
    //
    //  cal.setTime(d);
    //  if (cal.inDaylightTime() != startsInDST)
    //  {
    //      logln("Error: inDaylightTime(" + d + ") != " + startsInDST);
    //  }
    //
    //  while ((max - min) >  INTERVAL)
    //  {
    //      long mid = (min + max) >> 1;
    //      cal.setTime(new Date(mid));
    //      if (cal.inDaylightTime() == startsInDST)
    //      {
    //      min = mid;
    //      }
    //      else
    //      {
    //      max = mid;
    //      }
    //  }
    //
    //  logln("Binary Search Before: " + showDate(min));
    //  logln("Binary Search After:  " + showDate(max));
    //    }
    //
    //    static void _testUsingMillis(Date d, boolean startsInDST)
    //    {
    //  long millis = d.getTime();
    //  long max = millis + (long)(370 * ONE_DAY); // A year plus extra
    //
    //  boolean lastDST = startsInDST;
    //  while (millis < max)
    //  {
    //      cal.setTime(new Date(millis));
    //      boolean inDaylight = cal.inDaylightTime();
    //
    //      if (inDaylight != lastDST)
    //      {
    //      logln("Switch " + (inDaylight ? "into" : "out of")
    //                 + " DST at " + (new Date(millis)));
    //      lastDST = inDaylight;
    //      }
    //
    //      millis += 15*ONE_MINUTE;
    //  }
    //    }
    //
    //    static void _testUsingFields(int y, boolean startsInDST)
    //    {
    //  boolean lastDST = startsInDST;
    //  for (int m = 0; m < 12; ++m)
    //  {
    //      for (int d = 1; d <= MONTH_LENGTH[m]; ++d)
    //      {
    //      for (int h = 0; h < 24; ++h)
    //      {
    //          for (int min = 0; min < 60; min += 15)
    //          {
    //          cal.clear();
    //          cal.set(y, m, d, h, min);
    //          boolean inDaylight = cal.inDaylightTime();
    //          if (inDaylight != lastDST)
    //          {
    //              lastDST = inDaylight;
    //              log("Switch " + (lastDST ? "into" : "out of")
    //                       + " DST at " + y + "/" + (m+1) + "/" + d
    //                       + " " + showNN(h) + ":" + showNN(min));
    //              logln(" " + cal.getTime());
    //
    //              cal.set(y, m, d, h-1, 45);
    //              log("Before = "
    //+ y + "/" + (m+1) + "/" + d
    //+ " " + showNN(h-1) + ":" + showNN(45));
    //              logln(" " + cal.getTime());
    //          }
    //          }
    //      }
    //      }
    //  }
    //    }
    //
    //    public void Test1()
    //    {
    //  logln(Locale.getDefault().getDisplayName());
    //  logln(TimeZone.getDefault().getID());
    //  logln(new Date(0));
    //
    //  if (true)
    //  {
    //      logln("========================================");
    //      _testUsingBinarySearch(new Date(97,0,1), false);
    //      logln("========================================");
    //      _testUsingBinarySearch(new Date(97,6,1), true);
    //  }
    //
    //  if (true)
    //  {
    //      logln("========================================");
    //      logln("Stepping using millis");
    //      _testUsingMillis(new Date(97,0,1), false);
    //  }
    //
    //  if (true)
    //  {
    //      logln("========================================");
    //      logln("Stepping using fields");
    //      _testUsingFields(1997, false);
    //  }
    //
    //  if (false)
    //  {
    //      cal.clear();
    //      cal.set(1997, 3, 5, 10, 0);
    //      //  cal.inDaylightTime();
    //      logln("Date = " + cal.getTime());
    //      logln("Millis = " + cal.getTime().getTime()/3600000);
    //  }
    //    }

    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
    //----------------------------------------------------------------------

    void _testUsingBinarySearch(SimpleTimeZone tz, Date d, long expectedBoundary)
    {
        // Given a date with a year start, find the Daylight onset
        // and end.  The given date should be 1/1/xx in some year.

        // Use a binary search, assuming that we have a Standard
        // time at the midpoint.
        long min = d.getTime();
        long max = min + (long)(365.25 / 2 * ONE_DAY);

        // First check the boundaries
        boolean startsInDST = tz.inDaylightTime(d);

        if (tz.inDaylightTime(new Date(max)) == startsInDST)
        {
            logln("Error: inDaylightTime(" + (new Date(max)) + ") != " + (!startsInDST));
        }

        while ((max - min) >  INTERVAL)
        {
            long mid = (min + max) >> 1;
            if (tz.inDaylightTime(new Date(mid)) == startsInDST)
            {
                min = mid;
            }
            else
            {
                max = mid;
            }
        }

        logln("Binary Search Before: " + showDate(min));
        logln("Binary Search After:  " + showDate(max));

        long mindelta = expectedBoundary - min;
        long maxdelta = max - expectedBoundary;
        if (mindelta >= 0 && mindelta <= INTERVAL &&
            mindelta >= 0 && mindelta <= INTERVAL)
            logln("PASS: Expected boundary at " + expectedBoundary);
        else
            errln("FAIL: Expected boundary at " + expectedBoundary);
    }

    /*
      static void _testUsingMillis(Date d, boolean startsInDST)
      {
      long millis = d.getTime();
      long max = millis + (long)(370 * ONE_DAY); // A year plus extra

      boolean lastDST = startsInDST;
      while (millis < max)
      {
      cal.setTime(new Date(millis));
      boolean inDaylight = cal.inDaylightTime();

      if (inDaylight != lastDST)
      {
      logln("Switch " + (inDaylight ? "into" : "out of")
      + " DST at " + (new Date(millis)));
      lastDST = inDaylight;
      }

      millis += 15*ONE_MINUTE;
      }
      }
      */

    /**
     * Test new rule formats.
     */
    public void TestNewRules()
    {
        //logln(Locale.getDefault().getDisplayName());
        //logln(TimeZone.getDefault().getID());
        //logln(new Date(0));

        if (true)
        {
            // Doesn't matter what the default TimeZone is here, since we
            // are creating our own TimeZone objects.

            SimpleTimeZone tz;

            logln("-----------------------------------------------------------------");
            logln("Aug 2ndTues .. Mar 15");
            tz = new SimpleTimeZone(-8*ONE_HOUR, "Test_1",
                                    Calendar.AUGUST, 2, Calendar.TUESDAY, 2*ONE_HOUR,
                                    Calendar.MARCH, 15, 0, 2*ONE_HOUR);
            //logln(tz.toString());
            logln("========================================");
            _testUsingBinarySearch(tz, new Date(97,0,1), 858416400000L);
            logln("========================================");
            _testUsingBinarySearch(tz, new Date(97,6,1), 871380000000L);

            logln("-----------------------------------------------------------------");
            logln("Apr Wed>=14 .. Sep Sun<=20");
            tz = new SimpleTimeZone(-8*ONE_HOUR, "Test_2",
                                    Calendar.APRIL, 14, -Calendar.WEDNESDAY, 2*ONE_HOUR,
                                    Calendar.SEPTEMBER, -20, -Calendar.SUNDAY, 2*ONE_HOUR);
            //logln(tz.toString());
            logln("========================================");
            _testUsingBinarySearch(tz, new Date(97,0,1), 861184800000L);
            logln("========================================");
            _testUsingBinarySearch(tz, new Date(97,6,1), 874227600000L);
        }

        /*
          if (true)
          {
          logln("========================================");
          logln("Stepping using millis");
          _testUsingMillis(new Date(97,0,1), false);
          }

          if (true)
          {
          logln("========================================");
          logln("Stepping using fields");
          _testUsingFields(1997, false);
          }

          if (false)
          {
          cal.clear();
          cal.set(1997, 3, 5, 10, 0);
          //    cal.inDaylightTime();
          logln("Date = " + cal.getTime());
          logln("Millis = " + cal.getTime().getTime()/3600000);
          }
          */
    }

    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
    // Long Bug
    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
    //----------------------------------------------------------------------

    //public void Test3()
    //{
    //    findDaylightBoundaryUsingTimeZone(new Date(97,6,1), true);
    //}

    /**
     * Find boundaries by stepping.
     */
    void findBoundariesStepwise(int year, long interval, TimeZone z, int expectedChanges)
    {
        Date d = new Date(year - 1900, Calendar.JANUARY, 1);
        long time = d.getTime(); // ms
        long limit = time + ONE_YEAR + ONE_DAY;
        boolean lastState = z.inDaylightTime(d);
        int changes = 0;
        logln("-- Zone " + z.getID() + " starts in " + year + " with DST = " + lastState);
        logln("useDaylightTime = " + z.useDaylightTime());
        while (time < limit)
        {
            d.setTime(time);
            boolean state = z.inDaylightTime(d);
            if (state != lastState)
            {
                logln((state ? "Entry " : "Exit ") +
                      "at " + d);
                lastState = state;
                ++changes;
            }
            time += interval;
        }
        if (changes == 0)
        {
            if (!lastState && !z.useDaylightTime()) logln("No DST");
            else errln("FAIL: DST all year, or no DST with true useDaylightTime");
        }
        else if (changes != 2)
        {
            errln("FAIL: " + changes + " changes seen; should see 0 or 2");
        }
        else if (!z.useDaylightTime())
        {
            errln("FAIL: useDaylightTime false but 2 changes seen");
        }
        if (changes != expectedChanges)
        {
            errln("FAIL: " + changes + " changes seen; expected " + expectedChanges);
        }
    }

    public void TestStepwise()
    {
        findBoundariesStepwise(1997, ONE_DAY, TimeZone.getTimeZone("EST"), 2);
        findBoundariesStepwise(1997, ONE_DAY, TimeZone.getTimeZone("ACT"), 0);
        findBoundariesStepwise(1997, ONE_DAY, TimeZone.getTimeZone(AUSTRALIA), 2);
    }
}
