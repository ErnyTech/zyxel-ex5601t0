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
#include "zos_api.h"

dal_param_t PORT_MIRROR_param[]={		
	{"Index", 	 				dalType_int,	0,	0,	NULL},
	{"LanPort", 	 dalType_string,	0,	0,	NULL,"LAN1|LAN2|LAN3|LAN4",0},
	{"Status", 	 dalType_boolean,	0,	0,	NULL},
	{"ZCFG_LOG_Enable", 	 dalType_boolean,	0,	0,	NULL},
	{"KERNEL_LOG_Enable", 	 dalType_boolean,	0,	0,	NULL},
	{"SysLogFileSize", 	 	dalType_int,	32,	16384,	NULL},
	{"Enable", 	 dalType_boolean,	0,	0,	NULL},
	{"LogServerIP", 	 dalType_v4Addr,	0,	0,	NULL},
	{"SendLogPeriodicTime", 	 dalType_int,	0,	0,	NULL},
	{"LogServerAccount", 	 dalType_string,	0,	0,	NULL},
	{"LogServerPwd", 	 dalType_string,	0,	0,	NULL},
	{NULL,					0,	0,	0,	NULL},

};

#ifdef CONFIG_TAAAB_PORTMIRROR
dal_param_t TAAAB_PORT_MIRROR_param[]={
	{"Index", 	 				dalType_int,	0,	0,	NULL},
	{"Enable",						dalType_boolean,	0,	0,		NULL,	NULL,				0},
	{"MonInterface", 	 				dalType_string,	0,	0,	NULL},
	{"VID", 	 				dalType_int,	0,	0,	NULL},
	{"Direction", 	 				dalType_string,	0,	0,	NULL},
	{"MirrorInterface", 	 				dalType_int,	0,	0,	NULL},
	{"Timeout", 	 				dalType_int,	0,	0,	NULL},
	{"Key", 	 				dalType_string,	0,	0,	NULL},
	{NULL,					0,	0,	0,	NULL},

};
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
dal_param_t PacketCapture_param[]={		
	{"Enable",						dalType_boolean,	0,	0,		NULL,	NULL,				0},
	{"MonitorInterface", 	 				dalType_string,	0,	0,	NULL},
	{"Direction", 	 dalType_string,	0,	0,	NULL,"ingress|egress|both",0},
	{"Filename", 	 dalType_string,	0,	0,	NULL},
	{"Storage", 	 dalType_string,	0,	0,	NULL,"internal|external",0},
	{"Timeout",					dalType_int,	0,	0,	NULL},
	{"UploadFilename",	 dalType_string,	0,	0,	NULL},
	{"UploadStorage",	 dalType_string,	0,	0,	NULL,"internal|external",0},
	{"UploadProtocol", 	 dalType_string,	0,	0,	NULL,"ftp|tftp",0},
	{"UploadURL",	 dalType_string,	0,	0,	NULL},
	{"UploadUsername",	 dalType_string,	0,	0,	NULL},
	{"UploadPassword", 	 dalType_string,	0,	0,	NULL},
	{NULL,					0,	0,	0,	NULL},

};
#endif

void zcfgFeDalShowPORT_MIRROR(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj,*obj2,*obj3;
	int Index;
	const char *Interface,*Direction, *LanPort,*Status;
	
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}	
	obj = json_object_array_get_idx(Jarray, 0);
	obj2 = json_object_object_get(obj, "portmirror");
	
	printf("%-7s %-10s %-12s %-10s %-10s\n",
		    "Index","Interface","Direction","LanPort","Status");
 		
	len = json_object_array_length(obj2);
	for(i=0;i<len;i++){
		obj3 = json_object_array_get_idx(obj2, i);
		if(obj3 != NULL){
			Index = json_object_get_int(json_object_object_get(obj3, "Index"));
			Interface = json_object_get_string(json_object_object_get(obj3, "MonitorInterface"));
			Direction = json_object_get_string(json_object_object_get(obj3, "Direction"));
			LanPort = json_object_get_string(json_object_object_get(obj3, "LanPort"));
			Status = json_object_get_string(json_object_object_get(obj3, "Status"));

			printf("%-7d %-10s %-12s %-10s %-10s\n",Index,Interface,Direction,LanPort,Status);
		}
	}	
 }





