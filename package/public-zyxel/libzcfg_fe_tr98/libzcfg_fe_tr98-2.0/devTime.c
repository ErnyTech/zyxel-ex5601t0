#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcmd_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_msg.h"
#include "zcfg_debug.h"
#include "zcfg_fe_tr98.h"
#include "devTime_parameter.h"

extern tr98Object_t tr98Obj[];

/*  TR98 Object Name : InternetGatewayDevice.Time

    Related object in TR181:
    Device.Time
*/
zcfgRet_t timeObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t timeIid;
	struct json_object *timeObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	IID_INIT(timeIid);
	if((ret = feObjJsonGet(RDM_OID_TIME, &timeIid, &timeObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 time object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*special case*/
		if(strcmp(paramList->name, "LocalTimeZoneName") == 0){
			paramValue = json_object_object_get(timeObj, "LocalTimeZone");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;		
		}
#if 1
		else if(strcmp(paramList->name, "LocalTimeZone") == 0){
			paramList++;
			continue;		
		}
#else /*OBSOLETED*/
		else if(strcmp(paramList->name, "LocalTimeZone") == 0){
			paramValue = json_object_object_get(timeObj, "X_ZYXEL_TimeZone");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;		
		}
		else if(strcmp(paramList->name, "DaylightSavingsUsed") == 0){
			paramValue = json_object_object_get(timeObj, "X_ZYXEL_DaylightSavings");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;		
		}
#endif
		paramValue = json_object_object_get(timeObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(timeObj);

	return ZCFG_SUCCESS;	
}

zcfgRet_t timeObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char timeObjName[128] = {0};
	char mapObj[128] = {0};
	uint32_t  timeOid = 0;
	objIndex_t timeIid;
	tr98Parameter_t *paramList = NULL;
	struct json_object *timeObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(timeIid);
	snprintf(timeObjName, sizeof(timeObjName),"Device.%s", mapObj);
	timeOid = zcfgFeObjNameToObjId(timeObjName, &timeIid);
	if((ret = zcfgFeObjJsonGet(timeOid, &timeIid, &timeObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = timeObj;
		timeObj = NULL;
		timeObj = zcfgFeJsonMultiObjAppend(timeOid, &timeIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/

		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(timeObj, paramList->name);
			/* __ZyXEL__, Melissa, 20161228, [#32046] The parameter LocalTimeZoneName can't set success. */
			if(strcmp(paramList->name, "LocalTimeZoneName") == 0){
				tr181ParamValue = json_object_object_get(timeObj, "LocalTimeZone");
			}
			if(tr181ParamValue != NULL) {

				/*special case*/
				if(strcmp(paramList->name, "LocalTimeZoneName") == 0){
					json_object_object_add(timeObj, "LocalTimeZone", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;		
				}
#if 1	
				else if(strcmp(paramList->name, "LocalTimeZone") == 0){
					paramList++;
					continue;		
				}
#else /*OBSOLETED*/
				else if(strcmp(paramList->name, "LocalTimeZone") == 0){
					json_object_object_add(timeObj, "X_ZYXEL_TimeZone", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;		
				}
				else if(strcmp(paramList->name, "DaylightSavingsUsed") == 0){
					json_object_object_add(timeObj, "X_ZYXEL_DaylightSavings", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;		
				}
#endif
				json_object_object_add(timeObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		paramList++;	
	} /*Edn while*/
	
	/*Set Device.Time*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(timeOid, &timeIid, timeObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(timeObj);
			return ret;
		}
		json_object_put(timeObj);
	}

	return ZCFG_SUCCESS;	
}
