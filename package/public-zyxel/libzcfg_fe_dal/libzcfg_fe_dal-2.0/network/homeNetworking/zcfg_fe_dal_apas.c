#ifdef ZYXEL_ANY_PORT_ANY_SERVICE
#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
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

dal_param_t APAS_param[] =
{
	{"Enable", 		dalType_boolean, 	0, 	0,	NULL,	NULL, 	dal_Edit},
	{NULL, 			0, 					0, 	0, 	NULL,	NULL,	NULL}
};

dal_param_t APASMacRule_param[] =
{
	{"Index",		dalType_int, 		0, 	0,	NULL,	NULL, 	dal_Edit|dal_Delete},
	{"Enable",		dalType_boolean, 	0, 	0,	NULL,	NULL, 	dal_Add},
	{"Name",		dalType_string, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{"MacRule",		dalType_string, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{"WanBridgeIntf", dalType_string,	0,	0,	NULL,	NULL,	dal_Add},
	{NULL, 			0, 					0, 	0, 	NULL,	NULL,	NULL}
};

void zcfgFeDalShowAPAS(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-10s \n", "Enable");
	len = json_object_array_length(Jarray);
	for(i=0; i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-10s \n", json_object_get_boolean(json_object_object_get(obj, "Enable")));
	}
}

zcfgRet_t zcfgFeDal_APAS_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *apasObj = NULL;
	objIndex_t objIid = {0};
	bool enable = false;

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_A_P_A_S, &objIid, &apasObj)) == ZCFG_SUCCESS){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		if(json_object_object_get(Jobj, "Enable") != NULL) json_object_object_add(apasObj, "Enable", json_object_new_boolean(enable));

		ret = zcfgFeObjJsonSet(RDM_OID_A_P_A_S, &objIid, apasObj, NULL);
		zcfgFeJsonObjFree(apasObj);
	}
	return ret;
}

zcfgRet_t zcfgFeDal_APAS_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *apasObj = NULL;	//Get data model parameter
	objIndex_t objIid = {0};

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_A_P_A_S, &objIid , &apasObj)) == ZCFG_SUCCESS){
		struct json_object *paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(apasObj,"Enable")));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(apasObj);
	}
	return ret;
}


zcfgRet_t zcfgFeDalAPAS(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_APAS_Edit(Jobj, NULL);
	}else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_APAS_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

void zcfgFeDalShowAPASMacRule(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-10s %-10s %-10s %-40s %-10s \n", "Index", "Enable", "Name", "MacRule", "WanBridgeIntf");
	len = json_object_array_length(Jarray);
	for(i=0; i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-10s %-10s %-10s %-40s %-10s \n",
		    json_object_get_string(json_object_object_get(obj, "Index")),
		    json_object_get_boolean(json_object_object_get(obj, "Enable")),
			json_object_get_boolean(json_object_object_get(obj, "Name")),
		    json_object_get_string(json_object_object_get(obj, "MacRule")),
		    json_object_get_string(json_object_object_get(obj, "WanBridgeIntf")));
	}
}

