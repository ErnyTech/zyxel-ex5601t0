#include <ctype.h>

#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
//#include "zcfg_fe_rdm_string.h"
#include "zcmd_schema.h"
#include "zcfg_msg.h"
#include "zcfg_debug.h"
#include "zos_api.h"

extern void *schemaShmAddr;
extern void *objNameShmAddr;
extern void *paramNameShmAddr;
extern int schemaShmSize;
extern int myEid;


#include "zcfg_rdm_oid.h"
/*
 *  Function Name: zcfgFeObjNameToObjId
 *  Description: Translate the object name to object id
 *
 */
int zcfgFeObjNameToObjId(char *objName, objIndex_t *objIid)
{
	char *token = NULL;
	char *fullPathName = NULL;
	char *tmpName = NULL;
	char *name = NULL;
	uint8_t level = 0;
	uint16_t len = 0, c = 0, paramCount = 0;
	uint32_t oid = 0;
	objInfo_t *objInfo = NULL;
	bool find = false;
	char *tmp_ptr = NULL;

	if(!objName || !objIid)
		return ZCFG_INTERNAL_ERROR;

	IID_INIT(*objIid);
#if 0
#ifdef ZCFG_TR98_SUPPORT
	if(strstr(objName, "InternetGatewayDevice") != NULL) {
		printf("A TR98 data model\n");
		return ZCFG_TR98_DATA_MODEL;
	}
#endif
#endif
	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	if(objName == NULL || strlen(objName) == 0) {
		zcfgLog(ZCFG_LOG_ERR, "Object Name is NULL\n");
		return ZCFG_NO_SUCH_OBJECT;
	}

	fullPathName = (char *)malloc(strlen(objName)+8);
	tmpName = (char *)malloc(strlen(objName)+8);
	if (!strncmp(objName, "Device.", 7) || !strcmp(objName, "Device")){
		strcpy(tmpName, objName);
	}
	else{
		sprintf(tmpName, "%s.%s", "Device", objName);
	}

	token = strtok_r(tmpName, ".", &tmp_ptr);
	/*InternetGatewayDevice*/
	strcpy(fullPathName, token);
	token = strtok_r(NULL, ".", &tmp_ptr);

	while(token != NULL) {
		strcat(fullPathName, ".");

		len = strlen(token);
		for(c = 0; c < len; c++) {
			if(!isdigit(*(token + c)))
				break;
		}

		if(len == c) {
			strcat(fullPathName, "i");
			if(level < SUPPORTED_INDEX_LEVEL)
				objIid->idx[level] = atoi(token);

			level++;
		}
		else {
			strcat(fullPathName, token);
		}

		token = strtok_r(NULL, ".", &tmp_ptr);
	}

	if(level >= SUPPORTED_INDEX_LEVEL) {
		free(fullPathName);
		free(tmpName);
		printf("%s: Object index error\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	objIid->level = level;

	while(oid < schemaShmSize) {
		objInfo = GET_OBJ_INFO_BY_OID(oid);
		name = GET_OBJ_NAME(objInfo);

		if(strcmp(fullPathName, name) == 0) {
			find = true;
			break;
		}
		else {
			paramCount = objInfo->count;
			oid = oid + sizeof(objInfo_t) + paramCount*sizeof(parameterInfo_t);
		}
	}

	free(fullPathName);
	free(tmpName);

	if(!find) {
		zcfgLog(ZCFG_LOG_ERR, "Can't find OID by object name %s\n", objName);
		return ZCFG_NO_SUCH_OBJECT;
	}

	return oid;
}

int zcfgTr181ObjNameOid(char *objName)
{
	uint32_t oid = 0;
	objInfo_t *objInfo = NULL;
	char *name = NULL;
	char *fullPathName;
	bool found = false;
	uint16_t paramCount;

	if(!objName)
		return ZCFG_INTERNAL_ERROR;

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	if(objName == NULL || strlen(objName) == 0) {
		zcfgLog(ZCFG_LOG_ERR, "Object Name is NULL\n");
		return ZCFG_NO_SUCH_OBJECT;
	}

	fullPathName = (char *)malloc(strlen(objName)+8);
	if (!strncmp(objName, "Device.", strlen("Device.")) || !strcmp(objName, "Device")){
		strcpy(fullPathName, objName);
	}
	else{
		sprintf(fullPathName, "%s.%s", "Device", objName);
	}

	while(oid < schemaShmSize) {
		objInfo = GET_OBJ_INFO_BY_OID(oid);
		name = GET_OBJ_NAME(objInfo);

		if(strcmp(fullPathName, name) == 0) {
			found = true;
			break;
		}
		else {
			paramCount = objInfo->count;
			oid = oid + sizeof(objInfo_t) + paramCount*sizeof(parameterInfo_t);
		}
	}

	free(fullPathName);

	if(!found) {
		zcfgLog(ZCFG_LOG_ERR, "Can't find OID by object name %s\n", objName);
		return ZCFG_NO_SUCH_OBJECT;
	}

	return oid;
}

const char *zcfgFeTr181OidToName(zcfg_offset_t oid)
{
	objInfo_t *objInfo;
	int nameStrLen = 0;


	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	if(oid > schemaShmSize)
		return NULL;

	objInfo = GET_OBJ_INFO_BY_OID(oid);
	const char *name = (const char *)GET_OBJ_NAME(objInfo);
	if((nameStrLen = strlen(name)) >= MAX_TR181_PATHNAME || nameStrLen == 0){
		zcfgLog(ZCFG_LOG_INFO, "%s: no object\n", __FUNCTION__);
		return NULL;
	}

	return name;
}

zcfgRet_t zcfgFeObjIdToName(zcfg_offset_t oid, objIndex_t *objIid, char *objName)
{
	objInfo_t *objInfo = NULL;
	const char *name = NULL;
	char pathName[MAX_TR181_PATHNAME] = {0};
	char instNumStr[5] = {0};
	int n = 0, level = 0, objNameLen = 0;
	const char *pos = NULL;

	if(oid > schemaShmSize)
		return ZCFG_NO_SUCH_OBJECT;
 	else if(!objIid || !objName)
		return ZCFG_INTERNAL_ERROR;

	objInfo = GET_OBJ_INFO_BY_OID(oid);
	name = GET_OBJ_NAME(objInfo);
	if(strlen(name) >= MAX_TR181_PATHNAME)
		return ZCFG_INTERNAL_ERROR;

	strcpy(pathName, name);

	pos = (pathName + n);
	while(*pos != '\0') {
		if(*pos == 'i' && (n != 0) && *(pos - 1) == '.' && (*(pos + 1) == '.' || *(pos + 1) == '\0')) {
			sprintf(instNumStr, "%d", objIid->idx[level++]);
			strcat(objName, instNumStr);
			objNameLen += strlen(instNumStr);
		}
		else {
			*(objName + (objNameLen++)) = *pos;
		}
		pos = (pathName + (++n));
	}
	*(objName + objNameLen) = '\0';

	return ZCFG_SUCCESS;
}

/*
 *  Function Name: zcfgFeParamValGet 
 *  Description: Get a parameter value from the json object get from zcmd.
 *  Parameters: value: the json object contains data values and is get from zcmd.
 *
 *  Return value : 1. NULL : the parameter doesn't exist in json object
 *                 2. All the return values are string format
 */
char* zcfgFeParamValGet(struct json_object *value, char *parameterName)
{
	struct json_object *paramVal = NULL;

	paramVal = json_object_object_get(value, parameterName);
	if(paramVal == NULL)
		return NULL;

	switch(json_object_get_type(paramVal)) {
		case json_type_boolean:
			if(json_object_get_boolean(paramVal))
				return "1";
			else
				return "0";
		case json_type_string:
			return (char *)json_object_get_string(paramVal);
		case json_type_int:
			return (char *)json_object_to_json_string(paramVal);
		default:
			return NULL;
	}
}
/*
 *  Function Name: zcfgFeParamChk 
 *  Description: Check the type of parameter to be set is valid or not
 *
 *  Return value : 1. ZCFG_SUCCESS
 *                 2. ZCFG_ERROR_PARAMETER_TYPE : the parameter type is difference from the definition in schema
 *                 3. ZCFG_NO_SUCH_PARAMETER : the parameter name to set can not be found in schema
 *                 4. ZCFG_SET_READ_ONLY_PARAM : set a read-only parameter
 *		   5. ZCFG_INVALID_PARAM_VALUE_LENGTH : the value length is invalid
 */
static zcfgRet_t zcfgFeParamChk(zcfg_offset_t oid, char *parameterName, uint32_t type, char *value)
{
	objInfo_t *objInfo = NULL;
	int strLen = 0;
	char *p = NULL;
	int index = 0;
	
	objInfo = GET_OBJ_INFO_BY_OID(oid);

	GET_PAMAMETERS_IN_OBJECT(objInfo, paramInfo, paramName) {
		if(strcmp(paramName, parameterName) == 0) {
			if(paramInfo->attr & PARAMETER_ATTR_READONLY) {
				zcfgLog(ZCFG_LOG_ERR, "Can not set a read-only parameter %s\n", paramName);
				return ZCFG_SET_READ_ONLY_PARAM;
			}

			if(type != paramInfo->type) {
				if( type == json_type_string && paramInfo->type==json_type_hex){
					/*pass*/
				}
				else if( (type == json_type_uint32 && paramInfo->type == json_type_int) ||
						(type == json_type_int && paramInfo->type == json_type_uint32) ){
					zcfgLog(ZCFG_LOG_INFO, "%s: Int type not sync\n", __FUNCTION__);
				}
				else{
					zcfgLog(ZCFG_LOG_ERR, "Invalid type %d for parameter %s, the correct type should be %d\n", type, parameterName, paramInfo->type);
					return ZCFG_ERROR_PARAMETER_TYPE;
				}
			}

			if(type == json_type_string || type == json_type_time ||
				type == json_type_base64 || type == json_type_hex ) {
				strLen = strlen(value);
				if(strLen >= paramInfo->len) {
					zcfgLog(ZCFG_LOG_ERR, "The value length of %s is %d greater than %d defined in schema\n", parameterName, strLen, paramInfo->len);
					return ZCFG_INVALID_PARAM_VALUE_LENGTH;
				}
			}

			/*validate formate*/
			switch(type){
				case json_type_int:
					p = value;
					if(atol(value)<0){
						if(*p != '-'){
							return ZCFG_INVALID_PARAM_VALUE;
						}
						p++;
					}
					while(*p!='\0'){
						if(!isdigit(*p)){
							return ZCFG_INVALID_PARAM_VALUE;
						}
						p++;
					}
					break;
				case json_type_uint8:
				case json_type_uint16:
				case json_type_uint32:
				case json_type_ulong:
					p = value;
					while(*p!='\0'){
						if(!isdigit(*p)){
							return ZCFG_INVALID_PARAM_VALUE;
						}
						p++;
					}
					break;
				case json_type_boolean:
					if(strcasecmp(value, "true")==0 ){
						strcpy(value, "1");
						break;
					}
					else if(strcasecmp(value, "false")==0 ){
						strcpy(value, "0");
						break;
					}

					if( (strcmp(value, "0")!=0) && (strcmp(value, "1")!=0)){
						return ZCFG_INVALID_PARAM_VALUE;
					}
					break;
			
				case json_type_time:/*0001-01-01T00:00:00Z*/
					strLen = strlen(value);
					if(strLen != 19 && strLen != 20){
						return ZCFG_INVALID_PARAM_VALUE;
					}
					p = value;
					for(index = 0; index<strLen; index++){
						if(index==4 || index==7){
							if(!(*p == '-')){
								return ZCFG_INVALID_PARAM_VALUE;
							}
						}
						else if(index == 10){
							if(!(*p == 'T')){
								return ZCFG_INVALID_PARAM_VALUE;
							}
						}
						else if(index == 13 || index ==16){
							if(!(*p == ':')){
								return ZCFG_INVALID_PARAM_VALUE;
							}
						}
						else if(index == 19){
							if(!(*p == 'Z')){
								return ZCFG_INVALID_PARAM_VALUE;
							}
						}
						else{
							if(!isdigit(*p)){
								return ZCFG_INVALID_PARAM_VALUE;
							}
						}	
						p++;
					}
					break;
				case json_type_hex:
					strLen = strlen(value);
					p = value;
					for(index = 0; index<strLen; index++){
						if(!isdigit(*p) && (!((*p)>='a' && (*p)<='f')) && (!((*p)>='A' && (*p)<='F')))
							return ZCFG_INVALID_PARAM_VALUE;
						p++;
					}
					break;
				case json_type_base64: /*[A-Z,a-z,0-9, + /] ,if rest length is less than 4, fill of '=' character. */
					strLen = strlen(value);
					if( strLen%4 !=0) return ZCFG_INVALID_PARAM_VALUE;

					p = value;
					for(index = 0; index<strLen; index++){
						if(!isdigit(*p) && !isalpha(*p) && (*p !='+') && (*p != '/')){
							if((*p == '=') && (index >= strLen-2)){
								break;
							}
							else
								return ZCFG_INVALID_PARAM_VALUE;
						}
						p++;
					}
					// check end of =
					strLen -= index;
					for(index = 0; index<strLen; index++){
						if(*p != '=')
							return ZCFG_INVALID_PARAM_VALUE;
						p++;
					}
					break;
				default:
					break;
			}
			
			return ZCFG_SUCCESS;
		}
	}

	zcfgLog(ZCFG_LOG_ERR, "There is no parameter '%s' in schema\n", parameterName);
	return ZCFG_NO_SUCH_PARAMETER;
}

zcfgRet_t zcfgFeObjJsonQueryExist(zcfg_offset_t oid, objIndex_t *objIid)
{
	zcfgRet_t rst;
	char *result;

	if((rst = zcfgFeObjRead(oid, objIid, REQUEST_QUERY_OBJ_EXIST, &result)) == ZCFG_SUCCESS){
		ZOS_FREE(result);
	}

	return rst;
}

#ifdef ZCFG_LOG_USE_DEBUGCFG
struct json_object *zcfgFeDebugCfgRetrieve(const char *name, objIndex_t *debugCfgObjIid)
{
	struct json_object *obj = NULL;
	objIndex_t objIid;
	zcfgMsg_t debugCfgMsg;


	if(!name || !strlen(name) || !debugCfgObjIid)
		return obj;

	if(!strcmp(name, "zcmd")){
		return NULL;
	}

	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_LOG_CATEGORY, &objIid, &obj) == ZCFG_SUCCESS){
		const char *objName = json_object_get_string(json_object_object_get(obj, "Name"));
		if(objName && !strcmp(objName, name)){
			memcpy(debugCfgObjIid, &objIid, sizeof(objIndex_t));
			return obj;
		}

		json_object_put(obj);
		obj = NULL;
	}

	return obj;
}

