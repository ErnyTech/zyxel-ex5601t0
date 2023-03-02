
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t PPTP_param[] =
{
	{"pptpEnable", 		dalType_boolean,	0,	0,		NULL},
	{"pptpName", 		dalType_string, 	0, 	64,		NULL},
	{"pptpServer", 		dalType_string, 	0, 	64,		NULL},
	{"pptpUsername", 	dalType_string, 	0, 	256,	NULL},
	{"pptpPassword", 	dalType_string, 	0, 	64,		NULL},
	{"pptpLanInf", 		dalType_string, 	0, 	0,		NULL},
	{"pptpSecurity", 	dalType_string,		0, 	0,		NULL},
	{"pptpStatus", 		dalType_v4Addr,		0,	0,		NULL},
	{"pptpLocalIP", 	dalType_boolean,	0,	0,		NULL},
	{"pptpRemoteIP", 	dalType_boolean,	0,	0,		NULL},
	{"pptpOriAlgStatus",	dalType_boolean,	0,  0,      NULL},
	{NULL, 				0, 					0, 	0, 		NULL}
};


zcfgRet_t zcfgFeDalPptpEdit(struct json_object *Jobj, char *replyMsg)
{
	/*!
	*  Edit PPTP config.
	*
	*  @param[in]     Jobj       The PPTP object
	*  @param[in]	  replyMsg	 
	*
	*  @return		ZCFG_SUCCESS  		- Config PPTP success
	*				ZCFG_INTERNAL_ERROR	- Config PPTP fail
	*
	*  @note     Please ref. RDM_OID_PPTP_CFG
	*/
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *paraName;
	objIndex_t objIid;
	struct json_object *pptpJobj = NULL;
	int paramModified = 0, n = 0;

	paraName = (const char *)PPTP_param[n++].paraName;
	
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_PPTP_CFG, &objIid, &pptpJobj)) != ZCFG_SUCCESS) {
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}
	
	while(paraName){
		struct json_object *paramValue = json_object_object_get(Jobj, paraName);
		if(!paramValue) { //get next
			paraName = (const char *)PPTP_param[n++].paraName;
			continue;
		}		

		json_object_object_add(pptpJobj, paraName, JSON_OBJ_COPY(paramValue));
		paramModified = 1;			

		paraName = (const char *)PPTP_param[n++].paraName;
	}

	if(paramModified) {
		if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_PPTP_CFG, &objIid, pptpJobj, NULL)) != ZCFG_SUCCESS)
			printf("%s: Write object fail\n", __FUNCTION__);
	}
	
	json_object_put(pptpJobj);
	return ret;
}
zcfgRet_t zcfgFeDalPptpGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *pptpJobj = NULL;

	objIndex_t pptpIid = {0};
	paramJobj = json_object_new_object();
	if(zcfgFeObjJsonGet(RDM_OID_PPTP_CFG, &pptpIid, &pptpJobj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "pptpEnable", JSON_OBJ_COPY(json_object_object_get(pptpJobj, "pptpEnable")));
		json_object_object_add(paramJobj, "pptpName", JSON_OBJ_COPY(json_object_object_get(pptpJobj, "pptpName")));
		json_object_object_add(paramJobj, "pptpServer", JSON_OBJ_COPY(json_object_object_get(pptpJobj, "pptpServer")));
		json_object_object_add(paramJobj, "pptpUsername", JSON_OBJ_COPY(json_object_object_get(pptpJobj, "pptpUsername")));
		json_object_object_add(paramJobj, "pptpPassword", JSON_OBJ_COPY(json_object_object_get(pptpJobj, "pptpPassword")));
		json_object_object_add(paramJobj, "pptpLanInf", JSON_OBJ_COPY(json_object_object_get(pptpJobj, "pptpLanInf")));
		json_object_object_add(paramJobj, "pptpSecurity", JSON_OBJ_COPY(json_object_object_get(pptpJobj, "pptpSecurity")));
		json_object_object_add(paramJobj, "pptpStatus", JSON_OBJ_COPY(json_object_object_get(pptpJobj, "pptpStatus")));
		json_object_object_add(paramJobj, "pptpLocalIP", JSON_OBJ_COPY(json_object_object_get(pptpJobj, "pptpLocalIP")));
		json_object_object_add(paramJobj, "pptpRemoteIP", JSON_OBJ_COPY(json_object_object_get(pptpJobj, "pptpRemoteIP")));
		zcfgFeJsonObjFree(pptpJobj);
	}
	
	json_object_array_add(Jarray, paramJobj);

	
	return ret;
}

zcfgRet_t zcfgFeDalPptp(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalPptpEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalPptpGet(Jobj, Jarray, NULL);
	}
		
	return ret;
}
