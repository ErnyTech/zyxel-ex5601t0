#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t WIFI_WMM_param[] =
{
	//WiFi Access Point
	{"Index",			dalType_int,		0,	0,	NULL,	NULL,	dal_Edit}, // CLI
	{"WmmEnable",		dalType_boolean,	0,	0,	NULL,	NULL,	0}, // CLI
	{"UapsdEnable",		dalType_boolean,	0,	0,	NULL,	NULL,	0}, // CLI
	{"WMMEnable",		dalType_boolean,	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"UAPSDEnable",		dalType_boolean,	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	//RDM_OID_WIFI_RADIO
	{"OperatingStandards",	dalType_string,	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{NULL,		0,	0,	0,	NULL,	NULL,	0}
};

void zcfgFeDalShowWifiWmm(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-10s %-15s %-25s\n","Index","WMM Status","WMM APSD Status");
	printf("---------------------------------------------\n");

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj!=NULL){
			printf("%-10s ",json_object_to_json_string(json_object_object_get(obj, "Index")));
			printf("%-15s ",json_object_to_json_string(json_object_object_get(obj, "WMMEnable")));
			printf("%-25s ",json_object_to_json_string(json_object_object_get(obj, "UAPSDEnable")));
			printf("\n");
		}
	}
}

zcfgRet_t zcfgFeDalWifiWmmGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *obj = NULL;
	objIndex_t iid, ssidIid;
	struct json_object *wirelessMode24G = NULL;
	struct json_object *wirelessMode5G = NULL;
	struct json_object *wifiRadioObj = NULL;
	struct json_object *ssidObj = NULL;
	const char *SSIDReference = NULL;
	bool hiddenIntf = false;

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_RADIO, &iid, &wifiRadioObj) == ZCFG_SUCCESS){
#if 0 //IChia 20190906
		if(iid.idx[0] == 1){
			wirelessMode24G = JSON_OBJ_COPY(json_object_object_get(wifiRadioObj, "OperatingStandards"));
		}else if(iid.idx[0] == 2){
			wirelessMode5G = JSON_OBJ_COPY(json_object_object_get(wifiRadioObj, "OperatingStandards"));
		}
		zcfgFeJsonObjFree(wifiRadioObj);
#else
		if(iid.idx[0] == 1)
		{
			wirelessMode24G = json_object_new_string(json_object_get_string(json_object_object_get(wifiRadioObj, "OperatingStandards")));
		}else if(iid.idx[0] == 2)
		{
			wirelessMode5G = json_object_new_string(json_object_get_string(json_object_object_get(wifiRadioObj, "OperatingStandards")));
		}
		zcfgFeJsonObjFree(wifiRadioObj);
#endif
	}

	IID_INIT(iid);
	int count = 1;
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &iid, &obj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index", json_object_new_int(count));
#if 0 //IChia 20190906
		json_object_object_add(paramJobj, "WMMEnable", JSON_OBJ_COPY(json_object_object_get(obj,"WMMEnable")));
		json_object_object_add(paramJobj, "UAPSDEnable", JSON_OBJ_COPY(json_object_object_get(obj,"UAPSDEnable")));
#else
		json_object_object_add(paramJobj, "WMMEnable",	json_object_new_boolean(json_object_get_boolean(json_object_object_get(obj,"WMMEnable"))));
		json_object_object_add(paramJobj, "UAPSDEnable", json_object_new_boolean(json_object_get_boolean(json_object_object_get(obj,"UAPSDEnable"))));
#endif
#if 0 //IChia 20190906
		if(iid.idx[0] <= TOTAL_INTERFACE_24G){
			json_object_object_add(paramJobj, "OperatingStandards", wirelessMode24G);
		}
		else if(iid.idx[0] >= (TOTAL_INTERFACE_24G + 1) && iid.idx[0] <= (TOTAL_INTERFACE_24G + TOTAL_INTERFACE_5G_ssid)){
			json_object_object_add(paramJobj, "OperatingStandards", wirelessMode5G);
		}else{
			json_object_object_add(paramJobj, "OperatingStandards", json_object_new_string(""));
		}
