#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"


//#define DEVICE_DEBUG 1

#ifdef DEVICE_DEBUG
#define DEVICE_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define DEVICE_PRINT(format, ...)
#endif


dal_param_t ChangeIconName_param[]={
	{"Index",						dalType_int,		0,	0,	NULL, NULL, dal_Edit|dal_Delete},
	{"HostName",					dalType_string,		0,	64,	NULL,	NULL,	0},
	{"MacAddress",					dalType_string,		0,	64,	NULL,	NULL,	dal_Add},
	{"DeviceIcon",					dalType_string,		0,	32,	NULL,	NULL,	0},
	{"Internet_Blocking_Enable",	dalType_boolean,	0,	0,	NULL},
#ifdef ZYXEL_HOME_CYBER_SECURITY
	{"IPAddress",					dalType_string,		0,	48,	NULL,	NULL,	0},
	{"IPAddress6",					dalType_string,		0,	48,	NULL,	NULL,	0},
	{"BrowsingProtection",			dalType_boolean,	0,	0,	NULL},
	{"TrackingProtection",			dalType_boolean,	0,	0,	NULL},
	{"IOTProtection",					dalType_boolean,	0,	0,	NULL},
	{"Profile",						dalType_string,		0,	64,	NULL},
	{"X_ZYXEL_FirstSeen",			dalType_int,		0,	0,	NULL},
	{"X_ZYXEL_LastSeen",			dalType_int,		0,	0,	NULL},
	{"SourceVendorClassID",			dalType_string,		0,	0,	NULL},
#endif
	{"Update",						dalType_boolean,	0,	0,	NULL},
	{NULL,		0,	0,	0,	NULL}
};

static int
getIidByMac( char *mac, objIndex_t *iid)
{
	objIndex_t objIid;
	struct json_object *iconName = NULL;
	struct json_object *obj = NULL;

	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_CHANGE_ICON_NAME, &objIid, &iconName) == ZCFG_SUCCESS){
		if ( ( obj = json_object_object_get(iconName,"MacAddress")) != NULL ) {
			if ( strcmp(mac,json_object_get_string(obj)) == 0 ) {
				memcpy(iid,&objIid,sizeof(objIndex_t));
				zcfgFeJsonObjFree(iconName);
				return 0;
			}
		}
		zcfgFeJsonObjFree(iconName);
	}
	return -1;
} /* getIidByMac */


int zcfgFeDalShowChangeIconName(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return -1;
	}
	printf("%-5s %-32s %-17s %-5s %-3s %-3s %-4s %-32s\n", "Index", "Host Name","Mac Address", "InetB", "BrP","TrP","IoTP","Profile");
	printf("-----------------------------------------------------------------------------------------\n");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-5s %-32s %-17s %-5s %-3s %-3s %-4s %-32s\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "HostName")),
			json_object_get_string(json_object_object_get(obj, "MacAddress")),
			json_object_get_boolean(json_object_object_get(obj, "Internet_Blocking_Enable"))?"Y":"N",
#ifdef ZYXEL_HOME_CYBER_SECURITY
			json_object_get_boolean(json_object_object_get(obj, "BrowsingProtection"))?"Y":"N",
			json_object_get_boolean(json_object_object_get(obj, "TrackingProtection"))?"Y":"N",
			json_object_get_boolean(json_object_object_get(obj, "IOTProtection"))?"Y":"N",
			json_object_get_string(json_object_object_get(obj, "Profile")));
#else
			"N",
			"N",
			"N",
			"");
#endif

	}
	return 0;
}

