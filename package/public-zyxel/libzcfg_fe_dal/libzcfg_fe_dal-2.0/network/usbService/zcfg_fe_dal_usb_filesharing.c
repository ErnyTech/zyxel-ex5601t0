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

//#define JSON_OBJ_COPY(json_object) json_tokener_parse(json_object_to_json_string(json_object))

dal_param_t USB_FILESHARING_param[]={
	{"Index",				dalType_int,		0,	0,	NULL,	NULL,	0},
	//SAMBA
	{"FileShare",			dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"Browseable",	dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{NULL,		0,	0,	0,	NULL,	NULL,	0},
};
void zcfgFeDalShowUsbFilesharing(struct json_object *Jarray){
	struct json_object *obj = NULL;
	// struct json_object *servconfobj = NULL;
	// struct json_object *accountobj = NULL, *accounttmpobj = NULL;
	// struct json_object *sambaobj = NULL, *sambatmpobj = NULL;
	struct json_object *usbinfoobj = NULL;
	struct json_object *usbinfotmpobj = NULL;
	// int len_accountobj = 0;
	int len_usbinfoobj = 0, num_usbinfoobj;
	// int len_sambaobj = 0;
	// int num_accountobj, num_sambaobj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
	return;
}
	obj = json_object_array_get_idx(Jarray, 0);
	// servconfobj = json_object_object_get(obj, "Service Conf");
	// accountobj = json_object_object_get(obj, "Account");
	// sambaobj = json_object_object_get(obj, "Samba");
	usbinfoobj = json_object_object_get(obj, "Usb Info");
	// len_accountobj = json_object_array_length(accountobj);
	// len_sambaobj = json_object_array_length(sambaobj);
	len_usbinfoobj = json_object_array_length(usbinfoobj);
	
	if(len_usbinfoobj != 0){
		printf("USB Information: \n");
		printf("%-20s %-20s %-30s \n","Volume","Capacity(MB)","Used Space(MB)");
		for(num_usbinfoobj=0;num_usbinfoobj<len_usbinfoobj;num_usbinfoobj++){
			usbinfotmpobj = json_object_array_get_idx(usbinfoobj, num_usbinfoobj);
			printf("%-20s %-20s %-30s \n",
				json_object_get_string(json_object_object_get(usbinfotmpobj, "Volume")),
				json_object_get_string(json_object_object_get(usbinfotmpobj, "Capacity")),
				json_object_get_string(json_object_object_get(usbinfotmpobj, "Used Space")));
		}
	}
	else
		printf("USB do not plug in Device. \n");
	}
		
zcfgRet_t zcfgFeDalUsbFilesharingEdit(struct json_object *Jobj, char *replyMsg){
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *sambaObj = NULL;
	struct json_object *sambadirObj = NULL;
	objIndex_t sambaIid = {0};
	objIndex_t sambadirIid = {0};
	bool fileshare_enable = false; 
	bool browseable = false;
	struct json_object *fileshare_idxarray = NULL;
	struct json_object *browseable_array = NULL;
	int len = 0, i;
	#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	bool wan_enable = false; 
	char mode[20] = {0};
	
	fileshare_enable = json_object_get_boolean(json_object_object_get(Jobj, "FileShare"));
	wan_enable = json_object_get_boolean(json_object_object_get(Jobj, "FileShareWAN"));
	if(fileshare_enable == true && wan_enable == false)
	{
		   strcpy(mode,"LAN_ONLY");
	}
	else if(fileshare_enable == true && wan_enable == true)
	{
		   strcpy(mode,"ALL");
	}
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_SAMBA, &sambaIid, &sambaObj) == ZCFG_SUCCESS){
            if(fileshare_enable == true){
			    json_object_object_add(sambaObj, "Enable", json_object_new_boolean(fileshare_enable));
				json_object_object_add(sambaObj, "Mode", json_object_new_string(mode));
            }
			else{
				json_object_object_add(sambaObj, "Enable", json_object_new_boolean(fileshare_enable));
			}
			zcfgFeObjJsonSet(RDM_OID_ZY_SAMBA, &sambaIid, sambaObj, NULL);
			zcfgFeJsonObjFree(sambaObj);
		}
	#else
	if(json_object_object_get(Jobj, "FileShare")){
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_SAMBA, &sambaIid, &sambaObj) == ZCFG_SUCCESS){
			fileshare_enable = json_object_get_boolean(json_object_object_get(Jobj, "FileShare"));
			json_object_object_add(sambaObj, "Enable", json_object_new_boolean(fileshare_enable));
			zcfgFeObjJsonSet(RDM_OID_ZY_SAMBA, &sambaIid, sambaObj, NULL);
			zcfgFeJsonObjFree(sambaObj);
		}
	}
	#endif
	if(json_object_object_get(Jobj, "fsShareDirIid")){	//for GUI edit
		fileshare_idxarray = json_object_object_get(Jobj, "fsShareDirIid");
		browseable_array = json_object_object_get(Jobj, "Browseable");
		len = json_object_array_length(fileshare_idxarray);
		for(i=0;i<len;i++){
			IID_INIT(sambadirIid);
			sambadirIid.level = 1;
			sambadirIid.idx[0] = atoi(json_object_get_string(json_object_array_get_idx(fileshare_idxarray, i)));
			if(zcfgFeObjJsonGet(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, &sambadirObj) == ZCFG_SUCCESS){
				browseable = atoi(json_object_get_string(json_object_array_get_idx(browseable_array, i)));
				json_object_object_add(sambadirObj, "X_ZYXEL_Browseable", json_object_new_boolean(browseable));
				ret = zcfgFeObjJsonSet(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, sambadirObj, NULL);
				zcfgFeJsonObjFree(sambadirObj);
			}
		}
	}
	else if(json_object_object_get(Jobj, "Index") || json_object_object_get(Jobj, "Browseable")){	//for dalcmd edit
		if(!json_object_object_get(Jobj, "Index") || !json_object_object_get(Jobj, "Browseable")){
			strcpy(replyMsg, "Index and Browseable are mandatory parameter if user want to enable/disable Share Directory List.");
			return ZCFG_INVALID_PARAM_VALUE;
		}
		else {
			sambadirIid.level = 1;
			sambadirIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "Index"));
			if(zcfgFeObjJsonGet(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, &sambadirObj) == ZCFG_SUCCESS){
				browseable = json_object_get_boolean(json_object_object_get(Jobj, "Browseable"));
				json_object_object_add(sambadirObj, "X_ZYXEL_Browseable", json_object_new_boolean(browseable));
				ret = zcfgFeObjJsonSet(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, sambadirObj, NULL);
				zcfgFeJsonObjFree(sambadirObj);
			}
		}
	}
	return ret;
