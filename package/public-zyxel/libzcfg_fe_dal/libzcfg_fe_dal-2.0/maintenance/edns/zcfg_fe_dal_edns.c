#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#ifdef ZYXEL_TAAAG_EDNS
dal_param_t EDNS_param[] =
{
	{"EDNSEnable",		dalType_boolean,	0,	0,	NULL, 	NULL, 	0},
	{NULL,		0,	0,	0,	NULL,	NULL, 0}
};

zcfgRet_t zcfgFeDalEDNSGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t ednsIid={0};
	struct json_object *ednsObj = NULL;
	struct json_object *pramJobj = NULL;

	IID_INIT(ednsIid);
	if(zcfgFeObjJsonGet(RDM_OID_E_D_N_S, &ednsIid, &ednsObj) != ZCFG_SUCCESS){
		return ret;
	}

	pramJobj = json_object_new_object();
	json_object_object_add(pramJobj, "EDNSEnable", JSON_OBJ_COPY(json_object_object_get(ednsObj, "Enable")));
	json_object_array_add(Jarray, pramJobj);

	zcfgFeJsonObjFree(ednsObj);

	return ret;
}

zcfgRet_t zcfgFeDalEDNSEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	bool EDNSEnable = false;
	EDNSEnable = json_object_get_int(json_object_object_get(Jobj, "EDNSEnable"));
	struct json_object *ednsObj = NULL;
	objIndex_t ednsIid;

	IID_INIT(ednsIid);
	if(zcfgFeObjJsonGet(RDM_OID_E_D_N_S, &ednsIid, &ednsObj) == ZCFG_SUCCESS){
		json_object_object_add(ednsObj, "Enable", json_object_new_boolean(EDNSEnable));
		if(EDNSEnable)
			json_object_object_add(ednsObj, "State", json_object_new_string("Enabled"));
		else
			json_object_object_add(ednsObj, "State", json_object_new_string("Disabled"));
		
		zcfgFeObjJsonBlockedSet(RDM_OID_E_D_N_S, &ednsIid, ednsObj, NULL);
		zcfgFeJsonObjFree(ednsObj);
	}
	return ret;
}

zcfgRet_t zcfgFeDalEDNS(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!method || !Jobj){
		return ZCFG_INTERNAL_ERROR;
	}

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalEDNSEdit(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalEDNSGet(Jobj, Jarray, replyMsg);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
#endif