zcfgRet_t zcfgFeDalChangeIconNameGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *iconName = NULL;
	struct json_object *paramJobj = NULL;
	struct json_object *obj = NULL;
	int index = 0, inputIndex = -1, maxIndex;
	int update = 0;

	DEVICE_PRINT("%s: Jobj=\n%s\n",__FUNCTION__,json_object_get_string(Jobj));

	if ( ( obj = json_object_object_get(Jobj, "Update") ) != NULL ) {
		update = json_object_get_boolean(obj);
	}


	if ( ( obj = json_object_object_get(Jobj, "Index") ) != NULL ) {
		inputIndex = json_object_get_int(obj);
		maxIndex = zcfgFeObjMaxLenGet(RDM_OID_CHANGE_ICON_NAME);
		if ( ( inputIndex > maxIndex ) || ( inputIndex <= 0 ) ) {
			printf("%s: Index out of range(%d,%d)\n",__FUNCTION__,inputIndex,maxIndex);
			return ZCFG_NOT_FOUND;
		}
	}
	else if ( (obj = json_object_object_get(Jobj, "MacAddress")) != NULL) {
		if ( getIidByMac( (char *)json_object_get_string(obj), &objIid) != 0 ) {
			printf("%s: Can't find device (%s)\n",__FUNCTION__,json_object_get_string(obj));
			ret = ZCFG_NOT_FOUND;
			goto fail;
		}
		inputIndex = objIid.idx[0];
	}

	IID_INIT(objIid);
	while(1) {
		if ( update ) {
			if ( zcfgFeObjJsonGetNext(RDM_OID_CHANGE_ICON_NAME, &objIid, &iconName) != ZCFG_SUCCESS) {
				break;
			}
		}
		else {
			if ( zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_CHANGE_ICON_NAME, &objIid, &iconName) != ZCFG_SUCCESS) {
				break;
			}
		}
		DEVICE_PRINT("%s: level=%d,[%d,%d,%d]\n",__FUNCTION__,objIid.level, objIid.idx[0],objIid.idx[1],objIid.idx[2]);
		index = objIid.idx[0];
		if( inputIndex != -1 ) {
			if(index == inputIndex) {
				paramJobj = JSON_OBJ_COPY(iconName);
				json_object_object_add(paramJobj,"Index",json_object_new_int(index));
				json_object_array_add(Jarray, paramJobj);
				zcfgFeJsonObjFree(iconName);
				break;
			}
		}
		else {
			paramJobj = JSON_OBJ_COPY(iconName);
			json_object_object_add(paramJobj,"Index",json_object_new_int(index));
			json_object_array_add(Jarray, paramJobj);
		}
		zcfgFeJsonObjFree(iconName);
	}
fail:
	DEVICE_PRINT("%s: Jarray=\n%s\n",__FUNCTION__,json_object_get_string(Jarray));
	return ret;
}

zcfgRet_t zcfgFeDalChangeIconNameEdit(struct json_object *Jobj, char *replyMsg){
		zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid={0};
	int inputIndex, maxIndex;
	struct json_object *obj;
	struct json_object *iconNameObj = NULL;

	DEVICE_PRINT("%s: Jobj=\n%s\n",__FUNCTION__,json_object_get_string(Jobj));
	IID_INIT(objIid);
	if ( (obj = json_object_object_get(Jobj, "Index")) != NULL) {
		inputIndex = json_object_get_int (obj);
		maxIndex = zcfgFeObjMaxLenGet(RDM_OID_CHANGE_ICON_NAME);
		if ( ( inputIndex > maxIndex ) || ( inputIndex <= 0 ) ) {
			printf("%s: Index out of range(%d,%d)\n",__FUNCTION__,inputIndex,maxIndex);
			ret = ZCFG_REQUEST_REJECT;
			goto fail;
		}
		objIid.idx[0] = inputIndex;
		objIid.level = 1;
	}
	else if ( (obj = json_object_object_get(Jobj, "MacAddress")) != NULL) {
		if ( getIidByMac( (char *)json_object_get_string(obj), &objIid) != 0 ) {
			printf("%s: Can't find device (%s)\n",__FUNCTION__,json_object_get_string(obj));
			ret = ZCFG_REQUEST_REJECT;
			goto fail;
		}
	}

	if( (ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CHANGE_ICON_NAME, &objIid, &iconNameObj) ) == ZCFG_SUCCESS) {
		replaceParam(iconNameObj, "HostName", Jobj, "HostName");
		replaceParam(iconNameObj, "MacAddress", Jobj, "MacAddress");
		replaceParam(iconNameObj, "DeviceIcon", Jobj, "DeviceIcon");
		replaceParam(iconNameObj, "Internet_Blocking_Enable", Jobj, "Internet_Blocking_Enable");
#ifdef ZYXEL_HOME_CYBER_SECURITY
		replaceParam(iconNameObj, "IPAddress", Jobj, "IPAddress");
		replaceParam(iconNameObj, "IPAddress6", Jobj, "IPAddress6");
		replaceParam(iconNameObj, "BrowsingProtection", Jobj, "BrowsingProtection");
		replaceParam(iconNameObj, "TrackingProtection", Jobj, "TrackingProtection");
		replaceParam(iconNameObj, "IOTProtection", Jobj, "IOTProtection");
		replaceParam(iconNameObj, "Profile", Jobj, "Profile");
#endif
		ret = zcfgFeObjJsonSet(RDM_OID_CHANGE_ICON_NAME, &objIid, iconNameObj, NULL);
		zcfgFeJsonObjFree(iconNameObj);
	}
	else {
		ret = ZCFG_REQUEST_REJECT;
	}
