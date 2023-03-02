#include <ctype.h>
#include <json/json.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_dal.h"
#include "zyutil.h"

int highGpIdx = 1;
int mediumGpIdx = 2;
int lowGpIdx = 3;
#ifdef ABUU_CUSTOMIZATION
int vullGpIdx = 4;
int telek0mGpIdx = 5;
#endif
#ifdef ACAU_CUSTOMIZATION
int ftpGpIdx = 4;
#endif

dal_param_t USER_ACCOUNT_param[] =  //RDM_OID_ZY_LOG_CFG_GP_ACCOUNT
{
	{"Username",			dalType_string,  1, 	32,		NULL,	NULL,	dal_Add|dal_Edit|dal_Delete},
#ifdef ABUU_CUSTOMIZATION
	{"FirstIndex",			dalType_int,	 1, 	5,		NULL,	NULL,	dalcmd_Forbid}, // group index 
	{"SecondIndex", 		dalType_int,	 1, 	10,		NULL,	NULL,	dalcmd_Forbid}, // account index
#else
	{"FirstIndex",			dalType_int,	 1, 	3,		NULL,	NULL,	dalcmd_Forbid}, // group index 
	{"SecondIndex", 		dalType_int,	 1, 	4,		NULL,	NULL,	dalcmd_Forbid}, // account index
#endif
	{"Enabled", 			dalType_boolean, 0, 	0,		NULL,	NULL,	0}, //default true
	{"EnableQuickStart",	dalType_boolean, 0, 	0,		NULL,	NULL,	dalcmd_Forbid}, //default value based on schema.json
	{"oldPw",				dalType_string,  0, 	0,		NULL,	NULL,	dal_Add_ignore},
	{"Password",			dalType_string,  0, 	0,		NULL,	NULL,	dal_Add},
	{"verifyNewpassword",	dalType_string,  0, 	0,		NULL,	NULL,	dal_Add},
	{"AccountRetryTime",	dalType_int,	 0, 	5,		NULL,	NULL,	0},
	{"AccountIdleTime", 	dalType_int,	 60, 	3600,	NULL,	NULL,	0},
#ifdef WIND_ITALY_CUSTOMIZATION
	{"AccountLockTime", 	dalType_int,	 180,	5400,	NULL,	NULL,	0},
#elif defined(ACAU_CUSTOMIZATION)
	{"AccountLockTime", 	dalType_int,	 30,	5400,	NULL,	NULL,	0},
#else
	{"AccountLockTime", 	dalType_int,	 300, 	5400,	NULL,	NULL,	0},
#endif
#ifdef ABUU_CUSTOMIZATION
	{"group",				dalType_string,  0, 	0,		NULL,	"Administrator|User|Vull|Telek0m", dal_Add|dal_Edit_ignore}, //depend on login acocunt, see paramPrevilegeChange(), defatul is "User"
#elif defined(ACAU_CUSTOMIZATION)
	{"group",				dalType_string,  0, 	0,		NULL,	"Administrator|User|FTPS", dal_Add|dal_Edit_ignore}, //depend on login acocunt, see paramPrevilegeChange(), defatul is "User"
#elif defined(MSTC_TAAAG_MULTI_USER) /*__MSTC__, Linda, CPE User Definitions and Rights. */
	{"group",				dalType_string,  0, 	0,		NULL,	"Root|Administrator", dal_Add|dal_Edit_ignore}, //depend on login acocunt, see paramPrevilegeChange(), defatul is "User"
#else
	{"group",				dalType_string,  0, 	0,		NULL,	"Administrator|User", dal_Add|dal_Edit_ignore}, //depend on login acocunt, see paramPrevilegeChange(), defatul is "User"
#endif
	{"Index",				dalType_string,  0, 	0,		NULL,	NULL,	dal_Add_ignore|dal_Edit_ignore},
#ifdef ZYXEL_REMOTE_ACCESS_PRIVILEGE
	{"RemoteAccessPrivilege",dalType_string, 0, 	0,		NULL,	NULL,	dalcmd_Forbid},
#endif
	{NULL,					0,				 0, 	0,		NULL,	NULL,	0}
};

#ifdef CONFIG_ZYXEL_TR140
dal_param_t TTSAMBA_ACCOUNT_param[] =  //RDM_OID_ZY_LOG_CFG_GP_ACCOUNT
{
	{"Username",			dalType_string,  1, 	16,		NULL,	NULL,	dal_Add|dal_Edit|dal_Delete},
	{"Enabled", 			dalType_boolean, 0, 	0,		NULL,	NULL,	0}, //default true
	{"EnableQuickStart",	dalType_boolean, 0, 	0,		NULL,	NULL,	dalcmd_Forbid}, //default value based on schema.json
	{"Password",			dalType_string,  0, 	0,		NULL,	NULL,	dal_Add},
	{"verifyNewpassword",	dalType_string,  0, 	0,		NULL,	NULL,	dal_Add},
	{NULL,					0,				 0, 	0,		NULL,	NULL,	0}
};
#endif

