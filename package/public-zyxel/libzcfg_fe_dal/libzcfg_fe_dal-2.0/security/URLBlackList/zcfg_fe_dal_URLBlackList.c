#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

#define URL_NAME_UD "UserDefined"

//#define URL_DEBUG

#ifdef URL_DEBUG
#define URL_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define URL_PRINT(format, ...)
#endif

dal_param_t URLBlack_FILTER_param[]={
	{"Index",				dalType_int,	0,	0,	NULL, NULL, dal_Add|dal_Delete},
	{"Name",			dalType_string,		0,	0,	NULL,	NULL,	dal_Add},
	{"BlackList",		dalType_string,		0,	0,	NULL,	NULL,	0},
	{"FilterIndex",		dalType_string,		0,	0,	NULL,	NULL,	0},	
	{NULL,		0,	0,	0,	NULL}
};

void zcfgFeDalShowURLBlackFilter(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-6s %-6s %-15s %-30s %-30s\n", "Index", "Enable","Filter Name", "BlackList","FilterIndex");
	printf("-----------------------------------------------------------------------------------------\n");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj!=NULL){
			printf("%-6d ",i+1);
			//printf("%-6d ",json_object_get_int(json_object_object_get(obj, "Enable")));			
			printf("%-15s ",json_object_get_string(json_object_object_get(obj, "Name")));
			printf("%-30s ",json_object_get_string(json_object_object_get(obj, "BlackList")));
			printf("%-30s ",json_object_get_string(json_object_object_get(obj, "FilterIndex")));			
			printf("\n");
		}
		else{
			printf("ERROR!\n");
		}
	}
}

zcfgRet_t zcfgFeDalURLBlackFilterGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t URBlackFilterobjIid;
	struct json_object *URLBlackFilterJobj = NULL;
	struct json_object *pramJobj = NULL;

	int index = 0;
	//char TR181Path[64] = {0};
	int obj_MaxLength = 0;

	IID_INIT(URBlackFilterobjIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_BLACK_U_R_L_FILTER, &URBlackFilterobjIid, &URLBlackFilterJobj) == ZCFG_SUCCESS){
		index++;
		obj_MaxLength = zcfgFeObjMaxLenGet(RDM_OID_BLACK_U_R_L_FILTER);
		if(obj_MaxLength == 0)printf("RDM_OID_BLACK_U_R_L_FILTER have on data!\n");
		//sprintf(TR181Path, "X_ZYXEL_URLBlackFilter.%u", URBlackFilterobjIid.idx[0]);

		pramJobj = json_object_new_object();		
		json_object_object_add(pramJobj, "Name",JSON_OBJ_COPY(json_object_object_get(URLBlackFilterJobj, "Name")));
		json_object_object_add(pramJobj, "BlackList",JSON_OBJ_COPY(json_object_object_get(URLBlackFilterJobj, "Blacklist")));
		json_object_object_add(pramJobj, "FilterIndex",JSON_OBJ_COPY(json_object_object_get(URLBlackFilterJobj, "FilterIndex")));

		if(json_object_object_get(Jobj, "Index")) {
			if(index == json_object_get_int(json_object_object_get(Jobj, "Index"))) {
				json_object_array_add(Jarray, pramJobj);
				zcfgFeJsonObjFree(URLBlackFilterJobj);
				break;
			}
		}
		else{
				json_object_array_add(Jarray, JSON_OBJ_COPY(pramJobj));
		}
		if(pramJobj) zcfgFeJsonObjFree(pramJobj);
		if(URLBlackFilterJobj) zcfgFeJsonObjFree(URLBlackFilterJobj);
	}
	return ret;
}