#else
		IID_INIT(ssidIid);
		ssidIid.level = 1;
		if((SSIDReference = json_object_get_string(json_object_object_get(obj, "SSIDReference"))) != NULL
			&& sscanf(SSIDReference, "WiFi.SSID.%hhu", &(ssidIid.idx[0])) == 1
			&& zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &ssidIid, &ssidObj) == ZCFG_SUCCESS) {
			if(json_object_get_int(json_object_object_get(ssidObj, "X_ZYXEL_Attribute")))
				hiddenIntf = true;
			zcfgFeJsonObjFree(ssidObj);
		}

		if(!hiddenIntf && iid.idx[0] <= TOTAL_INTERFACE_24G){
			json_object_object_add(paramJobj, "OperatingStandards", json_object_new_string(json_object_get_string(wirelessMode24G)));
		}
		else if(!hiddenIntf && iid.idx[0] >= (TOTAL_INTERFACE_24G + 1) && iid.idx[0] <= (TOTAL_INTERFACE_24G + TOTAL_INTERFACE_5G))
		{
			json_object_object_add(paramJobj, "OperatingStandards", json_object_new_string(json_object_get_string(wirelessMode5G)));
		}else{
			json_object_object_add(paramJobj, "OperatingStandards", json_object_new_string(""));
		}
#endif
		json_object_array_add(Jarray, paramJobj);
		hiddenIntf = false;
		count++;
		zcfgFeJsonObjFree(obj);
	}
#if 1 //IChia 20190906
	zcfgFeJsonObjFree(wirelessMode24G);
	zcfgFeJsonObjFree(wirelessMode5G);
#endif
	return ret;
}

zcfgRet_t prepareWifiWmmObj(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	// if GUI, no need to prepare
	if(!json_object_object_get(Jobj, "Index"))
		return ZCFG_SUCCESS;

	// Jobj
	bool hasWmmEn;
	bool hasUAPSDEn;
	int index;
	bool wmmEn = false;
	bool uapsdEn = false;

	// iidArray[]
	struct json_object *ssidObj = NULL;
	objIndex_t ssidIid;
	char lowerLayer[15]={0};
	struct json_object *iidArray = NULL;

	// wmmEnableArray[]
	struct json_object *wmmEnableArray = NULL;
	wmmEnableArray = json_object_new_array();
	int len;
	int iidIdx, idx;
	bool enable;
	struct json_object *apObj = NULL;
	objIndex_t apIid;
	
	// Jobj
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	if(json_object_object_get(Jobj, "WmmEnable")){
		hasWmmEn = true;
		wmmEn = json_object_get_boolean(json_object_object_get(Jobj, "WmmEnable"));
	}
	else{
		hasWmmEn = false;
	}
	if(json_object_object_get(Jobj, "UapsdEnable")){
		hasUAPSDEn = true;
		uapsdEn = json_object_get_boolean(json_object_object_get(Jobj, "UapsdEnable"));
	}
	else{
		hasUAPSDEn = false;
	}

	// iidArray[]
	iidArray = json_object_new_array();
	IID_INIT(ssidIid);
	ssidIid.level = 1;
	ssidIid.idx[0] = index;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &ssidIid, &ssidObj) == ZCFG_SUCCESS){
		strcpy(lowerLayer,json_object_get_string(json_object_object_get(ssidObj, "LowerLayers")));
		zcfgFeJsonObjFree(ssidObj);
	}
	IID_INIT(ssidIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &ssidIid, &ssidObj) == ZCFG_SUCCESS){
		if(!strcmp(lowerLayer,json_object_get_string(json_object_object_get(ssidObj, "LowerLayers")))){
			json_object_array_add(iidArray, json_object_new_int(ssidIid.idx[0]));
		}
		zcfgFeJsonObjFree(ssidObj);
	}

	// wmmEnableArray[]
	len = json_object_array_length(iidArray);
	for(iidIdx = 0; iidIdx < len; iidIdx++){
		idx = json_object_get_int( json_object_array_get_idx(iidArray, iidIdx));
		IID_INIT(apIid);
		apIid.level = 1;
		apIid.idx[0] = idx;
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apObj) == ZCFG_SUCCESS){

			enable = json_object_get_boolean(json_object_object_get(apObj, "WMMEnable"));

			if(hasWmmEn && (apIid.idx[0]==index))
				json_object_array_add(wmmEnableArray, json_object_new_boolean(wmmEn));
			else
				json_object_array_add(wmmEnableArray, json_object_new_boolean(enable));

			if(!hasUAPSDEn){
				uapsdEn = json_object_get_boolean(json_object_object_get(apObj, "UAPSDEnable"));
				hasUAPSDEn = true;
			}
			
			zcfgFeJsonObjFree(apObj);
		}
	}
	
	json_object_object_add(Jobj, "UAPSDEnable", json_object_new_boolean(uapsdEn));
	json_object_object_add(Jobj, "WMMEnable", JSON_OBJ_COPY(wmmEnableArray));
	json_object_object_add(Jobj, "startidxarray", JSON_OBJ_COPY(iidArray));
	if(json_object_object_get(Jobj, "WmmEnable"))
		json_object_object_del(Jobj, "WmmEnable");
	if(json_object_object_get(Jobj, "UapsdEnable"))
		json_object_object_del(Jobj, "UapsdEnable");

	return ret;
}

