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

//#define JSON_OBJ_COPY(json_object) json_tokener_parse(json_object_to_json_string(json_object))

dal_param_t WIFI_OTHERS_param[]={
	//WiFi Radio
	{"Band",						dalType_string,		0,	0,	NULL,	"2.4GHz|5GHz",	dal_Edit},
	{"wlRtsThrshld",				dalType_int,		0,	2347,	NULL,	NULL,	hidden_extender_dalcmd_param},
	{"wlFrgThrshld",				dalType_int, 		256,	2346,	NULL,	NULL,	hidden_extender_dalcmd_param},
	{"AutoChannelTimer",			dalType_int,		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"wlCsScanTimer",				dalType_int,		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"wlTxPwrPcnt",					dalType_int,		0,	0,	NULL,	"20|40|60|80|100",	0},
	{"wlBcnIntvl",					dalType_int,		50,	1000,	NULL,	NULL,	0},
	{"wlDtmIntvl",					dalType_int,		1,	255,	NULL,	NULL,	0},
	{"wlMode",						dalType_wlanMode,	0,	0,	NULL,	NULL,	0},
	{"wlProtect",					dalType_string,		0,	0,	NULL,	"auto|off",	hidden_extender_dalcmd_param},
	//{"wlRifsAdvert",				dalType_string,		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"wlPreamble",					dalType_string, 	0,	0,	NULL,	"long|short",	dalcmd_Forbid},
	{"wlRxChainPwrSave",			dalType_int, 		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	//{"wlObssCox",					dalType_boolean, 	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"wlFrameburst",				dalType_boolean,	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"wlRegMode",					dalType_int,		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"wlDfsPreIsm",					dalType_int, 		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"wlTpcDb",						dalType_int, 		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"wlDfsPostIsm",				dalType_int, 		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"OperatingChannelBandwidth",	dalType_string, 	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	//WiFi AP Sec
	{"wlPmf",						dalType_string, 	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	//WiFi AP WPS
	{"wps_v2",						dalType_boolean, 	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"atf",							dalType_boolean, 	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"InActiveChan",				dalType_string, 	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"singleSKU", 					dalType_string, 	0,	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"acs_fcs_mode",				dalType_boolean, 	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"cert",						dalType_int,		0,	0,	NULL,	NULL,	0},
	{NULL,		0,	0,	0,	NULL,	NULL,	0},
};

struct json_object *wifiRadioOthersObj = NULL;
struct json_object *wifiApWpsOthersObj = NULL;
struct json_object *wifiSecOthersObj = NULL;

objIndex_t wifiRadioOthersIid = {0};
objIndex_t wifiApWpsOthersIid = {0};
objIndex_t wifiSecOthersIid = {0};

//compare
unsigned int wlRtsThrshld_ori;
unsigned int wlFrgThrshld_ori;
unsigned int wlTxPwrPcnt_ori;
unsigned int wlBcnIntvl_ori;
unsigned int wlDtmIntvl_ori;
const char* wlMode_ori;
const char* wlProtect_ori;
const char* wlPmf_ori;
bool atf_ori;
bool acs_fcs_mode_ori;
const char* wlPreamble_ori;
const char* InActiveChan_ori;
//WiFi Radio
unsigned int wlRtsThrshld;
unsigned int wlFrgThrshld;
unsigned int AutoChannelTimer;
unsigned int wlCsScanTimer;
unsigned int wlTxPwrPcnt;
unsigned int wlBcnIntvl;
unsigned int wlDtmIntvl;
const char* wlMode;
const char* wlProtect;
//const char* wlRifsAdvert;
const char* wlPreamble;
int wlRxChainPwrSave;
//bool wlObssCox;
bool wlFrameburst;
int wlRegMode;
int wlDfsPreIsm;
int wlTpcDb;
int wlDfsPostIsm;
const char* OperatingChannelBandwidth;
//WiFi AP Sec
const char* wlPmf;
//WiFi AP WPS
bool wps_v2;
bool atf;
bool acs_fcs_mode;
const char* InActiveChan = NULL;
const char* singleSKU = NULL;

void zcfgFeDalShowWifiOthers(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj != NULL){
			printf("%-20s : %s\n","Band",json_object_get_string(json_object_object_get(obj, "Band")));
			printf("-----------------------------------------------\n");
#ifndef ZYXEL_OPAL_EXTENDER
			printf("%-30s : %s\n","RTS/CTS Threshold",json_object_get_string(json_object_object_get(obj, "wlRtsThrshld")));
			printf("%-30s : %s\n","Fragmentation Threshold",json_object_get_string(json_object_object_get(obj, "wlFrgThrshld")));
#endif
			printf("%-30s : %s\n","Beacon Interval",json_object_get_string(json_object_object_get(obj, "wlBcnIntvl")));
			printf("%-30s : %s\n","DTIM Interval",json_object_get_string(json_object_object_get(obj, "wlDtmIntvl")));
			printf("%-30s : %s\n","ACS Periodl",json_object_get_string(json_object_object_get(obj, "AutoChannelTimer")));
			printf("%-30s : %s\n","802.11 Mode",json_object_get_string(json_object_object_get(obj, "wlMode")));
			printf("%-30s : %s\n","Output Power",json_object_get_string(json_object_object_get(obj, "wlTxPwrPcnt")));
#ifndef ZYXEL_OPAL_EXTENDER
			printf("%-30s : %s\n","802.11 Protection",json_object_get_string(json_object_object_get(obj, "wlProtect")));
			printf("%-30s : %s\n","Preamble",json_object_get_string(json_object_object_get(obj, "wlPreamble")));
#endif
			printf("%-30s : %s\n","Protected Management Frames",json_object_get_string(json_object_object_get(obj, "wlPmf")));
			printf("\n");
		}
	}
	
}

