#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
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

#ifdef ABUU_CUSTOMIZATION
int highIdx = 1;
int mediumIdx = 2;
int lowIdx = 3;
int vullIdx = 4;
int telek0mIdx = 5;
#endif

dal_param_t USB_SAMBA_param[]={
	//Mandatory parameter for dalcmd
	{"Index",			dalType_int,	0,	0,	NULL,	NULL,	dal_Delete|dal_Edit},
	//SAMBA Dir
	{"HostName",		dalType_usbInfo,	0,	0,	NULL,	NULL, 	dal_Add},
	{"FolderName",		dalType_sambafolder,0,	0,	NULL,	NULL, 	dal_Add},
	{"Description",		dalType_string,	0,	0,	NULL,	NULL, 	dal_Add},
	{"Public",			dalType_boolean,	0,	0,	NULL},	
	{"ValidUsers",		dalType_usrAccount,	0,	0,	NULL},
	//{"ValidUsers_GUI",		dalType_string,	0,	0,	NULL},
	{NULL,		0,	0,	0,	NULL},
};

void zcfgFeDalShowUsbSamba(struct json_object *Jarray){
	struct json_object *obj = NULL;
	struct json_object *servconfobj = NULL;
	struct json_object *accountobj = NULL, *accounttmpobj = NULL;
	struct json_object *sambaobj = NULL, *sambatmpobj = NULL;
	struct json_object *usbinfoobj = NULL;
	int len_accountobj = 0, len_sambaobj = 0, len_usbinfoobj = 0;
	int num_accountobj, num_sambaobj;
	//struct json_object *usbinfotmpobj = NULL;
	//int num_usbinfoobj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	servconfobj = json_object_object_get(obj, "Service Conf");
	accountobj = json_object_object_get(obj, "Account");
	sambaobj = json_object_object_get(obj, "Samba");
	usbinfoobj = json_object_object_get(obj, "Usb Info");
	len_accountobj = json_object_array_length(accountobj);
	len_sambaobj = json_object_array_length(sambaobj);
	len_usbinfoobj = json_object_array_length(usbinfoobj);

	/*if(len_usbinfoobj != 0){
		printf("USB Information: \n");
		printf("%-20s %-20s %-30s \n","Volume","Capacity(MB)","Used Space(MB)");
		for(num_usbinfoobj=0;num_usbinfoobj<len_usbinfoobj;num_usbinfoobj++){
			usbinfotmpobj = json_object_array_get_idx(usbinfoobj, num_usbinfoobj);
			printf("%-20s %-20s %-30s \n",
				json_object_get_string(json_object_object_get(usbinfotmpobj, "Volume")),
				json_object_get_string(json_object_object_get(usbinfotmpobj, "Capacity")),
				json_object_get_string(json_object_object_get(usbinfotmpobj, "Used Space")));
		}
	}*/
	
	printf("\nServer Configuration: \n");
	printf("File Sharing Services:  %s \n",json_object_get_string(json_object_object_get(servconfobj, "File Sharing Services")));
	
	if(len_sambaobj != 0 && len_usbinfoobj != 0){
		printf("\nShare Directory List: \n");
		printf("%-10s %-10s %-35s %-50s %-10s \n", "Index", "Status", "Share Name", "Share Path", "Share Description");
		for(num_sambaobj=0;num_sambaobj<len_sambaobj;num_sambaobj++){
			sambatmpobj = json_object_array_get_idx(sambaobj, num_sambaobj);
			printf("%-10s %-10s %-35s %-50s %-10s \n",
				json_object_get_string(json_object_object_get(sambatmpobj, "Index")),
				json_object_get_string(json_object_object_get(sambatmpobj, "Status")),
				json_object_get_string(json_object_object_get(sambatmpobj, "Share Name")),
				json_object_get_string(json_object_object_get(sambatmpobj, "Share Path")),
				json_object_get_string(json_object_object_get(sambatmpobj, "Share Description")));
		}
	}

	printf("\nAccount Management: \n");
	printf("%-20s %-30s \n","Status","User Name");
	for(num_accountobj=0;num_accountobj<len_accountobj;num_accountobj++){
		accounttmpobj = json_object_array_get_idx(accountobj, num_accountobj);
		printf("%-20s %-30s \n",
			json_object_get_string(json_object_object_get(accounttmpobj, "Status")),
			json_object_get_string(json_object_object_get(accounttmpobj, "Username")));
	}
}

