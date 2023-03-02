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

dal_param_t SENSOR_param[]={
{"Enable", 		dalType_boolean,	0,	0,	NULL},
{NULL, 			0, 					0, 0, NULL}
};

zcfgRet_t zcfgFeDal_Sensor_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t iid = {0};
	bool enable = false;

	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	if(json_object_object_get(Jobj, "Enable")){
		if(zcfgFeObjJsonGet(RDM_OID_ZY_PROXIMITY_SENSOR, &iid, &obj) == ZCFG_SUCCESS){
			json_object_object_add(obj, "Enable", json_object_new_boolean(enable));
			ret = zcfgFeObjJsonSet(RDM_OID_ZY_PROXIMITY_SENSOR, &iid, obj, NULL);
			zcfgFeJsonObjFree(obj);
		}		
	}
	return ret;
}

zcfgRet_t zcfgFeDal_Sensor_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	struct json_object *paramJobj = NULL;
	objIndex_t iid = {0};

	paramJobj = json_object_new_object();
	if(zcfgFeObjJsonGet(RDM_OID_ZY_PROXIMITY_SENSOR, &iid, &obj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(obj, "Enable")));
		zcfgFeJsonObjFree(obj);
	}
	json_object_array_add(Jarray, paramJobj);
	printf("%s %s %d \n",json_object_to_json_string(Jarray),__func__,__LINE__);
	
	return ret;
}

zcfgRet_t zcfgFeDalSensor(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_Sensor_Edit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_Sensor_Get(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}

	return ret;
}

