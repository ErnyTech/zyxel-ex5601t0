
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

#define SETCALLBLOCK			0
#define CLEANALLCALLBLOCK		1

dal_param_t VOIP_CALLBLOCK_param[]={
    {"Index",         dalType_int,        1,   20,  NULL,   NULL,   0},
    {"Enable",        dalType_boolean,    0,    0,  NULL,   NULL,   0},
    {"Direction",     dalType_string,     0,    0,  NULL,   NULL,   0},
    {"Description",   dalType_string,     0,    0,  NULL,   NULL,   0},
    {"PhoneNumber",   dalType_string,     0,    0,  NULL,   NULL,   0},
    {NULL,            0,                  0,    0,  NULL,   NULL,   0},
};

struct json_object *multiJobj;
struct json_object *callBlockListJobj;

objIndex_t CallBlockIid = {0};

bool cleanAll;
bool callBlockType;

void initCallBlockGlobalObjects(){
	multiJobj = NULL;
	callBlockListJobj = NULL;
	return;
}

zcfgRet_t zcfgFeDalCallBlockObjEditIidGet(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;

	cleanAll = json_object_get_boolean(json_object_object_get(Jobj, "CleanAll"));
	if (cleanAll == true) //Michael.20220405.001: Modify the macro constant 'TRUE' to 'true' to avoid Compile Error due to 'TRUE' is undeclared.
	{
		callBlockType = CLEANALLCALLBLOCK;
	}
	else
	{
		callBlockType = SETCALLBLOCK;
	}

	return ret;
}

void freeAllCallBlockObjects(){
	if(multiJobj) json_object_put(multiJobj);
	if(callBlockListJobj) json_object_put(callBlockListJobj);

	return;
}

zcfgRet_t zcfgFeDalVoipCleanAllCallBlock(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	IID_INIT(CallBlockIid);
	//printf("%s() Enter \n ",__FUNCTION__);

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_CALL_POLICY_BOOK_LIST, &CallBlockIid, &callBlockListJobj) == ZCFG_SUCCESS) {
		json_object_object_add(callBlockListJobj, "Enable", json_object_new_boolean(false));
		json_object_object_add(callBlockListJobj, "Direction", json_object_new_string("Incoming"));
		json_object_object_add(callBlockListJobj, "PhoneNumber", json_object_new_string(""));
		json_object_object_add(callBlockListJobj, "Description", json_object_new_string(""));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_CALL_POLICY_BOOK_LIST, &CallBlockIid, multiJobj, callBlockListJobj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_CALL_POLICY_BOOK_LIST);
			zcfgFeJsonObjFree(callBlockListJobj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(callBlockListJobj);
	}

	//printf("%s(): Append call policy book list obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;

}

//Anna 20210115 Modify zcfgFeDalVoipSetAllCallBlock() for setting parameters of call policy book list object via GUI call block page.
zcfgRet_t zcfgFeDalVoipSetAllCallBlock(struct json_object *Jobj){
	struct json_object *setJson = NULL;
	struct json_object *callBlock = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	int index;
	json_bool enable;
	char *direction;
	char *phoneNumber;
	char *description;

	dbg_printf("%s() Enter \n ",__FUNCTION__);


	/***Handle the feature: Block International Call.***/
#if 1 //Add to support the feature: Block International Call.
	//const char* PhoneNumberCheckLogic = NULL; //Unnecessary to consider 'PhoneNumberCheckLogic' in the Data Value Setting direction.
	const char* blockIntlCall = NULL;
	objIndex_t policyBookIid;
	struct json_object *bookObj = NULL;

	//PhoneNumberCheckLogic = json_object_get_string(json_object_object_get(Jobj, "PhoneNumberCheckLogic")); //Unnecessary to consider 'PhoneNumberCheckLogic' in the Data Value Setting direction.
	blockIntlCall = json_object_get_string(json_object_object_get(Jobj, "blockIntlCall"));

	IID_INIT(policyBookIid);
	policyBookIid.level = 1;
	policyBookIid.idx[0] = 1;

	if (zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CALL_POLICY_BOOK, &policyBookIid, &bookObj) == ZCFG_SUCCESS)
	{
		//json_object_object_add(bookObj, "PhoneNumberCheckLogic", json_object_new_string(PhoneNumberCheckLogic)); //Unnecessary to consider 'PhoneNumberCheckLogic' in the Data Value Setting direction.
		json_object_object_add(bookObj, "BlockInternationalCall", json_object_new_string(blockIntlCall));

		if ((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_CALL_POLICY_BOOK, &policyBookIid, multiJobj, bookObj)) == NULL)
		{
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_CALL_POLICY_BOOK);
			zcfgFeJsonObjFree(bookObj);
			return ZCFG_INTERNAL_ERROR;
		}

		zcfgFeJsonObjFree(bookObj);
	}
