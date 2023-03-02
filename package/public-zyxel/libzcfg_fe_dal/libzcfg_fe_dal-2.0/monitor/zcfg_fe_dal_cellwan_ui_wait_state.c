#include <ctype.h>
#include <json/json.h>
#include <time.h>
#include <sys/queue.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t CELLWAN_WAIT_STATE_param[]={
{"WAIT_STATE", dalType_string, 0, 0, NULL},
{"WAIT_STATE_APN", dalType_string, 0, 0, NULL},
{"WAIT_STATE_BAND", dalType_string, 0, 0, NULL},
{"WAIT_STATE_PIN", dalType_string, 0, 0, NULL},
{"WAIT_STATE_PLMN", dalType_string, 0, 0, NULL},
{"WAIT_STATE_SMS", dalType_string, 0, 0, NULL},
{ NULL,	0,	0,	0,	NULL}

};


zcfgRet_t zcfgFeDalCellWanWaitStateGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL, *intfZyUiWaitStateObj = NULL;
	objIndex_t iid = {0};
	paramJobj = json_object_new_object();

	IID_INIT(iid);
	iid.level=1;
	iid.idx[0]=1;
	if((ret =  zcfgFeObjJsonGet(RDM_OID_CELL_INTF_ZY_UI_WAIT_ST, &iid, &intfZyUiWaitStateObj)) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "WAIT_STATE_SMS", JSON_OBJ_COPY(json_object_object_get(intfZyUiWaitStateObj, "SmsWaitingStatus")));
		zcfgFeJsonObjFree(intfZyUiWaitStateObj);
	}

	json_object_array_add(Jarray, paramJobj);

	return ret;
}

void zcfgFeDalShowCellWanWaitState(struct json_object *Jarray){
	struct json_object *obj = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	obj = json_object_array_get_idx(Jarray, 0);
	
	printf("\nCellular UI Waiting State:\n");
	printf("%-30s %-10s \n","SMS:", json_object_get_string(json_object_object_get(obj, "WAIT_STATE_SMS")));

}

zcfgRet_t zcfgFeDalCellWanWaitStateEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid = {0};
	struct json_object *intfZyUiWaitStateObj = NULL;
	const char *waitStateSms = NULL;

	IID_INIT(iid);
	iid.level=1;
	iid.idx[0]=1;

	waitStateSms = json_object_get_string(json_object_object_get(Jobj, "WAIT_STATE_SMS"));

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF_ZY_UI_WAIT_ST, &iid, &intfZyUiWaitStateObj)) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "WAIT_STATE_SMS"))
			json_object_object_add(intfZyUiWaitStateObj, "SmsWaitingStatus", json_object_new_string(waitStateSms));
		zcfgFeObjJsonSet(RDM_OID_CELL_INTF_ZY_UI_WAIT_ST, &iid, intfZyUiWaitStateObj, NULL);
	}

	zcfgFeJsonObjFree(intfZyUiWaitStateObj);

	return ret;
}


zcfgRet_t zcfgFeDalCellWanWaitState(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalCellWanWaitStateEdit(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalCellWanWaitStateGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

