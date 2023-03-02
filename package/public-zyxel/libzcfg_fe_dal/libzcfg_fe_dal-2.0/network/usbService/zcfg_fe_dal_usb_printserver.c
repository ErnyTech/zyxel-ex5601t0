#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t USB_PRINTSERVER_param[] =
{
	//Print server
	{"Enable",				dalType_boolean,	0,	0,	NULL},
	{"IppMake",				dalType_string,		0,	0,	NULL},
	{"IppDevice", 			dalType_string, 	0,	0,	NULL},
	{"IppName",				dalType_string,		0,	0,	NULL},
	{NULL,		0,	0,	0,	NULL}
};

zcfgRet_t zcfgFeDalUsbPrintserverGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *printserverJobj = NULL;
	struct json_object *paramJobj = NULL;
	struct json_object *allObj = NULL;

	allObj = json_object_new_object();
	paramJobj = json_object_new_object();

	IID_INIT(objIid);
	if(zcfgFeObjJsonGet(RDM_OID_ZY_IPP_CFG, &objIid, &printserverJobj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(printserverJobj, "Enable")));
		json_object_object_add(paramJobj, "IppMake", JSON_OBJ_COPY(json_object_object_get(printserverJobj, "IppMake")));
		json_object_object_add(paramJobj, "IppDevice", JSON_OBJ_COPY(json_object_object_get(printserverJobj, "IppMake")));
		json_object_object_add(paramJobj, "IppName", JSON_OBJ_COPY(json_object_object_get(printserverJobj, "IppMake")));
		zcfgFeJsonObjFree(printserverJobj);
	}
	json_object_object_add(allObj, "PrintServer", paramJobj);
	json_object_array_add(Jarray, allObj);
	dbg_printf("%s : Jarray = %s\n",__FUNCTION__,json_object_to_json_string(Jarray));

	return ret;
}

zcfgRet_t zcfgFeDalUsbPrintserverEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *printserverJobj = NULL;
	bool enable;
	const char *ippMake = NULL;
	const char *ippDevice = NULL;
	const char *ippName = NULL;

	IID_INIT(objIid);
	if(zcfgFeObjJsonGet(RDM_OID_ZY_IPP_CFG, &objIid, &printserverJobj) == ZCFG_SUCCESS){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		ippMake = json_object_get_string(json_object_object_get(Jobj, "IppMake"));
		ippDevice = json_object_get_string(json_object_object_get(Jobj, "IppDevice"));
		ippName = json_object_get_string(json_object_object_get(Jobj, "IppName"));

		if(json_object_object_get(Jobj, "Enable"))
			json_object_object_add(printserverJobj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "IppMake"))
			json_object_object_add(printserverJobj, "IppMake", json_object_new_string(ippMake));
		if(json_object_object_get(Jobj, "IppDevice"))
			json_object_object_add(printserverJobj, "IppDevice", json_object_new_string(ippDevice));
		if(json_object_object_get(Jobj, "IppName"))
			json_object_object_add(printserverJobj, "IppName", json_object_new_string(ippName));
	}

	ret = zcfgFeObjJsonSet(RDM_OID_ZY_IPP_CFG, &objIid, printserverJobj, NULL);
	zcfgFeJsonObjFree(printserverJobj);
	return ret;
}

zcfgRet_t zcfgFeDalUsbPrintserver(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalUsbPrintserverEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalUsbPrintserverGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}
	
	return ret;
}
