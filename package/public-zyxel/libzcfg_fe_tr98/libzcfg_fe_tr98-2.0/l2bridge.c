
#include <stdlib.h>
#include <errno.h>
#include <json/json.h>
#include <limits.h>

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
#include "l2bridge_parameter.h"
#include "zos_api.h"

extern tr98Object_t tr98Obj[];

#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
int getMappingVlanTermPathName(char ipConnIindex, char *tr98pathname)
{
	objIndex_t objIid;
	rdm_MgmtSrv_t *MgmtSrvObj = NULL;
	rdm_IpIface_t *IpIfaceObj = NULL;
	rdm_PppIface_t *PppIfaceObj = NULL;
	char *ptr;
	char tr181PathName[128] = {0};
	zcfgRet_t ret = ZCFG_INTERNAL_ERROR;

	//get TR69 binding interface path name
	if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_MGMT_SRV, &objIid, (void **)&MgmtSrvObj) == ZCFG_SUCCESS) {
		if (strlen(MgmtSrvObj->X_ZYXEL_ActiveBoundInterface) > 0){
			if(zcfgFe181To98MappingNameGet(MgmtSrvObj->X_ZYXEL_ActiveBoundInterface, tr98pathname) == ZCFG_SUCCESS)
				ret = ZCFG_SUCCESS;
		}
		zcfgFeObjStructFree(MgmtSrvObj);
	}

	//change the IPConn/PPPConn index to ipConnIindex, find IPConn/PPPConn then get lowerlayer(VLANTermination)
	if (ret == ZCFG_SUCCESS){
		ret = ZCFG_INTERNAL_ERROR;
		if (ptr = strstr(tr98pathname, "WANIPConnection."))
			ptr += 16;
		else if (ptr = strstr(tr98pathname, "WANPPPConnection."))
			ptr += 17;

		*ptr = ipConnIindex;

		if(zcfgFe98To181MappingNameGet(tr98pathname, tr181PathName) == ZCFG_SUCCESS){
			IID_INIT(objIid);
			objIid.level = 1;
			if (strstr(tr181PathName, "IP") != NULL){
				sscanf(tr181PathName, "IP.Interface.%hhu", &objIid.idx[0]);
				if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_IP_IFACE, &objIid, (void **)&IpIfaceObj) == ZCFG_SUCCESS) {
					if (strstr(IpIfaceObj->LowerLayers, "VLANTermination")){
						strcpy(tr181PathName, IpIfaceObj->LowerLayers);
						ret = ZCFG_SUCCESS;
					}
					zcfgFeObjStructFree(IpIfaceObj);
				}
			}
			else if (strstr(tr181PathName, "PPP") != NULL){
				sscanf(tr181PathName, "PPP.Interface.%hhu", &objIid.idx[0]);
				if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_PPP_IFACE, &objIid, (void **)&PppIfaceObj) == ZCFG_SUCCESS) {
					if (strstr(PppIfaceObj->LowerLayers, "VLANTermination")){
						strcpy(tr181PathName, PppIfaceObj->LowerLayers);
						ret = ZCFG_SUCCESS;
					}
					zcfgFeObjStructFree(PppIfaceObj);
				}
			}
		}
	}

	if (ret == ZCFG_SUCCESS){
		ret = ZCFG_INTERNAL_ERROR;
		if(zcfgFe98To181MappingNameGet(tr181PathName, tr98pathname) == ZCFG_SUCCESS){
			ret = ZCFG_SUCCESS;
		}
	}

	return ret;
}
#endif

