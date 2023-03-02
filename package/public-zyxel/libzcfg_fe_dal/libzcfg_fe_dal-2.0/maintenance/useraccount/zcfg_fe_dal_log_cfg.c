#include <json/json.h>
#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_dal.h"
#include "zos_api.h"

dal_param_t LOG_CFG_param[] =  //RDM_OID_ZY_LOG_CFG
{
	{"DisableHighUser", 	dalType_boolean, 0, 	0,		NULL,	NULL,	dal_Edit}, 
	{NULL,					0,				 0, 	0,		NULL,	NULL,	0}
};

zcfgRet_t zcfgFeDal_Log_Cfg_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t logCfgIid = {0};
	struct json_object *logCfgObj = NULL, *paramJobj = NULL;

	IID_INIT(logCfgIid);
	if(zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG, &logCfgIid, &logCfgObj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "AutoDisableHighUser", JSON_OBJ_COPY(json_object_object_get(logCfgObj, "AutoDisableHighUser")));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(logCfgObj);
	}
	return ret;
}

//PUT function
zcfgRet_t zcfgFeDal_Log_Cfg_Edit(struct json_object *Jobj, char *replyMsg){ 
	zcfgRet_t ret = ZCFG_SUCCESS;

	objIndex_t logCfgIid = {0}, accountIid = {0};
	struct json_object *logCfgObj = NULL, *accountObj = NULL;
	bool autoDisable = false, disableHighUser = false;
	char username[32];

	// get from "cfg login_cfg edit --DisableHighUser"
	disableHighUser = json_object_get_boolean(json_object_object_get(Jobj, "DisableHighUser"));
	
	// get from config, flag "AutoDisableHighUser" to enable auto disable high user
	IID_INIT(logCfgIid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_LOG_CFG, &logCfgIid, &logCfgObj) == ZCFG_SUCCESS){
		autoDisable = json_object_get_boolean(json_object_object_get(logCfgObj, "AutoDisableHighUser"));
		zcfgFeJsonObjFree(logCfgObj);
	}

	if(autoDisable && disableHighUser){
		IID_INIT(accountIid);
		while(zcfgFeObjJsonGetNext(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, &accountObj) == ZCFG_SUCCESS){
			ZOS_STRNCPY(username, json_object_get_string(json_object_object_get(accountObj, "Username")), sizeof(username));
			// should not disable "root"
			if(accountIid.idx[0] == 1 && strcmp("root", username)){
				json_object_object_add(accountObj, "Enabled", json_object_new_boolean(false));
				printf("Disable User: %s\n", username);
				if(zcfgFeObjJsonBlockedSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, accountObj, NULL) != ZCFG_SUCCESS){
					printf("%s: zcfgFeObjJsonBlockedSet fail\n", __FUNCTION__);
					zcfgFeJsonObjFree(accountObj);
					return false;
				}
			}
			zcfgFeJsonObjFree(accountObj);
		}
	}
	else{ // if "AutoDisableHighUser" is disabled, high level user MUST not be disabled
		if(replyMsg) 
			strcpy(replyMsg, "Request reject");

		return ZCFG_REQUEST_REJECT;		
	}
	return ret;
}


zcfgRet_t zcfgFeDalLogCfg(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;	

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "GET")){
		ret = zcfgFeDal_Log_Cfg_Get(Jobj, Jarray, replyMsg);
	}else if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_Log_Cfg_Edit(Jobj, replyMsg);
	}else
		printf("Unknown method:%s\n", method);

	return ret;
}

