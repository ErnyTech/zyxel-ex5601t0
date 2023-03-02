#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zlog_api.h"
#if defined(ZYXEL_FEATURE_BITS)
#include "libzyutil_zyxelfeaturebits.h"
#endif

dal_param_t ADVANCE_param[]={
#ifdef CONFIG_TARGET_en75xx
	{"PhyRUSDS",		dalType_boolean,	0,	0,	NULL},
#else // if broadcom platform
	{"PhyRUS",			dalType_boolean,	0,	0,	NULL},
	{"PhyRDS", 			dalType_boolean,	0,	0,	NULL},
#endif
	{"Bitswap", 		dalType_boolean,	0,	0,	NULL},
	{"SRA", 			dalType_boolean,	0,	0,	NULL},
#ifdef  SUPPORT_DSL_BONDING
	{"DSLType", dalType_string,		0,	0,	NULL,	"auto|single|bonding",		NULL},
#endif
	{"EnableADSLPtm", 	dalType_boolean,	0,	0,	NULL},
	{"Gdmt", 	 		dalType_boolean,	0,	0,	NULL},
	{"Glite", 	 		dalType_boolean,	0,	0,	NULL},
	{"T1413", 	 		dalType_boolean,	0,	0,	NULL},
	{"ADSL2", 	 		dalType_boolean,	0,	0,	NULL},
	{"AnnexL", 	 		dalType_boolean,	0,	0,	NULL},
	{"AnnexB",			dalType_boolean,	0,	0,	NULL},
	{"ADSL2plus", 	 	dalType_boolean,	0,	0,	NULL},
	{"AnnexM", 	 		dalType_boolean,	0,	0,	NULL},
	{"VDSL2", 	 		dalType_boolean,	0,	0,	NULL},
#ifdef SUPPORT_VDSL2LR
	{"VDSL2LR", 	 	dalType_boolean,	0,	0,	NULL},
#endif
	{"Gfast",           dalType_boolean,    0,  0,  NULL},
	{"VDSL_8a", 	 	dalType_boolean,	0,	0,	NULL},
	{"VDSL_8b", 	 	dalType_boolean,	0,	0,	NULL},
	{"VDSL_8c", 	 	dalType_boolean,	0,	0,	NULL},
	{"VDSL_8d", 	 	dalType_boolean,	0,	0,	NULL},
	{"VDSL_12a", 	 	dalType_boolean,	0,	0,	NULL},
	{"VDSL_12b", 	 	dalType_boolean,	0,	0,	NULL},
	{"VDSL_17a", 	 	dalType_boolean,	0,	0,	NULL},
#ifdef ZYXEL_VDSL_Support_30A
	{"VDSL_30a", 	 	dalType_boolean,	0,	0,	NULL},
#endif
#ifdef ZYXEL_VDSL_Support_35B
	{"VDSL_35b", 	 	dalType_boolean,	0,	0,	NULL},
#endif
	{"VDSL_US0", 	 	dalType_boolean,	0,	0,	NULL},
	{NULL,					0,	0,	0,	NULL},
};

int kXdslFireDsSupported = 4194304; 			//10000000000000000000000
int kXdslFireUsSupported = 8388608; 			//100000000000000000000000
int kXdsl24kbyteInterleavingEnabled = 1048576;	//100000000000000000000
int zySupportDSL_Gfast = 0;

/* Init the object to false as default */
zcfgRet_t initAdslModulationCfg(struct json_object *JsonCfg)
{
	json_object_object_add(JsonCfg, "Gdmt", json_object_new_boolean(false));
	json_object_object_add(JsonCfg, "Glite", json_object_new_boolean(false));
	json_object_object_add(JsonCfg, "T1413", json_object_new_boolean(false));
	json_object_object_add(JsonCfg, "AnnexL", json_object_new_boolean(false));
	json_object_object_add(JsonCfg, "ADSL2", json_object_new_boolean(false));
	json_object_object_add(JsonCfg, "AnnexB", json_object_new_boolean(false));
	json_object_object_add(JsonCfg, "ADSL2plus", json_object_new_boolean(false));
	json_object_object_add(JsonCfg, "AnnexM", json_object_new_boolean(false));
	json_object_object_add(JsonCfg, "VDSL2", json_object_new_boolean(false));
#ifdef SUPPORT_VDSL2LR
	json_object_object_add(JsonCfg, "VDSL2LR", json_object_new_boolean(false));
#endif
	if (zySupportDSL_Gfast)
	{
		json_object_object_add(JsonCfg, "Gfast", json_object_new_boolean(false));
	}
	return ZCFG_SUCCESS;
}