void zcfgFeDalShowUserAccount(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	int AccountRetryTime, AccountIdleTime, AccountLockTime;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	//printf("%-10s %-10s %-15s %-15s %-15s %-15s %-15s\n", "Index", "Active", "User Name", "Retry Times", "Idle Timeout", "Lock Period", "Group");
	printf("%-15s %-10s %-15s %-22s %-22s %-15s\n", "User Name", "Active", "Retry Times", "Idle Timeout(second)", "Lock Period(second)", "Group");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		//printf("%-10s ",json_object_get_string(json_object_object_get(obj, "Index")));
		printf("%-15s ",Jgets(obj, "Username"));
		if(Jgetb(obj, "Enabled"))
			printf("%-10s ","Enable");
		else
			printf("%-10s ","Disable");

		AccountRetryTime= Jgeti(obj, "AccountRetryTime");
		AccountIdleTime= Jgeti(obj, "AccountIdleTime");
		AccountLockTime= Jgeti(obj, "AccountLockTime");
		printf("%-15d ", AccountRetryTime);
		printf("%-22d ", AccountIdleTime);
		printf("%-22d ", AccountLockTime);
		printf("%-15s ",json_object_get_string(json_object_object_get(obj, "group")));
#ifdef ZYXEL_REMOTE_ACCESS_PRIVILEGE
		printf("%-15s ",Jgets(obj, "RemoteAccessPrivilege"));
#endif		
		printf("\n");
	}
	
}

#if 0 //use username as key, not use index
static zcfgRet_t index_to_iid(int index, objIndex_t &iid){
	struct json_object *obj = NULL;
	int idx = 0;

	IID_INIT(*iid);
	iid->level=2;
	if(!strcmp(loginLevel, "high") || !strcmp(loginLevel, "medium"))
		iid->idx[0]=mediumGpIdx;
	else
		iid->idx[0]=lowGpIdx;

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, iid, &obj) == ZCFG_SUCCESS) {
		idx++;
		zcfgFeJsonObjFree(obj);
		if(idx == index){
			return ZCFG_SUCCESS;
		}
	}
	return ZCFG_NOT_FOUND;
}
#endif