zcfgRet_t zcfgFeDebugCfgObjInit(const char *name)
{
	struct json_object *debugCfgObj = NULL, *rdmLogSetObj;
	objIndex_t logSetObjIid, debugCfgObjIid;
	bool myDebugCfgEnable, rdmLogSetEnable, rdmLogSetZcfgEnable;

	IID_INIT(logSetObjIid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_LOG_SETTING, &logSetObjIid, &rdmLogSetObj) == ZCFG_SUCCESS){
		rdmLogSetEnable = json_object_get_boolean(json_object_object_get(rdmLogSetObj, "Enable"));
		rdmLogSetZcfgEnable = json_object_get_boolean(json_object_object_get(rdmLogSetObj, "ZCFG_LOG_Enable"));
		json_object_put(rdmLogSetObj);
	}else{
		return ZCFG_EMPTY_OBJECT;
	}

	debugCfgObj = zcfgFeDebugCfgRetrieve(name, &debugCfgObjIid);
	if(debugCfgObj){
		if(myDebugCfgObj)
			json_object_put(myDebugCfgObj);

		myDebugCfgObj = debugCfgObj;
		myDebugCfgEnable = json_object_get_boolean(json_object_object_get(debugCfgObj, "Enable"));
		if((debugCfgObjIid.idx[0] == 3 && rdmLogSetEnable && rdmLogSetZcfgEnable) ||
				(debugCfgObjIid.idx[0] != 3 && rdmLogSetEnable)){
				myDebugCfgObjLevel = (myDebugCfgEnable == true) ? json_object_get_int(json_object_object_get(debugCfgObj, "Level")) : 0;
		}
		printf("%s: utility %s\n", __FUNCTION__, name);
		return ZCFG_SUCCESS;
	}else
		return ZCFG_EMPTY_OBJECT;
}
#else
struct json_object *zcfgFeDebugCfgRetrieve(const char *name, objIndex_t *debugCfgObjIid)
{
	return NULL;
}

