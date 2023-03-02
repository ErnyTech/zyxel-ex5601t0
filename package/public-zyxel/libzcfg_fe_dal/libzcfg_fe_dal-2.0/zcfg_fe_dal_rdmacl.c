#include <json/json.h>
#include <sys/types.h>


#include "zcfg_common.h"
#include "zcfg_ret.h"
#include "zcfg_rdm_oid.h"

#include "zcmd_schema.h"
#include "zcfg_fe_schema.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zos_api.h"

#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_dal_rdmacl.h"

enum {
	ZHTTP_RDM_ACL_METHOD_QUERY = 1
};

const char *rdmACLIpIfaceParams[] = {"Name", "X_ZYXEL_SrvName", NULL};
const char *rdmACLDevSecCertParams[] = {"X_ZYXEL_Name", "X_ZYXEL_Type", NULL};
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
const char *rdmACLVlanGroupParams[] = {"GroupName", "VlanId", NULL};
#endif
const char *rdmACLBridgingBrParams[] = {"X_ZYXEL_BridgeName", NULL};
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
const char *rdmACLBridgingFltParams[] = {"Bridge", "SourceMACAddressFilterList", "SourceMACFromVendorClassIDFilter", "SourceMACFromVendorClassIDMode", "SourceMACFromClientIDFilter", "X_ZYXEL_SourceMACFromVSIFilter", "X_ZYXEL_APASSourceMACAFilter", NULL};
#else
const char *rdmACLBridgingFltParams[] = {"Bridge", "SourceMACAddressFilterList", "SourceMACFromVendorClassIDFilter", "SourceMACFromVendorClassIDMode", "SourceMACFromClientIDFilter", "X_ZYXEL_SourceMACFromVSIFilter", NULL};
#endif
const char *rdmACLClassifyParams[] = {"Name", NULL};
const char *rdmACLCategoryParams[] = {"Name", NULL};
const char *rdmACLMacAddressTableParams[] = {"Interface", "Mode", "Type", "AddrList", NULL};

//char *rdmACLObjectIid[] = {"1,2", "2"}; //[1,2,0,0,0,0], [2,0,0,0,0,0]
const char *rdmACLBridgingBrIid[] = {"1", NULL};

static rdmObjectACL_t rdmObjectACList[] = {
	{RDM_OID_IP_IFACE, ZHTTP_RDM_ACL_METHOD_QUERY, NULL, rdmACLIpIfaceParams},
	{RDM_OID_DEV_SEC_CERT, ZHTTP_RDM_ACL_METHOD_QUERY, NULL, rdmACLDevSecCertParams},
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	{RDM_OID_VLAN_GROUP, ZHTTP_RDM_ACL_METHOD_QUERY, NULL, rdmACLVlanGroupParams},
#endif
	{RDM_OID_BRIDGING_BR, ZHTTP_RDM_ACL_METHOD_QUERY, NULL, rdmACLBridgingBrParams},
	{RDM_OID_BRIDGING_FILTER, ZHTTP_RDM_ACL_METHOD_QUERY, NULL, rdmACLBridgingFltParams},
	{RDM_OID_LOG_CATEGORY, ZHTTP_RDM_ACL_METHOD_QUERY, NULL, rdmACLCategoryParams},
	{RDM_OID_LOG_CLASSIFY, ZHTTP_RDM_ACL_METHOD_QUERY, NULL, rdmACLClassifyParams},
	{RDM_OID_MAC_ADDRESS_TABLE, ZHTTP_RDM_ACL_METHOD_QUERY, NULL, rdmACLMacAddressTableParams},
	{0, 0, NULL, NULL}
};