void prepareJObj(struct json_object *Jobj){
	int ssidIndex=0;
	struct json_object *SsidObj = NULL;
	struct json_object *RadioObj = NULL;
	objIndex_t SsidIid={0};	
	objIndex_t RadioIid={0};	
	const char *tmpLowerLayer = NULL;
	uint8_t tmpRadioIndex;

	if(json_object_object_get(Jobj, "Band")){
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_RADIO, &RadioIid, &RadioObj) == ZCFG_SUCCESS){
			if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "Band")),json_object_get_string(json_object_object_get(RadioObj, "OperatingFrequencyBand")))){
				break;
			}
			zcfgFeJsonObjFree(RadioObj);
		}
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &SsidIid, &SsidObj) == ZCFG_SUCCESS){
			tmpLowerLayer = json_object_get_string(json_object_object_get(SsidObj, "LowerLayers"));
			sscanf(tmpLowerLayer, "WiFi.Radio.%hhu", &tmpRadioIndex);
			if(RadioIid.idx[0] == tmpRadioIndex){
				ssidIndex = SsidIid.idx[0];
				break;
			}
			zcfgFeJsonObjFree(SsidObj);
		}
		json_object_object_add(Jobj, "Index", json_object_new_int(ssidIndex));
		json_object_object_del(Jobj, "Band");
		zcfgFeJsonObjFree(SsidObj);
		zcfgFeJsonObjFree(RadioObj);
	}
}