zcfgRet_t zcfgFeDalWifiWmmEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *WifiApWmmJobj = NULL;
	struct json_object *wmmEnableArray = NULL;
	struct json_object *iidArray = NULL;
	int iidIdx = 0;
	int WMMEnable;
	int len = 0;
	int idx = 0;
	bool mainUapsdEnable = true;
	int mainWmmEnable = 0;

	// forceWMMEnable related
	objIndex_t ssidIid;
	objIndex_t radioIid;
	uint8_t tmpRadioIdx;
	struct json_object *ssidObj = NULL;
	struct json_object *radioObj = NULL;
	char* lowerLayer = NULL; // e.g. WiFi.Radio.2
	char* wlMode = NULL; // e.g. 11anac
	bool forceEnableWMM = false;
	bool printForceEnableMsg = false, apply = false;
	bool WMMEnable_ori = false, UAPSDEnable_ori = false, wmmEn = false, UAPSDEn = false;
	wmmEnableArray = json_object_object_get(Jobj, "WMMEnable");
	iidArray = json_object_object_get(Jobj, "startidxarray");
	mainUapsdEnable = json_object_get_int(json_object_object_get(Jobj, "UAPSDEnable"));
	len = json_object_array_length(iidArray);
	mainWmmEnable = json_object_get_int( json_object_array_get_idx(wmmEnableArray, 0));

	// Check if need forceWMMEnable
	IID_INIT(ssidIid);
	ssidIid.level = 1;
	ssidIid.idx[0] = json_object_get_int( json_object_array_get_idx(iidArray, 0));
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_SSID, &ssidIid, &ssidObj)) == ZCFG_SUCCESS){
		lowerLayer = (char *)json_object_get_string(json_object_object_get(ssidObj, "LowerLayers"));
		sscanf(lowerLayer, "WiFi.Radio.%hhu", &tmpRadioIdx);
		json_object_put(ssidObj);
	}
	else{
		dbg_printf("%s : Get RDM_OID_WIFI_SSID fail, ret = %d\n",__FUNCTION__,ret);
		return ret;
	}
	IID_INIT(radioIid);
	radioIid.level = 1;
	radioIid.idx[0] = tmpRadioIdx;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &radioIid, &radioObj)) == ZCFG_SUCCESS){
		wlMode = (char *)json_object_get_string(json_object_object_get(radioObj, "OperatingStandards"));
		if(wlMode == NULL){
			printf("%s : Get wireless mode error.\n",__FUNCTION__);
			json_object_put(radioObj);
			return ZCFG_NO_SUCH_PARAMETER;
		}
		else if((strchr(wlMode, 'n') != NULL) || (strstr(wlMode, "ac") != NULL)) {
			forceEnableWMM = true;
		}
		json_object_put(radioObj);
	}
	else{
		dbg_printf("%s : Get RDM_OID_WIFI_RADIO fail, ret = %d\n",__FUNCTION__,ret);
		return ret;
	}

	// printf warning message
	for(iidIdx = 0; iidIdx < len; iidIdx++){
		if(forceEnableWMM && !(json_object_get_int(json_object_array_get_idx(wmmEnableArray, iidIdx))))
			printForceEnableMsg = true;
	}
	if(printForceEnableMsg) 
		printf("WMM is mandatory to be enabled if 802.11 mode includes 802.11n or 802.11ac\n");

	IID_INIT(objIid);
	// Edit Guest SSIDs
	for(iidIdx = 1; iidIdx < len; iidIdx++){
		WMMEnable = json_object_get_int( json_object_array_get_idx(wmmEnableArray, iidIdx));
		idx = json_object_get_int( json_object_array_get_idx(iidArray, iidIdx));
		objIid.level = 1;
		objIid.idx[0] = idx;
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &objIid, &WifiApWmmJobj) == ZCFG_SUCCESS){
			WMMEnable_ori = json_object_get_boolean(json_object_object_get(WifiApWmmJobj,"WMMEnable"));
			UAPSDEnable_ori = json_object_get_boolean(json_object_object_get(WifiApWmmJobj, "UAPSDEnable"));
			UAPSDEn = json_object_get_boolean(json_object_object_get(Jobj, "UAPSDEnable"));
			if((WMMEnable == 1) || forceEnableWMM)
				json_object_object_add(WifiApWmmJobj, "WMMEnable", json_object_new_boolean(true));
			else
				json_object_object_add(WifiApWmmJobj, "WMMEnable", json_object_new_boolean(false));

			if(mainUapsdEnable == 1)
				json_object_object_add(WifiApWmmJobj, "UAPSDEnable", json_object_new_boolean(true));
			else
				json_object_object_add(WifiApWmmJobj, "UAPSDEnable", json_object_new_boolean(false));
			if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")), "GUI")){
				if(WMMEnable != WMMEnable_ori || UAPSDEn != UAPSDEnable_ori){
					apply = true;
					zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &objIid, WifiApWmmJobj, NULL);
				}
			}
			else{
				apply = true;
				zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &objIid, WifiApWmmJobj, NULL);
			}
			json_object_put(WifiApWmmJobj);
		}
	}
	
	// Edit Main SSID
	IID_INIT(objIid);
	objIid.level = 1;
	idx = json_object_get_int( json_object_array_get_idx(iidArray, 0));
	objIid.idx[0] = idx;
	if(zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT, &objIid, &WifiApWmmJobj) == ZCFG_SUCCESS){
		WMMEnable_ori = json_object_get_boolean(json_object_object_get(WifiApWmmJobj,"WMMEnable"));
		UAPSDEnable_ori = json_object_get_boolean(json_object_object_get(WifiApWmmJobj, "UAPSDEnable"));
		wmmEn = json_object_get_int( json_object_array_get_idx(wmmEnableArray, 0));
		UAPSDEn = json_object_get_boolean(json_object_object_get(Jobj, "UAPSDEnable"));
		if((mainWmmEnable == 1) || forceEnableWMM)
			json_object_object_add(WifiApWmmJobj, "WMMEnable", json_object_new_boolean(true));
		else
			json_object_object_add(WifiApWmmJobj, "WMMEnable", json_object_new_boolean(false));
		if(mainUapsdEnable == 1)
			json_object_object_add(WifiApWmmJobj, "UAPSDEnable", json_object_new_boolean(true));
		else
			json_object_object_add(WifiApWmmJobj, "UAPSDEnable", json_object_new_boolean(false));
	if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")), "GUI")){
		if(wmmEn != WMMEnable_ori || UAPSDEn != UAPSDEnable_ori)
			apply = true;
	}
	else
		apply = true;

	if(apply){
		zcfgFeObjJsonBlockedSet(RDM_OID_WIFI_ACCESS_POINT, &objIid, WifiApWmmJobj, NULL);
	}
	json_object_put(WifiApWmmJobj);
	}
	return ret;
}
zcfgRet_t zcfgFeDalWifiWmm(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;
	
	if(!strcmp(method, "PUT")) {
		if(prepareWifiWmmObj(Jobj, NULL)){
			printf("%s : failed preparing WMM object\n",__FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
		}
		ret = zcfgFeDalWifiWmmEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalWifiWmmGet(Jobj, Jarray, NULL);
	}
	else
		printf("Unknown method:%s\n", method);
	
	return ret;
}
