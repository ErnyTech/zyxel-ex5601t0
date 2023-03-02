#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

//#ifdef CAAAB_CUSTOMIZATION
#if defined(CAAAB_CUSTOMIZATION) || defined(ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE)

struct json_object *cbtWifiRadioAdvObj = NULL;
struct json_object *cbt5GWifiRadioAdvObj = NULL;
struct json_object *cbtWifiApWpsAdvObj = NULL;
struct json_object *cbt5GWifiApWpsAdvObj = NULL;
struct json_object *cbtWifiApAdvObj = NULL;
struct json_object *cbt5GWifiApAdvObj = NULL;

objIndex_t cbtWifiRadioAdvIid = {0};
objIndex_t cbt5GWifiRadioAdvIid = {0};
objIndex_t cbtWifiApWpsAdvIid = {0};
objIndex_t cbt5GWifiApWpsAdvIid = {0};
objIndex_t cbtWifiApAdvIid = {0};
objIndex_t cbt5GWifiApAdvIid = {0};

// RDM_OID_WIFI_RADIO
int cbtWifiChannel;
int cbt5GWifiChannel;
const char* cbtWifiMode;
const char* cbt5GWifiMode;
unsigned int cbtWifiRtsThrshld;
unsigned int cbt5GWifiRtsThrshld;
unsigned int cbtWifiFrgThrshld;
unsigned int cbt5GWifiFrgThrshld;
unsigned int cbtWifiTxPwrPcnt;
unsigned int cbt5GWifiTxPwrPcnt;
unsigned int cbtWifiBcnIntvl;
unsigned int cbt5GWifiBcnIntvl;
unsigned int cbtWifiDtmIntvl;
unsigned int cbt5GWifiDtmIntvl;
const char* cbtWifiProtect;
const char* cbt5GWifiProtect;
const char* cbtWifiPreamble;
const char* cbt5GWifiPreamble;
bool cbtWifiObssCox;
bool cbt5GWifiObssCox;

// RDM_OID_WIFI_ACCESS_POINT
bool cbtWifiHideSsid;
bool cbt5GWifiHideSsid;
bool cbtWifiWmm;
bool cbt5GWifiWmm;
bool cbtWifiWmmApsd;
bool cbt5GWifiWmmApsd;

// RDM_OID_WIFI_ACCESS_POINT_WPS
bool cbtWifiWps;
bool cbt5GWifiWps;

time_t t1, t2;

dal_param_t WIFI_ADV_CAAAB_param[]={
	//WiFi Radio
	{"cbtWifiChannel",			dalType_int,	0,	0,	NULL},
	{"cbt5GWifiChannel",		dalType_int,	0,	0,	NULL},
	{"cbtWifiMode",				dalType_string, 0,	0,	NULL},
	{"cbt5GWifiMode",			dalType_string, 0,	0,	NULL},
	{"cbtWifiRtsThrshld",		dalType_int,	0,	0,	NULL},
	{"cbt5GWifiRtsThrshld",		dalType_int,	0,	0,	NULL},
	{"cbtWifiFrgThrshld",		dalType_int,	0,	0,	NULL},
	{"cbt5GWifiFrgThrshld",		dalType_int,	0,	0,	NULL},
	{"cbtWifiTxPwrPcnt",		dalType_int,	0,	0,	NULL},
	{"cbt5GWifiTxPwrPcnt",		dalType_int,	0,	0,	NULL},
	{"cbtWifiBcnIntvl",			dalType_int,	0,	0,	NULL},
	{"cbt5GWifiBcnIntvl",		dalType_int,	0,	0,	NULL},
	{"cbtWifiDtmIntvl",			dalType_int,	0,	0,	NULL},
	{"cbt5GWifiDtmIntvl",		dalType_int,	0,	0,	NULL},
	{"cbtWifiProtect",			dalType_string, 0,	0,	NULL},
	{"cbt5GWifiProtect",		dalType_string, 0,	0,	NULL},
	{"cbtWifiPreamble",			dalType_string, 0,	0,	NULL},
	{"cbt5GWifiPreamble",		dalType_string, 0,	0,	NULL},
	{"cbtWifiObssCox",			dalType_boolean,0,	0,	NULL},
	{"cbt5GWifiObssCox",		dalType_boolean,0,	0,	NULL},
	// WiFi Access Point
	{"cbtWifiHideSsid",			dalType_boolean,0,	0,	NULL},
	{"cbt5GWifiHideSsid",		dalType_boolean,0,	0,	NULL},
	{"cbtWifiWmm",				dalType_boolean,0,	0,	NULL},
	{"cbt5GWifiWmm",			dalType_boolean,0,	0,	NULL},
	{"cbtWifiWmmApsd",			dalType_boolean,0,	0,	NULL},
	{"cbt5GWifiWmmApsd",		dalType_boolean,0,	0,	NULL},
	// WiFi Access Point WPS
	{"cbtWifiWps",				dalType_boolean,0,	0,	NULL},
	{"cbt5GWifiWps",			dalType_boolean,0,	0,	NULL},
	{NULL,		0,	0,	0,	NULL},
};

