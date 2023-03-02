
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

extern int ip4Compare(char *ip1, char *ip2);
dal_param_t NAT_ADDR_MAPPING_param[] =
{
	{"Index",				dalType_string,	0,	0,	NULL,	NULL,									 dal_Edit|dal_Delete},
	{"Type",				dalType_string,	0,	0,	NULL,	"One-to-One|Many-to-One|Many-to-Many",	 dal_Add},
	{"LocalStartIP",		dalType_string,	0,	0,	NULL,	NULL,									 dal_Add},
	{"LocalEndIP",			dalType_string,	0,	0,	NULL,	NULL,									 0},
	{"GlobalStartIP",		dalType_string,	0,	0,	NULL,	NULL,									 dal_Add},
	{"GlobalEndIP",		dalType_string,	0,	0,	NULL,	NULL,									 0},
	{"MappingRuleName",	dalType_string,	0,	0,	NULL,	NULL,									 dal_Add},
#ifndef ABOG_CUSTOMIZATION
	{"Interface",			dalType_LanRtWanWWan,	0,	0,	NULL,	NULL,									 dal_Add},
	{"X_ZYXEL_AutoDetectWanStatus",	dalType_boolean,	0,	0,		NULL,	NULL,				dalcmd_Forbid},
#endif
	{NULL,				0,				0,	0,	NULL,	NULL,									 dal_Add}
};

//static char *getIPInterfaceTR181Name(char *name)
//{
//	struct json_object *IpIfaceObj = NULL;
//	objIndex_t IpIfaceIid = {0};
//	static char tr181IPIfaceName[32];
//
//	if (name == NULL)
//		return NULL;
//	memset(tr181IPIfaceName, 0, sizeof(tr181IPIfaceName));
//	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &IpIfaceIid, &IpIfaceObj) == ZCFG_SUCCESS){
//		if(!strcmp(name,json_object_get_string(json_object_object_get(IpIfaceObj, "X_ZYXEL_SrvName")))){
//			sprintf(tr181IPIfaceName, "IP.Interface.%d",  IpIfaceIid.idx[0]);
//			zcfgFeJsonObjFree(IpIfaceObj);
//			return &tr181IPIfaceName[0];
//		}
//		zcfgFeJsonObjFree(IpIfaceObj);
//	}
//	return NULL;
//}

//static char *getIPInterfaceSrvName(char *ifname)
//{
//	struct json_object *IpIfaceObj = NULL;
//	objIndex_t IpIfaceIid = {0};
//	static char srvName[32];
//
//	if (ifname == NULL)
//		return NULL;
//	memset(srvName, 0, sizeof(srvName));
//	IpIfaceIid.level = 1;
//	sscanf(ifname, "IP.Interface.%hhu", &IpIfaceIid.idx[0]);
//	if (zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &IpIfaceIid, &IpIfaceObj) == ZCFG_SUCCESS){
//		strcpy(srvName, json_object_get_string(json_object_object_get(IpIfaceObj, "X_ZYXEL_SrvName")));
//		zcfgFeJsonObjFree(IpIfaceObj);
//		return &srvName[0];
//	}
//	return NULL;
//}

zcfgRet_t zcfgFeDalNatAddrMappingAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *paraName;
	objIndex_t objIid;
	struct json_object *NatAddrMappingJobj = NULL;
	int n=0;
	bool autoDetectWanStatus = false;
	paraName = (const char *)NAT_ADDR_MAPPING_param[n++].paraName;

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_NAT_ADDR_MAPPING, &objIid, NULL)) != ZCFG_SUCCESS) {
		printf("%s: fail to add NAT address mapping with ret = %d\n", __FUNCTION__, ret);
		return ret;
	}
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT_ADDR_MAPPING, &objIid, &NatAddrMappingJobj)) != ZCFG_SUCCESS) {
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}	
	while(paraName){
		struct json_object *paramValue = json_object_object_get(Jobj, paraName);
		if(!paramValue) {
			paraName = (const char *)NAT_ADDR_MAPPING_param[n++].paraName;
			continue;
		}
#ifndef ABOG_CUSTOMIZATION
		if(!strcmp(paraName, "Interface")){
			const char *Interface = json_object_get_string(paramValue);
			if(strlen(Interface)== 0){
				autoDetectWanStatus = true;
			}
		}
#endif
		json_object_object_add(NatAddrMappingJobj, paraName, JSON_OBJ_COPY(paramValue));
		paraName = (const char *)NAT_ADDR_MAPPING_param[n++].paraName;
	}
