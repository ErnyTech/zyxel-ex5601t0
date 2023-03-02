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

//#define URL_DEBUG

#ifdef URL_DEBUG
#define URL_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define URL_PRINT(format, ...)
#endif

dal_param_t URL_FILTER_param[]={
	{"Index",				dalType_int,	0,	0,	NULL, NULL, dal_Add|dal_Delete},
	{"Name",			dalType_string,		0,	0,	NULL,	NULL,	dal_Add},
	{"Enable",	dalType_boolean,	0,	0,	NULL, NULL, 0},	
	{"Description",		dalType_string,		0,	0,	NULL,	NULL,	0},
	{"FilterIndex",		dalType_string,		0,	0,	NULL,	NULL,	0},	
	{NULL,		0,	0,	0,	NULL}
};
	
void zcfgFeDalShowURLFilter(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;
	bool Enable;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-6s %-6s %-15s %-30s %-30s\n", "Index", "Enable","Filter Name", "Description", "FilterIndex");
	printf("----------------------------------------------------------------------------------------------\n");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj!=NULL){
			Enable = json_object_get_boolean(json_object_object_get(obj, "Enable"));
			printf("%-6d ",i+1);
			printf("%-6d ",Enable);
			printf("%-15s ",json_object_get_string(json_object_object_get(obj, "Name")));
			printf("%-30s ",json_object_get_string(json_object_object_get(obj, "Description")));
			printf("%-30s ",json_object_get_string(json_object_object_get(obj, "FilterIndex")));			
			printf("\n");
		}
		else{
			printf("ERROR!\n");
		}
	}
}

zcfgRet_t zcfgFeDalURLFilterGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *URLFilterJobj = NULL;
	struct json_object *pramJobj = NULL;

	int index = 0;
	char TR181Path[64] = {0};
	int obj_MaxLength = 0;


	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_U_R_L_FILTER, &objIid, &URLFilterJobj) == ZCFG_SUCCESS){
		index++;
		obj_MaxLength = zcfgFeObjMaxLenGet(RDM_OID_U_R_L_FILTER);
		if(obj_MaxLength == 0)printf("RDM_OID_U_R_L_FILTER have on data!\n");
		sprintf(TR181Path, "X_ZYXEL_URLFilter.%u", objIid.idx[0]);

		pramJobj = json_object_new_object();
		json_object_object_add(pramJobj, "Name",JSON_OBJ_COPY(json_object_object_get(URLFilterJobj, "Name")));
		json_object_object_add(pramJobj, "Description",JSON_OBJ_COPY(json_object_object_get(URLFilterJobj, "Description")));
#if 0 //ICHIA 20191021
		json_object_object_add(pramJobj, "Enable",json_object_object_get(URLFilterJobj, "Enable"));
		json_object_object_add(pramJobj, "FilterIndex",json_object_object_get(URLFilterJobj, "FilterIndex"));
#else
		json_object_object_add(pramJobj, "Enable",JSON_OBJ_COPY(json_object_object_get(URLFilterJobj, "Enable")));
		json_object_object_add(pramJobj, "FilterIndex",JSON_OBJ_COPY(json_object_object_get(URLFilterJobj, "FilterIndex")));
#endif
		if(json_object_object_get(Jobj, "Index")) {
			if(index == json_object_get_int(json_object_object_get(Jobj, "Index"))) {
				json_object_array_add(Jarray, pramJobj);
				if(URLFilterJobj) json_object_put(URLFilterJobj);
				if(pramJobj) json_object_put(pramJobj); 
				break;
			}
		}
		else {
			json_object_array_add(Jarray, JSON_OBJ_COPY(pramJobj));
		}
		if(URLFilterJobj) json_object_put(URLFilterJobj);
		if(pramJobj) json_object_put(pramJobj);	

	}
	return ret;
}