zcfgRet_t zcfgFeDalWifiOthersGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t SsidIid={0};
	objIndex_t RadioIid={0};
	objIndex_t APSecIid={0};
	bool enablePMF = false, protection = false;
	int pmfMethod = 0;
	struct json_object *SsidObj = NULL;
	struct json_object *RadioObj = NULL;
	struct json_object *APSecObj = NULL;
	struct json_object *pramJobj = NULL;
	const char *tmpLowerLayer = NULL;
	int index;
	uint8_t tmpRadioIndex;

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	SsidIid.level = 1;
	SsidIid.idx[0] = index;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_SSID, &SsidIid, &SsidObj)) != ZCFG_SUCCESS)
		return ret;
	tmpLowerLayer = json_object_get_string(json_object_object_get(SsidObj, "LowerLayers"));
	sscanf(tmpLowerLayer, "WiFi.Radio.%hhu", &tmpRadioIndex);
	RadioIid.level = 1;
	RadioIid.idx[0] = tmpRadioIndex;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &RadioIid, &RadioObj)) != ZCFG_SUCCESS)
		return ret;

	APSecIid.level = 1;
	APSecIid.idx[0] = index;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &APSecIid, &APSecObj)) != ZCFG_SUCCESS)
		return ret;

	pramJobj = json_object_new_object();
	json_object_object_add(pramJobj, "Band", JSON_OBJ_COPY(json_object_object_get(RadioObj, "OperatingFrequencyBand")));
	json_object_object_add(pramJobj, "SupportedStandards", JSON_OBJ_COPY(json_object_object_get(RadioObj, "SupportedStandards")));
	json_object_object_add(pramJobj, "wlRtsThrshld", JSON_OBJ_COPY(json_object_object_get(RadioObj, "X_ZYXEL_RTS_CTS_Threshold")));
	json_object_object_add(pramJobj, "wlFrgThrshld", JSON_OBJ_COPY(json_object_object_get(RadioObj, "X_ZYXEL_Fragment_Threshold")));
	json_object_object_add(pramJobj, "wlBcnIntvl", JSON_OBJ_COPY(json_object_object_get(RadioObj, "X_ZYXEL_Beacon_Interval")));
	json_object_object_add(pramJobj, "wlDtmIntvl", JSON_OBJ_COPY(json_object_object_get(RadioObj, "X_ZYXEL_DTIM_Interval")));
	json_object_object_add(pramJobj, "AutoChannelTimer", JSON_OBJ_COPY(json_object_object_get(RadioObj, "AutoChannelRefreshPeriod")));
	json_object_object_add(pramJobj, "wlMode", JSON_OBJ_COPY(json_object_object_get(RadioObj, "OperatingStandards")));
	json_object_object_add(pramJobj, "wlTxPwrPcnt", JSON_OBJ_COPY(json_object_object_get(RadioObj, "TransmitPower")));
	/* The type of X_ZYXEL_Protection for data model is boolean and the type of wlProtect for GUI is string. */
	protection = json_object_get_boolean(json_object_object_get(RadioObj, "X_ZYXEL_Protection"));
	if (protection == true)
	{
		json_object_object_add(pramJobj, "wlProtect", json_object_new_string("true"));
	}
	else
	{
		json_object_object_add(pramJobj, "wlProtect", json_object_new_string("false"));
	}
	json_object_object_add(pramJobj, "wlPreamble", JSON_OBJ_COPY(json_object_object_get(RadioObj, "X_ZYXEL_Preamble")));
	json_object_object_add(pramJobj, "InActiveChan", JSON_OBJ_COPY(json_object_object_get(RadioObj, "X_ZYXEL_InActiveChannels")));
	json_object_object_add(pramJobj, "atf", JSON_OBJ_COPY(json_object_object_get(RadioObj, "X_ZYXEL_AirtimeFairness")));
	json_object_object_add(pramJobj, "acs_fcs_mode", JSON_OBJ_COPY(json_object_object_get(RadioObj, "X_ZYXEL_FastChannelSelect")));

	enablePMF = json_object_get_boolean(json_object_object_get(APSecObj, "EnableManagementFrameProtection"));
	pmfMethod = json_object_get_int(json_object_object_get(APSecObj, "X_ZYXEL_PmfMethod"));
	if (enablePMF == false) {
		json_object_object_add(pramJobj, "wlPmf", json_object_new_string("disabled"));
	}
	else { //enablePMF = true
		if (pmfMethod == 0) {
			json_object_object_add(pramJobj, "wlPmf", json_object_new_string("capable"));
		}
		else {
			json_object_object_add(pramJobj, "wlPmf", json_object_new_string("required"));
		}
	}

	json_object_array_add(Jarray, pramJobj);

	if(SsidObj)
		zcfgFeJsonObjFree(SsidObj);
	if(RadioObj)
		zcfgFeJsonObjFree(RadioObj);
	if(APSecObj)
		zcfgFeJsonObjFree(APSecObj);
	return ret;
}

