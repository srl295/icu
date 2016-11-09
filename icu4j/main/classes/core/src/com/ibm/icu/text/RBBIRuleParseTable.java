// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
/*
 *******************************************************************************
 * Copyright (c) 2003-2016, International Business Machines
 * Corporation and others. All Rights Reserved.
 *******************************************************************************
 */
 
package com.ibm.icu.text;
 
/**
 * Generated Java File.  Do not edit by hand.
 * This file contains the state table for the ICU Rule Based Break Iterator
 * rule parser.
 * It is generated by the Perl script "rbbicst.pl" from
 * the rule parser state definitions file "rbbirpt.txt".
 * @internal 
 *
 */
class RBBIRuleParseTable
{
     static final short doCheckVarDef = 1;
     static final short doDotAny = 2;
     static final short doEndAssign = 3;
     static final short doEndOfRule = 4;
     static final short doEndVariableName = 5;
     static final short doExit = 6;
     static final short doExprCatOperator = 7;
     static final short doExprFinished = 8;
     static final short doExprOrOperator = 9;
     static final short doExprRParen = 10;
     static final short doExprStart = 11;
     static final short doLParen = 12;
     static final short doNOP = 13;
     static final short doNoChain = 14;
     static final short doOptionEnd = 15;
     static final short doOptionStart = 16;
     static final short doReverseDir = 17;
     static final short doRuleChar = 18;
     static final short doRuleError = 19;
     static final short doRuleErrorAssignExpr = 20;
     static final short doScanUnicodeSet = 21;
     static final short doSlash = 22;
     static final short doStartAssign = 23;
     static final short doStartTagValue = 24;
     static final short doStartVariableName = 25;
     static final short doTagDigit = 26;
     static final short doTagExpectedError = 27;
     static final short doTagValue = 28;
     static final short doUnaryOpPlus = 29;
     static final short doUnaryOpQuestion = 30;
     static final short doUnaryOpStar = 31;
     static final short doVariableNameExpectedErr = 32;
 
     static final short kRuleSet_default = 255;
     static final short kRuleSet_digit_char = 128;
     static final short kRuleSet_eof = 252;
     static final short kRuleSet_escaped = 254;
     static final short kRuleSet_name_char = 129;
     static final short kRuleSet_name_start_char = 130;
     static final short kRuleSet_rule_char = 131;
     static final short kRuleSet_white_space = 132;


   static class RBBIRuleTableElement { 
      short      fAction; 
      short      fCharClass; 
      short      fNextState; 
      short      fPushState; 
      boolean    fNextChar;  
      String     fStateName; 
      RBBIRuleTableElement(short a, int cc, int ns, int ps, boolean nc, String sn) {  
      fAction = a; 
      fCharClass = (short)cc; 
      fNextState = (short)ns; 
      fPushState = (short)ps; 
      fNextChar  = nc; 
      fStateName = sn; 
   } 
   }; 
  
