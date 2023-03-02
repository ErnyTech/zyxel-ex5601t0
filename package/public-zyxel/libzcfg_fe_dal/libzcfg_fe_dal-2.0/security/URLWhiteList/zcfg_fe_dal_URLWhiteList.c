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

dal_param_t URLWhite_FILTER_param[]={
	{"Index",				dalType_int,	0,	0,	NULL, NULL, dal_Add|dal_Delete},
	{"Name",			dalType_string,		0,	0,	NULL,	NULL,	dal_Add},
	{"WhiteList",		dalType_string,		0,	0,	NULL,	NULL,	0},
	{"FilterIndex",		dalType_string,		0,	0,	NULL,	NULL,	0},	
	{NULL,		0,	0,	0,	NULL}
};

void zcfgFeDalShowURLWhiteFilter(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-6s %-6s %-15s %-30s %-30s\n", "Index", "Enable","Filter Name", "WhiteList","FilterIndex");
	printf("-----------------------------------------------------------------------------------------\n");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){

		obj = json_object_array_get_idx(Jarray, i);
		
		if(obj!=NULL){
			printf("%-6d ",i+1);
			//printf("%-6d ",json_object_get_int(json_object_object_get(obj, "Enable")));			
			printf("%-15s ",json_object_get_string(json_object_object_get(obj, "Name")));
			printf("%-30s ",json_object_get_string(json_object_object_get(obj, "WhiteList")));
			printf("%-30s ",json_object_get_string(json_object_object_get(obj, "FilterIndex")));			
			printf("\n");
		}
		else{			
			printf("ERROR!\n");
		}
	}
}

zcfgRet_t zcfgFeDalURLWhiteFilterGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t URWhiteFilterobjIid;
	struct json_object *URLWhiteFilterJobj = NULL;
	struct json_object *pramJobj = NULL;

	int index = 0;
	char TR181Path[64] = {0};
	int obj_MaxLength = 0;

	IID_INIT(URWhiteFilterobjIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_WHITE_U_R_L_FILTER, &URWhiteFilterobjIid, &URLWhiteFilterJobj) == ZCFG_SUCCESS){
		index++;
		obj_MaxLength = zcfgFeObjMaxLenGet(RDM_OID_WHITE_U_R_L_FILTER);
		if(obj_MaxLength == 0)printf("RDM_OID_WHITE_U_R_L_FILTER have on data!\n");
		sprintf(TR181Path, "X_ZYXEL_URLWhiteFilter.%u", URWhiteFilterobjIid.idx[0]);

		pramJobj = json_object_new_object();		
		json_object_object_add(pramJobj, "Name",JSON_OBJ_COPY(json_object_object_get(URLWhiteFilterJobj, "Name")));
		json_object_object_add(pramJobj, "WhiteList",JSON_OBJ_COPY(json_object_object_get(URLWhiteFilterJobj, "Whitelist")));
		json_object_object_add(pramJobj, "FilterIndex",JSON_OBJ_COPY(json_object_object_get(URLWhiteFilterJobj, "FilterIndex")));


		if(json_object_object_get(Jobj, "Index")) {
			if(index == json_object_get_int(json_object_object_get(Jobj, "Index"))) {
				json_object_array_add(Jarray, pramJobj);
				zcfgFeJsonObjFree(URLWhiteFilterJobj);
				break;
			}
		}
		else{
				json_object_array_add(Jarray, JSON_OBJ_COPY(pramJobj));
		}
		if(URLWhiteFilterJobj) json_object_put(URLWhiteFilterJobj);
		if(pramJobj) json_object_put(pramJobj); 

	}
	return ret;
}