void initCbtWifiAdvObjects(){
	
	cbtWifiRadioAdvObj = NULL;
	cbt5GWifiRadioAdvObj = NULL;
	cbtWifiApWpsAdvObj = NULL;
	cbt5GWifiApWpsAdvObj = NULL;
	cbtWifiApAdvObj = NULL;
	cbt5GWifiApAdvObj = NULL;
	
	IID_INIT(cbtWifiRadioAdvIid);
	IID_INIT(cbt5GWifiRadioAdvIid);
	IID_INIT(cbtWifiApWpsAdvIid);
	IID_INIT(cbt5GWifiApWpsAdvIid);
	IID_INIT(cbtWifiApAdvIid);
	IID_INIT(cbt5GWifiApAdvIid);
}

zcfgRet_t freeAllCbtWifiAdvObjects(){
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if(cbtWifiRadioAdvObj) json_object_put(cbtWifiRadioAdvObj);
	if(cbt5GWifiRadioAdvObj) json_object_put(cbt5GWifiRadioAdvObj);
	if(cbtWifiApWpsAdvObj) json_object_put(cbtWifiApWpsAdvObj);
	if(cbt5GWifiApWpsAdvObj) json_object_put(cbt5GWifiApWpsAdvObj);
	if(cbtWifiApAdvObj) json_object_put(cbtWifiApAdvObj);
	if(cbt5GWifiApAdvObj) json_object_put(cbt5GWifiApAdvObj);
	
	return ret;
}

void getCbtWifiAdvInfo(struct json_object *Jobj){
	// Hide SSID
	cbtWifiHideSsid = json_object_get_boolean(json_object_object_get(Jobj, "cbtWifiHideSsid"));
	cbt5GWifiHideSsid = json_object_get_boolean(json_object_object_get(Jobj, "cbt5GWifiHideSsid"));
	// Channel
	cbtWifiChannel = json_object_get_int(json_object_object_get(Jobj, "cbtWifiChannel"));
	cbt5GWifiChannel = json_object_get_int(json_object_object_get(Jobj, "cbt5GWifiChannel"));
	// 802.11 Mode
	cbtWifiMode = json_object_get_string(json_object_object_get(Jobj, "cbtWifiMode"));
	cbt5GWifiMode = json_object_get_string(json_object_object_get(Jobj, "cbt5GWifiMode"));
	// RTS/CTS Threshold
	cbtWifiRtsThrshld = json_object_get_int(json_object_object_get(Jobj, "cbtWifiRtsThreshold"));
	cbt5GWifiRtsThrshld = json_object_get_int(json_object_object_get(Jobj, "cbt5GWifiRtsThreshold"));
	// Fragmentation Threshold
	cbtWifiFrgThrshld = json_object_get_int(json_object_object_get(Jobj, "cbtWifiFrgThreshold"));
	cbt5GWifiFrgThrshld = json_object_get_int(json_object_object_get(Jobj, "cbt5GWifiFrgThreshold"));
	// Output Power
	cbtWifiTxPwrPcnt = json_object_get_int(json_object_object_get(Jobj, "cbtWifiTxPwrPcnt"));
	cbt5GWifiTxPwrPcnt = json_object_get_int(json_object_object_get(Jobj, "cbt5GWifiTxPwrPcnt"));
	// Beacon Interval
	cbtWifiBcnIntvl = json_object_get_int(json_object_object_get(Jobj, "cbtWifiBcnIntvl"));
	cbt5GWifiBcnIntvl = json_object_get_int(json_object_object_get(Jobj, "cbt5GWifiBcnIntvl"));
	// DTIM Interval
	cbtWifiDtmIntvl = json_object_get_int(json_object_object_get(Jobj, "cbtWifiDtmIntvl"));
	cbt5GWifiDtmIntvl = json_object_get_int(json_object_object_get(Jobj, "cbt5GWifiDtmIntvl"));
	// 802.11 Protection
	cbtWifiProtect = json_object_get_string(json_object_object_get(Jobj, "cbtWifiProtect"));
	cbt5GWifiProtect = json_object_get_string(json_object_object_get(Jobj, "cbt5GWifiProtect"));
	// Preamble
	cbtWifiPreamble = json_object_get_string(json_object_object_get(Jobj, "cbtWifiPreamble"));
	cbt5GWifiPreamble = json_object_get_string(json_object_object_get(Jobj, "cbt5GWifiPreamble"));
	// WPS
	cbtWifiWps = json_object_get_boolean(json_object_object_get(Jobj, "cbtWifiWps"));
	cbt5GWifiWps = json_object_get_boolean(json_object_object_get(Jobj, "cbt5GWifiWps"));
	// OBSS Coexistence
	cbtWifiObssCox = json_object_get_boolean(json_object_object_get(Jobj, "cbtWifiObssCox"));
	cbt5GWifiObssCox = json_object_get_boolean(json_object_object_get(Jobj, "cbt5GWifiObssCox"));
	// WMM
	cbtWifiWmm = json_object_get_boolean(json_object_object_get(Jobj, "cbtWifiWmm"));
	cbt5GWifiWmm = json_object_get_boolean(json_object_object_get(Jobj, "cbt5GWifiWmm"));
	// WMM Automatic Power Save Delivery
	cbtWifiWmmApsd = json_object_get_boolean(json_object_object_get(Jobj, "cbtWifiWmmApsd"));
	cbt5GWifiWmmApsd = json_object_get_boolean(json_object_object_get(Jobj, "cbt5GWifiWmmApsd"));

	return;
}