zcfgRet_t zcfgFeDalUsbSambaAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *sambadirObj = NULL;
	objIndex_t sambadirIid = {0};
	const char *HostName = NULL, *curHostName = NULL;
	const char *FolderName = NULL, *curFolderName = NULL;
	const char *Description = NULL;
	char ValidUser[128] = {0};
	const char *Validuser_gui = NULL;
	const char *Validuser_cli = NULL;
	const char *user = NULL;
	//bool Browseable = false;
	bool Public = false;
	char RootPath[512] = {0};
 	char buff[128] = {0};
	char *tmp = NULL;
	int directoryMaxLength = -1;
	int len = 0;
	

	HostName = json_object_get_string(json_object_object_get(Jobj, "HostName"));
	FolderName = json_object_get_string(json_object_object_get(Jobj, "FolderName"));
	Description = json_object_get_string(json_object_object_get(Jobj, "Description"));
	//Browseable = json_object_get_boolean(json_object_object_get(Jobj, "Browseable"));	 //not necessary parameter
	Public = json_object_get_boolean(json_object_object_get(Jobj, "Public"));

	// validate parameter values
	if( (json_object_object_get(Jobj, "HostName") == NULL) ||
		(json_object_object_get(Jobj, "FolderName") == NULL) ||
		(json_object_object_get(Jobj, "Description") == NULL) ||
		(json_object_object_get(Jobj, "Public") == NULL) )
	{
		dbg_printf("%s():%d Empty parameter value(s)\n",__FUNCTION__,__LINE__);
		// json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("choose your error msg"));
		return ZCFG_INVALID_PARAM_VALUE;
	}

	// validate parameter value
	if(strlen(FolderName) > 128)
	{
		dbg_printf("%s():%d wrong parameter value, FolderName size is invalid\n",__FUNCTION__,__LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}

	directoryMaxLength = zcfgFeObjMaxLenGet(RDM_OID_ZY_SAMBA_DIR); 

	
	if(json_object_object_get(Jobj, "ValidUsers_GUI")){
		Validuser_gui = json_object_get_string(json_object_object_get(Jobj, "ValidUsers_GUI"));
		strcpy(ValidUser,Validuser_gui);
	}
	else if(json_object_object_get(Jobj, "ValidUsers")){
		Validuser_cli = json_object_get_string(json_object_object_get(Jobj, "ValidUsers"));
	 	strcpy(buff, Validuser_cli);
	 	user = strtok_r(buff, ",", &tmp);
	 	while(user != NULL){
	 		strcat(ValidUser,user);
	 		strcat(ValidUser," ");
	 		user = strtok_r(NULL, ",", &tmp);
	 	}
	 }
	//sprintf(RootPath,"/mnt/%s",HostName);
	snprintf(RootPath,sizeof(RootPath)-1,"/mnt/%s",HostName);
	if(!Public){
		if(!json_object_object_get(Jobj, "ValidUsers")){
			strcpy(replyMsg, "Please enter the valid users.");
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}
	
	// check if the (hostname,foldername) matches existing one
	IID_INIT(sambadirIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, &sambadirObj) == ZCFG_SUCCESS) {
		curHostName = json_object_get_string(json_object_object_get(sambadirObj, "X_ZYXEL_Hostname"));
		curFolderName = json_object_get_string(json_object_object_get(sambadirObj, "X_ZYXEL_FolderName"));
		
		if( !strcmp(curHostName, HostName) && !strcmp(curFolderName, FolderName)){
			strcpy(replyMsg, "Folder is duplicate");
			zcfgFeJsonObjFree(sambadirObj);
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.USB.Error.same_sharepath_err"));
			return ZCFG_INVALID_PARAM_VALUE;	
		}
			len++;
			zcfgFeJsonObjFree(sambadirObj);
	}

	if(directoryMaxLength > 0 && directoryMaxLength < (len+1) ){
		ret = ZCFG_INVALID_PARAM_VALUE;
		printf("Add a new obj would over the max vlaue %d!\n", directoryMaxLength);	
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.USB.Error.add_too_many_directory"));
		return ret;
	}
	else
		printf("Max is %d, current array length is %d\n", directoryMaxLength, len);


	IID_INIT(sambadirIid);
 	if(zcfgFeObjJsonAdd(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, NULL) == ZCFG_SUCCESS){
		if(zcfgFeObjJsonGet(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, &sambadirObj) == ZCFG_SUCCESS){
			json_object_object_add(sambadirObj, "X_ZYXEL_RootPath", json_object_new_string(RootPath));
			json_object_object_add(sambadirObj, "X_ZYXEL_Hostname", json_object_new_string(HostName));
			json_object_object_add(sambadirObj, "X_ZYXEL_FolderName", json_object_new_string(FolderName));
			json_object_object_add(sambadirObj, "X_ZYXEL_Path", json_object_new_string(FolderName));
			json_object_object_add(sambadirObj, "X_ZYXEL_Comment", json_object_new_string(Description));
			json_object_object_add(sambadirObj, "X_ZYXEL_Browseable", json_object_new_boolean(true));
			json_object_object_add(sambadirObj, "X_ZYXEL_Public", json_object_new_boolean(Public));
			if(json_object_object_get(Jobj, "ValidUsers")){
			if(strcmp(ValidUser,""))
				json_object_object_add(sambadirObj, "X_ZYXEL_ValidUsers", json_object_new_string(ValidUser));
			}
			ret = zcfgFeObjJsonSet(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, sambadirObj, NULL);
			zcfgFeJsonObjFree(sambadirObj);
		}
	}
	
	return ret;
	
	
}