zcfgRet_t setwithoutWlOthersObjects(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(wifiApWpsOthersObj) zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_WPS, &wifiApWpsOthersIid, wifiApWpsOthersObj, NULL);
	if(wifiRadioOthersObj) zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_RADIO, &wifiRadioOthersIid, wifiRadioOthersObj, NULL);

	return ret;
}

zcfgRet_t setAllWlOthersObjects(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS,ret_sec = ZCFG_SUCCESS,ret_radio = ZCFG_SUCCESS;
	
	if(wifiApWpsOthersObj) {
		ret_sec = zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_WPS, &wifiApWpsOthersIid, wifiApWpsOthersObj, NULL);
	}
	if(wifiRadioOthersObj){
		ret_radio = zcfgFeObjJsonBlockedSet(RDM_OID_WIFI_RADIO, &wifiRadioOthersIid, wifiRadioOthersObj, NULL);
	}
	if(ret_sec != ZCFG_SUCCESS){
		ret = ret_sec;
	}else if(ret_radio != ZCFG_SUCCESS){
		ret = ret_radio;
	}
	
	return ret;
}

zcfgRet_t freeAllWlOthersObjects(){
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if(wifiRadioOthersObj) json_object_put(wifiRadioOthersObj); 
	if(wifiApWpsOthersObj) json_object_put(wifiApWpsOthersObj);
	if(wifiSecOthersObj) json_object_put(wifiSecOthersObj);
	
	return ret;
}

