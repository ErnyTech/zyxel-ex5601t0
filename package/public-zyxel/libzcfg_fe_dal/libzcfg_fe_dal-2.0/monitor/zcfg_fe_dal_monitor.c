#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"


dal_param_t Traffic_Status_param[] = 
{
	{"TrafficStatus",		dalType_string,	0,	0,	NULL},
	{NULL,		0,	0,	0,	NULL}
};


/*
 *  Function Name: zcfgFeDalTrafficStatusGet
 *  Description: get All Req Intf Objs to Vue GUI for Traffic status
 *
 */
zcfgRet_t zcfgFeDalTrafficStatusGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *getAllReqIntfObjs = NULL;
	struct json_object *ipIfaceObj = NULL, *ipIfaceArray = NULL, *ipIfaceStObj = NULL, *ipIfaceStArray = NULL;
	struct json_object *pppIfaceObj = NULL, *pppIfaceArray = NULL,  *pppIfaceSt = NULL,  *pppIfaceStArray = NULL;
	struct json_object *ethIfaceObj = NULL, *ethIfaceArray = NULL,  *ethIfaceSt = NULL,  *ethIfaceStArray = NULL;
	struct json_object *bridgingPTObj = NULL, *bridgingPTArray = NULL,  *bridgingStatus = NULL,  *bridgingStatusArray = NULL;
	struct json_object *wifiSsidArray = NULL, *wifiRadiArray = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	struct json_object *wifiSsidObj = NULL, *wifiRadioObj = NULL;
#endif
	struct json_object *hostsObj = NULL, *hostsArray = NULL;
	struct json_object *changeIconNameObj = NULL, *changeIconNameArray = NULL;	
	objIndex_t iid;

	
	ipIfaceArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &iid, &ipIfaceObj) == ZCFG_SUCCESS) {
		json_object_array_add(ipIfaceArray, ipIfaceObj);
	}
	
	ipIfaceStArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE_STAT, &iid, &ipIfaceStObj) == ZCFG_SUCCESS) {
		json_object_array_add(ipIfaceStArray, ipIfaceStObj);
	}
	pppIfaceArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_PPP_IFACE, &iid, &pppIfaceObj) == ZCFG_SUCCESS) {
		json_object_array_add(pppIfaceArray, pppIfaceObj);
	}
	
	pppIfaceStArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_PPP_IFACE_STAT, &iid, &pppIfaceSt) == ZCFG_SUCCESS) {
		json_object_array_add(pppIfaceStArray, pppIfaceSt);
	}
	ethIfaceArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &iid, &ethIfaceObj) == ZCFG_SUCCESS) {
		json_object_array_add(ethIfaceArray, ethIfaceObj);
	}
	
	ethIfaceStArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_INTF_ST, &iid, &ethIfaceSt) == ZCFG_SUCCESS) {
		json_object_array_add(ethIfaceStArray, ethIfaceSt);
	}
	bridgingPTArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_BRIDGING_BR_PORT, &iid, &bridgingPTObj) == ZCFG_SUCCESS) {
		json_object_array_add(bridgingPTArray, bridgingPTObj);
	}
	
	bridgingStatusArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_BRIDGING_BR_PORT_ST, &iid, &bridgingStatus) == ZCFG_SUCCESS) {
		json_object_array_add(bridgingStatusArray, bridgingStatus);
	}
	wifiSsidArray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_WIFI_SSID, &iid, &wifiSsidObj) == ZCFG_SUCCESS) {
		json_object_array_add(wifiSsidArray, wifiSsidObj);
	}
#endif
	wifiRadiArray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_WIFI_RADIO, &iid, &wifiRadioObj) == ZCFG_SUCCESS) {
		json_object_array_add(wifiRadiArray, wifiRadioObj);
	}
#endif
	hostsArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_HOSTS_HOST, &iid, &hostsObj) == ZCFG_SUCCESS) {
		json_object_array_add(hostsArray, hostsObj);
	}
	
	changeIconNameArray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_CHANGE_ICON_NAME, &iid, &changeIconNameObj) == ZCFG_SUCCESS) {
		json_object_array_add(changeIconNameArray, changeIconNameObj);
	}
	

	getAllReqIntfObjs = json_object_new_object();
	json_object_array_add(Jarray, getAllReqIntfObjs);
	json_object_object_add(getAllReqIntfObjs, "ipIface", ipIfaceArray);
	json_object_object_add(getAllReqIntfObjs, "ipIfaceSt", ipIfaceStArray);
	json_object_object_add(getAllReqIntfObjs, "pppIface", pppIfaceArray);
	json_object_object_add(getAllReqIntfObjs, "pppIfaceSt", pppIfaceStArray);
	json_object_object_add(getAllReqIntfObjs, "ethIface", ethIfaceArray);
	json_object_object_add(getAllReqIntfObjs, "ethIfaceSt", ethIfaceStArray);		
	json_object_object_add(getAllReqIntfObjs, "bridgingPT", bridgingPTArray);
	json_object_object_add(getAllReqIntfObjs, "bridgingStatus", bridgingStatusArray);		
	json_object_object_add(getAllReqIntfObjs, "wifiSsid", wifiSsidArray);
	json_object_object_add(getAllReqIntfObjs, "wifiRadio", wifiRadiArray);
	json_object_object_add(getAllReqIntfObjs, "hosts", hostsArray);
	json_object_object_add(getAllReqIntfObjs, "changeIconName", changeIconNameArray);	
	return ret;
	
}
 zcfgRet_t zcfgFeDalTrafficStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
 {
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "GET")){
		ret = zcfgFeDalTrafficStatusGet(Jobj, Jarray, NULL);
	}

	return ret;	 
 }