zcfgRet_t zcfgFeDalURLFilterEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	struct json_object *URLFilterJobj = NULL;
	struct json_object *pramJobj = NULL;
	struct json_object *pramJobjcopy = NULL;	
	int objIndex;
	char *newName= NULL;
	char *oldName=NULL;
	char msg[30]={0};

	bool URLFilterEnable;
	objIndex_t objIid={0};
	pramJobj = json_object_new_object();
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_U_R_L_FILTER, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		if(pramJobj) json_object_put(pramJobj);	
		printf("Wrong index..\n");
		return ret;
	}
	if((ret = zcfgFeObjJsonGet(RDM_OID_U_R_L_FILTER, &objIid, &URLFilterJobj)) != ZCFG_SUCCESS) {
		if(pramJobj) json_object_put(pramJobj);		
		return ret;
	}
	if((pramJobjcopy = json_object_object_get(Jobj, "Name")) != NULL){
		newName = (char *)json_object_get_string(pramJobj);
		oldName = (char *)json_object_get_string(json_object_object_get(URLFilterJobj, "Name"));
		if(strcmp(newName,oldName)!=0){ // if newName != oldName
			ret = dalcmdParamCheck(&objIid, newName, RDM_OID_U_R_L_FILTER, "Name", "Name", msg);
			if(ret == ZCFG_SUCCESS){
				json_object_object_add(URLFilterJobj, "Name", JSON_OBJ_COPY(pramJobjcopy));
				//if(pramJobjcopy) json_object_put(pramJobjcopy);	
			}
			else{
				printf("%s\n",msg);
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.URLFilter.Input.pop_name_duplicate"));
				if(URLFilterJobj) json_object_put(URLFilterJobj);
				if(pramJobj) json_object_put(pramJobj);	
				//if(pramJobjcopy) json_object_put(pramJobjcopy);	
				return ret;
			}
		}
	}
	if((pramJobjcopy = json_object_object_get(Jobj, "Description")) != NULL){
		json_object_object_add(URLFilterJobj, "Description", JSON_OBJ_COPY(pramJobjcopy));
		//if(pramJobjcopy) json_object_put(pramJobjcopy);	
	}
	if((pramJobjcopy = json_object_object_get(Jobj, "FilterIndex")) != NULL){
		json_object_object_add(URLFilterJobj, "FilterIndex", JSON_OBJ_COPY(pramJobjcopy));	
		//if(pramJobjcopy) json_object_put(pramJobjcopy);	
	}
	URLFilterEnable = json_object_get_boolean(json_object_object_get(Jobj,"Enable"));
	json_object_object_add(URLFilterJobj, "Enable",json_object_new_boolean(URLFilterEnable));
	if((ret = zcfgFeObjJsonSet(RDM_OID_U_R_L_FILTER, &objIid, URLFilterJobj, NULL)) != ZCFG_SUCCESS);
	if(URLFilterJobj) json_object_put(URLFilterJobj);
	if(pramJobj) json_object_put(pramJobj); 
	return ret;
}


zcfgRet_t zcfgFeDalURLFilterAdd(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{

		zcfgRet_t ret = ZCFG_SUCCESS;
		objIndex_t objIid;
		struct json_object *URLFilterJobj = NULL;
		struct json_object *pramJobjcopy = NULL;
#if 0
		char *name = NULL;
		char msg[30]={0};
#endif
		char path[32] = {0};

#if 0
		if(!ISSET("DontCheckDupName") && json_object_object_get(Jobj, "Name")){
			name = json_object_get_string(json_object_object_get(Jobj, "Name"));
			ret = dalcmdParamCheck(NULL,name, RDM_OID_U_R_L_FILTER, "Name", "Name", msg);
			if(ret != ZCFG_SUCCESS){
				printf("%s\n",msg);
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.URLFilter.Input.pop_name_duplicate"));
				return ret;
			}
		}
#endif
		IID_INIT(objIid);
		if((ret = zcfgFeObjJsonAdd(RDM_OID_U_R_L_FILTER, &objIid, NULL)) != ZCFG_SUCCESS){
			if(ret == ZCFG_EXCEED_MAX_INS_NUM){
				printf("Cannot add entry since the maximum number of entries has been reached.\n");
			}
			return ret;
		}
		if((ret = zcfgFeObjJsonGet(RDM_OID_U_R_L_FILTER, &objIid, &URLFilterJobj)) != ZCFG_SUCCESS) {
			printf("%s: Object retrieve fail\n", __FUNCTION__);
			return ret;
		}
		json_object_object_add(URLFilterJobj, "Enable", json_object_new_boolean(true));
		json_object_object_add(URLFilterJobj, "Name", JSON_OBJ_COPY(json_object_object_get(Jobj, "Name")));
		if((pramJobjcopy = json_object_object_get(Jobj, "Description")) != NULL){
			json_object_object_add(URLFilterJobj, "Description", JSON_OBJ_COPY(pramJobjcopy));
			//if(pramJobjcopy) json_object_put(pramJobjcopy);			
		}
		URL_PRINT("%s: %s\n",__FUNCTION__,json_object_get_string(Jobj));
		objIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "Index"));
		sprintf(path, "X_ZYXEL_URLFilter.%lld",json_object_get_int(json_object_object_get(Jobj, "Index")));	
		URL_PRINT("%s: path=%s, %d\n",__FUNCTION__,path,objIid.idx[0]);
		json_object_object_add(URLFilterJobj, "FilterIndex", json_object_new_string(path));

		if((ret = zcfgFeObjJsonSet(RDM_OID_U_R_L_FILTER, &objIid, URLFilterJobj, NULL)) != ZCFG_SUCCESS){
			return ret;
		}
		if(URLFilterJobj) json_object_put(URLFilterJobj);
	return ret;
}

zcfgRet_t zcfgFeDalURLFilterDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid={0};
	int objIndex;
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_U_R_L_FILTER, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	ret = zcfgFeObjJsonDel(RDM_OID_U_R_L_FILTER, &objIid, NULL);
	if(ret == ZCFG_DELETE_REJECT){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.URLFilter.delete_pcp_sch_err_msg"));
		printf("%s %s %d \n",json_object_to_json_string(Jobj),__func__,__LINE__);
		return ret;
	}
	return ret;
}

zcfgRet_t zcfgFeDalURLFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;
	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalURLFilterEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalURLFilterAdd(Jobj, Jarray, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalURLFilterDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalURLFilterGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}
	return ret;
}