zcfgRet_t getBasicWlOthersInfo(struct json_object *Jobj, bool *apply){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t SsidIid={0};
	objIndex_t RadioIid={0};
	objIndex_t APSecIid={0};
	struct json_object *SsidObj = NULL;
	struct json_object *RadioObj = NULL;
	struct json_object *APSecObj = NULL;
	const char *tmpLowerLayer = NULL;
	int index, pmfMethod = 0;
	bool enablePMF = false, protection = false;
	uint8_t tmpRadioIndex;

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	SsidIid.level = 1;
	SsidIid.idx[0] = index;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_SSID, &SsidIid, &SsidObj)) != ZCFG_SUCCESS)
		return ret;
	tmpLowerLayer = json_object_get_string(json_object_object_get(SsidObj, "LowerLayers"));
	sscanf(tmpLowerLayer, "WiFi.Radio.%hhu", &tmpRadioIndex);
	RadioIid.level = 1;
	RadioIid.idx[0] = tmpRadioIndex;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &RadioIid, &RadioObj)) != ZCFG_SUCCESS)
		return ret;

	APSecIid.level = 1;
	APSecIid.idx[0] = index;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &APSecIid, &APSecObj)) != ZCFG_SUCCESS)
		return ret;

	wlRtsThrshld_ori = json_object_get_int(json_object_object_get(RadioObj, "X_ZYXEL_RTS_CTS_Threshold"));
	wlFrgThrshld_ori = json_object_get_int(json_object_object_get(RadioObj, "X_ZYXEL_Fragment_Threshold"));
	wlBcnIntvl_ori = json_object_get_int(json_object_object_get(RadioObj, "X_ZYXEL_Beacon_Interval"));
	wlDtmIntvl_ori = json_object_get_int(json_object_object_get(RadioObj, "X_ZYXEL_DTIM_Interval"));
	wlMode_ori = json_object_get_string(json_object_object_get(RadioObj, "OperatingStandards"));
	wlTxPwrPcnt_ori = json_object_get_int(json_object_object_get(RadioObj, "TransmitPower"));
	/* The type of X_ZYXEL_Protection for data model is boolean and the type of wlProtect for GUI is string. */
	protection = json_object_get_boolean(json_object_object_get(RadioObj, "X_ZYXEL_Protection"));
	if (protection == true)
		wlProtect_ori = "auto";
	else
		wlProtect_ori = "off";
	wlPreamble_ori = json_object_get_string(json_object_object_get(RadioObj, "X_ZYXEL_Preamble"));
	atf_ori = json_object_get_boolean(json_object_object_get(RadioObj, "X_ZYXEL_AirtimeFairness"));
	acs_fcs_mode_ori = json_object_get_boolean(json_object_object_get(RadioObj, "X_ZYXEL_FastChannelSelect"));

	enablePMF = json_object_get_boolean(json_object_object_get(APSecObj, "EnableManagementFrameProtection"));
	pmfMethod = json_object_get_int(json_object_object_get(APSecObj, "X_ZYXEL_PmfMethod"));
	InActiveChan_ori = json_object_get_string(json_object_object_get(RadioObj, "X_ZYXEL_InActiveChannels"));
	if (enablePMF == false) {
		 wlPmf_ori = "disabled";
	}
	else { //enablePMF = true
		if (pmfMethod == 0) {
			 wlPmf_ori = "capable";
		}
		else {
			 wlPmf_ori = "required";
		}
	}


	//WiFi Radio
	wlRtsThrshld = json_object_get_int(json_object_object_get(Jobj, "wlRtsThrshld"));
	wlFrgThrshld = json_object_get_int(json_object_object_get(Jobj, "wlFrgThrshld"));
	AutoChannelTimer = json_object_get_int(json_object_object_get(Jobj, "AutoChannelTimer"));
	wlCsScanTimer = json_object_get_int(json_object_object_get(Jobj, "wlCsScanTimer"));
	wlTxPwrPcnt = json_object_get_int(json_object_object_get(Jobj, "wlTxPwrPcnt"));
	wlBcnIntvl = json_object_get_int(json_object_object_get(Jobj, "wlBcnIntvl"));
	wlDtmIntvl = json_object_get_int(json_object_object_get(Jobj, "wlDtmIntvl"));
	wlMode = json_object_get_string(json_object_object_get(Jobj, "wlMode"));
	wlProtect = json_object_get_string(json_object_object_get(Jobj, "wlProtect"));
	//wlRifsAdvert = json_object_get_string(json_object_object_get(Jobj, "wlRifsAdvert"));
	wlPreamble = json_object_get_string(json_object_object_get(Jobj, "wlPreamble"));
	wlRxChainPwrSave = json_object_get_int(json_object_object_get(Jobj, "wlRxChainPwrSave"));
	//wlObssCox = json_object_get_boolean(json_object_object_get(Jobj, "wlObssCox"));;
	wlFrameburst = json_object_get_boolean(json_object_object_get(Jobj, "wlFrameburst"));
	wlRegMode = json_object_get_int(json_object_object_get(Jobj, "wlRegMode"));
	wlDfsPreIsm = json_object_get_int(json_object_object_get(Jobj, "wlDfsPreIsm"));
	wlTpcDb = json_object_get_int(json_object_object_get(Jobj, "wlTpcDb"));
	wlDfsPostIsm = json_object_get_int(json_object_object_get(Jobj, "wlDfsPostIsm"));
	OperatingChannelBandwidth = json_object_get_string(json_object_object_get(Jobj, "OperatingChannelBandwidth"));
	//WiFi AP Sec
	wlPmf = json_object_get_string(json_object_object_get(Jobj, "wlPmf"));
	//WiFi AP WPS
	wps_v2 = json_object_get_boolean(json_object_object_get(Jobj, "wps_v2"));
	atf = json_object_get_boolean(json_object_object_get(Jobj, "atf"));
	acs_fcs_mode = json_object_get_boolean(json_object_object_get(Jobj, "acs_fcs_mode"));
	InActiveChan = json_object_get_string(json_object_object_get(Jobj, "InActiveChan"));
	singleSKU = json_object_get_string(json_object_object_get(Jobj, "singleSKU"));

	if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")), "GUI")){
		if ((acs_fcs_mode_ori == acs_fcs_mode) && (atf_ori == atf) && !strcmp(wlPreamble_ori,wlPreamble) && !strcmp(wlProtect_ori,wlProtect) &&
		(wlTxPwrPcnt_ori == wlTxPwrPcnt) && !strcmp(wlMode_ori,wlMode) && (wlDtmIntvl_ori == wlDtmIntvl) &&
		(wlBcnIntvl_ori == wlBcnIntvl) && (wlFrgThrshld_ori == wlFrgThrshld) &&
		(wlRtsThrshld_ori == wlRtsThrshld) && !strcmp(wlPmf_ori,wlPmf) && !strcmp(InActiveChan, InActiveChan_ori)){
			*apply = false;
		}
	}
	if(SsidObj)
		zcfgFeJsonObjFree(SsidObj);
	if(RadioObj)
		zcfgFeJsonObjFree(RadioObj);
	if(APSecObj)
		zcfgFeJsonObjFree(APSecObj);
	return ret;
}