/*
 * For current design, high level account data not showen on GUI
 * high and medium level user get medium and low level account data
 * low level user get only low level account data
 *
 * Note: the index is not the same for different level login user
 * Index to iid maping mechanism need be the same for zcfgFeDal_User_Account_Get() and index_to_iid()
*/
zcfgRet_t zcfgFeDal_User_Account_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;
	char loginLevel[16] = {0};
	int index = 0;

	if(getLoginUserInfo(Jobj, NULL, loginLevel, NULL) != ZCFG_SUCCESS){
		if(replyMsg) strcpy(replyMsg, "Internal Error, can't get account information");
		dbg_printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}
	
	if (json_object_object_get(Jobj, "loginLevel") != NULL){
		IID_INIT(iid);
#ifdef CONFIG_ABZQ_CUSTOMIZATION
		printf("%s(157): loginLevel: %s\n",__FUNCTION__, json_object_get_string(json_object_object_get(Jobj, "Level")));
#endif
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP, &iid, &obj) == ZCFG_SUCCESS) {
			if(!strcmp( json_object_get_string(json_object_object_get(obj, "Level")),loginLevel) ){
				pramJobj = json_object_new_object();
				replaceParam(pramJobj, "GP_Privilege", obj, "GP_Privilege");
				json_object_array_add(Jarray, pramJobj);
				zcfgFeJsonObjFree(obj);
				break;
			}
			zcfgFeJsonObjFree(obj);
		}
		
	}else{

		IID_INIT(iid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &iid, &obj) == ZCFG_SUCCESS) {
#if 0 //def MSTC_TAAAG_MULTI_USER
			char loginUserName[32] = {0};
			const char *username=NULL;
			if(getLoginUserInfo(Jobj, loginUserName, loginLevel) != ZCFG_SUCCESS){
				dbg_printf("getLoginUserInfo Fail");
				if(replyMsg) strcpy(replyMsg, "Internal Error, can't get login user information");
				return ZCFG_INTERNAL_ERROR;
			}
			
			if(Jget(obj, "Username") != NULL)
				username = Jgets(obj, "Username");
				
			if(!strcmp(username, loginUserName)){
#else
#ifdef ABUU_CUSTOMIZATION
			// if not vull
			if(strcmp(loginLevel, "vull")){
				if(vullGpIdx == iid.idx[0] || highGpIdx == iid.idx[0]){
					zcfgFeJsonObjFree(obj);
					index++;
					continue;
				}
				if(!strcmp(loginLevel, "medium")){
					if( telek0mGpIdx == iid.idx[0]){
						zcfgFeJsonObjFree(obj);
						index++;
						continue;
					}
				}
				else if(!strcmp(loginLevel, "telek0m")){
					if( (mediumGpIdx == iid.idx[0]) ){ 
						zcfgFeJsonObjFree(obj);
						index++;
						continue;
					}
				}
				else if(!strcmp(loginLevel, "low")){
					if( (mediumGpIdx == iid.idx[0]) || (telek0mGpIdx == iid.idx[0])){
						zcfgFeJsonObjFree(obj);
						index++;
						continue;
					}
				}
			}
			// if vull
			else{
				if(highGpIdx == iid.idx[0]){
					zcfgFeJsonObjFree(obj);
					index++;
					continue;
				}
			}
#else
			if(!strcmp(loginLevel, "high") || !strcmp(loginLevel, "medium")){
#ifndef MSTC_TAAAG_MULTI_USER			
				if( highGpIdx == iid.idx[0]){
					zcfgFeJsonObjFree(obj);
					index++;
					continue;
				}
#else
				if( !strcmp(loginLevel, "medium")){
				if( highGpIdx == iid.idx[0]){
					zcfgFeJsonObjFree(obj);
					index++;
					continue;
				}
				}
#endif				
			}else{
				if( (highGpIdx == iid.idx[0]) || (mediumGpIdx == iid.idx[0])){
					zcfgFeJsonObjFree(obj);
					index++;
					continue;
				}
			}
#endif //ABUU_CUSTOMIZATION
#endif		
			
			pramJobj = json_object_new_object();
			json_object_object_add(pramJobj, "Index", json_object_new_int(index));
			replaceParam(pramJobj, "Enabled", obj, "Enabled");
			replaceParam(pramJobj, "Username", obj, "Username");
			replaceParam(pramJobj, "AccountRetryTime", obj, "AccountRetryTime");
			replaceParam(pramJobj, "AccountIdleTime", obj, "AccountIdleTime");
			replaceParam(pramJobj, "AccountLockTime", obj, "AccountLockTime");
#ifdef ZYXEL_PERIODICALLY_CHANGE_PASSWORD
			replaceParam(pramJobj, "PeriodicChangePasswordEnable", obj, "PeriodicChangePasswordEnable");
			replaceParam(pramJobj, "ChangePasswordInterval", obj, "ChangePasswordInterval");
#endif
	#ifdef ZYXEL_REMOTE_ACCESS_PRIVILEGE
			replaceParam(pramJobj, "RemoteAccessPrivilege", obj, "RemoteAccessPrivilege");
	#endif
	#ifdef ZYXEL_REMOTE_ACCOUNT_ACCESS_PRIVILEGE	
			replaceParam(pramJobj, "Privilege", obj, "Privilege");
	#endif
			if(iid.idx[0]==highGpIdx){//should not happen for current design
				json_object_object_add(pramJobj, "group",json_object_new_string("Root"));
			}
			else if(iid.idx[0]==mediumGpIdx){
				json_object_object_add(pramJobj, "group",json_object_new_string("Administrator"));
			}
#ifndef MSTC_TAAAG_MULTI_USER /*__MSTC__, Linda, CPE User Definitions and Rights. */					
			else if(iid.idx[0]==lowGpIdx){
				json_object_object_add(pramJobj, "group",json_object_new_string("User"));
			}
#endif			
#ifdef ABUU_CUSTOMIZATION
			else if(iid.idx[0]==telek0mGpIdx){
				json_object_object_add(pramJobj, "group",json_object_new_string("Telek0m"));
			}
			else if(iid.idx[0]==vullGpIdx){
				json_object_object_add(pramJobj, "group",json_object_new_string("Vull"));
			}
#endif
#ifdef ACAU_CUSTOMIZATION
			else if(iid.idx[0]==ftpGpIdx){
				json_object_object_add(pramJobj, "group",json_object_new_string("FTPS"));
			}
#endif
			else{
				dbg_printf("%s : Error\n",__FUNCTION__);
			}
			
			if(json_object_object_get(Jobj, "Index") != NULL) {
				if(index == json_object_get_int(json_object_object_get(Jobj, "Index"))) {
					json_object_array_add(Jarray, pramJobj);
					zcfgFeJsonObjFree(obj);
					break;
				}
			}else {
				json_object_array_add(Jarray, JSON_OBJ_COPY(pramJobj));
				zcfgFeJsonObjFree(pramJobj);
			}
			zcfgFeJsonObjFree(obj);
			index++;
#if 0 //def MSTC_TAAAG_MULTI_USER
			}
#endif
		}
	}
	return ret;
}

