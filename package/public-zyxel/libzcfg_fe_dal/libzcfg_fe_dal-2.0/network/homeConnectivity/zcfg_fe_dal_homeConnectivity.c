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

dal_param_t HOME_CONNECTIVYITY_param[]={
	{"one_connect_Enable", 			dalType_boolean,	0,	0,	NULL,	NULL,	0},
#ifdef ZYXEL_ONECONNECT_PARENTCTL_OPTION
	{"IsParenCtlFromGUI", 			dalType_boolean,	0,	0,	NULL,	NULL,	0},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
	{"WiFi_Enable", 				dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"WiFi_24G_NormalToBad_Threshold", 	dalType_int,		0,	0,	NULL,	NULL,	0},
	{"WiFi_24G_NormalToGood_Threshold", dalType_int,		0,	0,	NULL,	NULL,	0},
	{"WiFi_5G_NormalToBad_Threshold", 	dalType_int,		0,	0,	NULL,	NULL,	0},
	{"WiFi_5G_NormalToGood_Threshold",	dalType_int,		0,	0,	NULL,	NULL,	0},
	{"WiFi_24G_Upgrade_Threshold", 		dalType_int,		0,	0,	NULL,	NULL,	0},
#endif
	{NULL,					0,	0,	0,	NULL,	NULL,	0},
};

void zcfgFeDalShowHomeConnect(struct json_object *Jarray){
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	printf("One Connect Setup: \n");
	printf("%-30s %-10s \n", "One Connect:", json_object_get_string(json_object_object_get(obj, "one_connect_Enable")));
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
	printf("\nOne Connect Setup: \n");
	printf("%-30s %-10s \n", "Wifi:", json_object_get_string(json_object_object_get(obj, "AP_Steering_Enable")));
	printf("\nSNR Thresholds: \n");
	printf("%-30s %-10s \n", "2.4G_NormalToBad_Threshold(dB):", json_object_get_string(json_object_object_get(obj, "WiFi_24G_NormalToBad_Threshold")));
	printf("%-30s %-10s \n", "2.4G_NormalToGood_Threshold(dB):", json_object_get_string(json_object_object_get(obj, "WiFi_24G_NormalToGood_Threshold")));
	printf("%-30s %-10s \n", "5G_NormalToBad_Threshold(dB):", json_object_get_string(json_object_object_get(obj, "WiFi_5G_NormalToBad_Threshold")));
	printf("%-30s %-10s \n", "5G_NormalToGood_Threshold(dB):", json_object_get_string(json_object_object_get(obj, "WiFi_5G_NormalToGood_Threshold")));
	printf("%-30s %-10s \n", "2.4G_Upgrade_Threshold(dB):", json_object_get_string(json_object_object_get(obj, "WiFi_24G_Upgrade_Threshold")));
#endif

}


zcfgRet_t zcfgFeDal_Home_Connect_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
	struct json_object *oneconnectObj = NULL;
	objIndex_t oneconnectIid = {0};
	bool oneconnect_Enable = false;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
	struct json_object *wifiObj = NULL;
	objIndex_t wifiIid = {0};
#endif
#ifdef ZYXEL_ONECONNECT_PARENTCTL_OPTION
	bool IsParenCtlFromGUI = false;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
	bool wifi_Enable = false;
	int wifi_24G_NormalToBad_Threshold = 0;
	int wifi_24G_NormalToGood_Threshold = 0;
	int wifi_5G_NormalToBad_Threshold = 0;
	int wifi_5G_NormalToGood_Threshold = 0;
	int wifi_24G_Upgrade_Threshold = 0;

	IID_INIT(wifiIid);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
	IID_INIT(oneconnectIid);
	oneconnect_Enable = json_object_get_boolean(json_object_object_get(Jobj, "one_connect_Enable"));
#endif
#ifdef ZYXEL_ONECONNECT_PARENTCTL_OPTION
	IsParenCtlFromGUI = json_object_get_boolean(json_object_object_get(Jobj, "IsParenCtlFromGUI"));
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
	wifi_Enable = json_object_get_boolean(json_object_object_get(Jobj, "WiFi_Enable"));
	wifi_24G_NormalToBad_Threshold = json_object_get_int(json_object_object_get(Jobj, "WiFi_24G_NormalToBad_Threshold"));
	wifi_24G_NormalToGood_Threshold = json_object_get_int(json_object_object_get(Jobj, "WiFi_24G_NormalToGood_Threshold"));
	wifi_5G_NormalToBad_Threshold = json_object_get_int(json_object_object_get(Jobj, "WiFi_5G_NormalToBad_Threshold"));
	wifi_5G_NormalToGood_Threshold = json_object_get_int(json_object_object_get(Jobj, "WiFi_5G_NormalToGood_Threshold"));
	wifi_24G_Upgrade_Threshold = json_object_get_int(json_object_object_get(Jobj, "WiFi_24G_Upgrade_Threshold"));
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
	if(zcfgFeObjJsonGet(RDM_OID_ZY_ONE_CONNECT, &oneconnectIid, &oneconnectObj) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "one_connect_Enable"))
			json_object_object_add(oneconnectObj, "Enable", json_object_new_boolean(oneconnect_Enable));