/* Key2 is a type of configuration. Key3 is the data of configuration.
   Key3 is unique data and multiple key3 may have the same Key2 type.
   So modify the praseAdslModulationCfg back to  compare the third parameter only
*/
zcfgRet_t praseAdslModulationCfg(char *key1, char *key2, char *key3, struct json_object *JsonCfg){
	if(strstr(key1, key3) != NULL)
		json_object_object_add(JsonCfg, key2, json_object_new_boolean(true));
	else if(!strcmp(key1,"ADSL_Modulation_All,"))
		json_object_object_add(JsonCfg, key2, json_object_new_boolean(true));

	return ZCFG_SUCCESS;
}

zcfgRet_t dalcmdpraseAdslModCfg(char *key1, char *key2, char *key3, struct json_object *Jobj, struct json_object *JsonCfg){
	bool tmp = false;
	if(json_object_object_get(Jobj, key1))
		tmp = json_object_get_boolean(json_object_object_get(Jobj, key1));
	else
		tmp = json_object_get_boolean(json_object_object_get(JsonCfg, key1));
	if(tmp)
		strcat(key3, key2);
	return ZCFG_SUCCESS;
}		

void zcfgFeDalShowWanAdvance(struct json_object *Jarray){
	struct json_object *obj = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	printf("DSL Capacities: \n");
#ifdef CONFIG_TARGET_en75xx
	printf("%-20s %s \n","PhyR US/DS :", json_object_get_string(json_object_object_get(obj, "PhyrUSDS")));
#else  // if broadcom
	printf("%-20s %s \n","PhyR US :", json_object_get_string(json_object_object_get(obj, "PhyrUS")));
	printf("%-20s %s \n","PhyR DS :", json_object_get_string(json_object_object_get(obj, "PhyrDS")));
#endif
	printf("%-20s %s \n","Bitswap :", json_object_get_string(json_object_object_get(obj, "Bitswap")));
	printf("%-20s %s \n","SRA :", json_object_get_string(json_object_object_get(obj, "SRA")));
#ifdef  SUPPORT_DSL_BONDING
	printf("DSL Line Mode: \n");
	printf("\nState (System will reboot once the config is changed!):	%s\n",json_object_get_string(json_object_object_get(obj, "DSLType")));
#endif
	printf("\nDSL Modulation:\n");
	printf("%-20s %s \n","PTM over ADSL :", json_object_get_string(json_object_object_get(obj, "EnableADSLPtm")));
	printf("%-20s %s \n","G.dmt :", json_object_get_string(json_object_object_get(obj, "Gdmt")));
	printf("%-20s %s \n","G.lite :", json_object_get_string(json_object_object_get(obj, "Glite")));
	printf("%-20s %s \n","T1.413 :", json_object_get_string(json_object_object_get(obj, "T1413")));
	printf("%-20s %s \n","ADSL2 :", json_object_get_string(json_object_object_get(obj, "ADSL2")));
	printf("%-20s %s \n","AnnexB :", json_object_get_string(json_object_object_get(obj, "AnnexB")));	//EN75XX_DSL_AnnexB
	printf("%-20s %s \n","Annex L :", json_object_get_string(json_object_object_get(obj, "AnnexL")));
	printf("%-20s %s \n","ADSL2+ :", json_object_get_string(json_object_object_get(obj, "ADSL2plus")));
	printf("%-20s %s \n","Annex M :", json_object_get_string(json_object_object_get(obj, "AnnexM")));
	printf("%-20s %s \n","VDSL2 :", json_object_get_string(json_object_object_get(obj, "VDSL2")));
#ifdef SUPPORT_VDSL2LR
	printf("%-20s %s \n","VDSL2LR :", json_object_get_string(json_object_object_get(obj, "VDSL2LR")));
#endif
	if (zySupportDSL_Gfast)
	{
	    printf("%-20s %s \n","G.fast :", json_object_get_string(json_object_object_get(obj, "Gfast")));
	}
	printf("\nVDSL Profile:\n");
	printf("%-20s %s \n","8a Enable :", json_object_get_string(json_object_object_get(obj, "VDSL_8a")));
	printf("%-20s %s \n","8b Enable :", json_object_get_string(json_object_object_get(obj, "VDSL_8b")));
	printf("%-20s %s \n","8c Enable :", json_object_get_string(json_object_object_get(obj, "VDSL_8c")));
	printf("%-20s %s \n","8d Enable :", json_object_get_string(json_object_object_get(obj, "VDSL_8d")));
	printf("%-20s %s \n","12a Enable :", json_object_get_string(json_object_object_get(obj, "VDSL_12a")));
	printf("%-20s %s \n","12b Enable :", json_object_get_string(json_object_object_get(obj, "VDSL_12b")));
	printf("%-20s %s \n","17a Enable :", json_object_get_string(json_object_object_get(obj, "VDSL_17a")));
#ifdef ZYXEL_VDSL_Support_30A
	printf("%-20s %s \n","30a Enable :", json_object_get_string(json_object_object_get(obj, "VDSL_30a")));
#endif
#ifdef ZYXEL_VDSL_Support_35B
	printf("%-20s %s \n","35b Enable :", json_object_get_string(json_object_object_get(obj, "VDSL_35b")));
#endif
	printf("%-20s %s \n","US0 :", json_object_get_string(json_object_object_get(obj, "VDSL_US0")));
	
}