zcfgRet_t zcfgFeDebugCfgObjInit(const char *name)
{
	return ZCFG_EMPTY_OBJECT;
}
#endif

/*
 *  Function Name: zcfgFeJsonObjParamSet
 *  Description: Set the parameter value in a json object
 *  Parameters: json : a json object which contains data values 
 *
 *  Return value : 1. ZCFG_SUCCESS
 *                 2. ZCFG_INTERNAL_ERROR
 *                 3. ZCFG_ERROR_PARAMETER_TYPE : the parameter type is difference from the definition in schema
 *                 4. ZCFG_NO_SUCH_PARAMETER : the parameter name cannot be found in schema
 *                 5. ZCFG_SET_READ_ONLY_PARAM : set a read-only parameter
 *                 6. ZCFG_INVALID_PARAM_VALUE_LENGTH : the value length is invalid
 */
zcfgRet_t zcfgFeJsonObjParamSet(zcfg_offset_t oid, char *paramName, uint32_t type, struct json_object *setJobj, char *value)
{
	zcfgRet_t ret;
	struct json_object *setValue = NULL;
	
	zcfgLog(ZCFG_LOG_DEBUG, "Parameter Name %s %s\n", paramName, value);

	if(setJobj == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "The setJobj pointer is NULL value\n");
		return ZCFG_INTERNAL_ERROR;
	}

	ret = zcfgFeParamChk(oid, paramName, type, value);

	if(ret != ZCFG_SUCCESS) 
		return ret;

	switch (type) {
		case json_type_boolean:
			setValue = json_object_new_boolean(atoi(value));
			break;
		case json_type_int:
			setValue = json_object_new_int(atoi(value));
			break;
		case json_type_uint8:
		case json_type_uint16:
		case json_type_uint32:
		case json_type_ulong:
			setValue = json_object_new_int(strtoul(value, NULL, 10));
			break;
		case json_type_string:
		case json_type_hex:
		case json_type_base64:
		case json_type_time:
			setValue = json_object_new_string(value);
			json_object_set_type(setValue, type);
			break;
		default:
			zcfgLog(ZCFG_LOG_ERR, "Type not support\n");
			return ZCFG_ERROR_PARAMETER_TYPE;
			break;
	}

	json_object_object_add(setJobj, paramName, setValue);
	return ZCFG_SUCCESS;
}
/*
 *  Function Name: zcfgFeJsonMultiObjAppend 
 *  Description: append a single json onject to multiple json object with following format
 *  Parameters: multiJobj : a json object which contains multiple data objects to be set
 *   
 *  Multiple object format: {
 *                            "OID_1" : { "objIid_1_1" : {set_object1_1},
 *                                        "objIid_1_2" : {set_object1_2},                     
 *                                                     .
 *                                                     .
 *                                      },
 *                                      .
 *                                      .
 *                          }
 */
