
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t NAT_PCP_param[] =
{
	{"Index",				dalType_int,			0,	0,		NULL,	NULL,			dal_Edit|dal_Delete},
//	{"Enable",			dalType_boolean,		0,	0,		NULL,	NULL,			dal_Add},
//	{"ExternalIPAddress",		dalType_string,		1,	256,		NULL,	NULL,			dal_Add},
	{"InternalIPAddress",		dalType_string,		1,	46,		NULL,	NULL,			dal_Add},
	{"RequiredInternalPort",	dalType_int,			1,	65535,	NULL,	NULL,			dal_Add},
	{"RequiredPublicPort",		dalType_int,			1,	65535,	NULL,	NULL,			dal_Add},
//	{"AssignedPublicPort",		dalType_int,			1,	65535,	NULL,	NULL,			dal_Add},
//	{"ResultCode",		dalType_int,			1,	65535,	NULL,	NULL,			dal_Add},
	{"Protocol",		dalType_string,		0,	0,		NULL,	"TCP|UDP",	dal_Add},
	{"AllowPCPPortProposal",		dalType_int,		0,	0,		NULL,	NULL,	dal_Add},
	{NULL,				0,					0,	0,		NULL,	NULL,			0}
};

zcfgRet_t zcfgFeDalNatPcpAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatPcpJobj = NULL;
	objIndex_t objIid = {0};
	bool enable = false;
	int requiredInternalPort = 0;
	int requiredPublicPort = 0;
	int allowPCPPortProposal = 0;
	const char *protocol = NULL;
	const char *internalIPAddress = NULL;

	
	if (json_object_object_get(Jobj, "RequiredInternalPort")) {
		// check Description is duplicated or not.
		requiredInternalPort = json_object_get_int(json_object_object_get(Jobj, "RequiredInternalPort"));
		IID_INIT(objIid);
		while( (ret = zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_CONTROL, &objIid, &NatPcpJobj)) == ZCFG_SUCCESS) {
			if(requiredInternalPort == json_object_get_int(json_object_object_get(NatPcpJobj, "RequiredInternalPort"))){
				if(replyMsg != NULL)
					strcat(replyMsg, "Required Internal Port is duplicated");
				zcfgFeJsonObjFree(NatPcpJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			zcfgFeJsonObjFree(NatPcpJobj);
		}
	}

	IID_INIT(objIid);
	ret = zcfgFeObjJsonAdd(RDM_OID_NAT_PORT_CONTROL, &objIid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGet(RDM_OID_NAT_PORT_CONTROL, &objIid, &NatPcpJobj);
	}
	
	if(ret == ZCFG_SUCCESS){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		requiredInternalPort = json_object_get_int(json_object_object_get(Jobj, "RequiredInternalPort"));
		requiredPublicPort = json_object_get_int(json_object_object_get(Jobj, "RequiredPublicPort"));
		allowPCPPortProposal = json_object_get_int(json_object_object_get(Jobj, "AllowPCPPortProposal"));
		protocol = json_object_get_string(json_object_object_get(Jobj, "Protocol"));
		internalIPAddress = json_object_get_string(json_object_object_get(Jobj, "InternalIPAddress"));
	
		json_object_object_add(NatPcpJobj, "Enable", json_object_new_boolean(enable));
		json_object_object_add(NatPcpJobj, "RequiredInternalPort", json_object_new_int(requiredInternalPort));
		json_object_object_add(NatPcpJobj, "RequiredPublicPort", json_object_new_int(requiredPublicPort));
		json_object_object_add(NatPcpJobj, "AllowPCPPortProposal", json_object_new_int(allowPCPPortProposal));
		json_object_object_add(NatPcpJobj, "Protocol", json_object_new_string(protocol));
		json_object_object_add(NatPcpJobj, "InternalIPAddress", json_object_new_string(internalIPAddress));

		ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_CONTROL, &objIid, NatPcpJobj, NULL);
	}
	
	zcfgFeJsonObjFree(NatPcpJobj);
	return ret;
}