#else
	printf("%s : SAMBA BE not support\n", __FUNCTION__);
	return ZCFG_INTERNAL_ERROR;
#endif
}
	
zcfgRet_t zcfgFeDalUsbFilesharingGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *accountJobj = NULL;
	struct json_object *paramJobj = NULL;
	struct json_object *paramJobj1 = NULL;
	struct json_object *paramJobj2 = NULL;
	struct json_object *paramJobj3 = NULL;
	struct json_object *acctmpJobj = NULL;
	struct json_object *sambadirJobj = NULL;
	struct json_object *sambadirtmpJobj = NULL;
	struct json_object *sambaJobj = NULL;
	struct json_object *allJobj = NULL;
	struct json_object *usbdevtmpJobj = NULL;
	struct json_object *usbdevJobj = NULL;
	
	objIndex_t accountIid = {0};
	objIndex_t sambadirIid = {0};
	objIndex_t sambaIid = {0};
	objIndex_t usbdevIid = {0};
#ifdef CONFIG_ZYXEL_TR140
	struct json_object *ttaccountJobj = NULL;
	objIndex_t ttaccountIid = {0};
#endif

	acctmpJobj = json_object_new_array();
	sambadirtmpJobj = json_object_new_array();
	allJobj = json_object_new_object();
	usbdevtmpJobj = json_object_new_array();
	accountIid.level = 2;
	accountIid.idx[0] = 2;

	const char *path = NULL;
	const char *sharename = NULL;
	char rootpath[512] = {0};
#ifdef ABUU_CUSTOMIZATION
	char loginLevel[16] = {0};

	if(getLoginUserInfo(Jobj, NULL, loginLevel, NULL) != ZCFG_SUCCESS){
		if(replyMsg) strcpy(replyMsg, "Internal Error, can't get account information");
		dbg_printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}
#endif

#ifdef CONFIG_ZYXEL_TR140
		/*root*/
		IID_INIT(ttaccountIid);
		ttaccountIid.level=2;
		ttaccountIid.idx[0]=1;
		ttaccountIid.idx[1]=1;
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &ttaccountIid, &ttaccountJobj) == ZCFG_SUCCESS){
			paramJobj = json_object_new_object();
			if(json_object_object_get(ttaccountJobj, "Enabled") && json_object_object_get(ttaccountJobj, "Username")){
				json_object_object_add(paramJobj, "Status", JSON_OBJ_COPY(json_object_object_get(ttaccountJobj, "Enabled")));
				json_object_object_add(paramJobj, "Username", JSON_OBJ_COPY(json_object_object_get(ttaccountJobj, "Username")));
				json_object_array_add(acctmpJobj, paramJobj);
			}
			zcfgFeJsonObjFree(ttaccountJobj);
		}