void initGlobalWlOthersObjects(){
	
	wifiRadioOthersObj = NULL;
	wifiApWpsOthersObj = NULL;
	wifiSecOthersObj = NULL;
	
	IID_INIT(wifiRadioOthersIid);
	IID_INIT(wifiApWpsOthersIid);
	IID_INIT(wifiSecOthersIid);
}

zcfgRet_t editwifiRadioOthersObjects(struct json_object *Jobj){

	zcfgRet_t ret = ZCFG_SUCCESS;
	const char* tmpOperatingFrequencyBand;

	objIndex_t SsidIid={0};
	struct json_object *SsidObj = NULL;
	const char *tmpLowerLayer = NULL;
	uint8_t tmpRadioIndex;	
	
	SsidIid.level = 1;
	SsidIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "Index"));
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_SSID, &SsidIid, &SsidObj)) != ZCFG_SUCCESS)
		return ret;
	tmpLowerLayer = json_object_get_string(json_object_object_get(SsidObj, "LowerLayers"));
	sscanf(tmpLowerLayer, "WiFi.Radio.%hhu", &tmpRadioIndex);
	IID_INIT(wifiRadioOthersIid);
	wifiRadioOthersIid.level = 1;
	wifiRadioOthersIid.idx[0] = tmpRadioIndex;
	zcfgFeJsonObjFree(SsidObj);
	
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_RADIO, &wifiRadioOthersIid, &wifiRadioOthersObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(json_object_object_get(Jobj, "wlRtsThrshld"))
		json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_RTS_CTS_Threshold", json_object_new_int(wlRtsThrshld));
	if(json_object_object_get(Jobj, "wlFrgThrshld"))
		json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_Fragment_Threshold", json_object_new_int(wlFrgThrshld));
	if(json_object_object_get(Jobj, "AutoChannelTimer"))
		json_object_object_add(wifiRadioOthersObj, "AutoChannelRefreshPeriod", json_object_new_int(AutoChannelTimer));
	if(json_object_object_get(Jobj, "wlMode"))
		json_object_object_add(wifiRadioOthersObj, "OperatingStandards", json_object_new_string(wlMode));
	if(json_object_object_get(Jobj, "wlTxPwrPcnt"))
		json_object_object_add(wifiRadioOthersObj, "TransmitPower", json_object_new_int(wlTxPwrPcnt));
	if(json_object_object_get(Jobj, "wlProtect")){
		if(!strcmp(wlProtect, "off"))
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_Protection", json_object_new_boolean(false));
		else
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_Protection", json_object_new_boolean(true));
	}
	if(json_object_object_get(Jobj, "wlPreamble"))
		json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_Preamble", json_object_new_string(wlPreamble));
	//if (!strcmp(wlRifsAdvert, "off"))
	//	json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_RIFS_Advertisement", json_object_new_boolean(false));
	//else
	//	json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_RIFS_Advertisement", json_object_new_boolean(true));
	if(json_object_object_get(Jobj, "wlFrameburst"))
		json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_Frameburst", json_object_new_boolean(wlFrameburst));
	//json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_OBSS_coex", json_object_new_boolean(wlObssCox));
	tmpOperatingFrequencyBand = json_object_get_string(json_object_object_get(wifiRadioOthersObj, "OperatingFrequencyBand"));
	if(!strcmp(tmpOperatingFrequencyBand,"5GHz"))
	{
		if(json_object_object_get(Jobj, "wlDfsPreIsm"))
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_DfsPreIsm", json_object_new_int(wlDfsPreIsm));
		if(json_object_object_get(Jobj, "wlDfsPostIsm"))
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_DfsPostIsm", json_object_new_int(wlDfsPostIsm));
		if(json_object_object_get(Jobj, "wlTpcDb"))
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_TpcDb", json_object_new_int(wlTpcDb));
	}