fail:
	return ret;
}

zcfgRet_t zcfgFeDalChangeIconNameAdd(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid = {0};
	struct json_object *iconNameObj = NULL;

	DEVICE_PRINT("%s: Jobj=\n%s\n",__FUNCTION__,json_object_get_string(Jobj));
	IID_INIT(objIid);
	if ( ( ret = zcfgFeObjJsonBlockedAdd(RDM_OID_CHANGE_ICON_NAME, &objIid, NULL) ) == ZCFG_SUCCESS ) {
		if( (ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CHANGE_ICON_NAME, &objIid, &iconNameObj) ) == ZCFG_SUCCESS) {
			replaceParam(iconNameObj, "HostName", Jobj, "HostName");
			replaceParam(iconNameObj, "MacAddress", Jobj, "MacAddress");
			replaceParam(iconNameObj, "DeviceIcon", Jobj, "DeviceIcon");
			replaceParam(iconNameObj, "Internet_Blocking_Enable", Jobj, "Internet_Blocking_Enable");
#ifdef ZYXEL_HOME_CYBER_SECURITY
			replaceParam(iconNameObj, "IPAddress", Jobj, "IPAddress");
			replaceParam(iconNameObj, "IPAddress6", Jobj, "IPAddress6");
			replaceParam(iconNameObj, "BrowsingProtection", Jobj, "BrowsingProtection");
			replaceParam(iconNameObj, "TrackingProtection", Jobj, "TrackingProtection");
			replaceParam(iconNameObj, "IOTProtection", Jobj, "IOTProtection");
			replaceParam(iconNameObj, "Profile", Jobj, "Profile");
#endif
			ret = zcfgFeObjJsonSet(RDM_OID_CHANGE_ICON_NAME, &objIid, iconNameObj, NULL);
			zcfgFeJsonObjFree(iconNameObj);
		}
	}
	else {
		if ( replyMsg ) {
			strcat(replyMsg," Add fail");
		}
	}
	return ret;
} /* zcfgFeDalChangeIconNameAdd */

zcfgRet_t zcfgFeDalChangeIconNameDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid={0};
	int inputIndex, maxIndex;
	struct json_object *obj;

	DEVICE_PRINT("%s: Jobj=\n%s\n",__FUNCTION__,json_object_get_string(Jobj));
	IID_INIT(objIid);
	if ( (obj = json_object_object_get(Jobj, "Index")) != NULL) {
		inputIndex = json_object_get_int (obj);
		maxIndex = zcfgFeObjMaxLenGet(RDM_OID_CHANGE_ICON_NAME);
		if ( ( inputIndex > maxIndex ) || ( inputIndex <= 0 ) ) {
			printf("%s: Index out of range(%d,%d)\n",__FUNCTION__,inputIndex,maxIndex);
			ret = ZCFG_REQUEST_REJECT;
			goto fail;
		}
		objIid.idx[0] = inputIndex;
		objIid.level = 1;
	}
	else if ( (obj = json_object_object_get(Jobj, "MacAddress")) != NULL) {
		if ( getIidByMac( (char *)json_object_get_string(obj), &objIid) != 0 ) {
			printf("%s: Can't find device (%s)\n",__FUNCTION__,json_object_get_string(obj));
			ret = ZCFG_REQUEST_REJECT;
			goto fail;
		}
	}

	DEVICE_PRINT("%s: level=%d,[%d,%d,%d]\n",__FUNCTION__,objIid.level, objIid.idx[0],objIid.idx[1],objIid.idx[2]);
	ret = zcfgFeObjJsonDel(RDM_OID_CHANGE_ICON_NAME, &objIid, NULL);
	if(ret == ZCFG_DELETE_REJECT){
//		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.URLBlackFilter.delete_pcp_sch_err_msg"));
		printf("%s %s %d \n",json_object_to_json_string(Jobj),__func__,__LINE__);
		return ret;
	}
fail:
	return ret;
}

zcfgRet_t zcfgFeDalChangeIconName(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalChangeIconNameEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalChangeIconNameAdd(Jobj, Jarray, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalChangeIconNameDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalChangeIconNameGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}
	return ret;
}