//POST function
zcfgRet_t zcfgFeDal_User_Account_Add(struct json_object *Jobj, char *replyMsg){ 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *accountObj = NULL;
	objIndex_t accountIid ={0};
	const char *username = NULL;
#if	0
	const char *password = NULL;
	const char *oldPw = NULL;
	const char *verifyNewpassword = NULL;
	char *accountName = NULL;
	int accountRetryTime = 0;
	int accountIdleTime = 0;
	int accountLockTime = 0;
	bool enable = false;
	bool enableQuickStart = false;
	int groupnumber = 0;
#endif
	int FirstIndex = 0;
	char loginLevel[16] = {0};

	if(getLoginUserInfo(Jobj, NULL, loginLevel, NULL) != ZCFG_SUCCESS){
		if(replyMsg) strcpy(replyMsg, "Internal Error, can't get account information");
		dbg_printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}

	if(strcmp(Jgets(Jobj, "Password"), Jgets(Jobj, "verifyNewpassword"))){
		if(replyMsg) strcpy(replyMsg, "Password and verifyNewpassword don't match");
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.notMatch"));
		return ZCFG_REQUEST_REJECT;
	}

	username = Jgets(Jobj, "Username");
	if(getSpecificObj(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, "Username", json_type_string, (void *)username, NULL, NULL)){
		if(replyMsg) strcpy(replyMsg, "Username is duplicated");
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.account_repeat"));
		return ZCFG_REQUEST_REJECT;
	}

	//add default value
#ifndef MSTC_TAAAG_MULTI_USER /*__MSTC__, Linda, CPE User Definitions and Rights. */						
	if(!ISSET("group")) Jadds(Jobj, "group", "User");
#else
	if(!ISSET("group")) Jadds(Jobj, "group", "Administrator");
#endif
	if(!ISSET("Enabled")) Jaddb(Jobj, "Enabled", true);
	if(!ISSET("AccountRetryTime")) Jaddi(Jobj, "AccountRetryTime", 3);
	if(!ISSET("AccountIdleTime")) Jaddi(Jobj, "AccountIdleTime", 5*60);
	if(!ISSET("AccountLockTime")) Jaddi(Jobj, "AccountLockTime", 5*60);

	if(!strcmp("Administrator", Jgets(Jobj, "group"))){
#ifdef ABUU_CUSTOMIZATION
		if(!strcmp(loginLevel, "low") || !strcmp(loginLevel, "telek0m")){
#else
		if(!strcmp(loginLevel, "low")){
#endif
			if(replyMsg) strcpy(replyMsg, "Request Deny, can't add Administrator level account");
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.addAdminErr"));
			return ZCFG_REQUEST_REJECT;
		}
		FirstIndex = 2;
#ifdef WIND_ITALY_CUSTOMIZATION
		if(!ISSET("EnableQuickStart")) Jaddb(Jobj, "EnableQuickStart", false);
#else
		if(!ISSET("EnableQuickStart")) Jaddb(Jobj, "EnableQuickStart", true);
#endif
	}
#ifndef MSTC_TAAAG_MULTI_USER /*__MSTC__, Linda, CPE User Definitions and Rights. */							
	else if(!strcmp("User",Jgets(Jobj, "group"))){
		FirstIndex = 3;
#ifndef YAAAB_CUSTOMIZATION
		if(!ISSET("EnableQuickStart")) Jaddb(Jobj, "EnableQuickStart", false);
#else
		if(!ISSET("EnableQuickStart")) Jaddb(Jobj, "EnableQuickStart", true);
#endif
	}
#endif
#ifdef ABUU_CUSTOMIZATION
	else if(!strcmp("Vull",Jgets(Jobj, "group"))){
		if(strcmp(loginLevel, "vull")){
			if(replyMsg) strcpy(replyMsg, "Request Deny, can't add Administrator level account");
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.addAdminErr"));
			return ZCFG_REQUEST_REJECT;
		}
		FirstIndex = 4;
		if(!ISSET("EnableQuickStart")) Jaddb(Jobj, "EnableQuickStart", false);
	}
	else if(!strcmp("Telek0m",Jgets(Jobj, "group"))){
		if(!strcmp(loginLevel, "low") || !strcmp(loginLevel, "medium")){
			if(replyMsg) strcpy(replyMsg, "Request Deny, can't add Administrator level account");
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.addAdminErr"));
			return ZCFG_REQUEST_REJECT;
		}
		FirstIndex = 5;
		if(!ISSET("EnableQuickStart")) Jaddb(Jobj, "EnableQuickStart", false);
	}
#endif	
#ifdef ACAU_CUSTOMIZATION
	else if(!strcmp("FTPS",Jgets(Jobj, "group"))){
		FirstIndex = 4;
		if(!ISSET("EnableQuickStart")) Jaddb(Jobj, "EnableQuickStart", false);
	}
#endif
	else
		dbg_printf("ERROR!\n");

	IID_INIT(accountIid);
	accountIid.level = 1;
	accountIid.idx[0] = FirstIndex;
	ret = zcfgFeObjJsonAdd(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, NULL);
	
	if( ZCFG_SUCCESS == ret ){
		ret = zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, &accountObj);
	}

	if( ZCFG_SUCCESS == ret){
		replaceParam(accountObj, "Enabled", Jobj, "Enabled");
		replaceParam(accountObj, "EnableQuickStart", Jobj, "EnableQuickStart");
		replaceParam(accountObj, "Username", Jobj, "Username");
		replaceParam(accountObj, "Password", Jobj, "Password");
		replaceParam(accountObj, "AccountRetryTime", Jobj, "AccountRetryTime");
		replaceParam(accountObj, "AccountIdleTime", Jobj, "AccountIdleTime");
		replaceParam(accountObj, "AccountLockTime", Jobj, "AccountLockTime");
#ifdef ZYXEL_PERIODICALLY_CHANGE_PASSWORD
		replaceParam(accountObj, "PeriodicChangePasswordEnable", Jobj, "PeriodicChangePasswordEnable");
		replaceParam(accountObj, "ChangePasswordInterval", Jobj, "ChangePasswordInterval");
#endif
		//replaceParam(accountObj, "verifyNewpassword", Jobj, "verifyNewpassword");
		//replaceParam(accountObj, "groupnumber", Jobj, "groupnumber");
#ifdef ZYXEL_REMOTE_ACCESS_PRIVILEGE
		replaceParam(accountObj, "RemoteAccessPrivilege", Jobj, "RemoteAccessPrivilege");
#endif
#ifdef ZYXEL_REMOTE_ACCOUNT_ACCESS_PRIVILEGE	
		replaceParam(accountObj, "Privilege", Jobj, "Privilege");
#endif		
		ret = zcfgFeObjJsonBlockedSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, accountObj, NULL);
	}
	
	if( ZCFG_EXCEED_MAX_INS_NUM == ret){
#ifdef ABUU_CUSTOMIZATION 
		if(replyMsg)
			strcpy(replyMsg, "Error: Each group can not exceed 10 user accounts!");
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.exceed_account_max_err_ABUU"));
#else
		if(replyMsg)
			strcpy(replyMsg, "Error: Each group can not exceed 4 user accounts!");
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.exceed_account_max_err"));
#endif
	}
	
	zcfgFeJsonObjFree(accountObj);
	return ret;
}

