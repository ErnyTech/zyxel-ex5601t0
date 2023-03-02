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
#include "zcfg_fe_dal.h"
#include "zyutil.h"

#define HIGH_LEVEL_ACCOUNT_IDX 1
#define MEDIUM_LEVEL_ACCOUNT_IDX 2
#define LOW_LEVEL_ACCOUNT_IDX 3

#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
static int highGpIdx = 1;
static int mediumGpIdx = 2;
#endif

dal_param_t LOGIN_PRIVILEGE_param[]={		
	{"Enabled", 	 dalType_boolean,	0,	0,	NULL},
	{"Username", 	 dalType_string,	0,	0,	NULL},
	{"Password", 	 dalType_string,	0,	0,	NULL},
	{"EnableQuickStart", 	 dalType_boolean,	0,	0,	NULL},
	{"Page", 	 dalType_string,	0,	0,	NULL},
	{NULL,					0,	0,	0,	NULL},

};

zcfgRet_t zcfgFeDal_LoginPrivilege_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *loginPrivilegeObj = NULL;
	struct json_object *loginPrivilegeAccountObj = NULL;
	struct json_object *reqObjectindex = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t loginPrivilegeIid = {0};
	bool enable = false;
	const char *username = NULL;
	const char *password = NULL;
	const char *oldPassword = NULL;
	bool enablequickstart = false;
	const char *page = NULL;
	int index0 = 0;
	int index1 = 0;
	char loginLevel[16] = {0};
	char loginUserName[32] = {0};

	if(getLoginUserInfo(Jobj, loginUserName, loginLevel, NULL) != ZCFG_SUCCESS){
		printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}
	
	IID_INIT(loginPrivilegeIid);
	reqObjectindex = json_object_array_get_idx(Jobj,0);
	index0 = json_object_get_int(json_object_object_get(reqObjectindex, "Index0"));
	index1 = json_object_get_int(json_object_object_get(reqObjectindex, "Index1"));
	
	if(index0 == HIGH_LEVEL_ACCOUNT_IDX && !strcmp(loginLevel, "high")){
		loginPrivilegeIid.idx[0] = index0;
		loginPrivilegeIid.idx[1] = index1;
		loginPrivilegeIid.level = 2;
		ret = zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginPrivilegeIid, &loginPrivilegeObj);
		if(ret == ZCFG_SUCCESS){
			reqObject = json_object_array_get_idx(Jobj,1);
			enable = json_object_get_boolean(json_object_object_get(reqObject, "Enabled"));
			username = json_object_get_string(json_object_object_get(reqObject, "Username"));
			password = json_object_get_string(json_object_object_get(reqObject, "Password"));
#ifndef HAAAA_CUSTOMIZATION
			oldPassword = json_object_get_string(json_object_object_get(reqObject, "oldPassword"));
#endif
			if((json_object_object_get(reqObject, "EnableQuickStart"))){
				enablequickstart = json_object_get_boolean(json_object_object_get(reqObject, "EnableQuickStart"));
			}
			if(username && password && strlen(username) && strlen(password))
			{
#ifndef HAAAA_CUSTOMIZATION
				int authResult = -1;

				if(oldPassword == NULL || strlen(oldPassword)==0) {
					if(replyMsg) strcpy(replyMsg, "Wrong --oldPw value");
					ret =  ZCFG_NOT_FOUND; //GUI use ZCFG_NOT_FOUND to detect oldPw error.
				} else {
					authResult = zyUtilAuthUserAccount(username, oldPassword);
					if (authResult == 1) {
						if(strcmp(Jgets(loginPrivilegeObj, "Password"), oldPassword)){
							if(replyMsg) strcpy(replyMsg, "Wrong --oldPw value");
							ret =  ZCFG_NOT_FOUND; //GUI use ZCFG_NOT_FOUND to detect oldPw error.
						}
					}
					else if (authResult < 0) {
						if(replyMsg) strcpy(replyMsg, "Wrong --oldPw value");
						ret = ZCFG_NOT_FOUND; //GUI use ZCFG_NOT_FOUND to detect oldPw error.
					}
				}
#ifdef PACKAGE_WEB_GUI_VUE
				if(ret != ZCFG_SUCCESS){
					if(replyMsg) 
						strcpy(replyMsg, "Wrong --oldPw value");
					json_object_object_add(reqObject, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.oldpassword_err2"));
					zcfgFeJsonObjFree(loginPrivilegeObj);
					return ret;
				}
#endif
				if(ret == ZCFG_SUCCESS) {
					json_object_object_add(loginPrivilegeObj, "Enabled", json_object_new_boolean(enable));
					json_object_object_add(loginPrivilegeObj, "Username", json_object_new_string(username));
					json_object_object_add(loginPrivilegeObj, "Password", json_object_new_string(password));
					if((json_object_object_get(reqObject, "EnableQuickStart"))){
						json_object_object_add(loginPrivilegeObj, "EnableQuickStart", json_object_new_boolean(enablequickstart));
					}
#ifdef MSTC_TAAAG_MULTI_USER					
					cleanCurrUser(username, oldPassword);
#endif					
					zcfgFeObjJsonSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginPrivilegeIid, loginPrivilegeObj, NULL);
				}
#else
				json_object_object_add(loginPrivilegeObj, "Enabled", json_object_new_boolean(enable));
				json_object_object_add(loginPrivilegeObj, "Username", json_object_new_string(username));
				json_object_object_add(loginPrivilegeObj, "Password", json_object_new_string(password));
				json_object_object_add(loginPrivilegeObj, "EnableQuickStart", json_object_new_boolean(enablequickstart));
				zcfgFeObjJsonSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginPrivilegeIid, loginPrivilegeObj, NULL);
#endif
			}
		}
	}
	else if((index0 == MEDIUM_LEVEL_ACCOUNT_IDX && (!strcmp(loginLevel, "medium") || !strcmp(loginLevel, "high"))) || 
	  index0 == LOW_LEVEL_ACCOUNT_IDX){
		loginPrivilegeIid.idx[0] = index0;
		loginPrivilegeIid.level = 1;
		reqObject = json_object_array_get_idx(Jobj,1);
		page = json_object_get_string(json_object_object_get(reqObject, "Page"));
		enablequickstart = json_object_get_boolean(json_object_object_get(reqObject, "EnableQuickStart"));
		ret = zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG_GP, &loginPrivilegeIid, &loginPrivilegeObj);
		if(ret == ZCFG_SUCCESS){

			json_object_object_add(loginPrivilegeObj, "Page", json_object_new_string(page));
			json_object_object_add(loginPrivilegeObj, "HiddenPage", json_object_new_string(""));
			zcfgFeObjJsonSet(RDM_OID_ZY_LOG_CFG_GP, &loginPrivilegeIid, loginPrivilegeObj, NULL);
		}

		if(json_object_object_get(reqObject, "EnableQuickStart")){
			IID_INIT(loginPrivilegeIid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginPrivilegeIid, &loginPrivilegeAccountObj)== ZCFG_SUCCESS){
				if(loginPrivilegeIid.idx[0] == index0){
					json_object_object_add(loginPrivilegeAccountObj, "EnableQuickStart", json_object_new_boolean(enablequickstart));
					zcfgFeObjJsonSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginPrivilegeIid, loginPrivilegeAccountObj, NULL);
				}
				zcfgFeJsonObjFree(loginPrivilegeAccountObj);
			}
		}
