
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_dal.h"

dal_param_t POLICYROUTE_param[] =
{
	{"Enable",							dalType_boolean,0,	0,	NULL,	NULL,	dal_Add},
	//{"X_ZYXEL_PolicyRoute",				dalType_boolean,0,	0,	NULL},	write in dal
	{"RouteName",						dalType_string,	0,	0,	NULL,	NULL,	dal_Add},
	{"SourceIpAddress",					dalType_v4Addr,	0,	0,	NULL,	NULL,	0},
	{"SourceSubnetMask",				dalType_v4Mask,	0,	0,	NULL,	NULL,	0},
	{"Protocol",						dalType_string,	0,	0,	NULL	,"TCP|UDP|ALL",	0},
	{"SourcePort",						dalType_int,	0,	65535,	NULL,	NULL,	0},
	{"SourceMacAddress",				dalType_string,	0,	0,	NULL,	NULL,	0},
	{"SourceIfName",					dalType_LanPortListBr0NoWiFi,	0,	0,	NULL,	NULL,	0},	
	{"WANInterface",					dalType_RtWanWWan,	0,	0,	NULL,	NULL,	dal_Add},
	{"Index",							dalType_int,	0,	0,	NULL,	NULL,	dal_Delete|dal_Edit},
	{NULL,									0,				0,	0,	0},
};

struct json_object *policyRTObj;

bool X_ZYXEL_PolicyRoute;
bool Enable;
const char *X_ZYXEL_PolicyRouteName;
const char *X_ZYXEL_SourceIpAddress;
const char *X_ZYXEL_SourceSubnetMask;
const char *X_ZYXEL_SourceMacAddress;
const char *X_ZYXEL_SourceIfName;
int X_ZYXEL_SourcePort;
int X_ZYXEL_Protocol;
const char *Interface;

void zcfgFeDalShowPolicyRoute(struct json_object *Jarray){
	struct json_object *obj = NULL;
	int len = 0, i;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-8s %-10s %-10s %-18s %-20s %-10s %-13s %-20s %-20s %-10s\n", 
		"Index", "Status", "Name", "Source IP", "Source Subnet Mask", "Protocol", "Source Port", "Source MAC", "Source Interface", "WAN Interface");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-8s %-10s %-10s %-18s %-20s %-10s %-13s %-20s %-20s %-10s\n", 
		json_object_get_string(json_object_object_get(obj, "Index")),
		json_object_get_string(json_object_object_get(obj, "Enable")),
		json_object_get_string(json_object_object_get(obj, "RouteName")),
		json_object_get_string(json_object_object_get(obj, "SourceIpAddress")),
		json_object_get_string(json_object_object_get(obj, "SourceSubnetMask")),
		json_object_get_string(json_object_object_get(obj, "Protocol")),
		json_object_get_string(json_object_object_get(obj, "SourcePort")),
		json_object_get_string(json_object_object_get(obj, "SourceMacAddress")),
		json_object_get_string(json_object_object_get(obj, "SourceIfName")),
		json_object_get_string(json_object_object_get(obj, "WANInterface")));
	
	}
}


void initPolicyRouteGlobalObjects(){
	policyRTObj = NULL;
	return;
}

void freeAllPolicyRouteObjects(){
	if(policyRTObj) json_object_put(policyRTObj);
	return;
}

void getPolicyRouteBasicInfo(struct json_object *Jobj, char *replyMsg){
	const char *protocol = NULL;
	//X_ZYXEL_PolicyRoute = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_PolicyRoute"));
	Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	X_ZYXEL_PolicyRouteName = json_object_get_string(json_object_object_get(Jobj, "RouteName"));
	X_ZYXEL_SourceIpAddress = json_object_get_string(json_object_object_get(Jobj, "SourceIpAddress"));
	X_ZYXEL_SourceSubnetMask = json_object_get_string(json_object_object_get(Jobj, "SourceSubnetMask"));
	X_ZYXEL_SourceMacAddress = json_object_get_string(json_object_object_get(Jobj, "SourceMacAddress"));
	X_ZYXEL_SourceIfName = json_object_get_string(json_object_object_get(Jobj, "SourceIfName"));
	X_ZYXEL_SourcePort = json_object_get_int(json_object_object_get(Jobj, "SourcePort"));
	protocol = json_object_get_string(json_object_object_get(Jobj, "Protocol"));
	if(json_object_object_get(Jobj, "Protocol")){
		if(!strcmp(protocol, "TCP"))
			X_ZYXEL_Protocol = 6;
		else if(!strcmp(protocol, "UDP"))
			X_ZYXEL_Protocol = 17;
		else if(!strcmp(protocol, "ALL"))
			X_ZYXEL_Protocol = -1;
		if(X_ZYXEL_Protocol == -1 && json_object_object_get(Jobj, "SourcePort")){
			X_ZYXEL_SourcePort = 0;
		}
}
	Interface = json_object_get_string(json_object_object_get(Jobj, "WANInterface"));

	return;
}