//PUT function
zcfgRet_t zcfgFeDal_User_Account_Edit(struct json_object *Jobj, char *replyMsg){ 
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
//SHA512_PASSWD
	int authResult = -1;
//SHA512_PASSWD
#ifdef CONFIG_ABZQ_CUSTOMIZATION
	char *userName = NULL;
	char *accessMode = NULL;
	char *srvName = NULL;
#endif

	if(getLoginUserInfo(Jobj, NULL, loginLevel, NULL) != ZCFG_SUCCESS){
		if(replyMsg) strcpy(replyMsg, "Internal Error, can't get account information");
		dbg_printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}

	jsonType = json_object_get_type(Jobj);
	
	if(jsonType == json_type_object){
		
		username = Jgets(Jobj, "Username");
		if(!getSpecificObj(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, "Username", json_type_string, (void *)username, &accountIid, &accountObj)){
			if(replyMsg) strcpy(replyMsg, "Request reject");
			return ZCFG_REQUEST_REJECT;
		}
#ifdef MSTC_TAAAG_MULTI_USER
		if(accountIid.idx[0] < highGpIdx || (!strcmp(loginLevel, "low") && accountIid.idx[0] < lowGpIdx)){
#else
		if(accountIid.idx[0] < mediumGpIdx || (!strcmp(loginLevel, "low") && accountIid.idx[0] < lowGpIdx)){
#endif
			if(replyMsg) strcpy(replyMsg, "Request reject");
			zcfgFeJsonObjFree(accountObj);
			return ZCFG_REQUEST_REJECT;
		}

		if(ISSET("Password")){
			if(strcmp("", Jgets(Jobj ,"Password"))){//if psw is not empty
				if(ISSET("oldPw")){
					if(!strcmp("", Jgets(Jobj ,"oldPw"))){//if old psw is empty
						if(replyMsg) {
							strcpy(replyMsg, "Need --oldPw to modify Password");
						}
						json_object_object_add(Jobj, "__multi_lang_replyMsg", 
							json_object_new_string("zylang.Common.oldPassword_empty"));
						zcfgFeJsonObjFree(accountObj);
						return ZCFG_NOT_FOUND; //GUI use ZCFG_NOT_FOUND to detect oldPw error.
					}
					else{
						if(ISSET("verifyNewpassword")){//if verify psw is empty
							if(!strcmp("", Jgets(Jobj ,"verifyNewpassword"))){
								if(replyMsg) {
									strcpy(replyMsg, "Need --verifyNewpassword to modify Password");
								}
								json_object_object_add(Jobj, "__multi_lang_replyMsg", 
									json_object_new_string("zylang.Common.verify_password_empty"));
								zcfgFeJsonObjFree(accountObj);
								return ZCFG_REQUEST_REJECT;
							}
							else{
								//check if verify psw is correct
								if(strcmp(Jgets(Jobj, "Password"), Jgets(Jobj, "verifyNewpassword"))){
									if(replyMsg) 
										strcpy(replyMsg, "--Password and --verifyNewpassword don't match");
									json_object_object_add(Jobj, "__multi_lang_replyMsg", 
										json_object_new_string("zylang.Common.notMatch"));
									zcfgFeJsonObjFree(accountObj);
									return ZCFG_REQUEST_REJECT;
								}

								//use username to check current psw
								authResult = zyUtilAuthUserAccount(username, Jgets(Jobj ,"oldPw"));
								if(authResult == 1){
									curPassword = Jgets(accountObj, "Password");
									if(strcmp(curPassword, Jgets(Jobj ,"oldPw"))!= 0){
										if(replyMsg) 
											strcpy(replyMsg, "Wrong --oldPw value");
										json_object_object_add(Jobj, "__multi_lang_replyMsg", 
											json_object_new_string("zylang.Common.oldpassword_err2"));
										zcfgFeJsonObjFree(accountObj);
										return ZCFG_NOT_FOUND; //GUI use ZCFG_NOT_FOUND to detect oldPw error.
									}
								}
								else if (authResult < 0){
									if(replyMsg) 
										strcpy(replyMsg, "Wrong --oldPw value");
									json_object_object_add(Jobj, "__multi_lang_replyMsg", 
										json_object_new_string("zylang.Common.oldpassword_err2"));
									zcfgFeJsonObjFree(accountObj);
									return ZCFG_NOT_FOUND; //GUI use ZCFG_NOT_FOUND to detect oldPw error.
								}							
								//if current psw and verify psw is checked, update data model psw
								replaceParam(accountObj, "Password", Jobj, "Password");

#ifdef MSTC_TAAAG_MULTI_USER
								char *encodePassword = NULL,*pawd = NULL;
								pawd = Jgets(Jobj ,"oldPw");
								encodePassword = base64_encode((unsigned char*)pawd, strlen(pawd));
								cleanCurrUser(username, encodePassword);
#endif								
							}
						}
						else{
							if(replyMsg) {
								strcpy(replyMsg, "Need --verifyNewpassword to modify Password");
							}
							json_object_object_add(Jobj, "__multi_lang_replyMsg", 
								json_object_new_string("zylang.Common.verify_password_empty"));
							zcfgFeJsonObjFree(accountObj);
							return ZCFG_REQUEST_REJECT;
						}
					}
				}
				else{
					if(replyMsg) {
						strcpy(replyMsg, "Need --oldPw to modify Password");
					}
					json_object_object_add(Jobj, "__multi_lang_replyMsg", 
						json_object_new_string("zylang.Common.oldPassword_empty"));
					zcfgFeJsonObjFree(accountObj);
					return ZCFG_NOT_FOUND; //GUI use ZCFG_NOT_FOUND to detect oldPw error.
				}
			}
		}

#ifdef ACAU_CUSTOMIZATION
		if(!strcmp(username, "ftps")){
			system("echo 1 > /data/ftps\n");
		}
#endif

		replaceParam(accountObj, "Enabled", Jobj, "Enabled");
		replaceParam(accountObj, "AccountRetryTime", Jobj, "AccountRetryTime");
		replaceParam(accountObj, "AccountIdleTime", Jobj, "AccountIdleTime");
		replaceParam(accountObj, "AccountLockTime", Jobj, "AccountLockTime");
#ifdef ZYXEL_PERIODICALLY_CHANGE_PASSWORD
		replaceParam(accountObj, "PeriodicChangePasswordEnable", Jobj, "PeriodicChangePasswordEnable");
		replaceParam(accountObj, "ChangePasswordInterval", Jobj, "ChangePasswordInterval");
#endif
#ifdef ZYXEL_REMOTE_ACCESS_PRIVILEGE	
		replaceParam(accountObj, "RemoteAccessPrivilege", Jobj, "RemoteAccessPrivilege");
#endif
#ifdef ZYXEL_REMOTE_ACCOUNT_ACCESS_PRIVILEGE	
		replaceParam(accountObj, "Privilege", Jobj, "Privilege");
#endif	
#ifdef CONFIG_ABZQ_CUSTOMIZATION
		userName = json_object_get_string(json_object_object_get(Jobj, "DAL_LOGIN_USERNAME"));
		accessMode = json_object_get_string(json_object_object_get(Jobj, "AccessMode"));
		srvName = json_object_get_string(json_object_object_get(Jobj, "SrvName"));
#endif

        ret = zcfgFeObjJsonBlockedSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, accountObj, NULL);
#ifdef CONFIG_ABZQ_CUSTOMIZATION
		if(ret == ZCFG_SUCCESS)
			zcfgLogPrefix(ZCFG_LOG_INFO, ZCFG_LOGPRE_ACCOUNT, "%s User '%s' via %s change '%s' account password\n", accessMode, userName, srvName, username);
			copySystemLogToFlash (true);
#endif

		zcfgFeJsonObjFree(accountObj);
	}

	//GUI only, enable/dissable mutil accounts
	if(jsonType == json_type_array){
		while((reqObj = json_object_array_get_idx(Jobj, arrayIdx)) !=NULL){
			arrayIdx++;
			username = Jgets(reqObj, "Username");
			if((Jget(reqObj, "Enabled")!=NULL) && getSpecificObj(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, "Username", json_type_string, (void *)username, &accountIid, &accountObj)){
				if(accountIid.idx[0] < mediumGpIdx || (!strcmp(loginLevel, "low") && accountIid.idx[0] < lowGpIdx)){
					zcfgFeJsonObjFree(accountObj);
					continue;
				}
				
				enable = Jgetb(accountObj, "Enabled");
				if(enable != Jgetb(reqObj, "Enabled")){
					replaceParam(accountObj, "Enabled", reqObj, "Enabled");
                    zcfgFeObjJsonBlockedSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, accountObj, NULL);
				}
				zcfgFeJsonObjFree(accountObj);
			}
		}
	}
	
	return ret;
}