/*  TR98 Object Name : InternetGatewayDevice.Layer2Bridging

    Related object in TR181:
    Device.Bridging
*/
zcfgRet_t l2BridingObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  bridingOid = 0;
	objIndex_t bridingIid;
	struct json_object *bridingJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char zyxelPrefixParam[64] = {0};

	printf("Enter %s\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else                /* mappingPathName will be Bridging */
		printf("TR181 object %s\n", mappingPathName);

	IID_INIT(bridingIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	bridingOid = zcfgFeObjNameToObjId(tr181Obj, &bridingIid);
	if((ret = feObjJsonGet(bridingOid, &bridingIid, &bridingJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 Bridging object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		paramValue = json_object_object_get(bridingJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		strcpy(zyxelPrefixParam, "X_ZYXEL_");
		strcat(zyxelPrefixParam, paramList->name);
		paramValue = json_object_object_get(bridingJobj, zyxelPrefixParam);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(bridingJobj);
	return ZCFG_SUCCESS;
}

zcfgRet_t l2BridingObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

		if(strcmp(tr181ParamName, "X_ZYXEL_MaxMarkingEntries") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "MaxMarkingEntries");
			break;
		}

		if(strcmp(tr181ParamName, "X_ZYXEL_MarkingNumberOfEntries") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "MarkingNumberOfEntries");
			break;
		}

		if(strcmp(tr181ParamName, "X_ZYXEL_AvailableInterfaceNumberOfEntries") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "AvailableInterfaceNumberOfEntries");
			break;
		}

		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int l2BridingObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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
		if(strcmp(paramList->name, "MaxMarkingEntries") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_MaxMarkingEntries");
		}else if(strcmp(paramList->name, "MarkingNumberOfEntries") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_MarkingNumberOfEntries");
		}else if(strcmp(paramList->name, "AvailableInterfaceNumberOfEntries") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_AvailableInterfaceNumberOfEntries");
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

zcfgRet_t l2BridingObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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
		if(strcmp(paramList->name, "MaxMarkingEntries") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_MaxMarkingEntries");
		}else if(strcmp(paramList->name, "MarkingNumberOfEntries") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_MarkingNumberOfEntries");
		}else if(strcmp(paramList->name, "AvailableInterfaceNumberOfEntries") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_AvailableInterfaceNumberOfEntries");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if(strcmp(paramList->name, "MaxMarkingEntries") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "X_ZYXEL_MaxMarkingEntries", attrValue);
		}else if(strcmp(paramList->name, "MarkingNumberOfEntries") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "X_ZYXEL_MarkingNumberOfEntries", attrValue);
		}else if(strcmp(paramList->name, "AvailableInterfaceNumberOfEntries") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "X_ZYXEL_AvailableInterfaceNumberOfEntries", attrValue);
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


/*  TR98 Object Name : InternetGatewayDevice.Layer2Bridging.Bridge.i

    Related object in TR181:
    Device.Bridging.Bridge.i
    Device.Bridging.Bridge.i.Port.i
*/
zcfgRet_t l2BrObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	char tmpChar[4] ={0};
	char paramName[64] = {0};
	uint32_t  brOid = 0;
	objIndex_t brIid;
	struct json_object *brJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	uint8_t numOfInstance = 0;
	char *ptr = NULL;
	char zyxelPrefixParam[64] = {0};
#ifdef SAAAD_TR98_CUSTOMIZATION
	char tr98vlanPathName[128];
#endif

	printf("Enter %s\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
#ifdef SAAAD_TR98_CUSTOMIZATION
#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
		ptr = strstr(tr98FullPathName, "Bridge.") + 7;
		if (*ptr < '4' && *ptr > '0'){ // Bridge.1 ~ Bridge.3
			if (getMappingVlanTermPathName(*ptr ,tr98vlanPathName) != ZCFG_SUCCESS)
				return ZCFG_NO_SUCH_OBJECT;
		}
#else
		sprintf(tr98vlanPathName, "%s.%s", tr98FullPathName, "VLAN.1");
#endif
		ztrdbg(ZTR_DL_DEBUG, "tr98vlanPathName=%s\n",tr98vlanPathName);
		if((ret = zcfgFe98To181MappingNameGet(tr98vlanPathName, mappingPathName)) == ZCFG_SUCCESS){
			ztrdbg(ZTR_DL_DEBUG, "mappingPathName=%s\n",mappingPathName);
			*tr98Jobj = json_object_new_object();
			return ret;
		}
#endif
		ztrdbg(ZTR_DL_DEBUG, "98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else        /* mappingPathName will be Bridging */
		ztrdbg(ZTR_DL_DEBUG, "%s : TR181 object %s\n", __FUNCTION__, mappingPathName);

	ptr = strrchr(mappingPathName, '.');
	strcpy(tmpChar, ptr+1);
	numOfInstance = atoi(tmpChar);

	IID_INIT(brIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	brOid = zcfgFeObjNameToObjId(tr181Obj, &brIid);
	if((ret = feObjJsonGet(brOid, &brIid, &brJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 Bridging object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		paramValue = json_object_object_get(brJobj, paramList->name);
		if((paramValue != NULL) && !strcmp(paramList->name, "PortNumberOfEntries")) {
			int num = json_object_get_int(paramValue);
			num = (num > 0) ? (num-1) : 0;
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(num));
		}
		else if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		/*we default use this object index number as parameter 'BridgeKey' under tr98 */
		if(strstr(paramList->name, "BridgeKey") != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(numOfInstance));
			paramList++;
			continue;
		}

		if(strstr(paramList->name, "Standard") != NULL){
			sprintf(paramName, "%s", "Standard");
			paramValue = json_object_object_get(brJobj, paramName);
			if(paramValue != NULL){
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(strstr(paramList->name, "Enable") != NULL){
			sprintf(paramName, "%s", "Enable");
			paramValue = json_object_object_get(brJobj, paramName);
			if(paramValue != NULL){
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(strstr(paramList->name, "Status") != NULL){
			sprintf(paramName, "%s", "Status");
			paramValue = json_object_object_get(brJobj, paramName);
			if(paramValue != NULL){
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		strcpy(zyxelPrefixParam, "X_ZYXEL_");
		strcat(zyxelPrefixParam, paramList->name);
		paramValue = json_object_object_get(brJobj, zyxelPrefixParam);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}
	json_object_put(brJobj);
	return ZCFG_SUCCESS;
}

zcfgRet_t l2BrObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  brOid = 0;
	objIndex_t brIid;
	struct json_object *brJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	char zyxelPrefixParam[64] = {0};

	ztrdbg(ZTR_DL_INFO, "Enter %s\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		ztrdbg(ZTR_DL_ERR, "98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else      /* mappingPathName will be Bridging */
		ztrdbg(ZTR_DL_DEBUG, "TR181 object %s\n", mappingPathName);

	IID_INIT(brIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	brOid = zcfgFeObjNameToObjId(tr181Obj, &brIid);
	if((ret = zcfgFeObjJsonGet(brOid, &brIid, &brJobj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = brJobj;
		brJobj = NULL;
		brJobj = zcfgFeJsonMultiObjAppend(RDM_OID_BRIDGING_BR, &brIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		printf("Parameter Name %s\n", paramList->name);
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			/*Write value from Config to Device.Bridging.Bridge.i*/
			tr181ParamValue = json_object_object_get(brJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(brJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		/*special case*/
		if(strstr(paramList->name, "Standard") != NULL){
			json_object_object_add(brJobj, "Standard", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		if(strstr(paramList->name, "Enable") != NULL){
			json_object_object_add(brJobj, "Enable", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		strcpy(zyxelPrefixParam, "X_ZYXEL_");
		strcat(zyxelPrefixParam, paramList->name);
		tr181ParamValue = json_object_object_get(brJobj, zyxelPrefixParam);
		if(tr181ParamValue != NULL) {
			json_object_object_add(brJobj, zyxelPrefixParam, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		paramList++;
	} /*Edn while*/

	/*Set Bridging.Bridge.i */
	if(multiJobj)
		json_object_put(tmpObj);
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_BRIDGING_BR, &brIid, brJobj, NULL)) != ZCFG_SUCCESS ){
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Bridging.Bridge.i Fail\n", __FUNCTION__);
			json_object_put(brJobj);
			return ret;
		}
		else
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Bridging.Bridge.i Success\n", __FUNCTION__);

		json_object_put(brJobj);
	}
	return ZCFG_SUCCESS;
}

// InternetGatewayDevice.Layer2Bridging.Bridge.{i}.        Bridging.Bridge.{i}.
zcfgRet_t l2BrObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t iid;
	rdm_BridgingBrPort_t *bridgePortObj = NULL;

	printf("Enter %s\n", __FUNCTION__);

	/*add the object "Device.Bridging.Bridge.i" */
	IID_INIT(iid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_BRIDGING_BR, &iid, NULL)) != ZCFG_SUCCESS){
		printf("Add Instance Fail!!\n");
		return ret;
	}

	/*add the management object "Device.Bridging.Bridge.i.Port.1"*/
	if((ret = zcfgFeObjStructAdd(RDM_OID_BRIDGING_BR_PORT, &iid, NULL)) != ZCFG_SUCCESS){
		printf("Add Instance Fail!!\n");
		return ret;
	}
//printf("306 Level %d objIid.idx[%d,%d,%d,%d,%d,%d]\n",  iid.level, iid.idx[0], iid.idx[1], iid.idx[2], iid.idx[3], iid.idx[4], iid.idx[5]);

	if((ret = zcfgFeObjStructGet(RDM_OID_BRIDGING_BR_PORT, &iid, (void **)&bridgePortObj)) == ZCFG_SUCCESS){
		bridgePortObj->Enable = true;
		bridgePortObj->ManagementPort = true;
		if((ret = zcfgFeObjStructSet(RDM_OID_BRIDGING_BR_PORT, &iid, (void *)bridgePortObj, NULL)) != ZCFG_SUCCESS)
			printf("Set Bridge Port Object Fail\n");

		zcfgFeObjStructFree(bridgePortObj);
	}
	else
		return ret;

	*idx = iid.idx[0];
	return ZCFG_SUCCESS;
}

zcfgRet_t l2BrObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mappingPathName[MAX_TR181_PATHNAME] = {0};
	uint32_t  oid = 0;
	objIndex_t objIid;
	//struct json_object *brPortJobj = NULL;
	struct json_object *brJobj = NULL;

	ztrdbg(ZTR_DL_INFO, "Enter %s\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(objIid);
	printf("mappingPathName %s\n", mappingPathName);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	oid = zcfgFeObjNameToObjId(tr181Obj, &objIid);
	if(oid != RDM_OID_BRIDGING_BR)
		return ZCFG_INTERNAL_ERROR;

	if(zcfgFeObjJsonGet(RDM_OID_BRIDGING_BR, &objIid, &brJobj) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}

	ret = zcfgFeObjJsonDelayRdmDel(RDM_OID_BRIDGING_BR, &objIid, NULL);
	if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS)) {
		ztrdbg(ZTR_DL_ERR, "%s: Delete bridge Object Fail\n", __FUNCTION__);
		json_object_put(brJobj);
		return ret;
	}

	json_object_put(brJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t l2BrObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

		if(strcmp(tr181ParamName, "Standard") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "BridgeStandard");
			break;
		}

		if(strcmp(tr181ParamName, "Enable") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "BridgeEnable");
			break;
		}

		if(strcmp(tr181ParamName, "Status") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "BridgeStatus");
			break;
		}

		if(strcmp(tr181ParamName, "X_ZYXEL_BridgeName") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "BridgeName");
			break;
		}

		if(strcmp(tr181ParamName, "X_ZYXEL_VLANID") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "VLANID");
			break;
		}

		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int l2BrObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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
		if(strcmp(paramList->name, "BridgeStandard") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Standard");
		}else if(strcmp(paramList->name, "BridgeEnable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "BridgeStatus") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Status");
		}else if(strcmp(paramList->name, "BridgeName") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_BridgeName");
		}else if(strcmp(paramList->name, "VLANID") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_VLANID");
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

zcfgRet_t l2BrObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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
		if(strcmp(paramList->name, "BridgeStandard") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Standard");
		}else if(strcmp(paramList->name, "BridgeEnable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "BridgeStatus") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Status");
		}else if(strcmp(paramList->name, "BridgeName") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_BridgeName");
		}else if(strcmp(paramList->name, "VLANID") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_VLANID");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}

		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if(strcmp(paramList->name, "BridgeStandard") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Standard", attrValue);
		}else if(strcmp(paramList->name, "BridgeEnable") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Enable", attrValue);
		}else if(strcmp(paramList->name, "BridgeStatus") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Status", attrValue);
		}else if(strcmp(paramList->name, "BridgeName") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "X_ZYXEL_BridgeName", attrValue);
		}else if(strcmp(paramList->name, "VLANID") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "X_ZYXEL_VLANID", attrValue);
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

/*  TR98 Object Name : InternetGatewayDevice.Layer2Bridging.Filter.i

    Related object in TR181:
    Device.Bridging.Filter.i
*/
zcfgRet_t l2BrFilterObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tmpChar[4] = {0};
	//char tr181Obj[128] = {0};
	//char paramName[64] = {0};
	char filterBrRef[32] = {0};
	char filterIntf[32] = {0};
	char numOfFilterIntf[4] = {0};
	uint32_t  brPortOid = 0;
	char *ptr = NULL;
	uint8_t numOfInstance = 0;
	objIndex_t brFilterIid, brPortIid;
	struct json_object *brFilterJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char zyxelPrefixParam[64] = {0};
	rdm_BridgingBrPort_t *brPortObj = NULL;

	ztrdbg(ZTR_DL_INFO, "Enter %s\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		ztrdbg(ZTR_DL_ERR, "%s: 98 to 181 Mapping Fail!!\n", __FUNCTION__);
		return ret;
	}
	else   /* mappingPathName will be Bridging.Filter.i */
		ztrdbg(ZTR_DL_DEBUG, "%s: TR181 object %s\n", __FUNCTION__, mappingPathName);

	ptr = strrchr(mappingPathName, '.');
	ZOS_STRNCPY(tmpChar, ptr+1,sizeof(tmpChar));
	numOfInstance = atoi(tmpChar);

	IID_INIT(brFilterIid);
#if 0
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	brFilterOid = zcfgFeObjNameToObjId(tr181Obj, &brFilterIid);
	if((ret = feObjJsonGet(brFilterOid, &brFilterIid, &brFilterJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
#else
	brFilterIid.level = 1;
	sscanf(mappingPathName, "Bridging.Filter.%hhu", &brFilterIid.idx[0]);
	if((ret = feObjJsonGet(RDM_OID_BRIDGING_FILTER, &brFilterIid, &brFilterJobj, updateFlag)) != ZCFG_SUCCESS) {
		ztrdbg(ZTR_DL_ERR, "%s : Get Obj Fail\n", __FUNCTION__);
		return ret;
	}
#endif

	/*fill up tr98 Bridging object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		paramValue = json_object_object_get(brFilterJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		/*we default use this object index number as parameter 'FilterKey' under tr98 */
		//if((strstr(paramList->name, "FilterKey") != NULL) ||(strstr(paramList->name, "FilterInterface") != NULL)){
		if(strstr(paramList->name, "FilterKey") != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(numOfInstance));
			paramList++;
			continue;
		}

		/*we default use index number of object "Device.Bridging.Bridge.i" as the parameter 'FilterBridgeReference' under tr98 */
		if(strstr(paramList->name, "FilterBridgeReference") != NULL){
			//sprintf(paramName, "%s", "Bridge");
			paramValue = json_object_object_get(brFilterJobj, "Bridge");
			const char *paramBridge = json_object_get_string(paramValue);
			if(paramBridge && strlen(paramBridge)) {
				zos_snprintf(filterBrRef,sizeof(filterBrRef), "%s", paramBridge);
				ptr = NULL;
				ptr = strrchr(filterBrRef, '.');
				ZOS_STRNCPY(tmpChar,ptr+1,sizeof(tmpChar));
				numOfInstance = atoi(tmpChar);
				json_object_object_add(*tr98Jobj, paramList->name,  json_object_new_int(numOfInstance));
			}
			else
				json_object_object_add(*tr98Jobj, paramList->name,  json_object_new_int(-1));

			paramList++;
			continue;
		}

		/*  we default use parameter 'X_ZYXEL_AvailableInterfaceKey' under object "Bridging.Bridge.i.Port.i" as the parameter 'FilterInterface' under tr98.
		 *  actually in tr98 , the parameter 'FilterInterface' is associated with the corresponding 'AvailableInterfaceKey' under tr98 object
		 *  'InternetGatewayDevice.Layer2Bridging.AvailableInterface.i',
		 *  but in tr181, it doesn't have the AvailableInterface object, so we add the parameters which belong 'AvailableInterface' under object 'Bridging.Bridge.i.Port.i'.
		 */
		if(strstr(paramList->name, "FilterInterface") != NULL){
			//sprintf(paramName, "%s", "Interface");
			const char *paramStr = json_object_get_string(json_object_object_get(brFilterJobj, "Interface"));
			if(paramStr && strlen(paramStr)){
				IID_INIT(brPortIid);
				zos_snprintf(filterIntf,sizeof(filterIntf), "Device.%s", paramStr);
				brPortOid = zcfgFeObjNameToObjId(filterIntf, &brPortIid);
				if((ret = feObjStructGet(brPortOid, &brPortIid, (void **)&brPortObj, updateFlag)) == ZCFG_SUCCESS) {
					sprintf(numOfFilterIntf,"%d",brPortObj->X_ZYXEL_AvailableInterfaceKey);
					zcfgFeObjStructFree(brPortObj);
				}

				if(strlen(numOfFilterIntf))
					json_object_object_add(*tr98Jobj, paramList->name,  json_object_new_string(numOfFilterIntf));
				else
					json_object_object_add(*tr98Jobj, paramList->name,  json_object_new_string(""));
			}
			else
				json_object_object_add(*tr98Jobj, paramList->name,  json_object_new_string(""));

			paramList++;
			continue;
		}

		if(strstr(paramList->name, "Enable") != NULL){
			//sprintf(paramName, "%s", "Enable");
			paramValue = json_object_object_get(brFilterJobj, "Enable");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(strstr(paramList->name, "Status") != NULL){
			//sprintf(paramName, "%s", "Status");
			paramValue = json_object_object_get(brFilterJobj, "Status");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		strcpy(zyxelPrefixParam, "X_ZYXEL_");
		strcat(zyxelPrefixParam, paramList->name);
		paramValue = json_object_object_get(brFilterJobj, zyxelPrefixParam);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

	json_object_put(brFilterJobj);

	return ZCFG_SUCCESS;
}

static bool  crParamEmpty(struct json_object *param)
{
	json_type parmType = json_object_get_type(param);
	bool empty = true;

	switch(parmType) {
	case json_type_boolean:
		empty = json_object_get_boolean(param) ? false : true;
		break;
	case json_type_string:
		{
		const char *value = json_object_get_string(param);
		empty = (value && strlen(value)) ? false : true;
		break;
		}
	default: break;
	}

	return empty;
}

#define TR98_PARM_FILTER_CFG "{\
    \"VLANIDFilter\": {}, \"EthertypeFilterList\": {}, \"SourceMACAddressFilterList\": {}, \"DestMACAddressFilterList\": {}, \
    \"SourceMACFromVendorClassIDFilter\": {}, \"DestMACFromVendorClassIDFilter\": {}, \"SourceMACFromClientIDFilter\": {}, \
    \" DestMACFromClientIDFilter\": {}, \"SourceMACFromUserClassIDFilter\": {}, \"DestMACFromUserClassIDFilter\": {} }"

#define paramThere(parmList, parmName) (paramList && (json_object_object_get(parmList, parmName)!=NULL))

zcfgRet_t l2BrFilterObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char brPath[32] = {0};
	char brPortPath[32] = {0};
	char tr181Obj[128] = {0};
	uint32_t  brFilterOid = 0;
	objIndex_t brFilterIid, brPortIid;
	uint8_t filterIntf =0;
	struct json_object *brFilterJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	char zyxelPrefixParam[64] = {0};
	rdm_BridgingBrPort_t *brPortObj = NULL;
	bool crParameterNotEmpty = false;
	struct json_object *crParmsObj = NULL;
	int bridgeReference = 0;
	struct json_object *ipIfaceObj = NULL;
	objIndex_t ipIfaceIid;
	bool bridgeSpecified = false;

	printf("Enter %s\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else       /* mappingPathName will be Bridging.Filter.i */
		printf("TR181 object %s\n", mappingPathName);

	IID_INIT(brFilterIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	brFilterOid = zcfgFeObjNameToObjId(tr181Obj, &brFilterIid);
	if((ret = zcfgFeObjJsonGet(brFilterOid, &brFilterIid, &brFilterJobj)) != ZCFG_SUCCESS)
		return ret;

	crParmsObj = json_tokener_parse((const char *)TR98_PARM_FILTER_CFG);

	if(multiJobj){
		tmpObj = brFilterJobj;
		brFilterJobj = NULL;
		brFilterJobj = zcfgFeJsonMultiObjAppend(RDM_OID_BRIDGING_FILTER, &brFilterIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			if((crParameterNotEmpty == false) && paramThere(crParmsObj, paramList->name)) {
				crParameterNotEmpty = (crParamEmpty(paramValue) == true) ? false : true;
			}

			/*Write value from Config to Device.Bridging.Filter.i*/
			tr181ParamValue = json_object_object_get(brFilterJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(brFilterJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		/*special case*/
		if(strstr(paramList->name, "Enable") != NULL){
			json_object_object_add(brFilterJobj, "Enable", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		if(strstr(paramList->name, "FilterBridgeReference") != NULL){
			bridgeReference =  json_object_get_int(paramValue);
			if(bridgeReference > 0) {
				sprintf(brPath, "Bridging.Bridge.%d", bridgeReference);
				json_object_object_add(brFilterJobj, "Bridge", json_object_new_string(brPath));
			}
			paramList++;
			continue;
		}

		if(strstr(paramList->name, "FilterInterface") != NULL){
			IID_INIT(brPortIid);
			const char *intf = json_object_get_string(paramValue);
			if(intf && strlen(intf) && ((filterIntf = atoi(intf)) > 0)) {
				//filterIntf = json_object_get_int(paramValue);
				while(zcfgFeObjStructGetNext(RDM_OID_BRIDGING_BR_PORT, &brPortIid, (void **)&brPortObj) == ZCFG_SUCCESS){
					if (brPortObj->X_ZYXEL_AvailableInterfaceKey == filterIntf){
						sprintf(brPortPath, "Bridging.Bridge.%d.Port.%d", brPortIid.idx[0], brPortIid.idx[1]);
						zcfgFeObjStructFree(brPortObj);
						break;
					}
					else
						zcfgFeObjStructFree(brPortObj);
				}
			}

			if(strlen(brPortPath))
				json_object_object_add(brFilterJobj, "Interface", json_object_new_string(brPortPath));
			paramList++;
			continue;
		}

		strcpy(zyxelPrefixParam, "X_ZYXEL_");
		strcat(zyxelPrefixParam, paramList->name);
		tr181ParamValue = json_object_object_get(brFilterJobj, zyxelPrefixParam);
		if(tr181ParamValue != NULL) {
			json_object_object_add(brFilterJobj, zyxelPrefixParam, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		paramList++;
	} /*Edn while*/

	if(bridgeReference > 0) {
		char ipIfaceIfName[30] = {0};

		sprintf(ipIfaceIfName, "br%d", (bridgeReference-1));
		IID_INIT(ipIfaceIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS) {
			const char *ifName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_IfName"));
			if(ifName && strlen(ifName) && !strcmp(ifName, ipIfaceIfName)) {
				bridgeSpecified = true;
				break;
			}
			json_object_put(ipIfaceObj);
		}
		if(multiJobj && bridgeSpecified == true) {
			zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE, &ipIfaceIid, multiJobj, ipIfaceObj);
			json_object_put(ipIfaceObj);
		}
	}

	if(crParmsObj) {
		json_object_object_add(brFilterJobj, "X_ZYXEL_Enable_Criteria", json_object_new_boolean(crParameterNotEmpty));
		json_object_put(crParmsObj);
	}

	/*Set Device.Bridging.Filter.i */
	if(multiJobj)
		json_object_put(tmpObj);
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_BRIDGING_FILTER, &brFilterIid, brFilterJobj, NULL)) != ZCFG_SUCCESS ){
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Bridging.Filter.i Fail\n", __FUNCTION__);
			json_object_put(brFilterJobj);
			return ret;
		}
		else
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Bridging.Filter.i Success\n", __FUNCTION__);

		json_object_put(brFilterJobj);

		if(bridgeSpecified != true)
			return ZCFG_SUCCESS;

		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ipIfaceIid, ipIfaceObj, NULL)) != ZCFG_SUCCESS ){
			zcfgLog(ZCFG_LOG_ERR, "%s : Set IP.Interface Fail\n", __FUNCTION__);
			json_object_put(ipIfaceObj);
			return ret;
		}
		else
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set IP.Interface Success\n", __FUNCTION__);

		json_object_put(ipIfaceObj);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t l2BrFilterObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	ztrdbg(ZTR_DL_INFO, "Enter %s\n", __FUNCTION__);

	/*add the object "Device.Bridging.Filter.i" */
	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_BRIDGING_FILTER, &objIid, NULL)) != ZCFG_SUCCESS){
		ztrdbg(ZTR_DL_ERR, "Add Instance Fail!!\n");
		return ret;
	}

	*idx = objIid.idx[0];
	return ZCFG_SUCCESS;
}

zcfgRet_t l2BrFilterObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mappingPathName[32] = {0};
	uint32_t  oid = 0;
	objIndex_t objIid;

	ztrdbg(ZTR_DL_INFO, "Enter %s\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(objIid);
	ztrdbg(ZTR_DL_DEBUG, "%s: mappingPathName %s\n", __FUNCTION__, mappingPathName);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	oid = zcfgFeObjNameToObjId(tr181Obj, &objIid);

	ret = zcfgFeObjJsonDel(oid, &objIid, NULL);
	if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
		ztrdbg(ZTR_DL_ERR, "%s: Delete Object Fail\n", __FUNCTION__);

	return ret;
}

zcfgRet_t l2BrFilterObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

		if(strcmp(tr181ParamName, "Bridge") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "FilterBridgeReference");
			break;
		}

		if(strcmp(tr181ParamName, "Enable") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "FileterEnable");
			break;
		}

		if(strcmp(tr181ParamName, "Status") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "FilterStatus");
			break;
		}

		if(strcmp(tr181ParamName, "Interface") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "FilterInterface");
			break;
		}

		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int l2BrFilterObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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
		if(strcmp(paramList->name, "FilterBridgeReference") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Bridge");
		}else if(strcmp(paramList->name, "FileterEnable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "FilterStatus") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Status");
		}else if(strcmp(paramList->name, "FilterInterface") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Interface");
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

zcfgRet_t l2BrFilterObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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
		if(strcmp(paramList->name, "FilterBridgeReference") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Bridge");
		}else if(strcmp(paramList->name, "FileterEnable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "FilterStatus") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Status");
		}else if(strcmp(paramList->name, "FilterInterface") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Interface");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}

		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if(strcmp(paramList->name, "FilterBridgeReference") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Bridge", attrValue);
		}else if(strcmp(paramList->name, "FileterEnable") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Enable", attrValue);
		}else if(strcmp(paramList->name, "FilterStatus") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Status", attrValue);
		}else if(strcmp(paramList->name, "FilterInterface") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Interface", attrValue);
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

/*  TR98 Object Name : InternetGatewayDevice.Layer2Bridging.AvailableInterface.i

    Related object in TR181:
    Device.Bridging.Bridge.i.Port.i
*/
zcfgRet_t l2BrAvailableIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[32] = {0};
	objIndex_t objIid;
	struct json_object *brPortJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char zyxelPrefixParam[64] = {0};
	char tr181Obj[32] = {0};
	char mapping98ObjName[128] = {0};

	printf("Enter %s\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else   /* mappingPathName will be Bridging */
		printf("TR181 object %s\n", mappingPathName);

	IID_INIT(objIid);
#if 0
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	oid = zcfgFeObjNameToObjId(tr181Obj, &objIid);

	if((ret = feObjJsonGet(oid, &objIid, &brPortJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
#else
	objIid.level = 2;
	sscanf(mappingPathName, "Bridging.Bridge.%hhu.Port.%hhu", &objIid.idx[0], &objIid.idx[1]);
	if((ret = feObjJsonGet(RDM_OID_BRIDGING_BR_PORT, &objIid, &brPortJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
#endif
	/*fill up tr98 Bridging object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*special case*/
		if(strstr(paramList->name, "InterfaceReference") != NULL){
			sprintf(tr181Obj, "%s", json_object_get_string(json_object_object_get(brPortJobj, "LowerLayers")));
			zcfgFe181To98MappingNameGet(tr181Obj, mapping98ObjName);
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(mapping98ObjName));
			paramList++;
			continue;
		}

		strcpy(zyxelPrefixParam, "X_ZYXEL_");
		strcat(zyxelPrefixParam, paramList->name);
		paramValue = json_object_object_get(brPortJobj, zyxelPrefixParam);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(brPortJobj);
	return ZCFG_SUCCESS;
}

zcfgRet_t l2BrAvailableIntfObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

		if(strcmp(tr181ParamName, "LowerLayers") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "InterfaceReference");
			break;
		}

		if(strcmp(tr181ParamName, "X_ZYXEL_AvailableInterfaceKey") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "AvailableInterfaceKey");
			break;
		}

		if(strcmp(tr181ParamName, "X_ZYXEL_InterfaceType") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "InterfaceType");
			break;
		}

		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int l2BrAvailableIntfObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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
		if(strcmp(paramList->name, "InterfaceReference") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "LowerLayers");
		}else if(strcmp(paramList->name, "AvailableInterfaceKey") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_AvailableInterfaceKey");
		}else if(strcmp(paramList->name, "InterfaceType") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_InterfaceType");
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

zcfgRet_t l2BrAvailableIntfObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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
		if(strcmp(paramList->name, "InterfaceReference") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "LowerLayers");
		}else if(strcmp(paramList->name, "AvailableInterfaceKey") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_AvailableInterfaceKey");
		}else if(strcmp(paramList->name, "InterfaceType") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_InterfaceType");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}

		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if(strcmp(paramList->name, "BridgeStandard") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Standard", attrValue);
		}else if(strcmp(paramList->name, "BridgeEnable") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Enable", attrValue);
		}else if(strcmp(paramList->name, "BridgeStatus") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Status", attrValue);
		}else{
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		}

		if(strcmp(paramList->name, "InterfaceReference") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "LowerLayers", attrValue);
		}else if(strcmp(paramList->name, "AvailableInterfaceKey") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "X_ZYXEL_AvailableInterfaceKey", attrValue);
		}else if(strcmp(paramList->name, "InterfaceType") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "X_ZYXEL_InterfaceType", attrValue);
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

/*

  InternetGatewayDevice.Layer2Bridging.Bridge.{i}.Port.{i}.            Device.Bridging.Bridge.{i}.Port.{i}.
*/
zcfgRet_t l2BrPortObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t brPortObjIid;
	char mappingPathName[100] = {0};
	struct json_object *tr181BrPortObj = NULL;
	struct json_object *paramValue;
	tr98Parameter_t *paramList = NULL;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}

	printf("%s: tr181 %s\n", __FUNCTION__, mappingPathName);

	IID_INIT(brPortObjIid);
	brPortObjIid.level = 2;
	sscanf(mappingPathName, "Bridging.Bridge.%hhu.Port.%hhu", &brPortObjIid.idx[0], &brPortObjIid.idx[1]);
	if((ret = feObjJsonGet(RDM_OID_BRIDGING_BR_PORT, &brPortObjIid, &tr181BrPortObj, TR98_GET_WITHOUT_UPDATE)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		paramValue = json_object_object_get(tr181BrPortObj, paramList->name);
		if(!paramValue){
			if(!strcmp(paramList->name, "PortEnable"))
				paramValue = json_object_object_get(tr181BrPortObj, "Enable");
			else if(!strcmp(paramList->name, "PortInterface")) {
				paramValue = json_object_object_get(tr181BrPortObj, "X_ZYXEL_AvailableInterfaceKey");
				if(paramValue) {
					char portInterface[20] = {0};
					sprintf(portInterface, "%lld", json_object_get_int(paramValue));
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(portInterface));
				}
				paramList++;
				continue;
			}
		}

		if(paramValue){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(tr181BrPortObj);

	return ZCFG_SUCCESS;
}

#define ETHWAN_IFNAME "eth4.0"
#define MAX_ETH_INTF_NUM   4
	#if defined(ZYXEL_EASYMESH)
		#define MAX_LAN_INTF_NUM (MAX_ETH_INTF_NUM + TOTAL_INTERFACE_24G + TOTAL_INTERFACE_5G - 2) //-2 :exclude preset & backhaul
	#else
		#define MAX_LAN_INTF_NUM (MAX_ETH_INTF_NUM + TOTAL_INTERFACE_24G + TOTAL_INTERFACE_5G)
	#endif
#define MAX_INTERFACEKEY  20


/*

  InternetGatewayDevice.Layer2Bridging.Bridge.{i}.Port.{i}.        Bridging.Bridge.{i}.Port.{i}.
*/
zcfgRet_t l2BrPortObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t brBdgPtIid, ssidIid;
	struct json_object *brBdgPtObj = NULL;
	struct json_object *ssidObj = NULL;
	char mappingPathName[MAX_TR181_PATHNAME] = {0};
	struct json_object *paramValue, *tr181ParamValue, *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}

	if(!strstr(mappingPathName, "Bridge")) {
		printf("%s: Mapping not correct\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	IID_INIT(brBdgPtIid);
	sscanf(mappingPathName, "Bridging.Bridge.%hhu.Port.%hhu", &brBdgPtIid.idx[0], &brBdgPtIid.idx[1]);
	brBdgPtIid.level = 2;
	if((ret = zcfgFeObjJsonGet(RDM_OID_BRIDGING_BR_PORT, &brBdgPtIid, &brBdgPtObj)) != ZCFG_SUCCESS) {
		printf("%s: Retrieve Bridging.Bridge.i.Port.i fail\n", __FUNCTION__);
		return ret;
	}

	if(multiJobj){
		tmpObj = brBdgPtObj;
		brBdgPtObj = NULL;
		brBdgPtObj = zcfgFeJsonMultiObjAppend(RDM_OID_BRIDGING_BR_PORT, &brBdgPtIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		//printf("Parameter Name %s\n", paramList->name);
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			/*Write value from Config to Device.Bridging.Bridge.i*/
			tr181ParamValue = json_object_object_get(brBdgPtObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(brBdgPtObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		/*special case*/
		if(strstr(paramList->name, "PortEnable") != NULL){
			json_object_object_add(brBdgPtObj, "Enable", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		if(strstr(paramList->name, "PortInterface") != NULL){
			const char *portIntf = json_object_get_string(paramValue);
			char *endptr = NULL;
			char ifName[15] = {0}, trIntfName[21] = {0};
			long int interfaceKey = strtol(portIntf, &endptr, 10);
			errno = 0;
			if((errno == ERANGE && (interfaceKey == LONG_MAX || interfaceKey == LONG_MIN)) || (errno != 0 && interfaceKey == 0)
				|| (interfaceKey > MAX_INTERFACEKEY)) {
				zcfgLog(ZCFG_LOG_ERR, "%s: %s not valid\n", __FUNCTION__, paramList->name);
				if(!multiJobj)
					json_object_put(brBdgPtObj);

				return ZCFG_INVALID_PARAM_VALUE;
			}
			if(interfaceKey == 0) {
				sprintf(ifName, ETHWAN_IFNAME);
			}
			else if((interfaceKey > 0) && (interfaceKey <= MAX_ETH_INTF_NUM)) {
				sprintf(trIntfName, "Ethernet.Interface.%hhu", (int)interfaceKey);
				sprintf(ifName, "eth%d.0", (int)(interfaceKey-1));
			}
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
			else if((interfaceKey > MAX_ETH_INTF_NUM) && (interfaceKey <= MAX_LAN_INTF_NUM)) {
				sprintf(trIntfName, "WiFi.SSID.%hhu", (int)(interfaceKey-MAX_ETH_INTF_NUM));
			// get ssid obj
				IID_INIT(ssidIid);
				ssidIid.level = 1;
				ssidIid.idx[0] = (int)(interfaceKey-MAX_ETH_INTF_NUM);

				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &ssidIid, &ssidObj) == ZCFG_SUCCESS) {
					strcpy(ifName, json_object_get_string(json_object_object_get(ssidObj, "Name")));
				}
				zcfgFeObjStructFree(ssidObj);
			}
#endif
			else {
				if(!multiJobj)
					json_object_put(brBdgPtObj);

				return ZCFG_INVALID_PARAM_VALUE;
			}
			json_object_object_add(brBdgPtObj, "X_ZYXEL_AvailableInterfaceKey", json_object_new_int((int)interfaceKey));
			json_object_object_add(brBdgPtObj, "LowerLayers", json_object_new_string(trIntfName));

			json_object_object_add(brBdgPtObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(brBdgPtObj, "X_ZYXEL_InterfaceType", json_object_new_string("LANInterface"));
			json_object_object_add(brBdgPtObj, "X_ZYXEL_IfName", json_object_new_string(ifName));

			paramList++;
			continue;
		}

		paramList++;
	} /*Edn while*/

	/*Set Bridging.Bridge.i */
	if(multiJobj) {
		//printf("%s: Deliver multiObj\n", __FUNCTION__);
		zcfgLog(ZCFG_LOG_INFO, "%s: Deliver multiObj\n", __FUNCTION__);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_BRIDGING_BR_PORT, &brBdgPtIid, brBdgPtObj, NULL)) != ZCFG_SUCCESS ){
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Bridging.Bridge.i.Port.i Fail\n", __FUNCTION__);
			json_object_put(brBdgPtObj);
			return ret;
		}
		else
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Bridging.Bridge.i.Port.i Success\n", __FUNCTION__);

		json_object_put(brBdgPtObj);
	}

	return ZCFG_SUCCESS;
}

// InternetGatewayDevice.Layer2Bridging.Bridge.{i}.Port.
zcfgRet_t l2BrPortObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid, mgmtObjIid;
	rdm_BridgingBrPort_t *brBdgPtObj = NULL;
	char mappingPathName[MAX_TR181_PATHNAME] = {0};
	char tr98TmpName[MAX_TR181_PATHNAME] = {0};
	char *ptr = NULL;

	printf("%s\n", __FUNCTION__);

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".Port");
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, mappingPathName)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_INFO, "%s: Mapping query fail\n", __FUNCTION__);
		return ret;
	}

#if ZYXEL_GRE_SUPPORT
	/*check it is gre br or not*/
	rdm_BridgingBr_t *brBdgObj = NULL;
	IID_INIT(objIid);
	sscanf(mappingPathName, "Bridging.Bridge.%hhu", &objIid.idx[0]);
	objIid.level = 1;
	if((ret = zcfgFeObjStructGet(RDM_OID_BRIDGING_BR, &objIid, (void **)&brBdgObj)) == ZCFG_SUCCESS)
	{
		if(strstr(brBdgObj->X_ZYXEL_BridgeName, "brgre"))
		{
			printf("%s: can not modify gre br\n", __FUNCTION__);
			return ZCFG_INVALID_PARAM_VALUE;
		}
		zcfgFeObjStructFree(brBdgObj);
	}
#endif

	IID_INIT(objIid);
	sscanf(mappingPathName, "Bridging.Bridge.%hhu", &objIid.idx[0]);
	objIid.level = 1;
	if((ret = zcfgFeObjStructAdd(RDM_OID_BRIDGING_BR_PORT, &objIid, NULL)) != ZCFG_SUCCESS){
		printf("%s: Add Bridge Port Object Fail!!\n", __FUNCTION__);
		return ret;
	}

	if((ret = zcfgFeObjStructGet(RDM_OID_BRIDGING_BR_PORT, &objIid, (void **)&brBdgPtObj)) == ZCFG_SUCCESS){
		brBdgPtObj->Enable = true;
		brBdgPtObj->ManagementPort = false;
		if((ret = zcfgFeObjStructSet(RDM_OID_BRIDGING_BR_PORT, &objIid, (void *)brBdgPtObj, NULL)) != ZCFG_SUCCESS)
			printf("%s: Set Bridge Port Object Fail\n", __FUNCTION__);

		zcfgFeObjStructFree(brBdgPtObj);
	}

	memcpy(&mgmtObjIid, &objIid, sizeof(objIndex_t));
	mgmtObjIid.idx[1] = 1;
	mgmtObjIid.level = 1;
	if((ret = zcfgFeObjStructGet(RDM_OID_BRIDGING_BR_PORT, &mgmtObjIid, (void **)&brBdgPtObj)) == ZCFG_SUCCESS){
		char bridgingPortName[30] = {0};
		sprintf(bridgingPortName, ",Bridging.Bridge.%hhu.Port.%hhu", objIid.idx[0], objIid.idx[1]);
		if(strlen(brBdgPtObj->LowerLayers))
			ZOS_STRCAT(brBdgPtObj->LowerLayers, bridgingPortName, sizeof(brBdgPtObj->LowerLayers));
		else
			ZOS_STRNCPY(brBdgPtObj->LowerLayers, bridgingPortName+1, sizeof(brBdgPtObj->LowerLayers));

		if((ret = zcfgFeObjStructSet(RDM_OID_BRIDGING_BR_PORT, &mgmtObjIid, (void *)brBdgPtObj, NULL)) != ZCFG_SUCCESS)
			printf("%s: Set Bridge Mgmt Port Object Fail\n", __FUNCTION__);

		zcfgFeObjStructFree(brBdgPtObj);
	}

	//add mapping
#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	*idx = objIid.idx[0];
#else
	printf("%s: return %d\n", __FUNCTION__, ret);

	if((ret == ZCFG_SUCCESS) || (ret == ZCFG_DELAY_APPLY)) {
		objIndex_t tr98ObjIid;

		IID_INIT(tr98ObjIid);
		sscanf(tr98FullPathName, "InternetGatewayDevice.Layer2Bridging.Bridge.%hhu", &tr98ObjIid.idx[0]);
		tr98ObjIid.level = 1;
		if(zcfg98To181ObjMappingAdd(e_TR98_PORT, &tr98ObjIid, RDM_OID_BRIDGING_BR_PORT, &objIid) == ZCFG_SUCCESS) {
			printf("%s: Added Bridge.Port %hhu\n", __FUNCTION__, tr98ObjIid.idx[1]);
			*idx = tr98ObjIid.idx[1];
		}
	}
#endif

	return ret;
}

// InternetGatewayDevice.Layer2Bridging.Bridge.{i}.Port.
zcfgRet_t l2BrPortObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid, mgmtObjIid;
	char mappingPathName[MAX_TR181_PATHNAME] = {0};
	rdm_BridgingBrPort_t *brBdgPtObj = NULL;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_INFO, "%s: Mapping query fail\n", __FUNCTION__);
		return ret;
	}

	if(!strstr(mappingPathName, "Bridge")) {
		printf("%s: Mapping not correct\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	
#if ZYXEL_GRE_SUPPORT
	/*check it is gre br or not*/
	rdm_BridgingBr_t *brBdgObj = NULL;
	IID_INIT(objIid);
	sscanf(mappingPathName, "Bridging.Bridge.%hhu", &objIid.idx[0]);
	objIid.level = 1;
	if((ret = zcfgFeObjStructGet(RDM_OID_BRIDGING_BR, &objIid, (void **)&brBdgObj)) == ZCFG_SUCCESS)
	{
		if(strstr(brBdgObj->X_ZYXEL_BridgeName, "GRE_BR"))
		{
			printf("%s: can not modify gre br\n", __FUNCTION__);
			return ZCFG_INVALID_PARAM_VALUE;
		}
		zcfgFeObjStructFree(brBdgObj);
	}
#endif


	IID_INIT(objIid);
	sscanf(mappingPathName, "Bridging.Bridge.%hhu.Port.%hhu", &objIid.idx[0], &objIid.idx[1]);
	objIid.level = 2;
	ret = zcfgFeObjJsonDel(RDM_OID_BRIDGING_BR_PORT, &objIid, NULL);
	if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY))
		printf("%s: Delete Bridging.Bridge.Port fail\n", __FUNCTION__);

	return ret;

	IID_INIT(mgmtObjIid);
	memcpy(&mgmtObjIid, &objIid, sizeof(objIndex_t));
	mgmtObjIid.idx[1] = 1;
	if((ret = zcfgFeObjStructGet(RDM_OID_BRIDGING_BR_PORT, &mgmtObjIid, (void **)&brBdgPtObj)) == ZCFG_SUCCESS){
		char bridgingPortName[30] = {0};
		char *modifiedLowerLayers = NULL;
		sprintf(bridgingPortName, "Bridging.Bridge.%hhu.Port.%hhu", objIid.idx[0], objIid.idx[1]);
		if((modifiedLowerLayers = csvStringStripTerm((const char *)brBdgPtObj->LowerLayers, (const char *)bridgingPortName))) {
			strcpy(brBdgPtObj->LowerLayers, modifiedLowerLayers);
			free(modifiedLowerLayers);
		}

		if((ret = zcfgFeObjStructSet(RDM_OID_BRIDGING_BR_PORT, &mgmtObjIid, (void *)brBdgPtObj, NULL)) != ZCFG_SUCCESS)
			printf("%s: Set Bridge Mgmt Port Object Fail\n", __FUNCTION__);

		zcfgFeObjStructFree(brBdgPtObj);
	}

	return ret;
}

zcfgRet_t l2BrVlanObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	char tmpChar[4] ={0};
	//char paramName[64] = {0};
	uint32_t  vlanOid = 0;
	objIndex_t vlanIid;
	struct json_object *vlanJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	uint8_t numOfInstance;
	char *ptr = NULL;
	//char zyxelPrefixParam[64] = {0};
#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	char tr98vlanPathName[128];
#endif

	printf("Enter %s\n", __FUNCTION__);

#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	ptr = strstr(tr98FullPathName, "Bridge.") + 7;
	if (*ptr < '4' && *ptr > '0'){ // Bridge.1 ~ Bridge.3
		if (getMappingVlanTermPathName(*ptr ,tr98vlanPathName) != ZCFG_SUCCESS)
			return ZCFG_NO_SUCH_OBJECT;
	}

	printf("tr98vlanPathName=%s\n",tr98vlanPathName);
	if((ret = zcfgFe98To181MappingNameGet(tr98vlanPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	   }
	else
		printf("TR181 object %s\n", mappingPathName);
#else
  if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
    }
	else
		printf("TR181 object %s\n", mappingPathName);
#endif

	ptr = strstr(mappingPathName, "VLANTermination.");
	strcpy(tmpChar, ptr+16);
	numOfInstance = atoi(tmpChar);

	IID_INIT(vlanIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	vlanOid = zcfgFeObjNameToObjId(tr181Obj, &vlanIid);
	if((ret = feObjJsonGet(vlanOid, &vlanIid, &vlanJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		paramValue = json_object_object_get(vlanJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
#ifdef SAAAD_TR98_CUSTOMIZATION
		if(!strcmp(paramList->name, "VLANName")) {
			paramValue = json_object_object_get(vlanJobj, "X_ZYXEL_VLANName");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "VLANEnable")) {
			paramValue = json_object_object_get(vlanJobj, "Enable");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			paramList++;
			continue;
		}

#endif

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(vlanJobj);
	return ZCFG_SUCCESS;
}

zcfgRet_t l2BrVlanObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  vlanOid = 0;
	objIndex_t vlanIid;
	struct json_object *vlanJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	//char zyxelPrefixParam[64] = {0};
#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	char tr98vlanPathName[128] = {0};
	char *ptr = NULL;
#endif

	printf("Enter %s\n", __FUNCTION__);

#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	ptr = strstr(tr98FullPathName, "Bridge.") + 7;
	if (*ptr < '4' && *ptr > '0'){ // Bridge.1 ~ Bridge.3
		if (getMappingVlanTermPathName(*ptr ,tr98vlanPathName) != ZCFG_SUCCESS)
			return ZCFG_NO_SUCH_OBJECT;
	}

	printf("tr98vlanPathName=%s\n",tr98vlanPathName);
	if((ret = zcfgFe98To181MappingNameGet(tr98vlanPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	   }
	else
		printf("TR181 object %s\n", mappingPathName);
#else
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else
		printf("TR181 object %s\n", mappingPathName);
#endif

	IID_INIT(vlanIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	vlanOid = zcfgFeObjNameToObjId(tr181Obj, &vlanIid);
	if((ret = zcfgFeObjJsonGet(vlanOid, &vlanIid, &vlanJobj)) != ZCFG_SUCCESS){
		return ret;
	}
	if(multiJobj){
		tmpObj = vlanJobj;
		vlanJobj = NULL;
		vlanJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ETH_VLAN_TERM, &vlanIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(vlanJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(vlanJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		/*special case*/
#ifdef SAAAD_TR98_CUSTOMIZATION
		if(!strcmp(paramList->name, "VLANName")){
			tr181ParamValue = json_object_object_get(vlanJobj, "X_ZYXEL_VLANName");
			if(tr181ParamValue != NULL) {
				json_object_object_add(vlanJobj, "X_ZYXEL_VLANName", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "VLANEnable")){
			tr181ParamValue = json_object_object_get(vlanJobj, "Enable");
			if(tr181ParamValue != NULL) {
				json_object_object_add(vlanJobj, "Enable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			paramList++;
			continue;
		}
#endif
		paramList++;
	} /*Edn while*/

	if(multiJobj)
		json_object_put(tmpObj);
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ETH_VLAN_TERM, &vlanIid, vlanJobj, NULL)) != ZCFG_SUCCESS ){
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Ethernet.VLANTermination.i Fail\n", __FUNCTION__);
			json_object_put(vlanJobj);
			return ret;
		}
		else
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Ethernet.VLANTermination.i Success\n", __FUNCTION__);

		json_object_put(vlanJobj);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t l2BrVlanObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;
#ifdef SAAAD_TR98_CUSTOMIZATION
	char tr98ObjNameTmp[128] = {0};
	char tmpBuf[128];
	char *ptr = NULL;
#endif

	printf("Enter %s\n", __FUNCTION__);


#ifdef SAAAD_TR98_CUSTOMIZATION
	printf("tr98FullPathName=%s\n", tr98FullPathName);

	//SAAAD, only allow one VLAN object in each bridge object
	//InternetGatewayDevice.Layer2Bridging.Bridge.i.VLAN.1
	if (tr98FullPathName[strlen(tr98FullPathName)-1] == '.')
		sprintf(tr98ObjNameTmp, "%s%d", tr98FullPathName, 1);
	else
		sprintf(tr98ObjNameTmp, "%s.%d", tr98FullPathName, 1);

#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	ptr = strstr(tr98ObjNameTmp, "Bridge.") + 7;
	if (*ptr < '4' && *ptr > '0'){ // Bridge.1 ~ Bridge.3
		if (getMappingVlanTermPathName(*ptr ,tr98ObjNameTmp) != ZCFG_SUCCESS)
			return ZCFG_INTERNAL_ERROR;
	}
#endif

	//if tr98ObjNameTmp already exist, return error.
	if (zcfgFe98To181MappingNameGet(tr98ObjNameTmp, tmpBuf) == ZCFG_SUCCESS)
		return ZCFG_INTERNAL_ERROR;
#endif

	/*add the object "Device.Bridging.Filter.i" */
	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_ETH_VLAN_TERM, &objIid, NULL)) != ZCFG_SUCCESS){
		printf("Add Instance Fail!!\n");
		return ret;
	}

#ifdef SAAAD_TR98_CUSTOMIZATION
	*idx = 1;
#else
	*idx = objIid.idx[0];
#endif

	return ZCFG_SUCCESS;
}

zcfgRet_t l2BrVlanObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mappingPathName[32] = {0};
	uint32_t  oid = 0;
	objIndex_t objIid;
#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	char tr98vlanPathName[128];
	char *ptr = NULL;
#endif

	printf("Enter %s\n", __FUNCTION__);

#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	ptr = strstr(tr98FullPathName, "Bridge.") + 7;
	if (*ptr < '4' && *ptr > '0'){ // Bridge.1 ~ Bridge.3
		if (getMappingVlanTermPathName(*ptr ,tr98vlanPathName) != ZCFG_SUCCESS)
			return ZCFG_NO_SUCH_OBJECT;
	}

	printf("tr98vlanPathName=%s\n",tr98vlanPathName);

	if((ret = zcfgFe98To181MappingNameGet(tr98vlanPathName, mappingPathName)) != ZCFG_SUCCESS)
		return ret;
#else
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS)
		return ret;
#endif

	IID_INIT(objIid);
	printf("mappingPathName %s\n", mappingPathName);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	oid = zcfgFeObjNameToObjId(tr181Obj, &objIid);

	ret = zcfgFeObjJsonDel(oid, &objIid, NULL);
	if(ret == ZCFG_SUCCESS)
		printf("Delete Object Success\n");
	else
		printf("Delete Object Fail\n");
		return ret;

	return ZCFG_SUCCESS;
}


zcfgRet_t l2BrVlanObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "VLANEnable");
			break;
		}

		if(strcmp(tr181ParamName, "X_ZYXEL_VLANName") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "VLANName");
			break;
		}

		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int l2BrVlanObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;
#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	char tr98vlanPathName[128] = {0};
	char *ptr = NULL;
	zcfgRet_t ret;
#endif

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	ptr = strstr(tr98FullPathName, "Bridge.") + 7;
	if (*ptr < '4' && *ptr > '0'){ // Bridge.1 ~ Bridge.3
		if (getMappingVlanTermPathName(*ptr ,tr98vlanPathName) != ZCFG_SUCCESS)
			return ZCFG_NO_SUCH_OBJECT;
	}

	printf("tr98vlanPathName=%s\n",tr98vlanPathName);
	if((ret = zcfgFe98To181MappingNameGet(tr98vlanPathName, mapObj)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else
		printf("TR181 object %s\n", mapObj);
#else
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
#endif

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
		if(strcmp(paramList->name, "VLANEnable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "VLANName") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_VLANName");
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

zcfgRet_t l2BrVlanObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;
#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	char tr98vlanPathName[128] = {0};
	char *ptr = NULL;
#endif

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

#ifdef SAAAD_BRIDGE_INDEX_CUSTOMIZATION
	ptr = strstr(tr98FullPathName, "Bridge.") + 7;
	if (*ptr < '4' && *ptr > '0'){ // Bridge.1 ~ Bridge.3
		if (getMappingVlanTermPathName(*ptr ,tr98vlanPathName) != ZCFG_SUCCESS)
			return ZCFG_NO_SUCH_OBJECT;
	}

	printf("tr98vlanPathName=%s\n",tr98vlanPathName);
	if((ret = zcfgFe98To181MappingNameGet(tr98vlanPathName, mapObj)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else
		printf("TR181 object %s\n", mapObj);
#else
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
#endif

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
		if(strcmp(paramList->name, "VLANEnable") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "VLANName") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "X_ZYXEL_VLANName");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}

		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if(strcmp(paramList->name, "VLANEnable") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Enable", attrValue);
		}else if(strcmp(paramList->name, "VLANName") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "X_ZYXEL_VLANName", attrValue);
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