#ifdef HAAAA_CUSTOMIZATION
		//__ZyXEL__, JackYu, modify account also, for HT requires enabling account password modification at login previlige page
		loginPrivilegeIid.idx[0] = index0;
		loginPrivilegeIid.idx[1] = index1;
		loginPrivilegeIid.level = 2;
		ret = zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginPrivilegeIid, &loginPrivilegeObj);
		if(ret == ZCFG_SUCCESS){
			reqObject = json_object_array_get_idx(Jobj,2);
			enable = json_object_get_boolean(json_object_object_get(reqObject, "Enabled"));
			username = json_object_get_string(json_object_object_get(reqObject, "Username"));
			password = json_object_get_string(json_object_object_get(reqObject, "Password"));
			enablequickstart = json_object_get_boolean(json_object_object_get(reqObject, "EnableQuickStart"));

			//validate
			if(username && password && strlen(username) && strlen(password))
			{
				json_object_object_add(loginPrivilegeObj, "Enabled", json_object_new_boolean(enable));
				json_object_object_add(loginPrivilegeObj, "Username", json_object_new_string(username));
				json_object_object_add(loginPrivilegeObj, "Password", json_object_new_string(password));
				json_object_object_add(loginPrivilegeObj, "EnableQuickStart", json_object_new_boolean(enablequickstart));
				zcfgFeObjJsonSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginPrivilegeIid, loginPrivilegeObj, NULL);
			}
		}
#endif
	}
#if 0 /* <JoannaSu>, <20191211>, <Avoid the invalid free as obj gets from json_object_array_get_idx> */	
	zcfgFeJsonObjFree(reqObjectindex);
	zcfgFeJsonObjFree(reqObject);
#endif
	zcfgFeJsonObjFree(loginPrivilegeObj);
	return ret;

}

