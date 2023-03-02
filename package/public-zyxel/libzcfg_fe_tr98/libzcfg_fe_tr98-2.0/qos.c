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


extern tr98Object_t tr98Obj[];

zcfgRet_t qMgmtObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t qosIid;
	struct json_object *qosObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	int defaultPolicer = -1;
	int defaultQueue = -1;

	IID_INIT(qosIid);
	if((ret = feObjJsonGet(RDM_OID_QOS, &qosIid, &qosObj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos object with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name){
		/* get parameter value from tr181 */
		paramValue = json_object_object_get(qosObj, paramList->name);

		/* write it to tr098 json object */
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "DefaultPolicer")){
				sscanf(json_object_get_string(paramValue), "Device.QoS.Policer.%d", &defaultPolicer);
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(defaultPolicer));
			}
			else if(!strcmp(paramList->name, "DefaultQueue")){
				sscanf(json_object_get_string(paramValue), "Device.QoS.Queue.%d", &defaultPolicer);
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(defaultPolicer));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}

		/* special case */
		if(!strcmp(paramList->name, "Enable")){
			paramValue = json_object_object_get(qosObj, "X_ZYXEL_Enabled");

			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(!strcmp(paramList->name, "X_ZYXEL_MaxShaperEntries")){
			paramValue = json_object_object_get(qosObj, "MaxShaperEntries");

			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(!strcmp(paramList->name, "X_ZYXEL_ShaperNumberOfEntries")){
			paramValue = json_object_object_get(qosObj, "ShaperNumberOfEntries");

			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		
		/*Not defined in tr181, give it a default value*/
		if(!strcmp(paramList->name, "MaxQueues")){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(0));
			paramList++;
			continue;
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	zcfgFeJsonObjFree(qosObj);

	return ZCFG_SUCCESS;

}

zcfgRet_t qMgmtSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t qosIid;
	struct json_object *qosObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	char defaultPolicer[128] = "";
	char defaultQueue[128] = "";
	int tmp = -1;

	IID_INIT(qosIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_QOS, &qosIid, &qosObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos object with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	/* if multiple set is required, just appended JSON to multiJobj */
	if(multiJobj){
		tmpObj = qosObj;
		qosObj = NULL;
		qosObj = zcfgFeJsonMultiObjAppend(RDM_OID_QOS, &qosIid, multiJobj, tmpObj);
	}

	/* set(update) value to target object */
	if(qosObj){
		paramList = tr98Obj[handler].parameter;
		while(paramList->name){
			/* get parameter value from tr098 */
			paramValue = json_object_object_get(tr98Jobj, paramList->name);

			/* write it to tr181 json object */
			if(paramValue != NULL) {
				if(!strcmp(paramList->name, "DefaultPolicer")){
					defaultPolicer[0] = '\0';
					tmp = -1;
					if((tmp = json_object_get_int(paramValue)) != -1){ 
						sprintf(defaultPolicer, "Device.QoS.Policer.%d", json_object_get_int(paramValue));
					}
					json_object_object_add(qosObj, paramList->name, json_object_new_string(defaultPolicer));
				}
				else if(!strcmp(paramList->name, "DefaultQueue")){
					defaultQueue[0] = '\0';
					tmp = -1;
					if((tmp = json_object_get_int(paramValue)) != -1){ 
						sprintf(defaultQueue, "Device.QoS.Queue.%d", json_object_get_int(paramValue));
					}
					json_object_object_add(qosObj, paramList->name, json_object_new_string(defaultQueue));
				}
				else if(!strcmp(paramList->name, "Enable")){
					json_object_object_add(qosObj, "X_ZYXEL_Enabled", JSON_OBJ_COPY(paramValue));
				}
				else if(!strcmp(paramList->name, "X_ZYXEL_MaxShaperEntries")){
					json_object_object_add(qosObj, "MaxShaperEntries", JSON_OBJ_COPY(paramValue));
				}
				else if(!strcmp(paramList->name, "X_ZYXEL_ShaperNumberOfEntries")){
					json_object_object_add(qosObj, "ShaperNumberOfEntries", JSON_OBJ_COPY(paramValue));
				}
				else if(!strcmp(paramList->name, "MaxQueues")){
					zcfgLog(ZCFG_LOG_ERR, "%s : MaxQueues is not supported in QoS object\n", __FUNCTION__);
				}
				else{
					json_object_object_add(qosObj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
			}

			paramList++;
			continue;
		}
	}

	/* is not multiple set, jsut set object immediately after update parameter value */
	if(!multiJobj){
		if((ret = zcfgFeObjJsonSet(RDM_OID_QOS, &qosIid, qosObj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set RDM_OID_QOS object Fail with ret=%d\n", __FUNCTION__, ret);
		}
		zcfgFeJsonObjFree(qosObj);
		return ret;
	}
	zcfgFeJsonObjFree(tmpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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
		
		if(strcmp(tr181ParamName, "X_ZYXEL_Enabled") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "Enable");
			break;
		}

		if(strcmp(tr181ParamName, "MaxShaperEntries") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "X_ZYXEL_MaxShaperEntries");
			break;
		}
		
		if(strcmp(tr181ParamName, "ShaperNumberOfEntries") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "X_ZYXEL_ShaperNumberOfEntries");
			break;
		}
		
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int qMgmtAttrGet(char *tr98FullPathName, int handler, char *paramName)
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
		if(strcmp(paramList->name, "Enable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_Enabled");
		}else if(strcmp(paramList->name, "X_ZYXEL_MaxShaperEntries") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "MaxShaperEntries");
		}else if(strcmp(paramList->name, "X_ZYXEL_ShaperNumberOfEntries") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "ShaperNumberOfEntries");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t qMgmtAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
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
		if(strcmp(paramList->name, "Enable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_Enabled");
		}else if(strcmp(paramList->name, "X_ZYXEL_MaxShaperEntries") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "MaxShaperEntries");
		}else if(strcmp(paramList->name, "X_ZYXEL_ShaperNumberOfEntries") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "ShaperNumberOfEntries");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if(strcmp(paramList->name, "Enable") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "X_ZYXEL_Enabled", attrValue);
		}else if(strcmp(paramList->name, "X_ZYXEL_MaxShaperEntries") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "MaxShaperEntries", attrValue);
		}else if(strcmp(paramList->name, "X_ZYXEL_ShaperNumberOfEntries") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "ShaperNumberOfEntries", attrValue);
		}else{
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		}
		
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}


