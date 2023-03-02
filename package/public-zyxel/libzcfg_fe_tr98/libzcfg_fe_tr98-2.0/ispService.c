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
#include "ispService_parameter.h"

extern tr98Object_t tr98Obj[];

#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
/* InternetGatewayDevice.Services.IspServices.i */

zcfgRet_t ispSrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ispSrvObjFormate[128] = "InternetGatewayDevice.Services.IspServices.%hhu";
	uint32_t  ispSrvOid = 0;
	objIndex_t ispSrvIid;
	struct json_object *ispSrvObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
#ifdef AVAST_AGENT_PACKAGE //Avast Agent
	char buffer[1024] = {0};
	char sysCmd[512] = {0};
	char *scriptName = NULL;
	FILE *fp = NULL;
#endif
	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(ispSrvIid);
	ispSrvIid.level= 1;
	if(sscanf(tr98FullPathName, ispSrvObjFormate, ispSrvIid.idx) != 1) return ZCFG_INVALID_OBJECT;
	ispSrvOid = RDM_OID_PACKAGE;
	
	if((ret = feObjJsonGet(ispSrvOid, &ispSrvIid, &ispSrvObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 PACKAGE object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ispSrvObj, paramList->name);
		if(paramValue != NULL) {	
#ifdef AVAST_AGENT_PACKAGE //Avast Agent
			if (strcmp(paramList->name, "Script") == 0){
				scriptName = json_object_get_string(paramValue);
				sprintf(sysCmd, "sh /misc/etc/init.d/%s status", scriptName);
			}
			if (strcmp(paramList->name, "Status") == 0){
				if((fp = popen(sysCmd, "r")) != NULL && fgets(buffer, sizeof(buffer), fp) != NULL) {
					printf("zcfgFeDalPackage3rdPartyGet : syscmd get status buffer: %s\n", buffer);
				}
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(buffer));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));	
			}
#else
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
#endif
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(ispSrvObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t ispSrvObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ispSrvObjFormate[128] = "InternetGatewayDevice.Services.IspServices.%hhu";
	uint32_t  ispSrvOid = 0;
	objIndex_t ispSrvIid;
	struct json_object *ispSrvObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	bool bEnable = false;
#ifdef AVAST_AGENT_PACKAGE //WIND request : enable agent and "Hash" can not be NULL
	char *pHash = NULL;
#endif
	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(ispSrvIid);
	ispSrvIid.level= 1;
	if(sscanf(tr98FullPathName, ispSrvObjFormate, ispSrvIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	ispSrvOid = RDM_OID_PACKAGE;

	if((ret = zcfgFeObjJsonGet(ispSrvOid, &ispSrvIid, &ispSrvObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = ispSrvObj;
		ispSrvObj = NULL;
		ispSrvObj = zcfgFeJsonMultiObjAppend(ispSrvOid, &ispSrvIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(ispSrvObj, paramList->name);
			if (strcmp(paramList->name, "Enable") == 0){
				bEnable = json_object_get_boolean(paramValue);
			}
#ifdef AVAST_AGENT_PACKAGE //WIND request : enable agent and "Hash" can not be NULL
			if (strcmp(paramList->name, "Hash") == 0){
				pHash = json_object_get_string(paramValue);
			}
#endif
			if(tr181ParamValue != NULL) {
				json_object_object_add(ispSrvObj, paramList->name, JSON_OBJ_COPY(paramValue));
				if (strcmp(paramList->name, "Enable") == 0){
					printf("%s : paramList->name >> %s = %d  \n", __FUNCTION__, paramList->name, bEnable);
				}
				paramList++;
				continue;
			}
		}
		paramList++;	
	} /*Edn while*/
	
	if (bEnable == true){
		SetParenCtlDisableForce();
	}

#ifdef AVAST_AGENT_PACKAGE //WIND request : enable agent and "Hash" can not be NULL
	if ((strlen(pHash) == 0) && (bEnable == true)){
		printf("%s : Hash is NULL >> %s = %d  \n", __FUNCTION__, paramList->name, bEnable);
		json_object_object_add(ispSrvObj, "Enable", json_object_new_boolean(false));
	}
#endif

	/*Set */
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(ispSrvOid, &ispSrvIid, ispSrvObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(ispSrvObj);
			return ret;
		}
		json_object_put(ispSrvObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t ispSrvObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	uint32_t  ispSrvOid = 0;
	objIndex_t ispSrvIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(ispSrvIid);
	ispSrvOid = RDM_OID_PACKAGE;

	if((ret = zcfgFeObjStructAdd(ispSrvOid, &ispSrvIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add IspServices Fail.\n", __FUNCTION__);
		return ret;
	}

	*idx = ispSrvIid.idx[0];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t ispSrvObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char ispSrvObjFormate[128] = "InternetGatewayDevice.Services.IspServices.%hhu";
	uint32_t  ispSrvOid = 0;
	objIndex_t ispSrvIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(ispSrvIid);
	ispSrvIid.level= 1;
	if(sscanf(tr98FullPathName, ispSrvObjFormate, ispSrvIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	ispSrvOid = RDM_OID_PACKAGE;


	ret = zcfgFeObjStructDel(ispSrvOid, &ispSrvIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG, "%s : Delete IspServices.%hhu Fail\n", __FUNCTION__, ispSrvIid.idx[0]);
	}

	return ret;
}
#endif

void SetParenCtlDisableForce(){
	printf("Enter : %s\n", __FUNCTION__);

	zcfgRet_t ret = ZCFG_SUCCESS;

	objIndex_t objIid = {0};
	uint32_t  parenCtlOid = 0;
	struct json_object *paCtlJobj = NULL;
	struct json_object *tmpObj = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PAREN_CTL, &objIid, &paCtlJobj)) != ZCFG_SUCCESS){
		return;
	}
	
	json_object_object_add(paCtlJobj, "Enable", json_object_new_boolean(false));

	if((ret = zcfgFeObjJsonSet(RDM_OID_PAREN_CTL, &objIid, paCtlJobj, NULL)) != ZCFG_SUCCESS ){
		json_object_put(paCtlJobj);
		return;
	}
	json_object_put(paCtlJobj);

}

