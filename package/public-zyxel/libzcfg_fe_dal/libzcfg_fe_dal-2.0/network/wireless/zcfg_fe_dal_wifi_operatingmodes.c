#ifdef ZYXEL_OPAL_EXTENDER
#include <json/json.h>
#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_msg.h"

dal_param_t WIFI_OperatingModes_param[] =
{
	//WiFi REPEATER
	{"Mode",                dalType_string,    0,  0,  NULL,        "repeater|ap",                                              0},
   //WiFi SSID
	{"SSID",                dalType_string,     0,  0,  NULL,       NULL,                                                       0},
	//WiFi Access Point Sec
#ifdef ZYXEL_WIFI_SUPPORT_WEP_WPA_MIXMODE
#ifdef ZYXEL_DPP_R2
	{"wpaMode",             dalType_string,     0,  0,  NULL,       "wpapsk|wpawpa2psk|wpa2psk|wpawpa2|wpa3sae|wpa2pskwpa3sae|dpp|wpa3saedpp|wpa2pskwpa3saedpp", 0},
#else//ZYXEL_DPP_R2
	{"wpaMode",             dalType_string,     0,  0,  NULL,       "wpapsk|wpawpa2psk|wpa2psk|wpawpa2|wpa3sae|wpa2pskwpa3sae", 0},
#endif//ZYXEL_DPP_R2
#else //ZYXEL_WIFI_SUPPORT_WEP_WPA_MIXMODE
#ifdef ZYXEL_DPP_R2
	{"wpaMode",             dalType_string,     0,  0,  NULL,       "wpa2psk|wpa2|wpa3sae|wpa2pskwpa3sae|dpp|wpa3saedpp|wpa2pskwpa3saedpp",                      0},
#else//ZYXEL_DPP_R2
	{"wpaMode",             dalType_string,     0,  0,  NULL,       "wpa2psk|wpa2|wpa3sae|wpa2pskwpa3sae",                      0},
#endif//ZYXEL_DPP_R2
#endif
	{"psk_value",           dalType_string,     0,  0,  NULL,       NULL,                                                       0},
	{"secMode",             dalType_int,        0,  0,  NULL,       "43|149|255",                                   dalcmd_Forbid},
	{NULL,      0,  0,  0,  NULL,   NULL,   0}
};

