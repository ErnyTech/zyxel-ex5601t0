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

dal_param_t CELLWAN_MGMT_param[]={		//RDM_OID_REMO_SRV_IP_PASS_THRU
	{"Name", 					dalType_string,	0,	0,		NULL, NULL, 				dalcmd_Forbid},
	{"WAN_Interface", 			dalType_string,	0,	0,		NULL, "Any_WAN|Multi_WAN", 				0},
	{"BoundInterfaceList", 		dalType_MultiRtWanWWan,		0,	0,		NULL, NULL, 				0},
	{"Mode", 					dalType_string,	0,	0,		NULL, "IPPT_ONLY|IPPT_WAN|IPPT_TR_DM|None", 	0},
	{"Port", 	 				dalType_int,	1,	65535,	NULL, NULL, 				0},
	{"Index", 	 				dalType_int,	0,	0,		NULL, NULL, 				0},
	{"Protocol", 				dalType_string,	0,	0,		NULL, NULL, 				dalcmd_Forbid},
	{"RestartDeamon", 		 	dalType_boolean,0,	0,		NULL, NULL, 				dalcmd_Forbid},
#ifdef	CAAAB_CUSTOMIZATION
	{"LifeTime", 	 			dalType_int,	0,	0,		NULL, NULL, 				0},		//if(CustomerName.search("CBBT") != -1)
#endif
	{NULL,						0,				0,	0,		NULL, NULL, 				0},
};

zcfgRet_t zcfgFeDalCellWanMgmtGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	char loginLevel[16] = {0};
	struct json_object *remoMgmtIpPassthruJobj = NULL;
	struct json_object *pramJobj = NULL;
	struct json_object *mgmtSrvObj = NULL;
	struct json_object *zyLogCfgGpObj = NULL;
	struct json_object *conflictPortJarray = NULL;
	struct json_object *remoSrvIpPassthruObj = NULL;
	struct json_object *remoSrvIpPassthruJarray = NULL;

	conflictPortJarray = json_object_new_array();
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MGMT_SRV, &iid, &mgmtSrvObj)) == ZCFG_SUCCESS)
		json_object_array_add(conflictPortJarray, JSON_OBJ_COPY(json_object_object_get(mgmtSrvObj, "X_ZYXEL_ConnectionRequestPort")));
	zcfgFeJsonObjFree(mgmtSrvObj);

	remoSrvIpPassthruJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_REMO_SRV_IP_PASS_THRU, &iid, &remoSrvIpPassthruObj) == ZCFG_SUCCESS) {
		pramJobj = json_object_new_object();
		json_object_object_add(pramJobj, "Index", json_object_new_int(iid.idx[0]));
		json_object_object_add(pramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(remoSrvIpPassthruObj, "Name")));
		json_object_object_add(pramJobj, "Port", JSON_OBJ_COPY(json_object_object_get(remoSrvIpPassthruObj, "Port")));
		json_object_object_add(pramJobj, "Mode", JSON_OBJ_COPY(json_object_object_get(remoSrvIpPassthruObj, "Mode")));
		json_object_object_add(pramJobj, "BoundInterfaceList", JSON_OBJ_COPY(json_object_object_get(remoSrvIpPassthruObj, "BoundInterfaceList")));
		json_object_array_add(remoSrvIpPassthruJarray, pramJobj);
		zcfgFeJsonObjFree(remoSrvIpPassthruObj);
	}
	
	remoMgmtIpPassthruJobj = json_object_new_object();
	json_object_array_add(Jarray, remoMgmtIpPassthruJobj);
	json_object_object_add(remoMgmtIpPassthruJobj, "ConflictPort", conflictPortJarray);
	json_object_object_add(remoMgmtIpPassthruJobj, "RemoSrvIpPassthru", remoSrvIpPassthruJarray);


	if(getLoginUserInfo(Jobj, NULL, loginLevel, NULL) != ZCFG_SUCCESS){
		if(replyMsg) strcpy(replyMsg, "Internal Error, can't get account information");
		dbg_printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}
	if(json_object_object_get(Jobj, "loginLevel") != NULL){
		IID_INIT(iid);
		printf("loginLevel: %s\n", loginLevel);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP, &iid, &zyLogCfgGpObj) == ZCFG_SUCCESS) {
			if(!strcmp( json_object_get_string(json_object_object_get(zyLogCfgGpObj, "Level")),loginLevel) ){
				json_object_object_add(remoMgmtIpPassthruJobj, "GpPrivilege", JSON_OBJ_COPY(json_object_object_get(zyLogCfgGpObj, "GP_Privilege")));
				zcfgFeJsonObjFree(zyLogCfgGpObj);
				break;
			}
			zcfgFeJsonObjFree(zyLogCfgGpObj);
		}
	}

	return ret;
}

