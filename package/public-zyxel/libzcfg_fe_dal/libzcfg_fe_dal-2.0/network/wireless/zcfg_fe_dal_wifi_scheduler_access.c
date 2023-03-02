#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
bool defaultwlanConfig;
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON

dal_param_t WLAN_SCH_ACCESS_param[] =
{
	//Schedule
	{"Enable",		dalType_boolean,	0,	0,	NULL,	NULL,	NULL},		

#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
	{"AutoSwitchWiFiRadio",		dalType_boolean,	0,	0,	NULL,	NULL,	NULL},	
	{"MasterShip",		        dalType_boolean,	0,	0,	NULL,	NULL,	NULL},	
	{"LastConfiguration",		dalType_string, 	0,	0,	NULL,	NULL,	NULL},
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
	{NULL,		0,	0,	0,	NULL,	NULL,	0}
};

void zcfgFeDalShowWlanSchAccess(struct json_object *Jarray){
	int i;
#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
	char *lastConfig = NULL;
#endif
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray,0);
	printf("WLAN Scheduler Access     \n");
	printf("==========================\n");
	printf("WLAN Scheduler Enable    : ");
	if(json_object_get_boolean(json_object_object_get(obj,"Enable")))
		printf("Enabled\n");
	else
		printf("Disabled\n");

#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
	printf("Auto Switch Interface    : ");
	if(json_object_get_boolean(json_object_object_get(obj,"AutoSwitchWiFiRadio"))){
		printf("Enabled\n");
	}
	else{
		printf("Disabled\n");
	}
	
    printf("Scheduler Mastership     : ");
	if(json_object_get_boolean(json_object_object_get(obj,"MasterShip"))){
		printf("Enabled\n");
	}
	else{
		printf("Disabled\n");
	}
	printf("WiFi Last Configuration  : ");
	lastConfig = json_object_get_string(json_object_object_get(obj, "LastConfiguration"));		
	printf("%-30s ", lastConfig);
	
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
	
	printf("\n");
}

zcfgRet_t zcfgFeDalWlanSchedulerAccessGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *wlanSchObj = NULL;
	struct json_object *pramJobj = NULL;
	struct json_object *enable = NULL;

#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
	struct json_object *autoSwitch = NULL;
	struct json_object *masterShip = NULL;
	struct json_object *lastConfig = NULL;
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON

	
	IID_INIT(objIid);
	ret = zcfgFeObjJsonGet(RDM_OID_WLAN_SCHEDULER_ACCESS, &objIid, &wlanSchObj);
	enable = json_object_object_get(wlanSchObj, "Enable");
	
#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
	autoSwitch = json_object_object_get(wlanSchObj, "AutoSwitchWiFiRadio");
	masterShip = json_object_object_get(wlanSchObj, "MasterShip");
	lastConfig = json_object_object_get(wlanSchObj, "LastConfiguration");
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON	

	pramJobj = json_object_new_object();
	json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(enable));

#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
	json_object_object_add(pramJobj, "AutoSwitchWiFiRadio", JSON_OBJ_COPY(autoSwitch));
	json_object_object_add(pramJobj, "MasterShip", JSON_OBJ_COPY(masterShip));
	json_object_object_add(pramJobj, "LastConfiguration", JSON_OBJ_COPY(lastConfig));
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON

	json_object_array_add(Jarray, pramJobj);
	zcfgFeJsonObjFree(wlanSchObj);
	return ret;
}

