#include <json/json.h>
#include <ctype.h>
#include <sys/stat.h>
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
#include "zcfg_webgui_flag.h"
#include "zyExt_parameter.h"

#if ZYXEL_TR69_REMOTE_MANAGE_SEPARATE_PARAMETER
#define REMOET_LAN (0x1 << 0)
#define REMOET_WLAN (0x1 << 1)
#define REMOET_WAN (0x1 << 2)
#define REMOET_TSTDM (0x1 << 3)
#define MODE_LAN_withoutWL 1
#define MODE_WL_ONLY 2
#define MODE_LAN_ONLY 3
#define MODE_WAN_ONLY 4
#define MODE_LAN_withoutWL_WAN 5
#define MODE_WL_WAN 6
#define MODE_LAN_WAN 7
#define MODE_Trust_Dm 8
#define MODE_LAN_withoutWL_TstDm 9
#define MODE_WL_Trust_Dm 10
#define MODE_LAN_TstDm 11

#endif

extern tr98Object_t tr98Obj[];

//InternetGatewayDevice.X_ZYXEL_EXT.
zcfgRet_t zyExtObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
#if 0
	char mappingPathName[128] = {0};

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}

	printf("%s: %s\n", __FUNCTION__, mappingPathName);
#endif

	struct json_object *extObj = NULL, *paramObj;
	objIndex_t objIid;
	IID_INIT(objIid);
	zcfgRet_t rst = ZCFG_NO_SUCH_OBJECT;

	if((rst = zcfgFeObjJsonGet(RDM_OID_ZY_EXT, &objIid, &extObj)) != ZCFG_SUCCESS){
		printf("%s: retrieve X_ZYXEL_EXT object fail\n", __FUNCTION__);

		return rst;
	}

	struct tr98Parameter_s *parmList = tr98Obj[handler].parameter;
	if(!parmList){
		return ZCFG_NO_SUCH_OBJECT;
	}

	*tr98Jobj = json_object_new_object();
	while(parmList->name){
		paramObj = json_object_object_get(extObj, parmList->name);
		if(paramObj){
			json_object_object_add(*tr98Jobj, parmList->name, JSON_OBJ_COPY(paramObj));
		}
		parmList++;
	}

	json_object_put(extObj);

	return ZCFG_SUCCESS;
}


//InternetGatewayDevice.X_ZYXEL_EXT.
zcfgRet_t zyExtObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj )
{
	struct json_object *extObj = NULL, *paramObj;
	objIndex_t objIid;
	IID_INIT(objIid);

	printf("%s\n", __FUNCTION__);

	struct tr98Parameter_s *parmList = tr98Obj[handler].parameter;
	if(!parmList){
		return ZCFG_NO_SUCH_OBJECT;
	}

	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	extObj = json_object_new_object();

	while(parmList->name){
		paramObj = json_object_object_get(tr98Jobj, parmList->name);
		if(paramObj){
			json_object_object_add(extObj, parmList->name, JSON_OBJ_COPY(paramObj));
		}
		parmList++;
	}

	if(multiJobj){
		zcfgFeJsonMultiObjAppend(RDM_OID_ZY_EXT, &objIid, multiJobj, extObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ZY_EXT, &objIid, "partialset", json_object_new_boolean(true));
	}else{
		zcfgRet_t rst = zcfgFeObjJsonSet(RDM_OID_ZY_EXT, &objIid, extObj, NULL);
		json_object_put(extObj);

		return rst;
	}
	json_object_put(extObj);

	return ZCFG_SUCCESS;

}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.DNSRouteEntry.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.DNSRouteEntry.i
 */
zcfgRet_t zyExtDnsRtObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dnsRouteJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.DNSRouteEntry.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_DNS_RT_ENTRY, &objIid, &dnsRouteJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dnsRouteJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(dnsRouteJobj);

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.DNSRouteEntry.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.DNSRouteEntry.i
 */
zcfgRet_t zyExtDnsRtObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dnsRouteJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.DNSRouteEntry.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS_RT_ENTRY, &objIid, &dnsRouteJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = dnsRouteJobj;
		dnsRouteJobj = NULL;
		dnsRouteJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_RT_ENTRY, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(dnsRouteJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(dnsRouteJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_RT_ENTRY, &objIid, dnsRouteJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(dnsRouteJobj);
			return ret;
		}
		json_object_put(dnsRouteJobj);
	}

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.DNSRouteEntry.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.DNSRouteEntry.i
 */
zcfgRet_t zyExtDnsRtObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_DNS_RT_ENTRY, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.DNSRouteEntry.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.DNSRouteEntry.i
 */
zcfgRet_t zyExtDnsRtObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.DNSRouteEntry.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_DNS_RT_ENTRY, &objIid, NULL);;
}

zcfgRet_t zyExtDnsRtObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtDnsRtObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

zcfgRet_t zyExtDnsRtObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.DNSEntry.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.DNSEntry.i
 */
