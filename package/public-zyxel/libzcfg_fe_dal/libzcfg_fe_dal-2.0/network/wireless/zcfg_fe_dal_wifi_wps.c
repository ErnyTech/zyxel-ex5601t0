
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t WIFI_WPS_param[] =
{
	//WiFi Access Point WPS
	{"Enable",						dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"EnablePBC",					dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"EnableStaPin",				dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"EnableAPPin", 				dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"Band",						dalType_string,		0,	0,	NULL,	"2.4GHz|5GHz",	dal_Edit},
	{"X_ZYXEL_WPS_GenDevicePin",	dalType_boolean,	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_WPS_StationPin",		dalType_string,		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_WPS_Configured",		dalType_boolean,	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_WPS_Method",			dalType_string,		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_WPS_V2",			dalType_boolean,	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"ConfigMethodsEnabled",		dalType_string,		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{NULL,		0,	0,	0,	NULL,	NULL,	0}
};

void zcfgFeDalShowWifiWps(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-15s %-15s %-15s %-15s %-15s\n","Band","WPS Status","PBC Status","StaPin Status","APPin Status");
	printf("--------------------------------------------------------------------------------\n");
	
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj!=NULL){
			

			printf("%-15s ",json_object_get_string(json_object_object_get(obj, "Band")));
			
			if(json_object_get_boolean(json_object_object_get(obj, "Enable"))){
				printf("%-15s ","Enabled");
				if(json_object_get_boolean(json_object_object_get(obj, "X_ZYXEL_WPS_EnablePBC"))){
					printf("%-15s ","Enabled");
				}
				else{
					printf("%-15s ","Disabled");
				}
				if(json_object_get_boolean(json_object_object_get(obj, "X_ZYXEL_WPS_EnableStaPin"))){
					printf("%-15s ","Enabled");
				}
				else{
					printf("%-15s ","Disabled");
				}
				if(json_object_get_boolean(json_object_object_get(obj, "X_ZYXEL_WPS_EnableAPPin"))){
					printf("%-15s ","Enabled");
				}
				else{
					printf("%-15s ","Disabled");
				}
			}
			else{
				printf("%-15s %-15s %-15s %-15s","Disabled","N/A","N/A","N/A");
			}
			printf("\n");
		}
		else{
			printf("ERROR!\n");
		}
	}
}