void zcfgFeDalShowCellWanMgmt(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj=NULL;
	struct json_object *remoSrvIpPassthruJarray=NULL;

	int conflictPort, Index, Port;
	const char *Name, *Mode, *BoundIntflist = NULL;
	char mode[32] = {0};
	char port[32] = {0};

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	
	obj = json_object_array_get_idx(Jarray, 0);
	remoSrvIpPassthruJarray = json_object_object_get(obj, "RemoSrvIpPassthru");
	obj = json_object_array_get_idx(remoSrvIpPassthruJarray, 0);
	
	BoundIntflist = json_object_get_string(json_object_object_get(obj, "BoundInterfaceList"));
	if(!strcmp(BoundIntflist,""))
		printf("WAN Interface used for services: Any WAN \n");
	else
		printf("WAN Interface used for services: Multi WAN, %s \n",BoundIntflist);

	printf("%-7s %-10s %-12s %-10s\n",
		    "Index", "Name", "Mode", "Port");

	len = json_object_array_length(remoSrvIpPassthruJarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(remoSrvIpPassthruJarray, i);
		if(obj != NULL){
			Index = json_object_get_int(json_object_object_get(obj, "Index"));
			Port = json_object_get_int(json_object_object_get(obj, "Port"));
			Name = json_object_get_string(json_object_object_get(obj, "Name"));
			Mode = json_object_get_string(json_object_object_get(obj, "Mode"));

			if(strcmp(Mode, ""))
				strcpy(mode, Mode);
			else
				strcpy(mode, "None");

			if(strcmp(Name, "PING"))
				sprintf(port, "%d", Port);
			else
				strcpy(port, "N/A");

			printf("%-7u %-10s %-12s %-10s\n",Index,Name,mode,port);
		}

	}
}