zcfgRet_t zcfgFeDalNatPcpEdit(struct json_object *Jobj, char *replyMsg)
{

	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatPcpJobj = NULL;
	objIndex_t objIid = {0}, iid = {0};
	bool enable = false;
	int requiredInternalPort = 0;
	int requiredPublicPort = 0;
	int allowPCPPortProposal = 0;
	const char *protocol = NULL;
	const char *internalIPAddress = NULL;
	int index = 0;
	
	if (Jobj && json_object_object_get(Jobj, "idx") != NULL)
	{
		IID_INIT(objIid);
		objIid.level = 1;
		objIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}
	else
	{
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		ret = convertIndextoIid(index, &objIid, RDM_OID_NAT_PORT_CONTROL, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
	}

	if (json_object_object_get(Jobj, "RequiredInternalPort")) {
		// check Description is duplicated or not.
		requiredInternalPort = json_object_get_int(json_object_object_get(Jobj, "requiredInternalPort"));
		IID_INIT(iid);
		while( (ret = zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_CONTROL, &iid, &NatPcpJobj)) == ZCFG_SUCCESS) {
			if (objIid.idx[0] == iid.idx[0]) {
				zcfgFeJsonObjFree(NatPcpJobj);
				continue;
		}
			if(requiredInternalPort == json_object_get_int(json_object_object_get(NatPcpJobj, "RequiredInternalPort"))){
				if(replyMsg != NULL)
					strcat(replyMsg, "Required Internal Port is duplicated");
				zcfgFeJsonObjFree(NatPcpJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			zcfgFeJsonObjFree(NatPcpJobj);
		}
	}

	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT_PORT_CONTROL, &objIid, &NatPcpJobj)) != ZCFG_SUCCESS) {
		printf("%s():(ERROR) zcfgFeObjJsonGet get pcp fail oid(%d)\n ",__FUNCTION__,RDM_OID_NAT_PORT_CONTROL);
		return ZCFG_INTERNAL_ERROR;
	}

	if (json_object_object_get(Jobj, "Enable")) {
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		json_object_object_add(NatPcpJobj, "Enable", json_object_new_boolean(enable));
	}

	if (json_object_object_get(Jobj, "RequiredInternalPort")) {
		requiredInternalPort = json_object_get_int(json_object_object_get(Jobj, "RequiredInternalPort"));
		json_object_object_add(NatPcpJobj, "RequiredInternalPort", json_object_new_int(requiredInternalPort));
	}
	if (json_object_object_get(Jobj, "RequiredPublicPort")) {
		requiredPublicPort = json_object_get_int(json_object_object_get(Jobj, "RequiredPublicPort"));
		json_object_object_add(NatPcpJobj, "RequiredPublicPort", json_object_new_int(requiredPublicPort));
	}
	if (json_object_object_get(Jobj, "AllowPCPPortProposal")) {
		allowPCPPortProposal = json_object_get_int(json_object_object_get(Jobj, "AllowPCPPortProposal"));
		json_object_object_add(NatPcpJobj, "AllowPCPPortProposal", json_object_new_int(allowPCPPortProposal));
	}
	if (json_object_object_get(Jobj, "Protocol")) {
		protocol = json_object_get_string(json_object_object_get(Jobj, "Protocol"));
		json_object_object_add(NatPcpJobj, "Protocol", json_object_new_string(protocol));
	}
	if (json_object_object_get(Jobj, "InternalIPAddress")) {
		internalIPAddress = json_object_get_string(json_object_object_get(Jobj, "InternalIPAddress"));
		json_object_object_add(NatPcpJobj, "InternalIPAddress", json_object_new_string(internalIPAddress));
	}


	ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_CONTROL, &objIid, NatPcpJobj, NULL);
	zcfgFeJsonObjFree(NatPcpJobj);
	return ret;
}
zcfgRet_t zcfgFeDalNatPcpDelete(struct json_object *Jobj, char *replyMsg)
{
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatPcpJobj = NULL;
	objIndex_t objIid = {0};
	int index = 0;
	int count = 1;
	bool found = false;
	
	if (Jobj && json_object_object_get(Jobj, "idx") != NULL)
	{
		IID_INIT(objIid);
		objIid.level = 1;
		objIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
		ret = zcfgFeObjJsonDel(RDM_OID_NAT_PORT_CONTROL, &objIid, NULL);
		return ret;
	}
	
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	
	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_CONTROL, &objIid, &NatPcpJobj) == ZCFG_SUCCESS) {
		if(count == index){
			found = true;
			break;
		}
		else
			count++;
	}
	if(found){
		ret = zcfgFeObjJsonDel(RDM_OID_NAT_PORT_CONTROL, &objIid, NULL);
	}
	
	zcfgFeJsonObjFree(NatPcpJobj);
	return ret;
	
}