zcfgRet_t zcfgFeDalPolicyRouteObjEdit(struct json_object *Jobj, objIndex_t *Iid){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, Iid, &policyRTObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Policy route fail oid(%d)\n ",__FUNCTION__,RDM_OID_ROUTING_ROUTER_V4_FWD);
		return ZCFG_INTERNAL_ERROR;
	}

	if(policyRTObj != NULL){
		json_object_object_add(policyRTObj, "X_ZYXEL_PolicyRoute", json_object_new_boolean(true));
		if(json_object_object_get(Jobj, "Enable"))
		json_object_object_add(policyRTObj, "Enable", json_object_new_boolean(Enable));
		if(json_object_object_get(Jobj, "RouteName"))
		json_object_object_add(policyRTObj, "X_ZYXEL_PolicyRouteName", json_object_new_string(X_ZYXEL_PolicyRouteName));
		if(json_object_object_get(Jobj, "SourceIpAddress"))
		json_object_object_add(policyRTObj, "X_ZYXEL_SourceIpAddress", json_object_new_string(X_ZYXEL_SourceIpAddress));
		if(json_object_object_get(Jobj, "SourceSubnetMask"))
		json_object_object_add(policyRTObj, "X_ZYXEL_SourceSubnetMask", json_object_new_string(X_ZYXEL_SourceSubnetMask));
		if(json_object_object_get(Jobj, "SourceMacAddress"))
		json_object_object_add(policyRTObj, "X_ZYXEL_SourceMacAddress", json_object_new_string(X_ZYXEL_SourceMacAddress));
		if(json_object_object_get(Jobj, "SourceIfName"))
		json_object_object_add(policyRTObj, "X_ZYXEL_SourceIfName", json_object_new_string(X_ZYXEL_SourceIfName));
		if(json_object_object_get(Jobj, "SourcePort"))
			json_object_object_add(policyRTObj, "X_ZYXEL_SourcePort", json_object_new_int(X_ZYXEL_SourcePort));
		else 
			json_object_object_add(policyRTObj, "X_ZYXEL_SourcePort", json_object_new_string("0"));
		if(json_object_object_get(Jobj, "Protocol"))
			json_object_object_add(policyRTObj, "X_ZYXEL_Protocol", json_object_new_int(X_ZYXEL_Protocol));
		if(json_object_object_get(Jobj, "WANInterface"))
		json_object_object_add(policyRTObj, "Interface", json_object_new_string(Interface));
	}

	return ret;
}

zcfgRet_t zcfgFeDalPolicyRouteGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
zcfgRet_t zcfgFeDalPolicyRouteEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *Jarray = NULL;
	struct json_object *policyRtObj = NULL;
	objIndex_t policyRtIid = {0};
	int index = 0, idx = 0;
	zcfg_offset_t oid;
	Jarray = json_object_new_array();
	zcfgFeDalPolicyRouteGet(NULL, Jarray, NULL);
	initPolicyRouteGlobalObjects();
	getPolicyRouteBasicInfo(Jobj, replyMsg);
	oid = RDM_OID_ROUTING_ROUTER_V4_FWD;
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	
	while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &policyRtIid, &policyRtObj) == ZCFG_SUCCESS)
	{
		if(json_object_get_boolean(json_object_object_get(policyRtObj, "X_ZYXEL_PolicyRoute")))
		{
			idx++;
			if (index == idx)
			{
				zcfgFeJsonObjFree(policyRtObj);
				break;
			}
		}
		zcfgFeJsonObjFree(policyRtObj);
	}
	if (index != idx)
		ret = ZCFG_INVALID_PARAM_VALUE;

	if(ret != ZCFG_SUCCESS)
		return ret;

	if(json_object_object_get(Jobj, "RouteName")){
		ret = dalcmdParamCheck(&policyRtIid, X_ZYXEL_PolicyRouteName, oid, "X_ZYXEL_PolicyRouteName", "RouteName", replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
	}
	
	if((ret = zcfgFeDalPolicyRouteObjEdit(Jobj, &policyRtIid)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit Policy Route \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V4_FWD, &policyRtIid, policyRTObj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_ROUTING_ROUTER_V4_FWD);
		goto exit;
	}

exit:
	freeAllPolicyRouteObjects();
	return ret;
}



