#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
//#include "zcfg_fe_rdm_struct.h"
#include "zcmd_schema.h"
#include "zcmd_tool.h"
#include "zcfg_msg.h"
#include "zos_api.h"

extern void *schemaShmAddr;

zcfgRet_t zcfgFeObjStructGetNext(zcfg_offset_t oid, objIndex_t *objIid, void **requestStruct)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_NEXT_OBJ, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr);
		ZOS_FREE(result);
	}

	return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeObjStructGetNext(zcfg_offset_t oid, objIndex_t *objIid, void **requestStruct, const char *fileName, int line)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_NEXT_OBJ, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = _zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr, fileName, line);
		ZOS_FREE(result);
	}

	return ret;
}
#endif

zcfgRet_t zcfgFeObjStructGetNextWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, void **requestStruct)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_NEXT_OBJ_WITHOUT_UPDATE, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr);
		ZOS_FREE(result);
	}

	return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeObjStructGetNextWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, void **requestStruct, const char *fileName, int line)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_NEXT_OBJ_WITHOUT_UPDATE, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = _zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr, fileName, line);
		ZOS_FREE(result);
	}

	return ret;
}
#endif

zcfgRet_t zcfgFeObjStructGet(zcfg_offset_t oid, objIndex_t *objIid, void **requestStruct)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_OBJ, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr);
		ZOS_FREE(result);
	}

    return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeObjStructGet(zcfg_offset_t oid, objIndex_t *objIid, void **requestStruct, const char *fileName, int line)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_OBJ, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = _zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr, fileName, line);
		ZOS_FREE(result);
	}

	return ret;
}
#endif

zcfgRet_t zcfgFeObjStructGetWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, void **requestStruct)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_OBJ_WITHOUT_UPDATE, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr);
		ZOS_FREE(result);
	}

    return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeObjStructGetWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, void **requestStruct, const char *fileName, int line)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_OBJ_WITHOUT_UPDATE, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = _zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr, fileName, line);
		ZOS_FREE(result);
	}

        return ret;
}
#endif

zcfgRet_t zcfgFeSubInStructGetNext(zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *insIid, void **requestStruct)
{
	zcfgRet_t ret;
	char *result = NULL;

	if(objIid->level >= insIid->level) {
		memcpy(insIid, objIid, sizeof(objIndex_t));
		insIid->level++;
	}

	insIid->idx[insIid->level-1]++;

	ret = zcfgFeObjRead(oid, insIid, REQUEST_GET_NEXT_SUB, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr);
		ZOS_FREE(result);
	}

	return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeSubInStructGetNext(zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *insIid, void **requestStruct, const char *fileName, int line)
{
	zcfgRet_t ret;
	char *result = NULL;

	if(objIid->level >= insIid->level) {
		memcpy(insIid, objIid, sizeof(objIndex_t));
		insIid->level++;
	}

	insIid->idx[insIid->level-1]++;

	ret = zcfgFeObjRead(oid, insIid, REQUEST_GET_NEXT_SUB, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = _zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr, fileName, line);
		ZOS_FREE(result);
	}

	return ret;
}
#endif

zcfgRet_t zcfgFeSubInStructGetNextWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *insIid, void **requestStruct)
{
	zcfgRet_t ret;
	char *result = NULL;

	if(objIid->level >= insIid->level) {
		memcpy(insIid, objIid, sizeof(objIndex_t));
		insIid->level++;
	}

	insIid->idx[insIid->level-1]++;

	ret = zcfgFeObjRead(oid, insIid, REQUEST_GET_NEXT_SUB_WITHOUT_UPDATE, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr);
		ZOS_FREE(result);
	}

	return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeSubInStructGetNextWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *insIid, void **requestStruct, const char *fileName, int line)
{
	zcfgRet_t ret;
	char *result = NULL;

	if(objIid->level >= insIid->level) {
		memcpy(insIid, objIid, sizeof(objIndex_t));
		insIid->level++;
	}

	insIid->idx[insIid->level-1]++;

	ret = zcfgFeObjRead(oid, insIid, REQUEST_GET_NEXT_SUB_WITHOUT_UPDATE, &result);

	if(ret == ZCFG_SUCCESS) {
		/*Translate json string to user request structure*/
		ret = _zcmdJStringToStruct(requestStruct, result, oid, schemaShmAddr, fileName, line);
		ZOS_FREE(result);
	}

	return ret;
}
#endif