json_object* zcfgFeJsonMultiObjAppend(zcfg_offset_t oid, objIndex_t *objIid, json_object *multiJobj, json_object *jobj)
{
	struct json_object *newOidJobj = NULL;
	struct json_object *oidJobj = NULL;
	struct json_object *iidJobj = NULL;
	struct json_object *dupJobj = NULL;
	char objIndexID[32];
	char objID[16];

	if(multiJobj == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "The multiJobj pointer is NULL value\n");
		return NULL;
	}

	sprintf(objIndexID, "%d.%d.%d.%d.%d.%d.%d", objIid->level, objIid->idx[0], objIid->idx[1], objIid->idx[2], objIid->idx[3], objIid->idx[4], objIid->idx[5]);
	sprintf(objID, "%d", oid);

	/*Check if there is already a same OID object exist or not*/
	oidJobj = json_object_object_get(multiJobj, objID);
	if(oidJobj != NULL) {
		/*Get the object to be set by iid*/
		iidJobj = json_object_object_get(oidJobj, objIndexID);
		if(iidJobj != NULL) {
			return iidJobj;
		}
		else {
			/*Duplicate the json object that is going to be set*/
			dupJobj = json_tokener_parse(json_object_to_json_string(jobj));
			/*Add the new data object to be set to multiple json object*/
			json_object_object_add(oidJobj, objIndexID, dupJobj);
			return dupJobj;
		}
	}
	else {
		newOidJobj = json_object_new_object();
		json_object_object_add(multiJobj, objID, newOidJobj);
		/*Duplicate the json object is going to be set*/
		dupJobj = json_tokener_parse(json_object_to_json_string(jobj));
		/*Add the new data object to be set to multiple json object*/
		json_object_object_add(newOidJobj, objIndexID, dupJobj);
		return dupJobj;
	}
}

/*
 * Parameters: objIid: obj index to retrieve
 *             rtdIid: retrieved obj index while no objIid
 *             rtdObj: retrieved obj
 */