zcfgRet_t zcfgFeDal_PortMirror_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL, *ethObj = NULL, *portMirrorObj = NULL, *log_obj = NULL, *logObj = NULL, *loginInfo = NULL;
	objIndex_t ethIid = {0};
	objIndex_t portMirrorIid = {0};
	objIndex_t logIid = {0};
	int len = 0, i,index;
	const char *LanPort = NULL, *ethName = NULL, *loginlevel = NULL;
	char buff[5] = {0};
	bool status = false, log_enable = false, kernel_log_enable = false , AuthMode = false;
	struct json_object *newdbgObj = NULL, *curdbgObj = NULL;
	objIndex_t dbgIid = {0};
	bool new_dbg_enable = false, cur_dbg_enable = false;
	int new_period = 0, cur_period = 0;	
	const char *new_remote_IP = NULL, *cur_remote_IP = NULL, *new_server_account = NULL, *cur_server_account = NULL, *new_server_pwd = NULL, *cur_server_pwd = NULL , *new_rpcapd_IP = NULL;
	int syslog_size = 32;

	if(json_object_get_type(Jobj) == json_type_object){			//for CLI...
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		loginlevel = json_object_get_string(json_object_object_get(Jobj, "DAL_LOGIN_LEVEL"));
#ifdef ABUU_CUSTOMIZATION
		if(strcmp(loginlevel, "high") && strcmp(loginlevel, "vull")){
#else
		if(strcmp(loginlevel, "high")){
#endif
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.PortMirror.login_privilege_high"));
			return ZCFG_INVALID_PARAM_VALUE;
		}
		LanPort = json_object_get_string(json_object_object_get(Jobj, "LanPort"));
		while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &ethIid, &ethObj) == ZCFG_SUCCESS) {
			if(!strcmp(LanPort, json_object_get_string(json_object_object_get(ethObj, "X_ZYXEL_LanPort")))) {
				ethName = json_object_get_string(json_object_object_get(ethObj, "Name"));
				strcpy(buff, ethName);
				zcfgFeJsonObjFree(ethObj);
				break;
			}
			zcfgFeJsonObjFree(ethObj);
		}
		status = json_object_get_boolean(json_object_object_get(Jobj, "Status"));
		portMirrorIid.level = 1;
		portMirrorIid.idx[0] = index;
		if(zcfgFeObjJsonGet(RDM_OID_PORT_MIRROR, &portMirrorIid, &portMirrorObj) == ZCFG_SUCCESS) {
			json_object_object_add(portMirrorObj, "MirrorInterface", json_object_new_string(buff));
			json_object_object_add(portMirrorObj, "Status", json_object_new_boolean(status));
			zcfgFeObjJsonSet(RDM_OID_PORT_MIRROR, &portMirrorIid, portMirrorObj, NULL);
            zcfgFeJsonObjFree(portMirrorObj);
		}

	
		log_enable = json_object_get_boolean(json_object_object_get(Jobj, "ZCFG_LOG_Enable"));
		kernel_log_enable = json_object_get_boolean(json_object_object_get(Jobj, "KERNEL_LOG_Enable"));
		if(zcfgFeObjJsonGet(RDM_OID_LOG_SETTING, &logIid, &logObj) == ZCFG_SUCCESS) {
			json_object_object_add(logObj, "ZCFG_LOG_Enable", json_object_new_boolean(log_enable));
			json_object_object_add(logObj, "KERNEL_LOG_Enable", json_object_new_boolean(kernel_log_enable));

			zcfgFeObjJsonSet(RDM_OID_LOG_SETTING, &logIid, logObj, NULL);
	        zcfgFeJsonObjFree(logObj);
		}
	}
	else{
		
		loginInfo = obj = json_object_array_get_idx(Jobj, 0);
		loginlevel = json_object_get_string(json_object_object_get(loginInfo, "DAL_LOGIN_LEVEL"));
#ifdef ABUU_CUSTOMIZATION
		if(strcmp(loginlevel, "high") && strcmp(loginlevel, "vull")){
#else
		if(strcmp(loginlevel, "high")){
#endif
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.PortMirror.login_privilege_high"));
			return ZCFG_INVALID_PARAM_VALUE;
		}
		len = json_object_array_length(Jobj);
		for(i=0; i<len-2; i++) {
			obj = json_object_array_get_idx(Jobj, i);
			index = json_object_get_int(json_object_object_get(obj, "Index"));
			LanPort = json_object_get_string(json_object_object_get(obj, "LanPort"));
			IID_INIT(ethIid);
			while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &ethIid, &ethObj) == ZCFG_SUCCESS) {
				if(!strcmp(LanPort, json_object_get_string(json_object_object_get(ethObj, "X_ZYXEL_LanPort")))) {
					ethName = json_object_get_string(json_object_object_get(ethObj, "Name"));
					strcpy(buff, ethName);
					zcfgFeJsonObjFree(ethObj);
					break;
				}
				zcfgFeJsonObjFree(ethObj);
			}
			status = json_object_get_boolean(json_object_object_get(obj, "Status"));
			AuthMode = json_object_get_boolean(json_object_object_get(obj, "AuthMode"));
			new_rpcapd_IP = json_object_get_string(json_object_object_get(obj, "HOSTIP"));
			portMirrorIid.level = 1;
			portMirrorIid.idx[0] = index;
			if(zcfgFeObjJsonGet(RDM_OID_PORT_MIRROR, &portMirrorIid, &portMirrorObj) == ZCFG_SUCCESS) {
				json_object_object_add(portMirrorObj, "MirrorInterface", json_object_new_string(buff));
				json_object_object_add(portMirrorObj, "Status", json_object_new_boolean(status));
				if(new_rpcapd_IP != NULL)
				{
					json_object_object_add(portMirrorObj, "AuthMode", json_object_new_boolean(AuthMode));
					json_object_object_add(portMirrorObj, "HOSTIP", json_object_new_string(new_rpcapd_IP));
				}
				zcfgFeObjJsonSet(RDM_OID_PORT_MIRROR, &portMirrorIid, portMirrorObj, NULL);
				zcfgFeJsonObjFree(portMirrorObj);
			}
		}
		log_obj = json_object_array_get_idx(Jobj, len-2);
		log_enable = json_object_get_boolean(json_object_object_get(log_obj, "Log_Enable"));
		kernel_log_enable = json_object_get_boolean(json_object_object_get(log_obj, "KernelLog_Enable"));
		syslog_size = json_object_get_int(json_object_object_get(log_obj, "syslog_size"));
		if(zcfgFeObjJsonGet(RDM_OID_LOG_SETTING, &logIid, &logObj) == ZCFG_SUCCESS) {
			json_object_object_add(logObj, "ZCFG_LOG_Enable", json_object_new_boolean(log_enable));
			json_object_object_add(logObj, "KERNEL_LOG_Enable", json_object_new_boolean(kernel_log_enable));
			json_object_object_add(logObj, "SysLogFileSize", json_object_new_int(syslog_size));
			zcfgFeObjJsonSet(RDM_OID_LOG_SETTING, &logIid, logObj, NULL);
			zcfgFeJsonObjFree(logObj);
		}

		newdbgObj = json_object_object_get(json_object_array_get_idx(Jobj, len-1), "System_Debug");
		new_dbg_enable = json_object_get_boolean(json_object_object_get(newdbgObj, "Enable"));
		new_period = json_object_get_int(json_object_object_get(newdbgObj, "SendLogPeriodicTime"));
		new_remote_IP = json_object_get_string(json_object_object_get(newdbgObj, "LogServerIP"));
		new_server_account = json_object_get_string(json_object_object_get(newdbgObj, "LogServerAccount"));
		new_server_pwd = json_object_get_string(json_object_object_get(newdbgObj, "LogServerPwd"));
		if(zcfgFeObjJsonGet(RDM_OID_SYSTEM_DEBUG, &dbgIid, &curdbgObj) == ZCFG_SUCCESS) {
			cur_dbg_enable = json_object_get_boolean(json_object_object_get(curdbgObj, "Enable"));
			cur_period = json_object_get_int(json_object_object_get(curdbgObj, "SendLogPeriodicTime"));
			cur_remote_IP = json_object_get_string(json_object_object_get(curdbgObj, "LogServerIP"));
			cur_server_account = json_object_get_string(json_object_object_get(newdbgObj, "LogServerAccount"));
			cur_server_pwd = json_object_get_string(json_object_object_get(newdbgObj, "LogServerPwd"));

			if((new_dbg_enable != cur_dbg_enable) || (strcmp(new_remote_IP, cur_remote_IP)) || (new_period != cur_period) ||
				(strcmp(new_server_account, cur_server_account)) || (strcmp(new_server_pwd, cur_server_pwd))){
				json_object_object_add(curdbgObj, "Enable", json_object_new_boolean(new_dbg_enable));
				json_object_object_add(curdbgObj, "SendLogPeriodicTime", json_object_new_int(new_period));
				json_object_object_add(curdbgObj, "LogServerIP", json_object_new_string(new_remote_IP));
				json_object_object_add(curdbgObj, "LogServerAccount", json_object_new_string(new_server_account));
				json_object_object_add(curdbgObj, "LogServerPwd", json_object_new_string(new_server_pwd));

				zcfgFeObjJsonSet(RDM_OID_SYSTEM_DEBUG, &dbgIid, curdbgObj, NULL);
			}
			zcfgFeJsonObjFree(curdbgObj);
		}

		
	}	
	return ret;
}

zcfgRet_t updatePortMirrorTable(){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ipObj = NULL, *portMirrorObj = NULL, *obj = NULL;
	objIndex_t ipIid = {0}, portmirrorIid = {0}, oid = {0};
	char monitorIf[8] = "", ifName[8] = "";
	bool exist = false, outExist = false, inExist = false, direction;


#if 0 /* Remove behavior of "delete Port Mirror tables function" */

	/* 
		Check if every MonitorInterface(Port Mirror config) is exist and up 
	 	If not, delete it
	*/
	
	IID_INIT(portmirrorIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_PORT_MIRROR, &portmirrorIid, &portMirrorObj) == ZCFG_SUCCESS){
		ZOS_STRNCPY(monitorIf, json_object_get_string(json_object_object_get(portMirrorObj, "MonitorInterface")), 5);
		exist = false;
		IID_INIT(ipIid);
		while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipIid, &ipObj) == ZCFG_SUCCESS){
			ZOS_STRNCPY(ifName, json_object_get_string(json_object_object_get(ipObj, "X_ZYXEL_IfName")), 5);
			if(strstr(ifName, monitorIf) != NULL && !strcmp(json_object_get_string(json_object_object_get(ipObj, "Status")), "Up")
				&& json_object_get_boolean(json_object_object_get(ipObj, "Enable"))){
				exist = true;
				zcfgFeJsonObjFree(ipObj);
				break;
			}
			zcfgFeJsonObjFree(ipObj);
		}

		if(!exist)
			zcfgFeObjJsonDel(RDM_OID_PORT_MIRROR, &portmirrorIid, NULL);

		zcfgFeJsonObjFree(portMirrorObj);
	}
	
#endif

	// Add Current Interface to Port Mirror config(if interface is enabled and up)
	IID_INIT(ipIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipIid, &ipObj) == ZCFG_SUCCESS){
		ZOS_STRNCPY(ifName, json_object_get_string(json_object_object_get(ipObj, "X_ZYXEL_IfName")), 5);
		outExist = false;
		inExist = false;

		if(strcmp(ifName, "") && strcmp(ifName, "br0") && !strcmp(json_object_get_string(json_object_object_get(ipObj, "Status")), "Up")
			&& json_object_get_boolean(json_object_object_get(ipObj, "Enable"))){
			// Check if interface already exists
			IID_INIT(portmirrorIid);
			while(zcfgFeObjJsonGetNext(RDM_OID_PORT_MIRROR, &portmirrorIid, &portMirrorObj) == ZCFG_SUCCESS){
				ZOS_STRNCPY(monitorIf, json_object_get_string(json_object_object_get(portMirrorObj, "MonitorInterface")), 5);
				direction = json_object_get_boolean(json_object_object_get(portMirrorObj, "Direction"));
#if defined(BROADCOM_PLATFORM) && defined(XPON_SUPPORT) && (defined(CHIP_6858) || defined(CHIP_6856))
				if(!strcmp("wan0", monitorIf) && !direction){
					inExist = true;
				}
				if(!strcmp("wan0", monitorIf) && direction){
					outExist = true;
				}
#else
				if(!strcmp(ifName, monitorIf) && !direction){
					inExist = true;
				}
				if(!strcmp(ifName, monitorIf) && direction){
					outExist = true;
				}
#endif
				zcfgFeJsonObjFree(portMirrorObj);
			}
		
			// if interface does not exist, add it to Port Mirror Table
			// Direction: true > out flow, Direction: false > in flow
			if(!inExist){
				IID_INIT(oid);
				if(zcfgFeObjJsonAdd(RDM_OID_PORT_MIRROR, &oid, NULL) == ZCFG_SUCCESS){
					if(zcfgFeObjJsonGet(RDM_OID_PORT_MIRROR, &oid, &obj) == ZCFG_SUCCESS){
						json_object_object_add(obj, "MonitorInterface", json_object_new_string(ifName));
						json_object_object_add(obj, "Direction", json_object_new_boolean(false));
						zcfgFeObjJsonSetWithoutApply(RDM_OID_PORT_MIRROR, &oid, obj, NULL);
						zcfgFeJsonObjFree(obj);
					}
				}
			}
			if(!outExist){
				IID_INIT(oid);
				if(zcfgFeObjJsonAdd(RDM_OID_PORT_MIRROR, &oid, NULL) == ZCFG_SUCCESS){
					if(zcfgFeObjJsonGet(RDM_OID_PORT_MIRROR, &oid, &obj) == ZCFG_SUCCESS){
						json_object_object_add(obj, "MonitorInterface", json_object_new_string(ifName));
						json_object_object_add(obj, "Direction", json_object_new_boolean(true));
						zcfgFeObjJsonSetWithoutApply(RDM_OID_PORT_MIRROR, &oid, obj, NULL);
						zcfgFeJsonObjFree(obj);
					}
				}
			}
		}
		zcfgFeJsonObjFree(ipObj);
	}
	return ret;
}

