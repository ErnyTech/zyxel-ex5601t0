#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t SP_MGMT_SRV_param[]={		//RDM_OID_SP_REMO_SRV
	{"Name", 					dalType_string,	0,	0,		NULL,	NULL,	dalcmd_Forbid},
	{"Enable",					dalType_boolean,0,	0,		NULL,	NULL,	0},
	{"Port", 	 				dalType_int,	1,	65535,	NULL,	NULL,	0},
	{"Index", 	 				dalType_int,	0,	0,		NULL,	NULL,	dal_Edit},
	{"RestartDeamon", 		 	dalType_boolean,0,	0,		NULL,	NULL,	dalcmd_Forbid},
	{NULL,						0,				0,	0,		NULL,	NULL,	0},
};

zcfgRet_t zcfgFeDal_SPmgmtSrv_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_SP_REMO_SRV, &iid, &obj) == ZCFG_SUCCESS) {
		pramJobj = json_object_new_object();
		json_object_object_add(pramJobj, "Index", json_object_new_int(iid.idx[0]));
		json_object_object_add(pramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(obj, "Name")));
		json_object_object_add(pramJobj, "Port", JSON_OBJ_COPY(json_object_object_get(obj, "Port")));
		json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(obj, "Enable")));
		json_object_array_add(Jarray, pramJobj);
		zcfgFeJsonObjFree(obj);
	}
	
	return ret;
}


void zcfgFeDalShowSPMgmtSrv(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;
	int Index, Port;
	const char *Name;
	char port[32] = {0};
	bool Enable;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-7s %-10s %-12s %-10s\n",
		    "Index", "Name", "Port", "Enable");
	
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj != NULL){
			Index = json_object_get_int(json_object_object_get(obj, "Index"));
			Name = json_object_get_string(json_object_object_get(obj, "Name"));
			Port = json_object_get_int(json_object_object_get(obj, "Port"));
			Enable = json_object_get_boolean(json_object_object_get(obj, "Enable"));

			if(strcmp(Name, "PING"))
				sprintf(port, "%d", Port);
			else
				strcpy(port, "N/A");

			printf("%-7u %-10s %-12s %-10d\n",Index,Name,port,Enable);
		}

	}
	
}

zcfgRet_t zcfgFeDal_SPmgmtSrv_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *SPmgmtSrvObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t SPmgmtSrvIid = {0};

	const char *ServiceName = NULL;
	int port = 0;
	bool RestartHttpDeamon = false, EnableService =false, EnableHttpService = false;
	int index = 0, HttpIndex = 1;
	int NewPort = 0;
	int OriginalPort = 0;
	int len = 0;

	if(json_object_get_type(Jobj) == json_type_object){// set by dalcmd
		IID_INIT(SPmgmtSrvIid);
		index = json_object_get_int(json_object_object_get(Jobj,"Index"));
		SPmgmtSrvIid.level = 1;
		SPmgmtSrvIid.idx[0] = index;

		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SP_REMO_SRV, &SPmgmtSrvIid, &SPmgmtSrvObj) != ZCFG_SUCCESS) {
			if(replyMsg)
				sprintf(replyMsg, "Can't find remote Mgmt rule of index %d\n", index);
			return ZCFG_NOT_FOUND;
		}

		OriginalPort = json_object_get_int(json_object_object_get(SPmgmtSrvObj, "Port"));
		ServiceName = json_object_get_string(json_object_object_get(SPmgmtSrvObj,"Name"));
		if(json_object_object_get(Jobj, "Port") != NULL){
			port = json_object_get_int(json_object_object_get(Jobj, "Port"));
			if(!strcmp(ServiceName, "HTTP")){
				if(OriginalPort != port)
					json_object_object_add(SPmgmtSrvObj, "RestartDeamon", json_object_new_boolean(true));
				json_object_object_add(SPmgmtSrvObj, "Port", json_object_new_int(port));
			}
			else{
				strcpy(replyMsg, "Only HTTP can modify port.");
				zcfgFeJsonObjFree(SPmgmtSrvObj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}

		if(json_object_object_get(Jobj, "Enable") != NULL){
			EnableService = json_object_get_int(json_object_object_get(Jobj, "Enable"));
			json_object_object_add(SPmgmtSrvObj, "Enable", json_object_new_boolean(EnableService));
		}

		if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_SP_REMO_SRV, &SPmgmtSrvIid, SPmgmtSrvObj, NULL)) != ZCFG_SUCCESS){
			if(replyMsg)
				sprintf(replyMsg, "Set remote Mgmt rule fail!");
			zcfgFeJsonObjFree(SPmgmtSrvObj);
			return ret;
		}

		zcfgFeJsonObjFree(SPmgmtSrvObj);
		return ret;
	}
	len = json_object_array_length(Jobj);
	for(int reqObjIdx = 0;reqObjIdx<len;reqObjIdx++){
		reqObject = json_object_array_get_idx(Jobj, reqObjIdx);

			IID_INIT(SPmgmtSrvIid);
			index = json_object_get_int(json_object_object_get(reqObject, "Index"));
			SPmgmtSrvIid.idx[0] = index;
			SPmgmtSrvIid.level = 1;
			ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SP_REMO_SRV, &SPmgmtSrvIid, &SPmgmtSrvObj);
			if(ret == ZCFG_SUCCESS){
				EnableService = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
				port = json_object_get_int(json_object_object_get(reqObject, "Port"));
				ServiceName = json_object_get_string(json_object_object_get(SPmgmtSrvObj, "Name"));
				OriginalPort = json_object_get_int(json_object_object_get(SPmgmtSrvObj, "Port"));
				/*if port of http or https have be modified, let it be PUT at last*/
				if( !strcmp(ServiceName, "HTTP") ){
					if(port != OriginalPort){
						EnableHttpService = EnableService;
						RestartHttpDeamon = true;
						HttpIndex = index;
						NewPort = port;
						zcfgFeJsonObjFree(SPmgmtSrvObj);
						continue;
					}else{
						json_object_object_add(SPmgmtSrvObj, "Port", json_object_new_int(port));
					}
				}
				json_object_object_add(SPmgmtSrvObj, "Enable", json_object_new_int(EnableService));
				
				zcfgFeObjJsonBlockedSet(RDM_OID_SP_REMO_SRV, &SPmgmtSrvIid, SPmgmtSrvObj, NULL);
				zcfgFeJsonObjFree(SPmgmtSrvObj);
			}

	}
	
	if(RestartHttpDeamon){		
		IID_INIT(SPmgmtSrvIid);
		SPmgmtSrvIid.idx[0] = HttpIndex;
		SPmgmtSrvIid.level = 1;
		ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SP_REMO_SRV, &SPmgmtSrvIid, &SPmgmtSrvObj);
		if(ret == ZCFG_SUCCESS){
			json_object_object_add(SPmgmtSrvObj, "Enable", json_object_new_int(EnableHttpService));
			json_object_object_add(SPmgmtSrvObj, "Port", json_object_new_int(NewPort));
			json_object_object_add(SPmgmtSrvObj, "RestartDeamon", json_object_new_int(RestartHttpDeamon));
			zcfgFeObjJsonBlockedSet(RDM_OID_SP_REMO_SRV, &SPmgmtSrvIid, SPmgmtSrvObj, NULL);
			zcfgFeJsonObjFree(SPmgmtSrvObj);
		}
	}

	return ret;
}


zcfgRet_t zcfgFeDalSPMgmtSrv(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDal_SPmgmtSrv_Edit(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_SPmgmtSrv_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