zcfgRet_t zcfgFeJsonMultiObjRetrieve(struct json_object *multiJobj, zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *rtdIid, struct json_object **rtdObj)
{
	char identity[20] = {0};
	char index[36] = {0};
	struct json_object *oldObj;

	if(!multiJobj || !rtdObj)
		return ZCFG_INTERNAL_ERROR;

	*rtdObj = NULL;

	sprintf(identity, "%d", oid);

	if(objIid){
		sprintf(index, "%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu", objIid->level, objIid->idx[0], objIid->idx[1], objIid->idx[2],
				objIid->idx[3], objIid->idx[4], objIid->idx[5]);
	}

	oldObj = json_object_object_get(multiJobj, identity);
	if(oldObj){
		if(objIid){
			*rtdObj = json_object_object_get(oldObj, index);
			return (*rtdObj) ? ZCFG_SUCCESS : ZCFG_NO_SUCH_OBJECT;
		}
	}else
		return ZCFG_NO_SUCH_OBJECT;

	struct lh_entry *entry = (json_object_get_object(oldObj))->head;
	*rtdObj = entry ? (struct json_object *)entry->v : 0;
	if(*rtdObj && rtdIid){
		const char *entryIdxStr = (const char *)entry->k;
		sscanf(entryIdxStr, "%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu", &rtdIid->level, &rtdIid->idx[0], &rtdIid->idx[1], &rtdIid->idx[2],
				&rtdIid->idx[3], &rtdIid->idx[4], &rtdIid->idx[5]);
	}

	return (*rtdObj) ? ZCFG_SUCCESS : ZCFG_NO_SUCH_OBJECT;
}

zcfgRet_t zcfgFeJsonMultiObjAddParam(struct json_object *multiobj, zcfg_offset_t oid, objIndex_t *objIid, const char *parmName, struct json_object *paramObj)
{
	struct json_object *oldObj = NULL, *indexObj = NULL;
	char identity[20] = {0};
	char index[36] = {0};
	char paramStr[200] = {0};

	//if(multiobj oid objIid paramobj)

	sprintf(identity, "%d", oid);
	sprintf(index, "%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu", objIid->level, objIid->idx[0], objIid->idx[1], objIid->idx[2],
				objIid->idx[3], objIid->idx[4], objIid->idx[5]);

	oldObj = json_object_object_get(multiobj, identity);
	if(oldObj)
		indexObj = json_object_object_get(oldObj, index);

	if(indexObj && parmName && strlen(parmName)){
		sprintf(paramStr, "%s_%s", index, parmName);
		json_object_object_add(oldObj, paramStr, paramObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeJsonMultiObjRetrieveParam(struct json_object *multiobj, zcfg_offset_t oid, objIndex_t *objIid, const char *parmName, struct json_object **paramObj)
{
	struct json_object *oldObj = NULL, *indexObj = NULL;
	char identity[20] = {0};
	char index[36] = {0};
	char paramStr[200] = {0};

	//if(multiobj oid objIid paramobj)

	sprintf(identity, "%d", oid);
	sprintf(index, "%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu", objIid->level, objIid->idx[0], objIid->idx[1], objIid->idx[2],
				objIid->idx[3], objIid->idx[4], objIid->idx[5]);

	oldObj = json_object_object_get(multiobj, identity);
	if(oldObj)
		indexObj = json_object_object_get(oldObj, index);

	if(indexObj && parmName && strlen(parmName) && paramObj){
		sprintf(paramStr, "%s_%s", index, parmName);
		*paramObj = json_object_object_get(oldObj, paramStr);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeJsonMultiObjAddObjParam(struct json_object *multiobj, zcfg_offset_t oid, objIndex_t *objIid, const char *parmName, struct json_object *paramObj)
{
	struct json_object *indexObj = NULL;

	zcfgFeJsonMultiObjRetrieve(multiobj, oid, objIid, NULL, &indexObj);
	if(indexObj && parmName && strlen(parmName) && paramObj){
		json_object_object_add(indexObj, parmName, paramObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeJsonMultiObjRetrieveObjParam(struct json_object *multiobj, zcfg_offset_t oid, objIndex_t *objIid, const char *parmName, struct json_object **paramObj)
{
	struct json_object *indexObj = NULL;

	zcfgFeJsonMultiObjRetrieve(multiobj, oid, objIid, NULL, &indexObj);
	if(indexObj && parmName && strlen(parmName) && paramObj){
		*paramObj = json_object_object_get(indexObj, parmName);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeObjJsonGetNext(zcfg_offset_t oid, objIndex_t *objIid, struct json_object **value)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_NEXT_OBJ, &result);

	if(ret != ZCFG_INTERNAL_ERROR && ret != ZCFG_NO_MORE_INSTANCE) {
		if(result){
			*value = json_tokener_parse(result);
			ZOS_FREE(result);
		}
	}

	return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeObjJsonGetNext(zcfg_offset_t oid, objIndex_t *objIid, struct json_object **value, const char *fileName, int line)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_NEXT_OBJ, &result);

	if(ret != ZCFG_INTERNAL_ERROR && ret != ZCFG_NO_MORE_INSTANCE) {
		if(result){
			*value = _json_tokener_parse(result, fileName, line);
			ZOS_FREE(result);
		}
	}

	return ret;
}
#endif

zcfgRet_t zcfgFeObjJsonGetNextWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, struct json_object **value)
{
        zcfgRet_t ret;
        char *result = NULL;

        ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_NEXT_OBJ_WITHOUT_UPDATE, &result);

		if(ret != ZCFG_INTERNAL_ERROR && ret != ZCFG_NO_MORE_INSTANCE) {
			if(result){
				*value = json_tokener_parse(result);
				ZOS_FREE(result);
			}
        }

        return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeObjJsonGetNextWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, struct json_object **value, const char *fileName, int line)
{
        zcfgRet_t ret;
        char *result = NULL;

        ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_NEXT_OBJ_WITHOUT_UPDATE, &result);

		if(ret != ZCFG_INTERNAL_ERROR && ret != ZCFG_NO_MORE_INSTANCE) {
			if(result){
				*value = _json_tokener_parse(result, fileName, line);
				ZOS_FREE(result);
			}
        }

        return ret;
}
#endif

zcfgRet_t zcfgFeObjJsonGet(zcfg_offset_t oid, objIndex_t *objIid, struct json_object **value)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_OBJ, &result);

	if(ret == ZCFG_SUCCESS) {
		if(result){
			*value = json_tokener_parse(result);
			ZOS_FREE(result);
		}
	}

	return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeObjJsonGet(zcfg_offset_t oid, objIndex_t *objIid, struct json_object **value, const char *fileName, int line)
{
	zcfgRet_t ret;
	char *result = NULL;

	ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_OBJ, &result);

	if(ret == ZCFG_SUCCESS) {
		if(result){
			*value = _json_tokener_parse(result, fileName, line);
			ZOS_FREE(result);
		}
	}

	return ret;
}
#endif

/* hilo : 0 get higherlayer by lowerlayer
   hilo : 1 get lowerlayer by higherlayer
*/
zcfgRet_t zcfgFeIfStackObjJsonGet(char *layer, int hilo, struct json_object **value)
{
	zcfgRet_t ret;

	ret = zcfgFeIfStackObjReadByLayer(layer, hilo, (char **)value);

	return ret;
}


zcfgRet_t zcfgFeWholeObjJsonGet(char *tr181path, struct json_object **value)

{
	zcfgRet_t ret;
	char *result = NULL;
	
	ret = zcfgFeWholeObjReadByName(tr181path, REQUEST_GET_WHOLE_OBJ_BY_NAME, &result);

	if(ret == ZCFG_SUCCESS) {
		if(result){
			*value = json_tokener_parse(result);
			ZOS_FREE(result);
			result = NULL;			
		}
	}

	return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeWholeObjJsonGet(char *tr181path, struct json_object **value, const char *fileName, int line)

{
	zcfgRet_t ret;
	char *result = NULL;
	
	ret = zcfgFeWholeObjReadByName(tr181path, REQUEST_GET_WHOLE_OBJ_BY_NAME, &result);

	if(ret == ZCFG_SUCCESS) {
		if(result){
			*value = _json_tokener_parse(result, fileName, line);
			ZOS_FREE(result);
			result = NULL;			
		}
	}

	return ret;
}
#endif

zcfgRet_t zcfgFeObjJsonGetWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, struct json_object **value)
{
        zcfgRet_t ret;
        char *result = NULL;

        ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_OBJ_WITHOUT_UPDATE, &result);

        if(ret == ZCFG_SUCCESS) {
			if(result){	
				*value = json_tokener_parse(result);
                ZOS_FREE(result);
			}
        }

        return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeObjJsonGetWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, struct json_object **value, const char *fileName, int line)
{
        zcfgRet_t ret;
        char *result = NULL;

        ret = zcfgFeObjRead(oid, objIid, REQUEST_GET_OBJ_WITHOUT_UPDATE, &result);

        if(ret == ZCFG_SUCCESS) {
			if(result){	
				*value = _json_tokener_parse(result, fileName, line);
                ZOS_FREE(result);
			}
        }

        return ret;
}
#endif

zcfgRet_t zcfgFeSubInObjJsonGetNext(zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *insIid, struct json_object **value)
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
			if(result){
                /*Translate json string to user request structure*/
                *value = json_tokener_parse(result);
                ZOS_FREE(result);
			}
        }

        return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeSubInObjJsonGetNext(zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *insIid, struct json_object **value, const char *fileName, int line)
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
			if(result){
                /*Translate json string to user request structure*/
                *value = _json_tokener_parse(result, fileName, line);
                ZOS_FREE(result);
			}
        }

        return ret;
}
#endif

