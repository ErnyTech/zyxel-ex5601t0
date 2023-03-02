
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

dal_param_t MAIL_NOTIFICATION_param[]={
	{"SMTPServerAddress",					dalType_string, 0,	0,	NULL,	NULL,	dal_Add},
	{"SMTPServerPort",						dalType_int,	0,	65535,	NULL,	NULL,	dal_Add},	
	{"AuthUser",							dalType_string,	0,	0,	NULL,	NULL,	dal_Add},
	{"AuthPass",							dalType_string,	0,	0,	NULL,	NULL,	dal_Add},
	{"EmailAddress",						dalType_emailAddr,	0,	0,	NULL,	NULL,	dal_Add},
	{"SMTPSecurity",						dalType_string,	0,	0,	NULL,	"ssl|starttls",	dal_Add},
	{"Index",								dalType_int,	0,	0,	NULL,	NULL,	dal_Delete},
	{NULL,									0,				0,	0,	NULL},
};

unsigned int SMTPServerPort;
const char* SMTPServerAddress;
const char* AuthUser;
const char* AuthPass;
const char* EmailAddress;
const char* SMTPSecurity;
const char* DeleteMail;

struct json_object *mailNotificationJobj;

objIndex_t delIid;

void freeAllMailNotificationObjects(){
	if(mailNotificationJobj) json_object_put(mailNotificationJobj);
	return;
}

void initMailNotificationGlobalObjects(){
	mailNotificationJobj = NULL;

	return;
}

void getMailNotificationBasicInfo(struct json_object *Jobj){
	SMTPServerPort = json_object_get_int(json_object_object_get(Jobj, "SMTPServerPort"));
	SMTPServerAddress = json_object_get_string(json_object_object_get(Jobj, "SMTPServerAddress"));
	AuthUser = json_object_get_string(json_object_object_get(Jobj, "AuthUser"));
	AuthPass = json_object_get_string(json_object_object_get(Jobj, "AuthPass"));
	EmailAddress = json_object_get_string(json_object_object_get(Jobj, "EmailAddress"));
	SMTPSecurity = json_object_get_string(json_object_object_get(Jobj, "SMTPSecurity"));

	return;
}

zcfgRet_t zcfgFeDalMailNotificationDel(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	IID_INIT(delIid);
	int index = 0;
	zcfg_offset_t oid;
	oid = RDM_OID_MAIL_SERVICE;
	
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(index, &delIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	ret = zcfgFeObjJsonDel(oid, &delIid, NULL);
	return ret;
}


zcfgRet_t zcfgFeDalMailNotificationAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	initMailNotificationGlobalObjects();
	getMailNotificationBasicInfo(Jobj);
	objIndex_t objIid;

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_MAIL_SERVICE, &objIid, NULL)) != ZCFG_SUCCESS) {
		printf("%s: fail to add mail service with ret = %d\n", __FUNCTION__, ret);
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.EmailNotification.EmailNotification.error.add_email_err"));
		return false;
	}

	if((ret = zcfgFeObjJsonGet(RDM_OID_MAIL_SERVICE, &objIid, &mailNotificationJobj)) != ZCFG_SUCCESS) {
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.EmailNotification.EmailNotification.error.get_email_err"));
		return false;
	}

	json_object_object_add(mailNotificationJobj, "Enable", json_object_new_boolean(true));
	json_object_object_add(mailNotificationJobj, "RequiresTLS", json_object_new_boolean(false));
	json_object_object_add(mailNotificationJobj, "SMTPServerPort", json_object_new_int(SMTPServerPort));
	json_object_object_add(mailNotificationJobj, "SMTPServerAddress", json_object_new_string(SMTPServerAddress));
	json_object_object_add(mailNotificationJobj, "AuthUser", json_object_new_string(AuthUser));
	json_object_object_add(mailNotificationJobj, "AuthPass", json_object_new_string(AuthPass));
	json_object_object_add(mailNotificationJobj, "EmailAddress", json_object_new_string(EmailAddress));
	json_object_object_add(mailNotificationJobj, "SMTPSecurity", json_object_new_string(SMTPSecurity));

	if((ret = zcfgFeObjJsonSet(RDM_OID_MAIL_SERVICE, &objIid, mailNotificationJobj, NULL)) != ZCFG_SUCCESS)
	{
		dbg_printf("%s: Set mail service fail\n", __FUNCTION__);
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.EmailNotification.EmailNotification.error.set_email_err"));
		goto exit;
	}

exit:
	freeAllMailNotificationObjects();
	return ret;
}

void zcfgFeDalShowMailNotification(struct json_object *Jarray){
	struct json_object *obj = NULL;
	int len = 0, i;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-10s %-30s %-15s %-10s %-15s %-10s \n",
			"Index", "Mail Server Address", "Username", "Port", "Security", "E-mail Address");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-10s %-30s %-15s %-10s %-15s %-10s \n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "SMTPServerAddress")), 
			json_object_get_string(json_object_object_get(obj, "AuthUser")), 
			json_object_get_string(json_object_object_get(obj, "SMTPServerPort")), 
			json_object_get_string(json_object_object_get(obj, "SMTPSecurity")), 
			json_object_get_string(json_object_object_get(obj, "EmailAddress")));
	}
}


zcfgRet_t zcfgFeDalMailNotificationGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *emailObj = NULL;
	objIndex_t emailIid = {0};
	int index = 1;
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MAIL_SERVICE, &emailIid, &emailObj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index", json_object_new_int(index));
		json_object_object_add(paramJobj, "SMTPServerAddress", JSON_OBJ_COPY(json_object_object_get(emailObj, "SMTPServerAddress")));
		json_object_object_add(paramJobj, "AuthUser", JSON_OBJ_COPY(json_object_object_get(emailObj, "AuthUser")));
		json_object_object_add(paramJobj, "SMTPServerPort", JSON_OBJ_COPY(json_object_object_get(emailObj, "SMTPServerPort")));
		json_object_object_add(paramJobj, "SMTPSecurity", JSON_OBJ_COPY(json_object_object_get(emailObj, "SMTPSecurity")));
		json_object_object_add(paramJobj, "EmailAddress", JSON_OBJ_COPY(json_object_object_get(emailObj, "EmailAddress")));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(emailObj);
		index++;
	}

	return ret;
}

zcfgRet_t zcfgFeDalMailNotification(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDalMailNotificationAdd(Jobj, NULL);
	}else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalMailNotificationDel(Jobj, replyMsg);
	}else if(!strcmp(method, "GET")){
		ret = zcfgFeDalMailNotificationGet(Jobj, Jarray, NULL);
	}

	return ret;
}