zcfgRet_t zcfgFeDalCellWanMgmtEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *mgmtSrvPassthruObj = NULL;
	struct json_object *reqObject = NULL;
	struct json_object *obj = NULL;
	objIndex_t mgmtSrvPassthruIid = {0};
	objIndex_t Iid = {0};

	const char *boundinterfacelist = NULL;
	const char *mode = NULL;
	const char *protocol = NULL;
	const char *wnaIntf = NULL;
	int port = 0;
	bool restartdeamon = false;
	int index = 0;
	int origport = 0;
	int otherorigport = 0;
	int httpport = 0;
	int httpsport = 0;
	int lifetime = 0;
	int len = 0;
	len = json_object_array_length(Jobj);
	int tmp_idx[len];
	for(int i=0;i<len;i++){
		 if(i == len-2)		
		 	tmp_idx[i] = 1;
		 else if(i == len-1)
		 	tmp_idx[i] = 0;
		 else
		 	tmp_idx[i] = i+2;
	}		

	if(json_object_get_type(Jobj) == json_type_object){// set by dalcmd
		const char *Name = NULL;
		if(json_object_object_get(Jobj, "WAN_Interface")){
			if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "WAN_Interface")),"Multi_WAN")){
				wnaIntf = json_object_get_string(json_object_object_get(Jobj, "BoundInterfaceList"));
			}else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "WAN_Interface")),"Any_WAN")){
				wnaIntf = "";

			}
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_REMO_SRV_IP_PASS_THRU, &Iid, &obj) == ZCFG_SUCCESS){
				json_object_object_add(obj, "BoundInterfaceList", json_object_new_string(wnaIntf));
				zcfgFeObjJsonBlockedSet(RDM_OID_REMO_SRV_IP_PASS_THRU, &Iid, obj, NULL);
				zcfgFeJsonObjFree(obj);
			}
		}
		if(!json_object_object_get(Jobj, "WAN_Interface") && !json_object_object_get(Jobj, "Index")){
			strcpy(replyMsg, "Please enter WAN_Interface or Intex.");
			return ZCFG_INVALID_PARAM_VALUE;
		}
		if((index = json_object_get_int(json_object_object_get(Jobj, "Index"))) != 0){
			mode = json_object_get_string(json_object_object_get(Jobj, "Mode"));
			port = json_object_get_int(json_object_object_get(Jobj, "Port"));
			if(!mode && !port)//nothing changed
				return ZCFG_SUCCESS;
			
			
			IID_INIT(mgmtSrvPassthruIid);
			mgmtSrvPassthruIid.idx[0] = index;
			mgmtSrvPassthruIid.level = 1;
			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV_IP_PASS_THRU, &mgmtSrvPassthruIid, &mgmtSrvPassthruObj) != ZCFG_SUCCESS){
				if(replyMsg)
					sprintf(replyMsg, "Can't find remote Mgmt rule of index %d\n", index);
				return ZCFG_NOT_FOUND;
			}

			json_object_object_add(mgmtSrvPassthruObj, "RestartDeamon", json_object_new_boolean(false));
			origport = json_object_get_int(json_object_object_get(mgmtSrvPassthruObj, "Port"));
			if((port != 0) && (origport != port)){
				json_object_object_add(mgmtSrvPassthruObj, "Port", json_object_new_int(port));
				Name = json_object_get_string(json_object_object_get(mgmtSrvPassthruObj, "Name"));
				if(!strcmp(Name, "HTTP") || !strcmp(Name, "HTTPS")){
					json_object_object_add(mgmtSrvPassthruObj, "RestartDeamon", json_object_new_boolean(true));
				}
			}
			if(mode != NULL){
				if(!strcmp(mode, "None"))
					json_object_object_add(mgmtSrvPassthruObj, "Mode", json_object_new_string(""));
				else
					json_object_object_add(mgmtSrvPassthruObj, "Mode", json_object_new_string(mode));
			}

			if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_REMO_SRV_IP_PASS_THRU, &mgmtSrvPassthruIid, mgmtSrvPassthruObj, NULL)) != ZCFG_SUCCESS){
				if(replyMsg)
					sprintf(replyMsg, "Set remote Mgmt rule fail!");
                zcfgFeJsonObjFree(mgmtSrvPassthruObj);
				return ret;
			}
			zcfgFeJsonObjFree(mgmtSrvPassthruObj);				
			return ret;
		}
		return ZCFG_SUCCESS;
	}

	reqObject = json_object_array_get_idx(Jobj, len-1);
	protocol = json_object_get_string(json_object_object_get(reqObject, "Protocol"));
	origport = json_object_get_int(json_object_object_get(reqObject, "origport"));
	otherorigport = json_object_get_int(json_object_object_get(reqObject, "otherorigport"));
	httpport = json_object_get_int(json_object_object_get(reqObject, "httpport"));
	httpsport = json_object_get_int(json_object_object_get(reqObject, "httpsport"));

	if(!strcmp(protocol,"http")){
		if((origport-httpport == 0) && (otherorigport-httpsport != 0)){
			tmp_idx[len-2] = 0;
			tmp_idx[len-1] = 1;
		}
	}
	else if(!strcmp(protocol,"https")){
		tmp_idx[len-2] = 0;
		tmp_idx[len-1] = 1;
		if((origport-httpsport==0) && (otherorigport-httpport != 0)){
			tmp_idx[len-2] = 1;
			tmp_idx[len-1] = 0;
		}
		}
			
	for(int reqObjIdx = 0;reqObjIdx<len;reqObjIdx++){
		reqObject = json_object_array_get_idx(Jobj, tmp_idx[reqObjIdx]);

			IID_INIT(mgmtSrvPassthruIid);
			index = json_object_get_int(json_object_object_get(reqObject, "Index"));
			protocol = json_object_get_string(json_object_object_get(reqObject, "Protocol"));
			mgmtSrvPassthruIid.idx[0] = index;
			mgmtSrvPassthruIid.level = 1;
			ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV_IP_PASS_THRU, &mgmtSrvPassthruIid, &mgmtSrvPassthruObj);
			if(ret == ZCFG_SUCCESS){
				boundinterfacelist = json_object_get_string(json_object_object_get(reqObject, "BoundInterfaceList"));
				mode = json_object_get_string(json_object_object_get(reqObject, "Mode"));
				port = json_object_get_int(json_object_object_get(reqObject, "Port"));
				restartdeamon = json_object_get_boolean(json_object_object_get(reqObject, "RestartDeamon"));
				if(reqObjIdx-(len-1) == 0){
					restartdeamon = true;	//restart
				}
#ifdef	CAAAB_CUSTOMIZATION
				lifetime = json_object_get_int(json_object_object_get(reqObject, "LifeTime"));
				json_object_object_add(mgmtSrvPassthruObj, "LifeTime", json_object_new_int(lifetime));
#endif

				json_object_object_add(mgmtSrvPassthruObj, "BoundInterfaceList", json_object_new_string(boundinterfacelist));
				json_object_object_add(mgmtSrvPassthruObj, "Mode", json_object_new_string(mode));
				json_object_object_add(mgmtSrvPassthruObj, "Port", json_object_new_int(port));
				json_object_object_add(mgmtSrvPassthruObj, "RestartDeamon", json_object_new_int(restartdeamon));
				

				zcfgFeObjJsonBlockedSet(RDM_OID_REMO_SRV_IP_PASS_THRU, &mgmtSrvPassthruIid, mgmtSrvPassthruObj, NULL);
                zcfgFeJsonObjFree(mgmtSrvPassthruObj);
			}
	}

	zcfgFeJsonObjFree(mgmtSrvPassthruObj);

	return ret;
}


zcfgRet_t zcfgFeDalCellWanMgmt(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalCellWanMgmtEdit(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalCellWanMgmtGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