#endif // 1.


	/***Handle the Call Policy (Phone)Book's Rule List Entry.***/
	Jobj = json_object_object_get(Jobj, "CallBlock");

	IID_INIT(CallBlockIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_CALL_POLICY_BOOK_LIST, &CallBlockIid, &callBlockListJobj) == ZCFG_SUCCESS) {
		index = CallBlockIid.idx[1];
		callBlock = json_object_array_get_idx(Jobj, index-1);
		enable = json_object_get_boolean(json_object_object_get(callBlock, "Enable"));
		direction = (char *)json_object_get_string(json_object_object_get(callBlock, "Direction"));
		phoneNumber = (char *)json_object_get_string(json_object_object_get(callBlock, "PhoneNumber"));
		description = (char *)json_object_get_string(json_object_object_get(callBlock, "Description"));

		dbg_printf("(%d) %s %s %s %s\n",index,enable?"true":"false",direction,phoneNumber,description);

		json_object_object_add(callBlockListJobj, "Enable", json_object_new_boolean(enable));
		json_object_object_add(callBlockListJobj, "Direction", json_object_new_string(direction));
		json_object_object_add(callBlockListJobj, "PhoneNumber", json_object_new_string(phoneNumber));
		json_object_object_add(callBlockListJobj, "Description", json_object_new_string(description));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_CALL_POLICY_BOOK_LIST, &CallBlockIid, multiJobj, callBlockListJobj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_CALL_POLICY_BOOK_LIST);
			zcfgFeJsonObjFree(callBlockListJobj);
			return ZCFG_INTERNAL_ERROR;
		}

		zcfgFeJsonObjFree(callBlockListJobj);
	}


	dbg_printf("%s(): Append call policy book list obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	dbg_printf("%s() exit \n ",__FUNCTION__);
	return ret;

}

