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
#include "mgmtSrv_parameter.h"

extern tr98Object_t tr98Obj[];

/* InternetGatewayDevice.ManagementServer */
zcfgRet_t mgmtSrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  mgmtSrvOid = 0;
	objIndex_t mgmtSrvIid;
	struct json_object *mgmtSrvObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);	
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	zcfgFeHostsUpdateMsgSend();
	
	IID_INIT(mgmtSrvIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	mgmtSrvOid = zcfgFeObjNameToObjId(tr181Obj, &mgmtSrvIid);

	if((ret = feObjJsonGet(mgmtSrvOid, &mgmtSrvIid, &mgmtSrvObj, TR98_GET_WITHOUT_UPDATE)) != ZCFG_SUCCESS)
		return ret;
		

	/*fill up tr98 mgmtSrv object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
#ifdef ZYXEL_OPAL_EXTENDER
        if(paramList->attr & PARAMETER_ATTR_EXTENDER_HIDDEN)
        {
            paramList++;
            continue;
        }
#endif

		if(!strcmp(paramList->name, "X_ZYXEL_DelayReboot")){
			paramValue = json_object_object_get(mgmtSrvObj, "DelayReboot");
		}
		else{
			paramValue = json_object_object_get(mgmtSrvObj, paramList->name);
		}
		
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		if(!strcmp(paramList->name, "UDPConnectionRequestAddressNotificationLimit")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(0));
		}
		else
#endif
		json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
		paramList++;
	}
	json_object_put(mgmtSrvObj);

	return ZCFG_SUCCESS;
}
/* InternetGatewayDevice.ManagementServer */
zcfgRet_t mgmtSrvObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{

	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  mgmtSrvOid = 0;
	objIndex_t mgmtSrvIid;
	tr98Parameter_t *paramList = NULL;
	struct json_object *mgmtSrvObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(mgmtSrvIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	mgmtSrvOid = zcfgFeObjNameToObjId(tr181Obj, &mgmtSrvIid);
	if((ret = zcfgFeObjJsonGet(mgmtSrvOid, &mgmtSrvIid, &mgmtSrvObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = mgmtSrvObj;
		mgmtSrvObj = NULL;
		mgmtSrvObj = zcfgFeJsonMultiObjAppend(RDM_OID_MGMT_SRV, &mgmtSrvIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
		zcfgFeJsonMultiObjSetNonDelayApply(multiJobj);
	}

	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue == NULL) {
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "X_ZYXEL_QueryRootButNotRepliedObjects")){
			const char *objectPathList = json_object_get_string(paramValue);
			if(!zcfgSetRootQueryNotReplyObjects(objectPathList, 0)){
				if(!multiJobj)
					json_object_put(mgmtSrvObj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}

#ifdef TAAAB_HGW
		if(!strcmp(paramList->name, "URL")) {
			if(0 < strlen(json_object_get_string(paramValue))){
			    json_object_object_add(mgmtSrvObj, "X_ZYXEL_AllowOption43ChangeURL", json_object_new_boolean(false));
			}
			else{
				json_object_object_add(mgmtSrvObj, "X_ZYXEL_AllowOption43ChangeURL", json_object_new_boolean(true));
			}
		}
#endif	

		if(!strcmp(paramList->name, "X_ZYXEL_DelayReboot")){
			json_object_object_add(mgmtSrvObj, "DelayReboot", JSON_OBJ_COPY(paramValue));
		}
		else{
			json_object_object_add(mgmtSrvObj, paramList->name, JSON_OBJ_COPY(paramValue));
		}
		
		paramList++;
		continue;
	} /*Edn while*/
	
	/*Set Device.ManagementServer*/
	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MGMT_SRV, &mgmtSrvIid, mgmtSrvObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(mgmtSrvObj);
			return ret;
		}
		json_object_put(mgmtSrvObj);
	}

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.ManagementServer */
#if 0
zcfgRet_t mgmtSrvObjNotify(char *tr98ObjName, struct json_object *tr181ParamObj, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char *tr181ParamList = NULL;
	char *token = NULL;
	char tr98NotifyNameList[2048] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	tr181ParamList = (char *)json_object_get_string(tr181ParamObj);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();	
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		token = strtok(tr181ParamList, ",");
		while(token != NULL) {
			if(strcmp(token, paramList->name) == 0) {
				found = true;
				break;
			}
			token = strtok(NULL, ",");
		}

		if(found) {
			if(strlen(tr98NotifyNameList) == 0) {
				strcpy(tr98NotifyNameList, paramList->name);
			}
			else {
				strcat(tr98NotifyNameList, ",");
				strcpy(tr98NotifyNameList, paramList->name);
			}
		}
		
		paramList++;
		found = false;
	}

	if(strlen(tr98NotifyNameList) != 0) {
		json_object_object_add(*tr98NotifyInfo, tr98ObjName, json_object_new_string(tr98NotifyNameList));
	}

	return ZCFG_SUCCESS;
}
#endif

zcfgRet_t mgmtSrvObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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
#ifdef ZYXEL_OPAL_EXTENDER
        if(paramList->attr & PARAMETER_ATTR_EXTENDER_HIDDEN)
        {
            paramList++;
            continue;
        }
#endif
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

/* InternetGatewayDevice.ManagementServer */
int mgmtSrvObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  mgmtSrvOid = 0;
	objIndex_t mgmtSrvIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(mgmtSrvIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	mgmtSrvOid = zcfgFeObjNameToObjId(tr181Obj, &mgmtSrvIid);
		

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
#ifdef ZYXEL_OPAL_EXTENDER
        if(paramList->attr & PARAMETER_ATTR_EXTENDER_HIDDEN)
        {
            paramList++;
            continue;
        }
#endif
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		/*special case*/

		
		attrValue = zcfgFeParamAttrGetByName(mgmtSrvOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
			break;
		}

		break;
	}

	return attrValue;
}


/* InternetGatewayDevice.ManagementServer */
/* all use the batch set, only tr069 use this action*/
zcfgRet_t mgmtSrvObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  mgmtSrvOid = 0;
	objIndex_t mgmtSrvIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(mgmtSrvIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	mgmtSrvOid = zcfgFeObjNameToObjId(tr181Obj, &mgmtSrvIid);


	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(mgmtSrvOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if( (ret = zcfgFeMultiParamAttrAppend(mgmtSrvOid, multiAttrJobj, paramList->name, attrValue)) != ZCFG_SUCCESS){
			
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, mgmtSrvOid, paramList->name);
		}

		break;

	} /*Edn while*/
	
	return ret;
}

zcfgRet_t mgabDevObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mgabDevObjFormate[128] = "InternetGatewayDevice.ManagementServer.ManageableDevice.%hhu";
	uint32_t  mgabDevOid = 0;
	objIndex_t mgabDevIid;
	struct json_object *mgabDevObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	IID_INIT(mgabDevIid);
	mgabDevIid.level = 1;
	if(sscanf(tr98FullPathName, mgabDevObjFormate, mgabDevIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	mgabDevOid = RDM_OID_MGMT_SRV_MGAB_DEV;
	
	if((ret = feObjJsonGet(mgabDevOid, &mgabDevIid, &mgabDevObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 mgabDev object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(mgabDevObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(mgabDevObj);

	return ZCFG_SUCCESS;
}