zcfgRet_t setAllCbtWifiAdvObjects(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	// 2.4G set data model
	if(cbtWifiApWpsAdvObj)
		zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_WPS, &cbtWifiApWpsAdvIid, cbtWifiApWpsAdvObj, NULL);

	if(cbtWifiApAdvObj)
		zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &cbtWifiApAdvIid, cbtWifiApAdvObj, NULL);

	if(cbtWifiRadioAdvObj)
		zcfgFeObjJsonSet(RDM_OID_WIFI_RADIO, &cbtWifiRadioAdvIid, cbtWifiRadioAdvObj, NULL);

	// 5G set data model
	if(cbt5GWifiApWpsAdvObj)
		zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_WPS, &cbt5GWifiApWpsAdvIid, cbt5GWifiApWpsAdvObj, NULL);

	if(cbt5GWifiApAdvObj)
		zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &cbt5GWifiApAdvIid, cbt5GWifiApAdvObj, NULL);

	if(cbt5GWifiRadioAdvObj)
		zcfgFeObjJsonSet(RDM_OID_WIFI_RADIO, &cbt5GWifiRadioAdvIid, cbt5GWifiRadioAdvObj, NULL);
	
	return ret;
}

zcfgRet_t editCbtWifiRadioObject(struct json_object *Jobj){

	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *iidArray = NULL;
	uint8_t iidIdx = 0;

	// 2.4G Iid
	iidArray = json_object_object_get(Jobj, "cbtWifiAdvRadIid");
	for(iidIdx = 0; iidIdx < 6; iidIdx++){
		cbtWifiRadioAdvIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
		if(cbtWifiRadioAdvIid.idx[iidIdx] != 0)
			cbtWifiRadioAdvIid.level = iidIdx + 1;
	}
	// 5G Iid
	iidArray = json_object_object_get(Jobj, "cbt5GWifiAdvRadIid");
	for(iidIdx = 0; iidIdx < 6; iidIdx++){
		cbt5GWifiRadioAdvIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
		if(cbt5GWifiRadioAdvIid.idx[iidIdx] != 0)
			cbt5GWifiRadioAdvIid.level = iidIdx + 1;
	}

	// 2.4G data
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_RADIO, &cbtWifiRadioAdvIid, &cbtWifiRadioAdvObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(cbtWifiChannel == 0){
		json_object_object_add(cbtWifiRadioAdvObj, "AutoChannelEnable", json_object_new_boolean(true));
	}else{
#ifdef ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE
		if(cbtWifiChannel > 7)
			json_object_object_add(cbtWifiRadioAdvObj, "ExtensionChannel", json_object_new_string("BelowControlChannel"));
		else
			json_object_object_add(cbtWifiRadioAdvObj, "ExtensionChannel", json_object_new_string("AboveControlChannel"));
#endif
		json_object_object_add(cbtWifiRadioAdvObj, "AutoChannelEnable", json_object_new_boolean(false));
		json_object_object_add(cbtWifiRadioAdvObj, "Channel", json_object_new_int(cbtWifiChannel));
	}

	json_object_object_add(cbtWifiRadioAdvObj, "X_ZYXEL_RTS_CTS_Threshold", json_object_new_int(cbtWifiRtsThrshld));
	json_object_object_add(cbtWifiRadioAdvObj, "X_ZYXEL_Fragment_Threshold", json_object_new_int(cbtWifiFrgThrshld));
	json_object_object_add(cbtWifiRadioAdvObj, "X_ZYXEL_Beacon_Interval", json_object_new_int(cbtWifiBcnIntvl));
	json_object_object_add(cbtWifiRadioAdvObj, "X_ZYXEL_DTIM_Interval", json_object_new_int(cbtWifiDtmIntvl));
	json_object_object_add(cbtWifiRadioAdvObj, "OperatingStandards", json_object_new_string(cbtWifiMode));
	json_object_object_add(cbtWifiRadioAdvObj, "TransmitPower", json_object_new_int(cbtWifiTxPwrPcnt));

	if(!strcmp(cbtWifiProtect, "off"))
		json_object_object_add(cbtWifiRadioAdvObj, "X_ZYXEL_Protection", json_object_new_boolean(false));
	else
		json_object_object_add(cbtWifiRadioAdvObj, "X_ZYXEL_Protection", json_object_new_boolean(true));
	
	json_object_object_add(cbtWifiRadioAdvObj, "X_ZYXEL_Preamble", json_object_new_string(cbtWifiPreamble));
	json_object_object_add(cbtWifiRadioAdvObj, "X_ZYXEL_OBSS_coex", json_object_new_boolean(cbtWifiObssCox));

	// 5G data
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_RADIO, &cbt5GWifiRadioAdvIid, &cbt5GWifiRadioAdvObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(cbt5GWifiChannel == 0){
		json_object_object_add(cbt5GWifiRadioAdvObj, "AutoChannelEnable", json_object_new_boolean(true));
	}else{
		json_object_object_add(cbt5GWifiRadioAdvObj, "AutoChannelEnable", json_object_new_boolean(false));
		json_object_object_add(cbt5GWifiRadioAdvObj, "Channel", json_object_new_int(cbt5GWifiChannel));
	}

	json_object_object_add(cbt5GWifiRadioAdvObj, "X_ZYXEL_RTS_CTS_Threshold", json_object_new_int(cbt5GWifiRtsThrshld));
	json_object_object_add(cbt5GWifiRadioAdvObj, "X_ZYXEL_Fragment_Threshold", json_object_new_int(cbt5GWifiFrgThrshld));
	json_object_object_add(cbt5GWifiRadioAdvObj, "X_ZYXEL_Beacon_Interval", json_object_new_int(cbt5GWifiBcnIntvl));
	json_object_object_add(cbt5GWifiRadioAdvObj, "X_ZYXEL_DTIM_Interval", json_object_new_int(cbt5GWifiDtmIntvl));
	json_object_object_add(cbt5GWifiRadioAdvObj, "OperatingStandards", json_object_new_string(cbt5GWifiMode));
	json_object_object_add(cbt5GWifiRadioAdvObj, "TransmitPower", json_object_new_int(cbt5GWifiTxPwrPcnt));

	if(!strcmp(cbt5GWifiProtect, "off"))
		json_object_object_add(cbt5GWifiRadioAdvObj, "X_ZYXEL_Protection", json_object_new_boolean(false));
	else
		json_object_object_add(cbt5GWifiRadioAdvObj, "X_ZYXEL_Protection", json_object_new_boolean(true));

	json_object_object_add(cbt5GWifiRadioAdvObj, "X_ZYXEL_Preamble", json_object_new_string(cbt5GWifiPreamble));
	json_object_object_add(cbt5GWifiRadioAdvObj, "X_ZYXEL_OBSS_coex", json_object_new_boolean(cbt5GWifiObssCox));

	return ret;
}