zcfgRet_t zcfgFeObjStructSet(zcfg_offset_t oid, objIndex_t *objIid, void *requestStruct, char *feedbackMsg)
{
	zcfgRet_t ret;
	char *value = NULL;

	/*Translate the data structure to json string*/
	zcmdStructToJString(requestStruct, &value, oid, schemaShmAddr);
	if(value == NULL) {
		return ZCFG_INTERNAL_ERROR;
	}
	
	ret = zcfgFeObjWrite(oid, objIid, value, REQUEST_SET_OBJ, feedbackMsg);

	free(value);

	return ret;
}

#if 1 /* non-blocking config apply, ZyXEL, John */
zcfgRet_t zcfgFeObjStructBlockedSet(zcfg_offset_t oid, objIndex_t *objIid, void *requestStruct, char *feedbackMsg)
{
	zcfgRet_t ret;
	char *value = NULL;

	/*Translate the data structure to json string*/
	zcmdStructToJString(requestStruct, &value, oid, schemaShmAddr);
	if(value == NULL) {
		return ZCFG_INTERNAL_ERROR;
	}
	
	ret = zcfgFeObjWrite(oid, objIid, value, REQUEST_SET_OBJ | ZCFG_BLOCK_MODE_CONFIG_APPLY, feedbackMsg);

	free(value);

	return ret;
}
#endif

zcfgRet_t zcfgFeObjStructSetWithoutApply(zcfg_offset_t oid, objIndex_t *objIid, void *requestStruct, char *feedbackMsg)
{
	zcfgRet_t ret;
	char *value = NULL;

	/*Translate the data structure to json string*/
	zcmdStructToJString(requestStruct, &value, oid, schemaShmAddr);
	if(value == NULL) {
		return ZCFG_INTERNAL_ERROR;
	}

	ret = zcfgFeObjWrite(oid, objIid, value, REQUEST_SET_WITHOUT_APPLY, feedbackMsg);
	free(value);

	return ret;
}

zcfgRet_t zcfgFeObjStructAdd(zcfg_offset_t oid, objIndex_t *objIid, char *feedbackMsg)
{
	zcfgRet_t ret;

	ret = zcfgFeObjWrite(oid, objIid, NULL, REQUEST_ADD_INSTANCE, feedbackMsg);

	return ret;
}
#if 1 /* non-blocking config apply, ZyXEL, John */
zcfgRet_t zcfgFeObjStructBlockedAdd(zcfg_offset_t oid, objIndex_t *objIid, char *feedbackMsg)
{
	zcfgRet_t ret;

	ret = zcfgFeObjWrite(oid, objIid, NULL, REQUEST_ADD_INSTANCE | ZCFG_BLOCK_MODE_CONFIG_APPLY, feedbackMsg);

	return ret;
}

zcfgRet_t zcfgFeObjStructBlockedDel(zcfg_offset_t oid, objIndex_t *objIid, char *feedbackMsg)
{
	zcfgRet_t ret;

	ret = zcfgFeObjWrite(oid, objIid, NULL, REQUEST_DEL_INSTANCE  | ZCFG_BLOCK_MODE_CONFIG_APPLY, feedbackMsg);

	return ret;
}
#endif
zcfgRet_t zcfgFeObjStructDel(zcfg_offset_t oid, objIndex_t *objIid, char *feedbackMsg)
{
	zcfgRet_t ret;

	ret = zcfgFeObjWrite(oid, objIid, NULL, REQUEST_DEL_INSTANCE | ZCFG_BLOCK_MODE_CONFIG_APPLY, feedbackMsg);

	return ret;
}