zcfgRet_t zcfgFeDalNatPcpGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	struct json_object *NatPcpJobj = NULL;
	struct json_object *paramJobj = NULL;
	objIndex_t objIid = {0};
	int index = 0, idx = -1;

	if (Jobj && json_object_object_get(Jobj, "idx") != NULL)
	{
		idx = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}

	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_CONTROL, &objIid, &NatPcpJobj) == ZCFG_SUCCESS) {
		++index;
		if (idx == -1 || objIid.idx[0] == idx)
		{
			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(NatPcpJobj, "Enable")));
			json_object_object_add(paramJobj, "ExternalIPAddress", JSON_OBJ_COPY(json_object_object_get(NatPcpJobj, "ExternalIPAddress")));
			json_object_object_add(paramJobj, "InternalIPAddress", JSON_OBJ_COPY(json_object_object_get(NatPcpJobj, "InternalIPAddress")));
			json_object_object_add(paramJobj, "RequiredInternalPort", JSON_OBJ_COPY(json_object_object_get(NatPcpJobj, "RequiredInternalPort")));
			json_object_object_add(paramJobj, "RequiredPublicPort", JSON_OBJ_COPY(json_object_object_get(NatPcpJobj, "RequiredPublicPort")));
			json_object_object_add(paramJobj, "AssignedPublicPort", JSON_OBJ_COPY(json_object_object_get(NatPcpJobj, "AssignedPublicPort")));
			json_object_object_add(paramJobj, "Protocol", JSON_OBJ_COPY(json_object_object_get(NatPcpJobj, "Protocol")));
			json_object_object_add(paramJobj, "ResultCode", JSON_OBJ_COPY(json_object_object_get(NatPcpJobj, "ResultCode")));
			json_object_object_add(paramJobj, "AllowPCPPortProposal", JSON_OBJ_COPY(json_object_object_get(NatPcpJobj, "AllowPCPPortProposal")));
			json_object_object_add(paramJobj, "Index", json_object_new_int(index));
			json_object_object_add(paramJobj, "idx", json_object_new_int(objIid.idx[0]));
			json_object_array_add(Jarray, paramJobj);
		}
		zcfgFeJsonObjFree(NatPcpJobj);
	}

	return ZCFG_SUCCESS;
	
}

void zcfgFeDalShowPcp(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-5s %-15s %-7s %-15s %-17s %-15s %-15s %-15s %-15s %-13s\n",
		    "Index", "ExternalIPAddress", "Enable", "InternalIPAddress", "RequiredInternalPort", "RequiredPublicPort", "AssignedPublicPort", "Protocol", "ResultCode", "AllowPCPPortProposal");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-5lld %-15s %-7d %-15s %-17lld %-15lld %-15lld %-15s %-15s %-13lld\n",
			json_object_get_int(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "ExternalIPAddress")),
			json_object_get_boolean(json_object_object_get(obj, "Enable")),
			json_object_get_string(json_object_object_get(obj, "InternalIPAddress")),
			json_object_get_int(json_object_object_get(obj, "RequiredInternalPort")),
			json_object_get_int(json_object_object_get(obj, "RequiredPublicPort")),
			json_object_get_int(json_object_object_get(obj, "AssignedPublicPort")),
			json_object_get_string(json_object_object_get(obj, "Protocol")),
			json_object_get_string(json_object_object_get(obj, "ResultCode")),
			json_object_get_int(json_object_object_get(obj, "AllowPCPPortProposal"))
			);

	}
}

zcfgRet_t zcfgFeDalNatPcp(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalNatPcpEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalNatPcpAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalNatPcpDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDalNatPcpGet(Jobj, Jarray, replyMsg);
	}

	return ret;
}