zcfgRet_t zcfgFeDal_PortMirror_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL, *ethObj = NULL, *paramJobj = NULL, *port_mirror_array = NULL, *logObj = NULL, *tmp_array = NULL;
	objIndex_t iid = {0};
	objIndex_t ethIid = {0};
	objIndex_t logIid = {0};
	const char *mirrorIntf = NULL, *ethName = NULL, *LanPort = NULL, *loginlevel = NULL;
	bool direction = false;
	struct json_object *dbgObj = NULL;
	objIndex_t dbgIid = {0};

	
	loginlevel = json_object_get_string(json_object_object_get(Jobj, "DAL_LOGIN_LEVEL"));
#ifdef ABUU_CUSTOMIZATION
	if(strcmp(loginlevel, "high") && strcmp(loginlevel, "vull")){
#else
	if(strcmp(loginlevel, "high")){
#endif
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.PortMirror.login_privilege_high"));
		return ZCFG_INVALID_PARAM_VALUE;
	}

#if !defined(ECONET_PLATFORM) && !defined(CONFIG_ZYXEL_WEB_GUI_PORT_MIRROR_RPCAPD)
	updatePortMirrorTable();
#endif
	
	port_mirror_array = json_object_new_array();
	tmp_array = json_object_new_object();
	json_object_array_add(Jarray, tmp_array);
			
	while(zcfgFeObjJsonGetNext(RDM_OID_PORT_MIRROR, &iid, &obj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		
		json_object_object_add(paramJobj, "Index", json_object_new_int(iid.idx[0]));
		
		mirrorIntf = json_object_get_string(json_object_object_get(obj, "MirrorInterface"));
		IID_INIT(ethIid);
		while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &ethIid, &ethObj) == ZCFG_SUCCESS){
			ethName = json_object_get_string(json_object_object_get(ethObj, "Name"));
			if(!strcmp(mirrorIntf, ethName)){
				LanPort = json_object_get_string(json_object_object_get(ethObj, "X_ZYXEL_LanPort"));
				Jadds(paramJobj, "LanPort", LanPort);
				zcfgFeJsonObjFree(ethObj);
				break;
			}
			zcfgFeJsonObjFree(ethObj);
		}
		json_object_object_add(paramJobj, "MonitorInterface", JSON_OBJ_COPY(json_object_object_get(obj, "MonitorInterface")));
		direction = json_object_get_boolean(json_object_object_get(obj, "Direction"));
#if ECONET_PLATFORM
		Jadds(paramJobj, "Direction", "IN/OUT");
#else //if	//broadcom platform
		if(direction){
			Jadds(paramJobj, "Direction", "OUT");
		}
		else{
			Jadds(paramJobj, "Direction", "IN");
		}
#endif
		json_object_object_add(paramJobj, "Status", JSON_OBJ_COPY(json_object_object_get(obj, "Status")));
		json_object_object_add(paramJobj, "AuthMode", JSON_OBJ_COPY(json_object_object_get(obj, "AuthMode")));
		json_object_object_add(paramJobj, "HOSTIP", JSON_OBJ_COPY(json_object_object_get(obj, "HOSTIP")));
		json_object_array_add(port_mirror_array, paramJobj);
		zcfgFeJsonObjFree(obj);
	}
	if(zcfgFeObjJsonGet(RDM_OID_LOG_SETTING, &logIid, &logObj) == ZCFG_SUCCESS){
		json_object_object_add(tmp_array, "logSetting", logObj);	
	}
	json_object_object_add(tmp_array, "portmirror", port_mirror_array);

	if(zcfgFeObjJsonGet(RDM_OID_SYSTEM_DEBUG, &dbgIid, &dbgObj) == ZCFG_SUCCESS){
		json_object_object_add(tmp_array, "sys_dbg", dbgObj);
	}


	
	return ret;
}

zcfgRet_t zcfgFeDalPortMirror(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_PortMirror_Edit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_PortMirror_Get(Jobj, Jarray, NULL);
	}
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

#ifdef CONFIG_TAAAB_PORTMIRROR

zcfgRet_t checkstatus(char *key)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t accountiid;
	struct json_object *accountobj = NULL;

	IID_INIT(accountiid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountiid, &accountobj) == ZCFG_SUCCESS) {
		if(strcmp(Jgets(accountobj,"Username"),"root") == 0)
		{
			if(strcmp(Jgets(accountobj,"Password"),Jgets(accountobj,"DefaultPassword")) == 0 || strcmp(key,Jgets(accountobj,"Password"))!= 0)
			{
				ret = ZCFG_REQUEST_REJECT;
			}
		}
		zcfgFeJsonObjFree(accountobj);
	}

	return ret;
}

bool guiisEthWan(){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t objIid;
	char *lanPort = NULL;
	bool isethwan = false;
	
	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &objIid, &obj) == ZCFG_SUCCESS){
		lanPort = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_LanPort"));
		if(!strcmp(lanPort,"ETHWAN")){
	        isethwan = true;
			zcfgFeJsonObjFree(obj);
			break;
		}
			zcfgFeJsonObjFree(obj);
	}

	return isethwan;
}

int guiGetWorkingMirrIntf(int curindex){
    bool isethwan = false;
	int newindex = 0;

    isethwan = guiisEthWan();

	if(isethwan == true){
       if(curindex == 1)
	   	  newindex = 0;
	   else if(curindex == 2)
	   	  newindex = 1;
	   else if(curindex == 3)
	   	  newindex = 2;
	}
	else{
       if(curindex == 1)
	   	  newindex = 0;
	   else if(curindex == 2)
	   	  newindex = 1;
	   else if(curindex == 3)
	   	  newindex = 2;
	   else if(curindex == 4)
	   	  newindex = 4;
	}
	
    return newindex;
}