zcfgRet_t zcfgFeDalUsbSambaEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *sambadirObj = NULL;
	objIndex_t sambadirIid = {0}; 
	const char *Description = NULL;
	char ValidUser[128] = {0};
	const char *Validuser_gui = NULL;
	const char *Validuser_cli = NULL;
	const char *user = NULL;
	bool Public = false;
	char buff[128] = {0};
	char *tmp = NULL;

	sambadirIid.level = 1;
	sambadirIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "Index"));
	Description = json_object_get_string(json_object_object_get(Jobj, "Description")); 
	Public = json_object_get_boolean(json_object_object_get(Jobj, "Public"));

	if(json_object_object_get(Jobj, "ValidUsers_GUI")){
		Validuser_gui = json_object_get_string(json_object_object_get(Jobj, "ValidUsers_GUI"));
		strcpy(ValidUser,Validuser_gui);
	}
	else if(json_object_object_get(Jobj, "ValidUsers")){
		Validuser_cli = json_object_get_string(json_object_object_get(Jobj, "ValidUsers"));
	 	strcpy(buff, Validuser_cli);
	 	user = strtok_r(buff, ",", &tmp);
	 	while(user != NULL){
	 		strcat(ValidUser,user);
	 		strcat(ValidUser," ");
	 		user = strtok_r(tmp, ",", &tmp);
	 	}
	 }
	if(json_object_object_get(Jobj, "Public")){
	if(!Public){
		if(!json_object_object_get(Jobj, "ValidUsers")){
			strcpy(replyMsg, "Please enter the valid users.");
			return ZCFG_INVALID_PARAM_VALUE;
		}
	} 
	}
	if(zcfgFeObjJsonGet(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, &sambadirObj) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "Description"))
 		json_object_object_add(sambadirObj, "X_ZYXEL_Comment", json_object_new_string(Description));
		if(json_object_object_get(Jobj, "Public"))
		json_object_object_add(sambadirObj, "X_ZYXEL_Public", json_object_new_boolean(Public));
		if(json_object_object_get(Jobj, "ValidUsers")){
		if(strcmp(ValidUser,""))
			json_object_object_add(sambadirObj, "X_ZYXEL_ValidUsers", json_object_new_string(ValidUser));
		}
		ret = zcfgFeObjJsonSet(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, sambadirObj, NULL);
		zcfgFeJsonObjFree(sambadirObj);
	}
	
	return ret;
}
zcfgRet_t zcfgFeDalUsbSambaDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t sambadirIid = {0};
	
	sambadirIid.level = 1;
	sambadirIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "Index"));

	zcfgFeObjJsonDel(RDM_OID_ZY_SAMBA_DIR, &sambadirIid, NULL);
	return ret;
}

zcfgRet_t zcfgFeDalUsbSambaGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *accountJobj = NULL;
	struct json_object *paramJobj = NULL;
	struct json_object *paramJobj1 = NULL;
	struct json_object *paramJobj2 = NULL;
	struct json_object *paramJobj3 = NULL;
#ifdef  ABOG_CUSTOMIZATION
	struct json_object *paramJobj4 = NULL;
#endif
	struct json_object *acctmpJobj = NULL;
	struct json_object *sambadirJobj = NULL;
	struct json_object *sambadirtmpJobj = NULL;
	struct json_object *usbdevJobj = NULL;
	struct json_object *usbdevtmpJobj = NULL;