zcfgRet_t zcfgFeDalWifiWpsGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *WifiApWpsJobj = NULL;
	struct json_object *WifiApSecJobj = NULL;
	struct json_object *WifiRadioJobj = NULL;
	struct json_object *enable = NULL;
	struct json_object *enablePBC = NULL;
	struct json_object *enableStaPin = NULL;
	struct json_object *enableAPPin = NULL;
	struct json_object *devicePin = NULL;
	struct json_object *wpsConfigured = NULL;
	struct json_object *wpsSecMode = NULL;
	struct json_object *wlMode = NULL;
	struct json_object *pramJobj = NULL;
    const char* Band = NULL;
	bool support5G = false;
	
	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_RADIO, &objIid, &WifiRadioJobj) == ZCFG_SUCCESS) {
		Band = json_object_get_string(json_object_object_get(WifiRadioJobj, "SupportedFrequencyBands"));
		if(!strcmp(Band,"5GHz")){
			support5G = true;
		}
		zcfgFeJsonObjFree(WifiRadioJobj);
	}

	// Band; 2.4GHz
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_WPS, &objIid, &WifiApWpsJobj)) != ZCFG_SUCCESS) {
		printf("%s : Get object fail\n",__FUNCTION__);
		return ret;
	}
	enable = json_object_object_get(WifiApWpsJobj, "Enable");
	enablePBC = json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_EnablePBC");
	enableStaPin = json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_EnableStaPin");
	enableAPPin = json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_EnableAPPin");
	devicePin = json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_DevicePin");
	wpsConfigured = json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_Configured");

	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &objIid, &WifiApSecJobj)) != ZCFG_SUCCESS) {
                printf("%s : Get object fail\n",__FUNCTION__);
                return ret;
        }
	wpsSecMode = json_object_object_get(WifiApSecJobj, "ModeEnabled");

	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &objIid, &WifiRadioJobj)) != ZCFG_SUCCESS) {
                printf("%s : Get object fail\n",__FUNCTION__);
                return ret;
        }
	wlMode = json_object_object_get(WifiRadioJobj, "X_ZYXEL_WlMode");

	pramJobj = json_object_new_object();
	json_object_object_add(pramJobj, "Band", json_object_new_string("2.4GHz"));
	json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(enable));
	json_object_object_add(pramJobj, "X_ZYXEL_WPS_EnablePBC", JSON_OBJ_COPY(enablePBC));
	json_object_object_add(pramJobj, "X_ZYXEL_WPS_EnableStaPin", JSON_OBJ_COPY(enableStaPin));
	json_object_object_add(pramJobj, "X_ZYXEL_WPS_EnableAPPin", JSON_OBJ_COPY(enableAPPin));
	json_object_object_add(pramJobj, "X_ZYXEL_WPS_DevicePin", JSON_OBJ_COPY(devicePin));
	json_object_object_add(pramJobj, "X_ZYXEL_WPS_Configured", JSON_OBJ_COPY(wpsConfigured));
	json_object_object_add(pramJobj, "X_ZYXEL_WPS_V2", JSON_OBJ_COPY(json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_V2")));
	json_object_object_add(pramJobj, "ModeEnabled", JSON_OBJ_COPY(wpsSecMode));
	json_object_object_add(pramJobj, "X_ZYXEL_WlMode", JSON_OBJ_COPY(wlMode));
	json_object_array_add(Jarray, pramJobj);
	zcfgFeJsonObjFree(WifiApWpsJobj);
	zcfgFeJsonObjFree(WifiApSecJobj);
	zcfgFeJsonObjFree(WifiRadioJobj);

    if(support5G){

		// Band: 5GHz
		IID_INIT(objIid);
		objIid.level = 1;
		objIid.idx[0] = (TOTAL_INTERFACE_24G + 1);
		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_WPS, &objIid, &WifiApWpsJobj)) != ZCFG_SUCCESS) {
			printf("%s : Get object fail\n",__FUNCTION__);
			return ret;
		}
		enable = json_object_object_get(WifiApWpsJobj, "Enable");
		enablePBC = json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_EnablePBC");
		enableStaPin = json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_EnableStaPin");
		enableAPPin = json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_EnableAPPin");
		devicePin = json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_DevicePin");
		wpsConfigured = json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_Configured");
		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &objIid, &WifiApSecJobj)) != ZCFG_SUCCESS) {
            printf("%s : Get object fail\n",__FUNCTION__);
            return ret;
        }
        wpsSecMode = json_object_object_get(WifiApSecJobj, "ModeEnabled");

		objIid.idx[0] = 2;
		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &objIid, &WifiRadioJobj)) != ZCFG_SUCCESS) {
            printf("%s : Get object fail\n",__FUNCTION__);
            return ret;
        }
        wlMode = json_object_object_get(WifiRadioJobj, "X_ZYXEL_WlMode");

		pramJobj = json_object_new_object();
		json_object_object_add(pramJobj, "Band", json_object_new_string("5GHz"));
		json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(enable));
		json_object_object_add(pramJobj, "X_ZYXEL_WPS_EnablePBC", JSON_OBJ_COPY(enablePBC));
		json_object_object_add(pramJobj, "X_ZYXEL_WPS_EnableStaPin", JSON_OBJ_COPY(enableStaPin));
		json_object_object_add(pramJobj, "X_ZYXEL_WPS_EnableAPPin", JSON_OBJ_COPY(enableAPPin));
		json_object_object_add(pramJobj, "X_ZYXEL_WPS_DevicePin", JSON_OBJ_COPY(devicePin));
		json_object_object_add(pramJobj, "X_ZYXEL_WPS_Configured", JSON_OBJ_COPY(wpsConfigured));
		json_object_object_add(pramJobj, "X_ZYXEL_WPS_V2", JSON_OBJ_COPY(json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_V2")));
		json_object_object_add(pramJobj, "ModeEnabled", JSON_OBJ_COPY(wpsSecMode));
        json_object_object_add(pramJobj, "X_ZYXEL_WlMode", JSON_OBJ_COPY(wlMode));
		json_object_array_add(Jarray, pramJobj);
		zcfgFeJsonObjFree(WifiApWpsJobj);
		zcfgFeJsonObjFree(WifiApSecJobj);
		zcfgFeJsonObjFree(WifiRadioJobj);
    }

	return ret;
}

