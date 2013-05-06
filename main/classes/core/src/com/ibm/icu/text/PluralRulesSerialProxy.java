/*
 *******************************************************************************
 * Copyright (C) 2013, Google Inc, International Business Machines Corporation and         *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 */
package com.ibm.icu.text;

import java.io.ObjectStreamException;
import java.io.Serializable;

/**
 * @author markdavis
 *
 */
public class PluralRulesSerialProxy implements Serializable {
    private static final long serialVersionUID = 42L;
    private final String data;
    PluralRulesSerialProxy(String rules) {
        data = rules;
    }
    private Object readResolve() throws ObjectStreamException {
        return PluralRules.createRules(data);
    }
}