#ifdef  ABOG_CUSTOMIZATION
	struct json_object *usbdevinfoJobj = NULL;
	struct json_object *usbdevinfotmpJobj = NULL;
#endif	
	struct json_object *sambaJobj = NULL;
	struct json_object *allJobj = NULL;
	struct json_object *obj = NULL;
#ifdef  ABOG_CUSTOMIZATION
	const char *capacity_GByte = NULL;
	const char *usedspace_GByte = NULL;
	const char *deviceclass = NULL;
	const char *manufacturer = NULL;
	const char *productclass = NULL;
	const char *serialnumber = NULL;
#endif	
	objIndex_t accountIid = {0};
	objIndex_t sambadirIid = {0};
	objIndex_t usbdevIid = {0};
	objIndex_t sambaIid = {0};
#ifdef  ABOG_CUSTOMIZATION
	objIndex_t usbdevinfoIid = {0};
#endif
	objIndex_t Iid = {0};
#ifdef CONFIG_ZYXEL_TR140
    struct json_object *ttaccountJobj = NULL;
    objIndex_t ttaccountIid = {0};
#endif
#ifdef ABUU_CUSTOMIZATION
	char loginLevel[16] = {0};

	if(getLoginUserInfo(Jobj, NULL, loginLevel, NULL) != ZCFG_SUCCESS){
		if(replyMsg) strcpy(replyMsg, "Internal Error, can't get account information");
		dbg_printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}
#endif
	
	acctmpJobj = json_object_new_array();
	sambadirtmpJobj = json_object_new_array();
	usbdevtmpJobj = json_object_new_array();
#ifdef  ABOG_CUSTOMIZATION	
	usbdevinfotmpJobj = json_object_new_array();
#endif	
	allJobj = json_object_new_object();
	accountIid.level = 2;
	accountIid.idx[0] = 2;

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
    const char *mode = NULL;
#endif
	const char *path = NULL;
	const char *sharename = NULL;
	char rootpath[512] = {0};

#ifdef CONFIG_ZCFG_BE_MODULE_USB
	//trigger usb backend get usb capacity and usedspace
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV, &Iid, &obj) == ZCFG_SUCCESS){
		zcfgFeJsonObjFree(obj);
	}
#else
	printf("%s : USB not support\n", __FUNCTION__);
#endif

#ifdef CONFIG_ZYXEL_TR140
    /*root & admin*/
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

    IID_INIT(ttaccountIid);
    ttaccountIid.level=2;
    ttaccountIid.idx[0]=2;
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

	/*samba group*/
    IID_INIT(ttaccountIid);
    ttaccountIid.level=2;
    ttaccountIid.idx[0]=3;
    while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &ttaccountIid, &ttaccountJobj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		if(json_object_object_get(ttaccountJobj, "Enabled") && json_object_object_get(ttaccountJobj, "Username")){
			json_object_object_add(paramJobj, "Status", JSON_OBJ_COPY(json_object_object_get(ttaccountJobj, "Enabled")));
			json_object_object_add(paramJobj, "Username", JSON_OBJ_COPY(json_object_object_get(ttaccountJobj, "Username")));
			json_object_array_add(acctmpJobj, paramJobj);
		}
		zcfgFeJsonObjFree(ttaccountJobj);
	}   
#else
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, &accountJobj) == ZCFG_SUCCESS){
#ifdef ABUU_CUSTOMIZATION
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
#endif
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
		json_object_array_add(sambadirtmpJobj, paramJobj1);
		zcfgFeJsonObjFree(sambadirJobj);
	}

#ifdef CONFIG_ZCFG_BE_MODULE_USB
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV_PAR, &usbdevIid, &usbdevJobj) == ZCFG_SUCCESS){
		paramJobj2 = json_object_new_object();
#ifdef  ABOG_CUSTOMIZATION			
		capacity_GByte = json_object_get_string(json_object_object_get(usbdevJobj, "Capacity_GByte"));
		usedspace_GByte = json_object_get_string(json_object_object_get(usbdevJobj, "UsedSpace_GByte"));
#endif			
		json_object_object_add(paramJobj2, "Volume", JSON_OBJ_COPY(json_object_object_get(usbdevJobj, "PartitionName")));
		json_object_object_add(paramJobj2, "Capacity", JSON_OBJ_COPY(json_object_object_get(usbdevJobj, "Capacity")));
		json_object_object_add(paramJobj2, "Used Space", JSON_OBJ_COPY(json_object_object_get(usbdevJobj, "UsedSpace")));
#ifdef  ABOG_CUSTOMIZATION			
		json_object_object_add(paramJobj2, "Capacity_GByte", json_object_new_string(capacity_GByte));
		json_object_object_add(paramJobj2, "UsedSpace_GByte", json_object_new_string(usedspace_GByte));
#endif			
		json_object_array_add(usbdevtmpJobj, paramJobj2);
		zcfgFeJsonObjFree(usbdevJobj);
	}