zcfgRet_t zcfgFeDalWifiOperatingModesGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char *SSID, *wpaMode, *psk_value, *mode_enable, *Mode, *SSIDReference, *LowerLayers, *OperatingFrequencyBand;
	objIndex_t epIid, repIid, epProfIid, ssidIid, radioIid;
	struct json_object *epObj = NULL, *epSecObj = NULL, *epWPSObj = NULL, *epProfObj = NULL,*epProfSecObj = NULL, *repObj = NULL, *ssidObj = NULL, *radioObj = NULL;
	struct json_object *pramJobj = NULL, *obj2 = NULL;
	struct json_object *enable = NULL, *enablePBC = NULL, *enableStaPin = NULL, *enableAPPin = NULL, *devicePin = NULL, *wpsConfigured = NULL;

	pramJobj = json_object_new_object();
	IID_INIT(repIid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REPEATER, &repIid, &repObj) == ZCFG_SUCCESS){
		Mode = json_object_get_string(json_object_object_get(repObj, "Mode"));
		json_object_object_add(pramJobj, "Mode", json_object_new_string(Mode));
		zcfgFeJsonObjFree(repObj);
	}
	json_object_array_add(Jarray, pramJobj);
	IID_INIT(epIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_END_POINT, &epIid, &epObj) == ZCFG_SUCCESS) {
		SSIDReference = json_object_get_string(json_object_object_get(epObj,"SSIDReference"));
		obj2 = json_object_new_object();
		json_object_object_add(obj2, "SSIDReference", json_object_new_string(SSIDReference));
		IID_INIT(ssidIid);
		ssidIid.level = 1;
		sscanf(SSIDReference, "WiFi.SSID.%hhd", &ssidIid.idx[0]);
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &ssidIid, &ssidObj) == ZCFG_SUCCESS) {
			LowerLayers = json_object_get_string(json_object_object_get(ssidObj,"LowerLayers"));
			IID_INIT(radioIid);
			radioIid.level = 1;
			sscanf(LowerLayers, "WiFi.Radio.%hhd", &radioIid.idx[0]);
			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_RADIO, &radioIid, &radioObj) == ZCFG_SUCCESS) {
				OperatingFrequencyBand = json_object_object_get(radioObj,"OperatingFrequencyBand");
				json_object_object_add(obj2, "OperatingFrequencyBand", JSON_OBJ_COPY(OperatingFrequencyBand));
				zcfgFeJsonObjFree(radioObj);
			}
			zcfgFeJsonObjFree(ssidObj);
		}
		 if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_END_POINT_SEC, &epIid, &epSecObj) == ZCFG_SUCCESS) {
			wpaMode = json_object_get_string(json_object_object_get(epSecObj, "ModesSupported"));
			json_object_object_add(obj2, "ModesSupported", json_object_new_string(wpaMode));
			zcfgFeJsonObjFree(epSecObj);
		}
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_END_POINT_WPS, &epIid, &epWPSObj) == ZCFG_SUCCESS) {
			enable = json_object_object_get(epWPSObj, "Enable");
			enablePBC = json_object_object_get(epWPSObj, "X_ZYXEL_WPS_EnablePBC");
			enableStaPin = json_object_object_get(epWPSObj, "X_ZYXEL_WPS_EnableStaPin");
			enableAPPin = json_object_object_get(epWPSObj, "X_ZYXEL_WPS_EnableAPPin");
			devicePin = json_object_object_get(epWPSObj, "X_ZYXEL_WPS_DevicePin");
			wpsConfigured = json_object_object_get(epWPSObj, "X_ZYXEL_WPS_Configured");
			json_object_object_add(obj2, "Enable", JSON_OBJ_COPY(enable));
			json_object_object_add(obj2, "X_ZYXEL_WPS_EnablePBC", JSON_OBJ_COPY(enablePBC));
			json_object_object_add(obj2, "X_ZYXEL_WPS_EnableStaPin", JSON_OBJ_COPY(enableStaPin));
			json_object_object_add(obj2, "X_ZYXEL_WPS_EnableAPPin", JSON_OBJ_COPY(enableAPPin));
			json_object_object_add(obj2, "X_ZYXEL_WPS_DevicePin", JSON_OBJ_COPY(devicePin));
			json_object_object_add(obj2, "X_ZYXEL_WPS_Configured", JSON_OBJ_COPY(wpsConfigured));
			zcfgFeJsonObjFree(epWPSObj);
		}
		IID_INIT(epProfIid);
		epProfIid.level = 2;
		sscanf(json_object_get_string(json_object_object_get(epObj,"ProfileReference")), "WiFi.EndPoint.%hhu.Profile.%hhu", &epProfIid.idx[0], &epProfIid.idx[1]);
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_END_POINT_PROFILE, &epProfIid, &epProfObj) == ZCFG_SUCCESS) {
			SSID = json_object_get_string(json_object_object_get(epProfObj, "SSID"));
			json_object_object_add(obj2, "SSID", json_object_new_string(SSID));
			zcfgFeJsonObjFree(epProfObj);
		}
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_END_POINT_PROFILE_SEC, &epProfIid, &epProfSecObj) == ZCFG_SUCCESS) {
			mode_enable = json_object_get_string(json_object_object_get(epProfSecObj, "ModeEnabled"));
			json_object_object_add(obj2, "ModeEnabled", json_object_new_string(mode_enable));
			if(strlen(json_object_get_string(json_object_object_get(epProfSecObj, "KeyPassphrase")) ) > 0)
			{
			    json_object_object_add(obj2, "PskDisplay", JSON_OBJ_COPY(json_object_object_get(epProfSecObj, "KeyPassphrase")));
			}
			else
			{
			    json_object_object_add(obj2, "PskDisplay", JSON_OBJ_COPY(json_object_object_get(epProfSecObj, "PreSharedKey")));
			}
			zcfgFeJsonObjFree(epProfSecObj);
		}
		json_object_array_add(Jarray, obj2);
		zcfgFeJsonObjFree(epObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalWifiOperatingModesEdit(struct json_object *Jobj, char *replyMsg){
	bool apply = true;
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t ssidIid, epIid, repIid, epProfIid;
	struct json_object *ssidObj = NULL, *epProfObj = NULL, *epProfSecObj = NULL, *epSecObj = NULL, *repObj = NULL, *epObj = NULL, *epWpsObj = NULL;
	struct json_object *enable = NULL, *enablePBC = NULL, *enableStaPin = NULL, *enableAPPin = NULL, *X_ZYXEL_WPS_GenDevicePin = NULL;
	unsigned int POS_NONE = 43, POS_BASIC = 149, POS_MORESECURE = 255;
	char ModesSupported[200] = {0};
	char *SSIDReference;
	char *SSID = json_object_get_string(json_object_object_get(Jobj, "SSID"));
	char *psk_value = json_object_get_string(json_object_object_get(Jobj, "psk_value"));
	char *wpaMode = json_object_get_string(json_object_object_get(Jobj, "wpaMode"));
	char *ssidRef = json_object_get_string(json_object_object_get(Jobj, "ssidRef"));
	int  secMode = json_object_get_int(json_object_object_get(Jobj, "secMode"));
	char *Mode = json_object_get_string(json_object_object_get(Jobj, "Mode"));
	char *psk_lowercase = NULL;

	enable = json_object_object_get(Jobj, "enableWPS");
	enablePBC = json_object_object_get(Jobj, "enablePBC");
	enableStaPin = json_object_object_get(Jobj, "enableStaPin");
	enableAPPin = json_object_object_get(Jobj, "enableAPPin");
	X_ZYXEL_WPS_GenDevicePin = json_object_object_get(Jobj, "X_ZYXEL_WPS_GenDevicePin");

	if(ssidRef) {
		IID_INIT(epIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_END_POINT, &epIid, &epObj) == ZCFG_SUCCESS) {
			SSIDReference = json_object_get_string(json_object_object_get(epObj,"SSIDReference"));
			if(!strcmp(ssidRef, SSIDReference)) {
				if(SSID){
					IID_INIT(ssidIid);
					ssidIid.level = 1;
					sscanf(SSIDReference, "WiFi.SSID.%hhd", &ssidIid.idx[0]);
					if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &ssidIid, &ssidObj) == ZCFG_SUCCESS) {
						json_object_object_add(ssidObj, "SSID", json_object_new_string(SSID));
						zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_SSID, &ssidIid, ssidObj, NULL);
						zcfgFeJsonObjFree(ssidObj);
					}
					IID_INIT(epProfIid);
					epProfIid.level = 2;
					sscanf(json_object_get_string(json_object_object_get(epObj,"ProfileReference")), "WiFi.EndPoint.%hhu.Profile.%hhu", &epProfIid.idx[0], &epProfIid.idx[1]);
					if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_END_POINT_PROFILE, &epProfIid, &epProfObj) == ZCFG_SUCCESS) {
						json_object_object_add(epProfObj, "SSID", json_object_new_string(SSID));
						zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_END_POINT_PROFILE, &epProfIid, epProfObj, NULL);
						zcfgFeJsonObjFree(epProfObj);
					}
				}
				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_END_POINT_SEC, &epIid, &epSecObj) == ZCFG_SUCCESS) {
					strcpy(ModesSupported, json_object_get_string(json_object_object_get(epSecObj, "ModesSupported")));
					zcfgFeJsonObjFree(epSecObj);
				}
				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_END_POINT_PROFILE_SEC, &epProfIid, &epProfSecObj) == ZCFG_SUCCESS) {
					if(psk_value) {
						if(strlen(psk_value) == 64) {
							psk_lowercase = (char*)malloc(strlen(psk_value)+1);
							memset(psk_lowercase, 0, strlen(psk_value)+1);
							stolower(psk_value, psk_lowercase, strlen(psk_value));
							json_object_object_add(epProfSecObj, "KeyPassphrase", json_object_new_string(""));
							json_object_object_add(epProfSecObj, "PreSharedKey", json_object_new_string(psk_lowercase));
						}
						else {
							json_object_object_add(epProfSecObj, "KeyPassphrase", json_object_new_string(psk_value));
						}
					}
					if(secMode){
						if(secMode == POS_NONE){
							json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("None"));
						}else if(secMode == POS_MORESECURE){
							if(wpaMode){
								if(!strcmp(wpaMode,"wpa2psk") && strstr(ModesSupported, "WPA2")){
									json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
								}else if(!strcmp(wpaMode,"wpa2") && strstr(ModesSupported, "WPA2")){
									json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("WPA2-Enterprise"));
								}
#ifdef ZYXEL_WIFI_SUPPORT_WEP_WPA_MIXMODE
								else if(!strcmp(wpaMode,"wpapsk") && strstr(ModesSupported, "WPA-")){
									json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("WPA-Personal"));
								}else if(!strcmp(wpaMode,"wpa") && strstr(ModesSupported, "WPA-")){
									json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("WPA-Enterprise"));
								}
