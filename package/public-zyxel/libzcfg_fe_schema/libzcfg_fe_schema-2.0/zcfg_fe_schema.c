#include <json/json.h>

#include "zcfg_common.h"
#include "zcmd_schema.h"
#include "zcfg_fe_schema.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_debug.h"
#include "zcfg_rdm_oid.h"

extern void *schemaShmAddr;
extern void *objNameShmAddr;
extern void *paramNameShmAddr;
extern void *objDefaultValPtr;
extern int schemaShmSize;

bool attrSetRollback = false;

typedef struct zcfgAttrRecord_s {
	uint32_t oid;
	char paramName[PARAMETER_NAME_BUFF_SIZE];
	uint32_t attr;
	struct zcfgAttrRecord_s *next;
}zcfgAttrRecord_t;
/*
 *  Function Name: zcfgFeObjIsSingle
 *  Description: check object is single onject
 */
int zcfgFeObjIsSingle(zcfg_offset_t rdmObjId)
{
	objInfo_t *objInfo = NULL;

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);
	
	if(objInfo){
		if(!(objInfo->attr & ATTR_INDEXNODE) && objInfo->idxLevel == 0)
			return 1;
		else
			return 0;
	}

	return -1;
}
/*
 *  Function Name: zcfgFeObjAttrGet
 *  Description: Used to get object attribute
 */
int zcfgFeObjAttrGet(zcfg_offset_t rdmObjId)
{
	objInfo_t *objInfo = NULL;

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);

	return objInfo->attr;
}

int zcfgFeObjQueryMaxInstance(zcfg_offset_t rdmObjId)
{
	objInfo_t *objInfo = NULL;
	uint8_t maxInstance = 0;

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);

	if(objInfo->attr & ATTR_INDEXNODE && objInfo->idxLevel)
		maxInstance = objInfo->maxEntry[objInfo->idxLevel-1];

	return maxInstance;
}

/*
 *  Function Name: zcfgFeAllParamNameGet
 *  Description: Used to get all the name of parameters in the object.
 *               The result will be stored in json object "nameList"
 */
zcfgRet_t zcfgFeAllParamNameGet(zcfg_offset_t rdmObjId, struct json_object **nameList)
{
	objInfo_t *objInfo = NULL;
	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);
	*nameList = json_object_new_object();

	if(*nameList == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "Can't new a json object\n");
		return ZCFG_INTERNAL_ERROR;
	}

	GET_PAMAMETERS_IN_OBJECT(objInfo, paramInfo, paramName) {
		json_object_object_add(*nameList, paramName, json_object_new_string(""));
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeObjParamInfoQuery(zcfg_offset_t rdmObjId, struct json_object **objParamInfo)
{
	objInfo_t *objInfo;
	struct json_object *paramObj;

	//if(schemaShmAddr == NULL)
	//	zcfgFeSharedMemInit();

	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);
	*objParamInfo = json_object_new_object();

	GET_PAMAMETERS_IN_OBJECT(objInfo, paramInfo, parmName){
		paramObj = json_object_new_object();
		json_object_object_add(paramObj, "attr", json_object_new_int(paramInfo->attr));
		//json_object_object_add(paramObj, "type", json_object_new_int(paramInfo->type));

		json_object_object_add(*objParamInfo, parmName, paramObj);
	}

	return ZCFG_SUCCESS;
}
/*
 *  Function Name: zcfgFeParamTypeGetByName
 *  Description: Used to get parameter type from schema
 */
int zcfgFeParamTypeGetByName(zcfg_offset_t rdmObjId, char *parameterName)
{
	objInfo_t *objInfo = NULL;

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);

	GET_PAMAMETERS_IN_OBJECT(objInfo, paramInfo, paramName) {
		if(strcmp(paramName, parameterName) == 0)
			return (uint32_t)paramInfo->type;
	}

	return ZCFG_NO_SUCH_PARAMETER;
}
/*
 *  Function Name: zcfgFeParamAttrSet
 *  Description: Used to set parameter attribute
 *
 */