//Anna 20210115 Add zcfgFeDalVoipSetOneCallBlock() for setting parameters of specific call policy book list object via dalcmd.
zcfgRet_t zcfgFeDalVoipSetOneCallBlock(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int index;
	json_object *tempObj;

	dbg_printf("%s() Enter \n ",__FUNCTION__);

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));

	IID_INIT(CallBlockIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_CALL_POLICY_BOOK_LIST, &CallBlockIid, &callBlockListJobj) == ZCFG_SUCCESS) {
		if (index == CallBlockIid.idx[1]) {
			tempObj = json_object_object_get(Jobj, "Enable");
			if (tempObj) {
				json_object_object_add(callBlockListJobj, "Enable", json_object_new_boolean(json_object_get_boolean(tempObj)));
			}
			tempObj = json_object_object_get(Jobj, "Direction");
			if (tempObj) {
				json_object_object_add(callBlockListJobj, "Direction", json_object_new_string(json_object_get_string(tempObj)));
			}
			tempObj = json_object_object_get(Jobj, "PhoneNumber");
			if (tempObj) {
				json_object_object_add(callBlockListJobj, "PhoneNumber", json_object_new_string(json_object_get_string(tempObj)));
			}
			tempObj = json_object_object_get(Jobj, "Description");
			if (tempObj) {
				json_object_object_add(callBlockListJobj, "Description", json_object_new_string(json_object_get_string(tempObj)));
			}

			dbg_printf("%s(): set --- %s \n ",__FUNCTION__,json_object_to_json_string(callBlockListJobj));

			if((ret = zcfgFeObjJsonSet(RDM_OID_CALL_POLICY_BOOK_LIST, &CallBlockIid, callBlockListJobj, NULL)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_CALL_POLICY_BOOK_LIST);
			}
			zcfgFeJsonObjFree(callBlockListJobj);
			break;
		}

		zcfgFeJsonObjFree(callBlockListJobj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalVoipCallBlockEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	dbg_printf("%s(): input --- %s \n ",__FUNCTION__,json_object_to_json_string(Jobj));

	initCallBlockGlobalObjects();
	zcfgFeDalCallBlockObjEditIidGet(Jobj);
	multiJobj = zcfgFeJsonObjNew();

	if (json_object_object_get(Jobj, "CleanAll"))
	{	//for WebGUI edit.
		if(callBlockType == CLEANALLCALLBLOCK){
			if((ret = zcfgFeDalVoipCleanAllCallBlock(Jobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to edit call policy book list obj \n", __FUNCTION__);
				goto exit;
			}
		}else if(callBlockType == SETCALLBLOCK){
			if((ret = zcfgFeDalVoipSetAllCallBlock(Jobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to edit call policy book list obj \n", __FUNCTION__);
				goto exit;
			}
		}

		if((ret = zcfgFeMultiObjJsonSet(multiJobj, NULL)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_CALL_POLICY_BOOK_LIST);
			goto exit;
		}
	}
	else
	{	//for dalcmd edit.
		if((ret = zcfgFeDalVoipSetOneCallBlock(Jobj)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to edit one of the call policy book list obj \n", __FUNCTION__);
			goto exit;
		}
	}

exit:
	freeAllCallBlockObjects();
	return ret;
}

//Anna 20210115 Add zcfgFeDalVoipCallBlockDelete(), zcfgFeDalVoipCallBlockGet() and zcfgFeDalShowVoipCallBlock() for call blocking feature.
zcfgRet_t zcfgFeDalVoipCallBlockDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *callBlockObj = NULL;
	objIndex_t callBlockLid = {0};
	int idx = 0;

	idx = json_object_get_int(json_object_object_get(Jobj, "Index"));
	callBlockLid.level = 2;
	callBlockLid.idx[0] = 1;
	callBlockLid.idx[1] = idx;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CALL_POLICY_BOOK_LIST, &callBlockLid, &callBlockObj) == ZCFG_SUCCESS){
		json_object_object_add(callBlockObj, "Enable", json_object_new_boolean(false));
		json_object_object_add(callBlockObj, "Direction", json_object_new_string("Incoming"));
		json_object_object_add(callBlockObj, "PhoneNumber", json_object_new_string(""));
		json_object_object_add(callBlockObj, "Description", json_object_new_string(""));
		zcfgFeObjJsonSet(RDM_OID_CALL_POLICY_BOOK_LIST, &callBlockLid, callBlockObj, NULL);
		zcfgFeJsonObjFree(callBlockObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalVoipCallBlockGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *booklistJobj = NULL;
	struct json_object *callBlockList = NULL;
	struct json_object *policyBookObj = NULL;
	struct json_object *responseObj = NULL;

	objIndex_t booklistIid;
	objIndex_t policyBookIid;
	int idx = 1;
	const char *PhoneNumberCheckLogic = NULL;
	const char *blockInternationalCall = NULL;


	responseObj = json_object_new_object();
	json_object_array_add(Jarray, responseObj);


	/***Handle the feature: Block International Call.***/
	IID_INIT(policyBookIid);
	policyBookIid.level = 1;
	policyBookIid.idx[0] = 1;
	if (zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CALL_POLICY_BOOK, &policyBookIid, &policyBookObj) == ZCFG_SUCCESS)
	{
		PhoneNumberCheckLogic = json_object_get_string(json_object_object_get(policyBookObj, "PhoneNumberCheckLogic"));
		blockInternationalCall = json_object_get_string(json_object_object_get(policyBookObj, "BlockInternationalCall"));

		json_object_object_add(responseObj, "PhoneNumberCheckLogic", json_object_new_string(PhoneNumberCheckLogic));
		json_object_object_add(responseObj, "blockIntlCall", json_object_new_string(blockInternationalCall));

		zcfgFeJsonObjFree(policyBookObj);
	}


	/***Handle the Call Policy (Phone)Book's Rule List Entry.***/
	callBlockList = json_object_new_array();

	IID_INIT(booklistIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_CALL_POLICY_BOOK_LIST, &booklistIid, &booklistJobj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index", json_object_new_int(idx));
		json_object_object_add(paramJobj, "Enable", json_object_new_boolean(json_object_get_boolean(json_object_object_get(booklistJobj, "Enable"))));
		json_object_object_add(paramJobj, "Direction", JSON_OBJ_COPY(json_object_object_get(booklistJobj, "Direction")));
		json_object_object_add(paramJobj, "PhoneNumber", JSON_OBJ_COPY(json_object_object_get(booklistJobj, "PhoneNumber")));
		json_object_object_add(paramJobj, "Description", JSON_OBJ_COPY(json_object_object_get(booklistJobj, "Description")));
		json_object_array_add(callBlockList, paramJobj);
		zcfgFeJsonObjFree(booklistJobj);
		idx++;
	}

	if (idx > 1)
	{
		json_object_object_add(responseObj, "callBlockList", callBlockList);
	}

	return ret;
}

zcfgRet_t zcfgFeDalVoipCallBlock(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalVoipCallBlockEdit(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalVoipCallBlockGet(Jobj, Jarray, NULL);
	else if(!strcmp(method, "DELETE"))
		ret = zcfgFeDalVoipCallBlockDelete(Jobj, NULL);

	return ret;
}

void zcfgFeDalShowVoipCallBlock(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-10s %-10s %-15s %-20s %-10s \n","Index","Enable","Direction","Number","Description");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-10s %-10s %-15s %-20s %-10s \n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_boolean(json_object_object_get(obj, "Enable")) ? "true" : "false",
			json_object_get_string(json_object_object_get(obj, "Direction")),
			json_object_get_string(json_object_object_get(obj, "PhoneNumber")),
			json_object_get_string(json_object_object_get(obj, "Description")));
	}
}