zcfgRet_t zcfgFeDal_Advance_Edit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	struct json_object *advPtmLinkObj = NULL;
	objIndex_t advPtmLinkIid = {0};
#endif
	struct json_object *advDSLLineObj = NULL;
	struct json_object *jsonadslmodcfg = NULL;
	objIndex_t advDSLLineIid = {0};
	bool enableADSLPtm = false;
	int dslPhyCfg2Mask = 0;
	int dslPhyCfg2Value = 0;
	char *adslmdocfg = NULL;
	char tmpAdslModCfg[256] = {0};
    char *AnnexA_B = NULL;
	bool phyrUS = false;
	bool phyrDS = false;
#ifdef CONFIG_TARGET_en75xx
	bool phyrUSDS = false;
#endif
	bool enblPhyrUS = false;
	bool enblPhyrDS = false;
	bool bitswap = false;
	bool SRA = false;
#ifdef  SUPPORT_DSL_BONDING
	const char *DSLType = NULL;
#endif
    const char *xDSLcfgFromGui = NULL;
	char adslModulationCfg[512] = {0};
	bool AnnexM = false;
	bool VDSL_8a = false;
	bool VDSL_8b = false;
	bool VDSL_8c = false;
	bool VDSL_8d = false;
	bool VDSL_12a = false;
	bool VDSL_12b = false;
	bool VDSL_17a = false;
#ifdef ZYXEL_VDSL_Support_30A
	bool VDSL_30a = false;
#endif
#ifdef ZYXEL_VDSL_Support_35B
	bool VDSL_35b = false;
#endif
	bool VDSL_US0_8a = false;
	int len = 0;
	struct json_object *paramJobj = NULL;
	paramJobj = json_object_new_object();

	IID_INIT(advDSLLineIid);

	if(Jgetb(Jobj,"AnnexL") || Jgetb(Jobj,"AnnexB"))
		Jaddb(Jobj, "ADSL2", true);

	if(Jgetb(Jobj,"AnnexM") && !Jgetb(Jobj,"ADSL2") && !Jgetb(Jobj,"ADSL2plus"))
		Jaddb(Jobj, "ADSL2", true);

#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	IID_INIT(advPtmLinkIid);
	//PTM Link
	while(zcfgFeObjJsonGetNext(RDM_OID_PTM_LINK, &advPtmLinkIid, &advPtmLinkObj)== ZCFG_SUCCESS){
		enableADSLPtm = json_object_get_boolean(json_object_object_get(Jobj, "EnableADSLPtm"));
		if(json_object_object_get(Jobj, "EnableADSLPtm"))
		json_object_object_add(advPtmLinkObj, "X_ZYXEL_EnableADSLPtm", json_object_new_boolean(enableADSLPtm));
		zcfgFeObjJsonBlockedSet(RDM_OID_PTM_LINK, &advPtmLinkIid, advPtmLinkObj, NULL);
		zcfgFeJsonObjFree(advPtmLinkObj);
	}
