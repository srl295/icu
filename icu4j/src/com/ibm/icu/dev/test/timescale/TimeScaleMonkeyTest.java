/*
 **************************************************************************
 * Copyright (C) 2004, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                           *
 **************************************************************************
 *
 */

package com.ibm.icu.dev.test.timescale;

import com.ibm.icu.dev.test.TestFmwk;
import com.ibm.icu.util.UniversalTimeScale;
import com.ibm.icu.math.BigDecimal;
import java.util.Random;

/**
 * This class tests the UniversalTimeScale class by
 * generating ramdon values in range and making sure
 * that they round-trip correctly.
 */
public class TimeScaleMonkeyTest extends TestFmwk
{

    /**
     * The default constructor.
     */
    public TimeScaleMonkeyTest()
    {
    }
    
    private static final int LOOP_COUNT = 1000;
    private static final BigDecimal longMax = new BigDecimal(Long.MAX_VALUE);
    
    private Random ran = null;
    
    private long ranInt;
    private long ranMin;
    private long ranMax;
    
    private void initRandom(long min, long max)
    {
        BigDecimal interval = new BigDecimal(max).subtract(new BigDecimal(min));
        
        ranMin = min;
        ranMax = max;
        ranInt = 0;
        
        if (ran == null) {
            ran = createRandom();
        }
        
        if (interval.compareTo(longMax) < 0) {
            ranInt = interval.longValue();
        }
    }
    
    private final long randomInRange()
    {
        long value;
        
        if (ranInt != 0) {
            value = ran.nextLong() % ranInt;
            
            if (value < 0) {
                value = -value;
            }
            
            value += ranMin;
        } else {
            do {
                value = ran.nextLong();
            } while (value < ranMin || value > ranMax);
        }
        
        return value;
    }
    
    public void TestRoundTrip()
    {
        for (int scale = 0; scale < UniversalTimeScale.MAX_SCALE; scale += 1) {
            UniversalTimeScale.TimeScaleData data = UniversalTimeScale.getTimeScaleData(scale);
            int i = 0;
            
            initRandom(data.fromMin, data.fromMax);
            
            while (i < LOOP_COUNT) {
                long value = randomInRange();
                                
                long rt = UniversalTimeScale.toLong(UniversalTimeScale.from(value, scale), scale);
                
                if (rt != value) {
                    errln("Round-trip error: time scale = " + scale + ", value = " + value + ", round-trip = " + rt);
                }
                
                i += 1;
            }
        }
    }

    public static void main(String[] args)
    {
        new TimeScaleMonkeyTest().run(args);
    }
}
