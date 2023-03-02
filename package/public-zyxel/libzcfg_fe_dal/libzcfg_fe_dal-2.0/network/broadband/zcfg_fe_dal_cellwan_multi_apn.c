#include <ctype.h>
#include <json/json.h>
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

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t CELLWAN_MAPN_param[]={
{"AP_Enable", dalType_boolean, 0, 0, NULL},
{"AP_ManualAPN", dalType_boolean, 0, 0, NULL},
{"AP_APN", dalType_string, 0, 0, NULL},
{"AP_Username", dalType_string, 0, 0, NULL},
{"AP_Password", dalType_string, 0, 0, NULL},
{"AP_Auth_Type", dalType_string, 0, 0, NULL, "None|PAP|CHAP|Auto", 0},
{"AP_PDP_Type", dalType_string, 0, 0,  NULL, "IPv4|IPv6|IPv4v6", 0},
{ NULL,	0,	0,	0,	NULL}

};

zcfgRet_t zcfgFeDalCellWanMApnEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *accessPointObj = NULL;
	objIndex_t accessPointIid = {0};
	int index = -1;
	bool enableAPN = false;
	bool manualAPN = false;
	const char *apn = NULL;
	const char *username = NULL;
	const char *password = NULL;
	const char *authType = NULL;
	const char *pdpType = NULL;
	zcfg_offset_t accessPointRdmOid = RDM_OID_CELL_ACCESS_POINT;

	IID_INIT(accessPointIid);
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(index, &accessPointIid, accessPointRdmOid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	enableAPN = json_object_get_boolean(json_object_object_get(Jobj, "AP_Enable"));
	manualAPN = json_object_get_boolean(json_object_object_get(Jobj, "AP_ManualAPN"));
	apn = json_object_get_string(json_object_object_get(Jobj, "AP_APN"));
	username = json_object_get_string(json_object_object_get(Jobj, "AP_Username"));
	password = json_object_get_string(json_object_object_get(Jobj, "AP_Password"));
	authType = json_object_get_string(json_object_object_get(Jobj, "AP_Auth_Type"));
	pdpType = json_object_get_string(json_object_object_get(Jobj, "AP_PDP_Type"));

	if((ret = zcfgFeObjJsonGetWithoutUpdate(accessPointRdmOid, &accessPointIid, &accessPointObj)) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "AP_Enable"))
			json_object_object_add(accessPointObj, "Enable", json_object_new_boolean(enableAPN));
		if(json_object_object_get(Jobj, "AP_ManualAPN"))
			json_object_object_add(accessPointObj, "X_ZYXEL_ManualAPN", json_object_new_boolean(manualAPN));
		if(json_object_object_get(Jobj, "AP_APN"))
			json_object_object_add(accessPointObj, "APN", json_object_new_string(apn));
		if(json_object_object_get(Jobj, "AP_Username"))
			json_object_object_add(accessPointObj, "Username", json_object_new_string(username));
		if(json_object_object_get(Jobj, "AP_Password"))
			json_object_object_add(accessPointObj, "Password", json_object_new_string(password));
		if(json_object_object_get(Jobj, "AP_Auth_Type"))
			json_object_object_add(accessPointObj, "X_ZYXEL_AuthenticationType", json_object_new_string(authType));
		if(json_object_object_get(Jobj, "AP_PDP_Type"))
			json_object_object_add(accessPointObj, "X_ZYXEL_PDP_Type", json_object_new_string(pdpType));
		if((ret = zcfgFeObjJsonSet(accessPointRdmOid, &accessPointIid, accessPointObj, NULL)) != ZCFG_SUCCESS){
			zcfgFeJsonObjFree(accessPointObj);
			printf("zcfgFeObjJsonSet RDM_OID_CELL_ACCESS_POINT fail.\n");
			return ret;
		}
	}

	zcfgFeJsonObjFree(accessPointObj);

	return ret;	
}	

