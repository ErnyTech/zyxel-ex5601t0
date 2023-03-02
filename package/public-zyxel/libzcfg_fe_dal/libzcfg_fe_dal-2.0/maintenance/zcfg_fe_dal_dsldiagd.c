#ifdef ABUU_CUSTOMIZATION_DSL_TROUBLE_SHOOTING
#include <ctype.h>
#include <json/json.h>
#include <time.h>
#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t DSLDiagd_param[] =
{
	{"DSLDiagdEnable", 		dalType_boolean, 	0, 	0, 	NULL, 	NULL, 		0},
	{NULL, 					0, 				0, 	0, 	NULL}
};

zcfgRet_t zcfgFeDal_DSLDiagd_Edit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid = {0};
	struct json_object *obj = NULL;

	IID_INIT(iid);
	if (zcfgFeObjJsonGetWithoutUpdate(RDM_OID_D_S_L_DIAGD, &iid, &obj) == ZCFG_SUCCESS)
	{
		json_object_object_add(obj, "Enable", json_object_new_boolean(json_object_get_boolean(json_object_object_get(Jobj, "DSLDiagdEnable"))));
		zcfgFeObjJsonSet(RDM_OID_D_S_L_DIAGD, &iid, obj, NULL);
		zcfgFeJsonObjFree(obj);
	}
	
	return ret;
}

zcfgRet_t zcfgFeDal_DSLDiagd_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid = {0};
	struct json_object *obj = NULL;
	struct json_object *paramJobj = NULL;
	
	IID_INIT(iid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_D_S_L_DIAGD, &iid, &obj) == ZCFG_SUCCESS)
	{
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "DSLDiagdEnable", JSON_OBJ_COPY(json_object_object_get(obj, "Enable")));
		zcfgFeJsonObjFree(obj);
	}

	json_object_array_add(Jarray, paramJobj);

	return ret;
}

zcfgRet_t zcfgFeDalDSLDiagd(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
	{
		return ZCFG_INTERNAL_ERROR;
	}

	if(!strcmp(method, "PUT"))
	{
		ret = zcfgFeDal_DSLDiagd_Edit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET"))
	{
		ret = zcfgFeDal_DSLDiagd_Get(Jobj, Jarray, replyMsg);
	}
	else
	{
		printf("Unknown method:%s\n", method);
	}

	return ret;
}

#endif
