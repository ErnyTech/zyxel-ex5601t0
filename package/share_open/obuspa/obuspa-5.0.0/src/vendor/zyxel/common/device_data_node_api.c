/*
 * register_data_node.c
 *
 *  Created on: Mar 13, 2022
 *
 */

#include "json-c/json.h"

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_fe_schema.h"
#include "zcfg_fe_rdm_string.h"
#include "zcmd_schema.h"

#include "device_data_node_api.h"


int IsObjectIndexPath(char *obj_path){
	char *pobj_i;
	if(obj_path != NULL){
		pobj_i = obj_path;
		pobj_i = pobj_i + strlen(obj_path) - 1;
		if(*pobj_i == 'i'){
			return 1;
		}
	}
	return 0;
}
int Convert_USP_Obj_Path(char *pIn_obj_path,char *pOut_usp_obj_path){
	char *pObjName = NULL;
	char obj_path[256] = {0};
	strncpy(obj_path,pIn_obj_path,256-1);
	pObjName = strtok(obj_path, ".");
	while( pObjName != NULL ) {
		 if(strcmp(pObjName,"i") == 0){
			 strcat(pOut_usp_obj_path,"{i}");
		 }else{
			 strcat(pOut_usp_obj_path,pObjName);
		 }
		 pObjName = strtok(NULL, ".");
		 if(pObjName != NULL){
			 strcat(pOut_usp_obj_path,".");
		 }
	}
	return strlen(pOut_usp_obj_path);
}

int Convert_Param_NumEntries_Path(char *pIn_obj_param_path,char *pOut_param_numentries_path){
	int err = USP_ERR_OK;
	char *pParamPath = NULL;
	char obj_param_num_entries_path[256] = {0};
	strncpy(obj_param_num_entries_path,pIn_obj_param_path,256-1);
	pParamPath = strrchr(obj_param_num_entries_path,'.');
	if(pParamPath != NULL){
		*pParamPath = '\0';
		strncpy(pOut_param_numentries_path,obj_param_num_entries_path,256-1);
	}else{
		USP_LOG_Error("Invalid Object parameter path\n");
		return USP_ERR_INTERNAL_ERROR;
	}
	return err;
}

