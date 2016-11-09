# © 2016 and later: Unicode, Inc. and others.
# License & terms of use: http://www.unicode.org/copyright.html#License
GENRB_CLDR_VERSION = %version%
# A list of txt's to build
# Note:
#
#   If you are thinking of modifying this file, READ THIS.
#
# Instead of changing this file [unless you want to check it back in],
# you should consider creating a '%local%' file in this same directory.
# Then, you can have your local changes remain even if you upgrade or
# reconfigure ICU.
#
# Example '%local%' files:
#
#  * To add an additional locale to the list:
#    _____________________________________________________
#    |  GENRB_SOURCE_LOCAL =   myLocale.txt ...
#
#  * To REPLACE the default list and only build with a few
#    locales:
#    _____________________________________________________
#    |  GENRB_SOURCE = ar.txt ar_AE.txt en.txt de.txt zh.txt
#
#
# Generated by LDML2ICUConverter, from LDML source files.

# Aliases without a corresponding xx.xml file (see icu-config.xml & build.xml)
GENRB_SYNTHETIC_ALIAS = az_AZ.txt bs_BA.txt en_NH.txt en_RH.txt\
 in.txt in_ID.txt iw.txt iw_IL.txt ja_JP_TRADITIONAL.txt\
 mo.txt no.txt no_NO.txt no_NO_NY.txt pa_IN.txt\
 pa_PK.txt sh.txt sh_BA.txt sh_CS.txt sh_YU.txt\
 shi_MA.txt sr_BA.txt sr_CS.txt sr_Cyrl_CS.txt sr_Cyrl_YU.txt\
 sr_Latn_CS.txt sr_Latn_YU.txt sr_ME.txt sr_RS.txt sr_XK.txt\
 sr_YU.txt th_TH_TRADITIONAL.txt tl.txt tl_PH.txt uz_AF.txt\
 uz_UZ.txt vai_LR.txt zh_CN.txt zh_HK.txt zh_MO.txt\
 zh_SG.txt zh_TW.txt


# All aliases (to not be included under 'installed'), but not including root.
GENRB_ALIAS_SOURCE = $(GENRB_SYNTHETIC_ALIAS)