zcfgRet_t qMgmtQueObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t qIid;
	struct json_object *qObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char tr181Intf[128] = {0};
	char tr098Intf[128] = {0};

	IID_INIT(qIid);
	qIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.Queue.%hhu", &qIid.idx[qIid.level - 1]);
	if((ret = feObjJsonGet(RDM_OID_QOS_QUE, &qIid, &qObj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos queue object with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name){
		/* get parameter value from tr181 */
		paramValue = json_object_object_get(qObj, paramList->name);

		/* write it to tr098 json object */
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/* special case */
		if(!strcmp(paramList->name, "QueueEnable")){
			paramValue = json_object_object_get(qObj, "Enable");
			if(paramValue){
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "QueueStatus")){
			paramValue = json_object_object_get(qObj, "Status");
			const char *sts = NULL;
			if(paramValue && (sts = json_object_get_string(paramValue))){
				if(strstr(sts, "Error"))
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Error"));
				else
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "QueueInterface")){
			//json_object_get_boolean(json_object_object_get(qObj, "AllInterfaces"));
			const char *interfaceName = json_object_get_string(json_object_object_get(qObj, "Interface"));
			if(interfaceName && strlen(interfaceName)){
				strcpy(tr181Intf, interfaceName);
				if(!strcmp(tr181Intf, "WAN")){
					strcpy(tr098Intf, tr181Intf);
				}
				else if((ret = zcfgFe181To98MappingNameGet(tr181Intf, tr098Intf)) != ZCFG_SUCCESS){
					zcfgLog(ZCFG_LOG_ERR, "%s: fail to mapping queue intf from tr181 to tr098 with ret=%d\n", __FUNCTION__, ret);
					zcfgFeJsonObjFree(qObj);
					return ret;
				}
			}
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr098Intf));
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "QueueBufferLength")){
			paramValue = json_object_object_get(qObj, "BufferLength");
			if(paramValue){
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "QueueWeight")){
			paramValue = json_object_object_get(qObj, "Weight");
			if(paramValue){
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "QueuePrecedence")){
			paramValue = json_object_object_get(qObj, "Precedence");
			if(paramValue){
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}
		
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	zcfgFeJsonObjFree(qObj);
	return ZCFG_SUCCESS;

}
#ifdef ZCFG_TR98_PARAM_GUARD
zcfgRet_t qMgmtQueParmGurd(const char *objpath, const char *parmname, struct json_object *spvparamobj, struct json_object *parmGurdObj, char **faultString)
{
	ztrdbg(ZTR_SL_INFO, "%s: %s\n", __FUNCTION__, parmname);

	return ZCFG_SUCCESS;
}
#endif
zcfgRet_t qMgmtQueSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t qIid;
	struct json_object *qObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	char tr098Intf[128] = {0};
	char tr181Intf[128] = {0};

	IID_INIT(qIid);
	qIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.Queue.%hhu", &qIid.idx[qIid.level - 1]);
	if((ret = zcfgFeObjJsonGet(RDM_OID_QOS_QUE, &qIid, &qObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos queue object with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	/* if multiple set is required, just appended JSON to multiJobj */
	if(multiJobj){
		tmpObj = qObj;
		qObj = NULL;
		qObj = zcfgFeJsonMultiObjAppend(RDM_OID_QOS_QUE, &qIid, multiJobj, tmpObj);
		zcfgFeJsonObjFree(tmpObj);
	}

	/* set(update) value to target object */
	if(qObj){
		paramList = tr98Obj[handler].parameter;
		while(paramList->name){
			/* get parameter value from tr098 */
			paramValue = json_object_object_get(tr98Jobj, paramList->name);

			/* write it to tr181 json object */
			if(paramValue != NULL) {
				/* special case */
				if(!strcmp(paramList->name, "QueueEnable")){
					json_object_object_add(qObj, "Enable", JSON_OBJ_COPY(paramValue));
				}
				else if(!strcmp(paramList->name, "QueueStatus")){
					json_object_object_add(qObj, "Status", JSON_OBJ_COPY(paramValue));
				}
				else if(!strcmp(paramList->name, "QueueInterface")){
					tr098Intf[0] = '\0';
					strcpy(tr098Intf, json_object_get_string(paramValue));
					if(!strcmp(tr098Intf, "WAN")){
						strcpy(tr181Intf, tr098Intf);
					}
					else if(tr098Intf[0] != '\0' && (ret = zcfgFe98To181MappingNameGet(tr098Intf, tr181Intf)) != ZCFG_SUCCESS){
						zcfgLog(ZCFG_LOG_ERR, "%s: fail to mapping queue intf from tr98 to tr181 with ret=%d\n", __FUNCTION__, ret);
						zcfgFeJsonObjFree(qObj);
						return ret;
					}

					json_object_object_add(qObj, "Interface", json_object_new_string(tr181Intf));
				}
				else if(!strcmp(paramList->name, "QueueBufferLength")){
					json_object_object_add(qObj, "BufferLength", JSON_OBJ_COPY(paramValue));
				}
				else if(!strcmp(paramList->name, "QueueWeight")){
					json_object_object_add(qObj, "Weight", JSON_OBJ_COPY(paramValue));
				}
				else if(!strcmp(paramList->name, "QueuePrecedence")){
					json_object_object_add(qObj, "Precedence", JSON_OBJ_COPY(paramValue));
				}
				else{
					json_object_object_add(qObj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
			}

			paramList++;
			continue;
		}

		/* is not multiple set, jsut set object immediately after update parameter value */
		if(!multiJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_QOS_QUE, &qIid, qObj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set RDM_OID_QOS_QUE object Fail with ret=%d\n", __FUNCTION__, ret);
			}
			zcfgFeJsonObjFree(qObj);
			return ret;
		}
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtQueAdd(char *tr98FullPathName, int *idx){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t qIid;

	IID_INIT(qIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_QOS_QUE, &qIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to add qos queue instance with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*idx = qIid.idx[qIid.level - 1];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtQueDel(char *tr98FullPathName){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t qIid;

	IID_INIT(qIid);
	qIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.Queue.%hhu", &qIid.idx[qIid.level - 1]);

	if((ret = zcfgFeObjJsonDel(RDM_OID_QOS_QUE, &qIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to delete qos queue instance with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtQueNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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
		
		if(strcmp(tr181ParamName, "Enable") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "QueueEnable");
			break;
		}
		if(strcmp(tr181ParamName, "Status") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "QueueStatus");
			break;
		}
		if(strcmp(tr181ParamName, "Interface") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "QueueInterface");
			break;
		}
		if(strcmp(tr181ParamName, "BufferLength") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "QueueBufferLength");
			break;
		}
		if(strcmp(tr181ParamName, "Weight") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "QueueWeight");
			break;
		}
		if(strcmp(tr181ParamName, "Precedence") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "QueuePrecedence");
			break;
		}
		
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int qMgmtQueAttrGet(char *tr98FullPathName, int handler, char *paramName)
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
		if(strcmp(paramList->name, "QueueEnable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "QueueStatus") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Status");
		}else if(strcmp(paramList->name, "QueueInterface") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Interface");
		}else if(strcmp(paramList->name, "QueueBufferLength") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "BufferLength");
		}else if(strcmp(paramList->name, "QueueWeight") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Weight");
		}else if(strcmp(paramList->name, "QueuePrecedence") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Precedence");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t qMgmtQueAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
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
		if(strcmp(paramList->name, "QueueEnable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "QueueStatus") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Status");
		}else if(strcmp(paramList->name, "QueueInterface") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Interface");
		}else if(strcmp(paramList->name, "QueueBufferLength") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "BufferLength");
		}else if(strcmp(paramList->name, "QueueWeight") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Weight");
		}else if(strcmp(paramList->name, "QueuePrecedence") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Precedence");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if(strcmp(paramList->name, "QueueEnable") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Enable", attrValue);
		}else if(strcmp(paramList->name, "QueueStatus") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Status", attrValue);
		}else if(strcmp(paramList->name, "QueueInterface") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Interface", attrValue);
		}else if(strcmp(paramList->name, "QueueBufferLength") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "BufferLength", attrValue);
		}else if(strcmp(paramList->name, "QueueWeight") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Weight", attrValue);
		}else if(strcmp(paramList->name, "QueuePrecedence") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Precedence", attrValue);
		}else{
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		}
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}

zcfgRet_t qMgmtPolicerGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *qPolicerObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.Policer.%hhu", &objIid.idx[objIid.level - 1]);
	if((ret = feObjJsonGet(RDM_OID_QOS_POLICER, &objIid, &qPolicerObj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos policer object with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name){
		paramValue = json_object_object_get(qPolicerObj, paramList->name);
		if(paramValue){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "PolicerEnable")){
			paramValue = json_object_object_get(qPolicerObj, "Enable");
		}
		else if(!strcmp(paramList->name, "PolicerStatus")){
			bool enable = json_object_get_boolean(json_object_object_get(qPolicerObj, "Enable"));
			if(enable)
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Enabled"));
			else
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Disabled"));
		}

		if(paramValue){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		paramList++;
	}

	zcfgFeJsonObjFree(qPolicerObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtPolicerSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *qPolicerObj = NULL;
	// struct json_object *paramValue = NULL;
	struct json_object *tr98ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.Policer.%hhu", &objIid.idx[objIid.level - 1]);
	if((ret = feObjJsonGet(RDM_OID_QOS_POLICER, &objIid, &qPolicerObj, 1)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos policer object with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	/* if multiple set is required, just appended JSON to multiJobj */
	if(multiJobj){
		tmpObj = qPolicerObj;
		qPolicerObj = zcfgFeJsonMultiObjAppend(RDM_OID_QOS_POLICER, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name){
		tr98ParamValue = json_object_object_get(tr98Jobj, (const char *)paramList->name);
		if(!tr98ParamValue){
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "PolicerEnable"))
		{
			// if((paramValue = json_object_object_get(qPolicerObj, "Enable")))
			if((json_object_object_get(qPolicerObj, "Enable")))
				json_object_object_add(qPolicerObj, "Enable", JSON_OBJ_COPY(tr98ParamValue));
		}
		else{
			// if((paramValue = json_object_object_get(qPolicerObj, paramList->name))){
			if((json_object_object_get(qPolicerObj, paramList->name))){
				json_object_object_add(qPolicerObj, paramList->name, JSON_OBJ_COPY(tr98ParamValue));
			}
		}

		paramList++;
	}

	if(!multiJobj) {
		if((ret = zcfgFeObjJsonSet(RDM_OID_QOS_POLICER, &objIid, qPolicerObj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos policer object, ret=%d\n", __FUNCTION__, ret);
		}
		zcfgFeJsonObjFree(qPolicerObj);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtPolicerAdd(char *tr98FullPathName, int *idx){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_QOS_POLICER, &objIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to add qos policer instance with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	*idx = objIid.idx[0];
#else
	printf("%s: return %d\n", __FUNCTION__, ret);

	if((ret == ZCFG_SUCCESS) || (ret == ZCFG_DELAY_APPLY)) {
		objIndex_t tr98ObjIid;

		IID_INIT(tr98ObjIid);
		if(zcfg98To181ObjMappingAdd(e_TR98_POLICER, &tr98ObjIid, RDM_OID_QOS_POLICER, &objIid) == ZCFG_SUCCESS) {
			printf("%s: Added QueueManagement.Policer %hhu\n", __FUNCTION__, tr98ObjIid.idx[0]);
			*idx = tr98ObjIid.idx[0];
		}
	}
#endif
	
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtPolicerDel(char *tr98FullPathName){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.Policer.%hhu", &objIid.idx[objIid.level - 1]);

	if((ret = zcfgFeObjJsonDel(RDM_OID_QOS_POLICER, &objIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to delete qos policer instance with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	return ZCFG_SUCCESS;
}
#ifdef ZCFG_TR98_PARAM_GUARD
zcfgRet_t qMgmtClsParmGurd(const char *objpath, const char *parmname, struct json_object *spvparamobj, struct json_object *parmGurdObj, char **faultString)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181PathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	ztrdbg(ZTR_SL_INFO, "%s: %s\n", __FUNCTION__, parmname);
	if(!strcmp(parmname, "ClassInterface")){
		const char *spv = json_object_get_string(spvparamobj);
		//if(!strstr(spv, "WANDevice") && !strstr(spv, "LANDevice"))
		if(!strstr(spv, "LANDevice"))
			return ZCFG_INVALID_PARAM_VALUE;
		ret = (zcfgFe98To181ObjMappingNameGet(spv, tr181PathName) == ZCFG_SUCCESS) ? ZCFG_SUCCESS : ZCFG_INVALID_PARAM_VALUE;
	}

	return ret;
}
#endif
zcfgRet_t qMgmtClsObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t cIid;
	struct json_object *cObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	int classPolicer = -1;
	int classApp = -1;
	char tr181Intf[128] = {0};
	char tr098Intf[128] = {0};

	IID_INIT(cIid);
	cIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.Classification.%hhu", &cIid.idx[cIid.level - 1]);
	if((ret = feObjJsonGet(RDM_OID_QOS_CLS, &cIid, &cObj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos queue object with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name){
		/* get parameter value from tr181 */
		paramValue = json_object_object_get(cObj, paramList->name);
		
		/* write it to tr098 json object */
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/* special case */
		if(!strcmp(paramList->name, "ClassificationEnable")){
			paramValue = json_object_object_get(cObj, "Enable");
			if(paramValue){
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "ClassificationStatus")){
			paramValue = json_object_object_get(cObj, "Status");
			const char *sts = NULL;
			if(paramValue && (sts = json_object_get_string(paramValue))){
				if(strstr(sts, "Error"))
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Error"));
				else
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "ClassificationOrder")){
			paramValue = json_object_object_get(cObj, "Order");
			if(paramValue){
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "ClassInterface")){
			paramValue = json_object_object_get(cObj, "Interface");
			if(paramValue){
				tr181Intf[0] = '\0';
				tr098Intf[0] = '\0';
				strcpy(tr181Intf, json_object_get_string(paramValue));
				
				if(tr181Intf[0] != '\0'){
					/* not "LAN" or "lo" */
					if(strcmp(tr181Intf, "LAN") && strcmp(tr181Intf, "lo")){
						if((ret = zcfgFe181To98MappingNameGet( tr181Intf, tr098Intf)) != ZCFG_SUCCESS){
							zcfgLog(ZCFG_LOG_ERR, "%s: fail to mapping class intf from tr181 to tr098 with ret=%d\n", __FUNCTION__, ret);
							tr098Intf[0] = '\0';
						}
					}
					else{
						strcpy(tr098Intf, tr181Intf);
					}
				}
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr098Intf));
			}
			paramList++;
			continue;
		}
		
		if(!strcmp(paramList->name, "ClassPolicer")){
			paramValue = json_object_object_get(cObj, "Policer");
			const char *clsPolicerStr = json_object_get_string(paramValue);
			if(clsPolicerStr && strlen(clsPolicerStr)){
				//sscanf(json_object_get_string(paramValue), "Device.QoS.Policer.%d", &classPolicer);
				sscanf(clsPolicerStr, "Policer.%d", &classPolicer);
				if(classPolicer == 0)
					classPolicer = -1;
			}
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(classPolicer));
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "ClassApp")){
			paramValue = json_object_object_get(cObj, "App");
			if(paramValue){
				sscanf(json_object_get_string(paramValue), "Device.QoS.App.%d", &classApp);
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(classApp));
			}
			paramList++;
			continue;
		}
		
#if 0
		if(!strcmp(paramList->name, "SourceVendorSpecificInfoMode")){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Exact"));
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "DestVendorSpecificInfoMode")){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Exact"));
			paramList++;
			continue;
		}
#endif
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	
	zcfgFeJsonObjFree(cObj);
		
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtClsSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t cIid;
	struct json_object *cObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char policer[128] = {0};
	char app[128] = "";
	char *ptr = NULL;
	char tr098Intf[128] = {0};
	char tr181Intf[128] = {0};

	IID_INIT(cIid);
	cIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.Classification.%hhu", &cIid.idx[cIid.level - 1]);
	if((ret = zcfgFeObjJsonGet(RDM_OID_QOS_CLS, &cIid, &cObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos queue object with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	/* if multiple set is required, just appended JSON to multiJobj */
	if(multiJobj){
		tmpObj = cObj;
		cObj = NULL;
		cObj = zcfgFeJsonMultiObjAppend(RDM_OID_QOS_CLS, &cIid, multiJobj, tmpObj);
		zcfgFeJsonObjFree(tmpObj);
	}

	if(cObj){
		paramList = tr98Obj[handler].parameter;
		while(paramList->name){
			/* get parameter value from tr098 */
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			
			/* write it to tr181 json object */
			if(paramValue != NULL) {
				/* special case */
				if(!strcmp(paramList->name, "ClassificationEnable")){
					json_object_object_add(cObj, "Enable", JSON_OBJ_COPY(paramValue));
				}
				else if(!strcmp(paramList->name, "ClassificationStatus")){
					json_object_object_add(cObj, "Status", JSON_OBJ_COPY(paramValue));
				}
				else if(!strcmp(paramList->name, "ClassificationOrder")){
					json_object_object_add(cObj, "Order", JSON_OBJ_COPY(paramValue));
				}
				else if(!strcmp(paramList->name, "ClassInterface")){
					tr098Intf[0] = '\0';
					ptr = tr181Intf + strlen(tr181Intf);
					strcpy(tr098Intf, json_object_get_string(paramValue));
					if(tr098Intf[0] != '\0'){
						/* not LAN or lo */
						if(strcmp(tr098Intf, "LAN") && strcmp(tr098Intf, "lo")){
							if((ret = zcfgFe98To181MappingNameGet(tr098Intf, ptr)) != ZCFG_SUCCESS){
								zcfgLog(ZCFG_LOG_ERR, "%s: fail to mapping class intf from tr98 to tr181 with ret=%d\n", __FUNCTION__, ret);
								//zcfgFeJsonObjFree(cObj);
								goto complete;
							}

							if(*ptr == '\0')
							tr181Intf[0] = '\0';
						}
						else{
							strcpy(tr181Intf, tr098Intf);
						}
					}
					json_object_object_add(cObj, "Interface", json_object_new_string(tr181Intf));				
				}
				else if(!strcmp(paramList->name, "ClassPolicer")){
					int clsPolicer = json_object_get_int(paramValue);
					if(clsPolicer < -1 || clsPolicer == 0){
						ret = ZCFG_INVALID_PARAM_VALUE;
						goto complete;
					}
					//sprintf(policer, "Device.QoS.Policer.%d", json_object_get_int(paramValue));
					if(clsPolicer != -1)
						sprintf(policer, "Policer.%d", clsPolicer);
					json_object_object_add(cObj, "Policer", json_object_new_string(policer));
				}
				else if(!strcmp(paramList->name, "ClassApp")){
					sprintf(app, "Device.QoS.App.%d", json_object_get_int(paramValue));
					json_object_object_add(cObj, "App", json_object_new_string(app));
				}
#if 0
				else if(!strcmp(paramList->name, "SourceVendorSpecificInfoMode")){
					zcfgLog(ZCFG_LOG_ERR, "%s : SourceVendorSpecificInfoMode could only be \"Exact\"\n", __FUNCTION__);
				}
				else if(!strcmp(paramList->name, "DestVendorSpecificInfoMode")){
					zcfgLog(ZCFG_LOG_ERR, "%s : DestVendorSpecificInfoMode could only be \"Exact\"\n", __FUNCTION__);
				}
#endif
				else{
					json_object_object_add(cObj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
			}

			paramList++;
			continue;
		}
	}


complete:
	if(ret != ZCFG_SUCCESS){
		if(!multiJobj)
			zcfgFeJsonObjFree(cObj);
	}else if(!multiJobj){
		/* is not multiple set, jsut set object immediately after update parameter value */
		if((ret = zcfgFeObjJsonSet(RDM_OID_QOS_CLS, &cIid, cObj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set RDM_OID_QOS_CLS object Fail with ret=%d\n", __FUNCTION__, ret);
		}
		zcfgFeJsonObjFree(cObj);
		return ret;
	}
	
	return ret;
}

zcfgRet_t qMgmtClsAdd(char *tr98FullPathName, int *idx){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t cIid;

	IID_INIT(cIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_QOS_CLS, &cIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to add qos class instance with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*idx = cIid.idx[cIid.level - 1];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtClsDel(char *tr98FullPathName){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t cIid;

	IID_INIT(cIid);
	cIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.Classification.%hhu", &cIid.idx[cIid.level - 1]);

	if((ret = zcfgFeObjJsonDel(RDM_OID_QOS_CLS, &cIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to delete qos class instance with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtClsNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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
		
		if(strcmp(tr181ParamName, "Enable") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "ClassificationEnable");
			break;
		}
		if(strcmp(tr181ParamName, "Status") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "ClassificationStatus");
			break;
		}
		if(strcmp(tr181ParamName, "Order") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "ClassificationOrder");
			break;
		}
		if(strcmp(tr181ParamName, "Interface") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "ClassInterface");
			break;
		}
		if(strcmp(tr181ParamName, "Policer") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "ClassPolicer");
			break;
		}
		if(strcmp(tr181ParamName, "App") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "ClassApp");
			break;
		}
		
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int qMgmtClsAttrGet(char *tr98FullPathName, int handler, char *paramName)
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
		if(strcmp(paramList->name, "ClassificationEnable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "ClassificationStatus") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Status");
		}else if(strcmp(paramList->name, "ClassificationOrder") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Order");
		}else if(strcmp(paramList->name, "ClassInterface") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Interface");
		}else if(strcmp(paramList->name, "ClassPolicer") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Policer");
		}else if(strcmp(paramList->name, "ClassApp") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "App");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t qMgmtClsAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
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
		if(strcmp(paramList->name, "ClassificationEnable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "ClassificationStatus") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Status");
		}else if(strcmp(paramList->name, "ClassificationOrder") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Order");
		}else if(strcmp(paramList->name, "ClassInterface") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Interface");
		}else if(strcmp(paramList->name, "ClassPolicer") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Policer");
		}else if(strcmp(paramList->name, "ClassApp") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "App");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if(strcmp(paramList->name, "ClassificationEnable") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Enable", attrValue);
		}else if(strcmp(paramList->name, "ClassificationStatus") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Enable", attrValue);
		}else if(strcmp(paramList->name, "ClassificationOrder") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Order", attrValue);
		}else if(strcmp(paramList->name, "ClassInterface") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Interface", attrValue);
		}else if(strcmp(paramList->name, "ClassPolicer") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Policer", attrValue);
		}else if(strcmp(paramList->name, "ClassApp") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "App", attrValue);
		}else{
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		}
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}


zcfgRet_t qMgmtShaperObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t sIid;
	struct json_object *sObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char tr098Intf[128] = {0};
	char tmpTr098Intf[128] = {0};
	char tr181Intf[128] = {0};

	IID_INIT(sIid);
	sIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.X_ZYXEL_Shaper.%hhu", &sIid.idx[sIid.level - 1]);
	if((ret = feObjJsonGet(RDM_OID_QOS_SHAPER, &sIid, &sObj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos shaper object with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name){
		/* get parameter value from tr181 */
		paramValue = json_object_object_get(sObj, paramList->name);

		/* write it to tr098 json object */
		if(paramValue != NULL) {

			if(!strcmp(paramList->name, "Interface")){
				paramValue = json_object_object_get(sObj, "Interface");
				tr098Intf[0] = '\0';
				tmpTr098Intf[0] = '\0';
				tr181Intf[0] = '\0';
				strcpy(tr181Intf, json_object_get_string(paramValue));
				
				if(tr181Intf[0] != '\0' && (ret = zcfgFe181To98MappingNameGet(tr181Intf, tmpTr098Intf)) != ZCFG_SUCCESS){
					zcfgLog(ZCFG_LOG_ERR, "%s: fail to mapping shaper intf from tr181 to tr098 with ret=%d\n", __FUNCTION__, ret);
					zcfgFeJsonObjFree(sObj);
					return ret;
				}
				strcpy(tr098Intf, tmpTr098Intf);
				ReplaceStaticWANDeviceIndex(tmpTr098Intf, tr098Intf); //move active wan index to WANDEV.1 , no matter what wan type it is.
				json_object_object_add(*tr98Jobj, "Interface", json_object_new_string(tr098Intf));
				
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
		
	}

	zcfgFeJsonObjFree(sObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtShaperSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t sIid;
	struct json_object *sObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char policer[128] = "";
	char app[128] = "";
	char tr098Intf[128] = {0};
	char tmpTr098Intf[128] = {0};
	char tr181Intf[128] = {0};

	IID_INIT(sIid);
	sIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.X_ZYXEL_Shaper.%hhu", &sIid.idx[sIid.level - 1]);
	if((ret = zcfgFeObjJsonGet(RDM_OID_QOS_SHAPER, &sIid, &sObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get qos shaper object with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	/* if multiple set is required, just appended JSON to multiJobj */
	if(multiJobj){
		tmpObj = sObj;
		sObj = NULL;
		sObj = zcfgFeJsonMultiObjAppend(RDM_OID_QOS_SHAPER, &sIid, multiJobj, tmpObj);
		zcfgFeJsonObjFree(tmpObj);
	}

	if(sObj){
		paramList = tr98Obj[handler].parameter;
		while(paramList->name){
			/* get parameter value from tr098 */
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			
			/* write it to tr181 json object */
			if(paramValue != NULL) {
				if(!strcmp(paramList->name, "Interface")){
					tmpTr098Intf[0] = '\0';
					strcpy(tmpTr098Intf, json_object_get_string(paramValue));
					strcpy(tr098Intf, tmpTr098Intf);
					ReplaceWANDeviceIndex(tmpTr098Intf, tr098Intf); //recover the active wan from WANDEV.1 back to it original WANDEV.
					if(tr098Intf[0] != '\0' && (ret = zcfgFe98To181MappingNameGet(tr098Intf, tr181Intf)) != ZCFG_SUCCESS){
						zcfgLog(ZCFG_LOG_ERR, "%s: fail to mapping class intf from tr98 to tr181 with ret=%d\n", __FUNCTION__, ret);
						zcfgFeJsonObjFree(sObj);
						return ret;
					}
					
					json_object_object_add(sObj, "Interface", json_object_new_string(tr181Intf));
				}
				else{
					json_object_object_add(sObj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
			}

			paramList++;
			continue;
		}
	}

	/* is not multiple set, jsut set object immediately after update parameter value */
	if(!multiJobj){
		if((ret = zcfgFeObjJsonSet(RDM_OID_QOS_SHAPER, &sIid, sObj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set RDM_OID_QOS_SHAPER object Fail with ret=%d\n", __FUNCTION__, ret);
		}
		zcfgFeJsonObjFree(sObj);
		return ret;
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtShaperAdd(char *tr98FullPathName, int *idx){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t sIid;

	IID_INIT(sIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_QOS_SHAPER, &sIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to add qos shaper instance with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*idx = sIid.idx[sIid.level - 1];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtShaperDel(char *tr98FullPathName){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t sIid;

	IID_INIT(sIid);
	sIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.QueueManagement.X_ZYXEL_Shaper.%hhu", &sIid.idx[sIid.level - 1]);

	if((ret = zcfgFeObjJsonDel(RDM_OID_QOS_SHAPER, &sIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to delete qos shaper instance with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t qMgmtShaperNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int qMgmtShaperAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

zcfgRet_t qMgmtShaperAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
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