int Convert_Param_NumEntries_Name(char *pIn_obj_param_name,char *pOut_param_numentries_name){
	int err = USP_ERR_OK;
	char *pParamName = NULL;
	char obj_param_name[256] = {0};
	strncpy(obj_param_name,pIn_obj_param_name,256-1);
	pParamName = strrchr(obj_param_name,'.');
	if(pParamName != NULL){
		*pParamName = '\0';
		strcat(obj_param_name,"NumberOfEntries");
		strncpy(pOut_param_numentries_name,obj_param_name,256-1);
	}else{
		USP_LOG_Error("Invalid Object parameter path\n");
		return USP_ERR_INTERNAL_ERROR;
	}
	return err;
}
int Parser_ObjIdx(char *pIn_obj_param_pathname,objIndex_t *pobjIndex){
	int err = USP_ERR_OK;
	char *pParamName = NULL;
	char *pObjName = NULL;
	int num;
	char obj_param_path[256] = {0};
	strncpy(obj_param_path,pIn_obj_param_pathname,256-1);
	pParamName = strrchr(obj_param_path,'.');
	if(pParamName != NULL){
		*pParamName = '\0';
		pObjName = strtok(obj_param_path, ".");
		while( pObjName != NULL ) {
			 num = atoi(pObjName);
			 if(num > 0 && num < 255){
				 pobjIndex->idx[pobjIndex->level] = num;
				 pobjIndex->level++;
			 }
			 pObjName = strtok(NULL, ".");
		}
	}else{
		USP_LOG_Error("Invalid Object parameter path\n");
		return USP_ERR_INTERNAL_ERROR;
	}
	return err;
}
int Parser_Obj_Param_PathName(char *pIn_obj_param_pathname,char *pOut_obj_param_path,char *pOut_obj_param_name,objIndex_t *pobjIndex){
	int err = USP_ERR_OK;
	char *pParamName = NULL;
	char *pObjName = NULL;
	int num;
	char obj_param_path[256] = {0};
	strncpy(obj_param_path,pIn_obj_param_pathname,256-1);
	pParamName = strrchr(obj_param_path,'.');
	if(pParamName != NULL){
		strncpy(pOut_obj_param_name,pParamName+1,256-1);
		*pParamName = '\0';
		pObjName = strtok(obj_param_path, ".");
		while( pObjName != NULL ) {
			 num = atoi(pObjName);
			 if(num > 0 && num < 255){
				 pobjIndex->idx[pobjIndex->level] = num;
				 pobjIndex->level++;
				 strcat(pOut_obj_param_path,"i");
			 }else{
				 strcat(pOut_obj_param_path,pObjName);
			 }
			 pObjName = strtok(NULL, ".");
			 if(pObjName != NULL){
				 strcat(pOut_obj_param_path,".");
			 }
		}
	}else{
		USP_LOG_Error("Invalid Object parameter path\n");
		return USP_ERR_INTERNAL_ERROR;
	}
	return err;
}
int Parser_Obj_PathName(char *pIn_obj_param_pathname,char *pOut_obj_param_path,objIndex_t *pobjIndex){
	int err = USP_ERR_OK;
	char *pObjName = NULL;
	int num;
	char obj_param_path[256] = {0};
	strncpy(obj_param_path,pIn_obj_param_pathname,256-1);

	pObjName = strtok(obj_param_path, ".");
	while( pObjName != NULL ) {
		 num = atoi(pObjName);
		 if(num > 0 && num < 255){
			 pobjIndex->idx[pobjIndex->level] = num;
			 pobjIndex->level++;
			 strcat(pOut_obj_param_path,"i");
		 }else{
			 strcat(pOut_obj_param_path,pObjName);
		 }
		 pObjName = strtok(NULL, ".");
		 if(pObjName != NULL){
			 strcat(pOut_obj_param_path,".");
		 }
	}
	return err;
}
int Register_Object_Parameter_Node(char *object_parameter_path,int param_type,int attr,DM_REGISTER_DATA_NODE_CB *pdm_register_data_node_cb){
	int err = USP_ERR_OK;
	int DM_type = 0;
	//USP_LOG_Debug("object_parameter_path[%s], param_type[%x], attr:[%x]\n",object_parameter_path,param_type,attr);
	switch(param_type){
	case json_type_boolean:
		DM_type = DM_BOOL;
		break;
	case json_type_base64:
	case json_type_hex:
	case json_type_string:
		DM_type = DM_STRING;
		break;
	case json_type_int:
		DM_type = DM_INT;
		break;
	case json_type_uint8:
	case json_type_uint16:
	case json_type_uint32:
		DM_type = DM_UINT;
		break;
	case json_type_time:
		DM_type = DM_DATETIME;
		break;
	case json_type_ulong:
		DM_type = DM_ULONG;
		break;
	default:
		USP_LOG_Error("Unknown Parameter Type [%x]\n",param_type);
		return USP_ERR_INTERNAL_ERROR;
		break;
	}
	if((attr & PARAMETER_ATTR_HIDDEN) ==  PARAMETER_ATTR_HIDDEN){
		//Define hidden attributes in zcfg_schema.json will be ignored
		USP_LOG_Debug(" Ignore Hidden Attr [%x][%s]\n",attr,object_parameter_path);
	}else if((attr & PARAMETER_ATTR_WRITE) ==  PARAMETER_ATTR_WRITE){
		err |= USP_REGISTER_VendorParam_ReadWrite(object_parameter_path,
				pdm_register_data_node_cb->get_value_cb,pdm_register_data_node_cb->set_value_cb,pdm_register_data_node_cb->notify_set_cb ,DM_type);
	}else if((attr & PARAMETER_ATTR_READONLY) == PARAMETER_ATTR_READONLY){
		err |= USP_REGISTER_VendorParam_ReadOnly(object_parameter_path,
				pdm_register_data_node_cb->get_value_cb, DM_type);
	}else if((attr & PARAMETER_ATTR_SAVED) == PARAMETER_ATTR_SAVED) {
		// Define only save attributes in zcfg_schema.json will set read-only
		err |= USP_REGISTER_VendorParam_ReadOnly(object_parameter_path,
						pdm_register_data_node_cb->get_value_cb, DM_type);
	}else {
		USP_LOG_Error(" Unknown Attr [%x]\n",attr);
		return USP_ERR_INTERNAL_ERROR;
	}
	if (err != USP_ERR_OK) {
		USP_LOG_Error(" USP_REGISTER object_parameter_path [%s]\n",object_parameter_path);
	}
	return err;
}
int Register_Next_Object_Node(char *pnext_obj_node_path,DM_REGISTER_DATA_NODE_CB *pdm_register_data_node_cb){
	int err = USP_ERR_OK;
	int oid = 0;
	int paramType = 0;
	int attrVal = 0;
	char usp_obj_param_path_name[384] = {0};
	char next_obj_path_name[256] = {0};
	char usp_obj_path[256] = {0};
	zcfgSubObjNameList_t *psub_objects = NULL,*pcurr_sub_objects = NULL;
	struct json_object *nameList = NULL;
	objIndex_t objIid;
	IID_INIT(objIid);
	oid = zcfgFeObjNameToObjId( pnext_obj_node_path, &objIid);
	if(!RDM_OID_IS_INVALID(oid) && pdm_register_data_node_cb != NULL){
		psub_objects = zcfgFeSubObjNameListGet(oid);
		pcurr_sub_objects = psub_objects;
		while(pcurr_sub_objects != NULL) {
			snprintf(next_obj_path_name,256-1,"%s.%s",pnext_obj_node_path,pcurr_sub_objects->objName);
			//USP_LOG_Debug("sub_object_path_name[%s]\n",next_obj_path_name);
			if(IsObjectIndexPath(next_obj_path_name)){
				memset(usp_obj_path,0,256);
				Convert_USP_Obj_Path(next_obj_path_name,usp_obj_path);

				err |= USP_REGISTER_Object(usp_obj_path,
						pdm_register_data_node_cb->validate_add_cb,
						pdm_register_data_node_cb->add_cb,
						pdm_register_data_node_cb->notify_add_cb,
						pdm_register_data_node_cb->validate_del_cb,
						pdm_register_data_node_cb->del_cb,
						pdm_register_data_node_cb->notify_del_cb);

				if (err != USP_ERR_OK) {
					 break;
				}
			}
			err |= Register_Next_Object_Node(next_obj_path_name,pdm_register_data_node_cb);
			pcurr_sub_objects = pcurr_sub_objects->next;

		}
		if(err == USP_ERR_OK){
			memset(usp_obj_path,0,256);
			Convert_USP_Obj_Path(pnext_obj_node_path,usp_obj_path);
			zcfgFeAllParamNameGet(oid,&nameList);
			zcfgFeParamNameForEach(nameList, pParamName){
				paramType = zcfgFeParamTypeGetByName(oid, pParamName);
				attrVal = zcfgFeParamAttrGetByName(oid, pParamName);
				snprintf(usp_obj_param_path_name,384-1,"%s.%s",usp_obj_path,pParamName);

				err |= Register_Object_Parameter_Node(usp_obj_param_path_name,paramType,attrVal,pdm_register_data_node_cb);

				if (err != USP_ERR_OK) {
					break;
				}
			}
		}
		if(nameList != NULL){
			zcfgFeJsonObjFree(nameList);
		}
		if(psub_objects != NULL){
			zcfgFeSubObjListFree(psub_objects);
		}

	}
	return err;
}