int GetPVCPortMirrorInfo(objIndex_t ipIfaceIid, struct json_object *ipIfaceObj, char *L2MonitorInterface){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char ipIfacePath[32];
	const char *HigherLayer;
	const char *LowerLayer;
	char CurrMode[32],CurrEncapsulation[32],CurrType[32];
	int count = 0;
	int VlanID = -2;
	objIndex_t pppIfaceIid;
	struct json_object *pppIfaceObj = NULL; 
	objIndex_t vlanTermIid;
	struct json_object *vlanTermObj = NULL;
	objIndex_t atmLinkIid;
	struct json_object *atmLinkObj = NULL;
	objIndex_t ethLinkIid;
	struct json_object *ethLinkObj = NULL;
	objIndex_t ptmLinkIid;
	struct json_object *ptmLinkObj = NULL;
	objIndex_t ethIfaceIid;
	struct json_object *ethIfaceObj = NULL;


	sprintf(ipIfacePath, "IP.Interface.%u", ipIfaceIid.idx[0]);

	LowerLayer = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
	strcpy(CurrMode, json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionType")));
	
	HigherLayer = ipIfacePath;

		while(count < 4){
			
			count++;
			if(!strncmp(HigherLayer, "IP.Interface.", 13)){
				if(!strncmp(LowerLayer, "PPP.Interface.", 14)){
					IID_INIT(pppIfaceIid);
					pppIfaceIid.level = 1;
					sscanf(LowerLayer, "PPP.Interface.%hhu", &pppIfaceIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIfaceObj);
					HigherLayer = LowerLayer;
					LowerLayer = json_object_get_string(json_object_object_get(pppIfaceObj, "LowerLayers"));
				}
				else if(!strncmp("Ethernet.VLANTermination.", LowerLayer, 25)){
					IID_INIT(vlanTermIid);
					vlanTermIid.level = 1;
					sscanf(LowerLayer, "Ethernet.VLANTermination.%hhu", &vlanTermIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, &vlanTermObj);
					HigherLayer = LowerLayer;
					LowerLayer = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
					strcpy(CurrEncapsulation, "IPoE");
				}
				else if(!strncmp("ATM.Link.", LowerLayer, 9) && !strcmp("IP_Routed", CurrMode)){
					IID_INIT(atmLinkIid);
					atmLinkIid.level = 1;
					sscanf(LowerLayer, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj);
					strcpy(CurrEncapsulation, "IPoA");
					strcpy(CurrType, "ATM");
					break;
				}
				else{
					return ZCFG_REQUEST_REJECT;
				}
			}
			else if(!strncmp(HigherLayer, "PPP.Interface.", 14)){
				if(!strncmp("Ethernet.VLANTermination.", LowerLayer, 25)){
					IID_INIT(vlanTermIid);
					vlanTermIid.level = 1;
					sscanf(LowerLayer, "Ethernet.VLANTermination.%hhu", &vlanTermIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, &vlanTermObj);
					HigherLayer = LowerLayer;
					LowerLayer = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
					strcpy(CurrEncapsulation, "PPPoE");
				}
				else if(!strncmp("ATM.Link.", LowerLayer, 9)){
					IID_INIT(atmLinkIid);
					atmLinkIid.level = 1;
					sscanf(LowerLayer, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj);
					strcpy(CurrEncapsulation, "PPPoA");
					strcpy(CurrType, "ATM");
					break;
				}
				else{
					return ZCFG_REQUEST_REJECT;
				}
			}
			else if(!strncmp("Ethernet.VLANTermination.", HigherLayer, 25)){
				if(!strncmp("Ethernet.Link.", LowerLayer, 14)){
					IID_INIT(ethLinkIid);
					ethLinkIid.level = 1;
					sscanf(LowerLayer, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkObj);
					HigherLayer = LowerLayer;
					LowerLayer = json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
				}
				else{
					return ZCFG_REQUEST_REJECT;
				}
			}
			else if(!strncmp("Ethernet.Link.", HigherLayer, 14)){
				if(!strncmp("ATM.Link.", LowerLayer, 9)){
					IID_INIT(atmLinkIid);
					atmLinkIid.level = 1;
					sscanf(LowerLayer, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj);
					strcpy(CurrType, "ATM");
					break;
				}
				else if(!strncmp("PTM.Link.", LowerLayer, 9)){
					IID_INIT(ptmLinkIid);
					ptmLinkIid.level = 1;
					sscanf(LowerLayer, "PTM.Link.%hhu", &ptmLinkIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PTM_LINK, &ptmLinkIid, &ptmLinkObj);
					strcpy(CurrType, "PTM");
					break;
				}
				else if(!strncmp("Ethernet.Interface.", LowerLayer, 19)){
					IID_INIT(ethIfaceIid);
					ethIfaceIid.level = 1;
					sscanf(LowerLayer, "Ethernet.Interface.%hhu", &ethIfaceIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_IFACE, &ethIfaceIid, &ethIfaceObj);
					strcpy(CurrType, "ETH");
					if(json_object_get_boolean(json_object_object_get(ethIfaceObj, "Upstream")) == false){
						if(json_object_get_boolean(json_object_object_get(ethLinkObj, "X_ZYXEL_OrigEthWAN")) == false){
							return ZCFG_REQUEST_REJECT;
						}
					}
					break;
				}
				else if(!strcmp("", LowerLayer)){
					if(json_object_get_boolean(json_object_object_get(ethLinkObj, "X_ZYXEL_OrigEthWAN")) == false){
						return ZCFG_REQUEST_REJECT;
					}
					strcpy(CurrType, "ETH");
					break;
				}
				else{
					return ZCFG_REQUEST_REJECT;
				}
			}
		}

		if(!strcmp(CurrType, "ATM")){
			strcpy(L2MonitorInterface,json_object_get_string(json_object_object_get(atmLinkObj, "X_ZYXEL_IfName")));
			VlanID = -2;
		}
		else if(!strcmp(CurrType, "PTM")){
			strcpy(L2MonitorInterface,json_object_get_string(json_object_object_get(ptmLinkObj, "X_ZYXEL_IfName")));
			if(vlanTermObj != NULL)
			    VlanID = json_object_get_int(json_object_object_get(vlanTermObj, "VLANID"));
			else
				VlanID = -1;
		}
		else if(!strcmp(CurrType, "ETH")){
			strcpy(L2MonitorInterface,json_object_get_string(json_object_object_get(ethIfaceObj, "Name")));
			if(vlanTermObj != NULL)
				VlanID = json_object_get_int(json_object_object_get(vlanTermObj, "VLANID"));
			else
				VlanID = -1;
		}
		else{
			ret = ZCFG_INTERNAL_ERROR;
			goto error;
		}
	
error:	
		if(pppIfaceObj != NULL)
			zcfgFeJsonObjFree(pppIfaceObj);
		if(vlanTermObj != NULL)
			zcfgFeJsonObjFree(vlanTermObj);
		if(atmLinkObj != NULL)
			zcfgFeJsonObjFree(atmLinkObj);
		if(ethLinkObj != NULL)
			zcfgFeJsonObjFree(ethLinkObj);
		if(ptmLinkObj != NULL)
			zcfgFeJsonObjFree(ptmLinkObj);
		if(ethIfaceObj != NULL)
			zcfgFeJsonObjFree(ethIfaceObj);
	
		return VlanID;
}

zcfgRet_t GUIUpdatePortMirrorEntry(void){
    objIndex_t objIid_new;
    objIndex_t ipIfaceIid;
	struct json_object *portmirrorObj = NULL;
    struct json_object *ipIfaceObj = NULL;
	char PVCL2If[32] = {0};
	int PVCVlanID = -2;
	bool foundIN = false;
	bool foundOUT = false;
	bool isethwan = false;
	const char *X_ZYXEL_SrvName = NULL, *X_ZYXEL_ConnectionType = NULL, *LowerLayers =NULL;

		isethwan = guiisEthWan();

        IID_INIT(objIid_new);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_PORT_MIRROR, &objIid_new, &portmirrorObj) == ZCFG_SUCCESS) {
			zcfgFeObjJsonDel(RDM_OID_PORT_MIRROR, &objIid_new, NULL);
			zcfgFeJsonObjFree(portmirrorObj);
		}

 		IID_INIT(ipIfaceIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS) {
			//skip lan & 3g wan
			X_ZYXEL_SrvName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_SrvName"));
			X_ZYXEL_ConnectionType = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionType"));
			LowerLayers = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
			if(X_ZYXEL_SrvName==NULL || X_ZYXEL_ConnectionType==NULL || LowerLayers==NULL
				 || !strcmp(X_ZYXEL_SrvName,"") || !strcmp(X_ZYXEL_ConnectionType,"") || !strcmp(LowerLayers,"")){
				if(ipIfaceObj != NULL)
				zcfgFeJsonObjFree(ipIfaceObj); 
				continue;
			}
		
			if(strstr(Jgets(ipIfaceObj,"X_ZYXEL_IfName"), "br") != NULL)
			{
				if(ipIfaceObj != NULL)
				zcfgFeJsonObjFree(ipIfaceObj); 
				continue;
			}
			
			if(strcmp(Jgets(ipIfaceObj,"X_ZYXEL_SrvName"),"WWAN") == 0)
			{
				if(ipIfaceObj != NULL)
				zcfgFeJsonObjFree(ipIfaceObj); 
				continue;
			}
		
			if(isethwan == false){
				if(strstr(Jgets(ipIfaceObj,"X_ZYXEL_SrvName"), "ETHWAN") != NULL)
				{
					if(ipIfaceObj != NULL)
					zcfgFeJsonObjFree(ipIfaceObj); 
					continue;
				}
			}
		
		PVCVlanID = GetPVCPortMirrorInfo(ipIfaceIid,ipIfaceObj,PVCL2If);
		foundIN = false;
		foundOUT = false;	
		IID_INIT(objIid_new);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_PORT_MIRROR, &objIid_new, &portmirrorObj) == ZCFG_SUCCESS) {
			if(!strcmp(Jgets(portmirrorObj,"MonitorInterface"), PVCL2If) && (Jgeti(portmirrorObj,"vlanID") == PVCVlanID) )
			{
			   if(Jgetb(portmirrorObj,"Direction"))
				foundOUT = true;
			   else
				foundIN = true;
			}
			zcfgFeJsonObjFree(portmirrorObj);
		}
		
		if(!foundOUT)
		{
			/*add new OUT entry*/
			IID_INIT(objIid_new);
			if(zcfgFeObjJsonAdd(RDM_OID_PORT_MIRROR, &objIid_new, NULL) == ZCFG_SUCCESS) {
				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PORT_MIRROR, &objIid_new, &portmirrorObj) == ZCFG_SUCCESS) {
					Jadds(portmirrorObj,"MonitorInterface",PVCL2If);
					Jaddi(portmirrorObj,"vlanID",PVCVlanID);
					Jaddb(portmirrorObj,"Direction",true);
					Jadds(portmirrorObj,"MirrorInterface","eth1");
					Jaddb(portmirrorObj,"Status",false);
                    zcfgFeObjJsonSetWithoutApply(RDM_OID_PORT_MIRROR,&objIid_new,portmirrorObj,NULL);
					zcfgFeJsonObjFree(portmirrorObj);
				}
				else {
					printf("Get PortMirror Entry Fail\n");
				}
	        }
	        else {
	            printf("Add PortMirror Entry Fail\n");
	        }
		}
		
		if(!foundIN)
		{
			/*add new IN entry*/
			IID_INIT(objIid_new);
			if(zcfgFeObjJsonAdd(RDM_OID_PORT_MIRROR, &objIid_new, NULL) == ZCFG_SUCCESS) {
				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PORT_MIRROR, &objIid_new, &portmirrorObj) == ZCFG_SUCCESS) {		
					Jadds(portmirrorObj,"MonitorInterface",PVCL2If);
					Jaddi(portmirrorObj,"vlanID",PVCVlanID);
					Jaddb(portmirrorObj,"Direction",false);
					Jadds(portmirrorObj,"MirrorInterface","eth1");
					Jaddb(portmirrorObj,"Status",false);
                    zcfgFeObjJsonSetWithoutApply(RDM_OID_PORT_MIRROR,&objIid_new,portmirrorObj,NULL);
					zcfgFeJsonObjFree(portmirrorObj);
				}
				else {
					printf("Get PortMirror Entry Fail\n");
				}
	        }
	        else {
	            printf("Add PortMirror Entry Fail\n");
	        }
		}

		if(ipIfaceObj != NULL)
		zcfgFeJsonObjFree(ipIfaceObj); 
    }

	return ZCFG_SUCCESS;

}

