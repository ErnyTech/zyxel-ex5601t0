#include <ctype.h>
#include <json/json.h>
#include "zcfg_common.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_fe_rdm_string.h"
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN
dal_param_t MultiWan_param[] = {
	{"Enable", 					dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"Init", 					dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"Name",				    dalType_string,     0,	0,	NULL,		NULL,			0},
	{"Index",				    dalType_string,     0,	0,	NULL,		NULL,			0},
	{"Mode",				    dalType_string,     0,	0,	NULL,		NULL,			0},
	{"Priority",			    dalType_int,        0,	0,	NULL,		NULL,			0},
	{"Failback",		        dalType_string,     0,	0,	NULL,		NULL,			0},
	{NULL,	0,	0,	0,	NULL,	NULL,	0},
};


//RDM_OID_MULTI_WAN_IFACE

int transStringToObjIndex(objIndex_t* output, char* input)
{
	char* sp = NULL;
	char* index = NULL;
	char* delim = ".";
	index = strtok_r(input, delim, &sp);
	int i = 0;
	while (index != NULL)
	{
		output->idx[i++] = atoi(index);
		index = strtok_r(sp, delim, &sp);
	}
	output->level = i;
	return 0;
}

int transObjIndexToString(objIndex_t* input, char* output, size_t stringSize)
{
	int count = 0;
	int offset = 0;
	for (int i = 0; i < input->level; ++i)
	{
		offset += snprintf(output + offset, stringSize - offset, "%d.", input->idx[i]);
	}
	output[offset - 1] = '\0';
	return 0;
}

int ObjIndexCompare(objIndex_t* one, objIndex_t* two)
{
	if (one->level != two->level)
		return 1;
	for (int i = 0; i < one->level; ++i)
	{
		if (one->idx[i] != two->idx[i])
			return 1;
	}
	return 0;
}

zcfgRet_t zcfgFeDal_MultiWan_Edit(struct json_object* Jobj, char* replyMsg) {
	//printf("%s() enter\n", __func__);
	objIndex_t MultiWanObjIid = { 0 };
	IID_INIT(MultiWanObjIid);
	struct json_object* MultiWanRootObj = NULL, * MultiWanTmpObj = NULL, * currObj = NULL, * MultiWanArray = NULL, * MultiWanArrayObj = NULL, * objIndexObj = NULL;
	struct json_object* currArray = NULL, * currArrayObj = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	bool Enable;
	int index = 0;
	int MWALen = 0;
	int cALen = 0;
	int i, j, Priority;
	char* currArrayObjName, * MultiWanArrayObjName, * currArrayObjIndex;
	printf("Jobj\n %s\n", json_object_to_json_string(Jobj));
	if ((ret = zcfgFeObjJsonGet(RDM_OID_MULTI_WAN, &MultiWanObjIid, &MultiWanRootObj)) != ZCFG_SUCCESS)
		return ret;
	currObj = json_object_object_get(Jobj, "Enable");
	if (currObj)
	{
		json_object_object_add(MultiWanRootObj, "Enable", json_object_new_boolean(json_object_get_boolean(currObj)));
	}
	currObj = json_object_object_get(Jobj, "Init");
	if (currObj)
	{
		json_object_object_add(MultiWanRootObj, "Init", json_object_new_boolean(json_object_get_boolean(currObj)));
	}

	currObj = json_object_object_get(Jobj, "NumberOfInterface");
	if (currObj)
	{
		json_object_object_add(MultiWanRootObj, "NumberOfInterface", json_object_new_int(json_object_get_int(currObj)));
	}

	zcfgFeObjJsonBlockedSet(RDM_OID_MULTI_WAN, &MultiWanObjIid, MultiWanRootObj, NULL);
	zcfgFeJsonObjFree(MultiWanRootObj);
	objIndexObj = json_object_object_get(Jobj, "Index");
	if (objIndexObj)
	{
		currArrayObjIndex = json_object_get_string(objIndexObj);

		objIndex_t tmpiid, tmpiid2;
		IID_INIT(tmpiid);
		IID_INIT(tmpiid2);
		transStringToObjIndex(&tmpiid2, currArrayObjIndex);
		while (zcfgFeObjJsonGetNext(RDM_OID_MULTI_WAN_IFACE, &tmpiid, &MultiWanArrayObj) == ZCFG_SUCCESS)
		{
			if (!ObjIndexCompare(&tmpiid2, &tmpiid))
			{
				currObj = json_object_object_get(Jobj, "Priority");
				if (currObj)
				{
					json_object_object_add(MultiWanArrayObj, "Priority", json_object_new_int(json_object_get_int(currObj)));
				}

				currObj = json_object_object_get(Jobj, "Failback");
				if (currObj)
				{
					json_object_object_add(MultiWanArrayObj, "Failback", json_object_new_string(json_object_get_string(currObj)));
				}

				currObj = json_object_object_get(Jobj, "Mode");
				if (currObj)
				{
					json_object_object_add(MultiWanArrayObj, "Mode", json_object_new_string(json_object_get_string(currObj)));
				}
				currObj = json_object_object_get(Jobj, "Name");
				if (currObj)
				{
					json_object_object_add(MultiWanArrayObj, "Name", json_object_new_string(json_object_get_string(currObj)));
				}
				zcfgFeObjJsonBlockedSet(RDM_OID_MULTI_WAN_IFACE, &tmpiid, MultiWanArrayObj, NULL);
				zcfgFeJsonObjFree(MultiWanArrayObj);
				break;
			}
			zcfgFeJsonObjFree(MultiWanArrayObj);
		}
	}
	return ret;
}