int Register_Object_Node(char *obj_node_path,DM_REGISTER_DATA_NODE_CB *pdm_register_data_node_cb){
	int err = USP_ERR_OK;
	int oid = 0;
	int paramType = 0;
	int attrVal = 0;
	char usp_obj_param_path_name[256] = {0};
	char next_obj_path_name[256] = {0};
	char usp_obj_path[256] = {0};
	zcfgSubObjNameList_t *psub_objects = NULL,*pcurr_sub_objects = NULL;
	struct json_object *nameList = NULL;
	objIndex_t objIid;
	IID_INIT(objIid);
	oid = zcfgFeObjNameToObjId(obj_node_path, &objIid);
	if(!RDM_OID_IS_INVALID(oid) && pdm_register_data_node_cb != NULL){
		psub_objects = zcfgFeSubObjNameListGet(oid);
		pcurr_sub_objects = psub_objects;
		while(pcurr_sub_objects != NULL) {
			snprintf(next_obj_path_name,256-1,"%s.%s",obj_node_path,pcurr_sub_objects->objName);
			//USP_LOG_Debug("sub_object_path_name[%s]\n",next_obj_path_name);
			if(IsObjectIndexPath(next_obj_path_name)){
				memset(usp_obj_path,0,256);
				Convert_USP_Obj_Path(next_obj_path_name,usp_obj_path);
				err |= USP_REGISTER_Object(usp_obj_path,
						pdm_register_data_node_cb->validate_add_cb,
						pdm_register_data_node_cb->add_cb,
						pdm_register_data_node_cb->notify_add_cb,
						pdm_register_data_node_cb->validate_del_cb,
						pdm_register_data_node_cb->del_cb,
						pdm_register_data_node_cb->notify_del_cb);

				if (err != USP_ERR_OK) {
					break;
				}
			}
			err |= Register_Next_Object_Node(next_obj_path_name,pdm_register_data_node_cb);
			pcurr_sub_objects = pcurr_sub_objects->next;
		}
		if(err == USP_ERR_OK){
			zcfgFeAllParamNameGet(oid,&nameList);
			zcfgFeParamNameForEach(nameList, pParamName){
				paramType = zcfgFeParamTypeGetByName(oid, pParamName);
				attrVal = zcfgFeParamAttrGetByName(oid, pParamName);
				snprintf(usp_obj_param_path_name,256-1,"%s.%s",obj_node_path,pParamName);
				err |= Register_Object_Parameter_Node(usp_obj_param_path_name,paramType,attrVal,pdm_register_data_node_cb);
				if (err != USP_ERR_OK) {
					break;
				}
			}
		}
		if(nameList != NULL){
			zcfgFeJsonObjFree(nameList);
		}
		if(psub_objects != NULL){
			zcfgFeSubObjListFree(psub_objects);
		}
	}else{
		USP_LOG_Error("Invalid handle\n");
		err = USP_ERR_INTERNAL_ERROR;
	}
	return err;
}
int Register_Object_Paramter_Node(char *obj_node_path,DM_REGISTER_DATA_NODE_CB *pdm_register_data_node_cb){
	int err = USP_ERR_OK;
	int oid = 0;
	int paramType = 0;
	int attrVal = 0;
	char usp_obj_param_path_name[256] = {0};
	struct json_object *nameList = NULL;
	objIndex_t objIid;
	IID_INIT(objIid);
	oid = zcfgFeObjNameToObjId(obj_node_path, &objIid);
	if(!RDM_OID_IS_INVALID(oid) && pdm_register_data_node_cb != NULL){
		zcfgFeAllParamNameGet(oid,&nameList);
		zcfgFeParamNameForEach(nameList, pParamName){
			paramType = zcfgFeParamTypeGetByName(oid, pParamName);
			attrVal = zcfgFeParamAttrGetByName(oid, pParamName);
			snprintf(usp_obj_param_path_name,256-1,"%s.%s",obj_node_path,pParamName);
			err |= Register_Object_Parameter_Node(usp_obj_param_path_name,paramType,attrVal,pdm_register_data_node_cb);
			if (err != USP_ERR_OK) {
				break;
			}
		}
		if(nameList != NULL){
			zcfgFeJsonObjFree(nameList);
		}
	}else{
		USP_LOG_Error("Invalid handle\n");
		err = USP_ERR_INTERNAL_ERROR;
	}
	return err;
}
int Get_Object_Parameter_Value(dm_req_t *req, char *buf, int len){
	int err = USP_ERR_OK;
	int oid = 0;
	int ParamType;
	struct json_object *pjObj = NULL;
	struct json_object *pjParamValue = NULL;
	char object_parameter_path[256] = {0};
	char object_parameter_name[256] = {0};
	objIndex_t get_objIid;
	objIndex_t objIid;
	IID_INIT(get_objIid);
	IID_INIT(objIid);
	err |= Parser_Obj_Param_PathName(req->path,object_parameter_path,object_parameter_name,&get_objIid);
	if (err != USP_ERR_OK) {
		return err;
	}
	oid = zcfgFeObjNameToObjId(object_parameter_path, &objIid);
	if(!RDM_OID_IS_INVALID(oid)){
		if(zcfgFeObjJsonGet(oid, &get_objIid, &pjObj) == ZCFG_SUCCESS){
			pjParamValue = json_object_object_get(pjObj, object_parameter_name);
			if(pjParamValue != NULL){
				//USP_LOG_Debug("%s\n",json_object_to_json_string(pjParamValue));
				ParamType = json_object_get_type(pjParamValue);
				switch(ParamType){
					case json_type_boolean:
						req->val_union.value_bool = json_object_get_boolean(pjParamValue);
						break;
					case json_type_string:
						strncpy(buf,json_object_get_string(pjParamValue),len-1);
						break;
					case json_type_int:
						req->val_union.value_int = (int)json_object_get_int(pjParamValue);
						break;
					case json_type_uint8:
					case json_type_uint16:
					case json_type_uint32:
						req->val_union.value_uint = (unsigned int)json_object_get_int(pjParamValue);
						break;
					case json_type_time:
						req->val_union.value_datetime = USP_CONVERT_DateTimeToUnixTime((char*)json_object_get_string(pjParamValue));
						break;
					case json_type_ulong:
						req->val_union.value_ulong = (unsigned long long)json_object_get_int64(pjParamValue);
						break;
					default:
						USP_LOG_Error("Unknown Parameter Type [%x]\n",ParamType);
						err = USP_ERR_INTERNAL_ERROR;
						break;
					}
			 }else{
				 USP_LOG_Error("Invalid Parameter Value\n");
				 err = USP_ERR_INTERNAL_ERROR;
			 }
		}else{
			USP_LOG_Error("zcfgFeObjJsonGet fail\n");
			err = USP_ERR_INTERNAL_ERROR;
		}
	}else{
		USP_LOG_Error("Invalid OID handle\n");
		err = USP_ERR_INTERNAL_ERROR;
	}
	if(pjObj != NULL){
		zcfgFeJsonObjFree(pjObj);
	}
	return err;
}

