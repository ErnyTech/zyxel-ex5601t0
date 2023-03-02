#include <json/json.h>

#include "zcfg_common.h"
#include "zcmd_schema.h"
#include "zcmd_tool.h"
#include "zcfg_debug.h"
#include "zos_api.h"

extern void *paramNameShmAddr;

/*4-byte boundary*/
#define BOUNDARY 4

typedef union value_s {
	bool bool_val;
	int8_t int8_val;
	int16_t int16_val;
	int32_t int32_val;
	int64_t int64_val;
	uint8_t uint8_val;
	uint16_t uint16_val;
	uint32_t uint32_val;
	uint64_t ulong_val;
	char *str_val;
}value_t;

// remove this file data in leakage information.
#ifdef ZCFG_MEMORY_LEAK_DETECT
#undef json_object_new_object
#undef json_object_new_string
#endif

/*
 *  Function Name: zcmdStructToJString
 *  Description: This function is used to transform the requested structure into json string.
 *
 */
#ifdef ZCFG_MEMORY_LEAK_DETECT
void _zcmdStructToJString(void *rqst_struct, char **jstr, zcfg_offset_t rdmObjId, void *schemaShmAddr, const char *fileName, int line)
#else
void zcmdStructToJString(void *rqst_struct, char **jstr, zcfg_offset_t rdmObjId, void *schemaShmAddr)
#endif
{
	uint8_t type;
	uint32_t len;
	struct json_object *jobj = NULL;
	struct json_object *setValue = NULL;
	int pos = 0;
	int redundantSpace = 0;
	const char *requestStr = NULL;
	objInfo_t *objInfo;
	value_t v;

	//zcfgLog(ZCFG_LOG_DEBUG, "Enter function %s\n", __FUNCTION__);

	/* Get the object information in shared memory by using oid. */
	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);;

	jobj = json_object_new_object();
	/* Using the parameter information to transform the data structure 
	   to json string */
	GET_PAMAMETERS_IN_OBJECT(objInfo, paramInfo, paramName) {
		type = paramInfo->type;
		len = paramInfo->len;

		switch (type) {
			case json_type_boolean:
				//zcfgLog(ZCFG_LOG_DEBUG, "type_boolean\n");
				if(len > redundantSpace) {
					pos = pos + redundantSpace;
					redundantSpace = BOUNDARY - len;
				}
				else {
					redundantSpace = redundantSpace - len;
				}

				memcpy(&v.bool_val, rqst_struct+pos, len);
				pos = pos + len;

				//zcfgLog(ZCFG_LOG_DEBUG, "Key %s value %u\n", paramName, v.bool_val);
				setValue = json_object_new_boolean(v.bool_val);
				json_object_object_add(jobj, paramName, setValue);
				break;

			case json_type_int:
				if( len > redundantSpace ) {
					pos = pos + redundantSpace;
					redundantSpace = (len > BOUNDARY) ? (BOUNDARY*2)-len : BOUNDARY-len;
				}
				else {
					redundantSpace = redundantSpace - len;
				}

				if(len == 8) {
#ifdef CONFIG_TARGET_64 //In 64-bits OS, BOUNDARY for uint64_t/int64_t should be 8.
					if(pos % 8 != 0){ //(pos % 8) is 0 or 4 here, if it is 4, padding 4 bytes
						pos = pos + BOUNDARY;
					}
#endif
					memcpy(&v.int64_val, rqst_struct+pos, len);
					setValue = json_object_new_int(v.int64_val);
				}
				else if(len == 4) {
					memcpy(&v.int32_val, rqst_struct+pos, len);
					setValue = json_object_new_int(v.int32_val);
				}
				else if(len == 2) {
					memcpy(&v.int16_val, rqst_struct+pos, len);
					setValue = json_object_new_int(v.int16_val);
				}
				else {
					memcpy(&v.int8_val, rqst_struct+pos, len);
					setValue = json_object_new_int((int)v.int8_val);
				}

				pos = pos + len;
				json_object_object_add(jobj, paramName, setValue);
				break;
			case json_type_ulong:
			case json_type_uint32:
			case json_type_uint16:
			case json_type_uint8:
				if( len > redundantSpace ) {
					pos = pos + redundantSpace;
					redundantSpace = (len > BOUNDARY) ? (BOUNDARY*2)-len : BOUNDARY-len;
				}
				else {
					redundantSpace = redundantSpace - len;
				}

				if(len == 8) {
#ifdef CONFIG_TARGET_64 //In 64-bits OS, BOUNDARY for uint64_t/int64_t should be 8.
					if(pos % 8 != 0){ //(pos % 8) is 0 or 4 here, if it is 4, padding 4 bytes
						pos = pos + BOUNDARY;
					}
#endif
					memcpy(&v.ulong_val, rqst_struct+pos, len);
					setValue = json_object_new_int(v.ulong_val);
				}
				else if(len == 4) {
					memcpy(&v.uint32_val, rqst_struct+pos, len);
					setValue = json_object_new_int(v.uint32_val);
				}
				else if(len == 2) {
					memcpy(&v.uint16_val, rqst_struct+pos, len);
					setValue = json_object_new_int(v.uint16_val);
				}
				else {
					memcpy(&v.uint8_val, rqst_struct+pos, len);
					setValue = json_object_new_int((unsigned int)v.uint8_val);
				}

				pos = pos + len;
				json_object_object_add(jobj, paramName, setValue);
				break;
			case json_type_string:
			case json_type_time:
			case json_type_base64:
			case json_type_hex:
				//zcfgLog(ZCFG_LOG_DEBUG, "type_string\n");
				v.str_val = (char *)ZOS_MALLOC(len+1);
				if(v.str_val == NULL) {
					printf("%s : malloc fail\n", __FUNCTION__);
					json_object_put(jobj);
					return;
				}
				memcpy(v.str_val, rqst_struct+pos, len);
				pos = pos + len;
				if(len > redundantSpace) {
					len = len - redundantSpace;
					if((len % BOUNDARY) != 0)
						redundantSpace = BOUNDARY - (len % BOUNDARY);
					else
						redundantSpace = 0;
				}
				else {
					redundantSpace = redundantSpace - len;
				}

				//zcfgLog(ZCFG_LOG_DEBUG, "Key %s value %s\n", paramName, v.str_val);
				setValue = json_object_new_string(v.str_val);
				json_object_object_add(jobj, paramName, setValue);
				ZOS_FREE(v.str_val);
				break;
			default :
				zcfgLog(ZCFG_LOG_DEBUG, "Default type %d\n", type);
				break;
		}
	}

	requestStr = json_object_to_json_string(jobj);
	*jstr = (char *)ZOS_MALLOC(strlen(requestStr)+1);