#endif
								else if(!strcmp(wpaMode,"wpawpa2psk")){
									if(strstr(ModesSupported, "WPA-")){
										json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
									}else if(strstr(ModesSupported, "WPA2")){
										json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
									}
								}else if(!strcmp(wpaMode,"wpa3sae") && strstr(ModesSupported, "WPA3")){
									json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("WPA3-Personal"));
								}else if(!strcmp(wpaMode,"wpa2pskwpa3sae")){
									if(strstr(ModesSupported, "WPA3")){
										json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("WPA3-Personal-Transition"));
									}else if(strstr(ModesSupported, "WPA2")){
									json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
									}
								}
#ifdef ZYXEL_DPP_R2
								else if(!strcmp(wpaMode,"dpp") && strstr(ModesSupported, "DPP")){
									json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("DPP"));
									json_object_object_add(epProfSecObj, "MFPConfig", json_object_new_string("Required"));
								}else if(!strcmp(wpaMode,"wpa3saedpp") && strstr(ModesSupported, "DPP+WPA3-Personal")){
									json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("DPP+WPA3-Personal"));
									json_object_object_add(epProfSecObj, "MFPConfig", json_object_new_string("Required"));
								}else if(!strcmp(wpaMode,"wpa2pskwpa3saedpp") && strstr(ModesSupported, "DPP+WPA3-Personal-Transition")){
									json_object_object_add(epProfSecObj, "ModeEnabled", json_object_new_string("DPP+WPA3-Personal-Transition"));
									json_object_object_add(epProfSecObj, "MFPConfig", json_object_new_string("Required"));
								}