zcfgRet_t zcfgFeDalURLWhiteFilterEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	struct json_object *URLWhiteFilterJobj = NULL;
	struct json_object *pramJobj = NULL;
	struct json_object *pramJobjcopy = NULL;	
	int objIndex;
	char *newName= NULL;
	char *oldName=NULL;
	char msg[30]={0};

	objIndex_t objIid={0};
	pramJobj = json_object_new_object();	
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_WHITE_U_R_L_FILTER, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		if(pramJobj) json_object_put(pramJobj);
		printf("Wrong index..\n");
		return ret;
	}
	if((ret = zcfgFeObjJsonGet(RDM_OID_WHITE_U_R_L_FILTER, &objIid, &URLWhiteFilterJobj)) != ZCFG_SUCCESS) {
		if(pramJobj) json_object_put(pramJobj);	
		return ret;
	}
	if((pramJobjcopy = json_object_object_get(Jobj, "Name")) != NULL){
		newName = (char *)json_object_get_string(pramJobj);
		oldName = (char *)json_object_get_string(json_object_object_get(URLWhiteFilterJobj, "Name"));
		if(strcmp(newName,oldName)!=0){ // if newName != oldName
			ret = dalcmdParamCheck(&objIid, newName, RDM_OID_WHITE_U_R_L_FILTER, "Name", "Name", msg);
			if(ret == ZCFG_SUCCESS){
				json_object_object_add(URLWhiteFilterJobj, "Name", JSON_OBJ_COPY(pramJobjcopy));
				//if(pramJobjcopy) json_object_put(pramJobjcopy); 
			}
			else{
				printf("%s\n",msg);
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.URLWhiteFilter.Input.pop_name_duplicate"));
				if(URLWhiteFilterJobj) json_object_put(URLWhiteFilterJobj);
				if(pramJobj) json_object_put(pramJobj);
				//if(pramJobjcopy) zcfgFeJsonObjFree(pramJobjcopy);	
				return ret;
			}
		}
	}	
	if((pramJobjcopy = json_object_object_get(Jobj, "FilterIndex")) != NULL){
		json_object_object_add(URLWhiteFilterJobj, "FilterIndex", JSON_OBJ_COPY(pramJobjcopy));
		//if(pramJobjcopy) json_object_put(pramJobjcopy);	
	}
	if((pramJobjcopy = json_object_object_get(Jobj, "WhiteList")) != NULL){
		json_object_object_add(URLWhiteFilterJobj, "Whitelist", JSON_OBJ_COPY(pramJobjcopy));
	}
	if((ret = zcfgFeObjJsonSet(RDM_OID_WHITE_U_R_L_FILTER, &objIid, URLWhiteFilterJobj, NULL)) != ZCFG_SUCCESS);
	if(URLWhiteFilterJobj) json_object_put(URLWhiteFilterJobj);
	if(pramJobj) json_object_put(pramJobj);	
	return ret;
}


