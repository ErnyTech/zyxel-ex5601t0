
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t NAT_PORT_TRIGGERING_param[] =
{
	{"Index",				dalType_int,			0,	0,		NULL,	NULL,			dal_Edit|dal_Delete},
	{"Enable",			dalType_boolean,		0,	0,		NULL,	NULL,			dal_Add},
	{"Description",		dalType_string,		1,	256,		NULL,	NULL,			dal_Add},
#ifndef ABOG_CUSTOMIZATION
	{"Interface",			dalType_LanRtWanWWan,	0,	0,		NULL,	NULL,			dal_Add},
	{"X_ZYXEL_AutoDetectWanStatus",	dalType_boolean,0,	0,		NULL,	NULL,			dalcmd_Forbid},
#endif
	{"TriggerPortStart",	dalType_int,			1,	65535,	NULL,	NULL,			dal_Add},
	{"TriggerPortEnd",		dalType_int,			1,	65535,	NULL,	NULL,			dal_Add},
	{"OpenPortStart",		dalType_int,			1,	65535,	NULL,	NULL,			dal_Add},
	{"OpenPortEnd",		dalType_int,			1,	65535,	NULL,	NULL,			dal_Add},
	{"OpenProtocol",		dalType_string,		0,	0,		NULL,	"TCP|UDP|ALL",	dal_Add},
	{"TriggerProtocol",		dalType_string,		0,	0,		NULL,	"TCP|UDP|ALL",	dal_Add},
	{NULL,				0,					0,	0,		NULL,	NULL,			0}
};

zcfgRet_t zcfgFeDalNatPortTriggeringAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatPortTriggeringJobj = NULL;
	objIndex_t objIid = {0};
	bool enable = false;
	const char *interface = NULL;
	int triggerport = 0;
	int triggerportendrange = 0;
	int openport = 0;
	int openportendrange = 0;
	const char *protocol = NULL;
	const char *triggerprotocol = NULL;
	const char *description = NULL;
    bool autoDetectWanStatus = false;
	
	if (json_object_object_get(Jobj, "Description")) {
		// check Description is duplicated or not.
		description = json_object_get_string(json_object_object_get(Jobj, "Description"));
		IID_INIT(objIid);
		while( (ret = zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_TRIGGERING, &objIid, &NatPortTriggeringJobj)) == ZCFG_SUCCESS) {
			if(strcmp(description,json_object_get_string(json_object_object_get(NatPortTriggeringJobj, "Description")))==0){
				if(replyMsg != NULL)
					strcat(replyMsg, "Description is duplicated");
				zcfgFeJsonObjFree(NatPortTriggeringJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			zcfgFeJsonObjFree(NatPortTriggeringJobj);
		}
	}

	IID_INIT(objIid);
	ret = zcfgFeObjJsonAdd(RDM_OID_NAT_PORT_TRIGGERING, &objIid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGet(RDM_OID_NAT_PORT_TRIGGERING, &objIid, &NatPortTriggeringJobj);
	}
	
	if(ret == ZCFG_SUCCESS){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
#ifdef ABOG_CUSTOMIZATION
		autoDetectWanStatus = true;
#else
		interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
		if(!strcmp(interface,"")){
			autoDetectWanStatus = true;
		}
#endif
		triggerport = json_object_get_int(json_object_object_get(Jobj, "TriggerPortStart"));
		triggerportendrange = json_object_get_int(json_object_object_get(Jobj, "TriggerPortEnd"));
		openport = json_object_get_int(json_object_object_get(Jobj, "OpenPortStart"));
		openportendrange = json_object_get_int(json_object_object_get(Jobj, "OpenPortEnd"));
		protocol = json_object_get_string(json_object_object_get(Jobj, "OpenProtocol"));
		triggerprotocol = json_object_get_string(json_object_object_get(Jobj, "TriggerProtocol"));
		description = json_object_get_string(json_object_object_get(Jobj, "Description"));
	
		if (triggerport > triggerportendrange || openport > openportendrange) {
			if(replyMsg != NULL)
				strcat(replyMsg, "start port can not great than end port.");
			zcfgFeJsonObjFree(NatPortTriggeringJobj);
			return ZCFG_INVALID_PARAM_VALUE;
		}
		json_object_object_add(NatPortTriggeringJobj, "Enable", json_object_new_boolean(enable));
#ifdef ABOG_CUSTOMIZATION
		json_object_object_add(NatPortTriggeringJobj, "Interface", json_object_new_string(""));
#else
		json_object_object_add(NatPortTriggeringJobj, "Interface", json_object_new_string(interface));
#endif
		json_object_object_add(NatPortTriggeringJobj, "X_ZYXEL_AutoDetectWanStatus", json_object_new_boolean(autoDetectWanStatus));
		json_object_object_add(NatPortTriggeringJobj, "TriggerPort", json_object_new_int(triggerport));
		json_object_object_add(NatPortTriggeringJobj, "TriggerPortEndRange", json_object_new_int(triggerportendrange));
		json_object_object_add(NatPortTriggeringJobj, "OpenPort", json_object_new_int(openport));
		json_object_object_add(NatPortTriggeringJobj, "OpenPortEndRange", json_object_new_int(openportendrange));
		json_object_object_add(NatPortTriggeringJobj, "Protocol", json_object_new_string(protocol));
		json_object_object_add(NatPortTriggeringJobj, "TriggerProtocol", json_object_new_string(triggerprotocol));
		json_object_object_add(NatPortTriggeringJobj, "Description", json_object_new_string(description));
		ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_TRIGGERING, &objIid, NatPortTriggeringJobj, NULL);
	}
	
	zcfgFeJsonObjFree(NatPortTriggeringJobj);
	return ret;
}