zcfgRet_t zcfgFeSubInObjJsonGetNextWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *insIid, struct json_object **value)
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
			if(result){
                /*Translate json string to user request structure*/
                *value = json_tokener_parse(result);
                ZOS_FREE(result);
			}
        }

        return ret;
}

#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcfgFeSubInObjJsonGetNextWithoutUpdate(zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *insIid, struct json_object **value, const char *fileName, int line)
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
			if(result){
                /*Translate json string to user request structure*/
                *value = _json_tokener_parse(result, fileName, line);
                ZOS_FREE(result);
			}
        }

        return ret;
}
#endif

zcfgRet_t zcfgFeObjJsonSet(zcfg_offset_t oid, objIndex_t *objIid, struct json_object *jobj, char *fbkMsg)
{
	zcfgRet_t ret;
	char *value = NULL;

	value = (char *)json_object_to_json_string(jobj);

	if(value == NULL)
		return ZCFG_INTERNAL_ERROR;

	ret = zcfgFeObjWrite(oid, objIid, value, REQUEST_SET_OBJ, fbkMsg);
	return ret;
}

zcfgRet_t zcfgFeWholeObjJsonSet(char *tr181path, struct json_object *jobj, char *fbkMsg)
{
	zcfgRet_t ret;
	char *value = NULL;

	value = (char *)json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN);

	if(value == NULL)
		return ZCFG_INTERNAL_ERROR;

	ret =  zcfgFeWholeObjWriteByName(tr181path, value, REQUEST_SET_WHOLE_OBJ_BY_NAME, fbkMsg);
	return ret;
}