zcfgRet_t editCbtWifiApWpsObject(struct json_object *Jobj){

	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *iidArray = NULL;
	uint8_t iidIdx = 0;

	// 2.4G Iid
	iidArray = json_object_object_get(Jobj, "cbtWifiAdvApIid");
	for(iidIdx = 0; iidIdx < 6; iidIdx++){
		cbtWifiApWpsAdvIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
		if(cbtWifiApWpsAdvIid.idx[iidIdx] != 0)
			cbtWifiApWpsAdvIid.level = iidIdx + 1;
	}
	// 5G Iid
	iidArray = json_object_object_get(Jobj, "cbt5GWifiAdvApIid");
	for(iidIdx = 0; iidIdx < 6; iidIdx++){
		cbt5GWifiApWpsAdvIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
		if(cbt5GWifiApWpsAdvIid.idx[iidIdx] != 0)
			cbt5GWifiApWpsAdvIid.level = iidIdx + 1;
	}

	// 2.4G data
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_WPS, &cbtWifiApWpsAdvIid, &cbtWifiApWpsAdvObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	json_object_object_add(cbtWifiApWpsAdvObj, "Enable", json_object_new_boolean(cbtWifiWps));

	// 5G data
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_WPS, &cbt5GWifiApWpsAdvIid, &cbt5GWifiApWpsAdvObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	json_object_object_add(cbt5GWifiApWpsAdvObj, "Enable", json_object_new_boolean(cbt5GWifiWps));

	return ret;
}