zcfgRet_t zcfgFeDalWifiWpsEdit(struct json_object *Jobj, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *WifiApWpsJobj = NULL;
	struct json_object *enable = NULL;
	struct json_object *enablePBC = NULL;
	struct json_object *enableStaPin = NULL;
	struct json_object *enableAPPin = NULL;
	struct json_object *genDevicePin = NULL;
	struct json_object *stationPin = NULL;
	struct json_object *configured = NULL;
	struct json_object *method = NULL;
	struct json_object *configMethodsEnabled = NULL;
	char *wpsBand = NULL;
	// for wps apply check
	struct json_object *tmpObj = NULL;
	char *secMode = NULL;
	bool enableAPPin_ori = false,enableStaPin_ori = false, enablepbc_ori = false, enable_ori = false;
	bool enable_js = false, enablepbc_js = false, enableAPPin_js = false, enableStaPin_js = false, apply = false;
	wpsBand = (char *)json_object_get_string(json_object_object_get(Jobj, "Band"));
	IID_INIT(objIid);
	objIid.level = 1;
	if(!strcmp(wpsBand,"2.4GHz"))
		objIid.idx[0] = 1;	
	else if(!strcmp(wpsBand,"5GHz"))
		objIid.idx[0] = (TOTAL_INTERFACE_24G + 1);
	else{
		printf("\"Band\" value should be \"2.4GHz\" or \"5GHz\".\n");
		return ZCFG_INVALID_PARAM_VALUE;
	}
	
	// wps apply check
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &objIid, &tmpObj)) != ZCFG_SUCCESS){
		return ret;
	}
	secMode = (char *)json_object_get_string(json_object_object_get(tmpObj,"ModeEnabled"));
	if(!strcmp(secMode,"WEP-64") || !strcmp(secMode,"WEP-128")){
		printf("WPS does not support WEP mode, please change.\n");
		zcfgFeJsonObjFree(tmpObj);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	else if(!strcmp(secMode,"WPA-Personal")){
		printf("WPS does not support WPA-Personal mode, please change.\n");
		zcfgFeJsonObjFree(tmpObj);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	else if(!strcmp(secMode,"WPA3-Personal")){
		printf("WPS does not support WPA3-Personal mode, please change.\n");
		zcfgFeJsonObjFree(tmpObj);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	else{
		zcfgFeJsonObjFree(tmpObj);
	}
	
	// wps apply check
#if 0	/* Austin, 2019-02-19 10:58:36, fail to disable 5G WPS when hide ssid was enabled */
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT, &objIid, &tmpObj)) != ZCFG_SUCCESS){
		return ret;
	}
	if(!json_object_get_boolean(json_object_object_get(tmpObj,"SSIDAdvertisementEnabled"))){
		printf("WPS 2.0 does not support Hide SSID. Please deselect \"Hide SSID\" in Wireless -> General page.\n");
		zcfgFeJsonObjFree(tmpObj);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	else{
		zcfgFeJsonObjFree(tmpObj);
	}
#endif
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_WPS, &objIid, &WifiApWpsJobj)) != ZCFG_SUCCESS) {
		return ret;
	}
	enable_ori = json_object_get_boolean(json_object_object_get(WifiApWpsJobj, "Enable"));
	enablepbc_ori = json_object_get_boolean(json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_EnablePBC"));
	enableStaPin_ori = json_object_get_boolean(json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_EnableStaPin"));
	enableAPPin_ori = json_object_get_boolean(json_object_object_get(WifiApWpsJobj, "X_ZYXEL_WPS_EnableAPPin"));
	if( (enable = json_object_object_get(Jobj,"Enable")) != NULL ) {
		enable_js = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		if (enable_js != enable_ori){
			apply = true;
		}
		json_object_object_add(WifiApWpsJobj,"Enable", JSON_OBJ_COPY(enable));
	}
	if( (enablePBC = json_object_object_get(Jobj,"EnablePBC")) != NULL ) {
		enablepbc_js = json_object_get_boolean(json_object_object_get(Jobj, "EnablePBC"));
		if (enablepbc_js != enablepbc_ori){
			apply = true;
		}
		json_object_object_add(WifiApWpsJobj,"X_ZYXEL_WPS_EnablePBC", JSON_OBJ_COPY(enablePBC));
	}
	if( (enableStaPin = json_object_object_get(Jobj,"EnableStaPin")) != NULL ) {
		enableStaPin_js = json_object_get_boolean(json_object_object_get(Jobj, "EnableStaPin"));
		if (enableStaPin_js != enableStaPin_ori){
			apply = true;
		}
		json_object_object_add(WifiApWpsJobj,"X_ZYXEL_WPS_EnableStaPin", JSON_OBJ_COPY(enableStaPin));
	}
	if( (enableAPPin = json_object_object_get(Jobj,"EnableAPPin")) != NULL ) {
		enableAPPin_js = json_object_get_boolean(json_object_object_get(Jobj, "EnableAPPin"));
		if (enableAPPin_js != enableAPPin_ori){
			apply = true;
		}
		json_object_object_add(WifiApWpsJobj,"X_ZYXEL_WPS_EnableAPPin", JSON_OBJ_COPY(enableAPPin));
#ifdef ZYXEL_TAAAB_WPSSETUPLOCK
		if((json_object_get_boolean(enableAPPin) == true)) {
			json_object_object_add(WifiApWpsJobj, "X_ZYXEL_WPS_APPinState", json_object_new_string("UnLocked"));
		}
		else
		{
			json_object_object_add(WifiApWpsJobj, "X_ZYXEL_WPS_APPinState", json_object_new_string("LockedByLocalManagement"));
		}
#endif
	}
	if( (genDevicePin = json_object_object_get(Jobj,"X_ZYXEL_WPS_GenDevicePin")) != NULL ) {
		apply = true;
		json_object_object_add(WifiApWpsJobj,"X_ZYXEL_WPS_GenDevicePin", JSON_OBJ_COPY(genDevicePin));
	}
	if( (stationPin = json_object_object_get(Jobj,"X_ZYXEL_WPS_StationPin")) != NULL ) {
		apply = true;
		json_object_object_add(WifiApWpsJobj,"X_ZYXEL_WPS_StationPin", JSON_OBJ_COPY(stationPin));
	}
	if( (configured = json_object_object_get(Jobj,"X_ZYXEL_WPS_Configured")) != NULL ) {
		apply = true;
		json_object_object_add(WifiApWpsJobj,"X_ZYXEL_WPS_Configured", JSON_OBJ_COPY(configured));
	}
	if( (method = json_object_object_get(Jobj,"X_ZYXEL_WPS_Method")) != NULL ) {
		json_object_object_add(WifiApWpsJobj,"X_ZYXEL_WPS_Method", JSON_OBJ_COPY(method));
	}
	if( (configMethodsEnabled = json_object_object_get(Jobj,"ConfigMethodsEnabled")) != NULL ) {
		json_object_object_add(WifiApWpsJobj,"ConfigMethodsEnabled", JSON_OBJ_COPY(configMethodsEnabled));
	}
	if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")), "GUI")){
		if(apply)
			if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_WIFI_ACCESS_POINT_WPS, &objIid, WifiApWpsJobj, NULL)) != ZCFG_SUCCESS);
				json_object_put(WifiApWpsJobj);
	}
	else{
		if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_WIFI_ACCESS_POINT_WPS, &objIid, WifiApWpsJobj, NULL)) != ZCFG_SUCCESS);
		 	json_object_put(WifiApWpsJobj);
	}
	return ret;
}

zcfgRet_t zcfgFeDalWifiWps(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalWifiWpsEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalWifiWpsGet(Jobj, Jarray, NULL);
	}
	else 
		printf("Unknown method:%s\n", method);

	return ret;
}