#ifdef ZCFG_MEMORY_LEAK_DETECT
	recordZcfgAllocData(*jstr, fileName, line, 0);
#endif

	strcpy(*jstr, requestStr);

	json_object_put(jobj);


}
/*
 *  Function Name: zcmdJStringToStruct
 *  Description: Used to parse json string to fill up the object structure
 *
 */
#ifdef ZCFG_MEMORY_LEAK_DETECT
zcfgRet_t _zcmdJStringToStruct(void **objStruct, const char *jstr, zcfg_offset_t rdmObjId, void *schemaShmAddr, const char *fileName, int line)
#else
zcfgRet_t zcmdJStringToStruct(void **objStruct, const char *jstr, zcfg_offset_t rdmObjId, void *schemaShmAddr)
#endif
{
	uint8_t type;
	uint32_t len;
	struct json_object *jobj = NULL;
	struct json_object *tmpJobj = NULL;
	int pos = 0;
	int redundantSpace = 0;
	objInfo_t *objInfo;
	value_t v;

	//zcfgLog(ZCFG_LOG_DEBUG, "Enter function %s\n", __FUNCTION__);
	if(!objStruct || !jstr)
		return ZCFG_INTERNAL_ERROR;
	/* Get the object information in shared memory by using oid. */	
	objInfo = GET_OBJ_INFO_BY_OID(rdmObjId);;

	*objStruct = (void *)ZOS_MALLOC(objInfo->size);
	if(*objStruct == NULL){
		zcfgLog(ZCFG_LOG_ERR, "Malloc fail in %s\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	memset(*objStruct, 0, objInfo->size);

	/* With the information of each parameter, we can fill up data structure
           from json string. */
	jobj = json_tokener_parse(jstr);
	if(is_error(jobj)){
		ZOS_FREE(*objStruct);
		return ZCFG_INTERNAL_ERROR;
	}

	GET_PAMAMETERS_IN_OBJECT(objInfo, paramInfo, paramName) {
		type = paramInfo->type;
		len = paramInfo->len;
		tmpJobj = json_object_object_get(jobj, paramName);

		if(tmpJobj == NULL) {
			zcfgLog(ZCFG_LOG_ERR, "Can't get value for parameter %s\n", paramName);
			ZOS_FREE(*objStruct);
			json_object_put(jobj);
			return ZCFG_INTERNAL_ERROR;
		}

		switch (type) {
			case json_type_boolean: 
				//zcfgLog(ZCFG_LOG_DEBUG, "json_type_boolean\n");
				//zcfgLog(ZCFG_LOG_DEBUG, "Name %s length %d\n", paramName, len);
				v.bool_val = json_object_get_boolean(tmpJobj);

				if( len > redundantSpace ) {
					pos = pos + redundantSpace;
					redundantSpace = BOUNDARY - len;
				}
				else {
					redundantSpace = redundantSpace - len;
				}

				memcpy(*objStruct+pos, &v.bool_val, sizeof(bool));

				pos = pos + len;
				break;
			case json_type_ulong:
			case json_type_int:
			case json_type_uint32:
			case json_type_uint16:
			case json_type_uint8:
				//zcfgLog(ZCFG_LOG_DEBUG, "json_type_int\n");
				//zcfgLog(ZCFG_LOG_DEBUG, "Name %s length %d\n", paramName, len);
				if( len > redundantSpace ) {
					pos = pos + redundantSpace;
					redundantSpace = (len > BOUNDARY) ? (BOUNDARY*2)-len : BOUNDARY-len;
				}
				else {
					redundantSpace = redundantSpace - len;
				}

				if(len == 8) {
#ifdef CONFIG_TARGET_64 //In 64-bits OS, BOUNDARY for uint64_t/int64_t should be 8.
					if(pos % 8 != 0){ //(pos % 8) is 0 or 4 here, if it is 4, padding 4 bytes
						pos = pos + BOUNDARY;
					}
#endif
					v.ulong_val = json_object_get_int(tmpJobj);
					memcpy(*objStruct+pos, &v.ulong_val, sizeof(uint64_t));
				}
				else if(len == 4) {
					v.uint32_val = json_object_get_int(tmpJobj);
					memcpy(*objStruct+pos, &v.uint32_val, sizeof(int));
				}
				else if(len == 2) {
					v.uint16_val = json_object_get_int(tmpJobj);
					memcpy(*objStruct+pos, &v.uint16_val, sizeof(unsigned short));
				}
				else {
					v.uint8_val = (unsigned char)json_object_get_int(tmpJobj);
					memcpy(*objStruct+pos, &v.uint8_val, sizeof(unsigned char));
				}

				pos = pos + len;
				break;
			case json_type_string:
			case json_type_time:
			case json_type_base64:
			case json_type_hex:
				//zcfgLog(ZCFG_LOG_DEBUG, "json_type_string\n");
				//zcfgLog(ZCFG_LOG_DEBUG, "Name %s length %d\n", paramName, len);
                if(strlen(json_object_get_string(tmpJobj)) <= len){
                    v.str_val = (char *)ZOS_MALLOC(len+1);
                    ZOS_STRNCPY(v.str_val, json_object_get_string(tmpJobj), len+1);
                    memcpy(*objStruct+pos, v.str_val, len);
                    ZOS_FREE(v.str_val);

                    pos = pos + len;

                    if(len > redundantSpace) {
                        len = len - redundantSpace;
                        if((len % BOUNDARY) != 0)
                            redundantSpace = BOUNDARY - (len % BOUNDARY);
                        else
                            redundantSpace = 0;
                    }
                    else {
                        redundantSpace = redundantSpace - len;
                    }
                }else{
                    zcfgLog(ZCFG_LOG_ERR, "Input value was too long!!\n");
                    ZOS_FREE(*objStruct);
                    json_object_put(jobj);
                    return ZCFG_INVALID_PARAM_VALUE_LENGTH;
                }
				break;
			default :
				zcfgLog(ZCFG_LOG_ERR, "Default type %d\n", type);
				break;
		}
	}

	json_object_put(jobj);
#ifdef ZCFG_MEMORY_LEAK_DETECT
	recordZcfgAllocData(*objStruct, fileName, line, 0);
#endif

	return ZCFG_SUCCESS;
}