zcfgRet_t zcfgFeGetSchemaParameterByOid(uint32_t oid, struct json_object* defaultJobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	objInfo_t *objInfo;
	char *string = NULL;
	bool dftBool;

	struct json_object* setValue = NULL;

	if((schemaShmAddr == NULL) || (objDefaultValPtr == NULL))
		zcfgFeSharedMemInit();

	objInfo = GET_OBJ_INFO_BY_OID(oid);

	GET_PAMAMETERS_IN_OBJECT(objInfo, paramInfo, paramName){
		switch (paramInfo->type) {
			case json_type_boolean:
				/*Use default value defined in schema*/
				dftBool = paramInfo->iniVal;
				if(dftBool == 1) {
					setValue = json_object_new_boolean(true);
				}
				else {
					setValue = json_object_new_boolean(false);
				}

				json_object_object_add(defaultJobj, paramName, setValue);
				break;
			case json_type_int:
				/*Use default value defined in schema*/
				setValue = json_object_new_int(paramInfo->iniVal);
				json_object_object_add(defaultJobj, paramName, setValue);
				break;
			case json_type_uint8:
			case json_type_uint16:
			case json_type_uint32:
			case json_type_ulong:
				/*Use default value defined in schema*/
				setValue = json_object_new_int(paramInfo->iniVal);

				json_object_object_add(defaultJobj, paramName, setValue);
				break;
			case json_type_string:
			case json_type_time:
			case json_type_base64:
			case json_type_hex:
				/*Use default value defined in schema*/
				string = objDefaultValPtr + paramInfo->iniVal;
				setValue = json_object_new_string(string);

				json_object_object_add(defaultJobj, paramName, setValue);
				break;
			default:
				zcfgLog(ZCFG_LOG_ERR, "%s(): Get parameter[%s] error.\n", __FUNCTION__, paramName);
				return ZCFG_INTERNAL_ERROR;
				break;
		}
	}

	return ret;
}

zcfgRet_t zcfgFeParamAttrSet(zcfg_offset_t rdmObjId, char *parameterName, uint32_t attr)
{
	int ret = 0;
	objInfo_t *objInfo = NULL;
	char *objName = NULL;
	struct json_object *setParamInfo = NULL;
	struct json_object *schemaParamAttr = NULL;
	bool find = false;
	bool denyActiveNotifyRequest = false;
	uint32_t newNotifyAttr = 0;
	FILE *fptr = NULL;

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	if((fptr = fopen(ZCFG_ATTR_MODIFIED_SCHEMA_FILE, "r")) == NULL) {
		schemaParamAttr = json_object_new_object();
		if(schemaParamAttr == NULL) {
			zcfgLog(ZCFG_LOG_ERR, "Can't new a json object\n");
			return ZCFG_INTERNAL_ERROR;
		}
	}
	else {
		fclose(fptr);
		schemaParamAttr = json_object_from_file(ZCFG_ATTR_MODIFIED_SCHEMA_FILE); 
	}

	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);

	GET_PAMAMETERS_IN_OBJECT(objInfo, paramInfo, paramName) {
		if(strcmp(parameterName, paramName) == 0) {
			find = true;
			/*Notify attribute was modified*/
			if((paramInfo->attr & PARAMETER_ATTR_NOTIFY_MASK) != (attr & PARAMETER_ATTR_NOTIFY_MASK)) {
				newNotifyAttr = attr & PARAMETER_ATTR_NOTIFY_MASK;
				if(!attrSetRollback) {
					denyActiveNotifyRequest = paramInfo->attr & PARAMETER_ATTR_DENY_ACTIVENOTIFY_REQUEST;
					/*Deny Request for Active Notification*/
					if(newNotifyAttr == PARAMETER_ATTR_ACTIVENOTIFY && denyActiveNotifyRequest) {
						zcfgLog(ZCFG_LOG_ERR, "Can't change to active notify of %s\n", parameterName);
						json_object_put(schemaParamAttr);
						return ZCFG_SET_ATTR_ERROR;
					}
				}
			}
			paramInfo->attr = attr;
			break;
		}
	}

	if(!find) {
		zcfgLog(ZCFG_LOG_ERR, "There is not a parameter named %s in schema\n", parameterName);
		return ZCFG_NO_SUCH_PARAMETER;
	}

	objName = GET_OBJ_NAME(objInfo);
	setParamInfo = json_object_object_get(schemaParamAttr, objName);

	if(setParamInfo == NULL) {
		setParamInfo = json_object_new_object();
		json_object_object_add(schemaParamAttr, objName, setParamInfo);
	}
	
	json_object_object_add(setParamInfo, parameterName, json_object_new_int(attr));

	ret = json_object_to_file(ZCFG_ATTR_MODIFIED_SCHEMA_FILE, schemaParamAttr);

	if(ret == -1) {
		zcfgLog(ZCFG_LOG_ERR, "Save the changes of schema to file failed\n");
		json_object_put(schemaParamAttr);
		json_object_put(setParamInfo);
		return ZCFG_INTERNAL_ERROR;
	}

	json_object_put(schemaParamAttr);
	json_object_put(setParamInfo);

	return ZCFG_SUCCESS;
}
/*
 *  Function Name: zcfgFeParamAttrValGetByName
 *  Description: Used to get parameter attribute by parameter name
 */
