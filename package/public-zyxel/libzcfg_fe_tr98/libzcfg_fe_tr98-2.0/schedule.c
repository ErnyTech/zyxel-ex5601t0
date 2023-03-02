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
#include "schedule_parameter.h"

extern tr98Object_t tr98Obj[];


/* InternetGatewayDevice.X_ZYXEL_Schedule */
zcfgRet_t scheduleObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char scheduleObjFormate[128] = "InternetGatewayDevice.X_ZYXEL_EXT.Schedule.%hhu";
	uint32_t  scheduleOid = 0;
	objIndex_t scheduleIid;
	struct json_object *scheduleObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	IID_INIT(scheduleIid);
	scheduleIid.level= 1;
	if(sscanf(tr98FullPathName, scheduleObjFormate, scheduleIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	scheduleOid = RDM_OID_SCHEDULE;
	
	if((ret = feObjJsonGet(scheduleOid, &scheduleIid, &scheduleObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 schedule object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(scheduleObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(scheduleObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t scheduleObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char scheduleObjFormate[128] = "InternetGatewayDevice.X_ZYXEL_EXT.Schedule.%hhu";
	uint32_t  scheduleOid = 0;
	objIndex_t scheduleIid;
	struct json_object *scheduleObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(scheduleIid);
	scheduleIid.level= 1;
	if(sscanf(tr98FullPathName, scheduleObjFormate, scheduleIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	scheduleOid = RDM_OID_SCHEDULE;

	if((ret = zcfgFeObjJsonGet(scheduleOid, &scheduleIid, &scheduleObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = scheduleObj;
		scheduleObj = NULL;
		scheduleObj = zcfgFeJsonMultiObjAppend(scheduleOid, &scheduleIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(scheduleObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(scheduleObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		paramList++;	
	} /*Edn while*/
	
	/*Set */
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(scheduleOid, &scheduleIid, scheduleObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(scheduleObj);
			return ret;
		}
		json_object_put(scheduleObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t scheduleObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	uint32_t  scheduleOid = 0;
	objIndex_t scheduleIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(scheduleIid);
	scheduleOid = RDM_OID_SCHEDULE;

	if((ret = zcfgFeObjStructAdd(scheduleOid, &scheduleIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add X_ZYXEL_Schedule Fail.\n", __FUNCTION__);
		return ret;
	}

	*idx = scheduleIid.idx[0];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t scheduleObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char scheduleObjFormate[128] = "InternetGatewayDevice.X_ZYXEL_EXT.Schedule.%hhu";
	uint32_t  scheduleOid = 0;
	objIndex_t scheduleIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(scheduleIid);
	scheduleIid.level= 1;
	if(sscanf(tr98FullPathName, scheduleObjFormate, scheduleIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	scheduleOid = RDM_OID_SCHEDULE;


	ret = zcfgFeObjStructDel(scheduleOid, &scheduleIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG, "%s : Delete X_ZYXEL_Schedule.%hhu Fail\n", __FUNCTION__, scheduleIid.idx[0]);
	}

	return ret;
}
#if 0
zcfgRet_t scheduleObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int scheduleObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
		

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t scheduleObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}
#endif