zcfgRet_t zcfgFeDalURLBlackFilterEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	struct json_object *URLBlackFilterJobj = NULL;
	struct json_object *pramJobj = NULL;
	struct json_object *pramJobjcopy = NULL;
	int objIndex;
	char *newName= NULL;
	char *oldName=NULL;
	char msg[30]={0};

	objIndex_t objIid={0};
	pramJobj = json_object_new_object();
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_BLACK_U_R_L_FILTER, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		if(pramJobj) zcfgFeJsonObjFree(pramJobj);
		printf("Wrong index..\n");
		return ret;
	}
	if((ret = zcfgFeObjJsonGet(RDM_OID_BLACK_U_R_L_FILTER, &objIid, &URLBlackFilterJobj)) != ZCFG_SUCCESS) {
		if(pramJobj) zcfgFeJsonObjFree(pramJobj);	
		return ret;
	}
	if((pramJobjcopy = json_object_object_get(Jobj, "Name")) != NULL){
		newName = (char *)json_object_get_string(pramJobj);
		oldName = (char *)json_object_get_string(json_object_object_get(URLBlackFilterJobj, "Name"));
		if(strcmp(newName,oldName)!=0){ // if newName != oldName
			ret = dalcmdParamCheck(&objIid, newName, RDM_OID_BLACK_U_R_L_FILTER, "Name", "Name", msg);
			if(ret == ZCFG_SUCCESS){
				json_object_object_add(URLBlackFilterJobj, "Name", JSON_OBJ_COPY(pramJobjcopy));
				//if(pramJobjcopy) zcfgFeJsonObjFree(pramJobjcopy);			
			}
			else{
				printf("%s\n",msg);
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.URLBlackFilter.Input.pop_name_duplicate"));
				if(URLBlackFilterJobj) zcfgFeJsonObjFree(URLBlackFilterJobj);
				if(pramJobj) zcfgFeJsonObjFree(pramJobj);	
				//if(pramJobjcopy) zcfgFeJsonObjFree(pramJobjcopy);	
				return ret;
			}
		}
	}
	if((pramJobjcopy = json_object_object_get(Jobj, "FilterIndex")) != NULL){
		json_object_object_add(URLBlackFilterJobj, "FilterIndex", JSON_OBJ_COPY(pramJobjcopy));
		//if(pramJobjcopy) zcfgFeJsonObjFree(pramJobjcopy);
	}
	if((pramJobjcopy = json_object_object_get(Jobj, "BlackList")) != NULL){
		json_object_object_add(URLBlackFilterJobj, "Blacklist", JSON_OBJ_COPY(pramJobjcopy));
		//if(pramJobjcopy) zcfgFeJsonObjFree(pramJobjcopy);
	}
	if((ret = zcfgFeObjJsonSet(RDM_OID_BLACK_U_R_L_FILTER, &objIid, URLBlackFilterJobj, NULL)) != ZCFG_SUCCESS);
	if(pramJobj) zcfgFeJsonObjFree(pramJobj);
	if(URLBlackFilterJobj) zcfgFeJsonObjFree(URLBlackFilterJobj);
	return ret;
}