static int zcfgFeDalACLIidAuth(const char *objIidAuth[], char *iid)
{
	int n = 0, rst = -1;
	char iidStr[30+1] = {0};
	bool running = true;
	char *p;

	//!iid, !strlen(iid)
	ZOS_STRNCPY(iidStr, iid, sizeof(iidStr));
	p = strstr(iidStr, "0");
	if(p && p != &iidStr[0]){
		*(p-1) = '\0'; //n,0
	}

	while(objIidAuth[n] && strlen(iidStr) && running){
		rst = strcmp(iidStr, objIidAuth[n++]);
		running = (rst == 0) ? false : true;
	}

	return rst;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
static zcfgRet_t zcfgFeDalQueryRdmObj(rdmObjectACL_t *objAcl, queryRdmSubObjRout querySubObjRout, queryRdmObjRout queryObjNextRout, queryRdmObjRout queryObjRout,
			const char *oidStr, zcfg_offset_t objOid, objIndex_t *objIid, objIndex_t *childIid, struct json_object *response, const char *fileName, int line)
#else
static zcfgRet_t zcfgFeDalQueryRdmObj(rdmObjectACL_t *objAcl, queryRdmSubObjRout querySubObjRout, queryRdmObjRout queryObjNextRout, queryRdmObjRout queryObjRout,
			const char *oidStr, zcfg_offset_t objOid, objIndex_t *objIid, objIndex_t *childIid, struct json_object *response)
#endif
{
	objIndex_t *outIid;
	struct json_object *queriedObj, *value;
	zcfgRet_t ret;

	// !oidStr, !objIid, !response
	// !querySubObjRout && !queryObjNextRout && !queryObjRout
	if(querySubObjRout){ // query sub object
		if(!childIid)
			return ZCFG_INTERNAL_ERROR;

#ifdef ZCFG_MEMORY_LEAK_DETECT
		ret = querySubObjRout(objOid, objIid, childIid, &queriedObj, fileName, line);
#else
		ret = querySubObjRout(objOid, objIid, childIid, &queriedObj);
#endif
		outIid = childIid;
	}else if(queryObjNextRout){  // query next
		if(!objIid)
			return ZCFG_INTERNAL_ERROR;
		
#ifdef ZCFG_MEMORY_LEAK_DETECT
		ret = queryObjNextRout(objOid, objIid, &queriedObj, fileName, line);
#else
		ret = queryObjNextRout(objOid, objIid, &queriedObj);
#endif
		outIid = objIid;
	}else{  // query specific
		if(!objIid)
			return ZCFG_INTERNAL_ERROR;
#ifdef ZCFG_MEMORY_LEAK_DETECT
		ret = queryObjRout(objOid, objIid, &queriedObj, fileName, line);
#else
		ret = queryObjRout(objOid, objIid, &queriedObj);
#endif
		outIid = objIid;
	}

	if(ret != ZCFG_SUCCESS){
		return ret;
	}
	printf("%s: %u %hhu,%hhu,%hhu,%hhu,%hhu,%hhu queried!\n", __FUNCTION__, objOid, objIid->idx[0],objIid->idx[1],objIid->idx[2],
		objIid->idx[3],objIid->idx[4],objIid->idx[5]);

	const char **iidACL = objAcl->iid;
	if(iidACL && queryObjNextRout){
		//require iid authorization
		char objIidStrSave[30+1] = {0};
		sprintf(objIidStrSave, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu", objIid->idx[0],objIid->idx[1],objIid->idx[2],
				objIid->idx[3],objIid->idx[4],objIid->idx[5]);
		char *p = strstr(objIidStrSave, "0");
		if(p && p != &objIidStrSave[0]){
			*(p-1) = '\0'; //n,0
		}

		if(zcfgFeDalACLIidAuth(iidACL, objIidStrSave) != 0)
			return ZCFG_SUCCESS;
	}

	const char **rdmAcLParams = objAcl->params;
	if(rdmAcLParams){
		struct json_object *paramObj;
		const char *parmName;
		value = json_object_new_object();
		int n = 0;
		while((parmName = rdmAcLParams[n++])){
			paramObj = json_object_object_get(queriedObj, parmName);
			if(paramObj)
				json_object_object_add(value, parmName, JSON_OBJ_COPY(paramObj));
		}
	}else{
		value = queriedObj;
	}

	//char *retString = retStringGet[ret-ZCFG_RET_OFFSET];
	struct json_object *responseObj = json_object_new_object();
	struct json_object *iidArray = json_object_new_array();
	int iidIdx = 0;
	bool running = true;
	while(running){
		json_object_array_add(iidArray, json_object_new_int(outIid->idx[iidIdx++]));
		running = (iidIdx > 5) ? false : true;
	}

	json_object_object_add(responseObj, "Iid", iidArray);
	json_object_object_add(responseObj, "Object", value);
	json_object_array_add(response, responseObj);
	if(rdmAcLParams)
		json_object_put(queriedObj);

	return ZCFG_SUCCESS;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
static zcfgRet_t zcfgFeDalQueryRdmObjs(rdmObjectACL_t *objAcl, queryRdmSubObjRout querySubObjRout, queryRdmObjRout queryObjNextRout,
			const char *oidStr, zcfg_offset_t objOid, objIndex_t *objIid, struct json_object *response, const char *fileName, int line)
#else
static zcfgRet_t zcfgFeDalQueryRdmObjs(rdmObjectACL_t *objAcl, queryRdmSubObjRout querySubObjRout, queryRdmObjRout queryObjNextRout,
			const char *oidStr, zcfg_offset_t objOid, objIndex_t *objIid, struct json_object *response)
#endif
{
	zcfgRet_t ret;
	objIndex_t childIid;
	bool querying = true;
	//char *rdmAcLParams[]
	// !objOid, !objIid, !response
	// !querySubObjRout && !queryObjRout

	IID_INIT(childIid);
	while(querying){
		if(querySubObjRout){
#ifdef ZCFG_MEMORY_LEAK_DETECT
			ret = zcfgFeDalQueryRdmObj(objAcl, querySubObjRout, NULL, NULL, oidStr, objOid, objIid, &childIid, response, fileName, line);
#else
			ret = zcfgFeDalQueryRdmObj(objAcl, querySubObjRout, NULL, NULL, oidStr, objOid, objIid, &childIid, response);
#endif
		}else{ // query object next
#ifdef ZCFG_MEMORY_LEAK_DETECT
			ret = zcfgFeDalQueryRdmObj(objAcl, NULL, queryObjNextRout, NULL, oidStr, objOid, objIid, NULL, response, fileName, line);
#else
			ret = zcfgFeDalQueryRdmObj(objAcl, NULL, queryObjNextRout, NULL, oidStr, objOid, objIid, NULL, response);
#endif
		}

		if(ret != ZCFG_SUCCESS){
			querying = false;
		}
	}

	return ZCFG_SUCCESS;
}

static rdmObjectACL_t *retrieveRdmObjectACL(zcfg_offset_t oid)
{
	int n = 0;
	int picked = -1;
	rdmObjectACL_t *rdmObjectACListAddr = rdmObjectACList;
	rdmObjectACL_t *pickZhttpRdmACL = NULL;

	while(picked == -1 && rdmObjectACList[n].method)
	{
		picked = (oid == rdmObjectACList[n].oid) ? n : -1;
		n++;
	}

	pickZhttpRdmACL = (picked >= 0) ? (rdmObjectACListAddr+picked) : NULL;

	return pickZhttpRdmACL;
}

static void iidStr2ObjIid(const char *iid, objIndex_t *objIid)
{
	char iidStr[30+1] = {0};
	char *iidSavePtr = NULL, *p;
	int n = 0;

	ZOS_STRNCPY(iidStr, iid, sizeof(iidStr));
	IID_INIT((*objIid));
	p = strtok_r(iidStr, ",", &iidSavePtr);
	bool running = p ? true : false;

	while(running){
		objIid->idx[n] = atoi(p);
		objIid->level = (objIid->idx[n]) ? n+1 : objIid->level;
		running = (objIid->idx[n] > 0) ? true : false;

		p = running ? strtok_r(NULL, ",", &iidSavePtr) : NULL;
		running = p ? true : false;
		n++;
	}
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeDalQueryRdmObjHandler(zcfg_offset_t oid, const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg, const char *fileName, int line)
#else
zcfgRet_t zcfgFeDalQueryRdmObjHandler(zcfg_offset_t oid, const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
#endif
{
	struct json_object *iidObj;
	char oidStr[256+1] = {0};
	char iidStrSave[30+1] = {0};
	objIndex_t objIid;

	//printf("%s: Jobj=%s\n", __FUNCTION__, json_object_to_json_string(Jobj));
	printf("%s: oid=%u\n", __FUNCTION__, oid);
	//sprintf(oidStr, "%u", oid);

	const char *iidStr = (iidObj = json_object_object_get(Jobj, "iid")) ? json_object_get_string(iidObj) : NULL;
	printf("%s: iid=%s\n", __FUNCTION__, iidStr ? iidStr : "empty");
	if(iidStr){
		char *p1 = (p1 = strstr(iidStr, "[")) ? p1+1 : NULL;
		char *p2 = p1 ? strstr(p1, "]") : NULL;
		if(p1 && p2 && (p2 > p1)){
			strncpy(iidStrSave, p1, sizeof(iidStrSave)-1);
			iidStrSave[p2-p1] = '\0';
		}
	}

	rdmObjectACL_t *objACL = retrieveRdmObjectACL(oid);
	if(!objACL){
		// only authorized object could access by rdm oid/iid
		// printf("Not authorized!\n");
		return ZCFG_REQUEST_REJECT;
	}

	// authorize iid
	if(objACL->iid){
		//require iid authorization
		if(strlen(iidStrSave)){
			if(zcfgFeDalACLIidAuth(objACL->iid, iidStrSave) != 0)
				return ZCFG_REQUEST_REJECT;

			printf("%s: %s, %s authorized\n", __FUNCTION__, oidStr, iidStrSave);
			iidStr2ObjIid(iidStrSave, &objIid);
		}
	}else{
		if(strlen(iidStrSave))
			iidStr2ObjIid(iidStrSave, &objIid);
	}

	if(!strlen(iidStrSave)){
		IID_INIT(objIid);
		int objInfoAttr = zcfgFeObjAttrGet(oid);
		if(!(objInfoAttr & ATTR_INDEXNODE))
#ifdef ZCFG_MEMORY_LEAK_DETECT
			zcfgFeDalQueryRdmObj(objACL, NULL, NULL, (queryRdmObjRout)_zcfgFeObjJsonGet,
					oidStr, oid, &objIid, NULL, Jarray, fileName, line);
#else
			zcfgFeDalQueryRdmObj(objACL, NULL, NULL, (queryRdmObjRout)zcfgFeObjJsonGet,
					oidStr, oid, &objIid, NULL, Jarray);
#endif
		else
#ifdef ZCFG_MEMORY_LEAK_DETECT
			zcfgFeDalQueryRdmObjs(objACL, NULL, (queryRdmObjRout)_zcfgFeObjJsonGetNext,
					oidStr, oid, &objIid, Jarray, fileName, line);
#else
			zcfgFeDalQueryRdmObjs(objACL, NULL, (queryRdmObjRout)zcfgFeObjJsonGetNext,
					oidStr, oid, &objIid, Jarray);
#endif
	}else{ // specific
#ifdef ZCFG_MEMORY_LEAK_DETECT
		zcfgFeDalQueryRdmObj(objACL, NULL, NULL, (queryRdmObjRout)_zcfgFeObjJsonGet,
				oidStr, oid, &objIid, NULL, Jarray, fileName, line);
#else
		zcfgFeDalQueryRdmObj(objACL, NULL, NULL, (queryRdmObjRout)zcfgFeObjJsonGet,
				oidStr, oid, &objIid, NULL, Jarray);
#endif
	}

	return ZCFG_SUCCESS;
}