zcfgRet_t zcfgFeDal_MultiWan_Get(struct json_object* Jobj, struct json_object* Jarray, char* replyMsg) {
	objIndex_t Objidx;
	struct json_object* currObj = NULL, * MultiWanArrayObj = NULL, * MultiWanTmpObj = NULL, * tmpObj = NULL, * interfaceObj = NULL;
	IID_INIT(Objidx);
	zcfgRet_t ret = ZCFG_SUCCESS;
	char* currArrayObjIndex = NULL, * MultiWanArrayObjName = NULL;
	char indexstring[32];
	currObj = json_object_object_get(Jobj, "index");
	if (currObj)
	{
		currArrayObjIndex = json_object_get_string(currObj);
	}
	else
	{
		if ((ret = zcfgFeObjJsonGet(RDM_OID_MULTI_WAN, &Objidx, &currObj)) != ZCFG_SUCCESS)
			return ret;
		tmpObj = JSON_OBJ_COPY(currObj);
		interfaceObj = json_object_new_array();
		zcfgFeJsonObjFree(currObj);
	}

	objIndex_t tmpiid;
	IID_INIT(tmpiid);
	while (zcfgFeObjJsonGetNext(RDM_OID_MULTI_WAN_IFACE, &tmpiid, &MultiWanArrayObj) == ZCFG_SUCCESS)
	{
		if (currArrayObjIndex)
		{
			transObjIndexToString(&tmpiid, indexstring, sizeof(indexstring));
			if (!strcmp(indexstring, currArrayObjIndex)) //same find the key
			{
				json_object_object_add(MultiWanArrayObj, "Index", json_object_new_string(indexstring));
				json_object_array_add(Jarray, JSON_OBJ_COPY(MultiWanArrayObj));
				zcfgFeJsonObjFree(MultiWanArrayObj);
				break;
			}
		}
		else
		{
			transObjIndexToString(&tmpiid, indexstring, sizeof(indexstring));
			json_object_object_add(MultiWanArrayObj, "Index", json_object_new_string(indexstring));
			json_object_array_add(interfaceObj, JSON_OBJ_COPY(MultiWanArrayObj));
		}
		zcfgFeJsonObjFree(MultiWanArrayObj);
	}
	if (interfaceObj && tmpObj)
	{
		json_object_object_add(tmpObj, "interface", JSON_OBJ_COPY(interfaceObj));
		json_object_array_add(Jarray, JSON_OBJ_COPY(tmpObj));
		zcfgFeJsonObjFree(tmpObj);
		zcfgFeJsonObjFree(interfaceObj);
	}
	return ret;
}

zcfgRet_t zcfgFeDalShowMultiWan(struct json_object* Jarray) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	printf("zcfgFeDalShowMultiWan\n %s\n", json_object_to_json_string(Jarray));
	return ret;
}

zcfgRet_t zcfgFeDalMultiWan(const char* method, struct json_object* Jobj, struct json_object* Jarray, char* replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	if (!strcmp(method, "PUT"))
		ret = zcfgFeDal_MultiWan_Edit(Jobj, NULL);
	else if (!strcmp(method, "GET"))
		ret = zcfgFeDal_MultiWan_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
#endif