//DELETE function
zcfgRet_t zcfgFeDal_User_Account_Delete(struct json_object *Jobj, char *replyMsg){ 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *accountObj = NULL;
	objIndex_t accountIid = {0};
	const char *username;
	char loginLevel[16] = {0};
	char loginUserName[32] = {0};
	int objIndex;

	if(getLoginUserInfo(Jobj, loginUserName, loginLevel, NULL) != ZCFG_SUCCESS){
		dbg_printf("getLoginUserInfo Fail");
		if(replyMsg) strcpy(replyMsg, "Internal Error, can't get login user information");
		return ZCFG_INTERNAL_ERROR;
	}

    if(Jget(Jobj, "Username") != NULL){ // from CLI
    	username = Jgets(Jobj, "Username");
    	if(!getSpecificObj(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, "Username", json_type_string, (void *)username, &accountIid, &accountObj)){
    		if(replyMsg) strcpy(replyMsg, "Request reject");
    		return ZCFG_REQUEST_REJECT;
    	}
    }
    else{ // from GUI
		objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
		if(0 !=objIndex  ){
			ret = convertIndextoIid(objIndex, &accountIid, RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, NULL, NULL, replyMsg);
			if(ret != ZCFG_SUCCESS){
				dbg_printf("Wrong index..\n");
				return ret;
			}
			
		}else{
			IID_INIT(accountIid);
			accountIid.level = 2;
			accountIid.idx[0] = Jgeti(Jobj, "FirstIndex");
			accountIid.idx[1] = Jgeti(Jobj, "SecondIndex");
		}
        if(zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, &accountObj) != ZCFG_SUCCESS){
            dbg_printf("get target user info fail\n");
            if(replyMsg) strcpy(replyMsg, "Internal Error, can't get target user information");
            return ZCFG_INTERNAL_ERROR;
        }
        username = Jgets(accountObj, "Username");
    }

    if(!strcmp(username, loginUserName)){
        if(replyMsg) strcpy(replyMsg, "Can't delete yourself");
        return ZCFG_REQUEST_REJECT;
    }

    if(accountIid.idx[0] < mediumGpIdx || (!strcmp(loginLevel, "low") && accountIid.idx[0] < lowGpIdx)){
        if(replyMsg) strcpy(replyMsg, "Request reject");
        zcfgFeJsonObjFree(accountObj);
        return ZCFG_REQUEST_REJECT;
    }
	
	ret = zcfgFeObjJsonDel(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, NULL);
	zcfgFeJsonObjFree(accountObj);
	return ret;
}