int zcfgFeParamAttrGetByName(zcfg_offset_t rdmObjId, char *parameterName)
{
	objInfo_t *objInfo = NULL;

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);
	GET_PAMAMETERS_IN_OBJECT(objInfo, paramInfo, paramName) {
		if(strcmp(paramName, parameterName) == 0) {
			return paramInfo->attr;
		}
	}
	return ZCFG_NO_SUCH_PARAMETER;
}
/*
 *  Function Name: zcfgFeMultiParamAttrAppend
 *  Description: append a single attribute of parameter to a json object with following format
 *  
 *  Multiple Attribute format : { "OID1" : { Parameter1 : Attribute1,
 *                                           Parameter2 : Attribute2,
 *                                                      :
 *                                           Parametern : Attributen
 *                                         },
 *                                "OID2" : {
 *                                      :
 *                              }
 *
 */
zcfgRet_t zcfgFeMultiParamAttrAppend(zcfg_offset_t oid, json_object *multiAttrJobj, char *paramName, int newAttr)
{
	struct json_object *attrJobj = NULL;
	struct json_object *attrVal = NULL;
	char objID[16];

	if(multiAttrJobj == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "The multiJobj pointer is NULL value\n");
		return ZCFG_INTERNAL_ERROR;
	}

	sprintf(objID, "%d", oid);

	attrJobj = json_object_object_get(multiAttrJobj, objID);
	if(attrJobj != NULL) {
		json_object_object_add(attrJobj, paramName, json_object_new_int(newAttr));
	}
	else {
		attrJobj = json_object_new_object();
		if(attrJobj == NULL)
			return ZCFG_INTERNAL_ERROR;

		attrVal = json_object_new_int(newAttr);
		if(attrVal == NULL)
			return ZCFG_INTERNAL_ERROR;

		json_object_object_add(multiAttrJobj, objID, attrJobj);
		json_object_object_add(attrJobj, paramName, attrVal);
	}

	return ZCFG_SUCCESS;
}
/*
 *  Function Name: zcfgFeMultiAttrSet
 *  Description: Used to set multiple attribute
 *
 *  Return value : 1. ZCFG_SUCCESS
 *                 2. ZCFG_INTERNAL_ERROR
 *                 3. ZCFG_SET_ATTR_ERROR : modify the notification attribute, but the notification attribute can not be changed
 *                 4. ZCFG_NO_SUCH_PARAMETER : the paramter name to set can not be found in schema
 */