zcfgRet_t zcfgFeDal_TTPortMirror_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int len = 0, i;
    objIndex_t setttmirrorIid = {0};
	objIndex_t setmirrorIid = {0};
	struct json_object *setobj = NULL, *setttmirrorObj = NULL, *setportMirrorObj = NULL, *ttmirrorObj = NULL;
	const char *action = NULL;
	const char *ttmonIface = NULL, *ttdirection = NULL, *ttkey = NULL;
	bool ttmirrorenable = false;
	int ttmirrorIface = 0, ttmirrortimeout = 0;
	const char *setIfaceName = NULL;
	bool setstatus = false;
	int setvlan = 0;
	int newmirrIface = 0;
	char mirrorInter[128] = "";
	bool Ischange = false;
	
	len = json_object_array_length(Jobj);
	ttmirrorObj = json_object_array_get_idx(Jobj, len-1);
	action = Jgets(ttmirrorObj,"Action");

	if(action == NULL)
			return ZCFG_INVALID_PARAM_VALUE;
	
		ttmirrorenable = Jgetb(ttmirrorObj,"MirrorEnable");
		ttmonIface = Jgets(ttmirrorObj,"MonitorInterface");
		ttdirection = Jgets(ttmirrorObj,"Direction");
		ttmirrorIface = Jgeti(ttmirrorObj,"MirrorInterface");
		ttmirrortimeout = Jgeti(ttmirrorObj,"Timeout");	
		ttkey = Jgets(ttmirrorObj,"Key");

		if(checkstatus(ttkey) != ZCFG_SUCCESS)
			return ZCFG_INVALID_PARAM_VALUE;

		//set TAAAB mirror parameters
		IID_INIT(setttmirrorIid);
		if(zcfgFeObjJsonGet(RDM_OID_TAAAB_PORT_MIRROR, &setttmirrorIid, &setttmirrorObj) == ZCFG_SUCCESS) {
			json_object_object_add(setttmirrorObj, "Enable", json_object_new_boolean(ttmirrorenable));
			json_object_object_add(setttmirrorObj, "Direction", json_object_new_string(ttdirection));
			json_object_object_add(setttmirrorObj, "Timeout", json_object_new_int(ttmirrortimeout));
			json_object_object_add(setttmirrorObj, "MirrorInterface", json_object_new_int(ttmirrorIface));
			json_object_object_add(setttmirrorObj, "MonitorInterface", json_object_new_string(ttmonIface));
			if(!strcmp(action,"mirrorset"))
			   zcfgFeObjJsonSetWithoutApply(RDM_OID_TAAAB_PORT_MIRROR, &setttmirrorIid, setttmirrorObj, NULL);
			else
			   zcfgFeObjJsonSet(RDM_OID_TAAAB_PORT_MIRROR, &setttmirrorIid, setttmirrorObj, NULL);
			zcfgFeJsonObjFree(setttmirrorObj);
		}

		//sync Zyxel mirror parameters
		GUIUpdatePortMirrorEntry();
		
		for(i=0; i<len-1; i++) {
			setobj = json_object_array_get_idx(Jobj, i);
			setIfaceName = Jgets(setobj,"MonInterface");
			setvlan = Jgeti(setobj,"VID");
			setstatus = Jgetb(setobj,"Enable");

			IID_INIT(setmirrorIid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_PORT_MIRROR, &setmirrorIid, &setportMirrorObj) == ZCFG_SUCCESS) {
			   Ischange = false;
	           if(!strcmp(Jgets(setportMirrorObj,"MonitorInterface"),setIfaceName) && setvlan == Jgeti(setportMirrorObj,"vlanID") )
			   {
					//mirrorInterface
					newmirrIface = guiGetWorkingMirrIntf(ttmirrorIface);
					sprintf(mirrorInter,"eth%d",newmirrIface);
					if(strcmp(Jgets(setportMirrorObj,"MirrorInterface"), mirrorInter) != 0)
					{	
						json_object_object_add(setportMirrorObj, "MirrorInterface", json_object_new_string(mirrorInter));
						Ischange = true;
					}
				
					//In + Out
					if(strcmp(ttdirection, "both") == 0){
						if(Jgetb(setportMirrorObj,"Status") != setstatus ){
							json_object_object_add(setportMirrorObj, "Status", json_object_new_boolean(setstatus));
							Ischange = true;
						}
					}
					//In
					else if((strcmp(ttdirection, "ingress") == 0) && (Jgetb(setportMirrorObj,"Direction") == false)){
						if(Jgetb(setportMirrorObj,"Status") != setstatus ) {
							json_object_object_add(setportMirrorObj, "Status", json_object_new_boolean(setstatus));
							Ischange = true;
						}
					}
					//Out
					else if((strcmp(ttdirection, "egress") == 0) && (Jgetb(setportMirrorObj,"Direction") == true)){
						if(Jgetb(setportMirrorObj,"Status") != setstatus ){
							json_object_object_add(setportMirrorObj, "Status", json_object_new_boolean(setstatus));
							Ischange = true;
						}
					}
					else{
						if(Jgetb(setportMirrorObj,"Status") != false ){
							json_object_object_add(setportMirrorObj, "Status", json_object_new_boolean(false));
							Ischange = true;
						}
					}
				}

                   if(Ischange == true){
					if(!strcmp(action,"mirrorset"))
					  zcfgFeObjJsonSetWithoutApply(RDM_OID_PORT_MIRROR, &setmirrorIid, setportMirrorObj, NULL);
					else
			          zcfgFeObjJsonSet(RDM_OID_PORT_MIRROR, &setmirrorIid, setportMirrorObj, NULL);
                   }
				   else{
			        zcfgFeJsonObjFree(setportMirrorObj);
				   }
	         }
		}

	return ret;
}

