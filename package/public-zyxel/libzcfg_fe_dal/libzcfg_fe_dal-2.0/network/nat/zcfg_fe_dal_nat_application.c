
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t NAT_Application_param[] =
{
	{"Enable",		dalType_boolean,	0,	0,	NULL},
	{"ExternalPortStart",		dalType_int,	0,	63737,	NULL},
	{"ExternalPortEnd",		dalType_int,	0,	63737,	NULL},
	{"InternalPortStart",		dalType_int,	0,	63737,	NULL},
	{"InternalPortEnd",		dalType_int,	0,	63737,	NULL},
	{"InternalClient",		dalType_string,	0,	256,	NULL},
	{"Description",		dalType_string,	0,	256,	NULL},
#ifndef ABOG_CUSTOMIZATION
	{"Interface",		dalType_string,	0,	256,	NULL},
	{"X_ZYXEL_AutoDetectWanStatus",	dalType_boolean, 0,	0,	NULL},
#endif
	{"Protocol",		dalType_string,	0,	3,	NULL},	
	{NULL,		0,	0,	0,	NULL}
};

zcfgRet_t zcfgFeDalNatApplicationAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatAppJobj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t natIid = {0};
	int reqObjIdx = 0;
	bool enable = false;
	int externalport = 0;
	int externalportendrange = 0;
	int internalport = 0;
	int internalportendrange = 0;
	const char *internalclient = NULL;
	const char *description = NULL;
	const char *interface = NULL;
	const char *protocol = NULL;
    bool autoDetectWanStatus = false;
	

	while((reqObject = json_object_array_get_idx(Jobj, reqObjIdx)) !=NULL){
		IID_INIT(natIid);
		reqObjIdx++;
		ret = zcfgFeObjJsonAdd(RDM_OID_NAT_PORT_MAPPING, &natIid, NULL);
			if(ret == ZCFG_SUCCESS){
				ret = zcfgFeObjJsonGet(RDM_OID_NAT_PORT_MAPPING, &natIid, &NatAppJobj);
			}
		
		//set Param
		if(ret == ZCFG_SUCCESS){
			enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
			externalport = json_object_get_int(json_object_object_get(reqObject, "ExternalPortStart"));
			externalportendrange = json_object_get_int(json_object_object_get(reqObject, "ExternalPortEnd"));
			internalport = json_object_get_int(json_object_object_get(reqObject, "InternalPortStart"));
			internalportendrange = json_object_get_int(json_object_object_get(reqObject, "InternalPortEnd"));
			internalclient = json_object_get_string(json_object_object_get(reqObject, "InternalClient"));
			description = json_object_get_string(json_object_object_get(reqObject, "Description"));
#ifdef ABOG_CUSTOMIZATION
			autoDetectWanStatus = true;
#else
			interface = json_object_get_string(json_object_object_get(reqObject, "Interface"));
			if(!strcmp(interface,"")){
				autoDetectWanStatus = true;
			}
#endif
			protocol = json_object_get_string(json_object_object_get(reqObject, "Protocol"));

			json_object_object_add(NatAppJobj, "Enable", json_object_new_boolean(enable));
			json_object_object_add(NatAppJobj, "ExternalPort", json_object_new_int(externalport));
			json_object_object_add(NatAppJobj, "ExternalPortEndRange", json_object_new_int(externalportendrange));
			json_object_object_add(NatAppJobj, "InternalPort", json_object_new_int(internalport));
			json_object_object_add(NatAppJobj, "X_ZYXEL_InternalPortEndRange", json_object_new_int(internalportendrange));
			json_object_object_add(NatAppJobj, "InternalClient", json_object_new_string(internalclient));
			json_object_object_add(NatAppJobj, "Description", json_object_new_string(description));
#ifdef ABOG_CUSTOMIZATION
			json_object_object_add(NatAppJobj, "Interface", json_object_new_string(""));
#else
			json_object_object_add(NatAppJobj, "Interface", json_object_new_string(interface));
#endif
			json_object_object_add(NatAppJobj, "X_ZYXEL_AutoDetectWanStatus", json_object_new_boolean(autoDetectWanStatus));
			json_object_object_add(NatAppJobj, "Protocol", json_object_new_string(protocol));
	
			zcfgFeObjJsonSet(RDM_OID_NAT_PORT_MAPPING, &natIid, NatAppJobj, NULL);
		}
	}
	zcfgFeJsonObjFree(NatAppJobj);
		return ret;
	
	}