zcfgRet_t zcfgFeDalCellWanMApnGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int index = 0;
	objIndex_t accessPointIid = {0};
	struct json_object *paramJobj = NULL, *accessPointObj = NULL;

	IID_INIT(accessPointIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_CELL_ACCESS_POINT, &accessPointIid, &accessPointObj) == ZCFG_SUCCESS) {
		paramJobj = json_object_new_object();

		json_object_object_add(paramJobj, "AP_Enable", JSON_OBJ_COPY(json_object_object_get(accessPointObj, "Enable")));
		json_object_object_add(paramJobj, "AP_APN", JSON_OBJ_COPY(json_object_object_get(accessPointObj, "APN")));
		json_object_object_add(paramJobj, "AP_Username", JSON_OBJ_COPY(json_object_object_get(accessPointObj, "Username")));
		json_object_object_add(paramJobj, "AP_Password", JSON_OBJ_COPY(json_object_object_get(accessPointObj, "Password")));
		json_object_object_add(paramJobj, "AP_Interface", JSON_OBJ_COPY(json_object_object_get(accessPointObj, "Interface")));
		json_object_object_add(paramJobj, "AP_ManualAPN", JSON_OBJ_COPY(json_object_object_get(accessPointObj, "X_ZYXEL_ManualAPN")));
		json_object_object_add(paramJobj, "AP_Auth_Type", JSON_OBJ_COPY(json_object_object_get(accessPointObj, "X_ZYXEL_AuthenticationType")));
		json_object_object_add(paramJobj, "AP_PDP_Type", JSON_OBJ_COPY(json_object_object_get(accessPointObj, "X_ZYXEL_PDP_Type")));

		if(json_object_object_get(Jobj, "Index")){
			index = Jgeti(Jobj, "Index");
			if(index == accessPointIid.idx[0]){
				json_object_array_add(Jarray, JSON_OBJ_COPY(paramJobj));

				zcfgFeJsonObjFree(paramJobj);
				zcfgFeJsonObjFree(accessPointObj);
				break;
			}
		}
		else{
			json_object_array_add(Jarray, JSON_OBJ_COPY(paramJobj));
		}

		zcfgFeJsonObjFree(paramJobj);
		zcfgFeJsonObjFree(accessPointObj);
	}

	return ret;
}


typedef struct display_handler_s {
	int			width;
	int			minValue;
	char		*key;
	char		*trueValue;
	char		*falseValue;
}display_handler_t;

char *isMeetCriteria(bool criteria, char *trueValue, char *falseValue){
	return (criteria) ? trueValue : falseValue;
}

int jsonValueConvertNDisplay(json_object *obj, int width, int minValue, char *key, char *trueValue, char *falseValue)
{
	int tmpInt;
	bool tmpBool;
	char *tmpString = NULL;
	json_object *value = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;

	if((value = json_object_object_get(obj, key)) == NULL)
		return ZCFG_INTERNAL_ERROR;

	switch(json_object_get_type(value))
	{
		case json_type_boolean:
			/* printf("json_object_get_boolean(value):%d\n", json_object_get_boolean(value)); */
			tmpBool = json_object_get_boolean(value);
			printf("%*s ", width, isMeetCriteria(tmpBool, trueValue, falseValue));
			break;
		case json_type_int:
			/* printf("json_object_get_int(value):%lld\n", json_object_get_int(value)); */
			/* printf("json_object_get_string(value):%s\n", json_object_get_string(value)); */
			tmpInt = json_object_get_int(value);
			tmpString = json_object_get_string(value);
			printf("%*s ", width, isMeetCriteria(tmpInt == minValue, "N/A", tmpString));
			break;
		case json_type_string:
			/* printf("json_object_get_string(value):%s\n", json_object_get_string(value)); */
			tmpString = json_object_get_string(value);
			printf("%*s ", width, isMeetCriteria(!strcmp(tmpString, ""), "N/A", tmpString));
			break;
		default:
			ret = ZCFG_INVALID_OBJECT;
			printf("[%d] F:%s, unconverted type: %d\n", __LINE__, __func__, json_object_get_type(value));
			break;
	}

	return ret;
}

void zcfgFeDalShowCellWanMApn(struct json_object *Jarray){
	zcfgRet_t retConvert;
	json_object *Jobj = NULL;
	int idx, len = 0, handlerIdx = 0;
	display_handler_t displayHandler[] = {
		{-10,	0,	"AP_Enable",	"Enabled",	"Disabled"},
		{-10,	0,	"AP_ManualAPN",	"Manual",	"Automatic"},
		{-20,	0,	"AP_APN",	NULL,	NULL},
		{-20,	0,	"AP_Username",	NULL,	NULL},
		{-20,	0,	"AP_Password",	NULL,	NULL},
		{-15,	0,	"AP_Auth_Type",	NULL,	NULL},
		{-15,	0,	"AP_PDP_Type",	NULL,	NULL},
		{0,		0,	NULL,		NULL,	NULL}
	};

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-5s %-10s %-10s %-20s %-20s %-20s %-15s %-15s\n", "#", "Enable", "Mode", "APN", "Username", "Password", "Auth Type", "PDP Type");
	len = json_object_array_length(Jarray);
	for(idx = 0; idx < len; idx++){
		Jobj = json_object_array_get_idx(Jarray, idx);
		printf("%-5d ", idx+1);
		for(handlerIdx = 0; displayHandler[handlerIdx].key != NULL; handlerIdx++){
			retConvert = jsonValueConvertNDisplay(Jobj,
					displayHandler[handlerIdx].width,
					displayHandler[handlerIdx].minValue,
					displayHandler[handlerIdx].key,
					displayHandler[handlerIdx].trueValue,
					displayHandler[handlerIdx].falseValue);
			if(retConvert != ZCFG_SUCCESS)
				printf("[%d] F:%s K:%s, jsonValueConvertNDisplay Fail.\n", __LINE__, __func__, displayHandler[handlerIdx].key);
		}
		printf("\n");
	}
}


zcfgRet_t zcfgFeDalCellWanMApn(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalCellWanMApnEdit(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalCellWanMApnGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