zcfgRet_t zcfgFeDalNatPortTriggeringEdit(struct json_object *Jobj, char *replyMsg)
{

	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatPortTriggeringJobj = NULL;
	objIndex_t objIid = {0}, iid = {0};
	bool enable = false;
	const char *interface = NULL;
	int triggerport = 0;
	int triggerportendrange = 0;
	int openport = 0;
	int openportendrange = 0;
	const char *protocol = NULL;
	const char *triggerprotocol = NULL;
	const char *description = NULL;
	int index = 0;
	bool autoDetectWanStatus = false;
	
	if (Jobj && json_object_object_get(Jobj, "idx") != NULL)
	{
		IID_INIT(objIid);
		objIid.level = 1;
		objIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}
	else
	{
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		ret = convertIndextoIid(index, &objIid, RDM_OID_NAT_PORT_TRIGGERING, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
	}

	if (json_object_object_get(Jobj, "Description")) {
		// check Description is duplicated or not.
		description = json_object_get_string(json_object_object_get(Jobj, "Description"));
		IID_INIT(iid);
		while( (ret = zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_TRIGGERING, &iid, &NatPortTriggeringJobj)) == ZCFG_SUCCESS) {
			if (objIid.idx[0] == iid.idx[0]) {
				zcfgFeJsonObjFree(NatPortTriggeringJobj);
				continue;
		}
			if(strcmp(description,json_object_get_string(json_object_object_get(NatPortTriggeringJobj, "Description")))==0){
				if(replyMsg != NULL)
					strcat(replyMsg, "Description is duplicated");
				zcfgFeJsonObjFree(NatPortTriggeringJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			zcfgFeJsonObjFree(NatPortTriggeringJobj);
		}
		}

	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT_PORT_TRIGGERING, &objIid, &NatPortTriggeringJobj)) != ZCFG_SUCCESS) {
		printf("%s():(ERROR) zcfgFeObjJsonGet get port trigger fail oid(%d)\n ",__FUNCTION__,RDM_OID_NAT_PORT_TRIGGERING);
		return ZCFG_INTERNAL_ERROR;
	}

	if (json_object_object_get(Jobj, "Enable")) {
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		json_object_object_add(NatPortTriggeringJobj, "Enable", json_object_new_boolean(enable));
	}
#ifndef ABOG_CUSTOMIZATION
	if (json_object_object_get(Jobj, "Interface")) {
		interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
		if(!strcmp(interface,"")){
			autoDetectWanStatus = true;
		}
		json_object_object_add(NatPortTriggeringJobj, "X_ZYXEL_AutoDetectWanStatus", json_object_new_boolean(autoDetectWanStatus));
		json_object_object_add(NatPortTriggeringJobj, "Interface", json_object_new_string(interface));
	}