#ifdef ZYXEL_ONECONNECT_PARENTCTL_OPTION
		if(json_object_object_get(Jobj, "IsParenCtlFromGUI"))
			json_object_object_add(oneconnectObj, "IsParenCtlFromGUI", json_object_new_boolean(IsParenCtlFromGUI));
#endif
		zcfgFeObjJsonSet(RDM_OID_ZY_ONE_CONNECT, &oneconnectIid, oneconnectObj, NULL);
		zcfgFeJsonObjFree(oneconnectObj);
	}
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
	if(zcfgFeObjJsonGet(RDM_OID_WIFI_AP_STEERING, &wifiIid, &wifiObj) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "WiFi_Enable"))
		json_object_object_add(wifiObj, "Enable", json_object_new_boolean(wifi_Enable));
		if(json_object_object_get(Jobj, "WiFi_24G_NormalToBad_Threshold"))
		json_object_object_add(wifiObj, "WiFi_24G_NormalToBad_Threshold", json_object_new_int(wifi_24G_NormalToBad_Threshold));
		if(json_object_object_get(Jobj, "WiFi_24G_NormalToGood_Threshold"))
		json_object_object_add(wifiObj, "WiFi_24G_NormalToGood_Threshold", json_object_new_int(wifi_24G_NormalToGood_Threshold));
		if(json_object_object_get(Jobj, "WiFi_5G_NormalToBad_Threshold"))
		json_object_object_add(wifiObj, "WiFi_24G_Upgrade_Threshold", json_object_new_int(wifi_24G_Upgrade_Threshold));
		if(json_object_object_get(Jobj, "WiFi_5G_NormalToGood_Threshold"))
		json_object_object_add(wifiObj, "WiFi_5G_NormalToBad_Threshold", json_object_new_int(wifi_5G_NormalToBad_Threshold));
		if(json_object_object_get(Jobj, "WiFi_24G_Upgrade_Threshold"))
		json_object_object_add(wifiObj, "WiFi_5G_NormalToGood_Threshold", json_object_new_int(wifi_5G_NormalToGood_Threshold));
		zcfgFeObjJsonSet(RDM_OID_WIFI_AP_STEERING, &wifiIid, wifiObj, NULL);
		zcfgFeJsonObjFree(wifiObj);
	}
#endif
	return ret;
}

zcfgRet_t zcfgFeDal_Home_Connect_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
	struct json_object *oneconnectObj = NULL;
	objIndex_t oneconnectIid = {0};
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
	struct json_object *apsteeringObj = NULL;
	objIndex_t apsteeringIid = {0};
#endif
	paramJobj = json_object_new_object();
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_ONE_CONNECT, &oneconnectIid, &oneconnectObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "one_connect_Enable", JSON_OBJ_COPY(json_object_object_get(oneconnectObj, "Enable")));
#ifdef ZYXEL_ONECONNECT_PARENTCTL_OPTION
		json_object_object_add(paramJobj, "IsParenCtlFromGUI", JSON_OBJ_COPY(json_object_object_get(oneconnectObj, "IsParenCtlFromGUI")));
#endif
		zcfgFeJsonObjFree(oneconnectObj);
	}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_AP_STEERING, &apsteeringIid, &apsteeringObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "AP_Steering_Enable", JSON_OBJ_COPY(json_object_object_get(apsteeringObj, "Enable")));
		json_object_object_add(paramJobj, "WiFi_24G_NormalToBad_Threshold", JSON_OBJ_COPY(json_object_object_get(apsteeringObj, "WiFi_24G_NormalToBad_Threshold")));
		json_object_object_add(paramJobj, "WiFi_24G_NormalToGood_Threshold", JSON_OBJ_COPY(json_object_object_get(apsteeringObj, "WiFi_24G_NormalToGood_Threshold")));
		json_object_object_add(paramJobj, "WiFi_5G_NormalToBad_Threshold", JSON_OBJ_COPY(json_object_object_get(apsteeringObj, "WiFi_5G_NormalToBad_Threshold")));
		json_object_object_add(paramJobj, "WiFi_5G_NormalToGood_Threshold", JSON_OBJ_COPY(json_object_object_get(apsteeringObj, "WiFi_5G_NormalToGood_Threshold")));
		json_object_object_add(paramJobj, "WiFi_24G_Upgrade_Threshold", JSON_OBJ_COPY(json_object_object_get(apsteeringObj, "WiFi_24G_Upgrade_Threshold")));
		zcfgFeJsonObjFree(apsteeringObj);
	}
#endif
	json_object_array_add(Jarray, paramJobj);
	return ret;
}


zcfgRet_t zcfgFeDalHomeConnectivity(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_Home_Connect_Edit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_Home_Connect_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