zcfgRet_t editCbtWifiApObject(struct json_object *Jobj){

	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *iidArray = NULL;
	uint8_t iidIdx = 0;

	// 2.4G Iid
	iidArray = json_object_object_get(Jobj, "cbtWifiAdvApIid");
	for(iidIdx = 0; iidIdx < 6; iidIdx++){
		cbtWifiApAdvIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
		if(cbtWifiApAdvIid.idx[iidIdx] != 0)
			cbtWifiApAdvIid.level = iidIdx + 1;
	}
	// 5G Iid
	iidArray = json_object_object_get(Jobj, "cbt5GWifiAdvApIid");
	for(iidIdx = 0; iidIdx < 6; iidIdx++){
		cbt5GWifiApAdvIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
		if(cbt5GWifiApAdvIid.idx[iidIdx] != 0)
			cbt5GWifiApAdvIid.level = iidIdx + 1;
	}

	// 2.4G data
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &cbtWifiApAdvIid, &cbtWifiApAdvObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(cbtWifiHideSsid)
		json_object_object_add(cbtWifiApAdvObj, "SSIDAdvertisementEnabled", json_object_new_boolean(false));
	else
		json_object_object_add(cbtWifiApAdvObj, "SSIDAdvertisementEnabled", json_object_new_boolean(true));

	json_object_object_add(cbtWifiApAdvObj, "WMMEnable", json_object_new_boolean(cbtWifiWmm));
	json_object_object_add(cbtWifiApAdvObj, "UAPSDEnable", json_object_new_boolean(cbtWifiWmmApsd));

	// 5G data
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &cbt5GWifiApAdvIid, &cbt5GWifiApAdvObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(cbt5GWifiHideSsid)
		json_object_object_add(cbt5GWifiApAdvObj, "SSIDAdvertisementEnabled", json_object_new_boolean(false));
	else
		json_object_object_add(cbt5GWifiApAdvObj, "SSIDAdvertisementEnabled", json_object_new_boolean(true));

	json_object_object_add(cbt5GWifiApAdvObj, "WMMEnable", json_object_new_boolean(cbt5GWifiWmm));
	json_object_object_add(cbt5GWifiApAdvObj, "UAPSDEnable", json_object_new_boolean(cbt5GWifiWmmApsd));

	return ret;
}

zcfgRet_t zcfgFeDalCbtWifiAdvEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	initCbtWifiAdvObjects();
	getCbtWifiAdvInfo(Jobj);
	
	t2 = time(NULL);
	printf("line=%d, time=%d\n", __LINE__, (int) difftime(t2, t1));

	if((ret = editCbtWifiRadioObject(Jobj)) != ZCFG_SUCCESS){
		printf("editCbtWifiRadioObject fail\n", __LINE__);
		goto dalwlan_edit_fail;
	}

	if((ret = editCbtWifiApWpsObject(Jobj)) != ZCFG_SUCCESS){
		printf("editCbtWifiApWpsObject fail\n", __LINE__);
		goto dalwlan_edit_fail;
	}

	if((ret = editCbtWifiApObject(Jobj)) != ZCFG_SUCCESS){
		printf("editCbtWifiApObject fail\n", __LINE__);
		goto dalwlan_edit_fail;
	}
		
	t2 = time(NULL);
	printf("before set, time=%d seconds\n", (int) difftime(t2, t1));
	
	setAllCbtWifiAdvObjects(Jobj);
	
	sleep(5); //CAAAB case , the WiFi 2.4 and 5G setting at same time. The delay 5sec for next get.

dalwlan_edit_fail:
	freeAllCbtWifiAdvObjects();
	
	return ret;
}

zcfgRet_t zcfgFeDalCbtWifiAdv(const char *method, struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	t1 = time(NULL);
	
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalCbtWifiAdvEdit(Jobj, NULL);
	else
		printf("Unknown method:%s\n", method);
	
	t2 = time(NULL);
    printf("zcfgFeDalCbtWifiAdv() Total time=%d seconds ret=%d\n", (int) difftime(t2, t1), ret);

	return ret;
}
#endif