#endif //ZYXEL_DPP_R2
								else{
									ret = ZCFG_INVALID_PARAM_VALUE;
								}
							}
						}
					}
					zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_END_POINT_PROFILE_SEC, &epProfIid, epProfSecObj, NULL);
					zcfgFeJsonObjFree(epProfSecObj);
				}
				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_END_POINT_WPS, &epIid, &epWpsObj) == ZCFG_SUCCESS) {
					if(enable) json_object_object_add(epWpsObj,"Enable", JSON_OBJ_COPY(enable));
					if(enablePBC) json_object_object_add(epWpsObj,"X_ZYXEL_WPS_EnablePBC", JSON_OBJ_COPY(enablePBC));
					if(enableStaPin) json_object_object_add(epWpsObj,"X_ZYXEL_WPS_EnableStaPin", JSON_OBJ_COPY(enableStaPin));
					if(enableAPPin) json_object_object_add(epWpsObj,"X_ZYXEL_WPS_EnableAPPin", JSON_OBJ_COPY(enableAPPin));
					zcfgFeObjJsonBlockedSet(RDM_OID_WIFI_END_POINT_WPS, &epIid, epWpsObj, NULL);
					if(X_ZYXEL_WPS_GenDevicePin) {
						apply = false;
						json_object_object_add(epWpsObj,"X_ZYXEL_WPS_GenDevicePin", JSON_OBJ_COPY(X_ZYXEL_WPS_GenDevicePin));
						zcfgFeObjJsonBlockedSet(RDM_OID_WIFI_END_POINT_WPS, &epIid, epWpsObj, NULL);
					}
					zcfgFeJsonObjFree(epWpsObj);
				}
				zcfgFeJsonObjFree(epObj);
				break;
			}
			zcfgFeJsonObjFree(epObj);
		}
	}
	if(apply){
		IID_INIT(repIid);
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REPEATER, &repIid, &repObj) == ZCFG_SUCCESS){
			if(Mode) json_object_object_add(repObj,"Mode", json_object_new_string(Mode));
			zcfgFeObjJsonBlockedSet(RDM_OID_REPEATER, &repIid, repObj, NULL);
			zcfgFeJsonObjFree(repObj);
		}
	}


	return ret;
}

void zcfgFeDalShowOperatingModes(struct json_object *Jarray)
{
	int i = 0, len;
	struct json_object *obj;
	const char *OperMode = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	obj = json_object_array_get_idx(Jarray, 0);
	if(obj != NULL){
		OperMode = json_object_get_string(json_object_object_get(obj, "Mode"));
		printf("Operation Mode: %s\n", OperMode);
	}

	if(!strcmp(OperMode, "ap")) {
		return;
	}

	printf("-----------------------------------------------------------------------------------\n");
	printf("%-7s %-10s %-20s %-10s %-35s %-20s %-20s\n", "idx", "Enable", "ssidRef", "Band", "SSID", "SecurityMode", "PskValue");

	len = json_object_array_length(Jarray);
	for(i=1; i<len; i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj != NULL){
			printf("%-7d %-10s %-20s %-10s %-35s %-20s %-20s\n",
				i,
				json_object_get_string(json_object_object_get(obj, "Enable")),
				json_object_get_string(json_object_object_get(obj, "SSIDReference")),
				json_object_get_string(json_object_object_get(obj, "OperatingFrequencyBand")),
				json_object_get_string(json_object_object_get(obj, "SSID")),
				json_object_get_string(json_object_object_get(obj, "ModeEnabled")),
				json_object_get_string(json_object_object_get(obj, "PskDisplay")));
		}
	}

	return;
}

zcfgRet_t zcfgFeDalOperatingModes(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalWifiOperatingModesEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalWifiOperatingModesGet(Jobj, Jarray, NULL);
	}
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
#endif
