# *   Copyright (C) 1998-2011, International Business Machines
# *   Corporation and others.  All Rights Reserved.
RBNF_CLDR_VERSION = 2.0
# A list of txt's to build
# Note:
#
#   If you are thinking of modifying this file, READ THIS.
#
# Instead of changing this file [unless you want to check it back in],
# you should consider creating a 'rbnflocal.mk' file in this same directory.
# Then, you can have your local changes remain even if you upgrade or
# reconfigure ICU.
#
# Example 'rbnflocal.mk' files:
#
#  * To add an additional locale to the list:
#    _____________________________________________________
#    |  RBNF_SOURCE_LOCAL =   myLocale.txt ...
#
#  * To REPLACE the default list and only build with a few
#    locales:
#    _____________________________________________________
#    |  RBNF_SOURCE = ar.txt ar_AE.txt en.txt de.txt zh.txt
#
#
# Generated by LDML2ICUConverter, from LDML source files.

# Aliases without a corresponding xx.xml file (see icu-config.xml & build.xml)
RBNF_SYNTHETIC_ALIAS =


# All aliases (to not be included under 'installed'), but not including root.
RBNF_ALIAS_SOURCE = $(RBNF_SYNTHETIC_ALIAS)


# Ordinary resources
RBNF_SOURCE = af.txt am.txt ar.txt az.txt\
 be.txt bg.txt bs.txt ca.txt cs.txt\
 cy.txt da.txt de.txt el.txt en.txt\
 eo.txt es.txt es_419.txt es_AR.txt es_BO.txt\
 es_CL.txt es_CO.txt es_CR.txt es_DO.txt es_EC.txt\
 es_GT.txt es_HN.txt es_MX.txt es_NI.txt es_PA.txt\
 es_PE.txt es_PR.txt es_PY.txt es_SV.txt es_US.txt\
 es_UY.txt es_VE.txt et.txt fa.txt fa_AF.txt\
 fi.txt fil.txt fo.txt fr.txt fr_BE.txt\
 fr_CH.txt ga.txt he.txt hi.txt hr.txt\
 hu.txt hy.txt id.txt is.txt it.txt\
 ja.txt ka.txt kl.txt ko.txt lt.txt\
 lv.txt mk.txt ms.txt mt.txt nb.txt\
 nl.txt nn.txt pl.txt pt.txt pt_AO.txt\
 pt_GW.txt pt_MZ.txt pt_PT.txt pt_ST.txt ro.txt\
 ru.txt sk.txt sl.txt sq.txt sr.txt\
 sr_Latn.txt sv.txt ta.txt th.txt tr.txt\
 uk.txt vi.txt zh.txt zh_Hant.txt