# Ordinary resources
GENRB_SOURCE = af.txt af_NA.txt af_ZA.txt agq.txt\
 agq_CM.txt ak.txt ak_GH.txt am.txt am_ET.txt\
 ar.txt ar_001.txt ar_AE.txt ar_BH.txt ar_DJ.txt\
 ar_DZ.txt ar_EG.txt ar_EH.txt ar_ER.txt ar_IL.txt\
 ar_IQ.txt ar_JO.txt ar_KM.txt ar_KW.txt ar_LB.txt\
 ar_LY.txt ar_MA.txt ar_MR.txt ar_OM.txt ar_PS.txt\
 ar_QA.txt ar_SA.txt ar_SD.txt ar_SO.txt ar_SS.txt\
 ar_SY.txt ar_TD.txt ar_TN.txt ar_YE.txt as.txt\
 as_IN.txt asa.txt asa_TZ.txt ast.txt ast_ES.txt\
 az.txt az_Cyrl.txt az_Cyrl_AZ.txt az_Latn.txt az_Latn_AZ.txt\
 bas.txt bas_CM.txt be.txt be_BY.txt bem.txt\
 bem_ZM.txt bez.txt bez_TZ.txt bg.txt bg_BG.txt\
 bm.txt bm_ML.txt bn.txt bn_BD.txt bn_IN.txt\
 bo.txt bo_CN.txt bo_IN.txt br.txt br_FR.txt\
 brx.txt brx_IN.txt bs.txt bs_Cyrl.txt bs_Cyrl_BA.txt\
 bs_Latn.txt bs_Latn_BA.txt ca.txt ca_AD.txt ca_ES.txt\
 ca_FR.txt ca_IT.txt ce.txt ce_RU.txt cgg.txt\
 cgg_UG.txt chr.txt chr_US.txt ckb.txt ckb_IQ.txt\
 ckb_IR.txt cs.txt cs_CZ.txt cy.txt cy_GB.txt\
 da.txt da_DK.txt da_GL.txt dav.txt dav_KE.txt\
 de.txt de_AT.txt de_BE.txt de_CH.txt de_DE.txt\
 de_IT.txt de_LI.txt de_LU.txt dje.txt dje_NE.txt\
 dsb.txt dsb_DE.txt dua.txt dua_CM.txt dyo.txt\
 dyo_SN.txt dz.txt dz_BT.txt ebu.txt ebu_KE.txt\
 ee.txt ee_GH.txt ee_TG.txt el.txt el_CY.txt\
 el_GR.txt en.txt en_001.txt en_150.txt en_AG.txt\
 en_AI.txt en_AS.txt en_AT.txt en_AU.txt en_BB.txt\
 en_BE.txt en_BI.txt en_BM.txt en_BS.txt en_BW.txt\
 en_BZ.txt en_CA.txt en_CC.txt en_CH.txt en_CK.txt\
 en_CM.txt en_CX.txt en_CY.txt en_DE.txt en_DG.txt\
 en_DK.txt en_DM.txt en_ER.txt en_FI.txt en_FJ.txt\
 en_FK.txt en_FM.txt en_GB.txt en_GD.txt en_GG.txt\
 en_GH.txt en_GI.txt en_GM.txt en_GU.txt en_GY.txt\
 en_HK.txt en_IE.txt en_IL.txt en_IM.txt en_IN.txt\
 en_IO.txt en_JE.txt en_JM.txt en_KE.txt en_KI.txt\
 en_KN.txt en_KY.txt en_LC.txt en_LR.txt en_LS.txt\
 en_MG.txt en_MH.txt en_MO.txt en_MP.txt en_MS.txt\
 en_MT.txt en_MU.txt en_MW.txt en_MY.txt en_NA.txt\
 en_NF.txt en_NG.txt en_NL.txt en_NR.txt en_NU.txt\
 en_NZ.txt en_PG.txt en_PH.txt en_PK.txt en_PN.txt\
 en_PR.txt en_PW.txt en_RW.txt en_SB.txt en_SC.txt\
 en_SD.txt en_SE.txt en_SG.txt en_SH.txt en_SI.txt\
 en_SL.txt en_SS.txt en_SX.txt en_SZ.txt en_TC.txt\
 en_TK.txt en_TO.txt en_TT.txt en_TV.txt en_TZ.txt\
 en_UG.txt en_UM.txt en_US.txt en_US_POSIX.txt en_VC.txt\
 en_VG.txt en_VI.txt en_VU.txt en_WS.txt en_ZA.txt\
 en_ZM.txt en_ZW.txt eo.txt es.txt es_419.txt\
 es_AR.txt es_BO.txt es_BR.txt es_CL.txt es_CO.txt\
 es_CR.txt es_CU.txt es_DO.txt es_EA.txt es_EC.txt\
 es_ES.txt es_GQ.txt es_GT.txt es_HN.txt es_IC.txt\
 es_MX.txt es_NI.txt es_PA.txt es_PE.txt es_PH.txt\
 es_PR.txt es_PY.txt es_SV.txt es_US.txt es_UY.txt\
 es_VE.txt et.txt et_EE.txt eu.txt eu_ES.txt\
 ewo.txt ewo_CM.txt fa.txt fa_AF.txt fa_IR.txt\
 ff.txt ff_CM.txt ff_GN.txt ff_MR.txt ff_SN.txt\
 fi.txt fi_FI.txt fil.txt fil_PH.txt fo.txt\
 fo_DK.txt fo_FO.txt fr.txt fr_BE.txt fr_BF.txt\
 fr_BI.txt fr_BJ.txt fr_BL.txt fr_CA.txt fr_CD.txt\
 fr_CF.txt fr_CG.txt fr_CH.txt fr_CI.txt fr_CM.txt\
 fr_DJ.txt fr_DZ.txt fr_FR.txt fr_GA.txt fr_GF.txt\
 fr_GN.txt fr_GP.txt fr_GQ.txt fr_HT.txt fr_KM.txt\
 fr_LU.txt fr_MA.txt fr_MC.txt fr_MF.txt fr_MG.txt\
 fr_ML.txt fr_MQ.txt fr_MR.txt fr_MU.txt fr_NC.txt\
 fr_NE.txt fr_PF.txt fr_PM.txt fr_RE.txt fr_RW.txt\
 fr_SC.txt fr_SN.txt fr_SY.txt fr_TD.txt fr_TG.txt\
 fr_TN.txt fr_VU.txt fr_WF.txt fr_YT.txt fur.txt\
 fur_IT.txt fy.txt fy_NL.txt ga.txt ga_IE.txt\
 gd.txt gd_GB.txt gl.txt gl_ES.txt gsw.txt\
 gsw_CH.txt gsw_FR.txt gsw_LI.txt gu.txt gu_IN.txt\
 guz.txt guz_KE.txt gv.txt gv_IM.txt ha.txt\
 ha_GH.txt ha_NE.txt ha_NG.txt haw.txt haw_US.txt\
 he.txt he_IL.txt hi.txt hi_IN.txt hr.txt\
 hr_BA.txt hr_HR.txt hsb.txt hsb_DE.txt hu.txt\
 hu_HU.txt hy.txt hy_AM.txt id.txt id_ID.txt\
 ig.txt ig_NG.txt ii.txt ii_CN.txt is.txt\
 is_IS.txt it.txt it_CH.txt it_IT.txt it_SM.txt\
 ja.txt ja_JP.txt jgo.txt jgo_CM.txt jmc.txt\
 jmc_TZ.txt ka.txt ka_GE.txt kab.txt kab_DZ.txt\
 kam.txt kam_KE.txt kde.txt kde_TZ.txt kea.txt\
 kea_CV.txt khq.txt khq_ML.txt ki.txt ki_KE.txt\
 kk.txt kk_KZ.txt kkj.txt kkj_CM.txt kl.txt\
 kl_GL.txt kln.txt kln_KE.txt km.txt km_KH.txt\
 kn.txt kn_IN.txt ko.txt ko_KP.txt ko_KR.txt\
 kok.txt kok_IN.txt ks.txt ks_IN.txt ksb.txt\
 ksb_TZ.txt ksf.txt ksf_CM.txt ksh.txt ksh_DE.txt\
 kw.txt kw_GB.txt ky.txt ky_KG.txt lag.txt\
 lag_TZ.txt lb.txt lb_LU.txt lg.txt lg_UG.txt\
 lkt.txt lkt_US.txt ln.txt ln_AO.txt ln_CD.txt\
 ln_CF.txt ln_CG.txt lo.txt lo_LA.txt lrc.txt\
 lrc_IQ.txt lrc_IR.txt lt.txt lt_LT.txt lu.txt\
 lu_CD.txt luo.txt luo_KE.txt luy.txt luy_KE.txt\
 lv.txt lv_LV.txt mas.txt mas_KE.txt mas_TZ.txt\
 mer.txt mer_KE.txt mfe.txt mfe_MU.txt mg.txt\
 mg_MG.txt mgh.txt mgh_MZ.txt mgo.txt mgo_CM.txt\
 mk.txt mk_MK.txt ml.txt ml_IN.txt mn.txt\
 mn_MN.txt mr.txt mr_IN.txt ms.txt ms_BN.txt\
 ms_MY.txt ms_SG.txt mt.txt mt_MT.txt mua.txt\
 mua_CM.txt my.txt my_MM.txt mzn.txt mzn_IR.txt\
 naq.txt naq_NA.txt nb.txt nb_NO.txt nb_SJ.txt\
 nd.txt nd_ZW.txt nds.txt nds_DE.txt nds_NL.txt\
 ne.txt ne_IN.txt ne_NP.txt nl.txt nl_AW.txt\
 nl_BE.txt nl_BQ.txt nl_CW.txt nl_NL.txt nl_SR.txt\
 nl_SX.txt nmg.txt nmg_CM.txt nn.txt nn_NO.txt\
 nnh.txt nnh_CM.txt nus.txt nus_SS.txt nyn.txt\
 nyn_UG.txt om.txt om_ET.txt om_KE.txt or.txt\
 or_IN.txt os.txt os_GE.txt os_RU.txt pa.txt\
 pa_Arab.txt pa_Arab_PK.txt pa_Guru.txt pa_Guru_IN.txt pl.txt\
 pl_PL.txt ps.txt ps_AF.txt pt.txt pt_AO.txt\
 pt_BR.txt pt_CH.txt pt_CV.txt pt_GQ.txt pt_GW.txt\
 pt_LU.txt pt_MO.txt pt_MZ.txt pt_PT.txt pt_ST.txt\
 pt_TL.txt qu.txt qu_BO.txt qu_EC.txt qu_PE.txt\
 rm.txt rm_CH.txt rn.txt rn_BI.txt ro.txt\
 ro_MD.txt ro_RO.txt rof.txt rof_TZ.txt ru.txt\
 ru_BY.txt ru_KG.txt ru_KZ.txt ru_MD.txt ru_RU.txt\
 ru_UA.txt rw.txt rw_RW.txt rwk.txt rwk_TZ.txt\
 sah.txt sah_RU.txt saq.txt saq_KE.txt sbp.txt\
 sbp_TZ.txt se.txt se_FI.txt se_NO.txt se_SE.txt\
 seh.txt seh_MZ.txt ses.txt ses_ML.txt sg.txt\
 sg_CF.txt shi.txt shi_Latn.txt shi_Latn_MA.txt shi_Tfng.txt\
 shi_Tfng_MA.txt si.txt si_LK.txt sk.txt sk_SK.txt\
 sl.txt sl_SI.txt smn.txt smn_FI.txt sn.txt\
 sn_ZW.txt so.txt so_DJ.txt so_ET.txt so_KE.txt\
 so_SO.txt sq.txt sq_AL.txt sq_MK.txt sq_XK.txt\
 sr.txt sr_Cyrl.txt sr_Cyrl_BA.txt sr_Cyrl_ME.txt sr_Cyrl_RS.txt\
 sr_Cyrl_XK.txt sr_Latn.txt sr_Latn_BA.txt sr_Latn_ME.txt sr_Latn_RS.txt\
 sr_Latn_XK.txt sv.txt sv_AX.txt sv_FI.txt sv_SE.txt\
 sw.txt sw_CD.txt sw_KE.txt sw_TZ.txt sw_UG.txt\
 ta.txt ta_IN.txt ta_LK.txt ta_MY.txt ta_SG.txt\
 te.txt te_IN.txt teo.txt teo_KE.txt teo_UG.txt\
 th.txt th_TH.txt ti.txt ti_ER.txt ti_ET.txt\
 to.txt to_TO.txt tr.txt tr_CY.txt tr_TR.txt\
 twq.txt twq_NE.txt tzm.txt tzm_MA.txt ug.txt\
 ug_CN.txt uk.txt uk_UA.txt ur.txt ur_IN.txt\
 ur_PK.txt uz.txt uz_Arab.txt uz_Arab_AF.txt uz_Cyrl.txt\
 uz_Cyrl_UZ.txt uz_Latn.txt uz_Latn_UZ.txt vai.txt vai_Latn.txt\
 vai_Latn_LR.txt vai_Vaii.txt vai_Vaii_LR.txt vi.txt vi_VN.txt\
 vun.txt vun_TZ.txt wae.txt wae_CH.txt xog.txt\
 xog_UG.txt yav.txt yav_CM.txt yi.txt yi_001.txt\
 yo.txt yo_BJ.txt yo_NG.txt yue.txt yue_HK.txt\
 zgh.txt zgh_MA.txt zh.txt zh_Hans.txt zh_Hans_CN.txt\
 zh_Hans_HK.txt zh_Hans_MO.txt zh_Hans_SG.txt zh_Hant.txt zh_Hant_HK.txt\
 zh_Hant_MO.txt zh_Hant_TW.txt zu.txt zu_ZA.txt