zcfgRet_t zcfgFeDal_APASMacRule_Add(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *apasmacObj = NULL;
	objIndex_t objIid = {0};
	int count = 0;
	bool enable = false;
	const char *name = NULL;
	const char *mac = NULL;
	const char *brwan = NULL;

	IID_INIT(objIid);
	printf("QQQQQ %s %d ret %d\n", __FUNCTION__, __LINE__, ret);
	if((ret = zcfgFeObjJsonAdd(RDM_OIDMACF, &objIid, NULL)) == ZCFG_SUCCESS &&
	   (ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OIDMACF, &objIid, &apasmacObj)) == ZCFG_SUCCESS){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		name = json_object_get_string(json_object_object_get(Jobj, "Name"));
		mac = json_object_get_string(json_object_object_get(Jobj, "MacRule"));
		brwan = json_object_get_string(json_object_object_get(Jobj, "WanBridgeIntf"));
		json_object_object_add(apasmacObj, "Enable", json_object_new_boolean(enable));
		json_object_object_add(apasmacObj, "Name", json_object_new_string(name));
		json_object_object_add(apasmacObj, "MacRule", json_object_new_string(mac));
		json_object_object_add(apasmacObj, "WanBridgeIntf", json_object_new_string(brwan));
		ret = zcfgFeObjJsonBlockedSet(RDM_OIDMACF, &objIid, apasmacObj, NULL);

		zcfgFeJsonObjFree(apasmacObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDal_APASMacRule_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *apasmacObj = NULL;
	objIndex_t objIid = {0};
	int rowIndex = 0;
	int count = 1;
	bool enable = false;
	const char *name = NULL;
	const char *brwan = NULL;
	const char *mac = NULL;
	bool found = false;

	rowIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));

	IID_INIT(objIid);
	while((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OIDMACF, &objIid, &apasmacObj)) == ZCFG_SUCCESS) {
		if(count == rowIndex){
			found = true;
			break;
		}
		count++;
		zcfgFeJsonObjFree(apasmacObj);
	}
	
	if(found){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		name = json_object_get_string(json_object_object_get(Jobj, "Name"));
		mac = json_object_get_string(json_object_object_get(Jobj, "MacRule"));
		brwan = json_object_get_string(json_object_object_get(Jobj, "WanBridgeIntf"));
		if(json_object_object_get(Jobj, "Enable") != NULL) json_object_object_add(apasmacObj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "Name") != NULL) json_object_object_add(apasmacObj, "Name", json_object_new_string(name));
		if(json_object_object_get(Jobj, "MacRule") != NULL) json_object_object_add(apasmacObj, "MacRule", json_object_new_string(mac));
		if(json_object_object_get(Jobj, "WanBridgeIntf") != NULL) json_object_object_add(apasmacObj, "WanBridgeIntf", json_object_new_string(brwan));

		ret = zcfgFeObjJsonBlockedSet(RDM_OIDMACF, &objIid, apasmacObj, NULL);
		zcfgFeJsonObjFree(apasmacObj);
	}
	return ret;
}

zcfgRet_t zcfgFeDal_APASMacRule_Delete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *apasmacObj = NULL;
	objIndex_t objIid = {0};
	int rowIndex = 0;
	int count = 1;
	bool found = false;

	rowIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	IID_INIT(objIid);
	while((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OIDMACF, &objIid, &apasmacObj)) == ZCFG_SUCCESS) {
		if(count == rowIndex){
			found = true;
			break;
		}
		count++;
		zcfgFeJsonObjFree(apasmacObj);
	}

	if(found){
		ret = zcfgFeObjJsonDel(RDM_OIDMACF, &objIid, NULL);
		zcfgFeJsonObjFree(apasmacObj);
	}

	return ret;
}

void getMacRuleObj(int count, struct json_object *apasmacObj, struct json_object *Jarray){
	struct json_object *paramJobj = json_object_new_object();

	json_object_object_add(paramJobj, "Index", json_object_new_int(count));
	json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(apasmacObj,"Enable")));
	json_object_object_add(paramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(apasmacObj,"Name")));
	json_object_object_add(paramJobj, "MacRule", JSON_OBJ_COPY(json_object_object_get(apasmacObj,"MacRule")));
	json_object_object_add(paramJobj, "WanBridgeIntf", JSON_OBJ_COPY(json_object_object_get(apasmacObj,"WanBridgeIntf")));
	json_object_array_add(Jarray, paramJobj);
}

zcfgRet_t zcfgFeDal_APASMacRule_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *apasmacObj = NULL;	//Get data model parameter
	objIndex_t objIid = {0};
	int index = 0;

	IID_INIT(objIid);
	if(json_object_object_get(Jobj, "Index")){ //for New GUI get single obj
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		ret = convertIndextoIid(index, &objIid, RDM_OIDMACF, NULL, NULL, replyMsg);
		printf("QQQQQ %s %d ret %d\n", __FUNCTION__, __LINE__, ret);
		if(ret != ZCFG_SUCCESS)
			return ret;
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OIDMACF, &objIid , &apasmacObj)) == ZCFG_SUCCESS){
			getMacRuleObj(index, apasmacObj, Jarray);
			zcfgFeJsonObjFree(apasmacObj);
		}
	}else{
		//need allow 0 array so do not assign return value
		while(zcfgFeObjJsonGetNext(RDM_OIDMACF, &objIid, &apasmacObj) == ZCFG_SUCCESS){
			index ++;
			getMacRuleObj(index, apasmacObj, Jarray);
			zcfgFeJsonObjFree(apasmacObj);
		}
	}
	return ret;
}


zcfgRet_t zcfgFeDalAPASMacRule(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_APASMacRule_Edit(Jobj, NULL);
	}else if(!strcmp(method, "POST")) {
		ret = zcfgFeDal_APASMacRule_Add(Jobj, NULL);
	}else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDal_APASMacRule_Delete(Jobj, NULL);
	}else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_APASMacRule_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
#endif