    static RBBIRuleTableElement[] gRuleParseStateTable = { 
       new RBBIRuleTableElement(doNOP, 0, 0,0,  true,   null )     //  0 
     , new RBBIRuleTableElement(doExprStart, 254, 29, 9, false,   "start")     //  1 
     , new RBBIRuleTableElement(doNOP, 132, 1,0,  true,   null )     //  2 
     , new RBBIRuleTableElement(doNoChain,'^',  12, 9, true,   null )     //  3 
     , new RBBIRuleTableElement(doExprStart,'$',  88, 98, false,   null )     //  4 
     , new RBBIRuleTableElement(doNOP,'!',  19,0,  true,   null )     //  5 
     , new RBBIRuleTableElement(doNOP,';',  1,0,  true,   null )     //  6 
     , new RBBIRuleTableElement(doNOP, 252, 0,0,  false,   null )     //  7 
     , new RBBIRuleTableElement(doExprStart, 255, 29, 9, false,   null )     //  8 
     , new RBBIRuleTableElement(doEndOfRule,';',  1,0,  true,   "break-rule-end")     //  9 
     , new RBBIRuleTableElement(doNOP, 132, 9,0,  true,   null )     //  10 
     , new RBBIRuleTableElement(doRuleError, 255, 103,0,  false,   null )     //  11 
     , new RBBIRuleTableElement(doExprStart, 254, 29,0,  false,   "start-after-caret")     //  12 
     , new RBBIRuleTableElement(doNOP, 132, 12,0,  true,   null )     //  13 
     , new RBBIRuleTableElement(doRuleError,'^',  103,0,  false,   null )     //  14 
     , new RBBIRuleTableElement(doExprStart,'$',  88, 37, false,   null )     //  15 
     , new RBBIRuleTableElement(doRuleError,';',  103,0,  false,   null )     //  16 
     , new RBBIRuleTableElement(doRuleError, 252, 103,0,  false,   null )     //  17 
     , new RBBIRuleTableElement(doExprStart, 255, 29,0,  false,   null )     //  18 
     , new RBBIRuleTableElement(doNOP,'!',  21,0,  true,   "rev-option")     //  19 
     , new RBBIRuleTableElement(doReverseDir, 255, 28, 9, false,   null )     //  20 
     , new RBBIRuleTableElement(doOptionStart, 130, 23,0,  true,   "option-scan1")     //  21 
     , new RBBIRuleTableElement(doRuleError, 255, 103,0,  false,   null )     //  22 
     , new RBBIRuleTableElement(doNOP, 129, 23,0,  true,   "option-scan2")     //  23 
     , new RBBIRuleTableElement(doOptionEnd, 255, 25,0,  false,   null )     //  24 
     , new RBBIRuleTableElement(doNOP,';',  1,0,  true,   "option-scan3")     //  25 
     , new RBBIRuleTableElement(doNOP, 132, 25,0,  true,   null )     //  26 
     , new RBBIRuleTableElement(doRuleError, 255, 103,0,  false,   null )     //  27 
     , new RBBIRuleTableElement(doExprStart, 255, 29, 9, false,   "reverse-rule")     //  28 
     , new RBBIRuleTableElement(doRuleChar, 254, 38,0,  true,   "term")     //  29 
     , new RBBIRuleTableElement(doNOP, 132, 29,0,  true,   null )     //  30 
     , new RBBIRuleTableElement(doRuleChar, 131, 38,0,  true,   null )     //  31 
     , new RBBIRuleTableElement(doNOP,'[',  94, 38, false,   null )     //  32 
     , new RBBIRuleTableElement(doLParen,'(',  29, 38, true,   null )     //  33 
     , new RBBIRuleTableElement(doNOP,'$',  88, 37, false,   null )     //  34 
     , new RBBIRuleTableElement(doDotAny,'.',  38,0,  true,   null )     //  35 
     , new RBBIRuleTableElement(doRuleError, 255, 103,0,  false,   null )     //  36 
     , new RBBIRuleTableElement(doCheckVarDef, 255, 38,0,  false,   "term-var-ref")     //  37 
     , new RBBIRuleTableElement(doNOP, 132, 38,0,  true,   "expr-mod")     //  38 
     , new RBBIRuleTableElement(doUnaryOpStar,'*',  43,0,  true,   null )     //  39 
     , new RBBIRuleTableElement(doUnaryOpPlus,'+',  43,0,  true,   null )     //  40 
     , new RBBIRuleTableElement(doUnaryOpQuestion,'?',  43,0,  true,   null )     //  41 
     , new RBBIRuleTableElement(doNOP, 255, 43,0,  false,   null )     //  42 
     , new RBBIRuleTableElement(doExprCatOperator, 254, 29,0,  false,   "expr-cont")     //  43 
     , new RBBIRuleTableElement(doNOP, 132, 43,0,  true,   null )     //  44 
     , new RBBIRuleTableElement(doExprCatOperator, 131, 29,0,  false,   null )     //  45 
     , new RBBIRuleTableElement(doExprCatOperator,'[',  29,0,  false,   null )     //  46 
     , new RBBIRuleTableElement(doExprCatOperator,'(',  29,0,  false,   null )     //  47 
     , new RBBIRuleTableElement(doExprCatOperator,'$',  29,0,  false,   null )     //  48 
     , new RBBIRuleTableElement(doExprCatOperator,'.',  29,0,  false,   null )     //  49 
     , new RBBIRuleTableElement(doExprCatOperator,'/',  55,0,  false,   null )     //  50 
     , new RBBIRuleTableElement(doExprCatOperator,'{',  67,0,  true,   null )     //  51 
     , new RBBIRuleTableElement(doExprOrOperator,'|',  29,0,  true,   null )     //  52 
     , new RBBIRuleTableElement(doExprRParen,')',  255,0,  true,   null )     //  53 
     , new RBBIRuleTableElement(doExprFinished, 255, 255,0,  false,   null )     //  54 
     , new RBBIRuleTableElement(doSlash,'/',  57,0,  true,   "look-ahead")     //  55 
     , new RBBIRuleTableElement(doNOP, 255, 103,0,  false,   null )     //  56 
     , new RBBIRuleTableElement(doExprCatOperator, 254, 29,0,  false,   "expr-cont-no-slash")     //  57 
     , new RBBIRuleTableElement(doNOP, 132, 43,0,  true,   null )     //  58 
     , new RBBIRuleTableElement(doExprCatOperator, 131, 29,0,  false,   null )     //  59 
     , new RBBIRuleTableElement(doExprCatOperator,'[',  29,0,  false,   null )     //  60 
     , new RBBIRuleTableElement(doExprCatOperator,'(',  29,0,  false,   null )     //  61 
     , new RBBIRuleTableElement(doExprCatOperator,'$',  29,0,  false,   null )     //  62 
     , new RBBIRuleTableElement(doExprCatOperator,'.',  29,0,  false,   null )     //  63 
     , new RBBIRuleTableElement(doExprOrOperator,'|',  29,0,  true,   null )     //  64 
     , new RBBIRuleTableElement(doExprRParen,')',  255,0,  true,   null )     //  65 
     , new RBBIRuleTableElement(doExprFinished, 255, 255,0,  false,   null )     //  66 
     , new RBBIRuleTableElement(doNOP, 132, 67,0,  true,   "tag-open")     //  67 
     , new RBBIRuleTableElement(doStartTagValue, 128, 70,0,  false,   null )     //  68 
     , new RBBIRuleTableElement(doTagExpectedError, 255, 103,0,  false,   null )     //  69 
     , new RBBIRuleTableElement(doNOP, 132, 74,0,  true,   "tag-value")     //  70 
     , new RBBIRuleTableElement(doNOP,'}',  74,0,  false,   null )     //  71 
     , new RBBIRuleTableElement(doTagDigit, 128, 70,0,  true,   null )     //  72 
     , new RBBIRuleTableElement(doTagExpectedError, 255, 103,0,  false,   null )     //  73 
     , new RBBIRuleTableElement(doNOP, 132, 74,0,  true,   "tag-close")     //  74 
     , new RBBIRuleTableElement(doTagValue,'}',  77,0,  true,   null )     //  75 
     , new RBBIRuleTableElement(doTagExpectedError, 255, 103,0,  false,   null )     //  76 
     , new RBBIRuleTableElement(doExprCatOperator, 254, 29,0,  false,   "expr-cont-no-tag")     //  77 
     , new RBBIRuleTableElement(doNOP, 132, 77,0,  true,   null )     //  78 
     , new RBBIRuleTableElement(doExprCatOperator, 131, 29,0,  false,   null )     //  79 
     , new RBBIRuleTableElement(doExprCatOperator,'[',  29,0,  false,   null )     //  80 
     , new RBBIRuleTableElement(doExprCatOperator,'(',  29,0,  false,   null )     //  81 
     , new RBBIRuleTableElement(doExprCatOperator,'$',  29,0,  false,   null )     //  82 
     , new RBBIRuleTableElement(doExprCatOperator,'.',  29,0,  false,   null )     //  83 
     , new RBBIRuleTableElement(doExprCatOperator,'/',  55,0,  false,   null )     //  84 
     , new RBBIRuleTableElement(doExprOrOperator,'|',  29,0,  true,   null )     //  85 
     , new RBBIRuleTableElement(doExprRParen,')',  255,0,  true,   null )     //  86 
     , new RBBIRuleTableElement(doExprFinished, 255, 255,0,  false,   null )     //  87 
     , new RBBIRuleTableElement(doStartVariableName,'$',  90,0,  true,   "scan-var-name")     //  88 
     , new RBBIRuleTableElement(doNOP, 255, 103,0,  false,   null )     //  89 
     , new RBBIRuleTableElement(doNOP, 130, 92,0,  true,   "scan-var-start")     //  90 
     , new RBBIRuleTableElement(doVariableNameExpectedErr, 255, 103,0,  false,   null )     //  91 
     , new RBBIRuleTableElement(doNOP, 129, 92,0,  true,   "scan-var-body")     //  92 
     , new RBBIRuleTableElement(doEndVariableName, 255, 255,0,  false,   null )     //  93 
     , new RBBIRuleTableElement(doScanUnicodeSet,'[',  255,0,  true,   "scan-unicode-set")     //  94 
     , new RBBIRuleTableElement(doScanUnicodeSet,'p',  255,0,  true,   null )     //  95 
     , new RBBIRuleTableElement(doScanUnicodeSet,'P',  255,0,  true,   null )     //  96 
     , new RBBIRuleTableElement(doNOP, 255, 103,0,  false,   null )     //  97 
     , new RBBIRuleTableElement(doNOP, 132, 98,0,  true,   "assign-or-rule")     //  98 
     , new RBBIRuleTableElement(doStartAssign,'=',  29, 101, true,   null )     //  99 
     , new RBBIRuleTableElement(doNOP, 255, 37, 9, false,   null )     //  100 
     , new RBBIRuleTableElement(doEndAssign,';',  1,0,  true,   "assign-end")     //  101 
     , new RBBIRuleTableElement(doRuleErrorAssignExpr, 255, 103,0,  false,   null )     //  102 
     , new RBBIRuleTableElement(doExit, 255, 103,0,  true,   "errorDeath")     //  103 
 };
}; 