zcfgRet_t zcfgFeDal_TTPortMirror_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL, *port_mirror_array = NULL, *ttmirrorObj = NULL, *tmp_array = NULL;
	objIndex_t ttmirrorIid = {0};
	objIndex_t ipIfaceIid;
    struct json_object *ipIfaceObj = NULL;
	char PVCL2If[32] = {0};
	int PVCVlanID = -2;
	char ipIfacePath[32] = {0};
	char loginName[32] = {0};
	struct json_object *accountobj = NULL;
	objIndex_t accountiid = {0};
	const char *curusername = NULL, *curpassword = NULL, *defpassword = NULL;
	bool isethwan = false;
	const char *X_ZYXEL_SrvName = NULL, *X_ZYXEL_ConnectionType = NULL, *LowerLayers =NULL;
	int inifcount = 0;
	
	port_mirror_array = json_object_new_array();
	tmp_array = json_object_new_object();
			
	isethwan = guiisEthWan();
			
		IID_INIT(ttmirrorIid);
		zcfgFeObjJsonGet(RDM_OID_TAAAB_PORT_MIRROR, &ttmirrorIid, &ttmirrorObj);

		IID_INIT(ipIfaceIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS) {
			//skip lan & 3g wan
			X_ZYXEL_SrvName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_SrvName"));
			X_ZYXEL_ConnectionType = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionType"));
			LowerLayers = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
			if(X_ZYXEL_SrvName==NULL || X_ZYXEL_ConnectionType==NULL || LowerLayers==NULL
				 || !strcmp(X_ZYXEL_SrvName,"") || !strcmp(X_ZYXEL_ConnectionType,"") || !strcmp(LowerLayers,"")){
				if(ipIfaceObj != NULL)
				zcfgFeJsonObjFree(ipIfaceObj); 
				continue;
			}
		
			if(strstr(Jgets(ipIfaceObj,"X_ZYXEL_IfName"), "br") != NULL)
			{
				if(ipIfaceObj != NULL)
				zcfgFeJsonObjFree(ipIfaceObj); 
				continue;
			}
			
			if(strcmp(Jgets(ipIfaceObj,"X_ZYXEL_SrvName"),"WWAN") == 0)
			{
				if(ipIfaceObj != NULL)
				zcfgFeJsonObjFree(ipIfaceObj); 
				continue;
			}
		
			if(isethwan == false){
				if(strstr(Jgets(ipIfaceObj,"X_ZYXEL_SrvName"), "ETHWAN") != NULL)
				{
					if(ipIfaceObj != NULL)
					zcfgFeJsonObjFree(ipIfaceObj); 
					continue;
				}
			}
		
		PVCVlanID = GetPVCPortMirrorInfo(ipIfaceIid,ipIfaceObj,PVCL2If);

		paramJobj = json_object_new_object();
		
		json_object_object_add(paramJobj, "Index", json_object_new_int(ipIfaceIid.idx[0]));

		json_object_object_add(paramJobj, "MonInterface", json_object_new_string(PVCL2If));

		json_object_object_add(paramJobj, "VID", json_object_new_int(PVCVlanID));

        sprintf(ipIfacePath, "IP.Interface.%u", ipIfaceIid.idx[0]);

		json_object_object_add(paramJobj, "MonIfacePath", json_object_new_string(ipIfacePath));

		if(strstr(Jgets(ttmirrorObj,"MonitorInterface"),ipIfacePath))
           json_object_object_add(paramJobj, "Enable", json_object_new_boolean(true));
		else
		   json_object_object_add(paramJobj, "Enable", json_object_new_boolean(false));

		json_object_array_add(port_mirror_array, paramJobj);

		inifcount++;
		
		zcfgFeJsonObjFree(ipIfaceObj);
	}

	json_object_object_add(tmp_array, "portmirror", port_mirror_array);

	paramJobj = json_object_new_object();
	json_object_object_add(paramJobj, "MirrorFlag", JSON_OBJ_COPY(json_object_object_get(ttmirrorObj, "Enable")));
	json_object_object_add(paramJobj, "MonitorInterface", JSON_OBJ_COPY(json_object_object_get(ttmirrorObj, "MonitorInterface")));
	json_object_object_add(paramJobj, "Direction", JSON_OBJ_COPY(json_object_object_get(ttmirrorObj, "Direction")));
	json_object_object_add(paramJobj, "MirrorInterface", JSON_OBJ_COPY(json_object_object_get(ttmirrorObj, "MirrorInterface")));
	json_object_object_add(paramJobj, "Timeout", JSON_OBJ_COPY(json_object_object_get(ttmirrorObj, "Timeout")));
	json_object_object_add(paramJobj, "Key", JSON_OBJ_COPY(json_object_object_get(ttmirrorObj, "Key")));

    if(0==inifcount%2)
		json_object_object_add(paramJobj, "tablemid", json_object_new_int(inifcount/2));
	else
	    json_object_object_add(paramJobj, "tablemid", json_object_new_int((inifcount-1)/2));
		
	if(getLoginUserInfo(Jobj, loginName, NULL, NULL) != ZCFG_SUCCESS){
		if(replyMsg) strcpy(replyMsg, "Internal Error, can't get account information");
		return ZCFG_INTERNAL_ERROR;
	}

	if(!strcmp(loginName,"root"))
	{
			IID_INIT(accountiid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountiid, &accountobj) == ZCFG_SUCCESS) {
				curusername = json_object_get_string(json_object_object_get(accountobj, "Username"));
				if(!strcmp(curusername,"root")){
                   curpassword = json_object_get_string(json_object_object_get(accountobj, "Password"));
				   defpassword = json_object_get_string(json_object_object_get(accountobj, "DefaultPassword"));
				   if(!strcmp(curpassword,defpassword))
		           {
						   json_object_object_add(paramJobj, "UserPwdChanged", json_object_new_boolean(false));
		           }
		           else
		           {
						  json_object_object_add(paramJobj, "UserPwdChanged", json_object_new_boolean(true));
		           }
				   zcfgFeJsonObjFree(accountobj);
				   break;
                }
				zcfgFeJsonObjFree(accountobj);
			}
	}
	else
		json_object_object_add(paramJobj, "UserPwdChanged", json_object_new_boolean(true));
	
	json_object_object_add(tmp_array, "ttmirror", paramJobj);
	zcfgFeJsonObjFree(ttmirrorObj);

	json_object_array_add(Jarray, tmp_array);

	return ret;
}

