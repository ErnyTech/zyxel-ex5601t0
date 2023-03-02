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
#include "firewall_parameter.h"

extern tr98Object_t tr98Obj[];

/* InternetGatewayDevice.Firewall */
zcfgRet_t firewallObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  firewallOid = 0;
	objIndex_t firewallIid;
	struct json_object *firewallObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(firewallIid);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	firewallOid = zcfgFeObjNameToObjId(tr181Obj, &firewallIid);

	if((ret = feObjJsonGet(firewallOid, &firewallIid, &firewallObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
		

	/*fill up tr98 firewall object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if (strcmp("X_ZYXEL_IPv4_Enable", paramList->name)==0)
			paramValue = json_object_object_get(firewallObj, "IPv4_Enable");
		else if (strcmp("X_ZYXEL_IPv6_Enable", paramList->name)==0)
			paramValue = json_object_object_get(firewallObj, "IPv6_Enable");
		else
		paramValue = json_object_object_get(firewallObj, paramList->name);
		
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(firewallObj);

	return ZCFG_SUCCESS;
}
/* InternetGatewayDevice.Firewall */
zcfgRet_t firewallObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  firewallOid = 0;
	objIndex_t firewallIid;
	struct json_object *firewallObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(firewallIid);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	firewallOid = zcfgFeObjNameToObjId(tr181Obj, &firewallIid);
	if((ret = zcfgFeObjJsonGet(firewallOid, &firewallIid, &firewallObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = firewallObj;
		firewallObj = NULL;
		firewallObj = zcfgFeJsonMultiObjAppend(firewallOid, &firewallIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
#ifdef SAAAE_TR69_FIREWALL_CONTROL
                /*special case*/
                if(!strcmp(paramList->name, "Enable")) {
                        json_object_object_add(firewallObj, paramList->name, JSON_OBJ_COPY(paramValue));
			json_object_object_add(firewallObj, "IPv4_Enable", JSON_OBJ_COPY(paramValue));
			json_object_object_add(firewallObj, "IPv6_Enable", JSON_OBJ_COPY(paramValue));
                        paramList++;
                        continue;
                }
#endif
		if(paramValue != NULL) {
			if (strcmp("X_ZYXEL_IPv4_Enable", paramList->name)==0)
				tr181ParamValue = json_object_object_get(firewallObj, "IPv4_Enable");
			else if (strcmp("X_ZYXEL_IPv6_Enable", paramList->name)==0)
				tr181ParamValue = json_object_object_get(firewallObj, "IPv6_Enable");
			else
			tr181ParamValue = json_object_object_get(firewallObj, paramList->name);
			if(tr181ParamValue != NULL) {
				if (strcmp("X_ZYXEL_IPv4_Enable", paramList->name)==0)
					json_object_object_add(firewallObj, "IPv4_Enable", JSON_OBJ_COPY(paramValue));
				else if (strcmp("X_ZYXEL_IPv6_Enable", paramList->name)==0)
					json_object_object_add(firewallObj, "IPv6_Enable", JSON_OBJ_COPY(paramValue));
				else
				json_object_object_add(firewallObj, paramList->name, JSON_OBJ_COPY(paramValue));
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
		if((ret = zcfgFeObjJsonSet(firewallOid, &firewallIid, firewallObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(firewallObj);
			return ret;
		}
		json_object_put(firewallObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t firewallObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int firewallObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

zcfgRet_t firewallObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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


/* InternetGatewayDevice.Firewall.Level.i */
zcfgRet_t firewallLevelObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char levelObjFormate[128] = "InternetGatewayDevice.Firewall.Level.%hhu";
	uint32_t  levelOid = 0;
	objIndex_t levelIid;
	struct json_object *levelObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	IID_INIT(levelIid);
	levelIid.level = 1;
	if(sscanf(tr98FullPathName, levelObjFormate, levelIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	levelOid = RDM_OID_FIREWALL_LEVEL;
	
	if((ret = feObjJsonGet(levelOid, &levelIid, &levelObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 level object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(levelObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(levelObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t firewallLevelObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char levelObjFormate[128] = "InternetGatewayDevice.Firewall.Level.%hhu";
	uint32_t  levelOid = 0;
	objIndex_t levelIid;
	struct json_object *levelObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(levelIid);
	levelIid.level = 1;
	if(sscanf(tr98FullPathName, levelObjFormate, levelIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	levelOid = RDM_OID_FIREWALL_LEVEL;

	if((ret = zcfgFeObjJsonGet(levelOid, &levelIid, &levelObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = levelObj;
		levelObj = NULL;
		levelObj = zcfgFeJsonMultiObjAppend(levelOid, &levelIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(levelObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(levelObj, paramList->name, JSON_OBJ_COPY(paramValue));
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
		if((ret = zcfgFeObjJsonSet(levelOid, &levelIid, levelObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(levelObj);
			return ret;
		}
		json_object_put(levelObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t firewallLevelObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	uint32_t  levelOid = 0;
	objIndex_t levelIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(levelIid);
	levelOid = RDM_OID_FIREWALL_LEVEL;

	if((ret = zcfgFeObjStructAdd(levelOid, &levelIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add Firewall.Level Fail.\n", __FUNCTION__);
		return ret;
	}

	*idx = levelIid.idx[0];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t firewallLevelObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char levelObjFormate[128] = "InternetGatewayDevice.Firewall.Level.%hhu";
	uint32_t  levelOid = 0;
	objIndex_t levelIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(levelIid);
	levelIid.level = 1;
	if(sscanf(tr98FullPathName, levelObjFormate, levelIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	levelOid = RDM_OID_FIREWALL_LEVEL;


	ret = zcfgFeObjStructDel(levelOid, &levelIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG, "%s : Delete Firewall.Level.%hhu Fail\n", __FUNCTION__, levelIid.idx[0]);
	}

	return ret;
}

zcfgRet_t firewallLevelObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int firewallLevelObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

zcfgRet_t firewallLevelObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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


/* InternetGatewayDevice.Firewall.Chain.i */
zcfgRet_t firewallChainObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char chainObjFormate[128] = "InternetGatewayDevice.Firewall.Chain.%hhu";
	uint32_t  chainOid = 0;
	objIndex_t chainIid;
	struct json_object *chainObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(chainIid);
	chainIid.level = 1;
	if(sscanf(tr98FullPathName, chainObjFormate, chainIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	chainOid = RDM_OID_FIREWALL_CHAIN;

	if((ret = feObjJsonGet(chainOid, &chainIid, &chainObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}
		

	/*fill up tr98 chain object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(chainObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(chainObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t firewallChainObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char chainObjFormate[128] = "InternetGatewayDevice.Firewall.Chain.%hhu";
	uint32_t  chainOid = 0;
	objIndex_t chainIid;
	struct json_object *chainObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(chainIid);
	chainIid.level = 1;
	if(sscanf(tr98FullPathName, chainObjFormate, chainIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	chainOid = RDM_OID_FIREWALL_CHAIN;

	if((ret = zcfgFeObjJsonGet(chainOid, &chainIid, &chainObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = chainObj;
		chainObj = NULL;
		chainObj = zcfgFeJsonMultiObjAppend(chainOid, &chainIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(chainObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(chainObj, paramList->name, JSON_OBJ_COPY(paramValue));
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
		if((ret = zcfgFeObjJsonSet(chainOid, &chainIid, chainObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(chainObj);
			return ret;
		}
		json_object_put(chainObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t firewallChainObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	uint32_t  chainOid = 0;
	objIndex_t chainIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(chainIid);
	chainOid = RDM_OID_FIREWALL_CHAIN;

	if((ret = zcfgFeObjStructAdd(chainOid, &chainIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add Firewall.Chain Fail.\n", __FUNCTION__);
		return ret;
	}

	*idx = chainIid.idx[0];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t firewallChainObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char chainObjFormate[128] = "InternetGatewayDevice.Firewall.Chain.%hhu";
	uint32_t  chainOid = 0;
	objIndex_t chainIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(chainIid);
	chainIid.level = 1;
	if(sscanf(tr98FullPathName, chainObjFormate, chainIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	chainOid = RDM_OID_FIREWALL_CHAIN;


	ret = zcfgFeObjStructDel(chainOid, &chainIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG, "%s : Delete Firewall.Chain.%hhu Fail\n", __FUNCTION__, chainIid.idx[0]);
	}

	return ret;
}

zcfgRet_t firewallChainObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int firewallChainObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

zcfgRet_t firewallChainObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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


/* InternetGatewayDevice.Firewall.Chain.i.Rule.i */
zcfgRet_t firewallRuleObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ruleObjFormate[128] = "InternetGatewayDevice.Firewall.Chain.%hhu.Rule.%hhu";
	uint32_t  ruleOid = 0;
	objIndex_t ruleIid;
	struct json_object *ruleObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(ruleIid);
	ruleIid.level = 2;
	if(sscanf(tr98FullPathName, ruleObjFormate, &ruleIid.idx[0], &ruleIid.idx[1]) != 2) return ZCFG_INVALID_OBJECT;	
	ruleOid = RDM_OID_FIREWALL_CHAIN_RULE;

	if((ret = feObjJsonGet(ruleOid, &ruleIid, &ruleObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
		

	/*fill up tr98 rule object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ruleObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(ruleObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t firewallRuleObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ruleObjFormate[128] = "InternetGatewayDevice.Firewall.Chain.%hhu.Rule.%hhu";
	uint32_t  ruleOid = 0;
	objIndex_t ruleIid;
	struct json_object *ruleObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(ruleIid);
	ruleIid.level = 2;
	if(sscanf(tr98FullPathName, ruleObjFormate, &ruleIid.idx[0], &ruleIid.idx[1]) != 2) return ZCFG_INVALID_OBJECT;	
	ruleOid = RDM_OID_FIREWALL_CHAIN_RULE;

	if((ret = zcfgFeObjJsonGet(ruleOid, &ruleIid, &ruleObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = ruleObj;
		ruleObj = NULL;
		ruleObj = zcfgFeJsonMultiObjAppend(ruleOid, &ruleIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(ruleObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ruleObj, paramList->name, JSON_OBJ_COPY(paramValue));
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
		if((ret = zcfgFeObjJsonSet(ruleOid, &ruleIid, ruleObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(ruleObj);
			return ret;
		}
		json_object_put(ruleObj);
	}

	return ZCFG_SUCCESS;
}


zcfgRet_t firewallRuleObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char ruleObjFormate[128] = "InternetGatewayDevice.Firewall.Chain.%hhu.Rule";
	uint32_t  ruleOid = 0;
	objIndex_t ruleIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(ruleIid);
	ruleIid.level = 1;
	if(sscanf(tr98FullPathName, ruleObjFormate, &ruleIid.idx[0]) != 1) return ZCFG_INVALID_OBJECT;	
	ruleOid = RDM_OID_FIREWALL_CHAIN_RULE;

	if((ret = zcfgFeObjStructAdd(ruleOid, &ruleIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add Firewall.Chain.%hhu.Rule. Fail.\n", __FUNCTION__, ruleIid.idx[0]);
		return ret;
	}

	*idx = ruleIid.idx[1];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t firewallRuleObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char ruleObjFormate[128] = "InternetGatewayDevice.Firewall.Chain.%hhu.Rule.%hhu";
	uint32_t  ruleOid = 0;
	objIndex_t ruleIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(ruleIid);
	ruleIid.level = 2;
	if(sscanf(tr98FullPathName, ruleObjFormate, &ruleIid.idx[0], &ruleIid.idx[1]) != 2) return ZCFG_INVALID_OBJECT;	
	ruleOid = RDM_OID_FIREWALL_CHAIN_RULE;

	ret = zcfgFeObjStructDel(ruleOid, &ruleIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG, "%s : Delete Firewall.Chain.%hhu.Rule.%hhu Fail\n", __FUNCTION__, ruleIid.idx[0], ruleIid.idx[1]);
	}

	return ret;
}

zcfgRet_t firewallRuleObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int firewallRuleObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

zcfgRet_t firewallRuleObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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


