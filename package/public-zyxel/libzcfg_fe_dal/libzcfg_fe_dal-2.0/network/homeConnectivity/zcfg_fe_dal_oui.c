#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t OUI_param[]={
	{"Enable", 			dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{NULL,					0,	0,	0,	NULL,	NULL,	0},
};

void zcfgFeDalShowoui(struct json_object *Jarray){
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	printf("OUI name replace enalbe state: ");
	printf("%-10s \n", json_object_get_string(json_object_object_get(obj, "Enable")));
}

zcfgRet_t zcfgFeDal_oui_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ouiObj = NULL;
	objIndex_t ouiIid = {0};
	bool Enable = false;

	IID_INIT(ouiIid);
	Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	if(zcfgFeObjJsonGet(RDM_OID_ZY_HOST_NAME_REPLACE, &ouiIid, &ouiObj) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "Enable"))
		json_object_object_add(ouiObj, "Enable", json_object_new_boolean(Enable));
		zcfgFeObjJsonSet(RDM_OID_ZY_HOST_NAME_REPLACE, &ouiIid, ouiObj, NULL);
		zcfgFeJsonObjFree(ouiObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDal_oui_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *ouiObj = NULL;
	objIndex_t ouiIid = {0};

	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_HOST_NAME_REPLACE, &ouiIid, &ouiObj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(ouiObj, "Enable")));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(ouiObj);
	}
	return ret;
}

zcfgRet_t zcfgFeDaloui(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_oui_Edit(Jobj, NULL);		
	}
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_oui_Get(Jobj, Jarray, NULL);	
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
