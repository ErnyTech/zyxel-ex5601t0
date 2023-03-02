
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_webgui_flag.h"

#define SETRULE			0
#define CLEANALL		1

dal_param_t VOIP_CALLRULE_param[]={
	{"CleanAll",								dalType_boolean,	0,	0,	NULL},
	{"Index",									dalType_int,		0,	0,	NULL},
	{"Number",									dalType_string,		0,	0,	NULL},
	{"Name",									dalType_string,	0,	0,	NULL},
	{NULL,										0,				0,	0,	NULL},
};

struct json_object *multiJobj;
struct json_object *speedDailListJobj;

void zcfgFeDalShowVoipCallRule(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-10s %-20s %-10s \n","Index","Number","Description");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-10s %-20s %-10s \n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "OrigNumber")),
			json_object_get_string(json_object_object_get(obj, "Name")));
	}
}

objIndex_t CallRuleIid = {0};

bool cleanAll;
bool callRuleType;
char* Name;
char* OrigNumber;

void initSpeedDialGlobalObjects(){
	multiJobj = NULL;
	speedDailListJobj = NULL;
	return;
}

void freeAllSpeedDialObjects(){
	if(multiJobj) json_object_put(multiJobj);
	if(speedDailListJobj) json_object_put(speedDailListJobj);
	return;
}

zcfgRet_t zcfgFeDalCallRuleObjEditIidGet(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if (json_object_object_get(Jobj, "CleanAll"))
	{
		cleanAll = json_object_get_boolean(json_object_object_get(Jobj, "CleanAll"));
		if (cleanAll == true) //Michael.20220405.001: Modify the macro constant 'TRUE' to 'true' to avoid Compile Error due to 'TRUE' is undeclared.
		{
			callRuleType = CLEANALL;
		}
		else
		{
			callRuleType = SETRULE;
		}
	}
	else 	// by default callRuleType = 0
	{
		callRuleType = SETRULE;
	}
	return ret;
}

zcfgRet_t zcfgFeDalVoipCleanAllCallRule(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	IID_INIT(CallRuleIid);
	//printf("%s() Enter \n ",__FUNCTION__);

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_SPEED_DIAL_BOOK_LIST, &CallRuleIid, &speedDailListJobj) == ZCFG_SUCCESS) {
		json_object_object_add(speedDailListJobj, "OrigNumber", json_object_new_string(""));
		json_object_object_add(speedDailListJobj, "Name", json_object_new_string(""));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_SPEED_DIAL_BOOK_LIST, &CallRuleIid, multiJobj, speedDailListJobj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_SPEED_DIAL_BOOK_LIST);
			zcfgFeJsonObjFree(speedDailListJobj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(speedDailListJobj);
	}

	//printf("%s(): Append speed dial book list obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}

char *strsplit_getfirst(char **nStr, char *tok){
	char *ret = *nStr;
	if((nStr == NULL) || (*nStr == NULL)){
		return NULL;
	}
	*nStr = strstr(ret, tok);
	if(*nStr != NULL){
		*nStr[0] = '\0';
		*nStr = *nStr + 1;
	}else{
		*nStr = NULL;
	}

	return ret;
}