#endif

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, &accountJobj) == ZCFG_SUCCESS){
#ifdef ABUU_CUSTOMIZATION
		/*
			If login with telek0m account, cannot display admin level account, and vice versa
			Only VULL account login can see VULL account, other account level can't see VULL account
		*/
		if (accountIid.idx[0] == vullIdx && (strcmp(loginLevel,"vull") != 0)) {
			zcfgFeJsonObjFree(accountJobj);
			continue;
		}
		else {
			if (accountIid.idx[0] == mediumIdx && !strcmp(loginLevel,"telek0m")) {
				zcfgFeJsonObjFree(accountJobj);
				continue;
			}
			else if (accountIid.idx[0] == telek0mIdx && !strcmp(loginLevel,"medium")) {
				zcfgFeJsonObjFree(accountJobj);
				continue;
			}
		}
#endif
		paramJobj = json_object_new_object();
		if(json_object_object_get(accountJobj, "Enabled") && json_object_object_get(accountJobj, "Username")){
			json_object_object_add(paramJobj, "Status", JSON_OBJ_COPY(json_object_object_get(accountJobj, "Enabled")));
			json_object_object_add(paramJobj, "Username", JSON_OBJ_COPY(json_object_object_get(accountJobj, "Username")));
			json_object_array_add(acctmpJobj, paramJobj);
		}
		zcfgFeJsonObjFree(accountJobj);
	}

#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, &sambadirJobj) == ZCFG_SUCCESS){
		paramJobj1 = json_object_new_object();
		path = json_object_get_string(json_object_object_get(sambadirJobj, "X_ZYXEL_RootPath"));
		sharename = json_object_get_string(json_object_object_get(sambadirJobj, "X_ZYXEL_Path"));
		//sprintf(rootpath,"%s/%s",path,sharename);
		snprintf(rootpath,sizeof(rootpath)-1,"%s/%s",path,sharename);
		json_object_object_add(paramJobj1, "Index", json_object_new_int(sambadirIid.idx[0]));
		json_object_object_add(paramJobj1, "Status", JSON_OBJ_COPY(json_object_object_get(sambadirJobj, "X_ZYXEL_Browseable")));
		json_object_object_add(paramJobj1, "Share Name", json_object_new_string(sharename));
		json_object_object_add(paramJobj1, "Share Path", json_object_new_string(rootpath));
		json_object_object_add(paramJobj1, "Share Description", JSON_OBJ_COPY(json_object_object_get(sambadirJobj, "X_ZYXEL_Comment")));
		json_object_object_add(paramJobj1, "Hostname", JSON_OBJ_COPY(json_object_object_get(sambadirJobj, "X_ZYXEL_Hostname")));
		json_object_array_add(sambadirtmpJobj, paramJobj1);
		zcfgFeJsonObjFree(sambadirJobj);
	}
#else
	printf("%s : SAMBA BE not support\n", __FUNCTION__);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_USB
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV_PAR, &usbdevIid, &usbdevJobj) == ZCFG_SUCCESS){
		paramJobj2 = json_object_new_object();
		json_object_object_add(paramJobj2, "Volume", JSON_OBJ_COPY(json_object_object_get(usbdevJobj, "PartitionName")));
		json_object_object_add(paramJobj2, "Capacity", JSON_OBJ_COPY(json_object_object_get(usbdevJobj, "Capacity")));
		json_object_object_add(paramJobj2, "Used Space", JSON_OBJ_COPY(json_object_object_get(usbdevJobj, "UsedSpace")));
		json_object_array_add(usbdevtmpJobj, paramJobj2);
		zcfgFeJsonObjFree(usbdevJobj);
	}
#else
	printf("%s : USB BE not support\n", __FUNCTION__);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_SAMBA, &sambaIid, &sambaJobj) == ZCFG_SUCCESS){
		paramJobj3 = json_object_new_object();
		json_object_object_add(paramJobj3, "File Sharing Services", JSON_OBJ_COPY(json_object_object_get(sambaJobj, "Enable")));
		json_object_object_add(allJobj, "Service Conf", paramJobj3);
		zcfgFeJsonObjFree(sambaJobj);
	}
#else
	printf("%s : SAMBA BE not support\n", __FUNCTION__);
#endif
	json_object_object_add(allJobj, "Account", acctmpJobj);
	json_object_object_add(allJobj, "Samba", sambadirtmpJobj);
	json_object_object_add(allJobj, "Usb Info", usbdevtmpJobj);
	json_object_array_add(Jarray, allJobj);
	return ret;
}
zcfgRet_t zcfgFeDalUsbFilesharing(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalUsbFilesharingEdit(Jobj, replyMsg);
	else if(!strcmp(method,"GET"))
		ret = zcfgFeDalUsbFilesharingGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);
		
	return ret;
}