#endif
	if (json_object_object_get(Jobj, "TriggerPortStart")) {
		triggerport = json_object_get_int(json_object_object_get(Jobj, "TriggerPortStart"));
		json_object_object_add(NatPortTriggeringJobj, "TriggerPort", json_object_new_int(triggerport));
	}
	if (json_object_object_get(Jobj, "TriggerPortEnd")) {
		triggerportendrange = json_object_get_int(json_object_object_get(Jobj, "TriggerPortEnd"));
		json_object_object_add(NatPortTriggeringJobj, "TriggerPortEndRange", json_object_new_int(triggerportendrange));
	}
	if (json_object_object_get(Jobj, "OpenPortStart")) {
		openport = json_object_get_int(json_object_object_get(Jobj, "OpenPortStart"));
		json_object_object_add(NatPortTriggeringJobj, "OpenPort", json_object_new_int(openport));
	}
	if (json_object_object_get(Jobj, "OpenPortEnd")) {
		openportendrange = json_object_get_int(json_object_object_get(Jobj, "OpenPortEnd"));
		json_object_object_add(NatPortTriggeringJobj, "OpenPortEndRange", json_object_new_int(openportendrange));
	}
	if (json_object_object_get(Jobj, "OpenProtocol")) {
		protocol = json_object_get_string(json_object_object_get(Jobj, "OpenProtocol"));
		json_object_object_add(NatPortTriggeringJobj, "Protocol", json_object_new_string(protocol));
	}
	if (json_object_object_get(Jobj, "TriggerProtocol")) {
		triggerprotocol = json_object_get_string(json_object_object_get(Jobj, "TriggerProtocol"));
		json_object_object_add(NatPortTriggeringJobj, "TriggerProtocol", json_object_new_string(triggerprotocol));
	}
	if (json_object_object_get(Jobj, "Description")) {
		description = json_object_get_string(json_object_object_get(Jobj, "Description"));
		json_object_object_add(NatPortTriggeringJobj, "Description", json_object_new_string(description));
	}
	if (triggerport > triggerportendrange || openport > openportendrange) {
		if(replyMsg != NULL)
			strcat(replyMsg, "start port can not great than end port.");
		zcfgFeJsonObjFree(NatPortTriggeringJobj);
		return ZCFG_INVALID_PARAM_VALUE;
	}
		ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_TRIGGERING, &objIid, NatPortTriggeringJobj, NULL);
	zcfgFeJsonObjFree(NatPortTriggeringJobj);
	return ret;
}
zcfgRet_t zcfgFeDalNatPortTriggeringDelete(struct json_object *Jobj, char *replyMsg)
{
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatPortTriggeringJobj = NULL;
	objIndex_t objIid = {0};
	int index = 0;
	int count = 1;
	bool found = false;
	
	if (Jobj && json_object_object_get(Jobj, "idx") != NULL)
	{
		IID_INIT(objIid);
		objIid.level = 1;
		objIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
		ret = zcfgFeObjJsonDel(RDM_OID_NAT_PORT_TRIGGERING, &objIid, NULL);
		return ret;
	}
	
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	
	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_TRIGGERING, &objIid, &NatPortTriggeringJobj) == ZCFG_SUCCESS) {
		if(count == index){
			found = true;
			break;
		}
		else
			count++;
	}
	if(found){
		ret = zcfgFeObjJsonDel(RDM_OID_NAT_PORT_TRIGGERING, &objIid, NULL);
	}
	
	zcfgFeJsonObjFree(NatPortTriggeringJobj);
	return ret;
	
}