int Set_Object_Parameter_Value(dm_req_t *req, char *buf){
	int err = USP_ERR_OK;
	int oid = 0;
	int ParamType;
	struct json_object *pjObj = NULL;
	struct json_object *pjParamValue = NULL;
	char object_parameter_path[256] = {0};
	char object_parameter_name[256] = {0};
	char DateTime[128] = {0};
	objIndex_t set_objIid;
	objIndex_t objIid;
	IID_INIT(set_objIid);
	IID_INIT(objIid);
	err |= Parser_Obj_Param_PathName(req->path,object_parameter_path,object_parameter_name,&set_objIid);
	if (err != USP_ERR_OK) {
		return err;
	}
	oid = zcfgFeObjNameToObjId(object_parameter_path, &objIid);
	if(!RDM_OID_IS_INVALID(oid)){
		if(zcfgFeObjJsonGet(oid, &set_objIid, &pjObj) == ZCFG_SUCCESS){
			pjParamValue = json_object_object_get(pjObj, object_parameter_name);
			if(pjParamValue != NULL){
				//USP_LOG_Debug("%s\n",json_object_to_json_string(pjParamValue));
				ParamType = json_object_get_type(pjParamValue);
				switch(ParamType){
					case json_type_boolean:
						json_object_set_boolean(pjParamValue,req->val_union.value_bool);
						break;
					case json_type_string:
						json_object_set_string_len(pjParamValue,buf,strlen(buf)+1);
						break;
					case json_type_int:
						json_object_set_int(pjParamValue,req->val_union.value_int);
						break;
					case json_type_uint8:
					case json_type_uint16:
					case json_type_uint32:
						json_object_set_int(pjParamValue,req->val_union.value_uint);
						break;
					case json_type_time:
							USP_CONVERT_UnixTimeToDateTime(req->val_union.value_datetime,DateTime,128);
							json_object_set_string_len(pjParamValue,DateTime,strlen(DateTime)+1);
						break;
					case json_type_ulong:
						json_object_set_int64(pjParamValue,req->val_union.value_ulong);
						break;
					default:
						USP_LOG_Error("Unknown Parameter Type [%x]\n",ParamType);
						err = USP_ERR_INTERNAL_ERROR;
						break;
					}
			 }else{
				 USP_LOG_Error("Invalid Parameter Value\n");
				 err = USP_ERR_INTERNAL_ERROR;
			 }
		}else{
			USP_LOG_Error("zcfgFeObjJsonGet fail\n");
			err = USP_ERR_INTERNAL_ERROR;
		}
	}else{
		USP_LOG_Error("Invalid OID handle\n");
		err = USP_ERR_INTERNAL_ERROR;
	}

	if (err == USP_ERR_OK) {
		//USP_LOG_Debug("%s\n",json_object_to_json_string(pjObj));
		if(zcfgFeObjJsonSet(oid,&set_objIid,pjObj,NULL) == ZCFG_SUCCESS){
		}else{
			USP_LOG_Error("zcfgFeObjJsonSet fail\n");
			err = USP_ERR_INTERNAL_ERROR;
		}
	}
	if(pjObj != NULL){
		zcfgFeJsonObjFree(pjObj);
	}
	return err;
}
int Add_Object_Node(dm_req_t *req){
	int err = USP_ERR_OK;
	int oid = 0;
	char add_object_parameter_path[384] = {0};
	char object_parameter_path[256] = {0};
	objIndex_t add_objIid;
	objIndex_t objIid;
	IID_INIT(add_objIid);
	IID_INIT(objIid);
	snprintf(add_object_parameter_path,384-1,"%s.%d",req->path,req->inst->instances[req->inst->order-1]);
	err |= Parser_Obj_PathName(add_object_parameter_path,object_parameter_path,&add_objIid);
	if (err != USP_ERR_OK) {
		return err;
	}
	oid = zcfgFeObjNameToObjId(object_parameter_path, &objIid);
	if(!RDM_OID_IS_INVALID(oid)){
		objIid.level = add_objIid.level - 1;
		int c;
		for(c=0;c<objIid.level;c++){
			objIid.idx[c] = add_objIid.idx[c];
		}
		if(zcfgFeObjJsonAdd(oid, &objIid,NULL) == ZCFG_SUCCESS){
			err |= DATA_MODEL_NotifyInstanceAdded(add_object_parameter_path);
		}else{
			USP_LOG_Error("zcfgFeObjJsonAdd fail\n");
			err = USP_ERR_INTERNAL_ERROR;
		}
	}else{
		USP_LOG_Error("Invalid OID handle\n");
		err = USP_ERR_INTERNAL_ERROR;
	}

	return err;
}