#ifdef ZY_WIFI_DBDC_MODE
	if(!strcmp(tmpOperatingFrequencyBand,"2.4GHz"))
#endif
	{
		if(json_object_object_get(Jobj, "wlBcnIntvl"))
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_Beacon_Interval", json_object_new_int(wlBcnIntvl));
		if(json_object_object_get(Jobj, "wlDtmIntvl"))
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_DTIM_Interval", json_object_new_int(wlDtmIntvl));
	}

	if(json_object_object_get(Jobj, "atf"))
		json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_AirtimeFairness", json_object_new_boolean(atf));
	if(json_object_object_get(Jobj, "acs_fcs_mode"))
		json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_FastChannelSelect", json_object_new_boolean(acs_fcs_mode));
	if(json_object_object_get(Jobj, "InActiveChan")){
		if(InActiveChan) {
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_InActiveChannels", json_object_new_string(InActiveChan));
		}
		else {
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_InActiveChannels", json_object_new_string(""));
		}
	}
/* __ZyXEL__, Melissa, 20161111, Costomization for VMG1312-T20A Brazil */
	if(json_object_object_get(Jobj, "singleSKU")){
		if(singleSKU) {
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_SingleSKU", json_object_new_string(singleSKU));
		}
		else {
			json_object_object_add(wifiRadioOthersObj, "X_ZYXEL_SingleSKU", json_object_new_string(""));
		}
	}
	
	return ret;
}

zcfgRet_t editWifiOthersSecObject(struct json_object *Jobj){

	zcfgRet_t ret = ZCFG_SUCCESS;
	char lowerlayer[16] = {0}, main_radio[2] = {0}, cur_radio[2] = {0};
	struct json_object *wifiSsidObj = NULL;
	objIndex_t wifiSsidIid;

	// Search Main interface radio
	IID_INIT(wifiSsidIid);
	wifiSsidIid.level = 1;
	wifiSsidIid.idx[0] = json_object_get_int(json_object_object_get(Jobj,"Index"));
	if ((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &wifiSsidIid, &wifiSsidObj)) != ZCFG_SUCCESS) {
		return ret;
	}
	strcpy(lowerlayer, json_object_get_string(json_object_object_get(wifiSsidObj, "LowerLayers")));
	sscanf(lowerlayer, "WiFi.Radio.%1s", main_radio);
	zcfgFeJsonObjFree(wifiSsidObj);

	IID_INIT(wifiSecOthersIid);
	wifiSecOthersIid.level = 1;
	wifiSecOthersIid.idx[0] = json_object_get_int(json_object_object_get(Jobj,"Index"))-1;
	while (zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_SEC, &wifiSecOthersIid, &wifiSecOthersObj) == ZCFG_SUCCESS) {
		// Search Current interface radio
		IID_INIT(wifiSsidIid);
		wifiSsidIid.level = 1;
		wifiSsidIid.idx[0] = wifiSecOthersIid.idx[0];
		if ((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &wifiSsidIid, &wifiSsidObj)) != ZCFG_SUCCESS) {
			zcfgFeJsonObjFree(wifiSecOthersObj);
			return ret;
		}
		strcpy(lowerlayer, json_object_get_string(json_object_object_get(wifiSsidObj, "LowerLayers")));
		sscanf(lowerlayer, "WiFi.Radio.%1s", cur_radio);
		zcfgFeJsonObjFree(wifiSsidObj);

		if (!strcmp(cur_radio, main_radio)) { //Current interface radio = Main interface radio, update PMF value according band
			if (json_object_object_get(Jobj, "wlPmf")) {
				if (!strcmp(wlPmf, "disabled")) {
					json_object_object_add(wifiSecOthersObj, "EnableManagementFrameProtection", json_object_new_boolean(false));
					json_object_object_add(wifiSecOthersObj, "X_ZYXEL_PmfMethod", json_object_new_int(0));
				}
				else if (!strcmp(wlPmf, "capable")) {
					json_object_object_add(wifiSecOthersObj, "EnableManagementFrameProtection", json_object_new_boolean(true));
					json_object_object_add(wifiSecOthersObj, "X_ZYXEL_PmfMethod", json_object_new_int(0));
				}
				else { //wlPmf = required
					json_object_object_add(wifiSecOthersObj, "EnableManagementFrameProtection", json_object_new_boolean(true));
					json_object_object_add(wifiSecOthersObj, "X_ZYXEL_PmfMethod", json_object_new_int(1));
				}
			}
			zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_SEC, &wifiSecOthersIid, wifiSecOthersObj, NULL);
			zcfgFeJsonObjFree(wifiSecOthersObj);
		}
		else {
			zcfgFeJsonObjFree(wifiSecOthersObj);
			break;
		}
	}
	return ret;
}