#endif

	//DSL Line
	while(zcfgFeObjJsonGetNext(RDM_OID_DSL_LINE, &advDSLLineIid, &advDSLLineObj) == ZCFG_SUCCESS){
		if(advDSLLineObj){
			memset(adslModulationCfg,0,sizeof(adslModulationCfg));
#ifdef CONFIG_TARGET_en75xx
			phyrUSDS = json_object_get_int(json_object_object_get(Jobj, "PhyRUSDS"));
			json_object_object_add(advDSLLineObj, "X_ZYXEL_DslPhyCfg2Value", json_object_new_int(phyrUSDS));
#else // if broadcom
			dslPhyCfg2Mask = json_object_get_int(json_object_object_get(advDSLLineObj, "X_ZYXEL_DslPhyCfg2Mask"));
			dslPhyCfg2Value = json_object_get_int(json_object_object_get(advDSLLineObj, "X_ZYXEL_DslPhyCfg2Value"));
			if((dslPhyCfg2Mask & kXdslFireUsSupported) && (dslPhyCfg2Value & kXdslFireUsSupported)){
				enblPhyrUS = true;
				json_object_object_add(paramJobj, "PhyRUS", json_object_new_boolean(true));
			}else
				json_object_object_add(paramJobj, "PhyRUS", json_object_new_boolean(false));
			if((dslPhyCfg2Mask & kXdslFireDsSupported) && (dslPhyCfg2Value & kXdslFireDsSupported)){
				enblPhyrDS = true;
				json_object_object_add(paramJobj, "PhyRDS", json_object_new_boolean(true));
			}else
				json_object_object_add(paramJobj, "PhyRDS", json_object_new_boolean(false));

			
			if(json_object_object_get(Jobj, "PhyRUS")){
				phyrUS = json_object_get_boolean(json_object_object_get(Jobj, "PhyRUS"));
			}
			else{
				phyrUS = json_object_get_boolean(json_object_object_get(paramJobj, "PhyRUS"));
			}

			if(json_object_object_get(Jobj, "PhyRDS")){
				phyrDS = json_object_get_boolean(json_object_object_get(Jobj, "PhyRDS"));
			}
			else{
				phyrDS = json_object_get_boolean(json_object_object_get(paramJobj, "PhyRDS"));
			}
			
			if(enblPhyrUS != phyrUS){
				dslPhyCfg2Mask |= kXdslFireUsSupported;
				if(phyrUS)
					dslPhyCfg2Value |= kXdslFireUsSupported;
				else
					dslPhyCfg2Value &= ~kXdslFireUsSupported;
			}
			if(enblPhyrDS != phyrDS){
				dslPhyCfg2Mask |= kXdslFireDsSupported;
				if(phyrDS)
					dslPhyCfg2Value |= kXdslFireDsSupported;
				else
					dslPhyCfg2Value &= ~kXdslFireDsSupported;
			}
			json_object_object_add(advDSLLineObj, "X_ZYXEL_DslPhyCfg2Mask", json_object_new_int(dslPhyCfg2Mask));
			json_object_object_add(advDSLLineObj, "X_ZYXEL_DslPhyCfg2Value", json_object_new_int(dslPhyCfg2Value));
#endif
			bitswap = json_object_get_boolean(json_object_object_get(Jobj, "Bitswap"));
			if(bitswap && json_object_object_get(Jobj, "Bitswap"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_Bitswap", json_object_new_string("On"));
			else if(!bitswap && json_object_object_get(Jobj, "Bitswap"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_Bitswap", json_object_new_string("Off"));
			SRA = json_object_get_boolean(json_object_object_get(Jobj, "SRA"));
			if(SRA && json_object_object_get(Jobj, "SRA"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_SRA", json_object_new_string("On"));
			else if(!SRA && json_object_object_get(Jobj, "SRA"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_SRA", json_object_new_string("Off"));
#ifdef  SUPPORT_DSL_BONDING
			DSLType = json_object_get_string(json_object_object_get(Jobj, "DSLType"));
			json_object_object_add(advDSLLineObj, "X_ZYXEL_DSLType", json_object_new_string(DSLType));
#endif
			jsonadslmodcfg = json_object_new_object();
			adslmdocfg = (char *)json_object_get_string(json_object_object_get(advDSLLineObj, "X_ZYXEL_AdslModulationCfg"));
			strcpy(tmpAdslModCfg, adslmdocfg);
			len = strlen(tmpAdslModCfg);
			if(tmpAdslModCfg[len-1] != ',')
				strcat(tmpAdslModCfg, ",");
			initAdslModulationCfg(jsonadslmodcfg);
			/* Seperate the difference name because the praseAdslModulationCfg change back to compare the third parameter only.*/
			praseAdslModulationCfg(tmpAdslModCfg, "Gdmt", "ADSL_G.dmt,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "Gdmt", "Gdmt,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "Glite", "ADSL_G.lite,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "Glite", "Glite,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "T1413", "ADSL_ANSI_T1.413,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "T1413", "T1413,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "AnnexL", "ADSL_re-adsl,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "AnnexL", "AnnexL,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "ADSL2", "ADSL_G.dmt.bis,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "ADSL2", "ADSL2,", jsonadslmodcfg);
            if( (AnnexA_B = (char *)json_object_get_string(json_object_object_get(advDSLLineObj, "X_ZYXEL_ADSL2_AnnexA_B"))) != NULL )
            {
                if(strcmp(AnnexA_B, "AnnexB") == 0)
                {
                    praseAdslModulationCfg(tmpAdslModCfg, "AnnexB", "ADSL_over_ISDN,", jsonadslmodcfg);	//EN75XX_DSL_AnnexB
                    praseAdslModulationCfg(tmpAdslModCfg, "AnnexB", "AnnexB,", jsonadslmodcfg);	//EN75XX_DSL_AnnexB
                }
            }
			praseAdslModulationCfg(tmpAdslModCfg, "ADSL2plus", "ADSL_2plus,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "AnnexM", "AnnexM,", jsonadslmodcfg);
			praseAdslModulationCfg(tmpAdslModCfg, "VDSL2", "VDSL2,", jsonadslmodcfg);
#ifdef SUPPORT_VDSL2LR
			praseAdslModulationCfg(tmpAdslModCfg, "VDSL2LR", "VDSL2LR,", jsonadslmodcfg);
#endif
			if (zySupportDSL_Gfast)
			{
			    praseAdslModulationCfg(tmpAdslModCfg, "Gfast", "G.FAST,", jsonadslmodcfg);
				praseAdslModulationCfg(tmpAdslModCfg, "Gfast", "Gfast,", jsonadslmodcfg);
			}

			dalcmdpraseAdslModCfg("Gdmt", "ADSL_G.dmt,", adslModulationCfg, Jobj, jsonadslmodcfg);
			dalcmdpraseAdslModCfg("Glite", "ADSL_G.lite,", adslModulationCfg, Jobj, jsonadslmodcfg);
			dalcmdpraseAdslModCfg("T1413", "ADSL_ANSI_T1.413,", adslModulationCfg, Jobj, jsonadslmodcfg);
			dalcmdpraseAdslModCfg("AnnexL", "ADSL_re-adsl,", adslModulationCfg, Jobj, jsonadslmodcfg);
			dalcmdpraseAdslModCfg("ADSL2", "ADSL_G.dmt.bis,", adslModulationCfg, Jobj, jsonadslmodcfg);
			if( (AnnexA_B = (char *)json_object_get_string(json_object_object_get(advDSLLineObj, "X_ZYXEL_ADSL2_AnnexA_B"))) != NULL ) {
				if(strcmp(AnnexA_B, "AnnexB") == 0)
				{
					dalcmdpraseAdslModCfg("AnnexB", "ADSL_over_ISDN,", adslModulationCfg, Jobj, jsonadslmodcfg);	//EN75XX_DSL_AnnexB
				}
			}
			dalcmdpraseAdslModCfg("ADSL2plus", "ADSL_2plus,", adslModulationCfg, Jobj, jsonadslmodcfg);
			dalcmdpraseAdslModCfg("AnnexM", "AnnexM,", adslModulationCfg, Jobj, jsonadslmodcfg);
			dalcmdpraseAdslModCfg("VDSL2", "VDSL2,", adslModulationCfg, Jobj, jsonadslmodcfg);
#ifdef SUPPORT_VDSL2LR
			dalcmdpraseAdslModCfg("VDSL2LR", "VDSL2LR,", adslModulationCfg, Jobj, jsonadslmodcfg);
#endif
			if (zySupportDSL_Gfast)
			{
			    dalcmdpraseAdslModCfg("Gfast", "G.FAST,", adslModulationCfg, Jobj, jsonadslmodcfg);
			}

			if(adslModulationCfg != NULL){
				if(strcmp(adslModulationCfg, "")){
					len = strlen(adslModulationCfg);
					if(adslModulationCfg[len-1] == ',')
						adslModulationCfg[len-1] = '\0';
				}
				else{
					strcpy(replyMsg, "At least set one ADSL modulation.");
					Jadds(Jobj, "__multi_lang_replyMsg", "zylang.Broadband.Advanced.dbg_need_mod");
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			else{
				dbg_printf("adslModulationCfg is NULL");
			}
			xDSLcfgFromGui = json_object_get_string(json_object_object_get(Jobj, "X_ZYXEL_AdslModulationCfg"));

			if(xDSLcfgFromGui != NULL)
			{
				if(!strcmp(xDSLcfgFromGui,"ADSL_Modulation_All,") || !strcmp(xDSLcfgFromGui,"ADSL_Modulation_All")){
					/*This is by default value. if all of Modulations were active.*/
					json_object_object_add(advDSLLineObj, "X_ZYXEL_AdslModulationCfg", json_object_new_string("ADSL_Modulation_All"));
				}else{
					json_object_object_add(advDSLLineObj, "X_ZYXEL_AdslModulationCfg", json_object_new_string(adslModulationCfg));
				}
			}
			else //20200529 IChiaHuang Bug #129025: [GUI] Go Network setting > broadband > Advanced, DSL Modulation is not disable.
			{
				json_object_object_add(advDSLLineObj, "X_ZYXEL_AdslModulationCfg", json_object_new_string(adslModulationCfg));
			}

			if(json_object_get_boolean(json_object_object_get(Jobj, "AnnexB")))
            {
                json_object_object_add(advDSLLineObj, "X_ZYXEL_ADSL2_AnnexA_B", json_object_new_string("AnnexB"));
            }
			AnnexM = json_object_get_boolean(json_object_object_get(Jobj, "AnnexM"));
			if(json_object_object_get(Jobj, "AnnexM"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_ADSL2_AnnexM", json_object_new_boolean(AnnexM));

			VDSL_8a = json_object_get_boolean(json_object_object_get(Jobj, "VDSL_8a"));
			if(json_object_object_get(Jobj, "VDSL_8a"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_VDSL_8a", json_object_new_boolean(VDSL_8a));

			VDSL_8b = json_object_get_boolean(json_object_object_get(Jobj, "VDSL_8b"));
			if(json_object_object_get(Jobj, "VDSL_8b"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_VDSL_8b", json_object_new_boolean(VDSL_8b));

			VDSL_8c = json_object_get_boolean(json_object_object_get(Jobj, "VDSL_8c"));
			if(json_object_object_get(Jobj, "VDSL_8c"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_VDSL_8c", json_object_new_boolean(VDSL_8c));

			VDSL_8d = json_object_get_boolean(json_object_object_get(Jobj, "VDSL_8d"));
			if(json_object_object_get(Jobj, "VDSL_8d"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_VDSL_8d", json_object_new_boolean(VDSL_8d));

			VDSL_12a = json_object_get_boolean(json_object_object_get(Jobj, "VDSL_12a"));
			if(json_object_object_get(Jobj, "VDSL_12a"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_VDSL_12a", json_object_new_boolean(VDSL_12a));

			VDSL_12b = json_object_get_boolean(json_object_object_get(Jobj, "VDSL_12b"));
			if(json_object_object_get(Jobj, "VDSL_12b"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_VDSL_12b", json_object_new_boolean(VDSL_12b));

			VDSL_17a = json_object_get_boolean(json_object_object_get(Jobj, "VDSL_17a"));
			if(json_object_object_get(Jobj, "VDSL_17a"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_VDSL_17a", json_object_new_boolean(VDSL_17a));
#ifdef ZYXEL_VDSL_Support_30A
			VDSL_30a = json_object_get_boolean(json_object_object_get(Jobj, "VDSL_30a"));
			if(json_object_object_get(Jobj, "VDSL_30a"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_VDSL_30a", json_object_new_boolean(VDSL_30a));
#endif
#ifdef ZYXEL_VDSL_Support_35B
			VDSL_35b = json_object_get_boolean(json_object_object_get(Jobj, "VDSL_35b"));
			if(json_object_object_get(Jobj, "VDSL_35b"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_VDSL_35b", json_object_new_boolean(VDSL_35b));
#endif
			VDSL_US0_8a = json_object_get_boolean(json_object_object_get(Jobj, "VDSL_US0"));
			if(json_object_object_get(Jobj, "VDSL_US0"))
				json_object_object_add(advDSLLineObj, "X_ZYXEL_VDSL_US0_8a", json_object_new_boolean(VDSL_US0_8a));

			zcfgFeObjJsonSet(RDM_OID_DSL_LINE, &advDSLLineIid, advDSLLineObj, NULL);
		}
	    zcfgFeJsonObjFree(advDSLLineObj);
    }
	
	return ret;
}

zcfgRet_t zcfgFeDal_Advance_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	struct json_object *advPtmLinkObj = NULL;
	objIndex_t advPtmLinkIid = {0};
#endif
	struct json_object *advDSLLineObj = NULL;
	struct json_object *paramJobj = NULL;
	objIndex_t advDSLLineIid = {0};
	char *AdslModCfg = NULL;
	int DslPhyCfgMask = 0;
	int DslPhyCfgValue = 0;
	const char *Bitswap = NULL;
	const char *SRA = NULL;
    char *AnnexA_B = NULL;
	int len = 0;
	char tmpAdslModCfg[256] = {0};
	paramJobj = json_object_new_object();

#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	advPtmLinkIid.level = 1;
	advPtmLinkIid.idx[0] = 1;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PTM_LINK, &advPtmLinkIid, &advPtmLinkObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "EnableADSLPtm", JSON_OBJ_COPY(json_object_object_get(advPtmLinkObj, "X_ZYXEL_EnableADSLPtm")));
		zcfgFeJsonObjFree(advPtmLinkObj);
	}
#endif

	advDSLLineIid.level = 1;
	advDSLLineIid.idx[0] = 1;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DSL_LINE, &advDSLLineIid, &advDSLLineObj) == ZCFG_SUCCESS){
#ifdef CONFIG_TARGET_en75xx
		DslPhyCfgValue = json_object_get_int(json_object_object_get(advDSLLineObj, "X_ZYXEL_DslPhyCfg2Value"));
		if(DslPhyCfgValue)
			json_object_object_add(paramJobj, "PhyRUSDS", json_object_new_boolean(true));
		else
			json_object_object_add(paramJobj, "PhyRUSDS", json_object_new_boolean(false));
#else // if broadcom
		DslPhyCfgMask = json_object_get_int(json_object_object_get(advDSLLineObj, "X_ZYXEL_DslPhyCfg2Mask"));
		DslPhyCfgValue = json_object_get_int(json_object_object_get(advDSLLineObj, "X_ZYXEL_DslPhyCfg2Value"));
		if((DslPhyCfgMask & kXdslFireUsSupported) && (DslPhyCfgValue & kXdslFireUsSupported))
			json_object_object_add(paramJobj, "PhyRUS", json_object_new_boolean(true));
		else
			json_object_object_add(paramJobj, "PhyRUS", json_object_new_boolean(false));
		if((DslPhyCfgMask & kXdslFireDsSupported) && (DslPhyCfgValue & kXdslFireDsSupported))
			json_object_object_add(paramJobj, "PhyRDS", json_object_new_boolean(true));
		else
			json_object_object_add(paramJobj, "PhyRDS", json_object_new_boolean(false));
#endif
		Bitswap = json_object_get_string(json_object_object_get(advDSLLineObj, "X_ZYXEL_Bitswap"));
		SRA = json_object_get_string(json_object_object_get(advDSLLineObj, "X_ZYXEL_SRA"));
		if(!strcmp(Bitswap,"On"))
			json_object_object_add(paramJobj, "Bitswap", json_object_new_boolean(true));
		else if(!strcmp(Bitswap,"Off"))
			json_object_object_add(paramJobj, "Bitswap", json_object_new_boolean(false));
		if(!strcmp(SRA,"On"))
			json_object_object_add(paramJobj, "SRA", json_object_new_boolean(true));
		else if(!strcmp(SRA,"Off"))
			json_object_object_add(paramJobj, "SRA", json_object_new_boolean(false));
#ifdef  SUPPORT_DSL_BONDING
		json_object_object_add(paramJobj, "DSLType", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_DSLType")));
#endif
		AdslModCfg = (char *)json_object_get_string(json_object_object_get(advDSLLineObj, "X_ZYXEL_AdslModulationCfg"));
		strcpy(tmpAdslModCfg, AdslModCfg);
		len = strlen(tmpAdslModCfg);
		if(tmpAdslModCfg[len-1] != ','){
			strcat(tmpAdslModCfg, ",");
		}
		if(strcmp(tmpAdslModCfg,"ADSL_Modulation_All,")){
			initAdslModulationCfg(paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "Gdmt", "ADSL_G.dmt,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "Gdmt", "Gdmt,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "Glite", "ADSL_G.lite,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "Glite", "Glite,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "T1413", "ADSL_ANSI_T1.413,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "T1413", "T1413,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "AnnexL", "ADSL_re-adsl,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "AnnexL", "AnnexL,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "ADSL2", "ADSL_G.dmt.bis,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "ADSL2", "ADSL2,", paramJobj);
            if( (AnnexA_B = (char *)json_object_get_string(json_object_object_get(advDSLLineObj, "X_ZYXEL_ADSL2_AnnexA_B"))) != NULL )
            {
                if(strcmp(AnnexA_B, "AnnexB") == 0)
                {
                    praseAdslModulationCfg(tmpAdslModCfg, "AnnexB", "ADSL_over_ISDN,", paramJobj);	//EN75XX_DSL_AnnexB
                    praseAdslModulationCfg(tmpAdslModCfg, "AnnexB", "AnnexB,", paramJobj);	//EN75XX_DSL_AnnexB
                }
            }
			
			praseAdslModulationCfg(tmpAdslModCfg, "ADSL2plus", "ADSL_2plus,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "AnnexM", "AnnexM,", paramJobj);
			praseAdslModulationCfg(tmpAdslModCfg, "VDSL2", "VDSL2,", paramJobj);
#ifdef SUPPORT_VDSL2LR
			praseAdslModulationCfg(tmpAdslModCfg, "VDSL2LR", "VDSL2LR,", paramJobj);
#endif
			if (zySupportDSL_Gfast)
			{
			    praseAdslModulationCfg(tmpAdslModCfg, "Gfast", "G.FAST,", paramJobj);
				praseAdslModulationCfg(tmpAdslModCfg, "Gfast", "Gfast,", paramJobj);
			}
		}
		else{
			json_object_object_add(paramJobj, "Gdmt", json_object_new_boolean(true));
			json_object_object_add(paramJobj, "Glite", json_object_new_boolean(true));
			json_object_object_add(paramJobj, "T1413", json_object_new_boolean(true));
			json_object_object_add(paramJobj, "AnnexL", json_object_new_boolean(true));
			json_object_object_add(paramJobj, "ADSL2", json_object_new_boolean(true));
            if( (AnnexA_B = (char *)json_object_get_string(json_object_object_get(advDSLLineObj, "X_ZYXEL_ADSL2_AnnexA_B"))) != NULL )
            {
                if(strcmp(AnnexA_B, "AnnexB") == 0)
                {
                    json_object_object_add(paramJobj, "AnnexB", json_object_new_boolean(true));
                }
                else
                {
                    json_object_object_add(paramJobj, "AnnexB", json_object_new_boolean(false));
                }
            }
			json_object_object_add(paramJobj, "ADSL2plus", json_object_new_boolean(true));
			json_object_object_add(paramJobj, "AnnexM", json_object_new_boolean(true));
			json_object_object_add(paramJobj, "VDSL2", json_object_new_boolean(true));
#ifdef SUPPORT_VDSL2LR
			json_object_object_add(paramJobj, "VDSL2LR", json_object_new_boolean(true));
#endif

			if (zySupportDSL_Gfast)
			{
			    json_object_object_add(paramJobj, "Gfast", json_object_new_boolean(true));
			}
		}
		json_object_object_add(paramJobj, "VDSL_8a", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_VDSL_8a")));
		json_object_object_add(paramJobj, "VDSL_8b", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_VDSL_8b")));
		json_object_object_add(paramJobj, "VDSL_8c", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_VDSL_8c")));
		json_object_object_add(paramJobj, "VDSL_8d", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_VDSL_8d")));
		json_object_object_add(paramJobj, "VDSL_12a", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_VDSL_12a")));
		json_object_object_add(paramJobj, "VDSL_12b", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_VDSL_12b")));
		json_object_object_add(paramJobj, "VDSL_17a", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_VDSL_17a")));
		json_object_object_add(paramJobj, "VDSL_30a", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_VDSL_30a")));
		json_object_object_add(paramJobj, "VDSL_35b", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_VDSL_35b")));
		json_object_object_add(paramJobj, "VDSL_US0", JSON_OBJ_COPY(json_object_object_get(advDSLLineObj, "X_ZYXEL_VDSL_US0_8a")));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(advDSLLineObj);
	}

	return ret;
}


zcfgRet_t zcfgFeDalWanAdvance(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

#if defined(ZYXEL_FEATURE_BITS)
	if (libzyutil_zyxelfeaturebits_vdslGfastGet(&zySupportDSL_Gfast) !=  ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Fail to get feature bits");
    }
#endif

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDal_Advance_Edit(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_Advance_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