bool getURLFilterIid_black(objIndex_t *URLFilterIid){
	struct json_object *URLFilterObj = NULL;
	objIndex_t indexIid = {0};
	const char *name = NULL, *filter = NULL;

	IID_INIT(*URLFilterIid);	
	IID_INIT(indexIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_U_R_L_FILTER, &indexIid, &URLFilterObj) == ZCFG_SUCCESS) {
		name = json_object_get_string(json_object_object_get(URLFilterObj, "Name"));
		URL_PRINT("%s: name=%s\n",__FUNCTION__,name);
		if(!strcmp(name, URL_NAME_UD)){
			filter = json_object_get_string(json_object_object_get(URLFilterObj, "FilterIndex"));
			URLFilterIid->level = 1;
			URL_PRINT("%s: filter=%s\n",__FUNCTION__,filter);
			if(sscanf(filter, "X_ZYXEL_URLFilter.%hhu", &(URLFilterIid->idx[0])) == 1){
				zcfgFeJsonObjFree(URLFilterObj);
				return true;
			}			
			zcfgFeJsonObjFree(URLFilterObj);
			break;
		}
		zcfgFeJsonObjFree(URLFilterObj);
	}
	return false;
}
zcfgRet_t zcfgFeDalURLBlackFilterAdd(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{

		zcfgRet_t ret = ZCFG_SUCCESS;
		objIndex_t objIid = {0};		
		objIndex_t FilterIid = {0};
		struct json_object *URLBlackFilterJobj = NULL;
		struct json_object *pramJobj = NULL;
#if 0
		char *name = NULL;
		char msg[30]={0};
#endif
		char path[32] = {0};
		struct json_object *pramJobjcopy = NULL;
#if 0		
		if(!ISSET("DontCheckDupName") && json_object_object_get(Jobj, "Name")){
			name = json_object_get_string(json_object_object_get(Jobj, "Name"));
			ret = dalcmdParamCheck(NULL,name, RDM_OID_BLACK_U_R_L_FILTER, "Name", "Name", msg);
			if(ret != ZCFG_SUCCESS){
				printf("%s\n",msg);
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.URLBlackFilter.Input.pop_name_duplicate"));
				return ret;
			}
		}
#endif
		IID_INIT(objIid);
		pramJobj = json_object_new_object();	
		URL_PRINT("%s: Jobj=%s\n",__FUNCTION__,json_object_get_string(Jobj));
		if(((json_object_object_get(Jobj, "BlackList")) == NULL)||!(json_object_get_boolean(json_object_object_get(Jobj, "BlackList")))){
			ret = ZCFG_INTERNAL_ERROR;
			if(pramJobj) json_object_put(pramJobj); 	
			return ret; 		
		}		
		if(getURLFilterIid_black(&FilterIid)){
			objIid.idx[1] = json_object_get_int(json_object_object_get(Jobj, "Index"));
			URL_PRINT("%s: idx=%d,%d\n",__FUNCTION__,objIid.idx[1],FilterIid.idx[1]);
			ret = zcfgFeObjJsonBlockedAdd(RDM_OID_BLACK_U_R_L_FILTER, &FilterIid, NULL);
			if(ret == ZCFG_SUCCESS){
				memcpy(&objIid, &FilterIid, sizeof(objIndex_t));
				ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_BLACK_U_R_L_FILTER, &objIid, &URLBlackFilterJobj);
			}		
			if(ret == ZCFG_SUCCESS){
#if 0				
				json_object_object_foreach(Jobj, key, val){
					if(!strcmp(key, "Index")){
						continue;
					}
					json_object_object_add(URLBlackFilterJobj, key, val);
				}
#endif				
				json_object_object_add(URLBlackFilterJobj, "Name", JSON_OBJ_COPY(json_object_object_get(Jobj, "Name")));		
				if((pramJobjcopy = json_object_object_get(Jobj, "BlackList")) != NULL){
					json_object_object_add(URLBlackFilterJobj, "Blacklist", JSON_OBJ_COPY(pramJobjcopy));
					//if(pramJobjcopy) zcfgFeJsonObjFree(pramJobjcopy);
				}
				if((pramJobjcopy = json_object_object_get(Jobj, "FilterIndex")) == NULL){					
					sprintf(path, "X_ZYXEL_URLFilter.%d",objIid.idx[0]);
					json_object_object_add(URLBlackFilterJobj, "FilterIndex", json_object_new_string(path));					
				}else{
					json_object_object_add(URLBlackFilterJobj, "FilterIndex", JSON_OBJ_COPY(pramJobjcopy));
					//if(pramJobjcopy) zcfgFeJsonObjFree(pramJobjcopy);					
				}
				if((ret = zcfgFeObjJsonSet(RDM_OID_BLACK_U_R_L_FILTER, &objIid, URLBlackFilterJobj, NULL)) != ZCFG_SUCCESS){
					if(pramJobj) zcfgFeJsonObjFree(pramJobj);
					if(URLBlackFilterJobj) zcfgFeJsonObjFree(URLBlackFilterJobj);				
					return ret;
				}				
				if(pramJobj) zcfgFeJsonObjFree(pramJobj);
				if(URLBlackFilterJobj) zcfgFeJsonObjFree(URLBlackFilterJobj);			}			
		}
	return ret;
}
zcfgRet_t zcfgFeDalURLBlackFilterDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid={0};
	int objIndex;
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_BLACK_U_R_L_FILTER, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	ret = zcfgFeObjJsonDel(RDM_OID_BLACK_U_R_L_FILTER, &objIid, NULL);
	if(ret == ZCFG_DELETE_REJECT){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.URLBlackFilter.delete_pcp_sch_err_msg"));
		printf("%s %s %d \n",json_object_to_json_string(Jobj),__func__,__LINE__);
		return ret;
	}
	return ret;
}

zcfgRet_t zcfgFeDalURLBlackFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalURLBlackFilterEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalURLBlackFilterAdd(Jobj, Jarray, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalURLBlackFilterDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalURLBlackFilterGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}
	return ret;
}