zcfgRet_t editWifiOthersApWpsObject(struct json_object *Jobj){

	zcfgRet_t ret = ZCFG_SUCCESS;
	
	IID_INIT(wifiApWpsOthersIid);
	wifiApWpsOthersIid.level = 1;
	wifiApWpsOthersIid.idx[0] = json_object_get_int(json_object_object_get(Jobj,"Index"));
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_WPS, &wifiApWpsOthersIid, &wifiApWpsOthersObj)) != ZCFG_SUCCESS) {
		return ret;
	}
	
	if(json_object_object_get(Jobj, "wps_v2"))
		json_object_object_add(wifiApWpsOthersObj, "X_ZYXEL_WPS_V2", json_object_new_boolean(wps_v2));
	
	return ret;
}

zcfgRet_t zcfgFeDalWifiOthersEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	printf("%s %s %d \n",json_object_to_json_string(Jobj),__func__,__LINE__);
	bool apply = true;
	
	initGlobalWlOthersObjects();

	getBasicWlOthersInfo(Jobj, &apply);
	if(apply != true){
		goto dalwlan_edit_fail;
	}

	if((ret = editWifiOthersApWpsObject(Jobj)) != ZCFG_SUCCESS){
		printf("editWifiOthersApWpsObject fail at %d\n", __LINE__);
		goto dalwlan_edit_fail;
	}
	
	if((ret = editWifiOthersSecObject(Jobj)) != ZCFG_SUCCESS){
		printf("editWifiOthersSecObject fail at %d\n", __LINE__);
		goto dalwlan_edit_fail;
	}
		
	 
	if((ret = editwifiRadioOthersObjects(Jobj)) != ZCFG_SUCCESS){
		printf("editwifiRadioOthersObjects fail at %d\n", __LINE__);
		goto dalwlan_edit_fail;
	}
			
	if(json_object_get_int(json_object_object_get(Jobj, "cert")))
		ret = setwithoutWlOthersObjects(Jobj);
	else
		ret = setAllWlOthersObjects(Jobj);

dalwlan_edit_fail:
	freeAllWlOthersObjects();
	
	return ret;
}

zcfgRet_t zcfgFeDalWifiOthers(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	prepareJObj(Jobj);
	
	//if(!strcmp(method, "POST"))
		//ret = zcfgFeDalWifiOthersAdd(Jobj, NULL);
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalWifiOthersEdit(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalWifiOthersGet(Jobj, Jarray, replyMsg);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}


