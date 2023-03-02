
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t USB_MEDIASERVER_param[] =
{
	//Media server
	{"Enable",				dalType_boolean,	0,	0,	NULL},
	{"MediaRelativePath",	dalType_mediaserverPath,		0,	0,	NULL},
	{"Interface",			dalType_Lan,	 	0,	0,	NULL},	
	{NULL,		0,	0,	0,	NULL}
};

void zcfgFeDalShowUsbMediaserver(struct json_object *Jarray){
	int i, len;
	struct json_object *obj;
	const char *enable = NULL;
	const char *path = NULL;
	const char *intf = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	dbg_printf("%s : Jarray = %s",__FUNCTION__,json_object_to_json_string(Jarray));
	printf("%-15s %-15s %-20s\n","Enable/Disable","Interface","Media Server Path");

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray,i);

		if(json_object_get_boolean(json_object_object_get(obj, "Enable")))
			enable = "Enabled";
		else
			enable = "Disabled";

		path = json_object_get_string(json_object_object_get(obj, "MediaRelativePath"));
		intf = json_object_get_string(json_object_object_get(obj, "Interface"));
		printf("%-15s %-15s %s\n",enable,intf,path);
	}
}

zcfgRet_t zcfgFeDalUsbMediaserverGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *MediaserverJobj = NULL;
	struct json_object *dlnaObj = NULL;
	bool enable = false;
	const char *path = NULL;
	const char *intf = NULL;

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DLNA, &objIid, &dlnaObj)) != ZCFG_SUCCESS)
		return ret;

	enable = json_object_get_boolean(json_object_object_get(dlnaObj, "X_ZYXEL_Enable"));
	path = json_object_get_string(json_object_object_get(dlnaObj, "X_ZYXEL_MediaRelativePath"));
	intf = json_object_get_string(json_object_object_get(dlnaObj, "X_ZYXEL_Interface"));
	MediaserverJobj = json_object_new_object();
	json_object_object_add(MediaserverJobj, "Enable", json_object_new_boolean(enable));
	json_object_object_add(MediaserverJobj, "MediaRelativePath", json_object_new_string(path));
	json_object_object_add(MediaserverJobj, "Interface", json_object_new_string(intf));
	json_object_array_add(Jarray, MediaserverJobj);
	dbg_printf("%s : Jarray = %s\n",__FUNCTION__,json_object_to_json_string(Jarray));
	
	zcfgFeJsonObjFree(dlnaObj);
	return ret;
}

zcfgRet_t zcfgFeDalUsbMediaserverEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *MediaserverJobj = NULL;
	bool paramModified = false;
	bool enable, oldEnable;
	const char *path = NULL;
	const char *oldPath = NULL;
	const char *intf = NULL;
	const char *oldIntf = NULL;
	
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DLNA, &objIid, &MediaserverJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(json_object_object_get(Jobj, "Enable")){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		oldEnable = json_object_get_boolean(json_object_object_get(MediaserverJobj, "X_ZYXEL_Enable"));
		if(enable != oldEnable){
			json_object_object_add(MediaserverJobj, "X_ZYXEL_Enable", json_object_new_boolean(enable));
			paramModified = true;
		}
	}

	if(json_object_object_get(Jobj, "MediaRelativePath")){
		path = json_object_get_string(json_object_object_get(Jobj, "MediaRelativePath"));
		oldPath = json_object_get_string(json_object_object_get(MediaserverJobj, "X_ZYXEL_MediaRelativePath"));
		if(strcmp(path,oldPath)){
			json_object_object_add(MediaserverJobj, "X_ZYXEL_MediaRelativePath", json_object_new_string(path));
			paramModified = true;
		}
	}

	if(json_object_object_get(Jobj, "Interface")){
		intf = json_object_get_string(json_object_object_get(Jobj, "Interface"));
		oldIntf = json_object_get_string(json_object_object_get(MediaserverJobj, "X_ZYXEL_Interface"));
		if(strcmp(intf,oldIntf)){
			json_object_object_add(MediaserverJobj, "X_ZYXEL_Interface", json_object_new_string(intf));
			paramModified = true;
		}
	}
	
	if(paramModified){
		if((ret = zcfgFeObjJsonSet(RDM_OID_DLNA, &objIid, MediaserverJobj, NULL)) != ZCFG_SUCCESS){
			printf("Setting object failed..\n");
		}
	}
	json_object_put(MediaserverJobj);
	return ret;
}

zcfgRet_t zcfgFeDalUsbMediaserver(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalUsbMediaserverEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalUsbMediaserverGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}
	
	return ret;
}