bool getURLFilterIid(objIndex_t *URLFilterIid){
	struct json_object *URLFilterObj = NULL;
	objIndex_t indexIid = {0};
	const char *name = NULL, *filter = NULL;

	IID_INIT(*URLFilterIid);	
	IID_INIT(indexIid);

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_U_R_L_FILTER, &indexIid, &URLFilterObj) == ZCFG_SUCCESS) {
		name = json_object_get_string(json_object_object_get(URLFilterObj, "Name"));
		if(!strcmp(name, URL_NAME_UD)){
			filter = json_object_get_string(json_object_object_get(URLFilterObj, "FilterIndex"));
			URLFilterIid->level = 1;
			if(sscanf(filter, "X_ZYXEL_URLFilter.%hhu", &(URLFilterIid->idx[0])) == 1){
				zcfgFeJsonObjFree(URLFilterObj);
				return true;
			}			
			if(URLFilterObj) json_object_put(URLFilterObj);
			break;
		}
		if(URLFilterObj) json_object_put(URLFilterObj);
	}
	return false;
}
zcfgRet_t zcfgFeDalURLWhiteFilterAdd(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{

		zcfgRet_t ret = ZCFG_SUCCESS;
		objIndex_t objIid = {0};		
		objIndex_t FilterIid = {0};
		struct json_object *URLWhiteFilterJobj = NULL;
		struct json_object *pramJobj = NULL;
		struct json_object *pramJobjcopy = NULL;
#if 0		
		char *name = NULL;
		char msg[30]={0};
#endif
		char path[32] = {0};
#if 0
		if(!ISSET("DontCheckDupName") && json_object_object_get(Jobj, "Name")){
			name = json_object_get_string(json_object_object_get(Jobj, "Name"));
			ret = dalcmdParamCheck(NULL,name, RDM_OID_WHITE_U_R_L_FILTER, "Name", "Name", msg);
			if(ret != ZCFG_SUCCESS){
				printf("%s\n",msg);
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.URLWhiteFilter.Input.pop_name_duplicate"));
				return ret;
			}
		}
#endif
		IID_INIT(objIid);
		pramJobj = json_object_new_object();		
		if(((json_object_object_get(Jobj, "WhiteList")) == NULL)||!(json_object_get_boolean(json_object_object_get(Jobj, "WhiteList")))){
			ret = ZCFG_INTERNAL_ERROR;
			if(pramJobj) json_object_put(pramJobj); 	
			return ret; 		
		}		
		if(getURLFilterIid(&FilterIid)){
			objIid.idx[1] = json_object_get_int(json_object_object_get(Jobj, "Index"));
			ret = zcfgFeObjJsonBlockedAdd(RDM_OID_WHITE_U_R_L_FILTER, &FilterIid, NULL);
			if(ret == ZCFG_SUCCESS){
				memcpy(&objIid, &FilterIid, sizeof(objIndex_t));
				ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WHITE_U_R_L_FILTER, &objIid, &URLWhiteFilterJobj);
			}		
			if(ret == ZCFG_SUCCESS){
#if 0				
				json_object_object_foreach(Jobj, key, val){
					if(!strcmp(key, "Index")){
						continue;
					}
					json_object_object_add(URLWhiteFilterJobj, key, val);
				}
#endif				
				json_object_object_add(URLWhiteFilterJobj, "Name", JSON_OBJ_COPY(json_object_object_get(Jobj, "Name")));		
				if((pramJobjcopy = json_object_object_get(Jobj, "WhiteList")) != NULL){
					json_object_object_add(URLWhiteFilterJobj, "Whitelist", JSON_OBJ_COPY(pramJobjcopy));
					//if(pramJobjcopy) json_object_put(pramJobjcopy);	
				}
				if((pramJobjcopy = json_object_object_get(Jobj, "FilterIndex")) == NULL){					
					sprintf(path, "X_ZYXEL_URLFilter.%d",objIid.idx[0]);
					json_object_object_add(URLWhiteFilterJobj, "FilterIndex", json_object_new_string(path));					
				}else{
					json_object_object_add(URLWhiteFilterJobj, "FilterIndex", JSON_OBJ_COPY(pramJobjcopy));	
					//if(pramJobjcopy) json_object_put(pramJobjcopy);	
				}
				if((ret = zcfgFeObjJsonSet(RDM_OID_WHITE_U_R_L_FILTER, &objIid, URLWhiteFilterJobj, NULL)) != ZCFG_SUCCESS){
					return ret;
				}
				if(URLWhiteFilterJobj) json_object_put(URLWhiteFilterJobj);
				if(pramJobj) json_object_put(pramJobj);	
			}
		}
	return ret;
}
zcfgRet_t zcfgFeDalURLWhiteFilterDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid={0};
	int objIndex;
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_WHITE_U_R_L_FILTER, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	ret = zcfgFeObjJsonDel(RDM_OID_WHITE_U_R_L_FILTER, &objIid, NULL);
	if(ret == ZCFG_DELETE_REJECT){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.URLWhiteFilter.delete_pcp_sch_err_msg"));
		printf("%s %s %d \n",json_object_to_json_string(Jobj),__func__,__LINE__);
		return ret;
	}
	return ret;
}

zcfgRet_t zcfgFeDalURLWhiteFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalURLWhiteFilterEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalURLWhiteFilterAdd(Jobj, Jarray, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalURLWhiteFilterDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalURLWhiteFilterGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}
	return ret;
}

