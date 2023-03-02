#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t WLAN_CHANNELHOPPING_param[] =
{
	{"Index",		dalType_int,		0,	0,	NULL,	NULL,	NULL},
	{"SSID",		dalType_string,	0,	0,	NULL,	NULL,	NULL},
	{"Enable",	dalType_boolean,	0,	0,	NULL,	NULL,	dal_Edit},
	{"Interval",	dalType_int,		0,	0,	NULL,	NULL,	dal_Edit},
	{NULL,		0,				0,	0,	NULL,	NULL,	0}
};

zcfgRet_t zcfgFeDalWlanChannelHoppingGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *wifiSsidObj = NULL, *wifiSsidArray = NULL;
	struct json_object *getAllReqIntfObjs = NULL;
	objIndex_t iid;

	wifiSsidArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_WIFI_SSID, &iid, &wifiSsidObj) == ZCFG_SUCCESS) {
		if (json_object_get_int(json_object_object_get(wifiSsidObj, "X_ZYXEL_Attribute")) != 0) {
			zcfgFeJsonObjFree(wifiSsidObj);
			continue;	
		}
		json_object_array_add(wifiSsidArray, wifiSsidObj);
	}

	getAllReqIntfObjs = json_object_new_object();
	json_object_array_add(Jarray, getAllReqIntfObjs);
	json_object_object_add(getAllReqIntfObjs, "wifiSsid", wifiSsidArray);

	return ret;
}

zcfgRet_t zcfgFeDalWlanChannelHoppingEdit (struct json_object *Jobj, char *replyMsg)
{ 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ssidObj = NULL;
	struct json_object *reqObj = NULL;
	int reqObjIdx = 0;
	objIndex_t ssidIid = {0};

	bool bEnable = false;
	int iInterval = 0;
	char *pName = NULL, *pNameDb = NULL;

	while ((reqObj = json_object_array_get_idx(Jobj, reqObjIdx)) != NULL)
	{
		pName = json_object_get_string(json_object_object_get(reqObj, "Name"));
		if (pName == NULL) {
			reqObjIdx++;
			continue;
		}

		IID_INIT(ssidIid);
		while (zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &ssidIid, &ssidObj) == ZCFG_SUCCESS) {
			pNameDb = json_object_get_string(json_object_object_get(ssidObj, "Name"));
			if (pNameDb == NULL) {
				zcfgFeJsonObjFree(ssidObj);
				continue;
			}
			if (strcmp(pName, pNameDb)) {
				zcfgFeJsonObjFree(ssidObj);
				continue;
			}

			bEnable = json_object_get_boolean(json_object_object_get(reqObj, "X_ZYXEL_ChannelHoppingEnable"));
			iInterval = json_object_get_int(json_object_object_get(reqObj, "X_ZYXEL_ChannelHoppingInterval"));

			if (json_object_get_boolean(json_object_object_get(ssidObj, "X_ZYXEL_ChannelHoppingEnable")) != bEnable ||
			     json_object_get_int(json_object_object_get(ssidObj, "X_ZYXEL_ChannelHoppingInterval")) != iInterval) {
				json_object_object_add(ssidObj, "X_ZYXEL_ChannelHoppingEnable", json_object_new_boolean(bEnable));
				json_object_object_add(ssidObj, "X_ZYXEL_ChannelHoppingInterval", json_object_new_int(iInterval));

				ret = zcfgFeObjJsonSet(RDM_OID_WIFI_SSID, &ssidIid, ssidObj, NULL);
			}
			zcfgFeJsonObjFree(ssidObj);
			break;
		}

		reqObjIdx++;
	}
	
	return ret;
}

zcfgRet_t zcfgFeDalWlanChannelHopping (const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalWlanChannelHoppingEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalWlanChannelHoppingGet(Jobj, Jarray, NULL);
	}

	return ret;
}