zcfgRet_t zcfgFeDalPolicyRouteAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t policyRtIid = {0};
	zcfg_offset_t oid;
	//printf("%s():--- Add Policy Route Input Json Object --- \n%s \n ",__FUNCTION__,json_object_to_json_string(Jobj));
	initPolicyRouteGlobalObjects();
	getPolicyRouteBasicInfo(Jobj, replyMsg);
	oid = RDM_OID_ROUTING_ROUTER_V4_FWD;
	ret = dalcmdParamCheck(NULL, X_ZYXEL_PolicyRouteName, oid, "X_ZYXEL_PolicyRouteName", "RouteName",  replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	policyRtIid.level = 1;
	policyRtIid.idx[0] = 1;

	if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_ROUTING_ROUTER_V4_FWD, &policyRtIid, NULL)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonAdd add Policy Route \n ",__FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeDalPolicyRouteObjEdit(Jobj, &policyRtIid)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit Policy route \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V4_FWD, &policyRtIid, policyRTObj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_ROUTING_ROUTER_V4_FWD);
	}

exit:
	freeAllPolicyRouteObjects();
	return ret;
}

zcfgRet_t zcfgFeDalPolicyRouteDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *policyRtObj = NULL;
	objIndex_t policyRtIid = {0};
	int index = 0, idx = 0;

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));	
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, &policyRtIid, &policyRtObj) == ZCFG_SUCCESS)
	{
		if(json_object_get_boolean(json_object_object_get(policyRtObj, "X_ZYXEL_PolicyRoute")))
		{
			idx++;
			if (index == idx)
			{
				zcfgFeJsonObjFree(policyRtObj);
				break;
			}
		}
		zcfgFeJsonObjFree(policyRtObj);
	}
	if (index != idx)
		ret = ZCFG_INVALID_PARAM_VALUE;
	else 
		ret = zcfgFeObjJsonDel(RDM_OID_ROUTING_ROUTER_V4_FWD, &policyRtIid, NULL);

	return ret;
}


zcfgRet_t zcfgFeDalPolicyRouteGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *policyRtObj = NULL;
	objIndex_t policyRtIid = {0};
	int index = 0, protocolnum = 0;
	//struct json_object *WanLanList = NULL;
	//int len = 0;
	//getWanLanList(&WanLanList);
	//len = json_object_array_length(WanLanList);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, &policyRtIid, &policyRtObj) == ZCFG_SUCCESS){
		if(json_object_get_boolean(json_object_object_get(policyRtObj, "X_ZYXEL_PolicyRoute"))){
			index++;
			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "Index", json_object_new_int(index));
			json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(policyRtObj, "Enable")));
			json_object_object_add(paramJobj, "RouteName", JSON_OBJ_COPY(json_object_object_get(policyRtObj, "X_ZYXEL_PolicyRouteName")));
			json_object_object_add(paramJobj, "SourceIpAddress", JSON_OBJ_COPY(json_object_object_get(policyRtObj, "X_ZYXEL_SourceIpAddress")));
			json_object_object_add(paramJobj, "SourceSubnetMask", JSON_OBJ_COPY(json_object_object_get(policyRtObj, "X_ZYXEL_SourceSubnetMask")));
			protocolnum = json_object_get_int(json_object_object_get(policyRtObj, "X_ZYXEL_Protocol"));
			if(protocolnum == 6)
				json_object_object_add(paramJobj, "Protocol", json_object_new_string("TCP"));
			else if(protocolnum == 17)
				json_object_object_add(paramJobj, "Protocol", json_object_new_string("UDP"));
			else if(protocolnum == -1)
				json_object_object_add(paramJobj, "Protocol", json_object_new_string("ALL"));
			json_object_object_add(paramJobj, "SourcePort", JSON_OBJ_COPY(json_object_object_get(policyRtObj, "X_ZYXEL_SourcePort")));
			json_object_object_add(paramJobj, "WANInterface", JSON_OBJ_COPY(json_object_object_get(policyRtObj, "Interface")));
			json_object_object_add(paramJobj, "SourceMacAddress", JSON_OBJ_COPY(json_object_object_get(policyRtObj, "X_ZYXEL_SourceMacAddress")));
			json_object_object_add(paramJobj, "SourceIfName", JSON_OBJ_COPY(json_object_object_get(policyRtObj, "X_ZYXEL_SourceIfName")));

			json_object_array_add(Jarray, paramJobj);
		}
		zcfgFeJsonObjFree(policyRtObj);
	}
	return ret;
}

zcfgRet_t zcfgFeDalPolicyRoute(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalPolicyRouteEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalPolicyRouteAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalPolicyRouteDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDalPolicyRouteGet(Jobj, Jarray, NULL);
	}

	return ret;
}