zcfgRet_t zcfgFeDalWlanSchedulerAccessEdit(struct json_object *Jobj, char *replyMsg){ 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *wlanSchedulerActiveObj = NULL;
	struct json_object *reqObj = NULL;
	objIndex_t schedulerIid = {0};
	bool enable = false;

	struct json_object *objA = NULL;
	struct json_object *objB = NULL;
	reqObj = json_object_object_get(Jobj,"Enable");
	enable = json_object_get_boolean(reqObj);

#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
	defaultwlanConfig = false;
	struct json_object *reqAutoObj = NULL;
	struct json_object *reqMasterShipObj = NULL;
	struct json_object *reqLastConfigObj = NULL;

	bool autoSwitchEnable = false;
	bool masterShip = false;
	
	bool curEnable = false;
	bool curMasterShip = false;
	
	bool bMasterShipCfgConfig = false;
	char *lastConfig = NULL;
	
	reqAutoObj = json_object_object_get(Jobj,"AutoSwitchWiFiRadio");
	reqMasterShipObj = json_object_object_get(Jobj,"MasterShip");
	reqLastConfigObj = json_object_object_get(Jobj,"LastConfiguration");
	
	autoSwitchEnable = json_object_get_boolean(reqAutoObj);
	masterShip = json_object_get_boolean(reqMasterShipObj);
	lastConfig = json_object_get_string(reqLastConfigObj);
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON


	if((ret = zcfgFeObjJsonGet(RDM_OID_WLAN_SCHEDULER_ACCESS, &schedulerIid, &wlanSchedulerActiveObj)) != ZCFG_SUCCESS){
		printf("Get object fail!\n");
		return ret;
	}
	else{
#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
		curEnable = json_object_get_boolean(json_object_object_get(wlanSchedulerActiveObj, "Enable"));
		curMasterShip = json_object_get_boolean(json_object_object_get(wlanSchedulerActiveObj, "MasterShip"));
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON

		if(json_object_object_get(Jobj, "Enable")){
			json_object_object_add(wlanSchedulerActiveObj, "Enable", json_object_new_boolean(enable));
		}
#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
		if(json_object_object_get(Jobj, "AutoSwitchWiFiRadio")){
			json_object_object_add(wlanSchedulerActiveObj, "AutoSwitchWiFiRadio", json_object_new_boolean(autoSwitchEnable));
		}
		if(json_object_object_get(Jobj, "MasterShip")){
			json_object_object_add(wlanSchedulerActiveObj, "MasterShip", json_object_new_boolean(masterShip));
			bMasterShipCfgConfig = true;
		}

		if (bMasterShipCfgConfig == true){
			/* work around
			*************************************
			No config of MasterShip by GUI, just cfg wlan_sch_access commit only,
			but cfg wlan_sch_access single set will be effect to "turn off scheduler and set back wifi last config" function.
			*/
			printf("=== cfg wlan_sch_access MasterShip commit successful ====\n");
			ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_WLAN_SCHEDULER_ACCESS, &schedulerIid, wlanSchedulerActiveObj, NULL);
		}
		else{
			//enbale wlan scheduler set last modify wifi status
			if ((enable == true) && (enable != curEnable) && (curMasterShip == true)){
				if(json_object_object_get(Jobj, "LastConfiguration")){
					json_object_object_add(wlanSchedulerActiveObj, "LastConfiguration", json_object_new_string(lastConfig));
					#if 0 //zcfgFeDalWlanSchedulerAccessEdit
					printf("zcfgFeDalWlanSchedulerAccessEdit : updated wifi lastConfig = %s\n", lastConfig);
					#endif
				}
			}
			else if ((enable == false) && (enable != curEnable) && (curMasterShip == true)){
				if (zcfgFeDalDefaultWiFiConfig(lastConfig) == ZCFG_SUCCESS){
					defaultwlanConfig = true;
				}
			}
			ret = zcfgFeObjJsonSet(RDM_OID_WLAN_SCHEDULER_ACCESS, &schedulerIid, wlanSchedulerActiveObj, NULL);
		}
#else
		ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_WLAN_SCHEDULER_ACCESS, &schedulerIid, wlanSchedulerActiveObj, NULL);
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON

	}
	
	return ret;
}

zcfgRet_t zcfgFeDalDefaultWiFiConfig(char *lastConfig){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *ssidObj = NULL;
	
	const char *del = ",";
	bool enable;
	char *enableStatus[8];
	split(enableStatus, lastConfig, del);

	#if 0 //zcfgFeDalDefaultWiFiConfig
	printf("=== updated >> get last config ====\n");
	printf(">> enableStatus[0] = %s\n", enableStatus[0]);
	printf(">> enableStatus[1] = %s\n", enableStatus[1]);
	printf(">> enableStatus[2] = %s\n", enableStatus[2]);
	printf(">> enableStatus[3] = %s\n", enableStatus[3]);
	printf(">> enableStatus[4] = %s\n", enableStatus[4]);
	printf(">> enableStatus[5] = %s\n", enableStatus[5]);
	printf(">> enableStatus[6] = %s\n", enableStatus[6]);
	printf(">> enableStatus[7] = %s\n", enableStatus[7]);								
	printf("============================\n\n");
	printf("=== updated >> set last config ====\n");
	#endif

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_WIFI_SSID, &iid, &ssidObj) == ZCFG_SUCCESS){
		enable = false;
		if (iid.idx[0] < 9){
			if (json_object_get_int(json_object_object_get(ssidObj, "X_ZYXEL_Attribute")) == 0){				
				#if 0 //zcfgFeDalDefaultWiFiConfig
				if (!strcmp(json_object_get_string(json_object_object_get(ssidObj, "X_ZYXEL_GuestAPs")),"")){
					//GUEST	
					enableStatus = false;
				}
				else{
					//MAIN
					enableStatus = true;
				}
				#endif
				if (!strcmp(enableStatus[iid.idx[0]-1],"1")) {
					enable = true;
				}
				//printf(">> SET ssidObj[%d] = %d\n", iid.idx[0], enable);
				json_object_object_add(ssidObj, "Enable", json_object_new_boolean(enable));
			}
			zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_SSID, &iid, ssidObj, NULL);
			//printf("============================\n\n");
		}
		json_object_put(ssidObj);
	}
	return ret;
	
}

void zcfgFeDalUpdateWiFiConfig(void){
	
	objIndex_t radioIid;
	struct json_object *radioObj = NULL;
	IID_INIT(radioIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_WIFI_SSID, &radioIid, &radioObj) == ZCFG_SUCCESS){	
		if ((radioIid.idx[0] == 1) || (radioIid.idx[0] == 5)) {
			zcfgFeObjJsonBlockedSet(RDM_OID_WIFI_SSID, &radioIid, radioObj, NULL);
		}
		json_object_put(radioObj);
	}
	
}

void split(char **arr, char *str, const char *del)
{
   char *s = strtok(str, del);

   while(s != NULL) {
     *arr++ = s;
     s = strtok(NULL, del);
   }
}

zcfgRet_t zcfgFeDalWlanSchAccess(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
	defaultwlanConfig = false;
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalWlanSchedulerAccessEdit(Jobj, NULL);
#ifdef ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON
		if (defaultwlanConfig == true){
			zcfgFeDalUpdateWiFiConfig();
		}
#endif //ZYXEL_WLAN_SCHEDULER_AUTO_SWITCH_RADIO_ON		
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalWlanSchedulerAccessGet(Jobj, Jarray, NULL);
	}
	return ret;
}