zcfgRet_t zcfgFeDalUserAccount(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;	
	char loginLevel[16] = {0};
	char loginUserName[32] = {0};

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(getLoginUserInfo(Jobj, loginUserName, loginLevel, NULL) != ZCFG_SUCCESS){
		printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}

	if(!strcmp(method, "GET")){
		ret = zcfgFeDal_User_Account_Get(Jobj, Jarray, replyMsg);
	}else if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_User_Account_Edit(Jobj, replyMsg);
	}else if(!strcmp(method, "POST")) {
		ret = zcfgFeDal_User_Account_Add(Jobj, replyMsg);
	}else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDal_User_Account_Delete(Jobj, replyMsg);
	}else
		printf("Unknown method:%s\n", method);

	return ret;
}

#ifdef CONFIG_ZYXEL_TR140
zcfgRet_t zcfgFeDal_Samba_Account_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t ttaccountIid;
	struct json_object *ttaccountJobj = NULL;
	struct json_object *pramJobj = NULL;
	const char *UserName;

    UserName = json_object_get_string(json_object_object_get(Jobj, "Username"));

		IID_INIT(ttaccountIid);
		ttaccountIid.level=2;
		ttaccountIid.idx[0]=3;
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &ttaccountIid, &ttaccountJobj) == ZCFG_SUCCESS) {
		  if(!strcmp(UserName,json_object_get_string(json_object_object_get(ttaccountJobj, "Username")))){
			pramJobj = json_object_new_object();
			replaceParam(pramJobj, "Enabled", ttaccountJobj, "Enabled");
			replaceParam(pramJobj, "Username", ttaccountJobj, "Username");
			replaceParam(pramJobj, "Privilege", ttaccountJobj, "Privilege");
			zcfgFeJsonObjFree(ttaccountJobj);
            break;
		  }	
			zcfgFeJsonObjFree(ttaccountJobj);
		}

		json_object_array_add(Jarray, pramJobj);
		
	return ret;
}