#if 1 /* non-blocking config apply, ZyXEL, John */
zcfgRet_t zcfgFeObjJsonBlockedSet(zcfg_offset_t oid, objIndex_t *objIid, struct json_object *jobj, char *fbkMsg)
{
        zcfgRet_t ret;
	char *value = NULL;

	value = (char *)json_object_to_json_string(jobj);

	if(value == NULL)
		return ZCFG_INTERNAL_ERROR;

	ret = zcfgFeObjWrite(oid, objIid, value, REQUEST_SET_OBJ | ZCFG_BLOCK_MODE_CONFIG_APPLY, fbkMsg);
        return ret;
}

#endif

zcfgRet_t zcfgFeObjJsonSetWithoutApply(zcfg_offset_t oid, objIndex_t *objIid, struct json_object *jobj, char *fbkMsg)
{
	zcfgRet_t ret;
	char *value = NULL;

	value = (char *)json_object_to_json_string(jobj);

	if(value == NULL)
		return ZCFG_INTERNAL_ERROR;

	ret = zcfgFeObjWrite(oid, objIid, value, REQUEST_SET_WITHOUT_APPLY, fbkMsg);
	return ret;
}


zcfgRet_t zcfgFeObjJsonSetWithAdditions(zcfg_offset_t oid, objIndex_t *objIid, struct json_object *setObj, char *fbkMsg, uint32_t op, struct json_object *additions)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	uint32_t bits = op;
	bool reqReleaseObj = false;
	struct json_object *setObjAdditions = NULL;

	if(!objIid)
		return ZCFG_INTERNAL_ERROR;

#ifndef ZCFG_RPC_REQUEST_ADDITIONS
	additions = NULL;
#endif

	if(!setObj && !additions)
		return ZCFG_INTERNAL_ERROR;

	uint32_t options = op & (ZCFG_NO_WAIT_REPLY | ZCFG_BLOCK_MODE_CONFIG_APPLY | ZCFG_PARTIAL_OBJECT_SET);
	op = (op & ~(ZCFG_NO_WAIT_REPLY | ZCFG_BLOCK_MODE_CONFIG_APPLY | ZCFG_PARTIAL_OBJECT_SET));
	if(op != REQUEST_SET_OBJ && op != REQUEST_SET_WITHOUT_APPLY)
		return ZCFG_INTERNAL_ERROR;

	if(additions){
		setObjAdditions = json_object_new_object();
		json_object_object_add(setObjAdditions, "rpcadditions", additions);
		if(setObj)
			json_object_object_add(setObjAdditions, "rpcsetobject", setObj);

		reqReleaseObj = true;
	}else {
		if(!setObj)
			return ZCFG_REQUEST_REJECT;
		setObjAdditions = setObj;
	}

	op |= options;

	const char *value = json_object_to_json_string(setObjAdditions);
	if(additions){
		char *replyStr[1] = {0};
		struct json_object *rpcAdditionsReply = NULL;
		op |= ZCFG_MSG_RPC_ADDITIONS;
		rst = zcfgFeObjWrite(oid, objIid, value, op, (char *)replyStr);
		json_object_object_pull(setObjAdditions, "rpcadditions");

		rpcAdditionsReply = replyStr[0] ? json_tokener_parse(replyStr[0]) : NULL;
		if(rpcAdditionsReply){
			struct json_object *rpcApplyReply = json_object_object_pull_and_empty(rpcAdditionsReply, "rpcapplyreply");
			const char *rpcApplyReplyStr = NULL;
			if(fbkMsg && rpcApplyReply && (rpcApplyReplyStr = json_object_get_string(rpcApplyReply))){
				ZOS_STRNCPY(fbkMsg, rpcApplyReplyStr, FEEDBACK_MSG_LENGTH);
			}
			json_object_put(rpcApplyReply);
			if(json_object_object_length(rpcAdditionsReply))
				json_object_object_add(additions, "rpcadditionsreply", rpcAdditionsReply);
		}
		if(replyStr[0]) free(replyStr[0]);
	}else{
		rst = zcfgFeObjWrite(oid, objIid, value, op, fbkMsg);
	}

	if(reqReleaseObj){
		json_object_object_pull(setObjAdditions, "rpcsetobject");
		json_object_put(setObjAdditions);
	}

	return rst;
}

zcfgRet_t zcfgFeMultiObjJsonSet(struct json_object *multiJobj, char *feedbackMsg)
{
	zcfgRet_t ret;
	char *value = NULL;
	objIndex_t objIid;

	value = (char *)json_object_to_json_string(multiJobj);

	if(value == NULL)
		return ZCFG_INTERNAL_ERROR;

	IID_INIT(objIid);
	ret = zcfgFeObjWrite(0, &objIid, value, REQUEST_SET_MULTI_OBJ, feedbackMsg);

	return ret;

}

// run valid routine only
zcfgRet_t zcfgFeMultiObjJsonRunValid(struct json_object *multiJobj, char *feedbackmsg)
{
	if(!multiJobj)
		return ZCFG_INTERNAL_ERROR;
	json_object_object_add(multiJobj, "runvalidonly", json_object_new_boolean(true));
	zcfgRet_t rst = zcfgFeMultiObjJsonSet(multiJobj, feedbackmsg);

	json_object_object_del(multiJobj, "runvalidonly");

	return rst;
}