zcfgRet_t zcfgFeDal_LoginPrivilege_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *loginPrivilegeObj = NULL;
	objIndex_t loginPrivilegeIid = {0};
	char loginLevel[16] = {0};
	char loginUserName[32] = {0};

	if(getLoginUserInfo(Jobj, loginUserName, loginLevel, NULL) != ZCFG_SUCCESS){
		printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}

	IID_INIT(loginPrivilegeIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &loginPrivilegeIid, &loginPrivilegeObj) == ZCFG_SUCCESS){
#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
		if(!strcmp(loginLevel, "high") || !strcmp(loginLevel, "medium")){
			if( highGpIdx == loginPrivilegeIid.idx[0]){
				zcfgFeJsonObjFree(loginPrivilegeObj);
				continue;
			}
		}else{
			if( (highGpIdx == loginPrivilegeIid.idx[0]) || (mediumGpIdx == loginPrivilegeIid.idx[0])){
				zcfgFeJsonObjFree(loginPrivilegeObj);
				continue;
			}
		}
#else
		if(!strcmp(loginLevel, "high")){
			//do nothing
		}
		else if(!strcmp(loginLevel, "medium")){
			if(loginPrivilegeIid.idx[0] == HIGH_LEVEL_ACCOUNT_IDX){
				zcfgFeJsonObjFree(loginPrivilegeObj);
				continue;
			}
		}else if(!strcmp(loginLevel, "low")){
			if( (loginPrivilegeIid.idx[0] == HIGH_LEVEL_ACCOUNT_IDX) || (loginPrivilegeIid.idx[0] == MEDIUM_LEVEL_ACCOUNT_IDX)){
				zcfgFeJsonObjFree(loginPrivilegeObj);
				continue;
			}
		}
		else{
			break;
		}
#endif

		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index0", json_object_new_int(loginPrivilegeIid.idx[0]));
		json_object_object_add(paramJobj, "Index1", json_object_new_int(loginPrivilegeIid.idx[1]));
		json_object_object_add(paramJobj, "Enabled", JSON_OBJ_COPY(json_object_object_get(loginPrivilegeObj, "Enabled")));
		json_object_object_add(paramJobj, "Username", JSON_OBJ_COPY(json_object_object_get(loginPrivilegeObj, "Username")));
		//json_object_object_add(paramJobj, "Password", JSON_OBJ_COPY(json_object_object_get(loginPrivilegeObj, "Password")));
		json_object_object_add(paramJobj, "Password", json_object_new_string(""));
		json_object_object_add(paramJobj, "EnableQuickStart", JSON_OBJ_COPY(json_object_object_get(loginPrivilegeObj, "EnableQuickStart")));

		json_object_object_add(paramJobj, "Privilege", JSON_OBJ_COPY(json_object_object_get(loginPrivilegeObj, "Privilege")));

		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(loginPrivilegeObj);
	}

	IID_INIT(loginPrivilegeIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP, &loginPrivilegeIid, &loginPrivilegeObj) == ZCFG_SUCCESS){
#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
		if(!strcmp(loginLevel, "high") || !strcmp(loginLevel, "medium")){
			if( highGpIdx == loginPrivilegeIid.idx[0]){
				zcfgFeJsonObjFree(loginPrivilegeObj);
				continue;
			}
		}else{
			if( (highGpIdx == loginPrivilegeIid.idx[0]) || (mediumGpIdx == loginPrivilegeIid.idx[0])){
				zcfgFeJsonObjFree(loginPrivilegeObj);
				continue;
			}
		}
#else
		if(!strcmp(loginLevel, "high")){
			//do nothing
		}
		else if(!strcmp(loginLevel, "medium")){
			if(loginPrivilegeIid.idx[0] == HIGH_LEVEL_ACCOUNT_IDX){
				zcfgFeJsonObjFree(loginPrivilegeObj);
				continue;
			}
		}else if(!strcmp(loginLevel, "low")){
			if( (loginPrivilegeIid.idx[0] == HIGH_LEVEL_ACCOUNT_IDX) || (loginPrivilegeIid.idx[0] == MEDIUM_LEVEL_ACCOUNT_IDX)){
				zcfgFeJsonObjFree(loginPrivilegeObj);
				continue;
			}
		}
		else{
			break;
		}
#endif

		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index0", json_object_new_int(loginPrivilegeIid.idx[0]));
		json_object_object_add(paramJobj, "Index1", json_object_new_int(loginPrivilegeIid.idx[1]));
		json_object_object_add(paramJobj, "Page", JSON_OBJ_COPY(json_object_object_get(loginPrivilegeObj, "Page")));
		json_object_object_add(paramJobj, "HiddenPage", JSON_OBJ_COPY(json_object_object_get(loginPrivilegeObj, "HiddenPage")));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(loginPrivilegeObj);
	}
	
	return ret;
}

zcfgRet_t zcfgFeDalLoginPrivilege(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
	char loginLevel[16] = {0};
	char loginUserName[32] = {0};
#endif

#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(getLoginUserInfo(Jobj, loginUserName, loginLevel, NULL) != ZCFG_SUCCESS){
		printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}
#endif

	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_LoginPrivilege_Edit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_LoginPrivilege_Get(Jobj, Jarray, NULL);
	}
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