zcfgRet_t zcfgFeMultiAttrSet(struct json_object *multiAttrJobj)
{
	zcfgRet_t ret;
	objInfo_t *objInfo = NULL;
	uint32_t oldAttr = 0;
	int newAttr = 0;
	zcfgAttrRecord_t *head = NULL, *now = NULL;
	zcfgAttrRecord_t *new = NULL, *next = NULL;
	bool find = false;
	zcfg_offset_t rdmObjId = 0;

	json_object_object_foreach(multiAttrJobj, oid, attrVal) {
		rdmObjId = atoi(oid);
		objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);

		json_object_object_foreach(attrVal, paramName, attr) {

			GET_PAMAMETERS_IN_OBJECT(objInfo, paramInfo, parameterName) {
				if(strcmp(parameterName, paramName) == 0) {
					find = true;
					oldAttr = paramInfo->attr;
					break;
				}
			}

			newAttr = json_object_get_int(attr);
			ret = zcfgFeParamAttrSet(rdmObjId, paramName, newAttr);

			if(ret != ZCFG_SUCCESS) {
				zcfgLog(ZCFG_LOG_INFO, "Do rollback\n");
				attrSetRollback = true;
				for(now = head; now != NULL; now = now->next) {
					if(zcfgFeParamAttrSet(now->oid, now->paramName, now->attr) != ZCFG_SUCCESS)
						zcfgLog(ZCFG_LOG_ERR, "Recover %s attribute fail\n", now->paramName);
				}
				attrSetRollback = false;
				/*Free record link list*/
				now = head;
				while(now != NULL) {
					next = now->next;
					free(now);
					now = next;
				}

				head = NULL;
				now = NULL;
				return ret;
			}
			else {
				new = (zcfgAttrRecord_t *)malloc(sizeof(zcfgAttrRecord_t));
				new->oid = rdmObjId;
				new->attr = oldAttr;
				strcpy(new->paramName, paramName);
				new->next = NULL;

				if(head == NULL) {
					head = new;
					now = new;
				}
				else {
					now->next = new;
					now = new;
				}
			}
		}
	}

	/*Free record link list*/
	now = head;
	while(now != NULL) {
		next = now->next;
		free(now);
		now = next;
	}

	return ZCFG_SUCCESS;
}
/*
 *  Function Name: zcfgFeSubObjNameListGet
 *  Description: Used to get sub-object name list under the specified Object ID
 *
 *  Return value : 1. NULL : No sub-object
 *                 2. If sub-object exists, it will return a link-list containing all sub-object name.
 *
 *  Ex: For A.B and A.C.i, it will return B and C.i when you asks for sub-object name of A
 */
zcfgSubObjNameList_t* zcfgFeSubObjNameListGet(zcfg_offset_t rdmObjId)
{
	zcfgSubObjNameList_t *head = NULL, *now = NULL, *new = NULL;
        objInfo_t *objInfo = NULL;
	char *parentObjName = NULL;
	char *name = NULL;
	char *subName = NULL;
	uint32_t oid = 0;
	bool find = false;

        if(schemaShmAddr == NULL)
                zcfgFeSharedMemInit();

	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);
	parentObjName = GET_OBJ_NAME(objInfo);

	oid = rdmObjId + sizeof(objInfo_t) + (objInfo->count)*sizeof(parameterInfo_t);

	while(oid < schemaShmSize) {
		objInfo = GET_OBJ_INFO_BY_OID(oid);
		name = GET_OBJ_NAME(objInfo);
		if((subName = strstr(name, parentObjName)) != NULL) {
			//subName = subName + strlen(parentObjName) + 1;
			/*  Avoid the case : Device.IP
			 *		     Device.IPsec
			 */
			subName = subName + strlen(parentObjName);
			if(*subName != '.')
				break;
			else
				subName = subName + 1;

			if(strstr(subName, ".") == NULL) {
				find = true;
			}
			else {
				subName = strchr(subName, '.');
				if(*(subName+1) == 'i' && *(subName+2) != '.')
					find = true;
			}
		}

		if(find) {
			new = (zcfgSubObjNameList_t*)malloc(sizeof(zcfgSubObjNameList_t));
			subName = name + strlen(parentObjName) + 1;
			new->objName = (char *)malloc(strlen(subName)+1);
			strcpy(new->objName, subName);
			if(head == NULL) {
				new->next = NULL;
				head = new;
				now = new;
			}
			else {
				now->next = new;
				now = new;
				now->next = NULL;
			}
			find = false;
		}

		oid = oid + sizeof(objInfo_t) + (objInfo->count)*sizeof(parameterInfo_t);
	}

        return head;
}
/*
 *  Function Name: zcfgFeSubObjListFree 
 *  Description: Used to free the sub-object name link-list
 */
void zcfgFeSubObjListFree(zcfgSubObjNameList_t *head)
{
	zcfgSubObjNameList_t *now = NULL, *next = NULL;;

	now = head;
	while(now != NULL) {
		free(now->objName);
		next = now->next;
		free(now);
		now = next;
	}
}
