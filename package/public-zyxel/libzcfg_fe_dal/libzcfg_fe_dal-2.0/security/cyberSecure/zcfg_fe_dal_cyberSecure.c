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

dal_param_t CYBER_SECURE_param[]={
	{"CyberSecureEnable",	dalType_boolean,	0,	0,	NULL, NULL, dal_Add_ignore},
	{"browProtectEnable",	dalType_boolean,	0,	0,	NULL, NULL, dal_Add_ignore},
	{"trackProtectEnable",	dalType_boolean,	0,	0,	NULL, NULL, dal_Add_ignore},
	{"IoTEnable",	dalType_boolean,	0,	0,	NULL, NULL, dal_Add_ignore},	
	{NULL,		0,	0,	0,	NULL}
};

struct json_object *CyberSecureObj = NULL;
objIndex_t CyberSecureIid = {0};

bool CyberSecureEnable;
bool browProtectEnable;
bool trackProtectEnable;
bool IoTEnable;

zcfgRet_t freeAllCyberSecureObjects(){
	zcfgRet_t ret = ZCFG_SUCCESS;
			
	if(CyberSecureObj) json_object_put(CyberSecureObj); 			
	return ret;
}

void getBasicCyberSecureInfo(struct json_object *Jobj){
	
	CyberSecureEnable = json_object_get_boolean(json_object_object_get(Jobj, "CyberSecureEnable"));	
	browProtectEnable = json_object_get_boolean(json_object_object_get(Jobj, "browProtectEnable"));	
	trackProtectEnable = json_object_get_boolean(json_object_object_get(Jobj, "trackProtectEnable"));	
	IoTEnable = json_object_get_boolean(json_object_object_get(Jobj, "IoTEnable"));	
	return;
}

void initGlobalCyberSecureObjects(){		
	CyberSecureObj = NULL;		
	IID_INIT(CyberSecureIid);
		
}

zcfgRet_t editcyberSecureObject(struct json_object *Jobj){
	
	zcfgRet_t ret = ZCFG_SUCCESS;

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_HOME_CYBER_SECURITY, &CyberSecureIid, &CyberSecureObj)) != ZCFG_SUCCESS) {
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.get_cyberSecure_err"));		
		return ret;
	}
	if (CyberSecureEnable){
		json_object_object_add(CyberSecureObj, "Enable", json_object_new_boolean(CyberSecureEnable));
		json_object_object_add(CyberSecureObj, "BrowsingProtection", json_object_new_boolean(browProtectEnable));
		json_object_object_add(CyberSecureObj, "TrackingProtection", json_object_new_boolean(trackProtectEnable));
		json_object_object_add(CyberSecureObj, "IoTProtection", json_object_new_boolean(IoTEnable));		
		}
	else{
		json_object_object_add(CyberSecureObj, "Enable", json_object_new_boolean(false));
		//json_object_object_add(CyberSecureObj, "BrowsingProtection", json_object_new_boolean(false));
		//json_object_object_add(CyberSecureObj, "TrackingProtection", json_object_new_boolean(false));
		//json_object_object_add(CyberSecureObj, "IoTProtection", json_object_new_boolean(false));
		json_object_object_add(CyberSecureObj, "BrowsingProtection", json_object_new_boolean(browProtectEnable));
		json_object_object_add(CyberSecureObj, "TrackingProtection", json_object_new_boolean(trackProtectEnable));
		json_object_object_add(CyberSecureObj, "IoTProtection", json_object_new_boolean(IoTEnable));		
		}

	if((ret = zcfgFeObjJsonSet(RDM_OID_HOME_CYBER_SECURITY, &CyberSecureIid, CyberSecureObj, NULL)) != ZCFG_SUCCESS){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.set_cyberSecure_err"));		
		return ret;
	}

	return ret;
}

void zcfgFeDalShowCyberSecure(struct json_object *Jarray){
	struct json_object *Jobj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	if((Jobj = json_object_array_get_idx(Jarray, 0)) == NULL)
		return;

	if(Jget(Jobj, "CyberSecureEnable") != NULL){
		printf("Cyber_security Protection %s\n", Jgetb(Jobj, "CyberSecureEnable")?"Enabled":"Disabled");
		printf("browsing Protection       %s\n", Jgetb(Jobj, "browProtectEnable")?"Enabled":"Disabled");
		printf("tracking Protection       %s\n", Jgetb(Jobj, "trackProtectEnable")?"Enabled":"Disabled");
		printf("IOT Protection            %s\n", Jgetb(Jobj, "IoTEnable")?"Enabled":"Disabled");
		
	}
}

zcfgRet_t zcfgFeDalCyberSecureGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	
	objIndex_t objIid;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;
	
	IID_INIT(objIid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_HOME_CYBER_SECURITY, &objIid, &obj) == ZCFG_SUCCESS){
		pramJobj = json_object_new_object();
		replaceParam(pramJobj, "CyberSecureEnable", obj, "Enable");
		replaceParam(pramJobj, "browProtectEnable", obj, "BrowsingProtection");
		replaceParam(pramJobj, "trackProtectEnable", obj, "TrackingProtection");	
		replaceParam(pramJobj, "IoTEnable", obj, "IoTProtection");			
		json_object_array_add(Jarray, pramJobj);
		zcfgFeJsonObjFree(obj);
	}
	return ret;
}

zcfgRet_t zcfgFeDalCyberSecureEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	initGlobalCyberSecureObjects();
	getBasicCyberSecureInfo(Jobj);
	ret = editcyberSecureObject(Jobj);
	freeAllCyberSecureObjects();
	return ret;
}

zcfgRet_t zcfgFeDalCyberSecure(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalCyberSecureEdit(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalCyberSecureGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);
	return ret;
}