#endif

	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_SAMBA, &sambaIid, &sambaJobj) == ZCFG_SUCCESS){
		paramJobj3 = json_object_new_object();
		json_object_object_add(paramJobj3, "File Sharing Services", JSON_OBJ_COPY(json_object_object_get(sambaJobj, "Enable")));
		#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		bool fileshare_enable = false; 
		fileshare_enable = json_object_get_boolean(json_object_object_get(sambaJobj, "Enable"));
		mode = json_object_get_string(json_object_object_get(sambaJobj, "Mode"));
		if(fileshare_enable == true){
		if(!strcmp(mode,"LAN_ONLY")){
			json_object_object_add(paramJobj3, "WAN Enable", json_object_new_boolean(false));
		}
        else if(!strcmp(mode,"ALL")){
		    json_object_object_add(paramJobj3, "WAN Enable", json_object_new_boolean(true));
		}
		}
		else{
		    json_object_object_add(paramJobj3, "WAN Enable", json_object_new_boolean(false));
		}
		#endif
		json_object_object_add(allJobj, "Service Conf", paramJobj3);
		zcfgFeJsonObjFree(sambaJobj);
	}


#ifdef  ABOG_CUSTOMIZATION
#ifdef CONFIG_ZCFG_BE_MODULE_USB 
	usbdevinfoIid.level=2;
	usbdevinfoIid.idx[0]=1;
	usbdevinfoIid.idx[1]=2;
	if(zcfgFeObjJsonGet(RDM_OID_USB_USBHOSTS_HOST_DEV, &usbdevinfoIid, &usbdevinfoJobj) == ZCFG_SUCCESS){
		printf("usbdevinfoJobj: %s test %s: LINE:%d\n",usbdevinfoJobj,__FUNCTION__,__LINE__);
		fflush(stdout);
		paramJobj4 = json_object_new_object();
		deviceclass = json_object_get_string(json_object_object_get(usbdevinfoJobj, "DeviceClass"));
		manufacturer = json_object_get_string(json_object_object_get(usbdevinfoJobj, "Manufacturer"));
		productclass = json_object_get_string(json_object_object_get(usbdevinfoJobj, "ProductClass"));
		serialnumber = json_object_get_string(json_object_object_get(usbdevinfoJobj, "SerialNumber"));
		json_object_object_add(paramJobj4, "DeviceClass", json_object_new_string(deviceclass));
		json_object_object_add(paramJobj4, "Manufacturer", json_object_new_string(manufacturer));
		json_object_object_add(paramJobj4, "ProductClass", json_object_new_string(productclass));
		json_object_object_add(paramJobj4, "SerialNumber", json_object_new_string(serialnumber));
		json_object_array_add(usbdevinfotmpJobj, paramJobj4);
		zcfgFeJsonObjFree(usbdevinfoJobj);
	}
#endif
#endif
	json_object_object_add(allJobj, "Account", acctmpJobj);
	json_object_object_add(allJobj, "Samba", sambadirtmpJobj);
	json_object_object_add(allJobj, "Usb Info", usbdevtmpJobj);
#ifdef  ABOG_CUSTOMIZATION		
	json_object_object_add(allJobj, "Device_Info", usbdevinfotmpJobj);
#endif	
	json_object_array_add(Jarray, allJobj);
	return ret;
}

zcfgRet_t zcfgFeDalUsbSamba(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "POST"))
		ret = zcfgFeDalUsbSambaAdd(Jobj, replyMsg);
	else if(!strcmp(method, "PUT"))
	   ret = zcfgFeDalUsbSambaEdit(Jobj, NULL);
	else if(!strcmp(method, "DELETE"))
		ret = zcfgFeDalUsbSambaDelete(Jobj, NULL);
	else if(!strcmp(method,"GET"))
		ret = zcfgFeDalUsbSambaGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);
		
	return ret;
}