#if 1 /* non-blocking config apply, ZyXEL, John */
zcfgRet_t zcfgFeMultiObjJsonBlockedSet(struct json_object *multiJobj, char *feedbackMsg)
{
	zcfgRet_t ret;
	char *value = NULL;
	objIndex_t objIid;

	value = (char *)json_object_to_json_string(multiJobj);

	if(value == NULL)
		return ZCFG_INTERNAL_ERROR;

	IID_INIT(objIid);
	ret = zcfgFeObjWrite(0, &objIid, value, REQUEST_SET_MULTI_OBJ | ZCFG_BLOCK_MODE_CONFIG_APPLY, feedbackMsg);

	return ret;
}
#endif

zcfgRet_t zcfgFeObjJsonAdd(zcfg_offset_t oid, objIndex_t *objIid, char *feedbackMsg)
{
	zcfgRet_t ret;

	ret = zcfgFeObjWrite(oid, objIid, NULL, REQUEST_ADD_INSTANCE, feedbackMsg);

	return ret;
}
#if 1 /* non-blocking config apply, ZyXEL, John */
zcfgRet_t zcfgFeObjJsonBlockedAdd(zcfg_offset_t oid, objIndex_t *objIid, char *feedbackMsg)
{
	zcfgRet_t ret;

	ret = zcfgFeObjWrite(oid, objIid, NULL, REQUEST_ADD_INSTANCE | ZCFG_BLOCK_MODE_CONFIG_APPLY, feedbackMsg);

	return ret;
}
#endif

/* It wouldn't delete rdm object on 'delay apply' until 're-apply', but run as 'zcfgFeObjJsonDel'
    while not utilizing 'delay apply' */
zcfgRet_t zcfgFeObjJsonDelayRdmDel(zcfg_offset_t oid, objIndex_t *objIid, char *feedbackMsg)
{
	zcfgRet_t ret;

	ret = zcfgFeObjWrite(oid, objIid, NULL, ZCFG_MSG_REQUEST_DELAY_DELETE, feedbackMsg);

	return ret;
}

zcfgRet_t zcfgFeObjJsonDel(zcfg_offset_t oid, objIndex_t *objIid, char *feedbackMsg)
{
	zcfgRet_t ret;

#if 1 /* non-blocking config apply, ZyXEL, John */
	/* deleted instanced won;t be update to rdm(RAM) until backend process is done, as the result, we better 
	    set action, DELETE, as blocked mode in multi-thread achitecture */
	ret = zcfgFeObjWrite(oid, objIid, NULL, REQUEST_DEL_INSTANCE | ZCFG_BLOCK_MODE_CONFIG_APPLY, feedbackMsg);
#else
	ret = zcfgFeObjWrite(oid, objIid, NULL, REQUEST_DEL_INSTANCE, feedbackMsg);
#endif
	return ret;
}

zcfgRet_t zcfgObjGetFromInputObj(zcfg_offset_t rdmObjId, objIndex_t* objIid, struct json_object **getObj, struct json_object *rdm)
{
	zcfgLog(ZCFG_LOG_DEBUG, "%s: Enter.\n", __FUNCTION__);
	zcfgRet_t ret;
	objInfo_t *objInfo = (objInfo_t *)(schemaShmAddr + rdmObjId);
	char *objName = GET_OBJ_NAME(objInfo);

	if((ret = zcfgObjGetByNameFromInputObj(objName, objIid, getObj, rdm)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_DEBUG, "fail to get object by full path name\n");
		*getObj = NULL;
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgObjGetByNameFromInputObj(char *rdmObjName, objIndex_t* objIid, struct json_object **getObj, struct json_object *rdm)
{
	zcfgLog(ZCFG_LOG_DEBUG, "%s: Enter.\n", __FUNCTION__);
	char *node = NULL;
	char objName[OBJ_NAME_BUFF_SIZE];
	char *delim = ".";
	uint8_t i = 0;
	uint8_t notAnyMatch = 1;
	int32_t entryNum;
	enum json_type nodeType;
	struct json_object *tmpObj = NULL;
	struct json_object *currObj;
	char *tmp = NULL;

	strcpy(objName, rdmObjName);

	/* the first will be InternetGatewayDevice */
	node = strtok_r(objName, delim, &tmp);
	currObj = rdm;

	while((node = strtok_r(NULL, delim, &tmp)) != NULL){
		if(strcmp(node, "i")){
			notAnyMatch = 1;
			json_object_object_foreach(currObj,key,val){
				if(!strcmp(key, node)){
					notAnyMatch = 0;
					nodeType = json_object_get_type(val);
					if(nodeType == json_type_array){
						if(i > objIid->level-1){
							zcfgLog(ZCFG_LOG_DEBUG, "get object fail !!\n");
							*getObj = NULL;
							return ZCFG_INTERNAL_ERROR;
						}

						entryNum = json_object_array_length(val);
						/* No such instance */
						if(objIid->idx[i] > entryNum){
							zcfgLog(ZCFG_LOG_DEBUG, "get object fail, the specified entry is not existed!!\n");
							currObj = NULL;
							return ZCFG_NO_MORE_INSTANCE;
						}

						tmpObj = json_object_array_get_idx(val, objIid->idx[i] - 1);
						i++;
						if(tmpObj) /* get a specified indexed entry object */
							currObj = tmpObj;
						else   /* get a index node object */
							currObj = val;
					}
					else{
						currObj = val;
					}
					break;
				}
			}

			if(notAnyMatch){
				zcfgLog(ZCFG_LOG_DEBUG, "No such instances !!\n");
				*getObj = NULL;
				return ZCFG_NO_SUCH_OBJECT;
			}
		}
	}

	*getObj = currObj;

	return ZCFG_SUCCESS;
}