zcfgRet_t zcfgFeDalTTPortMirror(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_TTPortMirror_Edit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_TTPortMirror_Get(Jobj, Jarray, NULL);
	}
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
zcfgRet_t zcfgFeDal_PacketCapture_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *packetcaptureObj = NULL;
	struct json_object *packetcaptureuploadObj = NULL;
	objIndex_t Iid = {0};
	const char *intf = NULL, *direction = NULL, *filename = NULL, *storage = NULL, *clearflag = NULL;
	const char *upprotocol = NULL, *upfilename = NULL, *upstorage = NULL, *upurl = NULL, *upusername = NULL, *uppassword = NULL, *upflag = NULL;
	const char *action = NULL;
	bool pcenable = false;
	int timeout = 0;
	objIndex_t ipIfaceIid;
	struct json_object *ipIfaceObj = NULL;
	objIndex_t newipIfaceIid;
	struct json_object *newipIfaceObj = NULL;
	objIndex_t dslChannelIid;
	struct json_object *dslChannelObj = NULL;
	const char *dslType;
	const char *dslStatus;
	const char *New_SrvName = NULL;
	const char *New_TAAAB_SrvName = NULL;
	int WanConnectType = 0;
	char finalintf[64] = {0};
	char tmpintf[64] = {0};

	action = json_object_get_string(json_object_object_get(Jobj, "Action"));

	if(action == NULL)
		return ZCFG_INVALID_PARAM_VALUE;

    if(!strcmp(action,"capturestart") || !strcmp(action,"capturestop") || !strcmp(action,"capturesetup")){
	intf = json_object_get_string(json_object_object_get(Jobj, "MonitorInterface"));

	if(!strcmp(intf,"ALL")){
		strcpy(finalintf,intf);
	}
	else{
	strcpy(tmpintf,intf);
	IID_INIT(newipIfaceIid);
	newipIfaceIid.level = 1;
	sscanf(tmpintf, "IP.Interface.%hhu", &newipIfaceIid.idx[0]);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &newipIfaceIid, &newipIfaceObj) == ZCFG_SUCCESS)
	{
	    New_SrvName = Jgets(newipIfaceObj, "X_ZYXEL_SrvName");
		New_TAAAB_SrvName = Jgets(newipIfaceObj, "X_TT_SrvName");
	}		

    if(strstr(New_TAAAB_SrvName,"_DSL")!= NULL){	
	IID_INIT(dslChannelIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DSL_CHANNEL, &dslChannelIid, &dslChannelObj) == ZCFG_SUCCESS) {
		dslType = json_object_get_string(json_object_object_get(dslChannelObj, "LinkEncapsulationUsed"));
		dslStatus = json_object_get_string(json_object_object_get(dslChannelObj, "Status"));
		if(strstr(dslType,"ATM") && !strcmp(dslStatus,"Up")){
		   WanConnectType = 1;
		   json_object_put(dslChannelObj);
		   break;
		}
		else if(strstr(dslType,"PTM") && !strcmp(dslStatus,"Up")){
		   WanConnectType = 2;
		   json_object_put(dslChannelObj);
		   break;
		}
		
		json_object_put(dslChannelObj); 	
	}  

    if(WanConnectType == 1){
		IID_INIT(ipIfaceIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS) {
			if(!strcmp(Jgets(ipIfaceObj, "X_TT_SrvName"),New_TAAAB_SrvName) && strcmp(Jgets(ipIfaceObj, "X_ZYXEL_SrvName"),New_SrvName) && strstr(Jgets(ipIfaceObj, "X_ZYXEL_SrvName"),"_ADSL"))
			{
					sprintf(finalintf, "IP.Interface.%u", ipIfaceIid.idx[0]);
					zcfgFeJsonObjFree(ipIfaceObj);
					break;
			}
			zcfgFeJsonObjFree(ipIfaceObj);
		}
    }
	else{
		strcpy(finalintf,intf);
	}
    }
	else{
         strcpy(finalintf,intf);
	}
				if(newipIfaceObj != NULL)
			     zcfgFeJsonObjFree(newipIfaceObj);
	}
    }
	
	if(!strcmp(action,"capturestart"))
	{
	pcenable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	timeout = json_object_get_int(json_object_object_get(Jobj, "Timeout"));
	direction = json_object_get_string(json_object_object_get(Jobj, "Direction"));
	filename = json_object_get_string(json_object_object_get(Jobj, "Filename"));
	storage = json_object_get_string(json_object_object_get(Jobj, "Storage"));

    IID_INIT(Iid);
	ret = zcfgFeObjJsonGet(RDM_OID_PACKET_CAPTURE, &Iid, &packetcaptureObj);
	if(ret == ZCFG_SUCCESS){
		    json_object_object_add(packetcaptureObj, "Enable", json_object_new_boolean(pcenable));
			json_object_object_add(packetcaptureObj, "Timeout", json_object_new_int(timeout));
			json_object_object_add(packetcaptureObj, "MonitorInterface", json_object_new_string(finalintf));
			json_object_object_add(packetcaptureObj, "Direction", json_object_new_string(direction));
			json_object_object_add(packetcaptureObj, "FileName", json_object_new_string(filename));
			json_object_object_add(packetcaptureObj, "Storage", json_object_new_string(storage));

		ret = zcfgFeObjJsonSet(RDM_OID_PACKET_CAPTURE, &Iid, packetcaptureObj, NULL);
		zcfgFeJsonObjFree(packetcaptureObj);
	}
	}
	else if(!strcmp(action,"capturestop"))
	{
	pcenable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	timeout = json_object_get_int(json_object_object_get(Jobj, "Timeout"));
	direction = json_object_get_string(json_object_object_get(Jobj, "Direction"));
	filename = json_object_get_string(json_object_object_get(Jobj, "Filename"));
	storage = json_object_get_string(json_object_object_get(Jobj, "Storage"));

    IID_INIT(Iid);
	ret = zcfgFeObjJsonGet(RDM_OID_PACKET_CAPTURE, &Iid, &packetcaptureObj);
	if(ret == ZCFG_SUCCESS){
		    json_object_object_add(packetcaptureObj, "Enable", json_object_new_boolean(pcenable));
			json_object_object_add(packetcaptureObj, "Timeout", json_object_new_int(timeout));
			json_object_object_add(packetcaptureObj, "MonitorInterface", json_object_new_string(finalintf));
			json_object_object_add(packetcaptureObj, "Direction", json_object_new_string(direction));
			json_object_object_add(packetcaptureObj, "FileName", json_object_new_string(filename));
			json_object_object_add(packetcaptureObj, "Storage", json_object_new_string(storage));

		ret = zcfgFeObjJsonSet(RDM_OID_PACKET_CAPTURE, &Iid, packetcaptureObj, NULL);
		zcfgFeJsonObjFree(packetcaptureObj);
	}
	}
	else if(!strcmp(action,"capturesetup"))
	{
	pcenable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	timeout = json_object_get_int(json_object_object_get(Jobj, "Timeout"));
	direction = json_object_get_string(json_object_object_get(Jobj, "Direction"));
	filename = json_object_get_string(json_object_object_get(Jobj, "Filename"));
	storage = json_object_get_string(json_object_object_get(Jobj, "Storage"));

    IID_INIT(Iid);
	ret = zcfgFeObjJsonGet(RDM_OID_PACKET_CAPTURE, &Iid, &packetcaptureObj);
	if(ret == ZCFG_SUCCESS){
		    json_object_object_add(packetcaptureObj, "Enable", json_object_new_boolean(pcenable));
			json_object_object_add(packetcaptureObj, "Timeout", json_object_new_int(timeout));
			json_object_object_add(packetcaptureObj, "MonitorInterface", json_object_new_string(finalintf));
			json_object_object_add(packetcaptureObj, "Direction", json_object_new_string(direction));
			json_object_object_add(packetcaptureObj, "FileName", json_object_new_string(filename));
			json_object_object_add(packetcaptureObj, "Storage", json_object_new_string(storage));

		ret = zcfgFeObjJsonSet(RDM_OID_PACKET_CAPTURE, &Iid, packetcaptureObj, NULL);
		zcfgFeJsonObjFree(packetcaptureObj);
	}
	}
	else if(!strcmp(action,"captureclear"))
	{
	clearflag = json_object_get_string(json_object_object_get(Jobj, "clearflag"));

    IID_INIT(Iid);
	ret = zcfgFeObjJsonGet(RDM_OID_PACKET_CAPTURE, &Iid, &packetcaptureObj);
	if(ret == ZCFG_SUCCESS){
			json_object_object_add(packetcaptureObj, "Clear", json_object_new_string(clearflag));

		ret = zcfgFeObjJsonSet(RDM_OID_PACKET_CAPTURE, &Iid, packetcaptureObj, NULL);
		zcfgFeJsonObjFree(packetcaptureObj);
	}
	}
	else if(!strcmp(action,"upload"))
	{
	upflag = json_object_get_string(json_object_object_get(Jobj, "UploadFlag"));
	upprotocol = json_object_get_string(json_object_object_get(Jobj, "UploadProtocol"));
	upfilename = json_object_get_string(json_object_object_get(Jobj, "UploadFilename"));
	upstorage = json_object_get_string(json_object_object_get(Jobj, "UploadFilesource"));
	upurl = json_object_get_string(json_object_object_get(Jobj, "UploadURL"));
	upusername = json_object_get_string(json_object_object_get(Jobj, "UploadUsername"));
	uppassword = json_object_get_string(json_object_object_get(Jobj, "UploadPassword"));

    IID_INIT(Iid);
	ret = zcfgFeObjJsonGet(RDM_OID_PACKET_CAPTURE_UPLOAD, &Iid, &packetcaptureuploadObj);
	if(ret == ZCFG_SUCCESS){
		    json_object_object_add(packetcaptureuploadObj, "State", json_object_new_string(upflag));
			json_object_object_add(packetcaptureuploadObj, "FileName", json_object_new_string(upfilename));
			json_object_object_add(packetcaptureuploadObj, "FileSource", json_object_new_string(upstorage));
			json_object_object_add(packetcaptureuploadObj, "URL", json_object_new_string(upurl));
			json_object_object_add(packetcaptureuploadObj, "Protocol", json_object_new_string(upprotocol));
			if(!strcmp(upprotocol,"ftp")){
			     json_object_object_add(packetcaptureuploadObj, "Username", json_object_new_string(upusername));
				 json_object_object_add(packetcaptureuploadObj, "Password", json_object_new_string(uppassword));
			}

		ret = zcfgFeObjJsonSet(RDM_OID_PACKET_CAPTURE_UPLOAD, &Iid, packetcaptureuploadObj, NULL);
		zcfgFeJsonObjFree(packetcaptureuploadObj);
	}
	}
	else if(!strcmp(action,"upsetup"))
	{
	upprotocol = json_object_get_string(json_object_object_get(Jobj, "UploadProtocol"));
	upfilename = json_object_get_string(json_object_object_get(Jobj, "UploadFilename"));
	upstorage = json_object_get_string(json_object_object_get(Jobj, "UploadFilesource"));
	upurl = json_object_get_string(json_object_object_get(Jobj, "UploadURL"));
	upusername = json_object_get_string(json_object_object_get(Jobj, "UploadUsername"));
	uppassword = json_object_get_string(json_object_object_get(Jobj, "UploadPassword"));

    IID_INIT(Iid);
	ret = zcfgFeObjJsonGet(RDM_OID_PACKET_CAPTURE_UPLOAD, &Iid, &packetcaptureuploadObj);
	if(ret == ZCFG_SUCCESS){
			json_object_object_add(packetcaptureuploadObj, "FileName", json_object_new_string(upfilename));
			json_object_object_add(packetcaptureuploadObj, "FileSource", json_object_new_string(upstorage));
			json_object_object_add(packetcaptureuploadObj, "URL", json_object_new_string(upurl));
			json_object_object_add(packetcaptureuploadObj, "Protocol", json_object_new_string(upprotocol));
			if(!strcmp(upprotocol,"ftp")){
			     json_object_object_add(packetcaptureuploadObj, "Username", json_object_new_string(upusername));
				 json_object_object_add(packetcaptureuploadObj, "Password", json_object_new_string(uppassword));
			}

		ret = zcfgFeObjJsonSet(RDM_OID_PACKET_CAPTURE_UPLOAD, &Iid, packetcaptureuploadObj, NULL);
		zcfgFeJsonObjFree(packetcaptureuploadObj);
	}
	}

	return ret;
}