int Del_Object_Node(dm_req_t *req){
	int err = USP_ERR_OK;
	int oid = 0;
	char object_parameter_path[256] = {0};
	objIndex_t del_objIid;
	objIndex_t objIid;
	IID_INIT(del_objIid);
	IID_INIT(objIid);
	err |= Parser_Obj_PathName(req->path,object_parameter_path,&del_objIid);
	if (err != USP_ERR_OK) {
		return err;
	}
	oid = zcfgFeObjNameToObjId(object_parameter_path, &objIid);
	if(!RDM_OID_IS_INVALID(oid)){
		if(zcfgFeObjJsonDel(oid, &del_objIid, NULL) == ZCFG_SUCCESS){
			err |= DATA_MODEL_NotifyInstanceDeleted(req->path);
		}else{
			USP_LOG_Error("zcfgFeObjJsonDel fail\n");
			err = USP_ERR_INTERNAL_ERROR;
		}
	}else{
		USP_LOG_Error("Invalid OID handle\n");
		err = USP_ERR_INTERNAL_ERROR;
	}
	return err;
}
int Sync_Next_Object_NumberOfEntries_Node(char *psub_obj_path,char *pusp_obj_param_num_entries_path){
	int err = USP_ERR_OK;
	struct json_object *pjObj = NULL;
	struct json_object *pjParamValue = NULL;
	int ParamType;
	unsigned int number = 0;
	int oid = 0;
	char usp_obj_param_num_entries_path[384] = {0};
	char usp_obj_param_num_entries_pathname[384] = {0};
	char sub_obj_path_name[256] = {0};
	char obj_param_num_entries_pathname[384] = {0};
	char obj_param_num_entries_path[256] = {0};
	char obj_param_num_entries_name[256] = {0};
	zcfgSubObjNameList_t *psub_objects = NULL,*pcurr_sub_objects = NULL;
	objIndex_t objIid;
	IID_INIT(objIid);
	oid = zcfgFeObjNameToObjId(psub_obj_path, &objIid);
	if(!RDM_OID_IS_INVALID(oid)){
		psub_objects = zcfgFeSubObjNameListGet(oid);
		pcurr_sub_objects = psub_objects;
		while(pcurr_sub_objects != NULL) {
			snprintf(sub_obj_path_name,256-1,"%s.%s",psub_obj_path,pcurr_sub_objects->objName);
			//USP_LOG_Debug("sub_object_path_name[%s]\n",sub_obj_path_name);
			if(IsObjectIndexPath(sub_obj_path_name)){
				number = 0;
				memset(obj_param_num_entries_name,0,256);
				Convert_Param_NumEntries_Name(pcurr_sub_objects->objName,obj_param_num_entries_name);
				if(pusp_obj_param_num_entries_path != NULL){
					IID_INIT(objIid);
					snprintf(obj_param_num_entries_pathname,384-1,"%s.%s",pusp_obj_param_num_entries_path,obj_param_num_entries_name);
					Parser_ObjIdx(obj_param_num_entries_pathname,&objIid);
				}
				if(zcfgFeObjJsonGet(oid, &objIid, &pjObj) == ZCFG_SUCCESS){
						pjParamValue = json_object_object_get(pjObj, obj_param_num_entries_name);
						if(pjParamValue != NULL){
							//USP_LOG_Debug("%s:%s\n",obj_param_num_entries_pathname,json_object_to_json_string(pjParamValue));
							ParamType = json_object_get_type(pjParamValue);
							switch(ParamType){
								case json_type_int:
									number = json_object_get_int(pjParamValue);
									break;
								default:
									USP_LOG_Error("Error Parameter Type [%x]\n",ParamType);
									err = USP_ERR_INTERNAL_ERROR;
									break;
							}
						 }else{
							 USP_LOG_Error("Invalid [%s.%s] Parameter Value\n",psub_obj_path,obj_param_num_entries_name);
							 //err = USP_ERR_INTERNAL_ERROR;
						 }
				}else{
					USP_LOG_Error("zcfgFeObjJsonGet fail\n");
					err = USP_ERR_INTERNAL_ERROR;
				}
				if (err != USP_ERR_OK) {
					break;
				}
				if(number > 0){
					if(pusp_obj_param_num_entries_path == NULL){
						Convert_Param_NumEntries_Path(sub_obj_path_name,obj_param_num_entries_path);
					}else{
						snprintf(usp_obj_param_num_entries_pathname,384-1,"%s.%s",pusp_obj_param_num_entries_path,pcurr_sub_objects->objName);
						Convert_Param_NumEntries_Path(usp_obj_param_num_entries_pathname,obj_param_num_entries_path);
					}
					for(int i=1;i <= number;i++){
						snprintf(usp_obj_param_num_entries_path,384-1,"%s.%d",obj_param_num_entries_path,i);
						err = DATA_MODEL_InformInstance(usp_obj_param_num_entries_path);
						if (err == USP_ERR_OK) {
							err |= Sync_Next_Object_NumberOfEntries_Node(sub_obj_path_name,usp_obj_param_num_entries_path);
						}
						if (err != USP_ERR_OK) {
							break;
						}
					}
				}
			}else{
				err |= Sync_Next_Object_NumberOfEntries_Node(sub_obj_path_name,NULL);
				if (err != USP_ERR_OK) {
					break;
				}
			}

			pcurr_sub_objects = pcurr_sub_objects->next;
		}
		if(psub_objects != NULL){
			zcfgFeSubObjListFree(psub_objects);
		}
	}else{
		USP_LOG_Error("Invalid handle\n");
		err = USP_ERR_INTERNAL_ERROR;
	}
	return err;
}
int Sync_Object_NumberOfEntries_Node(char *data_node_path){
	int err = USP_ERR_OK;
	struct json_object *pjObj = NULL;
	struct json_object *pjParamValue = NULL;
	int ParamType;
	int number = 0;
	int oid = 0;
	char usp_obj_param_num_entries_path[384] = {0};
	char sub_obj_path_name[256] = {0};
	char obj_param_num_entries_path[256] = {0};
	char obj_param_num_entries_name[256] = {0};
	zcfgSubObjNameList_t *psub_objects = NULL,*pcurr_sub_objects = NULL;
	objIndex_t objIid;
	IID_INIT(objIid);
	oid = zcfgFeObjNameToObjId(data_node_path, &objIid);
	if(!RDM_OID_IS_INVALID(oid)){
		psub_objects = zcfgFeSubObjNameListGet(oid);
		pcurr_sub_objects = psub_objects;
		while(pcurr_sub_objects != NULL) {
			snprintf(sub_obj_path_name,256-1,"%s.%s",data_node_path,pcurr_sub_objects->objName);
			//USP_LOG_Debug("sub_object_path_name[%s]\n",sub_obj_path_name);
			if(IsObjectIndexPath(sub_obj_path_name)){
				number = 0;
				memset(obj_param_num_entries_name,0,256);
				Convert_Param_NumEntries_Name(pcurr_sub_objects->objName,obj_param_num_entries_name);
				if(zcfgFeObjJsonGet(oid, &objIid, &pjObj) == ZCFG_SUCCESS){
						pjParamValue = json_object_object_get(pjObj, obj_param_num_entries_name);
						if(pjParamValue != NULL){
							//USP_LOG_Debug("%s:%s\n",obj_param_num_entries_name,json_object_to_json_string(pjParamValue));
							ParamType = json_object_get_type(pjParamValue);
							switch(ParamType){
								case json_type_int:
									number = json_object_get_int(pjParamValue);
									break;
								default:
									USP_LOG_Error("Error Parameter Type [%x]\n",ParamType);
									err = USP_ERR_INTERNAL_ERROR;
									break;
							}
						 }else{
							 USP_LOG_Error("Invalid [%s.%s] Parameter Value\n",data_node_path,obj_param_num_entries_name);
							// err = USP_ERR_INTERNAL_ERROR;
						 }
				}else{
					USP_LOG_Error("zcfgFeObjJsonGet fail\n");
					err = USP_ERR_INTERNAL_ERROR;
				}
				if (err != USP_ERR_OK) {
					break;
				}
				if(number > 0){
					Convert_Param_NumEntries_Path(sub_obj_path_name,obj_param_num_entries_path);
					for(int i=1;i <= number;i++){
						snprintf(usp_obj_param_num_entries_path,384-1,"%s.%d",obj_param_num_entries_path,i);
						err = DATA_MODEL_InformInstance(usp_obj_param_num_entries_path);
						if (err == USP_ERR_OK) {
							err |= Sync_Next_Object_NumberOfEntries_Node(sub_obj_path_name,usp_obj_param_num_entries_path);
						}
						if (err != USP_ERR_OK) {
							break;
						}
					}
				}
			}else{
				err |= Sync_Next_Object_NumberOfEntries_Node(sub_obj_path_name,NULL);
				if (err != USP_ERR_OK) {
					break;
				}
			}

			pcurr_sub_objects = pcurr_sub_objects->next;
		}
		if(psub_objects != NULL){
			zcfgFeSubObjListFree(psub_objects);
		}
	}else{
		USP_LOG_Error("Invalid handle\n");
		err = USP_ERR_INTERNAL_ERROR;
	}
	return err;
}