#ifdef ABOG_CUSTOMIZATION
		json_object_object_add(NatAddrMappingJobj, "Interface", json_object_new_string(""));
#endif
    json_object_object_add(NatAddrMappingJobj, "X_ZYXEL_AutoDetectWanStatus", json_object_new_boolean(autoDetectWanStatus));	
	if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_ADDR_MAPPING, &objIid, NatAddrMappingJobj, NULL)) == ZCFG_SUCCESS)
	{
		sprintf(replyMsg, "[%d,%d,%d,%d,%d,%d]",objIid.idx[0],objIid.idx[1],objIid.idx[2],
				objIid.idx[3],objIid.idx[4],objIid.idx[5]);
		printf("reply msg %s\n",replyMsg);		
	}
	
	json_object_put(NatAddrMappingJobj);
	return ret;
}

extern bool validateIPv4(const char *ipStr); // implement zcfg_fe_dal.c
zcfgRet_t zcfgFeDalNatAddrMappingEdit(struct json_object *Jobj, char *replyMsg)
{

	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *paraName;
	objIndex_t objIid;
	struct json_object *NatAddrMappingJobj = NULL;
	int paramModified = 0, n = 0;
	struct json_object *iidArray = NULL;
	uint8_t iidIdx =0;
	bool autoDetectWanStatus = false;
	paraName = (const char *)NAT_ADDR_MAPPING_param[n++].paraName;
	
	IID_INIT(objIid);
	if ((iidArray = json_object_object_get(Jobj, "iid")) != NULL) {
	for(iidIdx=0;iidIdx<6;iidIdx++){
		objIid.idx[iidIdx] = json_object_get_int( json_object_array_get_idx(iidArray, iidIdx));
		if(objIid.idx[iidIdx] != 0)
			objIid.level = iidIdx+1;
	}
	} else if ( (iidArray = json_object_object_get(Jobj, "Index")) != NULL) {
		int indexGet = -1;

		indexGet = json_object_get_int(iidArray);
		IID_INIT(objIid);
		objIid.level = 1;
		objIid.idx[0] = indexGet;
		json_object_object_del(Jobj, "Index");
	}
	else {
		if(replyMsg != NULL)
			strcat(replyMsg, "internal error");
		return ZCFG_INTERNAL_ERROR;
	}

	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT_ADDR_MAPPING, &objIid, &NatAddrMappingJobj)) != ZCFG_SUCCESS) {
		return ret;
	}
	while(paraName){
		struct json_object *paramValue = json_object_object_get(Jobj, paraName);
		if(!paramValue) {
			paraName = (const char *)NAT_ADDR_MAPPING_param[n++].paraName;
			continue;
		}
#ifndef ABOG_CUSTOMIZATION
		if(!strcmp(paraName, "Interface")){
			const char *Interface = json_object_get_string(paramValue);
			if(strlen(Interface)== 0){
				autoDetectWanStatus = true;
			}
		}
#endif
		json_object_object_add(NatAddrMappingJobj, paraName, JSON_OBJ_COPY(paramValue));
		paramModified = 1;				
		paraName = (const char *)NAT_ADDR_MAPPING_param[n++].paraName;
	}
	json_object_object_add(NatAddrMappingJobj, "X_ZYXEL_AutoDetectWanStatus", json_object_new_boolean(autoDetectWanStatus));
	if(paramModified) {
		char *type = (char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "Type"));
		if (strcmp("One-to-One", type) == 0) {
			json_object_object_add(NatAddrMappingJobj, "LocalEndIP", json_object_new_string(""));
			json_object_object_add(NatAddrMappingJobj, "GlobalEndIP", json_object_new_string(""));
			if (!validateIPv4((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalStartIP")))
						|| !validateIPv4((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "GlobalStartIP")))) {
				if(replyMsg != NULL)
					strcat(replyMsg, "(ipv4 format error)");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (strcmp("0.0.0.0", json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalStartIP")))==0) {
				if(replyMsg != NULL)
					strcat(replyMsg, "LocalStartIP error");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (strcmp("0.0.0.0", json_object_get_string(json_object_object_get(NatAddrMappingJobj, "GlobalStartIP")))==0) {
				if(replyMsg != NULL)
					strcat(replyMsg, "GlobalStartIP error");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}
		else if (strcmp("Many-to-One", type) == 0) {
			json_object_object_add(NatAddrMappingJobj, "GlobalEndIP", json_object_new_string(""));
			if (!validateIPv4((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalStartIP")))
						|| !validateIPv4((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalEndIP")))
						|| !validateIPv4((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "GlobalStartIP")))) {
				if(replyMsg != NULL)
					strcat(replyMsg, "(ipv4 format error)");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (strcmp("0.0.0.0", json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalStartIP")))==0) {
				if(replyMsg != NULL)
					strcat(replyMsg, "LocalStartIP error");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (strcmp("0.0.0.0", json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalEndIP")))==0) {
				if(replyMsg != NULL)
					strcat(replyMsg, "LocalEndIP error");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (strcmp("0.0.0.0", json_object_get_string(json_object_object_get(NatAddrMappingJobj, "GlobalStartIP")))==0) {
				if(replyMsg != NULL)
					strcat(replyMsg, "GlobalStartIP error");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}

			if (ip4Compare((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalStartIP")), (char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalEndIP")))!=1) {
				if(replyMsg != NULL)
					strcat(replyMsg, "Local IP range is invalid");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}
		else if (strcmp("Many-to-Many", type) == 0) {
			if (!validateIPv4((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalStartIP")))
						|| !validateIPv4((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalEndIP")))
						|| !validateIPv4((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "GlobalStartIP")))
						|| !validateIPv4((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "GlobalEndIP")))) {
				if(replyMsg != NULL)
					strcat(replyMsg, "(ipv4 format error)");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (strcmp("0.0.0.0", json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalStartIP")))==0) {
				if(replyMsg != NULL)
					strcat(replyMsg, "LocalStartIP error");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (strcmp("0.0.0.0", json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalEndIP")))==0) {
				if(replyMsg != NULL)
					strcat(replyMsg, "LocalEndIP error");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (strcmp("0.0.0.0", json_object_get_string(json_object_object_get(NatAddrMappingJobj, "GlobalStartIP")))==0) {
				if(replyMsg != NULL)
					strcat(replyMsg, "GlobalStartIP error");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (strcmp("0.0.0.0", json_object_get_string(json_object_object_get(NatAddrMappingJobj, "GlobalEndIP")))==0) {
				if(replyMsg != NULL)
					strcat(replyMsg, "GlobalEndIP error");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (ip4Compare((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalStartIP")), (char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "LocalEndIP")))!=1) {
				if(replyMsg != NULL)
					strcat(replyMsg, "Local IP range is invalid");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			if (ip4Compare((char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "GlobalStartIP")), (char *)json_object_get_string(json_object_object_get(NatAddrMappingJobj, "GlobalEndIP")))!=1) {
				if(replyMsg != NULL)
					strcat(replyMsg, "Global IP range is invalid");
				zcfgFeJsonObjFree(NatAddrMappingJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}
		else {
			if(replyMsg != NULL)
				strcat(replyMsg, "Type error");
			zcfgFeJsonObjFree(NatAddrMappingJobj);
			return ZCFG_INVALID_PARAM_VALUE;
		}

		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_ADDR_MAPPING, &objIid, NatAddrMappingJobj, NULL)) != ZCFG_SUCCESS) {
			if(replyMsg != NULL)
				strcat(replyMsg, "Internal error");
		}
	}
	zcfgFeJsonObjFree(NatAddrMappingJobj);
	return ret;
}
zcfgRet_t zcfgFeDalNatAddrMappingDelete(struct json_object *Jobj, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *iidArray = NULL;
	uint8_t iidIdx =0;

	IID_INIT(objIid);
	if ( (iidArray = json_object_object_get(Jobj, "iid")) != NULL ) {
	for(iidIdx=0;iidIdx<6;iidIdx++){
		objIid.idx[iidIdx] = json_object_get_int( json_object_array_get_idx(iidArray, iidIdx));
		if(objIid.idx[iidIdx] != 0)
			objIid.level = iidIdx+1;
	}
	} else if ( (iidArray = json_object_object_get(Jobj, "Index")) != NULL ) {
		int index = 0;
	
		index = json_object_get_int(iidArray);
	
		IID_INIT(objIid);
		objIid.level = 1;
		objIid.idx[0] = index;
		ret = zcfgFeObjJsonDel(RDM_OID_NAT_ADDR_MAPPING, &objIid, NULL);
		if (ret != ZCFG_SUCCESS)
		{
			if(replyMsg != NULL)
				strcat(replyMsg, "internal error");
			return ZCFG_INTERNAL_ERROR;
		}
		return ret;
	}
	else {
		if(replyMsg != NULL)
			strcat(replyMsg, "internal error");
		return ZCFG_INTERNAL_ERROR;
	}
	
	zcfgFeObjJsonDel(RDM_OID_NAT_ADDR_MAPPING, &objIid, NULL);
	
	return ret;
	
}

void zcfgFeDalShowAddrMapping(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
#ifdef ABOG_CUSTOMIZATION
	printf("%-5s %-15s %-15s %-15s %-15s %-15s %-15s\n",
		    "Index", "MappingRuleName", "Type", "LocalStartIP", "LocalEndIP", "GlobalStartIP", "GlobalEndIP");
#else
	printf("%-5s %-15s %-15s %-15s %-15s %-15s %-15s %-15s\n",
		    "Index", "MappingRuleName", "Interface", "Type", "LocalStartIP", "LocalEndIP", "GlobalStartIP", "GlobalEndIP");
#endif
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
#ifdef ABOG_CUSTOMIZATION
		printf("%-5s %-15s %-15s %-15s %-15s %-15s %-15s\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "MappingRuleName")),
			json_object_get_string(json_object_object_get(obj, "Type")),
			json_object_get_string(json_object_object_get(obj, "LocalStartIP")),
			json_object_get_string(json_object_object_get(obj, "LocalEndIP")),
			json_object_get_string(json_object_object_get(obj, "GlobalStartIP")),
			json_object_get_string(json_object_object_get(obj, "GlobalEndIP")));
#else
		printf("%-5s %-15s %-15s %-15s %-15s %-15s %-15s %-15s\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "MappingRuleName")),
			json_object_get_string(json_object_object_get(obj, "Interface")),
			json_object_get_string(json_object_object_get(obj, "Type")),
			json_object_get_string(json_object_object_get(obj, "LocalStartIP")),
			json_object_get_string(json_object_object_get(obj, "LocalEndIP")),
			json_object_get_string(json_object_object_get(obj, "GlobalStartIP")),
			json_object_get_string(json_object_object_get(obj, "GlobalEndIP")));
#endif
	}
}

zcfgRet_t zcfgFeDalNatAddrMappingGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	struct json_object *NatAddrMappingJobj = NULL;
	struct json_object *paramJobj = NULL;
	struct json_object *indexObj = NULL;
	objIndex_t objIid = {0};
	int indexGet = -1;

	if ((indexObj = json_object_object_get(Jobj, "Index")) != NULL)
	{
		indexGet = json_object_get_int(indexObj);
	}

	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_NAT_ADDR_MAPPING, &objIid, &NatAddrMappingJobj) == ZCFG_SUCCESS) {
		if (indexGet == -1 || objIid.idx[0] == indexGet)
		{
			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "Enable")));
			json_object_object_add(paramJobj, "Type", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "Type")));
			json_object_object_add(paramJobj, "LocalStartIP", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "LocalStartIP")));
			json_object_object_add(paramJobj, "LocalEndIP", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "LocalEndIP")));
			json_object_object_add(paramJobj, "GlobalStartIP", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "GlobalStartIP")));
			json_object_object_add(paramJobj, "GlobalEndIP", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "GlobalEndIP")));
			json_object_object_add(paramJobj, "MappingSet", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "MappingSet")));
			json_object_object_add(paramJobj, "MappingRuleName", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "MappingRuleName")));
			json_object_object_add(paramJobj, "Description", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "Description")));
			json_object_object_add(paramJobj, "WANIntfName", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "WANIntfName")));
			json_object_object_add(paramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "Interface")));
            json_object_object_add(paramJobj, "X_ZYXEL_AutoDetectWanStatus", JSON_OBJ_COPY(json_object_object_get(NatAddrMappingJobj, "X_ZYXEL_AutoDetectWanStatus")));
			json_object_object_add(paramJobj, "Index", json_object_new_int(objIid.idx[0]));
			json_object_array_add(Jarray, paramJobj);
		}
		zcfgFeJsonObjFree(NatAddrMappingJobj);
	}
	return ZCFG_SUCCESS;
	
}

zcfgRet_t zcfgFeDalNatAddrMapping(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalNatAddrMappingEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalNatAddrMappingAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalNatAddrMappingDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDalNatAddrMappingGet(Jobj, Jarray, replyMsg);
	}

	return ret;
}