zcfgRet_t zcfgFeDal_PacketCapture_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *PacketCaptureJobj = NULL;
	objIndex_t PacketCaptureIid = {0};
	struct json_object *PacketCaptureUploadJobj = NULL;
	objIndex_t PacketCaptureUploadIid = {0};
	char loginName[32] = {0};
	struct json_object *accountobj = NULL;
	objIndex_t accountiid = {0};
	const char *curusername = NULL, *curpassword = NULL, *defpassword = NULL;
	const char *intf = NULL;
	objIndex_t ipIfaceIid;
	struct json_object *ipIfaceObj = NULL;
	objIndex_t newipIfaceIid;
	struct json_object *newipIfaceObj = NULL;
	const char *New_SrvName = NULL;
	const char *New_TAAAB_SrvName = NULL;
	char finalintf[64] = {0};
	char tmpintf[64] = {0};


	paramJobj = json_object_new_object();

    IID_INIT(PacketCaptureIid);
	if(zcfgFeObjJsonGet(RDM_OID_PACKET_CAPTURE, &PacketCaptureIid, &PacketCaptureJobj) == ZCFG_SUCCESS){

		intf = json_object_get_string(json_object_object_get(PacketCaptureJobj, "MonitorInterface"));
		
		if(!strcmp(intf,"ALL")){
			strcpy(finalintf,intf);
		}
		else{
		strcpy(tmpintf,intf);
		IID_INIT(newipIfaceIid);
		newipIfaceIid.level = 1;
		sscanf(tmpintf, "IP.Interface.%hhu", &newipIfaceIid.idx[0]);
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &newipIfaceIid, &newipIfaceObj) == ZCFG_SUCCESS)
		{
			New_SrvName = Jgets(newipIfaceObj, "X_ZYXEL_SrvName");
			New_TAAAB_SrvName = Jgets(newipIfaceObj, "X_TT_SrvName");
		}		
		
		if(strstr(New_TAAAB_SrvName,"_DSL")!= NULL){	
		if(strstr(New_SrvName,"_ADSL")!= NULL){
			IID_INIT(ipIfaceIid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS) {
				if(!strcmp(Jgets(ipIfaceObj, "X_TT_SrvName"),New_TAAAB_SrvName) && strcmp(Jgets(ipIfaceObj, "X_ZYXEL_SrvName"),New_SrvName) && strstr(Jgets(ipIfaceObj, "X_ZYXEL_SrvName"),"_VDSL"))
				{
						sprintf(finalintf, "IP.Interface.%u", ipIfaceIid.idx[0]);
						zcfgFeJsonObjFree(ipIfaceObj);
						break;
				}
				zcfgFeJsonObjFree(ipIfaceObj);
			}
		}
		else{
			strcpy(finalintf,intf);
		}
		}
		else{
			 strcpy(finalintf,intf);
		}
					if(newipIfaceObj != NULL)
					 zcfgFeJsonObjFree(newipIfaceObj);
		}

				json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(PacketCaptureJobj, "Enable")));
				json_object_object_add(paramJobj, "MonitorInterface", json_object_new_string(finalintf));
				json_object_object_add(paramJobj, "Direction", JSON_OBJ_COPY(json_object_object_get(PacketCaptureJobj, "Direction")));
				json_object_object_add(paramJobj, "Filename", JSON_OBJ_COPY(json_object_object_get(PacketCaptureJobj, "FileName")));
				json_object_object_add(paramJobj, "Storage", JSON_OBJ_COPY(json_object_object_get(PacketCaptureJobj, "Storage")));
				json_object_object_add(paramJobj, "Timeout", JSON_OBJ_COPY(json_object_object_get(PacketCaptureJobj, "Timeout")));

		zcfgFeJsonObjFree(PacketCaptureJobj);
	}

    IID_INIT(PacketCaptureUploadIid);
	if(zcfgFeObjJsonGet(RDM_OID_PACKET_CAPTURE_UPLOAD, &PacketCaptureUploadIid, &PacketCaptureUploadJobj) == ZCFG_SUCCESS){
				
				json_object_object_add(paramJobj, "UploadFilename", JSON_OBJ_COPY(json_object_object_get(PacketCaptureUploadJobj, "FileName")));
				json_object_object_add(paramJobj, "UploadStorage", JSON_OBJ_COPY(json_object_object_get(PacketCaptureUploadJobj, "FileSource")));
				json_object_object_add(paramJobj, "UploadProtocol", JSON_OBJ_COPY(json_object_object_get(PacketCaptureUploadJobj, "Protocol")));
				json_object_object_add(paramJobj, "UploadURL", JSON_OBJ_COPY(json_object_object_get(PacketCaptureUploadJobj, "URL")));
				json_object_object_add(paramJobj, "UploadUsername", JSON_OBJ_COPY(json_object_object_get(PacketCaptureUploadJobj, "Username")));
				json_object_object_add(paramJobj, "UploadPassword", JSON_OBJ_COPY(json_object_object_get(PacketCaptureUploadJobj, "Password")));

		zcfgFeJsonObjFree(PacketCaptureUploadJobj);
	}

	if(getLoginUserInfo(Jobj, loginName, NULL, NULL) != ZCFG_SUCCESS){
		if(replyMsg) strcpy(replyMsg, "Internal Error, can't get account information");
		return ZCFG_INTERNAL_ERROR;
	}

    //if(!strcmp(loginName,"supervisor"))
	if(!strcmp(loginName,"root"))
	{
			IID_INIT(accountiid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountiid, &accountobj) == ZCFG_SUCCESS) {
				curusername = json_object_get_string(json_object_object_get(accountobj, "Username"));
                //if(!strcmp(curusername,"supervisor")){
				if(!strcmp(curusername,"root")){
                   curpassword = json_object_get_string(json_object_object_get(accountobj, "Password"));
				   //if(!strcmp(curpassword,"uCkspK6P573B020"))
				   defpassword = json_object_get_string(json_object_object_get(accountobj, "DefaultPassword"));
				   if(!strcmp(curpassword,defpassword))
		           {
						   json_object_object_add(paramJobj, "UserPwdChanged", json_object_new_boolean(false));
		           }
		           else
		           {
						  json_object_object_add(paramJobj, "UserPwdChanged", json_object_new_boolean(true));
		           }
				   zcfgFeJsonObjFree(accountobj);
				   break;
                }
				zcfgFeJsonObjFree(accountobj);
			}
	}
	else
		json_object_object_add(paramJobj, "UserPwdChanged", json_object_new_boolean(true));


	json_object_array_add(Jarray, paramJobj);

	return ret;
}

zcfgRet_t zcfgFeDalPacketCapture(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_PacketCapture_Edit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_PacketCapture_Get(Jobj, Jarray, NULL);
	}
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
#endif


