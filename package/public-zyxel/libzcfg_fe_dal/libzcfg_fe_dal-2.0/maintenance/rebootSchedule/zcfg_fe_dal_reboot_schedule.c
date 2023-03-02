#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t REBOOT_SCHEDULE_param[]={
	{"Enable",		dalType_boolean,	0,		0,		NULL},
	{"Mode",		dalType_int,		0,		2,		NULL},
	{"Week",		dalType_int,		0,		6,		NULL},
	{"Day",			dalType_int,		1,		31,		NULL},
	{"Hour",		dalType_int,		0,		23,		NULL},
	{"Minute",		dalType_int,		0,		59,		NULL},
	{NULL, 			0, 					0,		0,		NULL}
};

zcfgRet_t zcfgFeDalRebootScheduleEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid = {0};
	struct json_object *rebootScheduleObj = NULL;

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_REBOOT_SCHEDULE, &objIid, &rebootScheduleObj)) == ZCFG_SUCCESS) {
		if(json_object_object_get(Jobj, "Enable") != NULL)
			json_object_object_add(rebootScheduleObj, "Enable", JSON_OBJ_COPY(json_object_object_get(Jobj, "Enable")));
		if(json_object_object_get(Jobj, "Mode") != NULL)
			json_object_object_add(rebootScheduleObj, "Mode", JSON_OBJ_COPY(json_object_object_get(Jobj, "Mode")));
		if(json_object_object_get(Jobj, "Week") != NULL)
			json_object_object_add(rebootScheduleObj, "Week", JSON_OBJ_COPY(json_object_object_get(Jobj, "Week")));
		if(json_object_object_get(Jobj, "Day") != NULL)
			json_object_object_add(rebootScheduleObj, "Day", JSON_OBJ_COPY(json_object_object_get(Jobj, "Day")));
		if(json_object_object_get(Jobj, "Hour") != NULL)
			json_object_object_add(rebootScheduleObj, "Hour", JSON_OBJ_COPY(json_object_object_get(Jobj, "Hour")));
		if(json_object_object_get(Jobj, "Minute") != NULL)
			json_object_object_add(rebootScheduleObj, "Minute", JSON_OBJ_COPY(json_object_object_get(Jobj, "Minute")));
		
		ret = zcfgFeObjJsonSet(RDM_OID_REBOOT_SCHEDULE, &objIid, rebootScheduleObj, NULL);
		zcfgFeJsonObjFree(rebootScheduleObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalRebootScheduleGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid = {0};
	struct json_object *rebootScheduleObj = NULL;
	struct json_object *paramJobj = NULL;

	paramJobj = json_object_new_object();

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_REBOOT_SCHEDULE, &objIid, &rebootScheduleObj)) == ZCFG_SUCCESS) {
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(rebootScheduleObj, "Enable")));
		json_object_object_add(paramJobj, "Mode", JSON_OBJ_COPY(json_object_object_get(rebootScheduleObj, "Mode")));
		json_object_object_add(paramJobj, "Week", JSON_OBJ_COPY(json_object_object_get(rebootScheduleObj, "Week")));
		json_object_object_add(paramJobj, "Day", JSON_OBJ_COPY(json_object_object_get(rebootScheduleObj, "Day")));
		json_object_object_add(paramJobj, "Hour", JSON_OBJ_COPY(json_object_object_get(rebootScheduleObj, "Hour")));
		json_object_object_add(paramJobj, "Minute", JSON_OBJ_COPY(json_object_object_get(rebootScheduleObj, "Minute")));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(rebootScheduleObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalRebootSchedule(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalRebootScheduleEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalRebootScheduleGet(Jobj, Jarray, NULL);
	}
	else {
		printf("Unknown method:%s\n", method);
	}

	return ret;
}