zcfgRet_t zyExtDnsEntryObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dnsJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.DNSEntry.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_DNS_ENTRY, &objIid, &dnsJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dnsJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(dnsJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtDnsEntryObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dnsJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.DNSEntry.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS_ENTRY, &objIid, &dnsJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = dnsJobj;
		dnsJobj = NULL;
		dnsJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_ENTRY, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(dnsJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(dnsJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_ENTRY, &objIid, dnsJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(dnsJobj);
			return ret;
		}
		json_object_put(dnsJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtDnsEntryObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_DNS_ENTRY, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyExtDnsEntryObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.DNSEntry.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_DNS_ENTRY, &objIid, NULL);;
}

#if 0
zcfgRet_t zyExtDnsEntryObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtDnsEntryObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  dnsOid = 0;
	objIndex_t dnsIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(dnsIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	dnsOid = zcfgFeObjNameToObjId(tr181Obj, &dnsIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(dnsOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtDnsEntryObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  dnsOid = 0;
	objIndex_t dnsIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(dnsIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	dnsOid = zcfgFeObjNameToObjId(tr181Obj, &dnsIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(dnsOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(dnsOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, dnsOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}
#endif
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.DynamicDNS
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.DynamicDNS
 */
 zcfgRet_t zyExtDDnsObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ddnsJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_D_DNS, &objIid, &ddnsJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(ddnsJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(ddnsJobj);

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.DynamicDNS
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.DynamicDNS
 */
zcfgRet_t zyExtDDnsObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ddnsJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_D_DNS, &objIid, &ddnsJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = ddnsJobj;
		ddnsJobj = NULL;
		ddnsJobj = zcfgFeJsonMultiObjAppend(RDM_OID_D_DNS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(ddnsJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ddnsJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_D_DNS, &objIid, ddnsJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(ddnsJobj);
			return ret;
		}
		json_object_put(ddnsJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtDDnsObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtDDnsObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

zcfgRet_t zyExtDDnsObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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

#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.Gpon_Info
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_Gpon_Info
 */
 zcfgRet_t zyExtGponInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *gponInfoJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_GPON_INFO, &objIid, &gponInfoJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(gponInfoJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
		/*special case*/
		if(!strcmp(paramList->name, "Enable")) {
			if(gponInfoJobj != NULL) {
				paramValue = json_object_object_get(gponInfoJobj, "SFP_Presence");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}

				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
				paramList++;
				continue;
			}
		}else if(!strcmp(paramList->name, "VendorName")) {
			if(gponInfoJobj != NULL) {
				paramValue = json_object_object_get(gponInfoJobj, "SFP_Vendor");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}

				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
				paramList++;
				continue;
			}
		}else if(!strcmp(paramList->name, "VendorSerial")) {
			if(gponInfoJobj != NULL) {
				paramValue = json_object_object_get(gponInfoJobj, "SFP_Serial");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}

				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
				paramList++;
				continue;
		}
		}else if(!strcmp(paramList->name, "Model")) { //SFP_Model
			if(gponInfoJobj != NULL) {
				paramValue = json_object_object_get(gponInfoJobj, "SFP_Model");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}

				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
				paramList++;
				continue;
			}
		}
#endif//ZYXEL_WIND_ITALY_CUSTOMIZATION

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(gponInfoJobj);

	return ZCFG_SUCCESS;
}
#endif

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.System_Info
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_System_Info
 */
 zcfgRet_t zyExtSysInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *sysinfoJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_SYSTEM_INFO, &objIid, &sysinfoJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(sysinfoJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(sysinfoJobj);

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.System_Info
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_System_Info
 */
zcfgRet_t zyExtSysInfoObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *sysinfoJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_SYSTEM_INFO, &objIid, &sysinfoJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = sysinfoJobj;
		sysinfoJobj = NULL;
		sysinfoJobj = zcfgFeJsonMultiObjAppend(RDM_OID_SYSTEM_INFO, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(sysinfoJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(sysinfoJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_SYSTEM_INFO, &objIid, sysinfoJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(sysinfoJobj);
			return ret;
		}
		json_object_put(sysinfoJobj);
	}
#ifdef CAAAB_CUSTOMIZATION
        bool isCAAABAccountCreated = false;
        struct stat ext;
        if(!stat("/var/CBT_Account", &ext) ){
                isCAAABAccountCreated = true;
        }

        rdm_ZyLogCfgGpAccount_t *accountObj = NULL;
        objIndex_t logAccountObjIid;
        IID_INIT(logAccountObjIid);
        char username[64] = {},password[64] = {},default_password[64] = {};
        while(zcfgFeObjJsonGetNext(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &logAccountObjIid, &accountObj) == ZCFG_SUCCESS) {
                if(json_object_get_string(json_object_object_get(accountObj, "Username")) != NULL ){
                        strcpy(username,json_object_get_string(json_object_object_get(accountObj, "Username")) );
                        strcpy(password,json_object_get_string(json_object_object_get(accountObj, "Password")) );
                        strcpy(default_password,json_object_get_string(json_object_object_get(accountObj, "DefaultPassword")) );
                }
                if(isCAAABAccountCreated){
					if(strcmp(username,"root") && strcmp(username,"supervisor") && strcmp(username,"admin")){
							if(zcfgFeObjJsonDel(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &logAccountObjIid, NULL) != ZCFG_SUCCESS){
									printf("%s(%d): fail to delete RDM_OID_ZY_LOG_CFG_GP_ACCOUNT",__FUNCTION__,__LINE__);
							}
					}
					if(!strcmp(username,"admin")){
							if(strcmp(password,default_password)){
									json_object_object_add(accountObj, "Password",json_object_new_string(default_password));
									if(zcfgFeObjJsonSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &logAccountObjIid, accountObj, NULL) != ZCFG_SUCCESS){
											printf("%s(%d): fail to set admin password",__FUNCTION__,__LINE__);
									}
							}
					}
					unlink("/var/CBT_Account");
                }
                zcfgFeJsonObjFree(accountObj);
        }
        
        
	rdm_RemoSrv_t *srvObj = NULL;
	objIndex_t srvObjIid;
	IID_INIT(srvObjIid);
	char srvName[32] = {},srvMode[32] = {};
			
	while(zcfgFeObjJsonGetNext(RDM_OID_REMO_SRV, &srvObjIid, &srvObj) == ZCFG_SUCCESS) {
		if(json_object_get_string(json_object_object_get(srvObj, "Name")) != NULL ){
			strcpy(srvName,json_object_get_string(json_object_object_get(srvObj, "Name")) );
			strcpy(srvMode,json_object_get_string(json_object_object_get(srvObj, "Mode")) );
		}
		if(!strcmp(srvName,"HTTP") || !strcmp(srvName,"HTTPS")){
			json_object_object_add(srvObj, "Mode",json_object_new_string("LAN_ONLY"));
			if(zcfgFeObjJsonSet(RDM_OID_REMO_SRV, &srvObjIid, srvObj, NULL) != ZCFG_SUCCESS){
				printf("%s(%d): fail to set RDM_OID_REMO_SRV",__FUNCTION__,__LINE__);
			}
		}
		zcfgFeJsonObjFree(srvObj);
	}
	unlink("/var/TR069SetMGMT");
#endif // 1
	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.System_Info
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_System_Info
 */
zcfgRet_t zyExtSysInfoObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.System_Info
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_System_Info
 */
int zyExtSysInfoObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.System_Info
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_System_Info
 */
zcfgRet_t zyExtSysInfoObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.TR064
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_TR064
 */
 zcfgRet_t zyExtTr064ObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *tr064Jobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_TR064, &objIid, &tr064Jobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(tr064Jobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(tr064Jobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.TR064
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_TR064
 */
zcfgRet_t zyExtTr064ObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *tr064Jobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_TR064, &objIid, &tr064Jobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = tr064Jobj;
		tr064Jobj = NULL;
		tr064Jobj = zcfgFeJsonMultiObjAppend(RDM_OID_TR064, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(tr064Jobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(tr064Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_TR064, &objIid, tr064Jobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(tr064Jobj);
			return ret;
		}
		json_object_put(tr064Jobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.TR064
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_TR064
 */
zcfgRet_t zyExtTr064ObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.TR064
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_TR064
 */
int zyExtTr064ObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  tr064Oid = 0;
	objIndex_t tr064Iid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(tr064Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	tr064Oid = zcfgFeObjNameToObjId(tr181Obj, &tr064Iid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(tr064Oid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.TR064
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_TR064
 */
zcfgRet_t zyExtTr064ObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  tr064Oid = 0;
	objIndex_t tr064Iid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(tr064Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	tr064Oid = zcfgFeObjNameToObjId(tr181Obj, &tr064Iid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(tr064Oid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(tr064Oid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, tr064Oid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_VlanGroup.i
 */
 zcfgRet_t zyExtVlanGroupObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *vlanJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_VLAN_GROUP, &objIid, &vlanJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(vlanJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(vlanJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_VlanGroup.i
 */
zcfgRet_t zyExtVlanGroupObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *vlanJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_VLAN_GROUP, &objIid, &vlanJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = vlanJobj;
		vlanJobj = NULL;
		vlanJobj = zcfgFeJsonMultiObjAppend(RDM_OID_VLAN_GROUP, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(vlanJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(vlanJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &objIid, vlanJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(vlanJobj);
			return ret;
		}
		json_object_put(vlanJobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_VlanGroup.i
 */
zcfgRet_t zyExtVlanGroupObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_VLAN_GROUP, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_VlanGroup.i
 */
zcfgRet_t zyExtVlanGroupObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_VLAN_GROUP, &objIid, NULL);;
}
#endif // CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP

#if 0
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_VlanGroup.i
 */
zcfgRet_t zyExtVlanGroupObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_VlanGroup.i
 */
int zyExtVlanGroupObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  vlanOid = 0;
	objIndex_t vlanIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(vlanIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	vlanOid = zcfgFeObjNameToObjId(tr181Obj, &vlanIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(vlanOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_VlanGroup.i
 */
zcfgRet_t zyExtVlanGroupObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  vlanOid = 0;
	objIndex_t vlanIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(vlanIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	vlanOid = zcfgFeObjNameToObjId(tr181Obj, &vlanIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(vlanOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(vlanOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, vlanOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}
#endif
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailService.i
 */
 zcfgRet_t zyExtEmailSrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *emailJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_MAIL_SERVICE, &objIid, &emailJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(emailJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(emailJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailService.i
 */
zcfgRet_t zyExtEmailSrvObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *emailJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_MAIL_SERVICE, &objIid, &emailJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = emailJobj;
		emailJobj = NULL;
		emailJobj = zcfgFeJsonMultiObjAppend(RDM_OID_MAIL_SERVICE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(emailJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(emailJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MAIL_SERVICE, &objIid, emailJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(emailJobj);
			return ret;
		}
		json_object_put(emailJobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailService.i
 */
zcfgRet_t zyExtEmailSrvObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_MAIL_SERVICE, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailService.i
 */
zcfgRet_t zyExtEmailSrvObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_MAIL_SERVICE, &objIid, NULL);;
}
#if 0
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailService.i
 */
zcfgRet_t zyExtEmailSrvObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailService.i
 */
int zyExtEmailSrvObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  emailOid = 0;
	objIndex_t emailIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(emailIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	emailOid = zcfgFeObjNameToObjId(tr181Obj, &emailIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(emailOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailService.i
 */
zcfgRet_t zyExtEmailSrvObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  emailOid = 0;
	objIndex_t emailIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(emailIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	emailOid = zcfgFeObjNameToObjId(tr181Obj, &emailIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(emailOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(emailOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, emailOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}
#endif
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailEventConfig.i
 */
 zcfgRet_t zyExtEmailEventObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *emailJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_MAIL_EVENT_CFG, &objIid, &emailJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(emailJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(emailJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailEventConfig.i
 */
zcfgRet_t zyExtEmailEventObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *emailJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_MAIL_EVENT_CFG, &objIid, &emailJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = emailJobj;
		emailJobj = NULL;
		emailJobj = zcfgFeJsonMultiObjAppend(RDM_OID_MAIL_EVENT_CFG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(emailJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(emailJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MAIL_EVENT_CFG, &objIid, emailJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(emailJobj);
			return ret;
		}
		json_object_put(emailJobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailEventConfig.i
 */
zcfgRet_t zyExtEmailEventObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_MAIL_EVENT_CFG, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailEventConfig.i
 */
zcfgRet_t zyExtEmailEventObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_MAIL_EVENT_CFG, &objIid, NULL);;
}
#if 0
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailEventConfig.i
 */
zcfgRet_t zyExtEmailEventObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailEventConfig.i
 */
int zyExtEmailEventObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  emailOid = 0;
	objIndex_t emailIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(emailIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	emailOid = zcfgFeObjNameToObjId(tr181Obj, &emailIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(emailOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_EmailNotification.EmailEventConfig.i
 */
zcfgRet_t zyExtEmailEventObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  emailOid = 0;
	objIndex_t emailIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(emailIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	emailOid = zcfgFeObjNameToObjId(tr181Obj, &emailIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(emailOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(emailOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, emailOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP
 */
 zcfgRet_t zyExtSnmpObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *snmpJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_SNMP, &objIid, &snmpJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(snmpJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(snmpJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP
 */
zcfgRet_t zyExtSnmpObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *snmpJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_SNMP, &objIid, &snmpJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = snmpJobj;
		snmpJobj = NULL;
		snmpJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_SNMP, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(snmpJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(snmpJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_SNMP, &objIid, snmpJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(snmpJobj);
			return ret;
		}
		json_object_put(snmpJobj);
	}

	return ZCFG_SUCCESS;
}
#endif

#if 0
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP
 */
zcfgRet_t zyExtSnmpObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP
 */
int zyExtSnmpObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP
 */
zcfgRet_t zyExtSnmpObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP.TrapAddr.i
 */
zcfgRet_t zyExtSnmpTrapObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *SnmpTrapJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.%hhu", &objIid.idx[0]);
	
	if((ret = feObjJsonGet(RDM_OID_TRAP_ADDR, &objIid, &SnmpTrapJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(SnmpTrapJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(SnmpTrapJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP.TrapAddr.i
 */
zcfgRet_t zyExtSnmpTrapObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *SnmpTrapJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_TRAP_ADDR, &objIid, &SnmpTrapJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = SnmpTrapJobj;
		SnmpTrapJobj = NULL;
		SnmpTrapJobj = zcfgFeJsonMultiObjAppend(RDM_OID_TRAP_ADDR, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(SnmpTrapJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(SnmpTrapJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_TRAP_ADDR, &objIid, SnmpTrapJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(SnmpTrapJobj);
			return ret;
		}
		json_object_put(SnmpTrapJobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP.TrapAddr.i
 */
zcfgRet_t zyExtSnmpTrapObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_TRAP_ADDR, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP.TrapAddr.i
 */
zcfgRet_t zyExtSnmpTrapObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_TRAP_ADDR, &objIid, NULL);;
}
#endif

#if 0
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP.TrapAddr.i
 */
zcfgRet_t zyExtSnmpTrapObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP.TrapAddr.i
 */
int zyExtSnmpTrapObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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
		
		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.X_ZYXEL_SNMP.TrapAddr.i
 */
zcfgRet_t zyExtSnmpTrapObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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
	}
	
	return ret;
}
#endif
#if defined(IAAAA_CUSTOMIZATION) || defined(ZYXEL_SP_DOMAIN_TIMER) || defined(GFIBER_CUSTOMIZATION)
zcfgRet_t zyExtRemoteMgmtObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	rdm_RemoSrv_t *remoSrv = NULL;
#ifdef IAAAA_CUSTOMIZATION
	rdm_IpIface_t *ipIface = NULL;
	char boundWanInterface[256] = {0};
	char boundWanInterfaceName[256] = {0};
	char *pc1, *pc2;
#endif

	tr98Parameter_t *paramList = NULL;
	struct json_object *paramValue = NULL;
	printf("%s : Enter\n", __FUNCTION__);
#ifdef IAAAA_CUSTOMIZATION
	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 1;

	if ((ret = feObjStructGet(RDM_OID_REMO_SRV, &objIid,  (void **)&remoSrv, updateFlag)) != ZCFG_SUCCESS)
		return ret;
	strcpy(boundWanInterface, remoSrv->BoundInterfaceList);
	zcfgFeObjStructFree(remoSrv);

	if (strlen(boundWanInterface)) {
		pc2 = &boundWanInterface[0];
		do {
			pc1 = pc2;
			pc2 = strstr(pc1, ",");
			if (pc2 != NULL) {
				*pc2 = 0;
				++pc2;
			}
			IID_INIT(objIid);
			objIid.level = 1;
			sscanf(pc1, "IP.Interface.%hhu", &objIid.idx[0]);
			if  ((ret = feObjStructGet(RDM_OID_IP_IFACE, &objIid,  (void **)&ipIface, updateFlag)) == ZCFG_SUCCESS) {
				strcat(boundWanInterfaceName, ipIface->X_ZYXEL_SrvName);
				strcat(boundWanInterfaceName, ",");
				zcfgFeObjStructFree(ipIface);
			}
			if (pc2 == NULL || pc2[0] == 0)
				break;
		} while (1);
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if (strcmp("BoundWANInterfaceList", paramList->name) == 0) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(boundWanInterfaceName));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
#else
		/*Get related tr181 objects*/
		IID_INIT(objIid);
		if((ret = feObjJsonGet(RDM_OID_REMO_MGMT, &objIid, &remoSrv, updateFlag)) != ZCFG_SUCCESS)
			return ret;

		/*fill up tr98 object from related tr181 objects*/
		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			/*Write parameter value from tr181 objects to tr98 object*/
			paramValue = json_object_object_get(remoSrv, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*Not defined in tr181, give it a default value*/
			printf("Can't find parameter %s in TR181\n", paramList->name);
			paramList++;
		}
		json_object_put(remoSrv);
#endif

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtRemoteMgmtObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMgmtJobj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
#ifdef IAAAA_CUSTOMIZATION
	char boundWanInterface[256] = {0};
    char boundWanInterfaceTmp[256] = {0};
	char boundWanInterfaceName[256] = {0};
#endif

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_REMO_MGMT, &objIid, &RemoteMgmtJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = RemoteMgmtJobj;
		RemoteMgmtJobj = NULL;
		RemoteMgmtJobj = zcfgFeJsonMultiObjAppend(RDM_OID_REMO_MGMT, &objIid, multiJobj, tmpObj);
	}
#ifdef IAAAA_CUSTOMIZATION
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if (strcmp("BoundWANInterfaceList", paramList->name) == 0) {
				rdm_IpIface_t *ipIface = NULL;
				char *pc1 = NULL, *pc2 = NULL;
				strcpy(boundWanInterfaceName, json_object_to_json_string(paramValue));
				pc2 = boundWanInterfaceName;
				if (*pc2 == '"') ++pc2;
				do {
					pc1 = pc2;
					// skip space char
					while (*pc1 == ' ')
						++pc1;
					// skip ,
					pc2 = strstr(pc1, ",");
					if (pc2 != NULL) {
						*pc2 = 0;
						++pc2;
						if (*pc2 == '"') *pc2 = 0;
					} else {
						// string end with "
						pc2 = strstr(pc1, "\"");
						if (pc2 != NULL) *pc2 = 0;
					}
					if (strlen(pc1) != 0) {
						IID_INIT(objIid);
						objIid.level = 1;
						while ((ret = feObjStructGetNext(RDM_OID_IP_IFACE, &objIid,  (void **)&ipIface, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) {
							if (strcmp(pc1, ipIface->X_ZYXEL_SrvName) == 0) {
								sprintf(boundWanInterfaceTmp, "%sIP.Interface.%d,", boundWanInterface, objIid.idx[0]);
                                sprintf(boundWanInterface, "%s", boundWanInterfaceTmp);
                                memset(boundWanInterfaceTmp, 0, sizeof(boundWanInterfaceTmp));
								zcfgFeObjStructFree(ipIface);
								break;
							}
							zcfgFeObjStructFree(ipIface);
						}
						if (ret != ZCFG_SUCCESS) {
							printf("can not find interface %s\n", pc1);
							json_object_put(RemoteMgmtJobj);
							return ret;
						}
					}
					if (pc2 == NULL || pc2[0] == 0)
						break;
				} while (1);
				json_object_object_add(RemoteMgmtJobj, paramList->name, json_object_new_string(boundWanInterface));
			}
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
#else
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(RemoteMgmtJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(RemoteMgmtJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
#endif

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else {
		if((ret = zcfgFeObjJsonSet(RDM_OID_REMO_MGMT, &objIid, RemoteMgmtJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(RemoteMgmtJobj);
			return ret;
		}
		json_object_put(RemoteMgmtJobj);
	}

	return ZCFG_SUCCESS;
}
#endif

#ifdef GFIBER_CUSTOMIZATION
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_RemoteManagement.
 */
zcfgRet_t zyExtRemoteMgmtObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtRemoteMgmtObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

        attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
        if(attrValue < 0 ) {
            attrValue = 0;
        }
        break;
    }

    return attrValue;
}

zcfgRet_t zyExtRemoteMgmtObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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

int getCurrentUptime(void){
	FILE * uptimefile = NULL;
	char uptime_chr[32] = {};
	char tmp[32] ={0};
	int uptime = -1;
	int i = 0;

	if((uptimefile = fopen("/proc/uptime", "r")) == NULL){
		printf("%s(%d): fail to get Sys Uptime.",__FUNCTION__,__LINE__);
		return uptime;
	}

	 fgets(uptime_chr, sizeof(uptime_chr), uptimefile);
	 fclose(uptimefile);
	
	while(i < strlen(uptime_chr)){
		if (*(uptime_chr+i) != '.'){
			*(tmp+i) = *(uptime_chr+i);
		}else if(*(uptime_chr+i) == '.'){
			break;
		}
		i++;
	}
	uptime = atoi(tmp);
	
	return uptime;
}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
/* InternetGatewayDevice.DeviceConfig */
zcfgRet_t devConfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	char tmpconfig[64*1024]="";
        char tempconfig[64*1024]="";
	char tmpstr[32*1024]="";
	FILE *fp;
	int bytes;
	int i = 0;
	
	printf("%s : Enter\n", __FUNCTION__);

	if(zcfgFeConfBackup("root") != ZCFG_SUCCESS){
		printf("backup restore: fail to backup.\n");
		return false;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "ConfigFile")) {
			system("tar -zcf /tmp/zcfg_config.tgz /tmp/zcfg_config.json");

			fp = fopen("/tmp/zcfg_config.tgz", "r");
			if (fp == NULL){
				strcpy(tmpconfig, "NONE");
			}
			else{
				bytes = fread(tmpstr, sizeof(char), 32*1024,fp);

				for(i = 0; i < bytes; i++)
				{
                                        strcpy(tempconfig,tmpconfig);
					sprintf(tmpconfig, "%s%02x", tempconfig, tmpstr[i]);
				}
				if(bytes<32*1024)
					tmpconfig[2*bytes]='\0';
			}
			fclose(fp);
			fp = NULL;

			system("rm -f  /tmp/zcfg_config.tgz");
			system("rm -f  /tmp/zcfg_config.encrypt");
			system("rm -f  /tmp/zcfg_config.json");
			
			printf("devConfObjGet; configfile len=%d\n", strlen(tmpconfig));
			//printf("devConfObjGet; configfile =%s\n", tmpconfig);
			
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tmpconfig));
			paramList++;
			continue;
		}
		else
		{		
			strcpy(tmpconfig, "NONE");
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tmpconfig));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.DeviceConfig */
zcfgRet_t devConfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *pstr = NULL;
	int i = 0;
	FILE *fp;
	char tmpvalue[8] = {0};
	char *value = NULL;
	char cmdContent[1024] = {0};
	uint8_t inttmpNum = 0;
	char buf[128] = {0};
	char Encryptpasswd[16] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "ConfigFile")) {
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			if(paramValue != NULL){
				value = json_object_get_string(paramValue);

				printf("configFile len=%d\n",  strlen(value));
				printf("paramValue=%s\n",  value);
				
				if(strlen(value) > 64*1024)
				{
					printf("configFile is large than 32k\n");
					return ZCFG_INVALID_PARAM_VALUE;
				}
				else if( (strlen(value) == 0) || ((strlen(value)%2)  != 0))
				{
					printf("configFile is not valid\n");
					return ZCFG_INVALID_PARAM_VALUE;
				}
				else
				{
					fp = fopen("/tmp/zcfg_config.tgz", "w");
					if(fp) 
					{
						pstr = value;
						for(i = 0; i < strlen(value)/2; i++)
						{
							strncpy(tmpvalue, pstr, 2);
							sscanf(tmpvalue, "%hhx", &inttmpNum);
							fwrite(&inttmpNum, 1, 1, fp);
							pstr = pstr + 2;
						}
						fclose(fp);

						system("tar -zxf /tmp/zcfg_config.tgz -C /tmp/");
						system("cp /tmp/tmp/zcfg_config.json /tmp/");

						/* encrypt /tmp/zcfg_config.json into /tmp/zcfg_config.encrypt */
						Encryptpasswd[0] = 'N';
						Encryptpasswd[2] = 'z';
						Encryptpasswd[4] = 'y';
						Encryptpasswd[1] = '3';
						Encryptpasswd[3] = '0';
						Encryptpasswd[5] = '9';
						Encryptpasswd[6] = '3';
						Encryptpasswd[7] = '\0';

						sprintf(cmdContent, "openssl enc -e -des3 -pass pass:%s -in /tmp/zcfg_config.json -out /tmp/zcfg_config.encrypt", Encryptpasswd);
						system(cmdContent);
						
						sprintf(cmdContent, "mv /tmp/zcfg_config.encrypt /var/restoreConfig");
						system(cmdContent);

						sprintf(buf, "/var/restoreConfig,root");
						
						if(zcfgFeRestoreDeCheck(buf) == ZCFG_SUCCESS)
						{
							printf("SPV InternetGatewayDevice.DeviceConfig: parse data OK, start to restore from config...\n");

							sprintf(cmdContent, "rm -rf /tmp/configfilecheck");
							system(cmdContent);
							
							sprintf(cmdContent, "echo 1 > /tmp/configfilecheck");
							system(cmdContent);

							/* delay do restore */
							//zcfgFeRestoreDefault(buf);
						}
						else
						{
							printf("SPV InternetGatewayDevice.DeviceConfig: fail to parse data !!!\n");
							return ZCFG_INVALID_PARAM_VALUE;
						}						
					}		
				}
				
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

		  
	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.DeviceConfig */
int devConfAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t DeviceConfigOid = 0;
	objIndex_t DeviceConfigIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(DeviceConfigIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	DeviceConfigOid = zcfgFeObjNameToObjId(tr181Obj, &DeviceConfigIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(DeviceConfigOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}


/* InternetGatewayDevice.DeviceConfig */
/* all use the batch set, only tr069 use this action*/
zcfgRet_t devConfObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t DeviceConfigOid = 0;
	objIndex_t DeviceConfigIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(DeviceConfigIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	DeviceConfigOid = zcfgFeObjNameToObjId(tr181Obj, &DeviceConfigIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(DeviceConfigOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(DeviceConfigOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, DeviceConfigOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

#endif



#if 1 // remote access
zcfgRet_t zyExtRemoteMgmtSrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMgmtSrvJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
#if ZYXEL_TR69_REMOTE_MANAGE_SEPARATE_PARAMETER
	const char *paramStr = NULL;
	const char *paramName = NULL;
	const char *substr =NULL;
#endif

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_REMO_SRV, &objIid, &RemoteMgmtSrvJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
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

	//printf("%s paramList->name = %s \n", __FUNCTION__, paramList->name);
#if ZYXEL_TR69_REMOTE_MANAGE_SEPARATE_PARAMETER //Louis
	if(!(strcmp(paramList->name,"LAN")))
	{	
		/*get InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.%hhu.Mode*/
		paramValue = json_object_object_get(RemoteMgmtSrvJobj, "Mode");
			if(paramValue != NULL) 
			{
				paramStr = json_object_get_string(paramValue);
				if (!strcmp(paramStr,"LAN_withoutWL")||!strcmp(paramStr,"LAN_withoutWL_WAN")||!strcmp(paramStr,"LAN_withoutWL_TstDm")
					||!strcmp(paramStr,"LAN_ONLY")||!strcmp(paramStr,"LAN_WAN")||!strcmp(paramStr,"LAN_TstDm"))
				{
					json_object_object_add(*tr98Jobj, "LAN", json_object_new_boolean(1));	
				}
				else{
					json_object_object_add(*tr98Jobj, "LAN", json_object_new_boolean(0));
				}

				paramList++;
				continue;
			}
		}
		else if(!(strcmp(paramList->name,"WLAN")))
		{
			/*get InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.%hhu.Mode*/
			paramValue = json_object_object_get(RemoteMgmtSrvJobj, "Mode");
			if(paramValue != NULL) 
			{
				paramStr = json_object_get_string(paramValue);
				if (!strcmp(paramStr,"WL_WAN")||!strcmp(paramStr,"WL_Trust_Dm")||!strcmp(paramStr,"WL_ONLY")
					||!strcmp(paramStr,"LAN_ONLY")||!strcmp(paramStr,"LAN_WAN")||!strcmp(paramStr,"LAN_TstDm"))
				{
					json_object_object_add(*tr98Jobj, "WLAN", json_object_new_boolean(1));	
				}
				else{
					json_object_object_add(*tr98Jobj, "WLAN", json_object_new_boolean(0));
				}
				paramList++;
				continue;
			}
		}
		else if(!(strcmp(paramList->name,"WAN")))
		{
			/*get InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.%hhu.Mode*/
			paramValue = json_object_object_get(RemoteMgmtSrvJobj, "Mode");
			if(paramValue != NULL) 
			{
				paramStr = json_object_get_string(paramValue);
				if (!strcmp(paramStr,"LAN_withoutWL_WAN")||!strcmp(paramStr,"WL_WAN")||!strcmp(paramStr,"LAN_WAN")
					||!strcmp(paramStr,"WAN_ONLY"))
				{
					json_object_object_add(*tr98Jobj, "WAN", json_object_new_boolean(1));	
				}
				else{
					json_object_object_add(*tr98Jobj, "WAN", json_object_new_boolean(0));
				}
				paramList++;
				continue;
			}
		}
		else if(!(strcmp(paramList->name,"TrustDomain")))
		{
			/*get InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.%hhu.Mode*/
			paramValue = json_object_object_get(RemoteMgmtSrvJobj, "Mode");
			if(paramValue != NULL) 
			{
				paramStr = json_object_get_string(paramValue);
				if (!strcmp(paramStr,"LAN_withoutWL_TstDm")||!strcmp(paramStr,"WL_Trust_Dm")||!strcmp(paramStr,"Trust_Dm")
					||!strcmp(paramStr,"LAN_TstDm"))
				{
					json_object_object_add(*tr98Jobj, "TrustDomain", json_object_new_boolean(1));	
				}
				else{
					json_object_object_add(*tr98Jobj, "TrustDomain", json_object_new_boolean(0));
				}
				paramList++;
				continue;
			}
		}
		else
		{
#endif
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(RemoteMgmtSrvJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#if ZYXEL_TR69_REMOTE_MANAGE_SEPARATE_PARAMETER
		}
#endif
		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(RemoteMgmtSrvJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtRemoteMgmtSrvObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMgmtSrvJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	bool isLanWan = false,isHttpService = false;
#ifdef IAAAA_CUSTOMIZATION
	struct json_object *rmsObj=NULL;
	char pv[256] = {0};
	objIndex_t rmsIid;
	bool bil = false;
#endif

#if ZYXEL_TR69_REMOTE_MANAGE_SEPARATE_PARAMETER
	int mode = 0;
#endif

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_REMO_SRV, &objIid, &RemoteMgmtSrvJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = RemoteMgmtSrvJobj;
		RemoteMgmtSrvJobj = NULL;
		RemoteMgmtSrvJobj = zcfgFeJsonMultiObjAppend(RDM_OID_REMO_SRV, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		//printf("%s paramList->name = %s \n", __FUNCTION__, paramList->name);
#if ZYXEL_TR69_REMOTE_MANAGE_SEPARATE_PARAMETER //Louis
		bool parambool = json_object_get_boolean(paramValue);
		const char *paramName = paramList->name;
		if(!(strcmp(paramList->name,"LAN")) && parambool)
		{
			mode |= REMOET_LAN; 
		}
		if(!(strcmp(paramList->name,"WLAN")) && parambool)
		{
			mode |= REMOET_WLAN; 
		}
		if(!(strcmp(paramList->name,"WAN")) && parambool)
		{
			mode |= REMOET_WAN; 
		}
		if(!(strcmp(paramList->name,"TrustDomain")) && parambool)
		{
			mode |= REMOET_TSTDM; 
		}
#endif

#ifdef CAAAB_CUSTOMIZATION
		const char *paramStr = json_object_get_string(paramValue);
		const char *paramName = paramList->name;
		if(!strcmp(paramStr,"LAN_WAN") && !strcmp(paramName,"Mode") ){
			isLanWan = true;
		}
#endif // CAAAB_CUSTOMIZATION
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(RemoteMgmtSrvJobj, paramList->name);
			if(tr181ParamValue != NULL) {
#ifdef IAAAA_CUSTOMIZATION
				if (strcmp("BoundInterfaceList", paramList->name) == 0) {
					pv[0] = 0;
					strncpy(pv, json_object_get_string(paramValue), 255);
					bil = true;
				}
#endif
				json_object_object_add(RemoteMgmtSrvJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

#if ZYXEL_TR69_REMOTE_MANAGE_SEPARATE_PARAMETER
	//printf("%s : mode = %d \n", __FUNCTION__, mode);
	switch (mode)
	{
		/*
		MODE_LAN_withoutWL			"LAN_withoutWL"
		MODE_LAN_withoutWL_WAN		"LAN_withoutWL_WAN"
		MODE_LAN_withoutWL_TstDm	"LAN_withoutWL_TstDm"
		MODE_WL_WAN			"WL_WAN"
		MODE_WL_Trust_Dm	"WL_Trust_Dm"
		MODE_WL_ONLY		"WL_ONLY"
		MODE_WAN_ONLY		"WAN_ONLY"
		MODE_LAN_ONLY		"LAN_ONLY"
		MODE_Trust_Dm		"Trust_Dm"
		MODE_LAN_WAN		"LAN_WAN"
		MODE_LAN_TstDm		"LAN_TstDm"
		MODE_ALL		"ALL"
		*/
		case MODE_LAN_withoutWL:
			printf("MODE_LAN_withoutWL\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("LAN_withoutWL"));
			break;
		case MODE_WL_ONLY:
			printf("MODE_WL_ONLY\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("WL_ONLY"));
			break;
		case MODE_LAN_ONLY:
			printf("MODE_LAN_ONLY\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("LAN_ONLY"));
			break;
		case MODE_WAN_ONLY:
			printf("MODE_WAN_ONLY\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("WAN_ONLY"));
			break;
		case MODE_LAN_withoutWL_WAN:
			printf("MODE_LAN_withoutWL_WAN\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("LAN_withoutWL_WAN"));
			break;
		case MODE_WL_WAN:
			printf("MODE_WL_WAN\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("WL_WAN"));
			break;
		case MODE_LAN_WAN:
			printf("MODE_LAN_WAN\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("LAN_WAN"));
			break;
		case MODE_Trust_Dm:
			printf("MODE_Trust_Dm\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("Trust_Dm"));
			break;
		case MODE_LAN_withoutWL_TstDm:
			printf("MODE_LAN_withoutWL_TstDm\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("LAN_withoutWL_TstDm"));
			break;
		case MODE_WL_Trust_Dm:
			printf("MODE_WL_Trust_Dm\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("WL_Trust_Dm"));
			break;
		case MODE_LAN_TstDm:
			printf("MODE_LAN_TstDm\n");
			json_object_object_add(RemoteMgmtSrvJobj, "Mode", json_object_new_string("LAN_TstDm"));
			break;
		default:
			break;
	}
#endif

	if(multiJobj){
		json_object_put(tmpObj);

#ifdef IAAAA_CUSTOMIZATION
		if (bil) {
			IID_INIT(rmsIid);
			while ((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_REMO_SRV, &rmsIid, &rmsObj) == ZCFG_SUCCESS)) {
				json_object_object_add(rmsObj, "BoundInterfaceList", json_object_new_string(pv));
				ret = zcfgFeObjJsonSet(RDM_OID_REMO_SRV, &rmsIid, rmsObj, NULL);
				json_object_put(rmsObj);
			}
		}
#endif

	}
	else{
#ifdef IAAAA_CUSTOMIZATION
		if (bil) {
			IID_INIT(rmsIid);
			while ((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_REMO_SRV, &rmsIid, &rmsObj) == ZCFG_SUCCESS)) {
				json_object_object_add(rmsObj, "BoundInterfaceList", json_object_new_string(pv));
				ret = zcfgFeObjJsonSet(RDM_OID_REMO_SRV, &rmsIid, rmsObj, NULL);
				json_object_put(rmsObj);
			}
		}
#endif
		if((ret = zcfgFeObjJsonSet(RDM_OID_REMO_SRV, &objIid, RemoteMgmtSrvJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(RemoteMgmtSrvJobj);
			return ret;
		}
		json_object_put(RemoteMgmtSrvJobj);
	}
	
#ifdef CAAAB_CUSTOMIZATION
	int currentUpTime = 0;
	char cmds[128] = {};
	if(isLanWan){
		currentUpTime = getCurrentUptime();
		sprintf(cmds,"echo %d > /var/TR069SetMGMT",currentUpTime);
		system(cmds);
	}
#endif // CAAAB_CUSTOMIZATION

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtRemoteMgmtSrvObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	return ZCFG_REQUEST_REJECT;
	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_REMO_SRV, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyExtRemoteMgmtSrvObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	return ZCFG_DELETE_REJECT;

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_REMO_SRV, &objIid, NULL);;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_RemoteManagement.Service.i
 */
zcfgRet_t zyExtRemoteMgmtSrvObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_RemoteManagement.Service.i
 */
int zyExtRemoteMgmtSrvObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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
#ifdef ZYXEL_OPAL_EXTENDER
        if(paramList->attr & PARAMETER_ATTR_EXTENDER_HIDDEN)
        {
            paramList++;
            continue;
        }
#endif
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_RemoteManagement.Service.i
 */
zcfgRet_t zyExtRemoteMgmtSrvObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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

zcfgRet_t zyExtRemoteMgmtTrustDomainObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
        zcfgRet_t ret;
        objIndex_t objIid;
        struct json_object *RemoteMgmtTrustDomainJobj = NULL;
        struct json_object *paramValue = NULL;
        tr98Parameter_t *paramList = NULL;

        printf("%s : Enter\n", __FUNCTION__);

        /*Get related tr181 objects*/
        IID_INIT(objIid);
        objIid.level = 1;
        sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.TrustDomain.%hhu", &objIid.idx[0]);

        if((ret = feObjJsonGet(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &objIid, &RemoteMgmtTrustDomainJobj, updateFlag)) != ZCFG_SUCCESS)
                return ret;

        /*fill up tr98 object from related tr181 objects*/
        *tr98Jobj = json_object_new_object();
        paramList = tr98Obj[handler].parameter;
        while(paramList->name != NULL) {
                /*Write parameter value from tr181 objects to tr98 object*/
                paramValue = json_object_object_get(RemoteMgmtTrustDomainJobj, paramList->name);
                if(paramValue != NULL) {
                        json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
                        paramList++;
                        continue;
                }

                /*Not defined in tr181, give it a default value*/
                printf("Can't find parameter %s in TR181\n", paramList->name);
                paramList++;
        }

        json_object_put(RemoteMgmtTrustDomainJobj);

        return ZCFG_SUCCESS;
}

zcfgRet_t zyExtRemoteMgmtTrustDomainObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMgmtTrustDomainJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.TrustDomain.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &objIid, &RemoteMgmtTrustDomainJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = RemoteMgmtTrustDomainJobj;
		RemoteMgmtTrustDomainJobj = NULL;
		RemoteMgmtTrustDomainJobj = zcfgFeJsonMultiObjAppend(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(RemoteMgmtTrustDomainJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(RemoteMgmtTrustDomainJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &objIid, RemoteMgmtTrustDomainJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(RemoteMgmtTrustDomainJobj);
			return ret;
		}
		json_object_put(RemoteMgmtTrustDomainJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtRemoteMgmtTrustDomainObjAdd(char *tr98FullPathName, int *idx)
{
        zcfgRet_t ret;
        objIndex_t objIid;

        printf("%s : Enter\n", __FUNCTION__);

        IID_INIT(objIid);
        ret = zcfgFeObjStructAdd(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &objIid, NULL);
        *idx = objIid.idx[0];                                                                                                                                           
        return ret;
}                                                        

zcfgRet_t zyExtRemoteMgmtTrustDomainObjDel(char *tr98FullPathName)
{
        objIndex_t objIid;

        printf("%s : Enter\n", __FUNCTION__);

        IID_INIT(objIid);
        objIid.level = 1;
        sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.TrustDomain.%hhu", &objIid.idx[0]);

        return zcfgFeObjStructDel(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &objIid, NULL);;
}
#endif


zcfgRet_t zyExtSpRemoteMgmtSrvObjAdd(char *tr98FullPathName, int *idx)
{
	return ZCFG_REQUEST_REJECT;


}

zcfgRet_t zyExtSpRemoteMgmtSrvObjDelete(char *tr98FullPathName)
{

	return ZCFG_DELETE_REJECT;

}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.SpService.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_RemoteManagement.SpService.i
 */
zcfgRet_t zyExtSpRemoteMgmtSrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMgmtSrvJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.SpService.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_SP_REMO_SRV, &objIid, &RemoteMgmtSrvJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(RemoteMgmtSrvJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(RemoteMgmtSrvJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtSpRemoteMgmtSrvObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMgmtSrvJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *RemoteMgmtSrvNameParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	//bool isLanWan = false,isHttpService = false;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.SpService.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_SP_REMO_SRV, &objIid, &RemoteMgmtSrvJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = RemoteMgmtSrvJobj;
		RemoteMgmtSrvJobj = NULL;
		RemoteMgmtSrvJobj = zcfgFeJsonMultiObjAppend(RDM_OID_SP_REMO_SRV, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(RemoteMgmtSrvJobj, paramList->name);
			RemoteMgmtSrvNameParamValue = json_object_object_get(RemoteMgmtSrvJobj, "Name");
			if(tr181ParamValue != NULL) {
				if(RemoteMgmtSrvNameParamValue != NULL)
				if( (strcmp(json_object_get_string(RemoteMgmtSrvNameParamValue),"HTTP")!=0) && (strcmp(paramList->name,"Port")==0) &&(strcmp(json_object_get_string(json_object_object_get(RemoteMgmtSrvJobj, "Port")),json_object_get_string(paramValue))!=0) ){
					printf("\nforbidden to set %s sparameter %s in TR181\n", json_object_get_string(RemoteMgmtSrvNameParamValue),paramList->name);
					return ZCFG_INVALID_PARAM_VALUE;
				}
				json_object_object_add(RemoteMgmtSrvJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_SP_REMO_SRV, &objIid, RemoteMgmtSrvJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(RemoteMgmtSrvJobj);
			return ret;
		}
		json_object_put(RemoteMgmtSrvJobj);
	}

	return ZCFG_SUCCESS;
}


zcfgRet_t zyExtSpRemoteMgmtTrustDomainObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
        zcfgRet_t ret;
        objIndex_t objIid;
        struct json_object *RemoteMgmtTrustDomainJobj = NULL;
        struct json_object *paramValue = NULL;
        tr98Parameter_t *paramList = NULL;

        printf("%s : Enter\n", __FUNCTION__);

        /*Get related tr181 objects*/
        IID_INIT(objIid);
        objIid.level = 1;
        sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.SpTrustDomain.%hhu", &objIid.idx[0]);

        if((ret = feObjJsonGet(RDM_OID_SP_TRUST_DOMAIN, &objIid, &RemoteMgmtTrustDomainJobj, updateFlag)) != ZCFG_SUCCESS)
                return ret;

        /*fill up tr98 object from related tr181 objects*/
        *tr98Jobj = json_object_new_object();
        paramList = tr98Obj[handler].parameter;
        while(paramList->name != NULL) {
                /*Write parameter value from tr181 objects to tr98 object*/
                paramValue = json_object_object_get(RemoteMgmtTrustDomainJobj, paramList->name);
                if(paramValue != NULL) {
                        json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
                        paramList++;
                        continue;
                }

                /*Not defined in tr181, give it a default value*/
                printf("Can't find parameter %s in TR181\n", paramList->name);
                paramList++;
        }

        json_object_put(RemoteMgmtTrustDomainJobj);

        return ZCFG_SUCCESS;
}

zcfgRet_t zyExtSpRemoteMgmtTrustDomainObjAdd(char *tr98FullPathName, int *idx)
{
        zcfgRet_t ret;
        objIndex_t objIid;

        printf("%s : Enter\n", __FUNCTION__);

        IID_INIT(objIid);
        ret = zcfgFeObjStructAdd(RDM_OID_SP_TRUST_DOMAIN, &objIid, NULL);
        *idx = objIid.idx[0];                                                                                                                                           
        return ret;
}                                                        

zcfgRet_t zyExtSpRemoteMgmtTrustDomainObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMgmtTrustDomainJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.SpTrustDomain.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_SP_TRUST_DOMAIN, &objIid, &RemoteMgmtTrustDomainJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = RemoteMgmtTrustDomainJobj;
		RemoteMgmtTrustDomainJobj = NULL;
		RemoteMgmtTrustDomainJobj = zcfgFeJsonMultiObjAppend(RDM_OID_SP_TRUST_DOMAIN, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(RemoteMgmtTrustDomainJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(RemoteMgmtTrustDomainJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_SP_TRUST_DOMAIN, &objIid, RemoteMgmtTrustDomainJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(RemoteMgmtTrustDomainJobj);
			return ret;
		}
		json_object_put(RemoteMgmtTrustDomainJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtSpRemoteMgmtTrustDomainObjDel(char *tr98FullPathName)
{
        objIndex_t objIid;

        printf("%s : Enter\n", __FUNCTION__);

        IID_INIT(objIid);
        objIid.level = 1;
        sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.SpTrustDomain.%hhu", &objIid.idx[0]);

        return zcfgFeObjStructDel(RDM_OID_SP_TRUST_DOMAIN, &objIid, NULL);;
}

#ifdef ABUU_CUSTOMIZATION
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_RemoteManagement.InterfaceTrustDomain.i
 */
zcfgRet_t zyExtIfaceTrustDomainObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMgmtIfaceTrustDmJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_INTERFACE_TRUST_DOMAIN, &objIid, &RemoteMgmtIfaceTrustDmJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(RemoteMgmtIfaceTrustDmJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(RemoteMgmtIfaceTrustDmJobj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtIfaceTrustDomainObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMgmtIfaceTrustDmJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_INTERFACE_TRUST_DOMAIN, &objIid, &RemoteMgmtIfaceTrustDmJobj)) != ZCFG_SUCCESS)
		return ret;

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(RemoteMgmtIfaceTrustDmJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(RemoteMgmtIfaceTrustDmJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	if(multiJobj){
		tmpObj = RemoteMgmtIfaceTrustDmJobj;
		RemoteMgmtIfaceTrustDmJobj = NULL;
		RemoteMgmtIfaceTrustDmJobj = zcfgFeJsonMultiObjAppend(RDM_OID_INTERFACE_TRUST_DOMAIN, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_INTERFACE_TRUST_DOMAIN, &objIid, RemoteMgmtIfaceTrustDmJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(RemoteMgmtIfaceTrustDmJobj);
			return ret;
		}
		json_object_put(RemoteMgmtIfaceTrustDmJobj);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtIfaceTrustDomainObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_INTERFACE_TRUST_DOMAIN, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyExtIfaceTrustDomainObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_INTERFACE_TRUST_DOMAIN, &objIid, NULL);;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.i.TrustDomainInfo.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_RemoteManagement.InterfaceTrustDomain.i.TrustDomainInfo.i
 */
zcfgRet_t zyExtIfaceTrustDomainInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ifaceTrustDmInfoPath[128] = "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.%hhu.TrustDomainInfo.%hhu";
	objIndex_t objIid;
	struct json_object *RemoteMgmtIfaceTrustDmInfoJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 2;
	if(sscanf(tr98FullPathName, ifaceTrustDmInfoPath, &objIid.idx[0], &objIid.idx[1]) != 2) 
		return ZCFG_INVALID_OBJECT;

	if((ret = feObjJsonGet(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &objIid, &RemoteMgmtIfaceTrustDmInfoJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(RemoteMgmtIfaceTrustDmInfoJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(RemoteMgmtIfaceTrustDmInfoJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtIfaceTrustDomainInfoObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ifaceTrustDmInfoPath[128] = "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.%hhu.TrustDomainInfo.%hhu";
	objIndex_t objIid;
	struct json_object *RemoteMgmtIfaceTrustDmInfoJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 2;
	if(sscanf(tr98FullPathName, ifaceTrustDmInfoPath, &objIid.idx[0], &objIid.idx[1]) != 2) 
		return ZCFG_INVALID_OBJECT;

	if((ret = zcfgFeObjJsonGet(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &objIid, &RemoteMgmtIfaceTrustDmInfoJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = RemoteMgmtIfaceTrustDmInfoJobj;
		RemoteMgmtIfaceTrustDmInfoJobj = NULL;
		RemoteMgmtIfaceTrustDmInfoJobj = zcfgFeJsonMultiObjAppend(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(RemoteMgmtIfaceTrustDmInfoJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(RemoteMgmtIfaceTrustDmInfoJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &objIid, RemoteMgmtIfaceTrustDmInfoJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(RemoteMgmtIfaceTrustDmInfoJobj);
			return ret;
		}
		json_object_put(RemoteMgmtIfaceTrustDmInfoJobj);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtIfaceTrustDomainInfoObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char ifaceTrustDmInfoPath[128] = "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.%hhu.TrustDomainInfo";
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	if(sscanf(tr98FullPathName, ifaceTrustDmInfoPath, &objIid.idx[0]) != 1) 
		return ZCFG_INVALID_OBJECT;
	if((ret = zcfgFeObjStructAdd(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &objIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}
	*idx = objIid.idx[1];
	
	return ret;
}

zcfgRet_t zyExtIfaceTrustDomainInfoObjDel(char *tr98FullPathName)
{
	char ifaceTrustDmInfoPath[128] = "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.%hhu.TrustDomainInfo.%hhu";
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	if(sscanf(tr98FullPathName, ifaceTrustDmInfoPath, &objIid.idx[0], &objIid.idx[1]) != 2)
		return ZCFG_INVALID_OBJECT;
	return zcfgFeObjStructDel(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &objIid, NULL);;
}
#endif //End of ABUU_CUSTOMIZATION

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.IGMP
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_EXT.IGMP
 */
 zcfgRet_t zyExtIgmpObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *igmpJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	const char *Interface = NULL;
	int len = 0;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_IGMP, &objIid, &igmpJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(igmpJobj, paramList->name);
		
		if(!strcmp(paramList->name, "Interface")) {
			Interface = json_object_get_string(paramValue);
			if(Interface) {
				char wanIpConnName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
				char tr98IntfList[TR98_MAX_OBJ_NAME_LENGTH * 5] = {0};
				char interface_str[256] = {0};
				char *ipIfaceName = NULL, *tmp = NULL;
				
				sprintf(interface_str, "%s", Interface);
				ipIfaceName = strtok_r(interface_str, ",", &tmp);
				while (ipIfaceName != NULL) {
					memset(wanIpConnName, 0, TR98_MAX_OBJ_NAME_LENGTH);
					
					if (zcfgFe181To98MappingNameGet(ipIfaceName, wanIpConnName) != ZCFG_SUCCESS) {
						ipIfaceName = strtok_r(NULL, "," ,&tmp);
						continue;
					}
					strcat(tr98IntfList, wanIpConnName);
					strcat(tr98IntfList, ",");
					ipIfaceName = strtok_r(NULL, ",", &tmp);
				}
				len = strlen(tr98IntfList);
				if(tr98IntfList[len-1] == ',')
					tr98IntfList[len-1] = '\0';
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98IntfList));
				paramList++;
				continue;
			}
		
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			paramList++;
			continue;
		}

		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(igmpJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.IGMP
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_IGMP
 */
zcfgRet_t zyExtIgmpObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *igmpJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	int igmpVersion = 3;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_IGMP, &objIid, &igmpJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = igmpJobj;
		igmpJobj = NULL;
		igmpJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_IGMP, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(igmpJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				if(!strcmp(paramList->name, "Interface")) {
#if 1 //Interface is a non-writable parameter
					paramList++;
					continue;
#else
					const char *Interface = NULL;
					Interface = json_object_get_string(paramValue);
					if(Interface) {
						char tr181IntfList[256] = {0};
						char ipIfaceName[32] = {0};
						char interface_str[TR98_MAX_OBJ_NAME_LENGTH * 10] = {0};
						char *wanIpConnName = NULL, *tmp = NULL;
						int len = 0;
						
						sprintf(interface_str, "%s", Interface);
						wanIpConnName = strtok_r(interface_str, ",", &tmp);
						while (wanIpConnName != NULL) {
							memset(ipIfaceName, 0, 32);
							
							if (zcfgFe98To181MappingNameGet(wanIpConnName, ipIfaceName) != ZCFG_SUCCESS) {
								wanIpConnName = strtok_r(NULL, "," ,&tmp);
								continue;
							}
							if(strstr(ipIfaceName, "PPP.Interface.")){
								zcfgFeTr181IfaceStackHigherLayerGet(ipIfaceName, ipIfaceName);
							}
							strcat(tr181IntfList, ipIfaceName);
							strcat(tr181IntfList, ",");
							wanIpConnName = strtok_r(NULL, ",", &tmp);
						}
						len = strlen(tr181IntfList);
						if(tr181IntfList[len-1] == ',')
							tr181IntfList[len-1] = '\0';
						json_object_object_add(igmpJobj, paramList->name, json_object_new_string(tr181IntfList));
					}
#endif
				}

				if(!strcmp(paramList->name, "IgmpVersion")){
					if(!(json_object_get_int(paramValue) >= 1 && json_object_get_int(paramValue) <=3)){
						json_object_object_add(igmpJobj, "IgmpVersion", json_object_new_int(igmpVersion));
					}
					else
						json_object_object_add(igmpJobj, "IgmpVersion", JSON_OBJ_COPY(paramValue));
				}
				else{
					json_object_object_add(igmpJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				}

				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_IGMP, &objIid, igmpJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(igmpJobj);
			return ret;
		}
		json_object_put(igmpJobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.IGMP
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_IGMP
 */
zcfgRet_t zyExtIgmpObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.IGMP
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_IGMP
 */
int zyExtIgmpObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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
		
		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.IGMP
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_IGMP
 */
zcfgRet_t zyExtIgmpObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.MLD
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_MLD
 */
 zcfgRet_t zyExtMldObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *mldJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	const char *Interface = NULL;
	int len = 0;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_MLD, &objIid, &mldJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(mldJobj, paramList->name);

		if(!strcmp(paramList->name, "Interface")) {
			Interface = json_object_get_string(paramValue);
			if(Interface) {
				char wanIpConnName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
				char tr98IntfList[TR98_MAX_OBJ_NAME_LENGTH * 5] = {0};
				char interface_str[256] = {0};
				char *ipIfaceName = NULL, *tmp = NULL;

				snprintf(interface_str, sizeof(interface_str), "%s", Interface);
				ipIfaceName = strtok_r(interface_str, ",", &tmp);
				while (ipIfaceName != NULL) {
					memset(wanIpConnName, 0, TR98_MAX_OBJ_NAME_LENGTH);

					if (zcfgFe181To98MappingNameGet(ipIfaceName, wanIpConnName) != ZCFG_SUCCESS) {
						ipIfaceName = strtok_r(NULL, "," ,&tmp);
						continue;
					}
					strncat(tr98IntfList, wanIpConnName, sizeof(tr98IntfList)-strlen(tr98IntfList)-1);
					strncat(tr98IntfList, ",", sizeof(tr98IntfList)-strlen(tr98IntfList)-1);
					ipIfaceName = strtok_r(NULL, ",", &tmp);
				}
				len = strlen(tr98IntfList);
				if(tr98IntfList[len-1] == ',')
					tr98IntfList[len-1] = '\0';
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98IntfList));
				paramList++;
				continue;
			}

			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			paramList++;
			continue;
		}

		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(mldJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.MLD
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_MLD
 */
zcfgRet_t zyExtMldObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *mldJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	int mldVersion = 2;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_MLD, &objIid, &mldJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = mldJobj;
		mldJobj = NULL;
		mldJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_MLD, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(mldJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				if(!strcmp(paramList->name, "Interface")) {
					paramList++;
					continue;
				}

				if(!strcmp(paramList->name, "MldVersion")){
					if(!(json_object_get_int(paramValue) >= 1 && json_object_get_int(paramValue) <=2)){
						json_object_object_add(mldJobj, "MldVersion", json_object_new_int(mldVersion));
					}
					else
						json_object_object_add(mldJobj, "MldVersion", JSON_OBJ_COPY(paramValue));
				}
				else{
					json_object_object_add(mldJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				}

				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_MLD, &objIid, mldJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(mldJobj);
			return ret;
		}
		json_object_put(mldJobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.MLD
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_MLD
 */
zcfgRet_t zyExtMldObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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
			snprintf(tr98Notify, sizeof(tr98Notify), "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.MLD
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_MLD
 */
int zyExtMldObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
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

		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}

	return attrValue;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.MLD
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_MLD
 */
zcfgRet_t zyExtMldObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
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

#if defined(SAAAD_RANDOM_PASSWORD) || defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
#ifdef ABUU_CUSTOMIZATION
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_LoginCfg
 */
zcfgRet_t zyExtLogCfgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LogCfgJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	// printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);

	if((ret = feObjJsonGet(RDM_OID_ZY_LOG_CFG, &objIid, &LogCfgJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(LogCfgJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(LogCfgJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_LoginCfg
 */
zcfgRet_t zyExtLogCfgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LogCfgJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	struct json_object *tr181ParamValue = NULL, *tmpObj = NULL;

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_LOG_CFG, &objIid, &LogCfgJobj, NULL)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = LogCfgJobj;
		LogCfgJobj = NULL;
		LogCfgJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_LOG_CFG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(tr98Jobj, paramList->name);

		tr181ParamValue = json_object_object_get(LogCfgJobj, paramList->name);
		if(paramValue && tr181ParamValue) {
			json_object_object_add(LogCfgJobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	if(multiJobj) {
		json_object_put(tmpObj);
	}
	else {
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_LOG_CFG, &objIid, LogCfgJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(LogCfgJobj);
			return ret;
		}
		json_object_put(LogCfgJobj);
	}

	return ZCFG_SUCCESS;
}
#endif //ABUU
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_LoginCfg
 */
zcfgRet_t zyExtLogCfgGpObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LogCfgGpJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;

	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.%hhu", &objIid.idx[0]);
#ifdef SAAAE_TR69_LOGINACCOUNT
	objIid.idx[0] += 1; //Don't display high level
#endif

	if((ret = feObjJsonGet(RDM_OID_ZY_LOG_CFG_GP, &objIid, &LogCfgGpJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
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
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(LogCfgGpJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(LogCfgGpJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_LoginCfg
 */
zcfgRet_t zyExtLogCfgGpObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LogCfgGpJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	struct json_object *tr181ParamValue = NULL, *tmpObj = NULL;

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.%hhu", &objIid.idx[0]);
#ifdef SAAAE_TR69_LOGINACCOUNT
	objIid.idx[0] += 1;
#endif

	if((ret = feObjJsonGet(RDM_OID_ZY_LOG_CFG_GP, &objIid, &LogCfgGpJobj, NULL)) != ZCFG_SUCCESS)
		return ret;

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(tr98Jobj, paramList->name);

		tr181ParamValue = json_object_object_get(LogCfgGpJobj, paramList->name);
		if(paramValue && tr181ParamValue) {
			json_object_object_add(LogCfgGpJobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	if(multiJobj) {
		tmpObj = LogCfgGpJobj;
		LogCfgGpJobj = NULL;
		LogCfgGpJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_LOG_CFG_GP, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}
	else {
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_LOG_CFG_GP, &objIid, LogCfgGpJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(LogCfgGpJobj);
			return ret;
		}
		json_object_put(LogCfgGpJobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.i.Account.i
 *
 *   Related object in TR181:
 *   Device.LoginCfg.LogGp.i.Account.i
 */
zcfgRet_t zyExtLogCfgGpAccountObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char loginCfgGpAccoutPath[128] = "InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.%hhu.Account.%hhu";
	uint32_t loginCfgGpAccoutOid = 0;
	objIndex_t loginCfgGpAccoutIid;
	struct json_object *loginCfgGpAccoutObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *default_password = NULL;
	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(loginCfgGpAccoutIid);
	loginCfgGpAccoutIid.level = 2;

	if(sscanf(tr98FullPathName, loginCfgGpAccoutPath, &loginCfgGpAccoutIid.idx[0], &loginCfgGpAccoutIid.idx[1]) != 2) return ZCFG_INVALID_OBJECT;
#ifdef SAAAE_TR69_LOGINACCOUNT
	loginCfgGpAccoutIid.idx[0] += 1; //Don't display high level
#endif
	loginCfgGpAccoutOid = RDM_OID_ZY_LOG_CFG_GP_ACCOUNT;

	if((ret = feObjJsonGet(loginCfgGpAccoutOid, &loginCfgGpAccoutIid, &loginCfgGpAccoutObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 rule object*/
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
		paramValue = json_object_object_get(loginCfgGpAccoutObj, paramList->name);
#ifdef TAAAD_CUSTOMIZATION
		if( loginCfgGpAccoutIid.idx[0] == 1 && (loginCfgGpAccoutIid.idx[1] == 1 || loginCfgGpAccoutIid.idx[1] == 2)){
			if(!strcmp(paramList->name, "Password")){
				default_password = json_object_to_json_string(paramValue);
				if(strlen(default_password)==2){
					paramValue = json_object_object_get(loginCfgGpAccoutObj, "DefaultPassword");
				}
			}
		}
#endif
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(loginCfgGpAccoutObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtLogCfgGpAccountObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char loginCfgGpAccoutPath[128] = "InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.%hhu.Account.%hhu";
	uint32_t  loginCfgGpAccoutOid = 0;
	objIndex_t loginCfgGpAccoutIid;
	struct json_object *loginCfgGpAccoutObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	char *tmpbuf = NULL;
	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(loginCfgGpAccoutIid);
	loginCfgGpAccoutIid.level = 2;
	if(sscanf(tr98FullPathName, loginCfgGpAccoutPath, &loginCfgGpAccoutIid.idx[0], &loginCfgGpAccoutIid.idx[1]) != 2) return ZCFG_INVALID_OBJECT;
#ifdef SAAAE_TR69_LOGINACCOUNT
	loginCfgGpAccoutIid.idx[0] += 1; //Don't display high level
#endif
	loginCfgGpAccoutOid = RDM_OID_ZY_LOG_CFG_GP_ACCOUNT;

	if((ret = zcfgFeObjJsonGet(loginCfgGpAccoutOid, &loginCfgGpAccoutIid, &loginCfgGpAccoutObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = loginCfgGpAccoutObj;
		loginCfgGpAccoutObj = NULL;
		loginCfgGpAccoutObj = zcfgFeJsonMultiObjAppend(loginCfgGpAccoutOid, &loginCfgGpAccoutIid, multiJobj, tmpObj);
	}
#ifdef TAAAD_CUSTOMIZATION
				if( loginCfgGpAccoutIid.idx[0] == 1 && (loginCfgGpAccoutIid.idx[1] == 1 || loginCfgGpAccoutIid.idx[1] == 2)){
					if(json_object_object_get(json_object_object_get(multiJobj,"spv"),"Password") || json_object_object_get(json_object_object_get(multiJobj,"spv"),"DefaultPassword")){
						sprintf(paramfault, "Password of root or assistant shouldn't be changed!");
						return ZCFG_INVALID_PARAM_VALUE;
					}
				}
#endif

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(json_object_object_get(json_object_object_get(multiJobj,"spv"),"Password") && !strcmp(paramList->name, "Password")){
			tmpbuf = json_object_get_string(paramValue);
			if(strlen(tmpbuf)==0){
				strncpy(paramfault, "Password", NAME_SIZE);
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(loginCfgGpAccoutObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(loginCfgGpAccoutObj, paramList->name, JSON_OBJ_COPY(paramValue));
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
		if((ret = zcfgFeObjJsonSet(loginCfgGpAccoutOid, &loginCfgGpAccoutIid, loginCfgGpAccoutObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(loginCfgGpAccoutObj);
			return ret;
		}
		json_object_put(loginCfgGpAccoutObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtLogCfgGpAccountObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char loginCfgGpAccoutPath[128] = "InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.%hhu.Account";
#ifdef SAAAD_RANDOM_PASSWORD
	struct json_object *loginCfgGpAccoutObj = NULL;
#endif
	objIndex_t loginCfgGpAccoutIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

#ifdef SAAAD_RANDOM_PASSWORD
	IID_INIT(loginCfgGpAccoutIid);
	loginCfgGpAccoutIid.level = 2;
	loginCfgGpAccoutIid.idx[0] = 1;
	loginCfgGpAccoutIid.idx[1] = 3;

	if(zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginCfgGpAccoutIid, &loginCfgGpAccoutObj) == ZCFG_SUCCESS){
		json_object_put(loginCfgGpAccoutObj);
		return ZCFG_REQUEST_REJECT;
	}
#endif

	IID_INIT(loginCfgGpAccoutIid);
	loginCfgGpAccoutIid.level = 1;
	if(sscanf(tr98FullPathName, loginCfgGpAccoutPath, &loginCfgGpAccoutIid.idx[0]) != 1) return ZCFG_INVALID_OBJECT;
#ifdef SAAAE_TR69_LOGINACCOUNT
	loginCfgGpAccoutIid.idx[0] += 1; //Don't display high level
#endif

#ifdef SAAAD_RANDOM_PASSWORD
	if(loginCfgGpAccoutIid.idx[0] != 1)
		return ZCFG_REQUEST_REJECT;
#endif
#ifdef CONFIG_ZYXEL_TR140
	if(loginCfgGpAccoutIid.idx[0]==1)
		return ZCFG_REQUEST_REJECT;
#endif
	if((ret = zcfgFeObjStructAdd(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginCfgGpAccoutIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add LoginCfg.LogGp.%hhu.Account. Fail.\n", __FUNCTION__, loginCfgGpAccoutIid.idx[0]);
		return ret;
	}

	*idx = loginCfgGpAccoutIid.idx[1];

	return ZCFG_SUCCESS;
}

#if defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
zcfgRet_t zyExtLogCfgGpAccountObjDel(char *tr98FullPathName)
{
        objIndex_t objIid;
		char loginCfgGpAccoutPath[128] = "InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.%hhu.Account.%hhu";

        printf("%s : Enter\n", __FUNCTION__);

        IID_INIT(objIid);
        objIid.level = 2;
		if(sscanf(tr98FullPathName, loginCfgGpAccoutPath, &objIid.idx[0], &objIid.idx[1]) != 2)
			return ZCFG_INVALID_OBJECT;
		
#ifdef SAAAE_TR69_LOGINACCOUNT
		objIid.idx[0] += 1;
#endif
#ifdef CONFIG_ZYXEL_TR140
        if((objIid.idx[0]==1 && objIid.idx[1]==1) || (objIid.idx[0]==2 && objIid.idx[1]==1))
			return ZCFG_DELETE_REJECT; 
#endif
        return zcfgFeObjStructDel(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &objIid, NULL);;
}
#endif

#endif
#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.WlanScheduler
 *
 *   Related object in TR181: Device.X_ZYXEL_WlanSchedulerAccess
 
 *
 */
zcfgRet_t zyExtWlanSchedulerObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *parenCtlJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_WLAN_SCHEDULER_ACCESS, &objIid, &parenCtlJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(parenCtlJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(parenCtlJobj);
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtWlanSchedulerObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *parenCtlJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_WLAN_SCHEDULER_ACCESS, &objIid, &parenCtlJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = parenCtlJobj;
		parenCtlJobj = NULL;
		parenCtlJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WLAN_SCHEDULER_ACCESS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(parenCtlJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(parenCtlJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WLAN_SCHEDULER_ACCESS, &objIid, parenCtlJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(parenCtlJobj);
			return ret;
		}
		json_object_put(parenCtlJobj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtWlanSchedulerObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtWlanSchedulerObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  parenCtlOid = 0;
	objIndex_t parenCtlIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(parenCtlIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	parenCtlOid = zcfgFeObjNameToObjId(tr181Obj, &parenCtlIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(parenCtlOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtWlanSchedulerObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  parenCtlOid = 0;
	objIndex_t parenCtlIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(parenCtlIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	parenCtlOid = zcfgFeObjNameToObjId(tr181Obj, &parenCtlIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(parenCtlOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(parenCtlOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, parenCtlOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.ParentalControl.Profile.i
 *
 *   Related object in TR181: Device.X_ZYXEL_ParentalControl.Profile.i
 *
 */
 /*
 *   TR98 Object Name :InternetGatewayDevice.X_ZYXEL_EXT.WlanScheduler.Rule.i
 *
 *   Related object in TR181: 	Device.WiFi.X_ZYXEL_WLAN_Scheduler.i
 
 *
 */
zcfgRet_t zyExtWlanSchedulerRuleObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *parenCtlProfJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.WlanScheduler.Rule.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_WLAN_SCHEDULER, &objIid, &parenCtlProfJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(parenCtlProfJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(parenCtlProfJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtWlanSchedulerRuleObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *parenCtlProfJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.WlanScheduler.Rule.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_WLAN_SCHEDULER, &objIid, &parenCtlProfJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = parenCtlProfJobj;
		parenCtlProfJobj = NULL;
		parenCtlProfJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WLAN_SCHEDULER, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(parenCtlProfJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(parenCtlProfJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WLAN_SCHEDULER, &objIid, parenCtlProfJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(parenCtlProfJobj);
			return ret;
		}
		json_object_put(parenCtlProfJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtWlanSchedulerRuleObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_WLAN_SCHEDULER, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyExtWlanSchedulerRuleObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.WlanScheduler.Rule.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_WLAN_SCHEDULER, &objIid, NULL);;
}

#endif
/* InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting */
zcfgRet_t zyExtLogSettingObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *logSettingJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_LOG_SETTING, &objIid, &logSettingJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(logSettingJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(logSettingJobj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtLogSettingObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *logSettingJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *setvalue = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_LOG_SETTING, &objIid, &logSettingJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = logSettingJobj;
		logSettingJobj = NULL;
		logSettingJobj = zcfgFeJsonMultiObjAppend(RDM_OID_LOG_SETTING, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
#ifdef ZYXEL_CUSTOMIZATION_SYSLOG			
			if(!strcmp(paramList->name,"LogMode")){
				setvalue = json_object_get_string(paramValue);
				if(strcmp(setvalue, "0") && strcmp(setvalue, "1")){
					ret = ZCFG_INVALID_PARAM_VALUE;
					strcpy(paramfault,"LogMode");
					return ret;
				}	
			}
#endif			
			tr181ParamValue = json_object_object_get(logSettingJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(logSettingJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_LOG_SETTING, &objIid, logSettingJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(logSettingJobj);
			return ret;
		}
		json_object_put(logSettingJobj);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtLogSettingObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtLogSettingObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  logOid = 0;
	objIndex_t logIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(logIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	logOid = zcfgFeObjNameToObjId(tr181Obj, &logIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(logOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtLogSettingObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  logOid = 0;
	objIndex_t logIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(logIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	logOid = zcfgFeObjNameToObjId(tr181Obj, &logIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(logOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(logOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, logOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.i */
zcfgRet_t zyExtLogClassifyObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *logClassifyJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.%hhu", &objIid.idx[0]);
	
	if((ret = feObjJsonGet(RDM_OID_LOG_CLASSIFY, &objIid, &logClassifyJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL)
	{
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(logClassifyJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	json_object_put(logClassifyJobj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtLogClassifyObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_LOG_CLASSIFY, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyExtLogClassifyObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_LOG_CLASSIFY, &objIid, NULL);;
}

zcfgRet_t zyExtLogClassifyObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtLogClassifyObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  logOid = 0;
	objIndex_t logIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(logIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	logOid = zcfgFeObjNameToObjId(tr181Obj, &logIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(logOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtLogClassifyObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  logOid = 0;
	objIndex_t logIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(logIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	logOid = zcfgFeObjNameToObjId(tr181Obj, &logIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(logOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(logOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, logOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.i.Category.i */
zcfgRet_t zyExtLogCategoryObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char logObjFormate[128] = "InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.%hhu.Category.%hhu";
	uint32_t  logOid = 0;
	objIndex_t logIid;
	struct json_object *logObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(logIid);
	logIid.level = 2;
	if(sscanf(tr98FullPathName, logObjFormate, &logIid.idx[0], &logIid.idx[1]) != 2)
		return ZCFG_INVALID_OBJECT;	
	logOid = RDM_OID_LOG_CATEGORY;

	if((ret = feObjJsonGet(logOid, &logIid, &logObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
	
	/*fill up tr98 rule object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(logObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(logObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtLogCategoryObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	char categoryPath[128] = "InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.%hhu.Category.%hhu";
	objIndex_t categoryIid;
	struct json_object *categoryObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(categoryIid);
	categoryIid.level = 2;
	if(sscanf(tr98FullPathName, categoryPath, &categoryIid.idx[0], &categoryIid.idx[1]) != 2) return ZCFG_INVALID_OBJECT;

	if((ret = zcfgFeObjJsonGet(RDM_OID_LOG_CATEGORY, &categoryIid, &categoryObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = categoryObj;
		categoryObj = NULL;
		categoryObj = zcfgFeJsonMultiObjAppend(RDM_OID_LOG_CATEGORY, &categoryIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(categoryObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(categoryObj, paramList->name, JSON_OBJ_COPY(paramValue));
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
		if((ret = zcfgFeObjJsonSet(RDM_OID_LOG_CATEGORY, &categoryIid, categoryObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(categoryObj);
			return ret;
		}
		json_object_put(categoryObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtLogCategoryObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char logObjFormate[128] = "InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.%hhu.Category";
	uint32_t  logOid = 0;
	objIndex_t logIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(logIid);
	logIid.level = 1;
	if(sscanf(tr98FullPathName, logObjFormate, &logIid.idx[0]) != 1) return ZCFG_INVALID_OBJECT;	
	logOid = RDM_OID_LOG_CATEGORY;

	if((ret = zcfgFeObjStructAdd(logOid, &logIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add X_ZYXEL_EXT.Log_Setting.LogClassify.%hhu.Category. Fail.\n", __FUNCTION__, logIid.idx[0]);
		return ret;
	}

	*idx = logIid.idx[1];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtLogCategoryObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char logObjFormate[128] = "InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.%hhu.Category.%hhu";
	uint32_t  logOid = 0;
	objIndex_t logIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(logIid);
	logIid.level = 2;
	if(sscanf(tr98FullPathName, logObjFormate, &logIid.idx[0], &logIid.idx[1]) != 2) return ZCFG_INVALID_OBJECT;	
	logOid = RDM_OID_LOG_CATEGORY;

	ret = zcfgFeObjStructDel(logOid, &logIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG, "%s : Delete Firewall.Chain.%hhu.Rule.%hhu Fail\n", __FUNCTION__, logIid.idx[0], logIid.idx[1]);
	}

	return ret;
}

zcfgRet_t zyExtLogCategoryObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtLogCategoryObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  logOid = 0;
	objIndex_t logIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(logIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	logOid = zcfgFeObjNameToObjId(tr181Obj, &logIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(logOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtLogCategoryObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  logOid = 0;
	objIndex_t logIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(logIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	logOid = zcfgFeObjNameToObjId(tr181Obj, &logIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(logOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(logOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, logOid, paramList->name);
		}
		break;

	}
	
	return ret;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.MacFilter */
zcfgRet_t zyExtMacFilterObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	//char tr181Obj[128] = {0};
	//char mapObj[128] = {0};
	uint32_t  macFilterOid = 0;
	objIndex_t macFilterIid;
	struct json_object *macFilterObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(macFilterIid);
	/*if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		printf("%d:Get %s failed\n", __LINE__, tr98FullPathName);
		return ZCFG_INVALID_OBJECT;
	}*/
	//snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	macFilterOid = RDM_OID_ZY_MAC_FILTER;

	if((ret = feObjJsonGet(macFilterOid, &macFilterIid, &macFilterObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 firewall object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(macFilterObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(macFilterObj);
	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.MacFilter */
zcfgRet_t zyExtMacFilterObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	//char tr181Obj[128] = {0};
	//char mapObj[128] = {0};
	uint32_t  macFilterOid = 0;
	objIndex_t macFilterIid;
	struct json_object *macFilterObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(macFilterIid);
	/*if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		printf("%d:Get %s failed\n", __LINE__, tr98FullPathName);
		return ZCFG_INVALID_OBJECT;
	}*/
	
	//snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	macFilterOid = RDM_OID_ZY_MAC_FILTER;
	if((ret = zcfgFeObjJsonGet(macFilterOid, &macFilterIid, &macFilterObj)) != ZCFG_SUCCESS)
		return ret;
	
	if(multiJobj){
		tmpObj = macFilterObj;
		macFilterObj = NULL;
		macFilterObj = zcfgFeJsonMultiObjAppend(macFilterOid, &macFilterIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(macFilterObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(macFilterObj, paramList->name, JSON_OBJ_COPY(paramValue));
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
		if((ret = zcfgFeObjJsonSet(macFilterOid, &macFilterIid, macFilterObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(macFilterObj);
			return ret;
		}
		json_object_put(macFilterObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMacFilterObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtMacFilterObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  macOid = 0;
	objIndex_t macIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(macIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	macOid = zcfgFeObjNameToObjId(tr181Obj, &macIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(macOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtMacFilterObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  macOid = 0;
	objIndex_t macIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(macIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	macOid = zcfgFeObjNameToObjId(tr181Obj, &macIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(macOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(macOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, macOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.MacFilter.WhiteList.i */
zcfgRet_t zyExtMacFilterWhiteListObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char levelObjFormate[128] = "InternetGatewayDevice.X_ZYXEL_EXT.MacFilter.WhiteList.%hhu";
	uint32_t  levelOid = 0;
	objIndex_t levelIid;
	struct json_object *levelObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	IID_INIT(levelIid);
	levelIid.level = 1;
	if(sscanf(tr98FullPathName, levelObjFormate, levelIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	levelOid = RDM_OID_ZY_MAC_FILTER_WHITE_LIST;

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

zcfgRet_t zyExtMacFilterWhiteListObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char whiteListObjFormate[128] = "InternetGatewayDevice.X_ZYXEL_EXT.MacFilter.WhiteList.%hhu";
	uint32_t  whiteListOid = 0;
	objIndex_t whiteListIid;
	struct json_object *whiteListObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	int index = 0;
	char macAddr[18] = {0};

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(whiteListIid);
	whiteListIid.level = 1;
	if(sscanf(tr98FullPathName, whiteListObjFormate, whiteListIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	whiteListOid = RDM_OID_ZY_MAC_FILTER_WHITE_LIST;

	if((ret = zcfgFeObjJsonGet(whiteListOid, &whiteListIid, &whiteListObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = whiteListObj;
		whiteListObj = NULL;
		whiteListObj = zcfgFeJsonMultiObjAppend(whiteListOid, &whiteListIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(whiteListObj, paramList->name);
			if(tr181ParamValue != NULL) {
				// -------- check validity of MAC start --------
				if(strcmp(paramList->name, "MACAddress") == 0){
					strcpy(macAddr, json_object_get_string(paramValue));
					if(strcmp(macAddr, "") != 0){
						for(; index < 17; index++) {
							if(index % 3 != 2 && !isxdigit(macAddr[index])){
								json_object_put(whiteListObj);
								return ZCFG_INVALID_ARGUMENTS;
							}
							else if(index % 3 == 2 && macAddr[index] != ':'){
								json_object_put(whiteListObj);
								return ZCFG_INVALID_ARGUMENTS;
							}
						}
						if(macAddr[17] != '\0'){
							json_object_put(whiteListObj);
							return ZCFG_INVALID_ARGUMENTS;
						}
					}
				}
				// -------- check validity of MAC end --------
				json_object_object_add(whiteListObj, paramList->name, JSON_OBJ_COPY(paramValue));
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
		if((ret = zcfgFeObjJsonSet(whiteListOid, &whiteListIid, whiteListObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(whiteListObj);
			return ret;
		}
		json_object_put(whiteListObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMacFilterWhiteListObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyExtMacFilterWhiteListObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char levelObjFormate[128] = "InternetGatewayDevice.X_ZYXEL_EXT.MacFilter.WhiteList.%hhu";
	uint32_t  levelOid = 0;
	objIndex_t levelIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(levelIid);
	levelIid.level = 1;
	if(sscanf(tr98FullPathName, levelObjFormate, levelIid.idx) != 1) return ZCFG_INVALID_OBJECT;	
	levelOid = RDM_OID_ZY_MAC_FILTER_WHITE_LIST;


	ret = zcfgFeObjStructDel(levelOid, &levelIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG, "%s : Delete X_ZYXEL_EXT.MacFilter.WhiteList.%hhu Fail\n", __FUNCTION__, levelIid.idx[0]);
	}

	return ret;
}

zcfgRet_t zyExtMacFilterWhiteListObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtMacFilterWhiteListObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  macOid = 0;
	objIndex_t macIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(macIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	macOid = zcfgFeObjNameToObjId(tr181Obj, &macIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(macOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtMacFilterWhiteListObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  macOid = 0;
	objIndex_t macIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(macIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	macOid = zcfgFeObjNameToObjId(tr181Obj, &macIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(macOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(macOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, macOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.ParentalControl
 *
 *   Related object in TR181: Device.X_ZYXEL_ParentalControl
 *
 */
zcfgRet_t zyExtParenCtlObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *parenCtlJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_PAREN_CTL, &objIid, &parenCtlJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(parenCtlJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(parenCtlJobj);
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtParenCtlObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *parenCtlJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_PAREN_CTL, &objIid, &parenCtlJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = parenCtlJobj;
		parenCtlJobj = NULL;
		parenCtlJobj = zcfgFeJsonMultiObjAppend(RDM_OID_PAREN_CTL, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(parenCtlJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(parenCtlJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_PAREN_CTL, &objIid, parenCtlJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(parenCtlJobj);
			return ret;
		}
		json_object_put(parenCtlJobj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtParenCtlObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtParenCtlObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  parenCtlOid = 0;
	objIndex_t parenCtlIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(parenCtlIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	parenCtlOid = zcfgFeObjNameToObjId(tr181Obj, &parenCtlIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(parenCtlOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtParenCtlObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  parenCtlOid = 0;
	objIndex_t parenCtlIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(parenCtlIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	parenCtlOid = zcfgFeObjNameToObjId(tr181Obj, &parenCtlIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(parenCtlOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(parenCtlOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, parenCtlOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.ParentalControl.Profile.i
 *
 *   Related object in TR181: Device.X_ZYXEL_ParentalControl.Profile.i
 *
 */
zcfgRet_t zyExtParenCtlProfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *parenCtlProfJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.ParentalControl.Profile.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_PAREN_CTL_PROF, &objIid, &parenCtlProfJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(parenCtlProfJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(parenCtlProfJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtParenCtlProfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *parenCtlProfJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.ParentalControl.Profile.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_PAREN_CTL_PROF, &objIid, &parenCtlProfJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = parenCtlProfJobj;
		parenCtlProfJobj = NULL;
		parenCtlProfJobj = zcfgFeJsonMultiObjAppend(RDM_OID_PAREN_CTL_PROF, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(parenCtlProfJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(parenCtlProfJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_PAREN_CTL_PROF, &objIid, parenCtlProfJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(parenCtlProfJobj);
			return ret;
		}
		json_object_put(parenCtlProfJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtParenCtlProfObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_PAREN_CTL_PROF, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyExtParenCtlProfObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.ParentalControl.Profile.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_PAREN_CTL_PROF, &objIid, NULL);;
}
#if 0
zcfgRet_t zyExtParenCtlProfObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtParenCtlProfObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  parentCtlOid = 0;
	objIndex_t parentCtlIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(parentCtlIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	parentCtlOid = zcfgFeObjNameToObjId(tr181Obj, &parentCtlIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(parentCtlOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtParenCtlProfObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  parentCtlOid = 0;
	objIndex_t parentCtlIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(parentCtlIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	parentCtlOid = zcfgFeObjNameToObjId(tr181Obj, &parentCtlIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(parentCtlOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(parentCtlOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, parentCtlOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.Security
 *
 *   Related object in TR181: Device.Security
 *
 */
zcfgRet_t zyExtSecObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *secJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_DEV_SEC, &objIid, &secJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(secJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(secJobj);
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtSecObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtSecObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  secOid = 0;
	objIndex_t secIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(secIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	secOid = zcfgFeObjNameToObjId(tr181Obj, &secIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(secOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtSecObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  secOid = 0;
	objIndex_t secIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(secIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	secOid = zcfgFeObjNameToObjId(tr181Obj, &secIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(secOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(secOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, secOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.Security.Certificate.i
 *
 *   Related object in TR181: Device.Security.Certificate.i
 *
 */
zcfgRet_t zyExtSecCertObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *secJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.Security.Certificate.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_DEV_SEC_CERT, &objIid, &secJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(secJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(secJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtSecCertObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *secJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.Security.Certificate.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_DEV_SEC_CERT, &objIid, &secJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = secJobj;
		secJobj = NULL;
		secJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DEV_SEC_CERT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(secJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(secJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DEV_SEC_CERT, &objIid, secJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(secJobj);
			return ret;
		}
		json_object_put(secJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtSecCertObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_DEV_SEC_CERT, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyExtSecCertObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.Security.Certificate.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_DEV_SEC_CERT, &objIid, NULL);;
}
#if 0
zcfgRet_t zyExtSecCertObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtSecCertObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  secOid = 0;
	objIndex_t secIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(secIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	secOid = zcfgFeObjNameToObjId(tr181Obj, &secIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(secOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtSecCertObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  secOid = 0;
	objIndex_t secIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(secIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	secOid = zcfgFeObjNameToObjId(tr181Obj, &secIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(secOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(secOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, secOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.SAMBA
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_SAMBA
 */
 zcfgRet_t zyExtSambaObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *sambaJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_SAMBA, &objIid, &sambaJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(sambaJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(sambaJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtSambaObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *sambaJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_SAMBA, &objIid, &sambaJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = sambaJobj;
		sambaJobj = NULL;
		sambaJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_SAMBA, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(sambaJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(sambaJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_SAMBA, &objIid, sambaJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(sambaJobj);
			return ret;
		}
		json_object_put(sambaJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtSambaObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtSambaObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  sambaOid = 0;
	objIndex_t sambaIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(sambaIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	sambaOid = zcfgFeObjNameToObjId(tr181Obj, &sambaIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(sambaOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtSambaObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  sambaOid = 0;
	objIndex_t sambaIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(sambaIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	sambaOid = zcfgFeObjNameToObjId(tr181Obj, &sambaIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(sambaOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(sambaOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, sambaOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif // CONFIG_ZCFG_BE_MODULE_SAMBA

#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.UPNP
 *
 *   Related object in TR181:
 *   Device.UPnP.Device
 *	 OID:RDM_OID_UPNP_DEV	
 */
 zcfgRet_t zyExtUPnPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *upnpJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_UPNP_DEV, &objIid, &upnpJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(upnpJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(upnpJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtUPnPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *upnpJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_UPNP_DEV, &objIid, &upnpJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = upnpJobj;
		upnpJobj = NULL;
		upnpJobj = zcfgFeJsonMultiObjAppend(RDM_OID_UPNP_DEV, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(upnpJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(upnpJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_UPNP_DEV, &objIid, upnpJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(upnpJobj);
			return ret;
		}
		json_object_put(upnpJobj);
	}

	return ZCFG_SUCCESS;
}
#endif
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.8021ag
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_8021ag
 */
zcfgRet_t zyExt8021agObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ieeeJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IEEE8021AG, &objIid, &ieeeJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(ieeeJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(ieeeJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExt8021agObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ieeeJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IEEE8021AG, &objIid, &ieeeJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = ieeeJobj;
		ieeeJobj = NULL;
		ieeeJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IEEE8021AG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(ieeeJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ieeeJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IEEE8021AG, &objIid, ieeeJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(ieeeJobj);
			return ret;
		}
		json_object_put(ieeeJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExt8021agObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExt8021agObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  ieeeOid = 0;
	objIndex_t ieeeIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(ieeeIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ieeeOid = zcfgFeObjNameToObjId(tr181Obj, &ieeeIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(ieeeOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExt8021agObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  ieeeOid = 0;
	objIndex_t ieeeIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(ieeeIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ieeeOid = zcfgFeObjNameToObjId(tr181Obj, &ieeeIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(ieeeOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(ieeeOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, ieeeOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

#ifdef ZYXEL_ETHOAM_TMSCTL
zcfgRet_t zyExt8023ahObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ieeeJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IEEE8023AH, &objIid, &ieeeJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(ieeeJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(ieeeJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExt8023ahObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ieeeJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IEEE8023AH, &objIid, &ieeeJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = ieeeJobj;
		ieeeJobj = NULL;
		ieeeJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IEEE8023AH, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(ieeeJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ieeeJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IEEE8023AH, &objIid, ieeeJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(ieeeJobj);
			return ret;
		}
		json_object_put(ieeeJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExt8023ahObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExt8023ahObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  ieeeOid = 0;
	objIndex_t ieeeIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(ieeeIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ieeeOid = zcfgFeObjNameToObjId(tr181Obj, &ieeeIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(ieeeOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExt8023ahObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  ieeeOid = 0;
	objIndex_t ieeeIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(ieeeIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ieeeOid = zcfgFeObjNameToObjId(tr181Obj, &ieeeIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(ieeeOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(ieeeOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, ieeeOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
/* InternetGatewayDevice.X_ZYXEL_EXT.OneConnect */
zcfgRet_t zyExtOneConnectObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *oneConnectJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_ONE_CONNECT, &objIid, &oneConnectJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(oneConnectJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(oneConnectJobj);
	
	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.OneConnect */
zcfgRet_t zyExtOneConnectObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *oneConnectJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_ONE_CONNECT, &objIid, &oneConnectJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = oneConnectJobj;
		oneConnectJobj = NULL;
		oneConnectJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_ONE_CONNECT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(oneConnectJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(oneConnectJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_ONE_CONNECT, &objIid, oneConnectJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(oneConnectJobj);
			return ret;
		}
		json_object_put(oneConnectJobj);
	}
	
	return ZCFG_SUCCESS;
}
#endif // CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT

#ifdef CONFIG_ZCFG_BE_MODULE_SPEED_TEST
/* InternetGatewayDevice.X_ZYXEL_EXT.SpeedTestInfo */
zcfgRet_t zyExtSpeedTestInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *speedTestInfoJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_SPEED_TEST, &objIid, &speedTestInfoJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(speedTestInfoJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(speedTestInfoJobj);
	
	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.SpeedTestInfo */
zcfgRet_t zyExtSpeedTestInfoObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *speedTestInfoJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_SPEED_TEST, &objIid, &speedTestInfoJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = speedTestInfoJobj;
		speedTestInfoJobj = NULL;
		speedTestInfoJobj = zcfgFeJsonMultiObjAppend(RDM_OID_SPEED_TEST, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(speedTestInfoJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(speedTestInfoJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_SPEED_TEST, &objIid, speedTestInfoJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(speedTestInfoJobj);
			return ret;
		}
		json_object_put(speedTestInfoJobj);
	}
	
	return ZCFG_SUCCESS;
}
#endif // CONFIG_ZCFG_BE_MODULE_SPEED_TEST

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
zcfgRet_t zyExtApSteeringObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *apSteeringJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_WIFI_AP_STEERING, &objIid, &apSteeringJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(apSteeringJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(apSteeringJobj);
	
	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.OneConnect */
zcfgRet_t zyExtApSteeringObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *apSteeringJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_AP_STEERING, &objIid, &apSteeringJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = apSteeringJobj;
		apSteeringJobj = NULL;
		apSteeringJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_AP_STEERING, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(apSteeringJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(apSteeringJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_AP_STEERING, &objIid, apSteeringJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(apSteeringJobj);
			return ret;
		}
		json_object_put(apSteeringJobj);
	}
	
	return ZCFG_SUCCESS;
}
#endif // CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING

#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
zcfgRet_t zyExtProximitySensorObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *proximitySensorJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_PROXIMITY_SENSOR, &objIid, &proximitySensorJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(proximitySensorJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(proximitySensorJobj);
	
	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.OneConnect */
zcfgRet_t zyExtProximitySensorObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *proximitySensorJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_PROXIMITY_SENSOR, &objIid, &proximitySensorJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = proximitySensorJobj;
		proximitySensorJobj = NULL;
		proximitySensorJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_PROXIMITY_SENSOR, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(proximitySensorJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(proximitySensorJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_PROXIMITY_SENSOR, &objIid, proximitySensorJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(proximitySensorJobj);
			return ret;
		}
		json_object_put(proximitySensorJobj);
	}
	
	return ZCFG_SUCCESS;
}
#endif // CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR


#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_BANDWIDTH_IMPROVEMENT
/* InternetGatewayDevice.X_ZYXEL_EXT.BandwidthImprovement */
zcfgRet_t zyExtBandwidthImprovementObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *improvementObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *status = NULL;

	printf("%s : Enter\n", __FUNCTION__);
	
	enum {
		Active = 0,
		OFF,
		OFF_NoDSLConnect,
		OFF_NoMobileConnect,
		OFF_DataRate128k
	};
	
	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_BAND_WIDTH_IMPROVEMENT, &objIid, &improvementObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	#if 0 //zyExtBandwidthImprovementObjGet
	printf("%s : Enter - 2 \n", __FUNCTION__);
	#endif
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(improvementObj, paramList->name);
		#if 0 //zyExtBandwidthImprovementObjGet
		printf("%s : Enter - 2 : paramList->name = %s\n", __FUNCTION__, paramList->name);
		#endif
		if(paramValue != NULL){
			//if (!strcmp(paramList->name, "Status")){
			if (0){ //skip
				if (json_object_get_int(paramValue) == Active){
					strcpy(status, "Active");
				}
				else if (json_object_get_int(paramValue) == OFF_NoDSLConnect){
					strcpy(status, "OFF: No DSL Connection");
				}
				else if (json_object_get_int(paramValue) == OFF_NoMobileConnect){
					strcpy(status, "OFF: No Mobile Device");
				}
				else {
					strcpy(status, "OFF");
				}
				//printf("### zyExtBandwidthImprovementObjGet ### GET status = %s\n");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(status));
				
			} 
			else{
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(improvementObj);
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtBandwidthImprovementObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *improvementObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_BAND_WIDTH_IMPROVEMENT, &objIid, &improvementObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = improvementObj;
		improvementObj = NULL;
		improvementObj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_BAND_WIDTH_IMPROVEMENT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(improvementObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(improvementObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_BAND_WIDTH_IMPROVEMENT, &objIid, improvementObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(improvementObj);
			return ret;
		}
		json_object_put(improvementObj);
	}
	
	return ZCFG_SUCCESS;
}

#endif

#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
/*Only for ABXA*/
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
/* InternetGatewayDevice.X_ZYXEL_EXT.WLANConfiguration.i */
zcfgRet_t zyExtWLANConfigurationObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t apIid;
	struct json_object *apSecJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	IID_INIT(apIid);
	apIid.level= 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.WLANConfiguration.%hhu", &apIid.idx[0]);

	/*There should be only one PreSharedKey Object*/
	if(apIid.idx[0] >= 9)
		return ZCFG_NO_MORE_INSTANCE;

	if((ret = feObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, &apSecJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	/*Write parameter value from tr181 objects to tr98 object*/
	if(json_object_get_boolean(json_object_object_get(apSecJobj, "X_ZYXEL_AutoGenPSK"))){
		paramValue = json_object_object_get(apSecJobj, "X_ZYXEL_Auto_PSK");
	}
	else if(strlen(json_object_get_string(json_object_object_get(apSecJobj, "KeyPassphrase")) ) > 0){
		paramValue = json_object_object_get(apSecJobj, "KeyPassphrase");
	}
	else{
		paramValue = json_object_object_get(apSecJobj, "PreSharedKey");
	}

	if(paramValue != NULL) {
		if(apIid.idx[0] <= 4)
			json_object_object_add(*tr98Jobj, "X_ZYXEL_PSK24", JSON_OBJ_COPY(paramValue));
		else
			json_object_object_add(*tr98Jobj, "X_ZYXEL_PSK5", JSON_OBJ_COPY(paramValue));
	}else{
		paramValue = json_object_new_string("");
		json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		json_object_put(paramValue);
	}

	json_object_put(apSecJobj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtWLANConfigurationObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t apIid, onessidIid;
	struct json_object *apSecJobj = NULL;
	struct json_object *PreSharedKey = NULL, *KeyPassphrase = NULL;
	struct json_object *tmpObj = NULL;
	const char *psk_value = NULL;
	bool onessid = false;
	struct json_object *onessidJobj = NULL;

	IID_INIT(apIid);
	apIid.level= 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.WLANConfiguration.%hhu", &apIid.idx[0]);
	if(apIid.idx[0] >= 9)
		return ZCFG_NO_MORE_INSTANCE;

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
	IID_INIT(onessidIid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ONE_SSID, &onessidIid, &onessidJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get ONESSID Fail\n", __FUNCTION__);
		return ret;
	}
	onessid = json_object_get_boolean(json_object_object_get(onessidJobj, "Enable"));
	zcfgFeJsonObjFree(onessidJobj);

	if(apIid.idx[0] == 5 || apIid.idx[0] == 6){//5G main,5G guest 1
		return ZCFG_REQUEST_REJECT;
	}
#endif

	if(apIid.idx[0] <= 4)
		psk_value = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_PSK24"));
	else
		psk_value = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_PSK5"));

	if(!psk_value || strlen(psk_value) == 0)
		return ZCFG_INVALID_PARAM_VALUE;

	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, &apSecJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);
		return ret;
	}

	PreSharedKey = json_object_object_get(apSecJobj, "PreSharedKey");
	KeyPassphrase = json_object_object_get(apSecJobj, "KeyPassphrase");

	if(multiJobj){
		tmpObj = apSecJobj;
		apSecJobj = NULL;
		apSecJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, multiJobj, tmpObj);
	}

	/*Write new parameter value from tr98 object to tr181 objects*/
	if(strlen(psk_value)==64 && PreSharedKey){
		int i=0;
		for(; i<64; i++){
			char c = *(psk_value+i);
			if( !((c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F')) ){
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto finish;
			}
		}
		json_object_object_add(apSecJobj, "PreSharedKey", json_object_new_string(psk_value));
		json_object_object_add(apSecJobj, "KeyPassphrase", json_object_new_string(""));
	}
	else if (KeyPassphrase){
		if((strlen(psk_value) !=0 && strlen(psk_value) <8) || 63 < strlen(psk_value)) {
			ret = ZCFG_INVALID_PARAM_VALUE;
			goto finish;
		}
		json_object_object_add(apSecJobj, "KeyPassphrase", json_object_new_string(psk_value));
	}else{
		ret = ZCFG_INVALID_PARAM_VALUE;
		goto finish;
	}

	/*Set Device.WiFi.AccessPoint.i.Security*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, apSecJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(apSecJobj);
			return ret;
		}
		json_object_put(apSecJobj);
	}

	return ZCFG_SUCCESS;

finish:
    if(multiJobj == NULL)
        json_object_put(apSecJobj);
	else
		json_object_put(tmpObj);

	strncpy(paramfault, "PreSharedKey", sizeof(paramfault));
    return ret;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.AirtimeFairness*/
zcfgRet_t zyExtAirtimeFairnessObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t radioIid;
	struct json_object *radioJobj = NULL;
	struct json_object *paramValue = NULL;

	IID_INIT(radioIid);
	radioIid.level = 1;
	radioIid.idx[0] = 2; //For 5G
	if((ret = feObjJsonGet(RDM_OID_WIFI_RADIO, &radioIid, &radioJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.Radio Fail\n", __FUNCTION__);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	if(json_object_object_get(radioJobj, "X_ZYXEL_AirtimeFairness")){
		paramValue = json_object_object_get(radioJobj, "X_ZYXEL_AirtimeFairness");
	}else
		zcfgLog(ZCFG_LOG_ERR, "%s :X_ZYXEL_AirtimeFairness Not found\n", __FUNCTION__);

	if(paramValue != NULL) {
		json_object_object_add(*tr98Jobj, "Enable", JSON_OBJ_COPY(paramValue));
	}

	printf("X_ZYXEL_AirtimeFairness = %s", json_object_to_json_string(*tr98Jobj));

	json_object_put(radioJobj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtAirtimeFairnessObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t radioIid;
	struct json_object *radioJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;

	IID_INIT(radioIid);
	radioIid.level = 1;
	radioIid.idx[0] = 2; //For 5G
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &radioIid, &radioJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.Radio Fail\n", __FUNCTION__);
		return ret;
	}

	if(multiJobj){
		tmpObj = radioJobj;
		radioJobj = NULL;
		radioJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_RADIO, &radioIid, multiJobj, tmpObj);
	}

	paramValue = json_object_object_get(tr98Jobj, "Enable");
	/*Write new parameter value from tr98 object to tr181 objects*/
	if(paramValue != NULL){
		json_object_object_add(radioJobj, "X_ZYXEL_AirtimeFairness", JSON_OBJ_COPY(paramValue));
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_RADIO, &radioIid, radioJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(radioJobj);
			return ret;
		}
		json_object_put(radioJobj);
	}

    return ZCFG_SUCCESS;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

/* InternetGatewayDevice.User.i */
zcfgRet_t zyExtAccountResetPasswordObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char accountResetPasswordPath[128] = "InternetGatewayDevice.User.%hhu";	
	uint32_t loginCfgGpAccoutOid = 0;
	objIndex_t loginCfgGpAccoutIid;
	struct json_object *loginCfgGpAccoutObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *default_password = NULL;
	int testint = 0;
	
	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(loginCfgGpAccoutIid);
	loginCfgGpAccoutIid.level = 2;
	loginCfgGpAccoutIid.idx[0] = 2; //admin group

	if (sscanf(tr98FullPathName, accountResetPasswordPath, &loginCfgGpAccoutIid.idx[1]) != 1){
		return ZCFG_INVALID_OBJECT;
	}

#if 1 //for user group
	if (loginCfgGpAccoutIid.idx[1] > 4){ //for user account.
		loginCfgGpAccoutIid.idx[0] = 3; //user group
		loginCfgGpAccoutIid.idx[1] = loginCfgGpAccoutIid.idx[1] - 4;
	}
	#endif

	//printf("### zyExtAccountResetPasswordObjGet ###  &loginCfgGpAccoutIid.idx[0] = %d\n", loginCfgGpAccoutIid.idx[0]);
	//printf("### zyExtAccountResetPasswordObjGet ###  &loginCfgGpAccoutIid.idx[1] = %d\n", loginCfgGpAccoutIid.idx[1]);
	
	loginCfgGpAccoutOid = RDM_OID_ZY_LOG_CFG_GP_ACCOUNT;

	if((ret = feObjJsonGet(loginCfgGpAccoutOid, &loginCfgGpAccoutIid, &loginCfgGpAccoutObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 rule object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
	
			if (!strcmp(paramList->name, "Username")){
			paramValue = json_object_object_get(loginCfgGpAccoutObj, paramList->name);
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

		if (!strcmp(paramList->name, "X_ZYXEL_ResetDefaultPassword")){
			paramValue = json_object_object_get(loginCfgGpAccoutObj, "ResetDefaultPassword");
				json_object_object_add(*tr98Jobj, "X_ZYXEL_ResetDefaultPassword", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(loginCfgGpAccoutObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtAccountResetPasswordObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char accountResetPasswordPath[128] = "InternetGatewayDevice.User.%hhu";	
	uint32_t  loginCfgGpAccoutOid = 0;
	objIndex_t loginCfgGpAccoutIid;
	struct json_object *loginCfgGpAccoutObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	
	IID_INIT(loginCfgGpAccoutIid);
	loginCfgGpAccoutIid.level = 2;
	loginCfgGpAccoutIid.idx[0] = 2; //admin group
	
	if(sscanf(tr98FullPathName, accountResetPasswordPath, &loginCfgGpAccoutIid.idx[1]) != 1) return ZCFG_INVALID_OBJECT;

#if 1 //for user group
	if (loginCfgGpAccoutIid.idx[1] > 4){ //for user account.
		loginCfgGpAccoutIid.idx[0] = 3; //user group
		loginCfgGpAccoutIid.idx[1] = loginCfgGpAccoutIid.idx[1] - 4;
	}
#endif

	loginCfgGpAccoutOid = RDM_OID_ZY_LOG_CFG_GP_ACCOUNT;

	if((ret = zcfgFeObjJsonGet(loginCfgGpAccoutOid, &loginCfgGpAccoutIid, &loginCfgGpAccoutObj)) != ZCFG_SUCCESS) return ret;
	
	if(multiJobj){
		tmpObj = loginCfgGpAccoutObj;
		loginCfgGpAccoutObj = NULL;
		loginCfgGpAccoutObj = zcfgFeJsonMultiObjAppend(loginCfgGpAccoutOid, &loginCfgGpAccoutIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(loginCfgGpAccoutObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(loginCfgGpAccoutObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		/*Write new parameter value from tr98 object to tr181 objects*/
		if (!strcmp(paramList->name, "X_ZYXEL_ResetDefaultPassword")){
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			if(paramValue != NULL) {
				tr181ParamValue = json_object_object_get(loginCfgGpAccoutObj, "ResetDefaultPassword");
				if(tr181ParamValue != NULL) {
					json_object_object_add(loginCfgGpAccoutObj, "ResetDefaultPassword", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}	
		}
		
		paramList++;
	} /*Edn while*/

	/*Set */
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(loginCfgGpAccoutOid, &loginCfgGpAccoutIid, loginCfgGpAccoutObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(loginCfgGpAccoutObj);
			return ret;
		}
		json_object_put(loginCfgGpAccoutObj);
	}

	return ZCFG_SUCCESS;
}
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.PrintServer
 *
 *   Related object in TR181: Device.X_ZYXEL_Ipp_Cfg
 *
 */
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
zcfgRet_t zyExtIppPrintServObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *printServJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_IPP_CFG, &objIid, &printServJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(printServJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(printServJobj);
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtIppPrintServObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *printServJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_IPP_CFG, &objIid, &printServJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = printServJobj;
		printServJobj = NULL;
		printServJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_IPP_CFG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(printServJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(printServJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_IPP_CFG, &objIid, printServJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(printServJobj);
			return ret;
		}
		json_object_put(printServJobj);
	}
	
	return ZCFG_SUCCESS;
}
#endif // CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG

#ifdef ZYXEL_LANDING_PAGE_FEATURE
/* "InternetGatewayDevice.X_ZYXEL_EXT.LandingPage" */
zcfgRet_t zyExtLandingPageObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *landingPageJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_LANDING_PAGE, &objIid, &landingPageJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(landingPageJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(landingPageJobj);
	return ZCFG_SUCCESS;
}

/* "InternetGatewayDevice.X_ZYXEL_EXT.LandingPage" */
zcfgRet_t zyExtLandingPageObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *landingPageJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_LANDING_PAGE, &objIid, &landingPageJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = landingPageJobj;
		landingPageJobj = NULL;
		landingPageJobj = zcfgFeJsonMultiObjAppend(RDM_OID_LANDING_PAGE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(landingPageJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(landingPageJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_LANDING_PAGE, &objIid, landingPageJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(landingPageJobj);
			return ret;
		}
		json_object_put(landingPageJobj);
	}
	return ZCFG_SUCCESS;
}
#endif
#if defined(ZyXEL_Online_Firmware_Upgrade) || defined(ZYXEL_WEB_GUI_ONLINE_FW_UPGRADE_VIA_URL)
/* "InternetGatewayDevice.X_ZYXEL_EXT.OnlineFWUgrade" */
zcfgRet_t zyExtOnlineFWUgradeObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *landingPageJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ONLINE_F_W_UPGRADE, &objIid, &landingPageJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(landingPageJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(landingPageJobj);
	return ZCFG_SUCCESS;
}

/* "InternetGatewayDevice.X_ZYXEL_EXT.OnlineFWUgrade" */
zcfgRet_t zyExtOnlineFWUgradeObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *landingPageJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ONLINE_F_W_UPGRADE, &objIid, &landingPageJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = landingPageJobj;
		landingPageJobj = NULL;
		landingPageJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ONLINE_F_W_UPGRADE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(landingPageJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(landingPageJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ONLINE_F_W_UPGRADE, &objIid, landingPageJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(landingPageJobj);
			return ret;
		}
		json_object_put(landingPageJobj);
	}
	return ZCFG_SUCCESS;
}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
/* "InternetGatewayDevice.X_ZYXEL_EXT.SFP_Management" */
zcfgRet_t zyExtSfpMgmObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *sfpMgmtJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_SFP_MANAGEMENT, &objIid, &sfpMgmtJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(sfpMgmtJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(sfpMgmtJobj);
	return ZCFG_SUCCESS;
}

/* "InternetGatewayDevice.X_ZYXEL_EXT.SFP_Management" */
zcfgRet_t zyExtSfpMgmObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *sfpMgmtJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_SFP_MANAGEMENT, &objIid, &sfpMgmtJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = sfpMgmtJobj;
		sfpMgmtJobj = NULL;
		sfpMgmtJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_SFP_MANAGEMENT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(sfpMgmtJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(sfpMgmtJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_SFP_MANAGEMENT, &objIid, sfpMgmtJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(sfpMgmtJobj);
			return ret;
		}
		json_object_put(sfpMgmtJobj);
	}
	return ZCFG_SUCCESS;
}
#endif


/* "InternetGatewayDevice.X_ZYXEL_EXT.DNS" */
zcfgRet_t zyExtDnsObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *Jobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	const char *X_ZYXEL_BoundInterfaceList = NULL;
	int len = 0;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_DNS, &objIid, &Jobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(Jobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "X_ZYXEL_BoundInterfaceList")) {
					X_ZYXEL_BoundInterfaceList = json_object_get_string(paramValue);
					
					if(X_ZYXEL_BoundInterfaceList) {
						char wanIpConnName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
						char tr98IntfList[TR98_MAX_OBJ_NAME_LENGTH * 5] = {0};
						char interface_str[256] = {0};
						char *ipIfaceName = NULL, *tmp = NULL;
						
						sprintf(interface_str, "%s", X_ZYXEL_BoundInterfaceList);
						ipIfaceName = strtok_r(interface_str, ",", &tmp);
						while (ipIfaceName != NULL) {
							memset(wanIpConnName, 0, TR98_MAX_OBJ_NAME_LENGTH);
							
							if (zcfgFe181To98MappingNameGet(ipIfaceName, wanIpConnName) != ZCFG_SUCCESS) {
								ipIfaceName = strtok_r(NULL, "," ,&tmp);
								continue;
							}
							strcat(tr98IntfList, wanIpConnName);
							strcat(tr98IntfList, ",");
							ipIfaceName = strtok_r(NULL, ",", &tmp);
						}
						len = strlen(tr98IntfList);
						if(tr98IntfList[len-1] == ',')
							tr98IntfList[len-1] = '\0';
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98IntfList));
						paramList++;
						continue;
					}
					
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
					paramList++;
					continue;
			}
			
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(Jobj);

	return ZCFG_SUCCESS;
}

/* "InternetGatewayDevice.X_ZYXEL_EXT.DNS" */
zcfgRet_t zyExtDnsObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *Jobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	const char *X_ZYXEL_BoundInterfaceList = NULL;
	int len;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS, &objIid, &Jobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = Jobj;
		Jobj = NULL;
		Jobj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name, "X_ZYXEL_BoundInterfaceList")) {
				X_ZYXEL_BoundInterfaceList = json_object_get_string(paramValue);
				
				if(X_ZYXEL_BoundInterfaceList) {
					char tr181IntfList[256] = {0};
					char ipIfaceName[32] = {0};
					char interface_str[TR98_MAX_OBJ_NAME_LENGTH * 10] = {0};
					char *wanIpConnName = NULL, *tmp = NULL;
					
					sprintf(interface_str, "%s", X_ZYXEL_BoundInterfaceList);
					wanIpConnName = strtok_r(interface_str, ",", &tmp);
					while (wanIpConnName != NULL) {
						memset(ipIfaceName, 0, 32);
						
						if (zcfgFe98To181MappingNameGet(wanIpConnName, ipIfaceName) != ZCFG_SUCCESS) {
							wanIpConnName = strtok_r(NULL, "," ,&tmp);
							continue;
						}
						if(strstr(ipIfaceName, "PPP.Interface.")){
							zcfgFeTr181IfaceStackHigherLayerGet(ipIfaceName, ipIfaceName);
						}
						strcat(tr181IntfList, ipIfaceName);
						strcat(tr181IntfList, ",");
						wanIpConnName = strtok_r(NULL, ",", &tmp);
					}
					len = strlen(tr181IntfList);
					if(tr181IntfList[len-1] == ',')
						tr181IntfList[len-1] = '\0';
					json_object_object_add(Jobj, paramList->name, json_object_new_string(tr181IntfList));
					paramList++;
					continue;
				}
				
				json_object_object_add(Jobj, paramList->name, json_object_new_string(""));
				paramList++;
				continue;
			}
			
			tr181ParamValue = json_object_object_get(Jobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DNS, &objIid, Jobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(Jobj);
			return ret;
		}
		json_object_put(Jobj);
	}

	return ZCFG_SUCCESS;
}

/* "InternetGatewayDevice.X_ZYXEL_EXT.FeatureFlag" */
zcfgRet_t zyExtFeatureFlagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *featureFlagJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_FEATURE_FLAG, &objIid, &featureFlagJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(featureFlagJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(featureFlagJobj);

	return ZCFG_SUCCESS;
}

/* "InternetGatewayDevice.X_ZYXEL_EXT.FeatureFlag" */
zcfgRet_t zyExtFeatureFlagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *featureFlagJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_FEATURE_FLAG, &objIid, &featureFlagJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = featureFlagJobj;
		featureFlagJobj = NULL;
		featureFlagJobj = zcfgFeJsonMultiObjAppend(RDM_OID_FEATURE_FLAG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(featureFlagJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(featureFlagJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_FEATURE_FLAG, &objIid, featureFlagJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(featureFlagJobj);
			return ret;
		}
		json_object_put(featureFlagJobj);
	}

	return ZCFG_SUCCESS;
}

/* "InternetGatewayDevice.X_ZYXEL_EXT.GUI_CUSTOMIZATION" */
zcfgRet_t zyExtCustomizationObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *customizationJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_GUI_CUSTOMIZATION, &objIid, &customizationJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(customizationJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(customizationJobj);
	return ZCFG_SUCCESS;
}

/* "InternetGatewayDevice.X_ZYXEL_EXT.GUI_CUSTOMIZATION" */
zcfgRet_t zyExtCustomizationObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *customizationJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_GUI_CUSTOMIZATION, &objIid, &customizationJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = customizationJobj;
		customizationJobj = NULL;
		customizationJobj = zcfgFeJsonMultiObjAppend(RDM_OID_GUI_CUSTOMIZATION, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(customizationJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(customizationJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_GUI_CUSTOMIZATION, &objIid, customizationJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(customizationJobj);
			return ret;
		}
		json_object_put(customizationJobj);
	}
	return ZCFG_SUCCESS;
}

#ifdef ZYXEL_IPV6_MAP
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_MAP
 *
 *   Related object in TR181:
 *   Device.MAP
 *   RDM_OID_MAP
 */

zcfgRet_t zyExtMAPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *mapJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_MAP, &objIid, &mapJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(mapJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_ERR, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(mapJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMAPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *mapJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_MAP, &objIid, &mapJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = mapJobj;
		mapJobj = NULL;
		mapJobj = zcfgFeJsonMultiObjAppend(RDM_OID_MAP, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(mapJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(mapJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		zcfgLog(ZCFG_LOG_ERR, "Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MAP, &objIid, mapJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(mapJobj);
			return ret;
		}
		json_object_put(mapJobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_MAP.Domain.i
 *
 *   Related object in TR181:
 *   Device.MAP.Domain.i
 *   RDM_OID_MAP_DOMAIN
 */
zcfgRet_t zyExtMAPDomainObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *mapDomainJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_MAP.Domain.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_MAP_DOMAIN, &objIid, &mapDomainJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(mapDomainJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_ERR, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(mapDomainJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMAPDomainObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t mapDomainIid;
	struct json_object *mapDomainObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(mapDomainIid);
	mapDomainIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_MAP.Domain.%hhu", &mapDomainIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_MAP_DOMAIN, &mapDomainIid, &mapDomainObj)) != ZCFG_SUCCESS)
		return ret;

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);

		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(mapDomainObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(mapDomainObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_ERR, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	} /*End while*/

	/*Set */
	if(multiJobj){
		tmpObj = mapDomainObj;
		mapDomainObj = NULL;
		mapDomainObj = zcfgFeJsonMultiObjAppend(RDM_OID_MAP_DOMAIN, &mapDomainIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MAP_DOMAIN, &mapDomainIid, mapDomainObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(mapDomainObj);
			return ret;
		}
		json_object_put(mapDomainObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMAPDomainObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t mapDomainIid;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter \n", __FUNCTION__);

	IID_INIT(mapDomainIid);
 	if((ret = zcfgFeObjStructAdd(RDM_OID_MAP_DOMAIN, &mapDomainIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add X_ZYXEL_MAP.Domain.%hhu. Fail.\n", __FUNCTION__, mapDomainIid.idx[0]);
		return ret;
	}
	*idx = mapDomainIid.idx[0];

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMAPDomainObjDel(char *tr98FullPathName)
{
	objIndex_t mapDomainIid;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	IID_INIT(mapDomainIid);
	mapDomainIid.level = 1;
	if(sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_MAP.Domain.%hhu", &mapDomainIid.idx[0]) != 1)
		return ZCFG_INVALID_OBJECT;

	return zcfgFeObjStructDel(RDM_OID_MAP_DOMAIN, &mapDomainIid, NULL);
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_MAP.Domain.i.Rule.i
 *
 *   Related object in TR181:
 *   Device.MAP.Domain.i.Rule.i
 *   RDM_OID_MAP_DOMAIN_RULE
 */
zcfgRet_t zyExtMAPDomainRuleObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t mapDomainRuleIid;
	struct json_object *mapDomainRuleObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(mapDomainRuleIid);
	mapDomainRuleIid.level = 2;

	if(sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_MAP.Domain.%hhu.Rule.%hhu", &mapDomainRuleIid.idx[0], &mapDomainRuleIid.idx[1]) != 2)
		return ZCFG_INVALID_OBJECT;

	if((ret = feObjJsonGet(RDM_OID_MAP_DOMAIN_RULE, &mapDomainRuleIid, &mapDomainRuleObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 rule object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(mapDomainRuleObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_ERR, "Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(mapDomainRuleObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMAPDomainRuleObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t mapDomainRuleIid;
	struct json_object *mapDomainRuleObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_ERR, "Enter %s\n", __FUNCTION__);

	IID_INIT(mapDomainRuleIid);
	mapDomainRuleIid.level = 2;
	if(sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_MAP.Domain.%hhu.Rule.%hhu", &mapDomainRuleIid.idx[0], &mapDomainRuleIid.idx[1]) != 2) return ZCFG_INVALID_OBJECT;

	if((ret = zcfgFeObjJsonGet(RDM_OID_MAP_DOMAIN_RULE, &mapDomainRuleIid, &mapDomainRuleObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = mapDomainRuleObj;
		mapDomainRuleObj = NULL;
		mapDomainRuleObj = zcfgFeJsonMultiObjAppend(RDM_OID_MAP_DOMAIN_RULE, &mapDomainRuleIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(mapDomainRuleObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(mapDomainRuleObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		paramList++;
	} /*End while*/

	/*Set */
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MAP_DOMAIN_RULE, &mapDomainRuleIid, mapDomainRuleObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(mapDomainRuleObj);
			return ret;
		}
		json_object_put(mapDomainRuleObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMAPDomainRuleObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t mapDomainRuleIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(mapDomainRuleIid);
	mapDomainRuleIid.level = 1;
	if(sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_MAP.Domain.%hhu.Rule", &mapDomainRuleIid.idx[0]) != 1)
		return ZCFG_INVALID_OBJECT;

	if((ret = zcfgFeObjStructAdd(RDM_OID_MAP_DOMAIN_RULE, &mapDomainRuleIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add X_ZYXEL_MAP.Domain.%hhu.Rule. Fail.\n", __FUNCTION__, mapDomainRuleIid.idx[0]);
		return ret;
	}
	*idx = mapDomainRuleIid.idx[1];

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMAPDomainRuleObjDel(char *tr98FullPathName)
{
	objIndex_t mapDomainRuleIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(mapDomainRuleIid);
	mapDomainRuleIid.level = 2;
	if(sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_MAP.Domain.%hhu.Rule.%hhu", &mapDomainRuleIid.idx[0], &mapDomainRuleIid.idx[1]) != 2)
		return ZCFG_INVALID_OBJECT;

	return zcfgFeObjStructDel(RDM_OID_MAP_DOMAIN_RULE, &mapDomainRuleIid, NULL);
}
#endif //End ZYXEL_IPV6_MAP

#ifdef ZYXEL_WIFI_HW_BUTTON_SWITCH
/* InternetGatewayDevice.X_TT.Configuration.Buttons.WiFi */
zcfgRet_t zyExtHwWiFiButtonObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *HwWiFiButtonObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_WI_FI, &objIid, &HwWiFiButtonObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(HwWiFiButtonObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(HwWiFiButtonObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtHwWiFiButtonObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *HwWiFiButtonObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_WI_FI, &objIid, &HwWiFiButtonObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = HwWiFiButtonObj;
		HwWiFiButtonObj = NULL;
		HwWiFiButtonObj = zcfgFeJsonMultiObjAppend(RDM_OID_WI_FI, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(HwWiFiButtonObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(HwWiFiButtonObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WI_FI, &objIid, HwWiFiButtonObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(HwWiFiButtonObj);
			return ret;
		}
		json_object_put(HwWiFiButtonObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtHwWiFiButtonObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtHwWiFiButtonObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t HwWiFiButtonOid = 0;
	objIndex_t HwWiFiButtonIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(HwWiFiButtonIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	HwWiFiButtonOid = zcfgFeObjNameToObjId(tr181Obj, &HwWiFiButtonIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(HwWiFiButtonOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtHwWiFiButtonObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t HwWiFiButtonOid = 0;
	objIndex_t HwWiFiButtonIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(HwWiFiButtonIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	HwWiFiButtonOid = zcfgFeObjNameToObjId(tr181Obj, &HwWiFiButtonIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(HwWiFiButtonOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(HwWiFiButtonOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, HwWiFiButtonOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif

#ifdef ZYXEL_TAAAG_TR69ALG////MSTC_TTALG, RichardFeng
/* InternetGatewayDevice.X_TT.Configuration.H323_ALG */
zcfgRet_t zyTTH323AlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *H323AlgObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_NAT, &objIid, &H323AlgObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(H323AlgObj, "X_ZYXEL_H323_ALG");
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(H323AlgObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTH323AlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *H323AlgObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
   // bool setQuickBri = false;
	const char *NowVal = NULL, *SetVal = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT, &objIid, &H323AlgObj)) != ZCFG_SUCCESS){
		return ret;
	}

    
	if(multiJobj){
		tmpObj = H323AlgObj;
		H323AlgObj = NULL;
		H323AlgObj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(H323AlgObj, "X_ZYXEL_H323_ALG");
			if(tr181ParamValue != NULL){
				json_object_object_add(H323AlgObj, "X_ZYXEL_H323_ALG", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT, &objIid, H323AlgObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(H323AlgObj);
			return ret;
		}
		json_object_put(H323AlgObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTH323AlgObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTH323AlgObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  H323AlgOid = 0;
	objIndex_t H323AlgIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(H323AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	H323AlgOid = zcfgFeObjNameToObjId(tr181Obj, &H323AlgIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(H323AlgOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTH323AlgObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  H323AlgOid = 0;
	objIndex_t H323AlgIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(H323AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	H323AlgOid = zcfgFeObjNameToObjId(tr181Obj, &H323AlgIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(H323AlgOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(H323AlgOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, H323AlgOid, paramList->name);
		}
		break;
	}
	
	return ret;
}



/* InternetGatewayDevice.X_TT.Configuration.H323_ALG */
zcfgRet_t zyTTSIPAlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AlgObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_NAT, &objIid, &AlgObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(AlgObj, "X_ZYXEL_SIP_ALG");
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(AlgObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTSIPAlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AlgObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
   // bool setQuickBri = false;
	const char *NowVal = NULL, *SetVal = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT , &objIid, &AlgObj)) != ZCFG_SUCCESS){
		return ret;
	}


    
	if(multiJobj){
		tmpObj = AlgObj;
		AlgObj = NULL;
		AlgObj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT , &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(AlgObj, "X_ZYXEL_SIP_ALG");
			if(tr181ParamValue != NULL){
				json_object_object_add(AlgObj, "X_ZYXEL_SIP_ALG", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT , &objIid, AlgObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(AlgObj);
			return ret;
		}
		json_object_put(AlgObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTSIPAlgObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTSIPAlgObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  AlgOid = 0;
	objIndex_t AlgIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AlgOid = zcfgFeObjNameToObjId(tr181Obj, &AlgIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(AlgOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTSIPAlgObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  AlgOid = 0;
	objIndex_t AlgIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AlgOid = zcfgFeObjNameToObjId(tr181Obj, &AlgIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(AlgOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(AlgOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, AlgOid, paramList->name);
		}
		break;
	}
	
	return ret;
}



/* InternetGatewayDevice.X_TT.Configuration.RTSP_ALG */
zcfgRet_t zyTTRTSPAlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AlgObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_NAT, &objIid, &AlgObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(AlgObj, "X_ZYXEL_RTSP_ALG");
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(AlgObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTRTSPAlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AlgObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
   // bool setQuickBri = false;
	const char *NowVal = NULL, *SetVal = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT, &objIid, &AlgObj)) != ZCFG_SUCCESS){
		return ret;
	}

/*    NowVal = json_object_get_string(json_object_object_get(H323AlgObj, "Enable"));  
	SetVal = json_object_get_string(json_object_object_get(tr98Jobj, "Enable"));
		    if(strcmp(NowVal,SetVal)){
                 setQuickBri = true;
			}
			else{
				setQuickBri = false;
			}*/
    
	if(multiJobj){
		tmpObj = AlgObj;
		AlgObj = NULL;
		AlgObj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(AlgObj, "X_ZYXEL_RTSP_ALG");
			if(tr181ParamValue != NULL){
				json_object_object_add(AlgObj, "X_ZYXEL_RTSP_ALG", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT, &objIid, AlgObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(AlgObj);
			return ret;
		}
		json_object_put(AlgObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTRTSPAlgObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTRTSPAlgObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  AlgOid = 0;
	objIndex_t AlgIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AlgOid = zcfgFeObjNameToObjId(tr181Obj, &AlgIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(AlgOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTRTSPAlgObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  AlgOid = 0;
	objIndex_t AlgIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AlgOid = zcfgFeObjNameToObjId(tr181Obj, &AlgIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(AlgOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(AlgOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, AlgOid, paramList->name);
		}
		break;
	}
	
	return ret;
}





/* InternetGatewayDevice.X_TT.Configuration.PPTP_ALG */
zcfgRet_t zyTTPPTPAlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AlgObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_NAT, &objIid, &AlgObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(AlgObj, "X_ZYXEL_PPTP_ALG");
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(AlgObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTPPTPAlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AlgObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
   // bool setQuickBri = false;
	const char *NowVal = NULL, *SetVal = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT, &objIid, &AlgObj)) != ZCFG_SUCCESS){
		return ret;
	}

/*    NowVal = json_object_get_string(json_object_object_get(H323AlgObj, "Enable"));  
	SetVal = json_object_get_string(json_object_object_get(tr98Jobj, "Enable"));
		    if(strcmp(NowVal,SetVal)){
                 setQuickBri = true;
			}
			else{
				setQuickBri = false;
			}*/
    
	if(multiJobj){
		tmpObj = AlgObj;
		AlgObj = NULL;
		AlgObj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(AlgObj, "X_ZYXEL_PPTP_ALG");
			if(tr181ParamValue != NULL){
				json_object_object_add(AlgObj, "X_ZYXEL_PPTP_ALG", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT, &objIid, AlgObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(AlgObj);
			return ret;
		}
		json_object_put(AlgObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTPPTPAlgObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTPPTPAlgObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  AlgOid = 0;
	objIndex_t AlgIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AlgOid = zcfgFeObjNameToObjId(tr181Obj, &AlgIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(AlgOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTPPTPAlgObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  AlgOid = 0;
	objIndex_t AlgIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AlgOid = zcfgFeObjNameToObjId(tr181Obj, &AlgIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(AlgOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(AlgOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, AlgOid, paramList->name);
		}
		break;
	}
	
	return ret;
}





/* InternetGatewayDevice.X_TT.Configuration.IPSec_ALG */
zcfgRet_t zyTTIPSecAlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AlgObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_NAT, &objIid, &AlgObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(AlgObj, "X_ZYXEL_IPSEC_ALG");
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(AlgObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTIPSecAlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AlgObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
   // bool setQuickBri = false;
	const char *NowVal = NULL, *SetVal = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT, &objIid, &AlgObj)) != ZCFG_SUCCESS){
		return ret;
	}

/*    NowVal = json_object_get_string(json_object_object_get(H323AlgObj, "Enable"));  
	SetVal = json_object_get_string(json_object_object_get(tr98Jobj, "Enable"));
		    if(strcmp(NowVal,SetVal)){
                 setQuickBri = true;
			}
			else{
				setQuickBri = false;
			}*/
    
	if(multiJobj){
		tmpObj = AlgObj;
		AlgObj = NULL;
		AlgObj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(AlgObj, "X_ZYXEL_IPSEC_ALG");
			if(tr181ParamValue != NULL){
				json_object_object_add(AlgObj, "X_ZYXEL_IPSEC_ALG", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT, &objIid, AlgObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(AlgObj);
			return ret;
		}
		json_object_put(AlgObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTIPSecAlgObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTIPSecAlgObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  AlgOid = 0;
	objIndex_t AlgIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AlgOid = zcfgFeObjNameToObjId(tr181Obj, &AlgIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(AlgOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTIPSecAlgObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  AlgOid = 0;
	objIndex_t AlgIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AlgOid = zcfgFeObjNameToObjId(tr181Obj, &AlgIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(AlgOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(AlgOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, AlgOid, paramList->name);
		}
		break;
	}
	
	return ret;
}





/* InternetGatewayDevice.X_TT.Configuration.L2TP_ALG */
zcfgRet_t zyTTL2TPAlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AlgObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_NAT, &objIid, &AlgObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(AlgObj, "X_ZYXEL_L2TP_ALG");
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(AlgObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTL2TPAlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AlgObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
   // bool setQuickBri = false;
	const char *NowVal = NULL, *SetVal = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT, &objIid, &AlgObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = AlgObj;
		AlgObj = NULL;
		AlgObj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(AlgObj, "X_ZYXEL_L2TP_ALG");
			if(tr181ParamValue != NULL){
				json_object_object_add(AlgObj, "X_ZYXEL_L2TP_ALG", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT, &objIid, AlgObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(AlgObj);
			return ret;
		}
		json_object_put(AlgObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTL2TPAlgObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTL2TPAlgObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  AlgOid = 0;
	objIndex_t AlgIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AlgOid = zcfgFeObjNameToObjId(tr181Obj, &AlgIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(AlgOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTL2TPAlgObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  AlgOid = 0;
	objIndex_t AlgIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(AlgIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AlgOid = zcfgFeObjNameToObjId(tr181Obj, &AlgIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(AlgOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(AlgOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, AlgOid, paramList->name);
		}
		break;
	}
	
	return ret;
}






/* InternetGatewayDevice.X_TT.Configuration.IGMPProxy */
zcfgRet_t zyTTIGMPProxyObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *IGMPProxyObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	const char *IGMPInterfaceList = NULL;
	objIndex_t ipIfaceIid;
	rdm_IpIface_t *ipIface = NULL;
	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_IGMP, &objIid, &IGMPProxyObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(IGMPProxyObj, "Interface");
		if(paramValue != NULL){
			IGMPInterfaceList = json_object_get_string(paramValue);
			if(!strlen(IGMPInterfaceList))
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			else{
				int notFound = 0;
				int count = 1;
				IID_INIT(ipIfaceIid);
				while((ret = zcfgFeObjStructGetNext(RDM_OID_IP_IFACE, &ipIfaceIid, (void **)&ipIface)) == ZCFG_SUCCESS) {
					char currInterface[32] = {0};
					//printf("%s : --%d--\n", __FUNCTION__,ipIfaceIid.idx[0]);
					if(strstr(ipIface->X_TAAAB_SrvName, "IPTV_") != NULL){
						sprintf(currInterface,"IP.Interface.%d",ipIfaceIid.idx[0]);
						//printf("%s : --%s--\n", __FUNCTION__,currInterface);
						if(strstr(IGMPInterfaceList, currInterface) == NULL){
							json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
							notFound = 1;
							zcfgFeObjStructFree(ipIface);
							break;
						}
					}
					count++;
					zcfgFeObjStructFree(ipIface);
				}
				if(!notFound)
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
			}
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(IGMPProxyObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTIGMPProxyObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *IGMPProxyObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	objIndex_t ipIfaceIid;
	rdm_IpIface_t *ipIface = NULL;
	const char *NowVal = NULL, *SetVal = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_IGMP, &objIid, &IGMPProxyObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = IGMPProxyObj;
		IGMPProxyObj = NULL;
		IGMPProxyObj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_IGMP, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(IGMPProxyObj, "Interface");
			if(tr181ParamValue != NULL){
				if(json_object_get_boolean(paramValue) == false)
					json_object_object_add(IGMPProxyObj, "Interface", json_object_new_string(""));
				else{
					int notFound = 0;
					char IGMPInterfaceList[128] = {0};
					IID_INIT(ipIfaceIid);
					while((ret = zcfgFeObjStructGetNext(RDM_OID_IP_IFACE, &ipIfaceIid, (void **)&ipIface)) == ZCFG_SUCCESS) {
						char currInterface[32] = {0};
						if(strstr(ipIface->X_TAAAB_SrvName, "IPTV_") != NULL){
							if(!notFound){
								sprintf(currInterface,"IP.Interface.%d",ipIfaceIid.idx[0]);
								notFound = 1;
							}
							else
								sprintf(currInterface,",IP.Interface.%d",ipIfaceIid.idx[0]);
							strcat(IGMPInterfaceList,currInterface);
						}						
						zcfgFeObjStructFree(ipIface);
					}
					json_object_object_add(IGMPProxyObj, "Interface", json_object_new_string(IGMPInterfaceList));
				}
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_IGMP, &objIid, IGMPProxyObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(IGMPProxyObj);
			return ret;
		}
		json_object_put(IGMPProxyObj);
	}

	IID_INIT(ipIfaceIid);
	while((ret = zcfgFeObjStructGetNext(RDM_OID_IP_IFACE, &ipIfaceIid, (void **)&ipIface)) == ZCFG_SUCCESS) {
		printf("%s : --%d--\n", __FUNCTION__,ipIfaceIid.idx[0]);
		if(strstr(ipIface->X_TAAAB_SrvName, "IPTV_") != NULL){
			zcfgFeObjStructSet(RDM_OID_IP_IFACE, &ipIfaceIid, (void *)ipIface, NULL);
		}
		zcfgFeObjStructFree(ipIface);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTIGMPProxyObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTIGMPProxyObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  IGMPProxyOid = 0;
	objIndex_t IGMPProxyIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(IGMPProxyIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	IGMPProxyOid = zcfgFeObjNameToObjId(tr181Obj, &IGMPProxyIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(IGMPProxyOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTIGMPProxyObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  IGMPProxyOid = 0;
	objIndex_t IGMPProxyIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(IGMPProxyIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	IGMPProxyOid = zcfgFeObjNameToObjId(tr181Obj, &IGMPProxyIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(IGMPProxyOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(IGMPProxyOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, IGMPProxyOid, paramList->name);
		}
		break;
	}
	
	return ret;
}





/* InternetGatewayDevice.X_TT.Configuration.SMTP */
zcfgRet_t zyTTSMTPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *SMTPObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);
		/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 8;
	if((ret = feObjJsonGet(RDM_OID_REMO_SRV, &objIid, &SMTPObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(SMTPObj, "Mode");
		if(paramValue != NULL){
			const char *SMTPMode = NULL;
			SMTPMode = json_object_get_string(paramValue); 
			if(!strcmp(SMTPMode,"LAN_WAN"))
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(1));
			else
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(0));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(SMTPObj);

	return ZCFG_SUCCESS;


}

zcfgRet_t zyTTSMTPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *SMTPObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
   // bool setQuickBri = false;
	const char *NowVal = NULL, *SetVal = NULL;

	printf("%s : Enter\n", __FUNCTION__);

    	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 8;
	if((ret = zcfgFeObjJsonGet(RDM_OID_REMO_SRV, &objIid, &SMTPObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = SMTPObj;
		SMTPObj = NULL;
		SMTPObj = zcfgFeJsonMultiObjAppend(RDM_OID_REMO_SRV, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(SMTPObj, "Mode");
			if(tr181ParamValue != NULL){
				int SMTPMode = 0;
				SMTPMode= json_object_get_boolean(paramValue);
				if(SMTPMode == 0)
					json_object_object_add(SMTPObj, "Mode", json_object_new_string(""));
				else if(SMTPMode == 1)
					json_object_object_add(SMTPObj, "Mode", json_object_new_string("LAN_WAN"));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_REMO_SRV, &objIid, SMTPObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(SMTPObj);
			return ret;
		}
		json_object_put(SMTPObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTSMTPObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTSMTPObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  SMTPOid = 0;
	objIndex_t SMTPIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(SMTPIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	SMTPOid = zcfgFeObjNameToObjId(tr181Obj, &SMTPIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(SMTPOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTSMTPObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  SMTPOid = 0;
	objIndex_t SMTPIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(SMTPIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	SMTPOid = zcfgFeObjNameToObjId(tr181Obj, &SMTPIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(SMTPOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(SMTPOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, SMTPOid, paramList->name);
		}
		break;
	}
	
	return ret;
}




/* InternetGatewayDevice.X_TT.Configuration.ICMP */
zcfgRet_t zyTTICMPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ICMPObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 7;
	if((ret = feObjJsonGet(RDM_OID_REMO_SRV, &objIid, &ICMPObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(ICMPObj, "Mode");
		if(paramValue != NULL){
			const char *ICMPMode = NULL;
			ICMPMode = json_object_get_string(paramValue); 
			if(!strcmp(ICMPMode,"LAN_WAN"))
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(3));
			else if(!strcmp(ICMPMode,"WAN_ONLY"))
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(2));
			else if(!strcmp(ICMPMode,"LAN_ONLY"))
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(1));
			else
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(0));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(ICMPObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTICMPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ICMPObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
   // bool setQuickBri = false;
	const char *NowVal = NULL, *SetVal = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 7;
	if((ret = zcfgFeObjJsonGet(RDM_OID_REMO_SRV, &objIid, &ICMPObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = ICMPObj;
		ICMPObj = NULL;
		ICMPObj = zcfgFeJsonMultiObjAppend(RDM_OID_REMO_SRV, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ICMPObj, "Mode");
			if(tr181ParamValue != NULL){
				int ICMPMode = 0;
				ICMPMode= json_object_get_int(paramValue);
				if(ICMPMode == 0)
					json_object_object_add(ICMPObj, "Mode", json_object_new_string(""));
				else if(ICMPMode == 1)
					json_object_object_add(ICMPObj, "Mode", json_object_new_string("LAN_ONLY"));
				else if(ICMPMode == 2)
					json_object_object_add(ICMPObj, "Mode", json_object_new_string("WAN_ONLY"));
				else if(ICMPMode == 3)
					json_object_object_add(ICMPObj, "Mode", json_object_new_string("LAN_WAN"));
				else{
					if(!multiJobj)
						json_object_put(ICMPObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_REMO_SRV, &objIid, ICMPObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ICMPObj);
			return ret;
		}
		json_object_put(ICMPObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTICMPObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTICMPObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  ICMPOid = 0;
	objIndex_t ICMPIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(ICMPIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ICMPOid = zcfgFeObjNameToObjId(tr181Obj, &ICMPIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(ICMPOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTICMPObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  ICMPOid = 0;
	objIndex_t ICMPIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(ICMPIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ICMPOid = zcfgFeObjNameToObjId(tr181Obj, &ICMPIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(ICMPOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(ICMPOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, ICMPOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

#ifdef CONFIG_TAAAB_DSL_BINDING
extern int WanConnectType;   //1:eth,2:ATM,3:PTM
#endif

void getADSLIPInterface(char *MatchInterface, char *NTPInterface){
	objIndex_t matchIpifaceIid;
	objIndex_t ipifaceIid;
	rdm_IpIface_t *matchIpiface = NULL;
	rdm_IpIface_t *ipiface = NULL;

	IID_INIT(matchIpifaceIid);
	matchIpifaceIid.level = 1;
	sscanf(MatchInterface, "IP.Interface.%hhu", &matchIpifaceIid.idx[0]);
	if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_IP_IFACE, &matchIpifaceIid, (void **)&matchIpiface) == ZCFG_SUCCESS){
		if(!strstr(matchIpiface->X_TAAAB_SrvName,"_DSL") || strstr(matchIpiface->X_ZYXEL_SrvName,"_ADSL") ){
			strcpy(NTPInterface,MatchInterface);
			zcfgFeObjStructFree(matchIpiface);
			return ;
		}
	}
	
	IID_INIT(ipifaceIid);
	while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipifaceIid, (void **)&ipiface) == ZCFG_SUCCESS){
		if(!strcmp(matchIpiface->X_TAAAB_SrvName,ipiface->X_TAAAB_SrvName) && strstr(ipiface->X_ZYXEL_SrvName,"_ADSL")){
			sprintf(NTPInterface,"IP.Interface.%d",ipifaceIid.idx[0]);
			zcfgFeObjStructFree(ipiface);
			return;
		}	
	}
}


/* InternetGatewayDevice.X_TT.Configuration.NTP */
zcfgRet_t zyTTNTPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *NTPObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	int len = 0;
	char iface[40] = {0};
	char lowerLayer[32] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);
		/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_TIME, &objIid, &NTPObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(NTPObj, "X_ZYXEL_BoundInterfaceList");
		if(paramValue != NULL){
			const char *X_ZYXEL_BoundInterfaceList = NULL;
			X_ZYXEL_BoundInterfaceList = json_object_get_string(paramValue);
			if(X_ZYXEL_BoundInterfaceList) {
				char wanIpConnName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
				char tr98IntfList[TR98_MAX_OBJ_NAME_LENGTH * 5] = {0};
				char interface_str[256] = {0};
				char *ipIfaceName = NULL, *tmp = NULL;
						
				sprintf(interface_str, "%s", X_ZYXEL_BoundInterfaceList);
				ipIfaceName = strtok_r(interface_str, ",", &tmp);
				while (ipIfaceName != NULL) {
							memset(wanIpConnName, 0, TR98_MAX_OBJ_NAME_LENGTH);
							memset(iface, 0, 40);
#ifdef CONFIG_TAAAB_DSL_BINDING							
							if(WanConnectType == 2)
								getADSLIPInterface(ipIfaceName,iface);
							else
#endif
								strcpy(iface, ipIfaceName);
							//printf("zyTTNTPObjGet  --%s--\r\n",iface);
							if((zcfgFeTr181IfaceStackLowerLayerGet(iface, lowerLayer)) != ZCFG_SUCCESS) {
								json_object_put(NTPObj);
								return ZCFG_NO_SUCH_OBJECT;
							}
							if(strstr(lowerLayer, "PPP.Interface") != NULL) {
								memset(iface, 0, sizeof(iface));
								strcpy(iface, lowerLayer);
							}
							if (zcfgFe181To98MappingNameGet(iface, wanIpConnName) != ZCFG_SUCCESS) {
								ipIfaceName = strtok_r(NULL, "," ,&tmp);
								continue;
							}
							strcpy(tr98ObjName, wanIpConnName);
							ReplaceStaticWANDeviceIndex(wanIpConnName, tr98ObjName);
							//printf("zyTTNTPObjGet22  --%s--\r\n",tr98ObjName);
							strcat(tr98IntfList, tr98ObjName);
							strcat(tr98IntfList, ",");
							ipIfaceName = strtok_r(NULL, ",", &tmp);
				}
				len = strlen(tr98IntfList);
				if(tr98IntfList[len-1] == ',')
					tr98IntfList[len-1] = '\0';
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98IntfList));
				paramList++;
				continue;
			}
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(NTPObj);

	return ZCFG_SUCCESS;


}

void getVDSLIPInterface(char *MatchInterface, char *NTPInterface){
	objIndex_t matchIpifaceIid;
	objIndex_t ipifaceIid;
	rdm_IpIface_t *matchIpiface = NULL;
	rdm_IpIface_t *ipiface = NULL;

	IID_INIT(matchIpifaceIid);
	matchIpifaceIid.level = 1;
	sscanf(MatchInterface, "IP.Interface.%hhu", &matchIpifaceIid.idx[0]);
	if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_IP_IFACE, &matchIpifaceIid, (void **)&matchIpiface) == ZCFG_SUCCESS){
		if(!strstr(matchIpiface->X_TAAAB_SrvName,"_DSL") || strstr(matchIpiface->X_ZYXEL_SrvName,"_VDSL") ){
			strcpy(NTPInterface,MatchInterface);
			zcfgFeObjStructFree(matchIpiface);
			return ;
		}
	}
	
	IID_INIT(ipifaceIid);
	while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipifaceIid, (void **)&ipiface) == ZCFG_SUCCESS){
		if(!strcmp(matchIpiface->X_TAAAB_SrvName,ipiface->X_TAAAB_SrvName) && strstr(ipiface->X_ZYXEL_SrvName,"_VDSL")){
			sprintf(NTPInterface,"IP.Interface.%d",ipifaceIid.idx[0]);
			zcfgFeObjStructFree(ipiface);
			return;
		}	
	}
}
zcfgRet_t zyTTNTPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *NTPObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
   // bool setQuickBri = false;
	const char *NowVal = NULL, *SetVal = NULL;
   const char *X_ZYXEL_BoundInterfaceList = NULL;
	int len = 0;
	printf("%s : Enter\n", __FUNCTION__);

    	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_TIME, &objIid, &NTPObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = NTPObj;
		NTPObj = NULL;
		NTPObj = zcfgFeJsonMultiObjAppend(RDM_OID_TIME, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name, "Interface")) {
				X_ZYXEL_BoundInterfaceList = json_object_get_string(paramValue);
				if(X_ZYXEL_BoundInterfaceList) {
					char tr181IntfList[256] = {0};
					char ipIfaceName[64] = {0};
					char interface_str[TR98_MAX_OBJ_NAME_LENGTH*10] = {0};
					char *wanIpConnName = NULL, *tmp = NULL;
					char tr98ConnObjName[128] = {0};
					char NTPInterface[32] = {0};
					
					sprintf(interface_str, "%s", X_ZYXEL_BoundInterfaceList);
					wanIpConnName = strtok_r(interface_str, ",", &tmp);
					while (wanIpConnName != NULL) {
						//sprintf(wanIpConnName, "%s", wanIpConnName);						
						memset(ipIfaceName, 0, 64);
						memset(NTPInterface, 0, 32);
						strcpy(tr98ConnObjName, wanIpConnName);
						ReplaceWANDeviceIndex(wanIpConnName, tr98ConnObjName);
						if (zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIfaceName) != ZCFG_SUCCESS) {
							wanIpConnName = strtok_r(NULL, "," ,&tmp);
							continue;
						}
						if(strstr(ipIfaceName, "PPP.Interface.")){
							zcfgFeTr181IfaceStackHigherLayerGet(ipIfaceName, ipIfaceName);
						}
						getVDSLIPInterface(ipIfaceName,NTPInterface);
						//printf("zyTTNTPObjSet111   --%s--\r\n",NTPInterface);
						strcat(tr181IntfList, NTPInterface);
						strcat(tr181IntfList, ",");
						wanIpConnName = strtok_r(NULL, ",", &tmp);
					}
					len = strlen(tr181IntfList);
					if(tr181IntfList[len-1] == ',')
						tr181IntfList[len-1] = '\0';
					//printf("zyTTNTPObjSet   --%s--\r\n",tr181IntfList);
					json_object_object_add(NTPObj, "X_ZYXEL_BoundInterfaceList", json_object_new_string(tr181IntfList));
					paramList++;
					continue;
				}
				json_object_object_add(NTPObj, "X_ZYXEL_BoundInterfaceList", json_object_new_string(""));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_TIME, &objIid, NTPObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(NTPObj);
			return ret;
		}
		json_object_put(NTPObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTNTPObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTNTPObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  SMTPOid = 0;
	objIndex_t SMTPIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(SMTPIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	SMTPOid = zcfgFeObjNameToObjId(tr181Obj, &SMTPIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(SMTPOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTNTPObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  SMTPOid = 0;
	objIndex_t SMTPIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(SMTPIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	SMTPOid = zcfgFeObjNameToObjId(tr181Obj, &SMTPIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(SMTPOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(SMTPOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, SMTPOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_DSL
// InternetGatewayDevice.X_ZYXEL_EXT.DSL
zcfgRet_t zyExtDSLGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t rst;
	objIndex_t objIid;
	struct json_object *dslObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *parmList = NULL;
	IID_INIT(objIid);
	if((rst = feObjJsonGet(RDM_OID_DSL, &objIid, &dslObj, updateFlag)) != ZCFG_SUCCESS){

		return rst;
	}

	*tr98Jobj = json_object_new_object();
	parmList = tr98Obj[handler].parameter;
	while(parmList->name != NULL){
		paramValue = json_object_object_get(dslObj, parmList->name);
		if(paramValue){
			json_object_object_add(*tr98Jobj, parmList->name, JSON_OBJ_COPY(paramValue));
			parmList++;
			continue;
		}

		parmList++;
	}

	json_object_put(dslObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtDSLSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t rst;
	objIndex_t objIid;
	struct json_object *dslObj = NULL, *tmpObj;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *parmList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	//sscanf(tr98FullPathName, " ");

	if((rst = zcfgFeObjJsonGet(RDM_OID_DSL, &objIid, &dslObj)) != ZCFG_SUCCESS)
		return rst;

	if(multiJobj){
		tmpObj = dslObj;
		dslObj = NULL;
		dslObj = zcfgFeJsonMultiObjAppend(RDM_OID_DSL, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	parmList = tr98Obj[handler].parameter;
	while(parmList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		if(!strcmp(parmList->name, "X_ZYXEL_ModemLabID")) {
			sprintf(paramfault, "%s.%s", tr98FullPathName, parmList->name);
			paramValue = json_object_object_get(tr98Jobj, parmList->name);
			if(paramValue != NULL && json_object_get_string(paramValue)) {
				json_object_object_add(dslObj, parmList->name, JSON_OBJ_COPY(paramValue));

			}
		}else{

			printf("Can't find parameter %s in TR181\n", parmList->name);
		}

		parmList++;
	}

	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if((rst = zcfgFeObjJsonSet(RDM_OID_DSL, &objIid, dslObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(dslObj);
			return rst;
		}
		json_object_put(dslObj);
	}

	return ZCFG_SUCCESS;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_DSL


#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
/* InternetGatewayDevice.X_ZYXEL_EXT.EasyMesh */
/* InternetGatewayDevice.X_ZYXEL_Mesh for ZYXEL_WIND_ITALY_CUSTOMIZATION*/ 
zcfgRet_t zyExtEasyMeshObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *EasyMeshObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_EASY_MESH, &objIid, &EasyMeshObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(EasyMeshObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
		// status for InternetGatewayDevice.X_ZYXEL_EasyMesh
		if(!strcmp(paramList->name, "Status"))
		{
			paramValue = json_object_object_get(EasyMeshObj, "Enable");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif		
		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(EasyMeshObj);

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.EasyMesh */
/* InternetGatewayDevice.X_ZYXEL_Mesh */
zcfgRet_t zyExtEasyMeshObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *EasyMeshObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_EASY_MESH, &objIid, &EasyMeshObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = EasyMeshObj;
		EasyMeshObj = NULL;
		EasyMeshObj = zcfgFeJsonMultiObjAppend(RDM_OID_EASY_MESH, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(EasyMeshObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(EasyMeshObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_EASY_MESH, &objIid, EasyMeshObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(EasyMeshObj);
			return ret;
		}
		json_object_put(EasyMeshObj);
	}

	return ZCFG_SUCCESS;
}
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
/* InternetGatewayDevice.X_ZYXEL_EXT.WiFiCommunity */
zcfgRet_t zyExtWiFiCommunityObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t iid = {0};
	objIndex_t objIid;
	tr98Parameter_t *paramList = NULL;
	struct json_object *obj = NULL;
	struct json_object *WiFiSsidObj = NULL;
	bool WiFiSsidEnable = false;
	unsigned int flag3 = 0;
	int WiFiCommAttribute;
	int count_all = 0;
	int count_true = 0;

	IID_INIT(iid);
	if(zcfgFeObjJsonGet(RDM_OID_GUI_CUSTOMIZATION, &iid, &obj) == ZCFG_SUCCESS){
		flag3 = json_object_get_int(json_object_object_get(obj, "Flag3"));
		zcfgFeJsonObjFree(obj);
	}
	
	if(!(flag3 & SHOW_WIFI_COMMUNITY))
	{
		return ZCFG_INVALID_OBJECT;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;

	while(paramList->name != NULL)
	{
		/* Enable/Disable all WiFi.SSID.i. which atttribute is 250. */
		if(!strcmp(paramList->name, "Enable"))
		{
			IID_INIT(objIid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &objIid,&WiFiSsidObj) == ZCFG_SUCCESS)
			{
				WiFiSsidEnable = json_object_get_boolean(json_object_object_get(WiFiSsidObj, "Enable"));
				WiFiCommAttribute = json_object_get_int(json_object_object_get(WiFiSsidObj, "X_ZYXEL_Attribute"));
				if(WiFiCommAttribute == 250)
				{
					count_all += 1;

					if(WiFiSsidEnable == true)
					{
						count_true += 1;
					}
				}
				zcfgFeJsonObjFree(WiFiSsidObj);
			}
			
			if((count_all != 0) && (count_all == count_true))
			{
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(1));
			}
			else
			{
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(0));
			}
			paramList++;
		}
	}
	return ZCFG_SUCCESS;
}
	
	
/* InternetGatewayDevice.X_ZYXEL_EXT.WiFiCommunity */
zcfgRet_t zyExtWiFiCommunityObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t iid = {0};
	objIndex_t objIid;
	tr98Parameter_t *paramList = NULL;
	struct json_object *obj = NULL;
	struct json_object *WiFiSsidObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	unsigned int flag3=0;
	int WiFiCommAttribute;

	IID_INIT(iid);
	if(zcfgFeObjJsonGet(RDM_OID_GUI_CUSTOMIZATION, &iid, &obj) == ZCFG_SUCCESS){
		flag3 = json_object_get_int(json_object_object_get(obj, "Flag3"));
		zcfgFeJsonObjFree(obj);
	}
	
	if(!(flag3 & SHOW_WIFI_COMMUNITY))
	{
		return ZCFG_INVALID_OBJECT;
	}	

	/*Get related tr181 objects*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL)
	{
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL)
		{	
			/* Enable/Disable all WiFi.SSID.i. which atttribute is 250. */
			if(!strcmp(paramList->name, "Enable"))
			{
				IID_INIT(objIid);
				while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &objIid, &WiFiSsidObj) == ZCFG_SUCCESS)
				{
					WiFiCommAttribute = json_object_get_int(json_object_object_get(WiFiSsidObj, "X_ZYXEL_Attribute"));
					
					if(WiFiCommAttribute == 250)
					{
						if(multiJobj)
						{
							tmpObj = WiFiSsidObj;
							WiFiSsidObj = NULL;
							WiFiSsidObj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_SSID, &objIid, multiJobj, tmpObj);
						}
						
						tr181ParamValue = json_object_object_get(WiFiSsidObj, paramList->name);
						if(tr181ParamValue != NULL)
						{
							json_object_object_add(WiFiSsidObj, paramList->name, JSON_OBJ_COPY(paramValue));
						}
						
						if(multiJobj)
						{
							zcfgFeJsonObjFree(tmpObj);
						}
						else
						{
							if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_SSID, &objIid, WiFiSsidObj, NULL)) != ZCFG_SUCCESS )
							{
								zcfgFeJsonObjFree(WiFiSsidObj);
								return ret;
							}
							zcfgFeJsonObjFree(WiFiSsidObj);
						}
					}
					else
					{
						zcfgFeJsonObjFree(WiFiSsidObj);
					}
				}
			}
		}
		paramList++;
	}
	return ZCFG_SUCCESS;
}
#endif //#ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
/* InternetGatewayDevice.X_ZYXEL_EXT.X_ZYXEL_ONESSID */
zcfgRet_t zyExtOneSsidObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *OneSsidObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_WIFI_ONE_SSID, &objIid, &OneSsidObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(OneSsidObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(OneSsidObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtOneSsidObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *OneSsidObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

#ifndef ZYXEL_WIND_ITALY_CUSTOMIZATION
#ifdef ZYXEL_EASYMESH
	objIndex_t easymeshIid;
	struct json_object *easymeshObj = NULL;
	IID_INIT(easymeshIid);
	if((zcfgFeObjJsonGet(RDM_OID_EASY_MESH, &easymeshIid, &easymeshObj)) == ZCFG_SUCCESS)
	{
		if(json_object_get_boolean(json_object_object_get(easymeshObj, "Enable")))
		{
			json_object_put(easymeshObj);
			return ZCFG_REQUEST_REJECT;
		}
		json_object_put(easymeshObj);
	}
#endif //ZYXEL_EASYMESH
#endif //ifndef ZYXEL_WIND_ITALY_CUSTOMIZATION

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ONE_SSID, &objIid, &OneSsidObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = OneSsidObj;
		OneSsidObj = NULL;
		OneSsidObj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_ONE_SSID, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(OneSsidObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(OneSsidObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_ONE_SSID, &objIid, OneSsidObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(OneSsidObj);
			return ret;
		}
		json_object_put(OneSsidObj);
	}

	return ZCFG_SUCCESS;
}
#endif //#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID

#ifdef CONFIG_ZCFG_BE_MODULE_DATA_USAGE
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_DataUsage
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_DataUsage
 *   RDM_OID_ZY_DATA_USAGE
 */

zcfgRet_t zyExtDataUsageObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dataUsageJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_DATA_USAGE, &objIid, &dataUsageJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dataUsageJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(dataUsageJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtDataUsageObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dataUsageJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_DATA_USAGE, &objIid, &dataUsageJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = dataUsageJobj;
		dataUsageJobj = NULL;
		dataUsageJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_DATA_USAGE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(dataUsageJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(dataUsageJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_DATA_USAGE, &objIid, dataUsageJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(dataUsageJobj);
			return ret;
		}
		json_object_put(dataUsageJobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_DataUsage.Lan.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_DataUsage.Lan.i
 *   RDM_OID_ZY_DATA_USAGE_LAN
 */
zcfgRet_t zyExtDataUsageLanObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dataUsageLanJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_DataUsage.Lan.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_ZY_DATA_USAGE_LAN, &objIid, &dataUsageLanJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dataUsageLanJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(dataUsageLanJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_DataUsage.Wan.i
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_DataUsage.Wan.i
 *   RDM_OID_ZY_DATA_USAGE_WAN
 */
zcfgRet_t zyExtDataUsageWanObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dataUsageWanJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_DataUsage.Wan.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_ZY_DATA_USAGE_WAN, &objIid, &dataUsageWanJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dataUsageWanJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(dataUsageWanJobj);

	return ZCFG_SUCCESS;
}
#endif

#ifdef ZYXEL_TAAAG_EDNS//"Add ppp username to edns option for TAAAB"
zcfgRet_t zyTTEDNSObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *EDNSJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_E_D_N_S, &objIid, &EDNSJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(EDNSJobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name,"State")){
					bool Enable = false;
					Enable = json_object_get_boolean(json_object_object_get(EDNSJobj, "Enable"));
					if(Enable)
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Enabled"));
					else
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Disabled"));
			}
			else
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(EDNSJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTEDNSObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *EDNSJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	struct json_object *DhcpPoolJobj = NULL;
	objIndex_t DhcpPoolIid;
	int setDhcpPoolJobj = 0;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_E_D_N_S, &objIid, &EDNSJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = EDNSJobj;
		EDNSJobj = NULL;
		EDNSJobj = zcfgFeJsonMultiObjAppend(RDM_OID_E_D_N_S, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(EDNSJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				if(!strcmp(paramList->name,"Enable")){
					if(json_object_get_boolean(paramValue)){					
						IID_INIT(DhcpPoolIid);
						DhcpPoolIid.level = 1;
						DhcpPoolIid.idx[0]=1;
						if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV_POOL, &DhcpPoolIid, &DhcpPoolJobj)) == ZCFG_SUCCESS){
							if(multiJobj && strcmp(json_object_get_string(json_object_object_get(DhcpPoolJobj, "X_ZYXEL_DNS_Type")),"DNS Proxy")){
								json_object_put(tmpObj);
								tmpObj = DhcpPoolJobj;
								DhcpPoolJobj = NULL;
								DhcpPoolJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_SRV_POOL, &DhcpPoolIid, multiJobj, tmpObj);
								json_object_object_add(DhcpPoolJobj, "X_ZYXEL_DNS_Type", json_object_new_string("DNS Proxy"));
								setDhcpPoolJobj = 1;//alrady set
							}
							else
								json_object_put(DhcpPoolJobj);
								
						}						
						system("echo EDNS >/tmp/TTnetEdns.txt");
					}
					else
						unlink("/tmp/TTnetEdns.txt");
					system("/etc/init.d/dnsmasq.sh restart");
				}					
				json_object_object_add(EDNSJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_E_D_N_S, &objIid, EDNSJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(EDNSJobj);
			return ret;
		}
		json_object_put(EDNSJobj);

		if(DhcpPoolJobj && setDhcpPoolJobj){
					if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL, &DhcpPoolIid, DhcpPoolJobj, NULL)) != ZCFG_SUCCESS ) {
						json_object_put(DhcpPoolJobj);
						return ret;
					}				
					json_object_put(DhcpPoolJobj);
		}
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTEDNSObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTEDNSObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  EDNSOid = 0;
	objIndex_t EDNSIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(EDNSIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	EDNSOid = zcfgFeObjNameToObjId(tr181Obj, &EDNSIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(EDNSOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTEDNSObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  EDNSOid = 0;
	objIndex_t EDNSIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(EDNSIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	EDNSOid = zcfgFeObjNameToObjId(tr181Obj, &EDNSIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(EDNSOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(EDNSOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, EDNSOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

#endif

#ifdef MSTC_TAAAG_MULTI_USER
zcfgRet_t zyNatDMZObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *natJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	
	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_NAT, &objIid, &natJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get NAT Fail\n", __FUNCTION__);
		return ret;
	}
	
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(!strcmp(paramList->name, "X_ZYXEL_DMZHost_Enable"))
		{
			if(natJobj){
				paramValue = json_object_object_get(natJobj, paramList->name);
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DMZHost"))
		{
			if(natJobj){
				paramValue = json_object_object_get(natJobj, paramList->name);
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
	}
	
	if(natJobj){
		json_object_put(natJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyNatDMZObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *natJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT, &objIid, &natJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		if(natJobj){
			tmpObj = natJobj;
			natJobj = NULL;
			natJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT, &objIid, multiJobj, tmpObj);	
		}
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/ 
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {			
			if(!strcmp(paramList->name, "X_ZYXEL_DMZHost_Enable")){
				if(natJobj){
					json_object_object_add(natJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_DMZHost")){
				if(natJobj){
					json_object_object_add(natJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT, &objIid, natJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(natJobj);
			return ret;
		}
		json_object_put(natJobj);
	}

	return ZCFG_SUCCESS;
}
#endif

#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER) || defined(TENDER_TAAAB_FEATURE)
#include <ctype.h>

bool checkTTPassword(char *pwd, char *oldpwd)
{
     int length=0,i=0,j=0;
     char tmppwd[512]={0};
	 char keyboardstring1[20]={0};
	 char keyboardstring2[20]={0};
	 char keyboardstring3[20]={0};
	 char keyboardstring4[20]={0};
	 bool isdigit=false,isletter=false;
	 char *decodepassword = NULL;
	 int oldlength = strlen(oldpwd);
	 
	 decodepassword = base64_decode((unsigned char*)oldpwd, &oldlength);
	 if(decodepassword && !strcmp(pwd,decodepassword))
	 	return false;
	 
	 strcpy(keyboardstring1,"qwertyuiop");
	 strcpy(keyboardstring2,"asdfghjkl");
	 strcpy(keyboardstring3,"zxcvbnm");
	 strcpy(keyboardstring4,"1234567890");
	 
     strcpy(tmppwd,pwd);
	 length = strlen(tmppwd);

	 for(i=0;i<length;i++){
         tmppwd[i] = tolower(tmppwd[i]);
     }
	 
     for(i=0;i<length;i++){
	 	if((tmppwd[i] >= 0x30) && (tmppwd[i] < 0x3A))
			isdigit = true;
         if((tmppwd[i] >= 0x61) && (tmppwd[i] < 0x7B))
		 	isletter = true;
     }

	 if(isdigit==false || isletter==false)
	 	return false;
	 
     for(i=0;i<length;i++){
         if((tmppwd[i+1] == (tmppwd[i]+1))&&(tmppwd[i+2] == (tmppwd[i+1]+1))&&(tmppwd[i+3] == (tmppwd[i+2]+1)))
		 	return false;
     }
	 
	 for(i=0;i<length;i++){
         if((tmppwd[i+1] == (tmppwd[i]-1))&&(tmppwd[i+2] == (tmppwd[i+1]-1))&&(tmppwd[i+3] == (tmppwd[i+2]-1)))
		 	return false;
     }
	 
	 for(i=0;i<length;i++){
		 for(j=0;j<strlen(keyboardstring1);j++){
			 if((tmppwd[i] == keyboardstring1[j])&&(tmppwd[i+1] == keyboardstring1[j+1])&&(tmppwd[i+2] == keyboardstring1[j+2])&&(tmppwd[i+3] == keyboardstring1[j+3]))
				return false;
		 }
     }

     for(i=0;i<length;i++){
		 for(j=0;j<strlen(keyboardstring2);j++){
			 if((tmppwd[i] == keyboardstring2[j])&&(tmppwd[i+1] == keyboardstring2[j+1])&&(tmppwd[i+2] == keyboardstring2[j+2])&&(tmppwd[i+3] == keyboardstring2[j+3]))
				return false;
		 }
     }

	 for(i=0;i<length;i++){
		 for(j=0;j<strlen(keyboardstring3);j++){
			 if((tmppwd[i] == keyboardstring3[j])&&(tmppwd[i+1] == keyboardstring3[j+1])&&(tmppwd[i+2] == keyboardstring3[j+2])&&(tmppwd[i+3] == keyboardstring3[j+3]))
				return false;
		 }
     }

	 for(i=0;i<length;i++){
		 for(j=0;j<strlen(keyboardstring4);j++){
			 if((tmppwd[i] == keyboardstring4[j])&&(tmppwd[i+1] == keyboardstring4[j+1])&&(tmppwd[i+2] == keyboardstring4[j+2])&&(tmppwd[i+3] == keyboardstring4[j+3]))
				return false;
		 }
     }

	 for(i=0;i<length;i++){
         if((tmppwd[i] == tmppwd[i+1])&&(tmppwd[i+1] == tmppwd[i+2])&&(tmppwd[i+2] == tmppwd[i+3]))
		 	return false;
     }

	 return true;
}

/* InternetGatewayDevice.X_TT.Users */
zcfgRet_t zyTTUsersObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *UsersObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_TAAAB_USERS, &objIid, &UsersObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(UsersObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(UsersObj);

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.X_TT.Users.User.i */
zcfgRet_t zyTTUserObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *UserObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
    objIid.level = 1;
    sscanf(tr98FullPathName, "InternetGatewayDevice.X_TTNET.Users.User.%hhu", &objIid.idx[0]);

    if((ret = feObjJsonGet(RDM_OID_TAAAB_USER, &objIid, &UserObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(UserObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(UserObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTUserObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *UserObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *setvalue = NULL;
	char *tr181value = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_TTNET.Users.User.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_TAAAB_USER, &objIid, &UserObj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = UserObj;
		UserObj = NULL;
		UserObj = zcfgFeJsonMultiObjAppend(RDM_OID_TAAAB_USER, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name,"Username")){
				setvalue = json_object_get_string(paramValue);
				if(!strcmp(setvalue, "")){
					ret = ZCFG_INVALID_PARAM_VALUE;
					strcpy(paramfault,"Username");
					return ret;
				}	
			}
			tr181ParamValue = json_object_object_get(UserObj, paramList->name);
			if(tr181ParamValue != NULL){
				if(!strcmp(paramList->name,"Password")){
					setvalue = json_object_get_string(paramValue);
					tr181value = json_object_get_string(tr181ParamValue);
					if(!checkTTPassword(setvalue,tr181value)){
						ret = ZCFG_INVALID_PARAM_VALUE;
						strcpy(paramfault,"Password");
						return ret;
					}	
										if(!strcmp(setvalue, "")){
						ret = ZCFG_INVALID_PARAM_VALUE;
						strcpy(paramfault,"Password");
						return ret;
					}	
					else if(strcmp(setvalue,tr181value)){
						char *encodepassword = NULL;
						encodepassword = base64_encode((unsigned char*)setvalue, strlen(setvalue));
						if(encodepassword != NULL){
							json_object_object_add(UserObj, paramList->name, json_object_new_string(encodepassword));
						}
						else{
							json_object_object_add(UserObj, paramList->name, json_object_new_string(""));
						}
						paramList++;
						continue;
					}
				}
				json_object_object_add(UserObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_TAAAB_USER, &objIid, UserObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(UserObj);
			return ret;
		}
		json_object_put(UserObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTUserObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t LogCfgGpIid;

	printf("%s : Enter\n", __FUNCTION__);

#if 0 //def MSTC_TAAAG_MULTI_USER
	IID_INIT(TTUserIid);

	if((TTUserIid.idx[0]==1) || (TTUserIid.idx[0]==2))
		return ZCFG_REQUEST_REJECT;

	if((ret = zcfgFeObjStructAdd(RDM_OID_TAAAB_USER, &TTUserIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add X_TTNET.Users.User Fail.\n", __FUNCTION__);
		return ret;
	}
#endif	
	IID_INIT(LogCfgGpIid);
	LogCfgGpIid.level = 1;
	LogCfgGpIid.idx[0] = 2;

	if((ret = zcfgFeObjStructAdd(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &LogCfgGpIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add RDM_OID_ZY_LOG_CFG_GP_ACCOUNT Fail.\n", __FUNCTION__);
		return ret;
	}

	*idx = LogCfgGpIid.idx[1] + 1;

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTUserObjDel(char *tr98FullPathName)
{
        objIndex_t objIid;
		int SSIndex = 0;
		int AccountIndex = 0;

        printf("%s : Enter\n", __FUNCTION__);

		sscanf(tr98FullPathName, "InternetGatewayDevice.X_TTNET.Users.User.%d", &AccountIndex);

		if(AccountIndex < 3)
			return ZCFG_DELETE_REJECT; 

        IID_INIT(objIid);
        objIid.level = 2;
		objIid.idx[0] = 2;
		objIid.idx[1] = AccountIndex - 1;
		
        return zcfgFeObjStructDel(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &objIid, NULL);
}

zcfgRet_t zyTTUserObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTUserObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t UserOid = 0;
	objIndex_t UserIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(UserIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	UserOid = zcfgFeObjNameToObjId(tr181Obj, &UserIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(UserOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTUserObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t UserOid = 0;
	objIndex_t UserIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(UserIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	UserOid = zcfgFeObjNameToObjId(tr181Obj, &UserIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(UserOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(UserOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, UserOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.UserInterface */
zcfgRet_t zyTTUserInterfaceObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *UserInterfaceObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_USER_INTERFACE, &objIid, &UserInterfaceObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(UserInterfaceObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(UserInterfaceObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTUserInterfaceObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *UserInterfaceObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_USER_INTERFACE, &objIid, &UserInterfaceObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = UserInterfaceObj;
		UserInterfaceObj = NULL;
		UserInterfaceObj = zcfgFeJsonMultiObjAppend(RDM_OID_USER_INTERFACE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(UserInterfaceObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(UserInterfaceObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_USER_INTERFACE, &objIid, UserInterfaceObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(UserInterfaceObj);
			return ret;
		}
		json_object_put(UserInterfaceObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTUserInterfaceObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTUserInterfaceObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t UserInterfaceOid = 0;
	objIndex_t UserInterfaceIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(UserInterfaceIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	UserInterfaceOid = zcfgFeObjNameToObjId(tr181Obj, &UserInterfaceIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(UserInterfaceOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTUserInterfaceObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t UserInterfaceOid = 0;
	objIndex_t UserInterfaceIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(UserInterfaceIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	UserInterfaceOid = zcfgFeObjNameToObjId(tr181Obj, &UserInterfaceIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(UserInterfaceOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(UserInterfaceOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, UserInterfaceOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.UserInterface.RemoteAccess */
zcfgRet_t zyTTRemoteAccessObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteAccessObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_REMOTE_ACCESS, &objIid, &RemoteAccessObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(RemoteAccessObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(RemoteAccessObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTRemoteAccessObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteAccessObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_REMOTE_ACCESS, &objIid, &RemoteAccessObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = RemoteAccessObj;
		RemoteAccessObj = NULL;
		RemoteAccessObj = zcfgFeJsonMultiObjAppend(RDM_OID_REMOTE_ACCESS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(RemoteAccessObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(RemoteAccessObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_REMOTE_ACCESS, &objIid, RemoteAccessObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(RemoteAccessObj);
			return ret;
		}
		json_object_put(RemoteAccessObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTRemoteAccessObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTRemoteAccessObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t RemoteAccessOid = 0;
	objIndex_t RemoteAccessIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(RemoteAccessIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	RemoteAccessOid = zcfgFeObjNameToObjId(tr181Obj, &RemoteAccessIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(RemoteAccessOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTRemoteAccessObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t RemoteAccessOid = 0;
	objIndex_t RemoteAccessIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(RemoteAccessIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	RemoteAccessOid = zcfgFeObjNameToObjId(tr181Obj, &RemoteAccessIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(RemoteAccessOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(RemoteAccessOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, RemoteAccessOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.UserInterface.LocalAccess */
zcfgRet_t zyTTLocalAccessObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LocalAccessObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_LOCAL_ACCESS, &objIid, &LocalAccessObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(LocalAccessObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(LocalAccessObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTLocalAccessObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LocalAccessObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_LOCAL_ACCESS, &objIid, &LocalAccessObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = LocalAccessObj;
		LocalAccessObj = NULL;
		LocalAccessObj = zcfgFeJsonMultiObjAppend(RDM_OID_LOCAL_ACCESS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(LocalAccessObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(LocalAccessObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_LOCAL_ACCESS, &objIid, LocalAccessObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(LocalAccessObj);
			return ret;
		}
		json_object_put(LocalAccessObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTLocalAccessObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTLocalAccessObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t LocalAccessOid = 0;
	objIndex_t LocalAccessIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(LocalAccessIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LocalAccessOid = zcfgFeObjNameToObjId(tr181Obj, &LocalAccessIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(LocalAccessOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTLocalAccessObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t LocalAccessOid = 0;
	objIndex_t LocalAccessIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(LocalAccessIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LocalAccessOid = zcfgFeObjNameToObjId(tr181Obj, &LocalAccessIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(LocalAccessOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(LocalAccessOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, LocalAccessOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Firewall */
zcfgRet_t zyTTFirewallObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *FirewallObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_TAAAB_FIREWALL, &objIid, &FirewallObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(FirewallObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(FirewallObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTFirewallObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *FirewallObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_TAAAB_FIREWALL, &objIid, &FirewallObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = FirewallObj;
		FirewallObj = NULL;
		FirewallObj = zcfgFeJsonMultiObjAppend(RDM_OID_TAAAB_FIREWALL, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(FirewallObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(FirewallObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_TAAAB_FIREWALL, &objIid, FirewallObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(FirewallObj);
			return ret;
		}
		json_object_put(FirewallObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTFirewallObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTFirewallObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t FirewallOid = 0;
	objIndex_t FirewallIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(FirewallIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	FirewallOid = zcfgFeObjNameToObjId(tr181Obj, &FirewallIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(FirewallOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTFirewallObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t FirewallOid = 0;
	objIndex_t FirewallIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(FirewallIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	FirewallOid = zcfgFeObjNameToObjId(tr181Obj, &FirewallIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(FirewallOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(FirewallOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, FirewallOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.UPnP.Device */
zcfgRet_t zyTTUPnPDeviceObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *UPnPDeviceObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_TAAAB_DEVICE, &objIid, &UPnPDeviceObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(UPnPDeviceObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(UPnPDeviceObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTUPnPDeviceObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *UPnPDeviceObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_TAAAB_DEVICE, &objIid, &UPnPDeviceObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = UPnPDeviceObj;
		UPnPDeviceObj = NULL;
		UPnPDeviceObj = zcfgFeJsonMultiObjAppend(RDM_OID_TAAAB_DEVICE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(UPnPDeviceObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(UPnPDeviceObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_TAAAB_DEVICE, &objIid, UPnPDeviceObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(UPnPDeviceObj);
			return ret;
		}
		json_object_put(UPnPDeviceObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTUPnPDeviceObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTUPnPDeviceObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t UPnPDeviceOid = 0;
	objIndex_t UPnPDeviceIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(UPnPDeviceIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	UPnPDeviceOid = zcfgFeObjNameToObjId(tr181Obj, &UPnPDeviceIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(UPnPDeviceOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTUPnPDeviceObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t UPnPDeviceOid = 0;
	objIndex_t UPnPDeviceIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(UPnPDeviceIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	UPnPDeviceOid = zcfgFeObjNameToObjId(tr181Obj, &UPnPDeviceIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(UPnPDeviceOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(UPnPDeviceOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, UPnPDeviceOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband */
zcfgRet_t zyTTBroadbandObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *BroadbandObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_BROADBAND, &objIid, &BroadbandObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(BroadbandObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(BroadbandObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTBroadbandObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *BroadbandObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_BROADBAND, &objIid, &BroadbandObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = BroadbandObj;
		BroadbandObj = NULL;
		BroadbandObj = zcfgFeJsonMultiObjAppend(RDM_OID_BROADBAND, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(BroadbandObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(BroadbandObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_BROADBAND, &objIid, BroadbandObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(BroadbandObj);
			return ret;
		}
		json_object_put(BroadbandObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTBroadbandObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTBroadbandObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t BroadbandOid = 0;
	objIndex_t BroadbandIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(BroadbandIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	BroadbandOid = zcfgFeObjNameToObjId(tr181Obj, &BroadbandIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(BroadbandOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTBroadbandObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t BroadbandOid = 0;
	objIndex_t BroadbandIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(BroadbandIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	BroadbandOid = zcfgFeObjNameToObjId(tr181Obj, &BroadbandIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(BroadbandOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(BroadbandOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, BroadbandOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.Internet_DSL */
zcfgRet_t zyTTInternetDSLObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *InternetDSLObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_INTERNET_DSL, &objIid, &InternetDSLObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(InternetDSLObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(InternetDSLObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTInternetDSLObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *InternetDSLObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_INTERNET_DSL, &objIid, &InternetDSLObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = InternetDSLObj;
		InternetDSLObj = NULL;
		InternetDSLObj = zcfgFeJsonMultiObjAppend(RDM_OID_INTERNET_DSL, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(InternetDSLObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(InternetDSLObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_INTERNET_DSL, &objIid, InternetDSLObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(InternetDSLObj);
			return ret;
		}
		json_object_put(InternetDSLObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTInternetDSLObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTInternetDSLObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t InternetDSLOid = 0;
	objIndex_t InternetDSLIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(InternetDSLIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	InternetDSLOid = zcfgFeObjNameToObjId(tr181Obj, &InternetDSLIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(InternetDSLOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTInternetDSLObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t InternetDSLOid = 0;
	objIndex_t InternetDSLIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(InternetDSLIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	InternetDSLOid = zcfgFeObjNameToObjId(tr181Obj, &InternetDSLIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(InternetDSLOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(InternetDSLOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, InternetDSLOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.Internet_WAN */
zcfgRet_t zyTTInternetWANObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *InternetWANObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_INTERNET_WAN, &objIid, &InternetWANObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(InternetWANObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(InternetWANObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTInternetWANObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *InternetWANObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_INTERNET_WAN, &objIid, &InternetWANObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = InternetWANObj;
		InternetWANObj = NULL;
		InternetWANObj = zcfgFeJsonMultiObjAppend(RDM_OID_INTERNET_WAN, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(InternetWANObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(InternetWANObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_INTERNET_WAN, &objIid, InternetWANObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(InternetWANObj);
			return ret;
		}
		json_object_put(InternetWANObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTInternetWANObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTInternetWANObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t InternetWANOid = 0;
	objIndex_t InternetWANIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(InternetWANIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	InternetWANOid = zcfgFeObjNameToObjId(tr181Obj, &InternetWANIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(InternetWANOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTInternetWANObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t InternetWANOid = 0;
	objIndex_t InternetWANIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(InternetWANIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	InternetWANOid = zcfgFeObjNameToObjId(tr181Obj, &InternetWANIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(InternetWANOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(InternetWANOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, InternetWANOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.IPTV_DSL */
zcfgRet_t zyTTIPTVDSLObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *IPTVDSLObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IPTV_DSL, &objIid, &IPTVDSLObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(IPTVDSLObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(IPTVDSLObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTIPTVDSLObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *IPTVDSLObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IPTV_DSL, &objIid, &IPTVDSLObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = IPTVDSLObj;
		IPTVDSLObj = NULL;
		IPTVDSLObj = zcfgFeJsonMultiObjAppend(RDM_OID_IPTV_DSL, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(IPTVDSLObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(IPTVDSLObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IPTV_DSL, &objIid, IPTVDSLObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(IPTVDSLObj);
			return ret;
		}
		json_object_put(IPTVDSLObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTIPTVDSLObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTIPTVDSLObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t IPTVDSLOid = 0;
	objIndex_t IPTVDSLIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(IPTVDSLIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	IPTVDSLOid = zcfgFeObjNameToObjId(tr181Obj, &IPTVDSLIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(IPTVDSLOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTIPTVDSLObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t IPTVDSLOid = 0;
	objIndex_t IPTVDSLIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(IPTVDSLIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	IPTVDSLOid = zcfgFeObjNameToObjId(tr181Obj, &IPTVDSLIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(IPTVDSLOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(IPTVDSLOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, IPTVDSLOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.IPTV_WAN */
zcfgRet_t zyTTIPTVWANObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *IPTVWANObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IPTV_WAN, &objIid, &IPTVWANObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(IPTVWANObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(IPTVWANObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTIPTVWANObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *IPTVWANObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IPTV_WAN, &objIid, &IPTVWANObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = IPTVWANObj;
		IPTVWANObj = NULL;
		IPTVWANObj = zcfgFeJsonMultiObjAppend(RDM_OID_IPTV_WAN, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(IPTVWANObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(IPTVWANObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IPTV_WAN, &objIid, IPTVWANObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(IPTVWANObj);
			return ret;
		}
		json_object_put(IPTVWANObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTIPTVWANObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTIPTVWANObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t IPTVWANOid = 0;
	objIndex_t IPTVWANIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(IPTVWANIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	IPTVWANOid = zcfgFeObjNameToObjId(tr181Obj, &IPTVWANIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(IPTVWANOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTIPTVWANObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t IPTVWANOid = 0;
	objIndex_t IPTVWANIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(IPTVWANIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	IPTVWANOid = zcfgFeObjNameToObjId(tr181Obj, &IPTVWANIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(IPTVWANOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(IPTVWANOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, IPTVWANOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.Mamg_WAN */
zcfgRet_t zyTTMamgWANObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *MamgWANObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_MANAGEMENT_WAN, &objIid, &MamgWANObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(MamgWANObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(MamgWANObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTMamgWANObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *MamgWANObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_MANAGEMENT_WAN, &objIid, &MamgWANObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = MamgWANObj;
		MamgWANObj = NULL;
		MamgWANObj = zcfgFeJsonMultiObjAppend(RDM_OID_MANAGEMENT_WAN, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(MamgWANObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(MamgWANObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MANAGEMENT_WAN, &objIid, MamgWANObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(MamgWANObj);
			return ret;
		}
		json_object_put(MamgWANObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTMamgWANObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTMamgWANObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t MamgWANOid = 0;
	objIndex_t MamgWANIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(MamgWANIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	MamgWANOid = zcfgFeObjNameToObjId(tr181Obj, &MamgWANIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(MamgWANOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTMamgWANObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t MamgWANOid = 0;
	objIndex_t MamgWANIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(MamgWANIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	MamgWANOid = zcfgFeObjNameToObjId(tr181Obj, &MamgWANIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(MamgWANOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(MamgWANOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, MamgWANOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.ACS_DSL */
zcfgRet_t zyTTACSDSLObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ACSDSLObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	bool isAddFromTr69 = false;
	struct json_object *ipinterfaceJarray = NULL;
	objIndex_t ipinterfaceIid;
	char tr69wanname[64] = {0};

	//printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ACS_DSL, &objIid, &ACSDSLObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	IID_INIT(ipinterfaceIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipinterfaceIid,&ipinterfaceJarray) == ZCFG_SUCCESS){
		isAddFromTr69 = json_object_get_boolean(json_object_object_get(ipinterfaceJarray, "isAddFromTr69"));
		strcpy(tr69wanname,json_object_get_string(json_object_object_get(ipinterfaceJarray, "X_ZYXEL_SrvName")));
		if(isAddFromTr69 == true && strstr(tr69wanname, "_VDSL")){
			printf("%s : Enter\n", __FUNCTION__);
			while(paramList->name != NULL){
				/*Write parameter value from tr181 objects to tr98 object*/
				paramValue = json_object_object_get(ACSDSLObj, paramList->name);
				if(paramValue != NULL){
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
							json_object_put(ipinterfaceJarray);
					continue;
				}

				/*Not defined in tr181, give it a default value*/
				printf("Can't find parameter %s in TR181\n", paramList->name);
				paramList++;
			}
		}
		json_object_put(ipinterfaceJarray);
	}

	json_object_put(ACSDSLObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTACSDSLObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ACSDSLObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ACS_DSL, &objIid, &ACSDSLObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = ACSDSLObj;
		ACSDSLObj = NULL;
		ACSDSLObj = zcfgFeJsonMultiObjAppend(RDM_OID_ACS_DSL, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ACSDSLObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ACSDSLObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ACS_DSL, &objIid, ACSDSLObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ACSDSLObj);
			return ret;
		}
		json_object_put(ACSDSLObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTACSDSLObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTACSDSLObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t ACSDSLOid = 0;
	objIndex_t ACSDSLIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(ACSDSLIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ACSDSLOid = zcfgFeObjNameToObjId(tr181Obj, &ACSDSLIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(ACSDSLOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTACSDSLObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t ACSDSLOid = 0;
	objIndex_t ACSDSLIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(ACSDSLIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ACSDSLOid = zcfgFeObjNameToObjId(tr181Obj, &ACSDSLIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(ACSDSLOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(ACSDSLOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, ACSDSLOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.ACS_WAN */
zcfgRet_t zyTTACSWANObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ACSWANObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	bool isAddFromTr69 = false;
	struct json_object *ipinterfaceJarray = NULL;
	objIndex_t ipinterfaceIid;
	char tr69wanname[64] = {0};

	//printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ACS_WAN, &objIid, &ACSWANObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	IID_INIT(ipinterfaceIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipinterfaceIid,&ipinterfaceJarray) == ZCFG_SUCCESS){
		isAddFromTr69 = json_object_get_boolean(json_object_object_get(ipinterfaceJarray, "isAddFromTr69"));
		strcpy(tr69wanname,json_object_get_string(json_object_object_get(ipinterfaceJarray, "X_TT_SrvName")));
		if(isAddFromTr69 == true && strstr(tr69wanname, "_WAN")){
			printf("%s : Enter\n", __FUNCTION__);
			while(paramList->name != NULL){
				/*Write parameter value from tr181 objects to tr98 object*/
				paramValue = json_object_object_get(ACSWANObj, paramList->name);
				if(paramValue != NULL){
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
							json_object_put(ipinterfaceJarray);
					continue;
				}

				/*Not defined in tr181, give it a default value*/
				printf("Can't find parameter %s in TR181\n", paramList->name);
				paramList++;
			}
		}
		json_object_put(ipinterfaceJarray);
	}

	json_object_put(ACSWANObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTACSWANObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ACSWANObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ACS_WAN, &objIid, &ACSWANObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = ACSWANObj;
		ACSWANObj = NULL;
		ACSWANObj = zcfgFeJsonMultiObjAppend(RDM_OID_ACS_WAN, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ACSWANObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ACSWANObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ACS_WAN, &objIid, ACSWANObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ACSWANObj);
			return ret;
		}
		json_object_put(ACSWANObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTACSWANObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTACSWANObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t ACSWANOid = 0;
	objIndex_t ACSWANIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(ACSWANIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ACSWANOid = zcfgFeObjNameToObjId(tr181Obj, &ACSWANIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(ACSWANOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTACSWANObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t ACSWANOid = 0;
	objIndex_t ACSWANIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(ACSWANIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ACSWANOid = zcfgFeObjNameToObjId(tr181Obj, &ACSWANIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(ACSWANOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(ACSWANOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, ACSWANOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.Advanced */
zcfgRet_t zyTTAdvancedObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AdvancedObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ADVANCED, &objIid, &AdvancedObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(AdvancedObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(AdvancedObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTAdvancedObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *AdvancedObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ADVANCED, &objIid, &AdvancedObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = AdvancedObj;
		AdvancedObj = NULL;
		AdvancedObj = zcfgFeJsonMultiObjAppend(RDM_OID_ADVANCED, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(AdvancedObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(AdvancedObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ADVANCED, &objIid, AdvancedObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(AdvancedObj);
			return ret;
		}
		json_object_put(AdvancedObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTAdvancedObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTAdvancedObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t AdvancedOid = 0;
	objIndex_t AdvancedIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(AdvancedIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AdvancedOid = zcfgFeObjNameToObjId(tr181Obj, &AdvancedIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(AdvancedOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTAdvancedObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t AdvancedOid = 0;
	objIndex_t AdvancedIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(AdvancedIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	AdvancedOid = zcfgFeObjNameToObjId(tr181Obj, &AdvancedIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(AdvancedOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(AdvancedOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, AdvancedOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless.General */
zcfgRet_t zyTTGeneralObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *GeneralObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_GENERAL, &objIid, &GeneralObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(GeneralObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(GeneralObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTGeneralObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *GeneralObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_GENERAL, &objIid, &GeneralObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = GeneralObj;
		GeneralObj = NULL;
		GeneralObj = zcfgFeJsonMultiObjAppend(RDM_OID_GENERAL, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(GeneralObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(GeneralObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_GENERAL, &objIid, GeneralObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(GeneralObj);
			return ret;
		}
		json_object_put(GeneralObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTGeneralObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTGeneralObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t GeneralOid = 0;
	objIndex_t GeneralIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(GeneralIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	GeneralOid = zcfgFeObjNameToObjId(tr181Obj, &GeneralIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(GeneralOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTGeneralObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t GeneralOid = 0;
	objIndex_t GeneralIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(GeneralIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	GeneralOid = zcfgFeObjNameToObjId(tr181Obj, &GeneralIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(GeneralOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(GeneralOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, GeneralOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless.MoreAP */
zcfgRet_t zyTTMoreAPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *MoreAPObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_MORE_AP, &objIid, &MoreAPObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(MoreAPObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(MoreAPObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTMoreAPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *MoreAPObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_MORE_AP, &objIid, &MoreAPObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = MoreAPObj;
		MoreAPObj = NULL;
		MoreAPObj = zcfgFeJsonMultiObjAppend(RDM_OID_MORE_AP, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(MoreAPObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(MoreAPObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MORE_AP, &objIid, MoreAPObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(MoreAPObj);
			return ret;
		}
		json_object_put(MoreAPObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTMoreAPObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTMoreAPObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t MoreAPOid = 0;
	objIndex_t MoreAPIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(MoreAPIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	MoreAPOid = zcfgFeObjNameToObjId(tr181Obj, &MoreAPIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(MoreAPOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTMoreAPObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t MoreAPOid = 0;
	objIndex_t MoreAPIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(MoreAPIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	MoreAPOid = zcfgFeObjNameToObjId(tr181Obj, &MoreAPIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(MoreAPOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(MoreAPOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, MoreAPOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless.WPS */
zcfgRet_t zyTTWPSObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *WPSObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_WPS, &objIid, &WPSObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(WPSObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(WPSObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTWPSObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *WPSObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_WPS, &objIid, &WPSObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = WPSObj;
		WPSObj = NULL;
		WPSObj = zcfgFeJsonMultiObjAppend(RDM_OID_WPS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(WPSObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(WPSObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WPS, &objIid, WPSObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(WPSObj);
			return ret;
		}
		json_object_put(WPSObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTWPSObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTWPSObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t WPSOid = 0;
	objIndex_t WPSIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(WPSIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	WPSOid = zcfgFeObjNameToObjId(tr181Obj, &WPSIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(WPSOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTWPSObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t WPSOid = 0;
	objIndex_t WPSIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(WPSIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	WPSOid = zcfgFeObjNameToObjId(tr181Obj, &WPSIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(WPSOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(WPSOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, WPSOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless.WMM */
zcfgRet_t zyTTWMMObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *WMMObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_WMM, &objIid, &WMMObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(WMMObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(WMMObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTWMMObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *WMMObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_WMM, &objIid, &WMMObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = WMMObj;
		WMMObj = NULL;
		WMMObj = zcfgFeJsonMultiObjAppend(RDM_OID_WMM, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(WMMObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(WMMObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WMM, &objIid, WMMObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(WMMObj);
			return ret;
		}
		json_object_put(WMMObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTWMMObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTWMMObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t WMMOid = 0;
	objIndex_t WMMIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(WMMIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	WMMOid = zcfgFeObjNameToObjId(tr181Obj, &WMMIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(WMMOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTWMMObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t WMMOid = 0;
	objIndex_t WMMIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(WMMIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	WMMOid = zcfgFeObjNameToObjId(tr181Obj, &WMMIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(WMMOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(WMMOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, WMMOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless.Others */
zcfgRet_t zyTTOthersObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *OthersObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_OTHERS, &objIid, &OthersObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(OthersObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(OthersObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTOthersObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *OthersObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_OTHERS, &objIid, &OthersObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = OthersObj;
		OthersObj = NULL;
		OthersObj = zcfgFeJsonMultiObjAppend(RDM_OID_OTHERS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(OthersObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(OthersObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_OTHERS, &objIid, OthersObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(OthersObj);
			return ret;
		}
		json_object_put(OthersObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTOthersObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTOthersObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t OthersOid = 0;
	objIndex_t OthersIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(OthersIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	OthersOid = zcfgFeObjNameToObjId(tr181Obj, &OthersIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(OthersOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTOthersObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t OthersOid = 0;
	objIndex_t OthersIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(OthersIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	OthersOid = zcfgFeObjNameToObjId(tr181Obj, &OthersIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(OthersOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(OthersOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, OthersOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.HomeNetworking.LANSetup */
zcfgRet_t zyTTLANSetupObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LANSetupObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_LAN_SETUP, &objIid, &LANSetupObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(LANSetupObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(LANSetupObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTLANSetupObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LANSetupObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_LAN_SETUP, &objIid, &LANSetupObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = LANSetupObj;
		LANSetupObj = NULL;
		LANSetupObj = zcfgFeJsonMultiObjAppend(RDM_OID_LAN_SETUP, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(LANSetupObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(LANSetupObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_LAN_SETUP, &objIid, LANSetupObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(LANSetupObj);
			return ret;
		}
		json_object_put(LANSetupObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTLANSetupObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTLANSetupObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t LANSetupOid = 0;
	objIndex_t LANSetupIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(LANSetupIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LANSetupOid = zcfgFeObjNameToObjId(tr181Obj, &LANSetupIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(LANSetupOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTLANSetupObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t LANSetupOid = 0;
	objIndex_t LANSetupIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(LANSetupIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LANSetupOid = zcfgFeObjNameToObjId(tr181Obj, &LANSetupIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(LANSetupOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(LANSetupOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, LANSetupOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.HomeNetworking.LANVLAN */
zcfgRet_t zyTTLANVLANObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LANVLANObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_LAN_VLAN, &objIid, &LANVLANObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(LANVLANObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(LANVLANObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTLANVLANObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LANVLANObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_LAN_VLAN, &objIid, &LANVLANObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = LANVLANObj;
		LANVLANObj = NULL;
		LANVLANObj = zcfgFeJsonMultiObjAppend(RDM_OID_LAN_VLAN, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(LANVLANObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(LANVLANObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_LAN_VLAN, &objIid, LANVLANObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(LANVLANObj);
			return ret;
		}
		json_object_put(LANVLANObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTLANVLANObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTLANVLANObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t LANVLANOid = 0;
	objIndex_t LANVLANIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(LANVLANIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LANVLANOid = zcfgFeObjNameToObjId(tr181Obj, &LANVLANIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(LANVLANOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTLANVLANObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t LANVLANOid = 0;
	objIndex_t LANVLANIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(LANVLANIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LANVLANOid = zcfgFeObjNameToObjId(tr181Obj, &LANVLANIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(LANVLANOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(LANVLANOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, LANVLANOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.Routing */
zcfgRet_t zyTTRoutingObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RoutingObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_PAGE_ROUTING, &objIid, &RoutingObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(RoutingObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(RoutingObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTRoutingObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RoutingObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_PAGE_ROUTING, &objIid, &RoutingObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = RoutingObj;
		RoutingObj = NULL;
		RoutingObj = zcfgFeJsonMultiObjAppend(RDM_OID_PAGE_ROUTING, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(RoutingObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(RoutingObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_PAGE_ROUTING, &objIid, RoutingObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(RoutingObj);
			return ret;
		}
		json_object_put(RoutingObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTRoutingObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTRoutingObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t RoutingOid = 0;
	objIndex_t RoutingIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(RoutingIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	RoutingOid = zcfgFeObjNameToObjId(tr181Obj, &RoutingIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(RoutingOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTRoutingObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t RoutingOid = 0;
	objIndex_t RoutingIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(RoutingIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	RoutingOid = zcfgFeObjNameToObjId(tr181Obj, &RoutingIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(RoutingOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(RoutingOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, RoutingOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.NetworkSetting.NAT.ALG */
zcfgRet_t zyTTALGObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ALGObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ALG, &objIid, &ALGObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(ALGObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(ALGObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTALGObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ALGObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ALG, &objIid, &ALGObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = ALGObj;
		ALGObj = NULL;
		ALGObj = zcfgFeJsonMultiObjAppend(RDM_OID_ALG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ALGObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ALGObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ALG, &objIid, ALGObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ALGObj);
			return ret;
		}
		json_object_put(ALGObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTALGObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTALGObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t ALGOid = 0;
	objIndex_t ALGIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(ALGIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ALGOid = zcfgFeObjNameToObjId(tr181Obj, &ALGIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(ALGOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTALGObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t ALGOid = 0;
	objIndex_t ALGIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(ALGIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ALGOid = zcfgFeObjNameToObjId(tr181Obj, &ALGIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(ALGOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(ALGOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, ALGOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Security.Firewall.DoS */
zcfgRet_t zyTTDoSObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *DoSObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_DOS, &objIid, &DoSObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(DoSObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(DoSObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTDoSObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *DoSObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DOS, &objIid, &DoSObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = DoSObj;
		DoSObj = NULL;
		DoSObj = zcfgFeJsonMultiObjAppend(RDM_OID_DOS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(DoSObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(DoSObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DOS, &objIid, DoSObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(DoSObj);
			return ret;
		}
		json_object_put(DoSObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTDoSObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTDoSObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t DoSOid = 0;
	objIndex_t DoSIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(DoSIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	DoSOid = zcfgFeObjNameToObjId(tr181Obj, &DoSIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(DoSOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTDoSObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t DoSOid = 0;
	objIndex_t DoSIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(DoSIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	DoSOid = zcfgFeObjNameToObjId(tr181Obj, &DoSIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(DoSOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(DoSOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, DoSOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.SystemMonitor.Log */
zcfgRet_t zyTTLogObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LogObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_LOG, &objIid, &LogObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(LogObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(LogObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTLogObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LogObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_LOG, &objIid, &LogObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = LogObj;
		LogObj = NULL;
		LogObj = zcfgFeJsonMultiObjAppend(RDM_OID_LOG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(LogObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(LogObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_LOG, &objIid, LogObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(LogObj);
			return ret;
		}
		json_object_put(LogObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTLogObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTLogObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t LogOid = 0;
	objIndex_t LogIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(LogIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LogOid = zcfgFeObjNameToObjId(tr181Obj, &LogIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(LogOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTLogObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t LogOid = 0;
	objIndex_t LogIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(LogIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LogOid = zcfgFeObjNameToObjId(tr181Obj, &LogIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(LogOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(LogOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, LogOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Easymenu.EM_RemoteAccess */
zcfgRet_t zyTTEMRemoteAccessObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteAccessObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_E_M__REMOTE_ACCESS, &objIid, &RemoteAccessObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(RemoteAccessObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(RemoteAccessObj);

	return ZCFG_SUCCESS;
}
zcfgRet_t zyTTEMRemoteAccessObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LocalAccessObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_E_M__REMOTE_ACCESS, &objIid, &LocalAccessObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = LocalAccessObj;
		LocalAccessObj = NULL;
		LocalAccessObj = zcfgFeJsonMultiObjAppend(RDM_OID_E_M__REMOTE_ACCESS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(LocalAccessObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(LocalAccessObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_E_M__REMOTE_ACCESS, &objIid, LocalAccessObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(LocalAccessObj);
			return ret;
		}
		json_object_put(LocalAccessObj);
	}
		  
	return ZCFG_SUCCESS;
}
/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Easymenu.EM_LocalAccess */
zcfgRet_t zyTTEMLoaclAccessObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LocalAccessObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_E_M__LOCAL_ACCESS, &objIid, &LocalAccessObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(LocalAccessObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(LocalAccessObj);

	return ZCFG_SUCCESS;
}
zcfgRet_t zyTTEMLoaclAccessObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LocalAccessObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_E_M__LOCAL_ACCESS, &objIid, &LocalAccessObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = LocalAccessObj;
		LocalAccessObj = NULL;
		LocalAccessObj = zcfgFeJsonMultiObjAppend(RDM_OID_E_M__LOCAL_ACCESS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(LocalAccessObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(LocalAccessObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_E_M__LOCAL_ACCESS, &objIid, LocalAccessObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(LocalAccessObj);
			return ret;
		}
		json_object_put(LocalAccessObj);
	}
		  
	return ZCFG_SUCCESS;
}
/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Maintenance.RemoteMGMT */
zcfgRet_t zyTTRemoteMGMTObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMGMTObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_REMOTE_MGMT, &objIid, &RemoteMGMTObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(RemoteMGMTObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(RemoteMGMTObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTRemoteMGMTObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *RemoteMGMTObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_REMOTE_MGMT, &objIid, &RemoteMGMTObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = RemoteMGMTObj;
		RemoteMGMTObj = NULL;
		RemoteMGMTObj = zcfgFeJsonMultiObjAppend(RDM_OID_REMOTE_MGMT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(RemoteMGMTObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(RemoteMGMTObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_REMOTE_MGMT, &objIid, RemoteMGMTObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(RemoteMGMTObj);
			return ret;
		}
		json_object_put(RemoteMGMTObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTRemoteMGMTObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTRemoteMGMTObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t RemoteMGMTOid = 0;
	objIndex_t RemoteMGMTIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(RemoteMGMTIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	RemoteMGMTOid = zcfgFeObjNameToObjId(tr181Obj, &RemoteMGMTIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(RemoteMGMTOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTRemoteMGMTObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t RemoteMGMTOid = 0;
	objIndex_t RemoteMGMTIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(RemoteMGMTIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	RemoteMGMTOid = zcfgFeObjNameToObjId(tr181Obj, &RemoteMGMTIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(RemoteMGMTOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(RemoteMGMTOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, RemoteMGMTOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Maintenance.TR069 */
zcfgRet_t zyTTTR069ObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *TR069Obj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_TR069, &objIid, &TR069Obj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(TR069Obj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(TR069Obj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTTR069ObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *TR069Obj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_TR069, &objIid, &TR069Obj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = TR069Obj;
		TR069Obj = NULL;
		TR069Obj = zcfgFeJsonMultiObjAppend(RDM_OID_TR069, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(TR069Obj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(TR069Obj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_TR069, &objIid, TR069Obj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(TR069Obj);
			return ret;
		}
		json_object_put(TR069Obj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTTR069ObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTTR069ObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t TR069Oid = 0;
	objIndex_t TR069Iid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(TR069Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	TR069Oid = zcfgFeObjNameToObjId(tr181Obj, &TR069Iid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(TR069Oid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTTR069ObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t TR069Oid = 0;
	objIndex_t TR069Iid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(TR069Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	TR069Oid = zcfgFeObjNameToObjId(tr181Obj, &TR069Iid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(TR069Oid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(TR069Oid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, TR069Oid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Maintenance.Time */
zcfgRet_t zyTTTimeObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *TimeObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_PAGE_TIME, &objIid, &TimeObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(TimeObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(TimeObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTTimeObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *TimeObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_PAGE_TIME, &objIid, &TimeObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = TimeObj;
		TimeObj = NULL;
		TimeObj = zcfgFeJsonMultiObjAppend(RDM_OID_PAGE_TIME, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(TimeObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(TimeObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_PAGE_TIME, &objIid, TimeObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(TimeObj);
			return ret;
		}
		json_object_put(TimeObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTTimeObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTTimeObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t TimeOid = 0;
	objIndex_t TimeIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(TimeIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	TimeOid = zcfgFeObjNameToObjId(tr181Obj, &TimeIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(TimeOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTTimeObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t TimeOid = 0;
	objIndex_t TimeIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(TimeIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	TimeOid = zcfgFeObjNameToObjId(tr181Obj, &TimeIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(TimeOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(TimeOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, TimeOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Maintenance.LogSetting */
zcfgRet_t zyTTLogSettingObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LogSettingObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_LOGSETTING, &objIid, &LogSettingObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(LogSettingObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(LogSettingObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTLogSettingObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LogSettingObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_LOGSETTING, &objIid, &LogSettingObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = LogSettingObj;
		LogSettingObj = NULL;
		LogSettingObj = zcfgFeJsonMultiObjAppend(RDM_OID_LOGSETTING, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(LogSettingObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(LogSettingObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_LOGSETTING, &objIid, LogSettingObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(LogSettingObj);
			return ret;
		}
		json_object_put(LogSettingObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTLogSettingObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTLogSettingObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t LogSettingOid = 0;
	objIndex_t LogSettingIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(LogSettingIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LogSettingOid = zcfgFeObjNameToObjId(tr181Obj, &LogSettingIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(LogSettingOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTLogSettingObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t LogSettingOid = 0;
	objIndex_t LogSettingIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(LogSettingIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LogSettingOid = zcfgFeObjNameToObjId(tr181Obj, &LogSettingIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(LogSettingOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(LogSettingOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, LogSettingOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Maintenance.FirmwareUpgrade */
zcfgRet_t zyTTFirmwareUpgradeObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *FirmwareUpgradeObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_FIRMWARE_UPGRADE, &objIid, &FirmwareUpgradeObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(FirmwareUpgradeObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(FirmwareUpgradeObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTFirmwareUpgradeObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *FirmwareUpgradeObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_FIRMWARE_UPGRADE, &objIid, &FirmwareUpgradeObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = FirmwareUpgradeObj;
		FirmwareUpgradeObj = NULL;
		FirmwareUpgradeObj = zcfgFeJsonMultiObjAppend(RDM_OID_FIRMWARE_UPGRADE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(FirmwareUpgradeObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(FirmwareUpgradeObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_FIRMWARE_UPGRADE, &objIid, FirmwareUpgradeObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(FirmwareUpgradeObj);
			return ret;
		}
		json_object_put(FirmwareUpgradeObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTFirmwareUpgradeObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTFirmwareUpgradeObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t FirmwareUpgradeOid = 0;
	objIndex_t FirmwareUpgradeIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(FirmwareUpgradeIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	FirmwareUpgradeOid = zcfgFeObjNameToObjId(tr181Obj, &FirmwareUpgradeIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(FirmwareUpgradeOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTFirmwareUpgradeObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t FirmwareUpgradeOid = 0;
	objIndex_t FirmwareUpgradeIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(FirmwareUpgradeIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	FirmwareUpgradeOid = zcfgFeObjNameToObjId(tr181Obj, &FirmwareUpgradeIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(FirmwareUpgradeOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(FirmwareUpgradeOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, FirmwareUpgradeOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Maintenance.Configuration */
zcfgRet_t zyTTConfigurationObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ConfigurationObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_CONFIGURATION, &objIid, &ConfigurationObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(ConfigurationObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(ConfigurationObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTConfigurationObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ConfigurationObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_CONFIGURATION, &objIid, &ConfigurationObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = ConfigurationObj;
		ConfigurationObj = NULL;
		ConfigurationObj = zcfgFeJsonMultiObjAppend(RDM_OID_CONFIGURATION, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ConfigurationObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ConfigurationObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_CONFIGURATION, &objIid, ConfigurationObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ConfigurationObj);
			return ret;
		}
		json_object_put(ConfigurationObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTConfigurationObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTConfigurationObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t ConfigurationOid = 0;
	objIndex_t ConfigurationIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(ConfigurationIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ConfigurationOid = zcfgFeObjNameToObjId(tr181Obj, &ConfigurationIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(ConfigurationOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTConfigurationObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t ConfigurationOid = 0;
	objIndex_t ConfigurationIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(ConfigurationIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ConfigurationOid = zcfgFeObjNameToObjId(tr181Obj, &ConfigurationIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(ConfigurationOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(ConfigurationOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, ConfigurationOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Maintenance.EDNS */
zcfgRet_t zyTTEDNSPageObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *EDNSPageObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_EDNS, &objIid, &EDNSPageObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(EDNSPageObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(EDNSPageObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTEDNSPageObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *EDNSPageObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_EDNS, &objIid, &EDNSPageObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = EDNSPageObj;
		EDNSPageObj = NULL;
		EDNSPageObj = zcfgFeJsonMultiObjAppend(RDM_OID_EDNS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(EDNSPageObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(EDNSPageObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_EDNS, &objIid, EDNSPageObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(EDNSPageObj);
			return ret;
		}
		json_object_put(EDNSPageObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTEDNSPageObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTEDNSPageObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t EDNSPageOid = 0;
	objIndex_t EDNSPageIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(EDNSPageIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	EDNSPageOid = zcfgFeObjNameToObjId(tr181Obj, &EDNSPageIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(EDNSPageOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTEDNSPageObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t EDNSPageOid = 0;
	objIndex_t EDNSPageIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(EDNSPageIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	EDNSPageOid = zcfgFeObjNameToObjId(tr181Obj, &EDNSPageIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(EDNSPageOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(EDNSPageOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, EDNSPageOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Engdebug */
zcfgRet_t zyTTEngdebugObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *EngdebugObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ENGDEBUG, &objIid, &EngdebugObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(EngdebugObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(EngdebugObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTEngdebugObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *EngdebugObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ENGDEBUG, &objIid, &EngdebugObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = EngdebugObj;
		EngdebugObj = NULL;
		EngdebugObj = zcfgFeJsonMultiObjAppend(RDM_OID_ENGDEBUG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(EngdebugObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(EngdebugObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ENGDEBUG, &objIid, EngdebugObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(EngdebugObj);
			return ret;
		}
		json_object_put(EngdebugObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTEngdebugObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTEngdebugObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t EngdebugOid = 0;
	objIndex_t EngdebugIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(EngdebugIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	EngdebugOid = zcfgFeObjNameToObjId(tr181Obj, &EngdebugIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(EngdebugOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTEngdebugObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t EngdebugOid = 0;
	objIndex_t EngdebugIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(EngdebugIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	EngdebugOid = zcfgFeObjNameToObjId(tr181Obj, &EngdebugIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(EngdebugOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(EngdebugOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, EngdebugOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.GUI.AccessRights.Page.Capture */
zcfgRet_t zyTTCaptureObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *CaptureObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_CAPTURE, &objIid, &CaptureObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(CaptureObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(CaptureObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTCaptureObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *CaptureObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_CAPTURE, &objIid, &CaptureObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = CaptureObj;
		CaptureObj = NULL;
		CaptureObj = zcfgFeJsonMultiObjAppend(RDM_OID_CAPTURE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(CaptureObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(CaptureObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_CAPTURE, &objIid, CaptureObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(CaptureObj);
			return ret;
		}
		json_object_put(CaptureObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTCaptureObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTCaptureObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t CaptureOid = 0;
	objIndex_t CaptureIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(CaptureIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	CaptureOid = zcfgFeObjNameToObjId(tr181Obj, &CaptureIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(CaptureOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTCaptureObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t CaptureOid = 0;
	objIndex_t CaptureIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(CaptureIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	CaptureOid = zcfgFeObjNameToObjId(tr181Obj, &CaptureIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(CaptureOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(CaptureOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, CaptureOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Configuration.Shell */
zcfgRet_t zyTTShellObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ShellObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_SHELL, &objIid, &ShellObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(ShellObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(ShellObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTShellObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ShellObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_SHELL, &objIid, &ShellObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = ShellObj;
		ShellObj = NULL;
		ShellObj = zcfgFeJsonMultiObjAppend(RDM_OID_SHELL, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ShellObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ShellObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_SHELL, &objIid, ShellObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ShellObj);
			return ret;
		}
		json_object_put(ShellObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTShellObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTShellObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t ShellOid = 0;
	objIndex_t ShellIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(ShellIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ShellOid = zcfgFeObjNameToObjId(tr181Obj, &ShellIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(ShellOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTShellObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t ShellOid = 0;
	objIndex_t ShellIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(ShellIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	ShellOid = zcfgFeObjNameToObjId(tr181Obj, &ShellIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(ShellOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(ShellOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, ShellOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif

#ifdef CONFIG_TAAAB_PORTMIRROR
zcfgRet_t GetPMHistory(char *value)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	FILE* PMHistory;
	char filename[64]= {0};
	char line[256];
	char newline[1024]= {0};
	char history[10240]= {0};
	char MonitorIface[512] = {0};
	char oldMonitorIface[512] = {0};
	char origin[16] = {0};
	char SourceIP[64] = {0};
	char timestamp[32] = {0};
	char Direction[16] = {0};
	char MirrorInterface[8] = {0};
	char Timeout[16] = {0};
	char iface[40] = {0};
	char lowerLayer[32] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};


	strcpy(filename,"/mnt/misc/PMHistory");
	if((PMHistory = fopen(filename,"r")) == NULL){
		printf("open %s fail !!",filename);
		strcpy(value,"");
	}else{
		while ( fgets(line, sizeof(line), PMHistory) ){
			sscanf(line,"%s %s %s %s %s %s %s |",timestamp, SourceIP, origin, oldMonitorIface, Direction, MirrorInterface, Timeout);

					strcpy(iface, oldMonitorIface);
					zcfgFeTr181IfaceStackLowerLayerGet(iface, lowerLayer);
					if(strstr(lowerLayer, "PPP.Interface") != NULL) {
						memset(iface, 0, sizeof(iface));
						strcpy(iface, lowerLayer);
						}
					zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName);
					strcpy(tr98ObjName, tmpTr98ObjName);
					ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName);
				    strcpy(MonitorIface, tr98ObjName);
                    
            sprintf(newline,"%s %s %s %s %s %s %s |", timestamp, SourceIP, origin, MonitorIface, Direction, MirrorInterface, Timeout);
			strcat(history,newline);
		}
		fclose(PMHistory);
		strcpy(value,history);
		printf("history = %s\n",history);
	}
	return ret;
}

/* InternetGatewayDevice.X_TT.PortMirroring */
zcfgRet_t zyTTPORTMIRRORObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *portmirrorObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char getvalue[1024] = {0};
	char cpy_monitorInterface[512] = {0};
	char *token = NULL;
	char singleintf[128] = {0};
	char gethistory[10240]= {0};
	char devIpIface[48] = {0};
	char iface[40] = {0};
	char lowerLayer[32] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_TAAAB_PORT_MIRROR, &objIid, &portmirrorObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(portmirrorObj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name, "MonitorInterface")) {
			paramValue = json_object_object_get(portmirrorObj, paramList->name);
			strcpy(devIpIface, json_object_get_string(paramValue));
			if(!strcmp(devIpIface, "")) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
					}
			else {
				strcpy(iface, devIpIface);
				if((zcfgFeTr181IfaceStackLowerLayerGet(iface, lowerLayer)) != ZCFG_SUCCESS) {
					json_object_put(portmirrorObj);
					return ZCFG_NO_SUCH_OBJECT;
					}
				if(strstr(lowerLayer, "PPP.Interface") != NULL) {
					memset(iface, 0, sizeof(iface));
					strcpy(iface, lowerLayer);
					}
				if((zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName)) != ZCFG_SUCCESS) {
					json_object_put(portmirrorObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				strcpy(tr98ObjName, tmpTr98ObjName);
				ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName);
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));
				}
				paramList++;
				continue;
			}

			if(!strcmp(paramList->name, "History")) {
				if(GetPMHistory(gethistory) != ZCFG_SUCCESS) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		        }
				else{
				    json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(gethistory));
				}
				paramList++;
				continue;
			}
			
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(portmirrorObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTPORTMIRRORObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t portmirrorobjIid;
	struct json_object *portmirrorObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char setIfaceList[1024] = {0};
    const char *setvalue = NULL;
	char devIpIface[300] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(portmirrorobjIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_TAAAB_PORT_MIRROR, &portmirrorobjIid, &portmirrorObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
		if(multiJobj){
			if(portmirrorObj){
				tmpObj = portmirrorObj;
				portmirrorObj = NULL;
				portmirrorObj = zcfgFeJsonMultiObjAppend(RDM_OID_TAAAB_PORT_MIRROR, &portmirrorobjIid, multiJobj, tmpObj);
				json_object_put(tmpObj);
			}
		}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name, "MonitorInterface")) {
			sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
			if(paramValue != NULL && json_object_get_string(paramValue)) {
				strcpy(tmpTr98ConnObjName, json_object_get_string(paramValue));
				strcpy(tr98ConnObjName, tmpTr98ConnObjName);
				ReplaceWANDeviceIndex(tmpTr98ConnObjName, tr98ConnObjName);
				if(zcfgFe98To181IpIface(tr98ConnObjName, devIpIface) == ZCFG_SUCCESS){
					json_object_object_add(portmirrorObj, paramList->name, json_object_new_string(devIpIface));
				}else{
					if(!multiJobj)
						json_object_put(portmirrorObj);

					sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
					return ZCFG_INVALID_PARAM_VALUE;
				}
				}
			strcpy(paramfault, "");
				paramList++;
				continue;
			}

            if(!strcmp(paramList->name, "Direction")) {
				setvalue = json_object_get_string(paramValue);
                if(strcmp(setvalue,"ingress") && strcmp(setvalue,"egress") && strcmp(setvalue,"both") ){
					json_object_put(portmirrorObj);
					return ZCFG_REQUEST_REJECT;
				}
				json_object_object_add(portmirrorObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

		    tr181ParamValue = json_object_object_get(portmirrorObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(portmirrorObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if(portmirrorObj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_TAAAB_PORT_MIRROR, &portmirrorobjIid, portmirrorObj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set RDM_OID_TAAAB_PORT_MIRROR Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set RDM_OID_TAAAB_PORT_MIRROR Success\n", __FUNCTION__);
			}
			json_object_put(portmirrorObj);
		}
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTPORTMIRRORObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTPORTMIRRORObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  TR069Oid = 0;
	objIndex_t TR069Iid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(TR069Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	TR069Oid = zcfgFeObjNameToObjId(tr181Obj, &TR069Iid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(TR069Oid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTPORTMIRRORObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  TR069Oid = 0;
	objIndex_t TR069Iid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(TR069Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	TR069Oid = zcfgFeObjNameToObjId(tr181Obj, &TR069Iid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(TR069Oid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(TR069Oid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, TR069Oid, paramList->name);
		}
		break;
	}
	
	return ret;
}

#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LIFEMOTE
/* InternetGatewayDevice.X_TTNET.Configuration.LifemoteAgent */
/* InternetGatewayDevice.X_ZYXEL_LifeMoteAgent */
zcfgRet_t zyExtLifemoteObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LifemoteObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	FILE *fp_lifemote = NULL;
	char tempBuffer[256] = {0};
	bool isfound = false;
	char testresult[32] = {0};
	
	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_LIFEMOTE_AGENT, &objIid, &LifemoteObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(LifemoteObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(LifemoteObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtLifemoteObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *LifemoteObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_LIFEMOTE_AGENT, &objIid, &LifemoteObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = LifemoteObj;
		LifemoteObj = NULL;
		LifemoteObj = zcfgFeJsonMultiObjAppend(RDM_OID_LIFEMOTE_AGENT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(LifemoteObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(LifemoteObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_LIFEMOTE_AGENT, &objIid, LifemoteObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(LifemoteObj);
			return ret;
		}
		json_object_put(LifemoteObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtLifemoteObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = {0};
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

int zyExtLifemoteObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[135] = {0};
	uint32_t LifemoteOid = 0;
	objIndex_t HwWiFiButtonIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(HwWiFiButtonIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LifemoteOid = zcfgFeObjNameToObjId(tr181Obj, &HwWiFiButtonIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(LifemoteOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtLifemoteObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[135] = {0};
	uint32_t LifemoteOid = 0;
	objIndex_t HwWiFiButtonIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(HwWiFiButtonIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	LifemoteOid = zcfgFeObjNameToObjId(tr181Obj, &HwWiFiButtonIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(LifemoteOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(LifemoteOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, LifemoteOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_QUANTWIFI
/* InternetGatewayDevice.X_TTNET.Configuration.QuantWifiAgent */
zcfgRet_t zyExtQuantwifiObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *QuantwifiObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	FILE *fp_quantwifi = NULL;
	char tempBuffer[256]="";
	bool isfound = false;
	char testresult[32]="";

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_QUANT_WIFI_AGENT, &objIid, &QuantwifiObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		if(!strcmp(paramList->name, "State")) {
			system("ps | grep quantwifi.sh >/tmp/quantwifistatus");
			isfound = false;
			if ((fp_quantwifi = fopen("/tmp/quantwifistatus","r")) != NULL ){
				while(fgets(tempBuffer, 256, fp_quantwifi)!= NULL) {
					if(strstr(tempBuffer,"ttnet-quantwifi.sh")){
						isfound = true;
						break;
					}
				}
				fclose(fp_quantwifi);

				system("rm -rf /tmp/quantwifistatus");

				if(isfound)
					strcpy(testresult, "RUNNING");
				else
					strcpy(testresult, "NONE");
			}
			else{
				strcpy(testresult, "NONE");
			}
			
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(testresult));
			paramList++;
			continue;
		}
		else
		{		
			paramValue = json_object_object_get(QuantwifiObj, paramList->name);
			if(paramValue != NULL){
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(QuantwifiObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtQuantwifiObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *QuantwifiObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_QUANT_WIFI_AGENT, &objIid, &QuantwifiObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = QuantwifiObj;
		QuantwifiObj = NULL;
		QuantwifiObj = zcfgFeJsonMultiObjAppend(RDM_OID_QUANT_WIFI_AGENT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(QuantwifiObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(QuantwifiObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_LIFEMOTE_AGENT, &objIid, QuantwifiObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(QuantwifiObj);
			return ret;
		}
		json_object_put(QuantwifiObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtQuantwifiObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyExtQuantwifiObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t QuantwifiOid = 0;
	objIndex_t HwWiFiButtonIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(HwWiFiButtonIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	QuantwifiOid = zcfgFeObjNameToObjId(tr181Obj, &HwWiFiButtonIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(QuantwifiOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyExtQuantwifiObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t QuantwifiOid = 0;
	objIndex_t HwWiFiButtonIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(HwWiFiButtonIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	QuantwifiOid = zcfgFeObjNameToObjId(tr181Obj, &HwWiFiButtonIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(QuantwifiOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(QuantwifiOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, QuantwifiOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
zcfgRet_t GetPacketCaptureHistory(char *value)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	FILE* CaptureHistory;
	char filename[64]= {0};
	char line[256];
	char history[1024]= {0};

	strcpy(filename,"/data/CaptureHistory");
	if((CaptureHistory = fopen(filename,"r")) == NULL){
		printf("open %s fail !!",filename);
		strcpy(value,"");
	}else{
		while ( fgets(line, sizeof(line), CaptureHistory) ){
			strcat(history,line);
		}
		fclose(CaptureHistory);
		strcpy(value,history);
		printf("history = %s\n",history);
	}
	return ret;
}

/* InternetGatewayDevice.X_TT.PacketCapture */
zcfgRet_t zyTTPACKETCAPTUREObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *packetcaptureObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char getvalue[256] = {0};
	char gethistory[1024]= {0};
    char devIpIface[48] = {0};
	char iface[40] = {0};
	char lowerLayer[32] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_PACKET_CAPTURE, &objIid, &packetcaptureObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(packetcaptureObj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name, "MonitorInterface")) {
			paramValue = json_object_object_get(packetcaptureObj, paramList->name);
			strcpy(devIpIface, json_object_get_string(paramValue));
			if(!strcmp(devIpIface, "")) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
		        }
			    else if(!strcmp(devIpIface, "ALL")){
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			    }
				else{
				strcpy(iface, devIpIface);
				if((zcfgFeTr181IfaceStackLowerLayerGet(iface, lowerLayer)) != ZCFG_SUCCESS) {
					json_object_put(packetcaptureObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				if(strstr(lowerLayer, "PPP.Interface") != NULL) {
					memset(iface, 0, sizeof(iface));
					strcpy(iface, lowerLayer);
				}
				if((zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName)) != ZCFG_SUCCESS) {
					json_object_put(packetcaptureObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				strcpy(tr98ObjName, tmpTr98ObjName);
				ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName);
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));
				}
				paramList++;
				continue;
			}

			if(!strcmp(paramList->name, "History")) {
				if(GetPacketCaptureHistory(gethistory) != ZCFG_SUCCESS) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		        }
				else{
				    json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(gethistory));
				}
				paramList++;
				continue;
			}

			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(packetcaptureObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTPACKETCAPTUREObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t packetcaptureobjIid;
	struct json_object *packetcaptureObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char setIfaceList[128] = {0};
    const char *setvalue = NULL;
	char devIpIface[300] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
    char tmpTr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(packetcaptureobjIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_PACKET_CAPTURE, &packetcaptureobjIid, &packetcaptureObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
		if(multiJobj){
			if(packetcaptureObj){
				tmpObj = packetcaptureObj;
				packetcaptureObj = NULL;
				packetcaptureObj = zcfgFeJsonMultiObjAppend(RDM_OID_PACKET_CAPTURE, &packetcaptureobjIid, multiJobj, tmpObj);
				json_object_put(tmpObj);
			}
		}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name, "MonitorInterface")) {
			sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
			if(paramValue != NULL && json_object_get_string(paramValue)) {
			   if(!strcmp(json_object_get_string(paramValue),"ALL")){
				   json_object_object_add(packetcaptureObj, paramList->name, JSON_OBJ_COPY(paramValue));
			   	}
			   else{
				strcpy(tmpTr98ConnObjName, json_object_get_string(paramValue));
				strcpy(tr98ConnObjName, tmpTr98ConnObjName);
				ReplaceWANDeviceIndex(tmpTr98ConnObjName, tr98ConnObjName);
				if(zcfgFe98To181IpIface(tr98ConnObjName, devIpIface) == ZCFG_SUCCESS){
					json_object_object_add(packetcaptureObj, paramList->name, json_object_new_string(devIpIface));
				}else{
					if(!multiJobj)
					json_object_put(packetcaptureObj);

					sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
					return ZCFG_INVALID_PARAM_VALUE;
				}
				}
				}
			strcpy(paramfault, "");
				paramList++;
				continue;
			}

            if(!strcmp(paramList->name, "Direction")) {
				setvalue = json_object_get_string(paramValue);
                if(strcmp(setvalue,"ingress") && strcmp(setvalue,"egress") && strcmp(setvalue,"both") ){
					json_object_put(packetcaptureObj);
					return ZCFG_REQUEST_REJECT;
				}
				json_object_object_add(packetcaptureObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

            if(!strcmp(paramList->name, "Storage")) {
				setvalue = json_object_get_string(paramValue);
                if(strcmp(setvalue,"internal") && strcmp(setvalue,"external") ){
					json_object_put(packetcaptureObj);
					return ZCFG_REQUEST_REJECT;
				}
				json_object_object_add(packetcaptureObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

		    tr181ParamValue = json_object_object_get(packetcaptureObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(packetcaptureObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if(packetcaptureObj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_PACKET_CAPTURE, &packetcaptureobjIid, packetcaptureObj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set RDM_OID_PACKET_CAPTURE Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set RDM_OID_PACKET_CAPTURE Success\n", __FUNCTION__);
			}
			json_object_put(packetcaptureObj);
		}
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTPACKETCAPTUREObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTPACKETCAPTUREObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  TR069Oid = 0;
	objIndex_t TR069Iid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(TR069Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	TR069Oid = zcfgFeObjNameToObjId(tr181Obj, &TR069Iid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(TR069Oid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTPACKETCAPTUREObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  TR069Oid = 0;
	objIndex_t TR069Iid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(TR069Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	TR069Oid = zcfgFeObjNameToObjId(tr181Obj, &TR069Iid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(TR069Oid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(TR069Oid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, TR069Oid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.PacketCapture.Upload */
zcfgRet_t zyTTPACKETCAPTUREUPLOADObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *packetcaptureuploadObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_PACKET_CAPTURE_UPLOAD, &objIid, &packetcaptureuploadObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(packetcaptureuploadObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(packetcaptureuploadObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTPACKETCAPTUREUPLOADObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t packetcaptureuploadobjIid;
	struct json_object *packetcaptureuploadObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
    const char *setvalue = NULL;
	
	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(packetcaptureuploadobjIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_PACKET_CAPTURE_UPLOAD, &packetcaptureuploadobjIid, &packetcaptureuploadObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
		if(multiJobj){
			if(packetcaptureuploadObj){
				tmpObj = packetcaptureuploadObj;
				packetcaptureuploadObj = NULL;
				packetcaptureuploadObj = zcfgFeJsonMultiObjAppend(RDM_OID_PACKET_CAPTURE_UPLOAD, &packetcaptureuploadobjIid, multiJobj, tmpObj);
			}
		}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
            if(!strcmp(paramList->name, "Protocol")) {
				setvalue = json_object_get_string(paramValue);
                if(strcmp(setvalue,"ftp") && strcmp(setvalue,"tftp") ){
					json_object_put(packetcaptureuploadObj);
					return ZCFG_REQUEST_REJECT;
				}
				json_object_object_add(packetcaptureuploadObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

            if(!strcmp(paramList->name, "FileSource")) {
				setvalue = json_object_get_string(paramValue);
                if(strcmp(setvalue,"internal") && strcmp(setvalue,"external") ){
					json_object_put(packetcaptureuploadObj);
					return ZCFG_REQUEST_REJECT;
				}
				json_object_object_add(packetcaptureuploadObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

		    tr181ParamValue = json_object_object_get(packetcaptureuploadObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(packetcaptureuploadObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if(packetcaptureuploadObj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_PACKET_CAPTURE_UPLOAD, &packetcaptureuploadobjIid, packetcaptureuploadObj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set RDM_OID_PACKET_CAPTURE_UPLOAD Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set RDM_OID_PACKET_CAPTURE_UPLOAD Success\n", __FUNCTION__);
			}
			json_object_put(packetcaptureuploadObj);
		}
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTPACKETCAPTUREUPLOADObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTPACKETCAPTUREUPLOADObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  TR069Oid = 0;
	objIndex_t TR069Iid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(TR069Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	TR069Oid = zcfgFeObjNameToObjId(tr181Obj, &TR069Iid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(TR069Oid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTPACKETCAPTUREUPLOADObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  TR069Oid = 0;
	objIndex_t TR069Iid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(TR069Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	TR069Oid = zcfgFeObjNameToObjId(tr181Obj, &TR069Iid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(TR069Oid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(TR069Oid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, TR069Oid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif // CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE

#ifdef CONFIG_ZYXEL_TR140
/* InternetGatewayDevice.Services.StorageService.i */
zcfgRet_t SSObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	int triggercount = 0;
	objIndex_t Iid = {0};
	struct json_object *obj = NULL;
	bool SSEnable = false;
	int PhysicalMediumNumber = 0,LogicalVolumeNumber = 0;
	bool samba = false;
	bool printserver = false;
	bool mediaserver = false;
	int SSIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d", &SSIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

#ifdef CONFIG_ZCFG_BE_MODULE_USB
    IID_INIT(Iid);
	//trigger usb backend get usb capacity and usedspace
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV, &Iid, &obj) == ZCFG_SUCCESS){
		zcfgFeJsonObjFree(obj);
	}

		IID_INIT(Iid);
		Iid.level=2;
		Iid.idx[0]=1;
		Iid.idx[1]=2;
		if(zcfgFeObjJsonGet(RDM_OID_USB_USBHOSTS_HOST_DEV, &Iid, &obj) == ZCFG_SUCCESS){
			PhysicalMediumNumber = 1;
			zcfgFeJsonObjFree(obj);
		}
#else
	printf("%s : USB not support\n", __FUNCTION__);
#endif

    IID_INIT(Iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV_PAR, &Iid, &obj) == ZCFG_SUCCESS){
        LogicalVolumeNumber++;
		zcfgFeJsonObjFree(obj);
	}	

	IID_INIT(Iid);
	if(zcfgFeObjJsonGet(RDM_OID_ZY_SAMBA, &Iid, &obj) == ZCFG_SUCCESS){
			samba = json_object_get_boolean(json_object_object_get(obj, "Enable"));
			zcfgFeJsonObjFree(obj);
	}
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
	IID_INIT(Iid);
	if(zcfgFeObjJsonGet(RDM_OID_ZY_IPP_CFG, &Iid, &obj) == ZCFG_SUCCESS){
		printserver = json_object_get_boolean(json_object_object_get(obj, "Enable"));
		zcfgFeJsonObjFree(obj);
	}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_DLNA
	IID_INIT(Iid);
	if(zcfgFeObjJsonGet(RDM_OID_DLNA, &Iid, &obj) == ZCFG_SUCCESS){
		mediaserver = json_object_get_boolean(json_object_object_get(obj, "X_ZYXEL_Enable"));
		zcfgFeJsonObjFree(obj);
	}
#endif
	if(samba == true || printserver == true || mediaserver == true)
		SSEnable = true;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "Enable")) {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(SSEnable));
						paramList++;
						continue;
		}
		else if(!strcmp(paramList->name, "PhysicalMediumNumberOfEntries")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(PhysicalMediumNumber));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "LogicalVolumeNumberOfEntries")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(LogicalVolumeNumber));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "UserAccountNumberOfEntries")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(2));
					paramList++;
					continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR98 List\n", paramList->name);
		paramList++;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t SSObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t SambaobjIid;
	struct json_object *SambaObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
	struct json_object *PrintServerObj = NULL;
	objIndex_t PrintServerobjIid;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_DLNA
	struct json_object *MediaserverObj = NULL;
	objIndex_t MediaserverobjIid;
#endif
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	bool enable = false;
	bool samba = false,sambaModified = false;
	bool printserver = false,printserverModified = false;
	bool mediaserver = false,mediaserverModified = false;
    int SSIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d", &SSIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			/*special case*/
			if(!strcmp(paramList->name, "Enable")) {
				enable = json_object_get_boolean(paramValue);
				IID_INIT(SambaobjIid);
				if(zcfgFeObjJsonGet(RDM_OID_ZY_SAMBA, &SambaobjIid, &SambaObj) == ZCFG_SUCCESS){
					samba = json_object_get_boolean(json_object_object_get(SambaObj, "Enable"));
					if(enable != samba){
						sambaModified = true;
						json_object_object_add(SambaObj, "Enable", JSON_OBJ_COPY(paramValue));
					}
				}

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
				IID_INIT(PrintServerobjIid);
				if(zcfgFeObjJsonGet(RDM_OID_ZY_IPP_CFG, &PrintServerobjIid, &PrintServerObj) == ZCFG_SUCCESS){
					printserver = json_object_get_boolean(json_object_object_get(PrintServerObj, "Enable"));
					if(enable != printserver){
						printserverModified = true;
					    json_object_object_add(PrintServerObj, "Enable", JSON_OBJ_COPY(paramValue));
					}
				}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_DLNA
				IID_INIT(MediaserverobjIid);
				if(zcfgFeObjJsonGet(RDM_OID_DLNA, &MediaserverobjIid, &MediaserverObj) == ZCFG_SUCCESS){
					mediaserver = json_object_get_boolean(json_object_object_get(MediaserverObj, "X_ZYXEL_Enable"));
					if(enable != mediaserver){
						mediaserverModified = true;
					    json_object_object_add(MediaserverObj, "X_ZYXEL_Enable", JSON_OBJ_COPY(paramValue));
					}
				}
#endif
				paramList++;
				continue;
			}
		}

		paramList++;
	}

	if(SambaObj && sambaModified){
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_SAMBA, &SambaobjIid, SambaObj, NULL)) != ZCFG_SUCCESS ){
		}
	}
	json_object_put(SambaObj);

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
	if(PrintServerObj && printserverModified){
		if((ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_ZY_IPP_CFG, &PrintServerobjIid, PrintServerObj, NULL)) != ZCFG_SUCCESS ){
		}
	}
	json_object_put(PrintServerObj);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_DLNA
	if(MediaserverObj && mediaserverModified){
		if((ret = zcfgFeObjJsonSet(RDM_OID_DLNA, &MediaserverobjIid, MediaserverObj, NULL)) != ZCFG_SUCCESS ){
		}
	}
	json_object_put(MediaserverObj);
#endif
  
	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.Services.StorageService.i.UserAccount.i */
zcfgRet_t SSUserAccountObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	struct json_object *paramValue = NULL;
	objIndex_t Iid = {0};
	struct json_object *obj = NULL;
	int SSIndex = 0;
	int AccountIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.UserAccount.%d", &SSIndex, &AccountIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

		IID_INIT(Iid);
		Iid.level=2;
	    if(AccountIndex == 1){
		Iid.idx[0]=1;
		Iid.idx[1]=1;
	    }
        else if(AccountIndex == 2){
		Iid.idx[0]=2;
		Iid.idx[1]=1;
	    }
		else{
			Iid.idx[0]=3;
			Iid.idx[1]=AccountIndex-2;
		}
		if(zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &Iid, &obj) != ZCFG_SUCCESS){
			zcfgFeJsonObjFree(obj);
			return ZCFG_NO_SUCH_OBJECT;
		}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "Enable")) {
						paramValue = json_object_object_get(obj, "Enabled");
						json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
						paramList++;
						continue;
		}
		else if(!strcmp(paramList->name, "Username")) {
					paramValue = json_object_object_get(obj, "Username");
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "Password")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
					paramList++;
					continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR98 List\n", paramList->name);
		paramList++;
	}

	json_object_put(obj);

	return ZCFG_SUCCESS;
}

zcfgRet_t SSUserAccountObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	objIndex_t Iid = {0};
	objIndex_t tempIid = {0};
	struct json_object *obj = NULL;
	struct json_object *tempobj = NULL;
	int SSIndex = 0;
	int AccountIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.UserAccount.%d", &SSIndex, &AccountIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

		IID_INIT(Iid);
		Iid.level=2;
	    if(AccountIndex == 1){
		Iid.idx[0]=1;
		Iid.idx[1]=1;
	    }
        else if(AccountIndex == 2){
		Iid.idx[0]=2;
		Iid.idx[1]=1;
	    }
		else{
			Iid.idx[0]=3;
			Iid.idx[1]=AccountIndex-2;
		}

		if(zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &Iid, &obj) != ZCFG_SUCCESS){
			zcfgFeJsonObjFree(obj);
			return ZCFG_NO_SUCH_OBJECT;
		}

	if(multiJobj){
		if(obj){
			tmpObj = obj;
			obj = NULL;
			obj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &Iid, multiJobj, tmpObj);
		}
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			/*special case*/
			if(!strcmp(paramList->name, "Enable")) {
				json_object_object_add(obj, "Enabled", JSON_OBJ_COPY(paramValue));
				if(AccountIndex > 2)
					json_object_object_add(obj, "Privilege", json_object_new_string("samba"));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "Username")) {
				IID_INIT(tempIid);
				while(zcfgFeObjJsonGetNext(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &tempIid, &tempobj) == ZCFG_SUCCESS){
				   if((Iid.idx[0] == tempIid.idx[0]) && (Iid.idx[1] == tempIid.idx[1])){
                          //do not compile with ourself
				   	}
				   else{
					if(!strcmp(json_object_get_string(paramValue),json_object_get_string(json_object_object_get(tempobj, "Username")))){
                        zcfgFeJsonObjFree(tempobj);
							if(multiJobj){
		                        json_object_put(tmpObj);
	                        }
							else{
                               if(obj){
							   	json_object_put(obj);
                               }
							}
						return ZCFG_INVALID_PARAM_VALUE;
					}
				   }
					zcfgFeJsonObjFree(tempobj);
				}

				json_object_object_add(obj, "Username", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "Password")) {
				json_object_object_add(obj, "Password", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
        if(obj){
			if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &Iid, obj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set RDM_OID_ZY_LOG_CFG_GP_ACCOUNT Obj Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set RDM_OID_ZY_LOG_CFG_GP_ACCOUNT Obj Success\n", __FUNCTION__);
			}
			json_object_put(obj);
		}
	}
  
	return ZCFG_SUCCESS;
}

zcfgRet_t SSUserAccountObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t loginCfgGpAccoutIid;
	int SSIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.UserAccount", &SSIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

	IID_INIT(loginCfgGpAccoutIid);
	loginCfgGpAccoutIid.level = 1;
	loginCfgGpAccoutIid.idx[0] = 3;

	if((ret = zcfgFeObjStructAdd(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginCfgGpAccoutIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add LoginCfg.LogGp.%hhu.Account. Fail.\n", __FUNCTION__, loginCfgGpAccoutIid.idx[0]);
		return ret;
	}

	*idx = loginCfgGpAccoutIid.idx[1]+2;

	return ZCFG_SUCCESS;
}

zcfgRet_t SSUserAccountObjDel(char *tr98FullPathName)
{
        objIndex_t objIid;
		int SSIndex = 0;
		int AccountIndex = 0;
		
		printf("%s : Enter\n", __FUNCTION__);
		
		sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.UserAccount.%d", &SSIndex, &AccountIndex);
		if(SSIndex != 1)
			return ZCFG_NO_SUCH_OBJECT;
		
		if(AccountIndex < 3)
			return ZCFG_DELETE_REJECT; 

        IID_INIT(objIid);
        objIid.level = 2;
		objIid.idx[0] = 3;
		objIid.idx[1] = AccountIndex - 2;
		
        return zcfgFeObjStructDel(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &objIid, NULL);
}

/* InternetGatewayDevice.Services.StorageService.i.Capabilites */
zcfgRet_t SSCapabilitesObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	int SSIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.Capabilites", &SSIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "FTPCapable")) {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
						paramList++;
						continue;
		}
		else if(!strcmp(paramList->name, "SFTPCapable")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "HTTPCapable")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "HTTPSCapable")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "HTTPWritable")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "VolumeEncryptionCapable")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "SupportedNetworkProtocols")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
					paramList++;
					continue;
		}
        else if(!strcmp(paramList->name, "SupportedFileSystemTypes")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
					paramList++;
					continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR98 List\n", paramList->name);
		paramList++;
	}

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.Services.StorageService.i.NetInfo */
zcfgRet_t SSNetInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	int SSIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.NetInfo", &SSIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "HostName")) {
							json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
							paramList++;
							continue;
		}
		else if(!strcmp(paramList->name, "DomainName")) {
							json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
							paramList++;
							continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR98\n", paramList->name);
		paramList++;
	}

	return ZCFG_SUCCESS;
}
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
/* InternetGatewayDevice.Services.StorageService.i.NetworkServer */
zcfgRet_t SSNetworkServerObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *SambaObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	int SSIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.NetworkServer", &SSIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_SAMBA, &objIid, &SambaObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "AFPEnable")) {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
						paramList++;
						continue;
		}
		else if(!strcmp(paramList->name, "NFSEnable")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "SMBEnable")) {
			        paramValue = json_object_object_get(SambaObj, "Enable");
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "NetworkProtocolAuthReq")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "SambaHostName")) {
			        paramValue = json_object_object_get(SambaObj, "SambaHostName");
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
		}
		
		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR98\n", paramList->name);
		paramList++;
	}

	json_object_put(SambaObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t SSNetworkServerObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *SambaObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	int SSIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.NetworkServer", &SSIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_SAMBA, &objIid, &SambaObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = SambaObj;
		SambaObj = NULL;
		SambaObj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_SAMBA, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			            /*special case*/
						if(!strcmp(paramList->name, "SMBEnable")) {
							json_object_object_add(SambaObj, "Enable", JSON_OBJ_COPY(paramValue));
							paramList++;
							continue;
						}
						else if(!strcmp(paramList->name, "SambaHostName")) {
							json_object_object_add(SambaObj, "SambaHostName", JSON_OBJ_COPY(paramValue));
							paramList++;
							continue;
						}
		}
		printf("Can't find parameter %s in TR98\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_SAMBA, &objIid, SambaObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(SambaObj);
			return ret;
		}
		json_object_put(SambaObj);
	}
		  
	return ZCFG_SUCCESS;
}
#endif
/* InternetGatewayDevice.Services.StorageService.i.FTPServer */
zcfgRet_t SSFTPServerObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ftpobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	int SSIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.FTPServer", &SSIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 3;
	if((ret = feObjJsonGet(RDM_OID_REMO_SRV, &objIid, &ftpobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "Enable") || !strcmp(paramList->name, "Status")) {
			paramValue = json_object_object_get(ftpobj, "Mode");
			if(paramValue != NULL){
				const char *ftpMode = NULL;
				ftpMode = json_object_get_string(paramValue); 
				if(strlen(ftpMode) > 0 )
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(1));
				else
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(0));
			paramList++;
			continue;
			}
		}
		else if(!strcmp(paramList->name, "MaxNumUsers")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "IdleTime")) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
                    json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(0));
#else
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
#endif
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "PortNumber")) {
			        paramValue = json_object_object_get(ftpobj, "Port");
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
		}
		
		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR98\n", paramList->name);
		paramList++;
	}

	json_object_put(ftpobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t SSFTPServerObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ftpobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	int SSIndex = 0;
	int ftpMode = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.FTPServer", &SSIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 3;
	if((ret = zcfgFeObjJsonGet(RDM_OID_REMO_SRV, &objIid, &ftpobj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = ftpobj;
		ftpobj = NULL;
		ftpobj = zcfgFeJsonMultiObjAppend(RDM_OID_REMO_SRV, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name, "Enable")) {
				ftpMode= json_object_get_boolean(paramValue);
					if(ftpMode == 0)
						json_object_object_add(ftpobj, "Mode", json_object_new_string(""));
					else if(ftpMode == 1)
						json_object_object_add(ftpobj, "Mode", json_object_new_string("LAN_WAN"));
					paramList++;
					continue;
			}
			else if(!strcmp(paramList->name, "PortNumber")) {
				json_object_object_add(ftpobj, "Port", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR98\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_REMO_SRV, &objIid, ftpobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ftpobj);
			return ret;
		}
		json_object_put(ftpobj);
	}
		  
	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.Services.StorageService.i.PhysicalMedium.i */
zcfgRet_t SSPhysicalMediumObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	objIndex_t Iid = {0};
	objIndex_t PhyIid = {0};
	objIndex_t LocIid = {0};
	struct json_object *obj = NULL;
	struct json_object *Phyobj = NULL;
	struct json_object *Locobj = NULL;
    int capacity = 0; 
	char pchstr[64] = {0};
	char * pch;
	char phyname[64] = {0};
	char connectiontype[32] = {0};
	int SSIndex = 0,PhyIndex = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.PhysicalMedium.%d", &SSIndex, &PhyIndex);
	if(SSIndex != 1 || PhyIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

    IID_INIT(Iid);
	//trigger usb backend get usb capacity and usedspace
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV, &Iid, &obj) == ZCFG_SUCCESS){
		zcfgFeJsonObjFree(obj);
	}

		IID_INIT(PhyIid);
		PhyIid.level=2;
		PhyIid.idx[0]=1;
		PhyIid.idx[1]=2;
		if(zcfgFeObjJsonGet(RDM_OID_USB_USBHOSTS_HOST_DEV, &PhyIid, &Phyobj) != ZCFG_SUCCESS){
               return ZCFG_NO_SUCH_OBJECT;
		}
		

    IID_INIT(LocIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV_PAR, &LocIid, &Locobj) == ZCFG_SUCCESS){
		if(strlen(phyname) == 0){
		      strcpy(pchstr,json_object_get_string(json_object_object_get(Locobj, "PartitionName")));
	   	      pch = strtok(pchstr,"_");	
			  strcpy(phyname,pch);
		}
        capacity = capacity + json_object_get_int(json_object_object_get(Locobj, "Capacity"));
		zcfgFeJsonObjFree(Locobj);
	}	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "Name")) {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(phyname));
						paramList++;
						continue;
		}
		else if(!strcmp(paramList->name, "ConnectionType")) {
			        sprintf(connectiontype,"USB%s",json_object_get_string(json_object_object_get(Phyobj, "USBVersion")));
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(connectiontype));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "Removable")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "Capacity")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(capacity));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "Status")) {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Standby"));
						paramList++;
						continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR98 List\n", paramList->name);
		paramList++;
	}

	json_object_put(Phyobj);

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.Services.StorageService.i.LogicalVolume.i */
zcfgRet_t SSLogicalVolumeObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	objIndex_t Iid = {0};
	objIndex_t LocIid = {0};
	struct json_object *obj = NULL;
	struct json_object *Locobj = NULL;
	struct json_object *paramValue = NULL;
	int SSIndex = 0,LogicalIndex = 0;
	int FolderNum = 0;
	int LogNum = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.LogicalVolume.%d", &SSIndex,&LogicalIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

	IID_INIT(Iid);
	//trigger usb backend get usb capacity and usedspace
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV, &Iid, &obj) == ZCFG_SUCCESS){
		zcfgFeJsonObjFree(obj);
	}
		
	IID_INIT(LocIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV_PAR, &LocIid, &Locobj) == ZCFG_SUCCESS){
		LogNum++;
		if(LogicalIndex == LogNum)
			break;
		else
			zcfgFeJsonObjFree(Locobj);
	}

	if(!Locobj)
		return ZCFG_NO_SUCH_OBJECT;

#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
    IID_INIT(Iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_SAMBA_DIR, &Iid, &obj) == ZCFG_SUCCESS){
        if(!strcmp(json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Hostname")),json_object_get_string(json_object_object_get(Locobj, "PartitionName"))))
			FolderNum++;
		zcfgFeJsonObjFree(obj);
	}
#endif
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "Name")) {
			            paramValue = json_object_object_get(Locobj, "PartitionName");
						json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
						paramList++;
						continue;
		}
		else if(!strcmp(paramList->name, "PhysicalReference")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("InternetGatewayDevice.Services.StorageService.1.PhysicalMedium.1"));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "Enable")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "FileSystem")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "Status")) {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Online"));
						paramList++;
						continue;
		}
		else if(!strcmp(paramList->name, "Capacity")) {
			        paramValue = json_object_object_get(Locobj, "Capacity");
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "UsedSpace")) {
			        paramValue = json_object_object_get(Locobj, "UsedSpace");
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "Encrypted")) {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
						paramList++;
						continue;
		}
		else if(!strcmp(paramList->name, "FolderNumberOfEntries")) {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(FolderNum));
						paramList++;
						continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR98 List\n", paramList->name);
		paramList++;
	}

	json_object_put(Locobj);

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.Services.StorageService.i.LogicalVolume.i.Folder.i */
zcfgRet_t SSFolderObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	objIndex_t Iid = {0};
	objIndex_t ForIid = {0};
	objIndex_t LocIid = {0};
	struct json_object *obj = NULL;
	struct json_object *Forobj = NULL;
	struct json_object *Locobj = NULL;
	struct json_object *paramValue = NULL;
	int SSIndex = 0,LogicalIndex = 0,FolderIndex = 0;
	int FolderNum = 0;
	int LogNum = 0;

	printf("%s : Enter\n", __FUNCTION__);

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.StorageService.%d.LogicalVolume.%d.Folder.%d", &SSIndex,&LogicalIndex,&FolderIndex);
	if(SSIndex != 1)
		return ZCFG_NO_SUCH_OBJECT;

    IID_INIT(Iid);
	//trigger usb backend get usb capacity and usedspace
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV, &Iid, &obj) == ZCFG_SUCCESS){
		zcfgFeJsonObjFree(obj);
	}

	IID_INIT(LocIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV_PAR, &LocIid, &Locobj) == ZCFG_SUCCESS){
		LogNum++;
		if(LogicalIndex == LogNum)
			break;
		else
			zcfgFeJsonObjFree(Locobj);
	}

	if(!Locobj)
		return ZCFG_NO_SUCH_OBJECT;

#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
    IID_INIT(ForIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_SAMBA_DIR, &ForIid, &Forobj) == ZCFG_SUCCESS){
        if(!strcmp(json_object_get_string(json_object_object_get(Forobj, "X_ZYXEL_Hostname")),json_object_get_string(json_object_object_get(Locobj, "PartitionName"))))
			FolderNum++;
		if(FolderIndex == FolderNum)
			break;
		else
		    zcfgFeJsonObjFree(Forobj);
	}
#endif
	if(!Forobj){
		json_object_put(Locobj);
		return ZCFG_NO_SUCH_OBJECT;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "Name")) {
			            paramValue = json_object_object_get(Forobj, "X_ZYXEL_Path");
						json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
						paramList++;
						continue;
		}
		else if(!strcmp(paramList->name, "Enable")) {
			        paramValue = json_object_object_get(Forobj, "X_ZYXEL_Browseable");
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
		}
		else if(!strcmp(paramList->name, "UserAccountAccess")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(2));
					paramList++;
					continue;
		}
		
		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR98 List\n", paramList->name);
		paramList++;
	}

	json_object_put(Locobj);

	json_object_put(Forobj);

	return ZCFG_SUCCESS;
}
#endif

#ifdef ZYXEL_IGMPDIAG 
/* InternetGatewayDevice.X_TTNET.Function.IGMPDiagnostics */
zcfgRet_t zyTTIGMPDiagnosticsObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *IGMPDiagnosticsObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char getvalue[1024] = {0};
	char cpy_monitorInterface[512] = {0};
	char *token = NULL;
	char singleintf[128] = {0};
	char gethistory[1024]= {0};
    char devIpIface[48] = {0};
	char iface[40] = {0};
	char lowerLayer[32] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_I_G_M_P_DIAGNOSTICS, &objIid, &IGMPDiagnosticsObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(IGMPDiagnosticsObj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name, "Interface")) {
			paramValue = json_object_object_get(IGMPDiagnosticsObj, paramList->name);
			strcpy(devIpIface, json_object_get_string(paramValue));
			if(!strcmp(devIpIface, "") || !strcmp(devIpIface, "(null)")) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
					}
			else {
				strcpy(iface, devIpIface);
				if((zcfgFeTr181IfaceStackLowerLayerGet(iface, lowerLayer)) != ZCFG_SUCCESS) {
					json_object_put(IGMPDiagnosticsObj);
					return ZCFG_NO_SUCH_OBJECT;
					}
				if(strstr(lowerLayer, "PPP.Interface") != NULL) {
					memset(iface, 0, sizeof(iface));
					strcpy(iface, lowerLayer);
					}
				if((zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName)) != ZCFG_SUCCESS) {
					json_object_put(IGMPDiagnosticsObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				strcpy(tr98ObjName, tmpTr98ObjName);
				ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName);
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));
				}
				paramList++;
				continue;
			}
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(IGMPDiagnosticsObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTIGMPDiagnosticsObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *IGMPDiagnosticsObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	const char *setvalue = NULL;
	char setIfaceList[1024] = {0};
	char devIpIface[300] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_I_G_M_P_DIAGNOSTICS, &objIid, &IGMPDiagnosticsObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = IGMPDiagnosticsObj;
		IGMPDiagnosticsObj = NULL;
		IGMPDiagnosticsObj = zcfgFeJsonMultiObjAppend(RDM_OID_I_G_M_P_DIAGNOSTICS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(IGMPDiagnosticsObj, paramList->name);
			if(tr181ParamValue != NULL){
				if(!strcmp(paramList->name,"Interface")){
			sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
			if(paramValue != NULL && json_object_get_string(paramValue)) {
				strcpy(tmpTr98ConnObjName, json_object_get_string(paramValue));
				strcpy(tr98ConnObjName, tmpTr98ConnObjName);
				ReplaceWANDeviceIndex(tmpTr98ConnObjName, tr98ConnObjName);
				if(zcfgFe98To181IpIface(tr98ConnObjName, devIpIface) == ZCFG_SUCCESS){
					json_object_object_add(IGMPDiagnosticsObj, paramList->name, json_object_new_string(devIpIface));
				}else{
					if(!multiJobj)
						json_object_put(IGMPDiagnosticsObj);

					sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
					return ZCFG_INVALID_PARAM_VALUE;
					}
					}
			strcpy(paramfault, "");
					paramList++;
					continue;
				}	
				json_object_object_add(IGMPDiagnosticsObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_I_G_M_P_DIAGNOSTICS, &objIid, IGMPDiagnosticsObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(IGMPDiagnosticsObj);
			return ret;
		}
		json_object_put(IGMPDiagnosticsObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTIGMPDiagnosticsObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTIGMPDiagnosticsObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  IGMPDiagnosticsOid = 0;
	objIndex_t IGMPDiagnosticsIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(IGMPDiagnosticsIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	IGMPDiagnosticsOid = zcfgFeObjNameToObjId(tr181Obj, &IGMPDiagnosticsIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(IGMPDiagnosticsOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTIGMPDiagnosticsObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  IGMPDiagnosticsOid = 0;
	objIndex_t IGMPDiagnosticsIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(IGMPDiagnosticsIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	IGMPDiagnosticsOid = zcfgFeObjNameToObjId(tr181Obj, &IGMPDiagnosticsIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(IGMPDiagnosticsOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(IGMPDiagnosticsOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, IGMPDiagnosticsOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif

#ifdef ZYXEL_TAAAB_WAN_CONN_DIAG 
/* InternetGatewayDevice.X_TTNET.Function.Connection */
zcfgRet_t zyTTWanConnDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *wanConnDiagObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_TAAAG_CONN_DIAG, &objIid, &wanConnDiagObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(wanConnDiagObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(wanConnDiagObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTWanConnDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *wanConnDiagObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_TAAAG_CONN_DIAG, &objIid, &wanConnDiagObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = wanConnDiagObj;
		wanConnDiagObj = NULL;
		wanConnDiagObj = zcfgFeJsonMultiObjAppend(RDM_OID_TAAAG_CONN_DIAG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(wanConnDiagObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(wanConnDiagObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_TAAAG_CONN_DIAG, &objIid, wanConnDiagObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(wanConnDiagObj);
			return ret;
		}
		json_object_put(wanConnDiagObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTWanConnDiagObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTWanConnDiagObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t wanConnDiagOid = 0;
	objIndex_t wanConnDiagIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(wanConnDiagIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	wanConnDiagOid = zcfgFeObjNameToObjId(tr181Obj, &wanConnDiagIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(wanConnDiagOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTWanConnDiagObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  wanConnDiagOid = 0;
	objIndex_t wanConnDiagIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(wanConnDiagIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	wanConnDiagOid = zcfgFeObjNameToObjId(tr181Obj, &wanConnDiagIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(wanConnDiagOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(wanConnDiagOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, wanConnDiagOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif

#ifdef ZYXEL_CUSTOMIZATION_SYSLOG
/* InternetGatewayDevice.X_TT.Configuration.Syslog */
zcfgRet_t zyTTSyslogObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *SyslogObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_SYSLOG, &objIid, &SyslogObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(SyslogObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(SyslogObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTSyslogObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *SyslogObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_SYSLOG, &objIid, &SyslogObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = SyslogObj;
		SyslogObj = NULL;
		SyslogObj = zcfgFeJsonMultiObjAppend(RDM_OID_SYSLOG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(SyslogObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(SyslogObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_SYSLOG, &objIid, SyslogObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(SyslogObj);
			return ret;
		}
		json_object_put(SyslogObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTSyslogObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTSyslogObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  SyslogOid = 0;
	objIndex_t SyslogIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(SyslogIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	SyslogOid = zcfgFeObjNameToObjId(tr181Obj, &SyslogIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(SyslogOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTSyslogObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  SyslogOid = 0;
	objIndex_t SyslogIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(SyslogIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	SyslogOid = zcfgFeObjNameToObjId(tr181Obj, &SyslogIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(SyslogOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(SyslogOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, SyslogOid, paramList->name);
		}
		break;
	}
	
	return ret;
}

/* InternetGatewayDevice.X_TT.Function.SyslogUpload */
zcfgRet_t zyTTSyslogUploadObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *SyslogUploadObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_SYSLOG_UPLOAD, &objIid, &SyslogUploadObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(SyslogUploadObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(SyslogUploadObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTSyslogUploadObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *SyslogUploadObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_SYSLOG_UPLOAD, &objIid, &SyslogUploadObj)) != ZCFG_SUCCESS){
		return ret;
	}
    
	if(multiJobj){
		tmpObj = SyslogUploadObj;
		SyslogUploadObj = NULL;
		SyslogUploadObj = zcfgFeJsonMultiObjAppend(RDM_OID_SYSLOG_UPLOAD, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(SyslogUploadObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(SyslogUploadObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_SYSLOG, &objIid, SyslogUploadObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(SyslogUploadObj);
			return ret;
		}
		json_object_put(SyslogUploadObj);
	}
		  
	return ZCFG_SUCCESS;
}

zcfgRet_t zyTTSyslogUploadObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
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

int zyTTSyslogUploadObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  SyslogUploadOid = 0;
	objIndex_t SyslogUploadIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(SyslogUploadIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	SyslogUploadOid = zcfgFeObjNameToObjId(tr181Obj, &SyslogUploadIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(SyslogUploadOid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyTTSyslogUploadObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  SyslogUploadOid = 0;
	objIndex_t SyslogUploadIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(SyslogUploadIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	SyslogUploadOid = zcfgFeObjNameToObjId(tr181Obj, &SyslogUploadIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(SyslogUploadOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(SyslogUploadOid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, SyslogUploadOid, paramList->name);
		}
		break;
	}
	
	return ret;
}
#endif

#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
/* InternetGatewayDevice.X_ZYXEL_EXT.MultipleConfig */
zcfgRet_t zyExtMultipleConfigObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *MultipleConfigObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_MULTIPLE_CONFIG, &objIid, &MultipleConfigObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(MultipleConfigObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(MultipleConfigObj);

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.X_ZYXEL_EXT.MultipleConfig */
zcfgRet_t zyExtMultipleConfigObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *MultipleConfigObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_MULTIPLE_CONFIG, &objIid, &MultipleConfigObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = MultipleConfigObj;
		MultipleConfigObj = NULL;
		MultipleConfigObj = zcfgFeJsonMultiObjAppend(RDM_OID_MULTIPLE_CONFIG, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(MultipleConfigObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(MultipleConfigObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MULTIPLE_CONFIG, &objIid, MultipleConfigObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(MultipleConfigObj);
			return ret;
		}
		json_object_put(MultipleConfigObj);
	}

	return ZCFG_SUCCESS;
}
#endif

/*InternetGatewayDevice.X_ZYXEL_IPV4RoutingTable.i*/
zcfgRet_t WANV4RoutingTableObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *routingTableObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_IPV4RoutingTable.%hhu", &objIid.idx[0]);
	if((ret = feObjJsonGet(RDM_OID_V4_ROUTING_TABLE, &objIid, &routingTableObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/

		paramValue = json_object_object_get(routingTableObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(routingTableObj);

	return ZCFG_SUCCESS;
}

/*InternetGatewayDevice.X_ZYXEL_IPV6RoutingTable.i*/
zcfgRet_t WANV6RoutingTableObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *routingTableObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_IPV6RoutingTable.%hhu", &objIid.idx[0]);
	if((ret = feObjJsonGet(RDM_OID_V6_ROUTING_TABLE, &objIid, &routingTableObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/

		paramValue = json_object_object_get(routingTableObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(routingTableObj);

	return ZCFG_SUCCESS;
}

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_MultiWan
 *
 *   Related object in TR181: Device.X_ZYXEL_MultiWan

 *
 */
zcfgRet_t zyExtMultiWanObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *multiWanJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_MULTI_WAN, &objIid, &multiWanJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(multiWanJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(multiWanJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMultiWanObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *multiWanJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_MULTI_WAN, &objIid, &multiWanJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = multiWanJobj;
		multiWanJobj = NULL;
		multiWanJobj = zcfgFeJsonMultiObjAppend(RDM_OID_MULTI_WAN, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(multiWanJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(multiWanJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MULTI_WAN, &objIid, multiWanJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(multiWanJobj);
			return ret;
		}
		json_object_put(multiWanJobj);
	}

	return ZCFG_SUCCESS;
}


/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_MultiWan.Interface.i
 *
 *   Related object in TR181: Device.X_ZYXEL_MultiWan.Interface.i
 *
 */
zcfgRet_t zyExtMultiWanIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *multiWanIntfJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_MultiWan.Interface.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_MULTI_WAN_IFACE, &objIid, &multiWanIntfJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(multiWanIntfJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(multiWanIntfJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMultiWanIntfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *multiWanIntfJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_MultiWan.Interface.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_MULTI_WAN_IFACE, &objIid, &multiWanIntfJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = multiWanIntfJobj;
		multiWanIntfJobj = NULL;
		multiWanIntfJobj = zcfgFeJsonMultiObjAppend(RDM_OID_MULTI_WAN_IFACE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(multiWanIntfJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(multiWanIntfJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MULTI_WAN_IFACE, &objIid, multiWanIntfJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(multiWanIntfJobj);
			return ret;
		}
		json_object_put(multiWanIntfJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtMultiWanIntfObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_MULTI_WAN_IFACE, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyExtMultiWanIntfObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_MultiWan.Interface.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_MULTI_WAN_IFACE, &objIid, NULL);;
}

#endif // CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN
/* "InternetGatewayDevice.X_ZYXEL_*"  - for those 181 and 098 which use the same data model*/
static zcfgRet_t zyExtCommonObjGetFromRDM(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag, objIndex_t *p_objIid, int rdm_oid)
{
	struct json_object *refObj = NULL, *paramObj;
	zcfgRet_t ret = ZCFG_NO_SUCH_OBJECT;
	printf("==>[Get] rdm_oid = %d; handler=%d [%s]\n", rdm_oid, handler, tr98FullPathName);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(rdm_oid, p_objIid, &refObj)) != ZCFG_SUCCESS) {
		printf("%s: retrieve object fail (rdm_oid = %d): ret = %d\n", __FUNCTION__, rdm_oid, ret);
		zcfgFeJsonObjFree(refObj);
		return ZCFG_NO_SUCH_OBJECT;
	}

	/*fill up tr98 object from related tr181 objects*/
	struct tr98Parameter_s *paramList = tr98Obj[handler].parameter;
	if(!paramList){
		zcfgFeJsonObjFree(refObj);
		return ZCFG_NO_SUCH_OBJECT;
	}

	*tr98Jobj = json_object_new_object();
	while(paramList->name) {
		/*Write parameter (defined in paramList) value from tr181 objects to tr98 object*/
		paramObj = json_object_object_get(refObj, paramList->name);
		if(paramObj){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramObj));
		}
		else {
			printf("Can't find parameter %s in TR181\n", paramList->name);
		}
		paramList++;
	}
	zcfgFeJsonObjFree(refObj);
	return ZCFG_SUCCESS;
}

static zcfgRet_t zyExtCommonObjSetToRDM(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, objIndex_t *p_objIid, int rdm_oid)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *targetJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("==>[Set] rdm_oid = %d; handler=%d [%s]\n", rdm_oid, handler, tr98FullPathName);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(rdm_oid, p_objIid, &targetJobj)) != ZCFG_SUCCESS) {
		printf("%s: retrieve object fail (rdm_oid = %d): ret = %d\n", __FUNCTION__, rdm_oid, ret);
		zcfgFeJsonObjFree(targetJobj);
		return ZCFG_NO_SUCH_OBJECT;
	}

	if(multiJobj){
		if(targetJobj) {
			tmpObj = targetJobj;
			targetJobj = NULL;
			targetJobj = zcfgFeJsonMultiObjAppend(rdm_oid, p_objIid, multiJobj, tmpObj);
		}
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(targetJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(targetJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	if(multiJobj) {
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(rdm_oid, p_objIid, targetJobj, NULL)) != ZCFG_SUCCESS ){
			zcfgLog(ZCFG_LOG_ERR, "%s : Set oid(%d) Obj Fail\n", __FUNCTION__, rdm_oid);
			json_object_put(targetJobj);
			return ret;
		}
		zcfgLog(ZCFG_LOG_DEBUG, "%s : Set oid(%d) Obj Success\n", __FUNCTION__, rdm_oid);
		json_object_put(targetJobj);
	}

	return ZCFG_SUCCESS;
}

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
/* "InternetGatewayDevice.X_ZYXEL_Cellular" */
zcfgRet_t zyExtCellObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	objIndex_t objIid;
	printf("==> %s : Enter\n", __FUNCTION__);
	IID_INIT(objIid);
	return zyExtCommonObjGetFromRDM(tr98FullPathName, handler, tr98Jobj, updateFlag, &objIid, RDM_OID_CELL);
}

/* "InternetGatewayDevice.X_ZYXEL_Cellular.Interface.i" */
zcfgRet_t zyExtCellIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	objIndex_t objIid;
	printf("==> %s : Enter\n", __FUNCTION__);
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_Cellular.Interface.%hhu", &objIid.idx[0]);
	return zyExtCommonObjGetFromRDM(tr98FullPathName, handler, tr98Jobj, updateFlag, &objIid, RDM_OID_CELL_INTF);
}
zcfgRet_t zyExtCellIntfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj)
{
	objIndex_t objIid;
	printf("==> %s : Enter\n", __FUNCTION__);
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_Cellular.Interface.%hhu", &objIid.idx[0]);
	return zyExtCommonObjSetToRDM(tr98FullPathName, handler, tr98Jobj, multiJobj, &objIid, RDM_OID_CELL_INTF);
}

/* "InternetGatewayDevice.X_ZYXEL_Cellular.Interface.i.USIM" */
zcfgRet_t zyExtCellIntfUSIMObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	objIndex_t objIid;
	printf("==> %s : Enter\n", __FUNCTION__);
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_Cellular.Interface.%hhu", &objIid.idx[0]);
	return zyExtCommonObjGetFromRDM(tr98FullPathName, handler, tr98Jobj, updateFlag, &objIid, RDM_OID_CELL_INTF_USIM);
}
zcfgRet_t zyExtCellIntfUSIMObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj)
{
	objIndex_t objIid;
	printf("==> %s : Enter\n", __FUNCTION__);
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_Cellular.Interface.%hhu", &objIid.idx[0]);
	return zyExtCommonObjSetToRDM(tr98FullPathName, handler, tr98Jobj, multiJobj, &objIid, RDM_OID_CELL_INTF_USIM);
}

/* "InternetGatewayDevice.X_ZYXEL_Cellular.AccessPoint.i" */
zcfgRet_t zyExtCellAccessPointObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	objIndex_t objIid;
	printf("==> %s : Enter\n", __FUNCTION__);
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_Cellular.AccessPoint.%hhu", &objIid.idx[0]);
	return zyExtCommonObjGetFromRDM(tr98FullPathName, handler, tr98Jobj, updateFlag, &objIid, RDM_OID_CELL_ACCESS_POINT);
}
zcfgRet_t zyExtCellAccessPointObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj)
{
	objIndex_t objIid;
	printf("==> %s : Enter\n", __FUNCTION__);
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_Cellular.AccessPoint.%hhu", &objIid.idx[0]);
	return zyExtCommonObjSetToRDM(tr98FullPathName, handler, tr98Jobj, multiJobj, &objIid, RDM_OID_CELL_ACCESS_POINT);
}

#endif // CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN

#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
/* InternetGatewayDevice.Services.X_ZYXEL_Package.i */
zcfgRet_t zyExtIpkPackageGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipkObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);

	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.X_ZYXEL_Package.%hhu", &objIid.idx[objIid.level - 1]);

	if((ret = feObjJsonGet(RDM_OID_PACKAGE, &objIid, &ipkObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/

		paramValue = json_object_object_get(ipkObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(ipkObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtIpkPackageSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipkObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);

	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.X_ZYXEL_Package.%hhu", &objIid.idx[objIid.level - 1]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_PACKAGE, &objIid, &ipkObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = ipkObj;
		ipkObj = NULL;
		ipkObj = zcfgFeJsonMultiObjAppend(RDM_OID_PACKAGE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);

		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ipkObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ipkObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_PACKAGE, &objIid, ipkObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ipkObj);
			return ret;
		}
		json_object_put(ipkObj);
	}
	return ZCFG_SUCCESS;
}
/* InternetGatewayDevice.Services.X_ZYXEL_Package_Install */
zcfgRet_t zyExtIpkPackageInstallGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipkObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);

	//objIid.level = 1;
	//sscanf(tr98FullPathName, "InternetGatewayDevice.Services.X_ZYXEL_Package.%hhu", &objIid.idx[objIid.level - 1]);

	if((ret = feObjJsonGet(RDM_OID_PACKAGE_INSTALL, &objIid, &ipkObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/

		paramValue = json_object_object_get(ipkObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(ipkObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtIpkPackageInstallSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipkObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);

	//objIid.level = 1;
	//sscanf(tr98FullPathName, "InternetGatewayDevice.Services.X_ZYXEL_Package.%hhu", &objIid.idx[objIid.level - 1]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_PACKAGE_INSTALL, &objIid, &ipkObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = ipkObj;
		ipkObj = NULL;
		ipkObj = zcfgFeJsonMultiObjAppend(RDM_OID_PACKAGE_INSTALL, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);

		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ipkObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ipkObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_PACKAGE_INSTALL, &objIid, ipkObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ipkObj);
			return ret;
		}
		json_object_put(ipkObj);
	}
	return ZCFG_SUCCESS;
}
#endif // CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE 