zcfgRet_t zcfgFeDalNatApplicationDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	struct json_object *iidArray = NULL;
	objIndex_t iid = {0};
	const char *key = NULL, *description = NULL;
	key = json_object_get_string(json_object_object_get(Jobj, "key"));
	iidArray = json_object_object_get(Jobj, "iid");
	int i, index = 0;
	if(json_object_object_get(Jobj, "key")){	//new GUI delete parameter from webGUI
		while(zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_MAPPING, &iid, &obj) == ZCFG_SUCCESS){
			description = json_object_get_string(json_object_object_get(obj, "Description"));
			if(!strcmp(key, description)){
				zcfgFeObjJsonDel(RDM_OID_NAT_PORT_MAPPING, &iid, NULL);

			}		
			zcfgFeJsonObjFree(obj);
		}
	}
	else if(iidArray != NULL){
		for(i=0;i<10;i++){
			index = json_object_get_int( json_object_array_get_idx(iidArray, i));
			if(index == 0)
				break;
			iid.idx[0] = index;
			iid.level = 1;
			zcfgFeObjJsonDel(RDM_OID_NAT_PORT_MAPPING, &iid, NULL);
		}
	}
	return ret;	
}
zcfgRet_t zcfgFeDalNatApplicationCheck(struct json_object *Jobj, char *replyMsg){	
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatAppJobj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t natIid = {0};
	int reqObjIdx = 0;
	int externalport = 0;
	int externalport_datamodel = 0;
	int externalportendrange = 0;
	int externalportendrange_datamodel = 0;
	const char *description = NULL;
	const char *description_datamodel = NULL;
	const char *protocol = NULL;
	const char *protocol_datamodel = NULL;
	while((reqObject = json_object_array_get_idx(Jobj, reqObjIdx)) !=NULL){
		IID_INIT(natIid);
		reqObjIdx++;
		externalport = json_object_get_int(json_object_object_get(reqObject, "ExternalPortStart"));
		externalportendrange = json_object_get_int(json_object_object_get(reqObject, "ExternalPortEnd"));
		description = json_object_get_string(json_object_object_get(reqObject, "Description"));
		protocol = json_object_get_string(json_object_object_get(reqObject, "Protocol"));
		while(zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_MAPPING, &natIid, &NatAppJobj) == ZCFG_SUCCESS) { 
			externalport_datamodel = json_object_get_int(json_object_object_get(NatAppJobj, "ExternalPortStart"));
			externalportendrange_datamodel = json_object_get_int(json_object_object_get(NatAppJobj, "ExternalPortEnd"));
			description_datamodel = json_object_get_string(json_object_object_get(NatAppJobj, "Description"));
			protocol_datamodel = json_object_get_string(json_object_object_get(NatAppJobj, "Protocol"));				
			if(!strcmp(description,description_datamodel) && !strcmp(protocol,protocol_datamodel) 
				&& externalport-externalport_datamodel == 0 && externalportendrange-externalportendrange_datamodel == 0){
				zcfgFeObjJsonDel(RDM_OID_NAT_PORT_MAPPING, &natIid, NULL);
				zcfgFeJsonObjFree(NatAppJobj);
			}
			zcfgFeJsonObjFree(NatAppJobj);
		}
	}	
	return ret;
}

zcfgRet_t zcfgFeDalNatApplicationGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *obj = NULL, *JarrObj = NULL;
	objIndex_t iid = {0};
	char *description = NULL;
	int len = 0, i;
	bool AddRule = false;

	while(zcfgFeObjJsonGetNext(RDM_OID_NAT_PORT_MAPPING, &iid, &obj) == ZCFG_SUCCESS){
		description = (char *)json_object_get_string(json_object_object_get(obj, "Description"));
		if(!strncmp("APP:", description, 4)){
			len = json_object_array_length(Jarray);
			if(len == 0){
				AddRule = true;
			}
			else{
				for(i = 0; i < len; i++){
					JarrObj	= json_object_array_get_idx(Jarray, i);
					if(!strcmp(json_object_get_string(json_object_object_get(JarrObj, "Original_Description")), description)){
						AddRule = false;
					}
					else {
						AddRule = true;
					}
				}
			}
			if(AddRule){
				paramJobj = json_object_new_object();
				json_object_object_add(paramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(obj, "Interface")));
				json_object_object_add(paramJobj, "InternalClient", JSON_OBJ_COPY(json_object_object_get(obj, "InternalClient")));
				json_object_object_add(paramJobj, "X_ZYXEL_AutoDetectWanStatus", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_AutoDetectWanStatus")));
				json_object_object_add(paramJobj, "Original_Description", json_object_new_string(description));
				description = (description + 5);
				json_object_object_add(paramJobj, "Description", json_object_new_string(description));
				json_object_array_add(Jarray, paramJobj);
			}
		}
		zcfgFeJsonObjFree(obj);
	}
	return ret;
}


zcfgRet_t zcfgFeDalNatApplication(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDalNatApplicationCheck(Jobj, NULL);
		ret = zcfgFeDalNatApplicationAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalNatApplicationDelete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalNatApplicationGet(Jobj, Jarray, NULL);
	}

	return ret;
}
