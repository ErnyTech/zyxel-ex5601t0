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

dal_param_t SFP_MGMT_param[]={
#ifdef ABPY_CUSTOMIZATION
{"Enable",		dalType_boolean,	0, 0, NULL},
#endif
{"Timer", 		dalType_int,		0, 10080, NULL},
{NULL, 			0, 					0, 0, NULL}
};

zcfgRet_t zcfgFeDal_SfpMgmt_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ssfpMgmtObj = NULL;
	objIndex_t objIid = {0};
	int timer = 0;

	timer = json_object_get_int(json_object_object_get(Jobj, "Timer"));
	
#ifdef ABPY_CUSTOMIZATION
	bool enable = false;
	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
#endif

	IID_INIT(objIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_SFP_MANAGEMENT, &objIid, &ssfpMgmtObj);
	if(ret == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "Timer"))
			json_object_object_add(ssfpMgmtObj, "Timer", json_object_new_int(timer));
		
#ifdef ABPY_CUSTOMIZATION
		if(json_object_object_get(Jobj, "Enable"))
			json_object_object_add(ssfpMgmtObj, "Enable", json_object_new_boolean(enable));
#endif

		zcfgFeObjJsonSet(RDM_OID_ZY_SFP_MANAGEMENT, &objIid, ssfpMgmtObj, NULL);
	}

	zcfgFeJsonObjFree(ssfpMgmtObj);
	return ret;
}

void zcfgFeDalShowSfpMgmt(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray,i);
		printf("%-20s: %s\n","Current power status ",json_object_get_string(json_object_object_get(obj, "Status")));
		printf("%-20s: %s\n","Disable Power time ",json_object_get_string(json_object_object_get(obj, "Timer")));
	}
}

zcfgRet_t zcfgFeDal_SfpMgmt_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *retObj = NULL;
	struct json_object *sfpMgmtObj = NULL;
	objIndex_t objIid = {0};
	int timer = 0; 
	const char *status = NULL;
#ifdef ABPY_CUSTOMIZATION
	bool enable = false;
#endif

	retObj = json_object_new_object();
	IID_INIT(objIid);
	ret = zcfgFeObjJsonGet(RDM_OID_ZY_SFP_MANAGEMENT, &objIid, &sfpMgmtObj);
	if(ret == ZCFG_SUCCESS){
		timer = json_object_get_int(json_object_object_get(sfpMgmtObj, "Timer"));
		status = json_object_get_string(json_object_object_get(sfpMgmtObj, "Status"));
		json_object_object_add(retObj, "Timer", json_object_new_int(timer));
		json_object_object_add(retObj, "Status", json_object_new_string(status));
#ifdef ABPY_CUSTOMIZATION
		enable = json_object_get_boolean(json_object_object_get(sfpMgmtObj, "Enable"));
		json_object_object_add(retObj, "Enable", json_object_new_int(enable));
#endif
		zcfgFeJsonObjFree(sfpMgmtObj);
	}
	json_object_array_add(Jarray, retObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeDalSfpMgmt(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_SfpMgmt_Edit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_SfpMgmt_Get(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}
	return ret;
}