zcfgRet_t zcfgFeDalNatPortTriggeringGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	struct json_object *NatPortTriggeringJobj = NULL;
	struct json_object *paramJobj = NULL;
	objIndex_t objIid = {0};
	int index = 0, idx = -1;

	if (Jobj && json_object_object_get(Jobj, "idx") != NULL)
	{
		idx = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}

	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_TRIGGERING, &objIid, &NatPortTriggeringJobj) == ZCFG_SUCCESS) {
		++index;
		if (idx == -1 || objIid.idx[0] == idx)
		{
			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(NatPortTriggeringJobj, "Enable")));
			json_object_object_add(paramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(NatPortTriggeringJobj, "Interface")));
			json_object_object_add(paramJobj, "X_ZYXEL_AutoDetectWanStatus", JSON_OBJ_COPY(json_object_object_get(NatPortTriggeringJobj, "X_ZYXEL_AutoDetectWanStatus")));
			json_object_object_add(paramJobj, "TriggerPort", JSON_OBJ_COPY(json_object_object_get(NatPortTriggeringJobj, "TriggerPort")));
			json_object_object_add(paramJobj, "TriggerPortEndRange", JSON_OBJ_COPY(json_object_object_get(NatPortTriggeringJobj, "TriggerPortEndRange")));
			json_object_object_add(paramJobj, "OpenPort", JSON_OBJ_COPY(json_object_object_get(NatPortTriggeringJobj, "OpenPort")));
			json_object_object_add(paramJobj, "OpenPortEndRange", JSON_OBJ_COPY(json_object_object_get(NatPortTriggeringJobj, "OpenPortEndRange")));
			json_object_object_add(paramJobj, "Protocol", JSON_OBJ_COPY(json_object_object_get(NatPortTriggeringJobj, "Protocol")));
			json_object_object_add(paramJobj, "TriggerProtocol", JSON_OBJ_COPY(json_object_object_get(NatPortTriggeringJobj, "TriggerProtocol")));
			json_object_object_add(paramJobj, "Description", JSON_OBJ_COPY(json_object_object_get(NatPortTriggeringJobj, "Description")));
			json_object_object_add(paramJobj, "Index", json_object_new_int(index));
			json_object_object_add(paramJobj, "idx", json_object_new_int(objIid.idx[0]));
			json_object_array_add(Jarray, paramJobj);
		}
		zcfgFeJsonObjFree(NatPortTriggeringJobj);
	}

	return ZCFG_SUCCESS;
	
}

void zcfgFeDalShowPortTrigger(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
#ifdef ABOG_CUSTOMIZATION
	printf("%-5s %-15s %-7s %-17s %-15s %-15s %-15s %-15s %-13s\n",
		    "Index", "Description", "Enable", "TriggerPortStart", "TriggerPortEnd", "TriggerProto.", "OpenPortStart", "OpenPortEnd", "OpenProtocol");
#else
	printf("%-5s %-15s %-7s %-15s %-17s %-15s %-15s %-15s %-15s %-13s\n",
		    "Index", "Description", "Enable", "WAN Interface", "TriggerPortStart", "TriggerPortEnd", "TriggerProto.", "OpenPortStart", "OpenPortEnd", "OpenProtocol");
#endif
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
#ifdef ABOG_CUSTOMIZATION
		printf("%-5s %-15s %-7d %-17s %-15s %-15s %-15s %-15s %-13s\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Description")),
			json_object_get_boolean(json_object_object_get(obj, "Enable")),
			json_object_get_string(json_object_object_get(obj, "TriggerPort")),
			json_object_get_string(json_object_object_get(obj, "TriggerPortEndRange")),
			json_object_get_string(json_object_object_get(obj, "TriggerProtocol")),
			json_object_get_string(json_object_object_get(obj, "OpenPort")),
			json_object_get_string(json_object_object_get(obj, "OpenPortEndRange")),
			json_object_get_string(json_object_object_get(obj, "Protocol")));
#else
		printf("%-5s %-15s %-7d %-15s %-17s %-15s %-15s %-15s %-15s %-13s\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Description")),
			json_object_get_boolean(json_object_object_get(obj, "Enable")),
			json_object_get_string(json_object_object_get(obj, "Interface")),
			json_object_get_string(json_object_object_get(obj, "TriggerPort")),
			json_object_get_string(json_object_object_get(obj, "TriggerPortEndRange")),
			json_object_get_string(json_object_object_get(obj, "TriggerProtocol")),
			json_object_get_string(json_object_object_get(obj, "OpenPort")),
			json_object_get_string(json_object_object_get(obj, "OpenPortEndRange")),
			json_object_get_string(json_object_object_get(obj, "Protocol")));
#endif
	}
}

zcfgRet_t zcfgFeDalNatPortTriggering(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalNatPortTriggeringEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalNatPortTriggeringAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalNatPortTriggeringDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDalNatPortTriggeringGet(Jobj, Jarray, replyMsg);
	}

	return ret;
}
