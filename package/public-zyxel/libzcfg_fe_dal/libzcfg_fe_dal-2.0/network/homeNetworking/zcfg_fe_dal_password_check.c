#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t CheckPwdAgain_param[]={
	{"oldPw", dalType_string,	0,	0,	NULL, NULL,	dal_Edit},
	{NULL,					0,	0,	0,	NULL},
};

void zcfgFeDalShowCheckPwdAgain(struct json_object *Jarray){
}

zcfgRet_t zcfgFeDal_CheckPwdAgain_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeDal_CheckPwdAgain_Edit(struct json_object *Jobj, char *replyMsg){ 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *accountObj = NULL;
	struct json_object *reqObj = NULL;
	int arrayIdx = 0;
	objIndex_t accountIid = {0};
	const char *username = NULL;
	const char *curPassword = NULL;
	bool enable = false;
	enum json_type jsonType;
	char loginLevel[16] = {0};
	char loginUserName[32] = {0};	
//SHA512_PASSWD
	int authResult = -1;
//SHA512_PASSWD
	if(getLoginUserInfo(Jobj, loginUserName, loginLevel, NULL) != ZCFG_SUCCESS){
		printf("%s() getLoginUserInfo Fail", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	//printf("%s() Jgets(Jobj ,oldPw)=%s\n", __FUNCTION__, Jgets(Jobj ,"oldPw"));
	IID_INIT(accountIid);
	if(!getSpecificObj(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, "Username", json_type_string, loginUserName, &accountIid, &accountObj)){
		if(replyMsg) strcpy(replyMsg, "Request reject");
		return ZCFG_REQUEST_REJECT;
	}
	
	
		if(!strcmp("", Jgets(Jobj ,"oldPw"))){
			curPassword = Jgets(accountObj, "Password");
			//printf("%s() curPassword=%s\n", __FUNCTION__, curPassword);
			if(strcmp(curPassword, Jgets(Jobj ,"oldPw"))!= 0){
				if(replyMsg) strcpy(replyMsg, "Wrong --oldPw value");
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.oldpassword_err2"));
				zcfgFeJsonObjFree(accountObj);
				return ZCFG_NOT_FOUND; //GUI use ZCFG_NOT_FOUND to detect oldPw error.
			}
		}
		else{
			if(!ISSET("oldPw")){
				if(replyMsg) strcpy(replyMsg, "Need --oldPw and --verifyNewpassword to modify Password");
				zcfgFeJsonObjFree(accountObj);
				return ZCFG_DELETE_REJECT;
			}
			
			authResult = zyUtilAuthUserAccount(loginUserName, Jgets(Jobj ,"oldPw"));
			if (authResult == 1) 
			{
				curPassword = Jgets(accountObj, "Password");
				//printf("%s() authResult==1, curPassword=%s\n", __FUNCTION__, curPassword);
				if(strcmp(curPassword, Jgets(Jobj ,"oldPw"))){
					if(replyMsg) strcpy(replyMsg, "Wrong --oldPw value");
					json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.oldpassword_err2"));
					zcfgFeJsonObjFree(accountObj);
					return ZCFG_NOT_FOUND; //GUI use ZCFG_NOT_FOUND to detect oldPw error.
				}
			}
			else if (authResult < 0)
			{
				if(replyMsg) strcpy(replyMsg, "Wrong --oldPw value");
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.oldpassword_err2"));
				zcfgFeJsonObjFree(accountObj);
				return ZCFG_NOT_FOUND; //GUI use ZCFG_NOT_FOUND to detect oldPw error.
			}
			//replaceParam(accountObj, "Password", Jobj, "Password");
		}
	
	return ret;
}

zcfgRet_t zcfgFeDalCheckPwdAgain(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDal_CheckPwdAgain_Edit(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_CheckPwdAgain_Get(Jobj, Jarray, NULL); 
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