zcfgRet_t zcfgFeDalVoipSetAllCallRule(struct json_object *Jobj){
	struct json_object *setJson = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter \n ",__FUNCTION__);
	struct json_object *callRule = NULL;
	int index, cmdIndex = 0;
	const char *number = NULL, *name = NULL;

	IID_INIT(CallRuleIid);
	if(json_object_object_get(Jobj, "Index"))
		cmdIndex = 	json_object_get_int(json_object_object_get(Jobj, "Index"));
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_SPEED_DIAL_BOOK_LIST, &CallRuleIid, &speedDailListJobj) == ZCFG_SUCCESS) {
		if(cmdIndex != 0){	//	This condition is for CLI used
			if(CallRuleIid.idx[1] == cmdIndex){
				if(json_object_object_get(Jobj, "Number")){
					number = json_object_get_string(json_object_object_get(Jobj, "Number"));
					json_object_object_add(speedDailListJobj, "OrigNumber", json_object_new_string(number));
				}
				if(json_object_object_get(Jobj, "Name")){
					name = json_object_get_string(json_object_object_get(Jobj, "Name"));
					json_object_object_add(speedDailListJobj, "Name", json_object_new_string(name));
				}
			}
		}
		else{	//	This condition is for GUI used
			index = CallRuleIid.idx[1];
			callRule = json_object_array_get_idx(Jobj, index-1);
			OrigNumber = (char *)json_object_get_string(json_object_object_get(callRule, "Number"));
			Name = (char *)json_object_get_string(json_object_object_get(callRule, "Name"));
			json_object_object_add(speedDailListJobj, "OrigNumber", json_object_new_string(OrigNumber));
			json_object_object_add(speedDailListJobj, "Name", json_object_new_string(Name));
		}
		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_SPEED_DIAL_BOOK_LIST, &CallRuleIid, multiJobj, speedDailListJobj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_SPEED_DIAL_BOOK_LIST);
			zcfgFeJsonObjFree(speedDailListJobj);
			return ZCFG_INTERNAL_ERROR;
		}

		zcfgFeJsonObjFree(speedDailListJobj);
	}

	//printf("%s(): Append speed dial book list obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//rintf("%s() exit \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalVoipCallRuleEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	json_object *speedDialObj = NULL;
	int idx = 0;

	initSpeedDialGlobalObjects();
	zcfgFeDalCallRuleObjEditIidGet(Jobj);
	multiJobj = zcfgFeJsonObjNew();

	if(callRuleType == CLEANALL){
		if((ret = zcfgFeDalVoipCleanAllCallRule(Jobj)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to clean all of speed dial rule \n", __FUNCTION__);
			goto exit;
		}
	}else if(callRuleType == SETRULE){
		speedDialObj = json_object_object_get(Jobj, "speedDial");
		if(json_object_object_get(Jobj, "Index")){
			idx = json_object_get_int(json_object_object_get(Jobj, "Index"));
			speedDialObj = JSON_OBJ_COPY(Jobj);
		}
		if(speedDialObj != NULL || idx != 0){
			if((ret = zcfgFeDalVoipSetAllCallRule(speedDialObj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to set all of speed dial rule \n", __FUNCTION__);
				goto exit;
			}
		}
	}

	if((ret = zcfgFeMultiObjJsonSet(multiJobj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_SPEED_DIAL_BOOK_LIST);
		goto exit;
	}

exit:
	freeAllSpeedDialObjects();
	return ret;
}

zcfgRet_t zcfgFeDalVoipCallRuleDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *speedDialObj = NULL;
	objIndex_t speedDialLid = {0};
	int idx = 0;

	idx = json_object_get_int(json_object_object_get(Jobj, "Index"));
	speedDialLid.level = 2;
	speedDialLid.idx[0] = 1;
	speedDialLid.idx[1] = idx;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SPEED_DIAL_BOOK_LIST, &speedDialLid, &speedDialObj) == ZCFG_SUCCESS){
		json_object_object_add(speedDialObj, "OrigNumber", json_object_new_string(""));
		json_object_object_add(speedDialObj, "Name", json_object_new_string(""));
		zcfgFeObjJsonSet(RDM_OID_SPEED_DIAL_BOOK_LIST, &speedDialLid, speedDialObj, NULL);
		zcfgFeJsonObjFree(speedDialObj);
	}
	
	return ret;
}


zcfgRet_t zcfgFeDalVoipCallRuleGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *booklistJobj = NULL;
	objIndex_t booklistLid = {0};
	int idx = 1;
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_SPEED_DIAL_BOOK_LIST, &booklistLid, &booklistJobj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index", json_object_new_int(idx));
		json_object_object_add(paramJobj, "OrigNumber", JSON_OBJ_COPY(json_object_object_get(booklistJobj, "OrigNumber")));
		json_object_object_add(paramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(booklistJobj, "Name")));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(booklistJobj);
		idx++;
	}
	return ret;
}


zcfgRet_t zcfgFeDalVoipCallRule(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalVoipCallRuleEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "DELETE"))
		ret = zcfgFeDalVoipCallRuleDelete(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalVoipCallRuleGet(Jobj, Jarray, NULL);

	return ret;
}