zcfgRet_t zcfgFeDal_Samba_Account_Add(struct json_object *Jobj, char *replyMsg){ 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *accountObj = NULL;
	objIndex_t accountIid ={0};
	const char *username = NULL;
	int FirstIndex = 0;

	username = Jgets(Jobj, "Username");
	if(getSpecificObj(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, "Username", json_type_string, username, NULL, NULL)){
		if(replyMsg) strcpy(replyMsg, "Username is duplicated");
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.account_repeat"));
		return ZCFG_REQUEST_REJECT;
	}

	//add default value
	if(!ISSET("group")) Jadds(Jobj, "group", "samba");
	if(!ISSET("Enabled")) Jaddb(Jobj, "Enabled", true);

	FirstIndex = 3;
	if(!ISSET("EnableQuickStart")) Jaddb(Jobj, "EnableQuickStart", false);

	IID_INIT(accountIid);
	accountIid.level = 1;
	accountIid.idx[0] = FirstIndex;
	ret = zcfgFeObjJsonAdd(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, NULL);
	
	if( ZCFG_SUCCESS == ret ){
		ret = zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, &accountObj);
	}

	if( ZCFG_SUCCESS == ret){
		replaceParam(accountObj, "Enabled", Jobj, "Enabled");
		replaceParam(accountObj, "EnableQuickStart", Jobj, "EnableQuickStart");
		replaceParam(accountObj, "Username", Jobj, "Username");
		replaceParam(accountObj, "Password", Jobj, "Password");
		replaceParam(accountObj, "Privilege", Jobj, "Privilege");
		ret = zcfgFeObjJsonBlockedSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, accountObj, NULL);
	}
	
	if( ZCFG_EXCEED_MAX_INS_NUM == ret){
		if(replyMsg) strcpy(replyMsg, "Error: Each group can not exceed 4 user accounts!");
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.exceed_account_max_err"));
	}
	
	zcfgFeJsonObjFree(accountObj);
	return ret;
}

zcfgRet_t zcfgFeDal_Samba_Account_Edit(struct json_object *Jobj, char *replyMsg){ 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *accountObj = NULL;
	objIndex_t accountIid = {0};
	const char *username = NULL;
		
		username = Jgets(Jobj, "Username");
		if(!getSpecificObj(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, "Username", json_type_string, username, &accountIid, &accountObj)){
			if(replyMsg) strcpy(replyMsg, "Request reject");
			return ZCFG_REQUEST_REJECT;
		}

		if(ISSET("Password")){
				replaceParam(accountObj, "Password", Jobj, "Password");
		}
		
		replaceParam(accountObj, "Enabled", Jobj, "Enabled");
		replaceParam(accountObj, "Privilege", Jobj, "Privilege");
        ret = zcfgFeObjJsonBlockedSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, accountObj, NULL);
		zcfgFeJsonObjFree(accountObj);

	return ret;
}

zcfgRet_t zcfgFeDal_Samba_Account_Delete(struct json_object *Jobj, char *replyMsg){ 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *accountObj = NULL;
	objIndex_t accountIid = {0};
	const char *username;
	char loginLevel[16] = {0};
	char loginUserName[32] = {0};
	int objIndex;

        username = Jgets(Jobj, "Username");
		
		IID_INIT(accountIid);
		accountIid.level = 2;
		accountIid.idx[0] = 3;
        while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, &accountObj) == ZCFG_SUCCESS) {
		  if(!strcmp(username,json_object_get_string(json_object_object_get(accountObj, "Username")))){
			  ret = zcfgFeObjJsonDel(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, NULL);
			zcfgFeJsonObjFree(accountObj);
            break;
		  }	
			zcfgFeJsonObjFree(accountObj);
		}
	
	return ret;
}

zcfgRet_t zcfgFeDalSambaAccount(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;	
	char loginLevel[16] = {0};
	char loginUserName[32] = {0};

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "GET")){
		ret = zcfgFeDal_Samba_Account_Get(Jobj, Jarray, replyMsg);
	}else if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_Samba_Account_Edit(Jobj, replyMsg);
	}else if(!strcmp(method, "POST")) {
		ret = zcfgFeDal_Samba_Account_Add(Jobj, replyMsg);
	}else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDal_Samba_Account_Delete(Jobj, replyMsg);
	}else
		printf("Unknown method:%s\n", method);

	return ret;
}

#endif

