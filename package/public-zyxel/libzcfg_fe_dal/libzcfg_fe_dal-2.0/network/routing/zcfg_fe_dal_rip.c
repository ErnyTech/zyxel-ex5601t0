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

dal_param_t RIP_param[] =
{
	{"Index", 	 							dalType_int,	0,	0,		NULL,	NULL,					dal_Edit},
	{"Enable",								dalType_boolean,0,	0,		NULL,	NULL,					0},
	{"Version",								dalType_string,	0,	0,		NULL,	"1|2|Both",				0},
	{"RipOpMode",							dalType_string,	0,	0,		NULL,	"Active|Passive",		0},
	{"DisableDefaultGw",					dalType_boolean,0,	0,		NULL,	NULL,					0},
	{"Ripitf_Enable",						dalType_boolean,0,	0,	  	NULL,	NULL,					dalcmd_Forbid},
	{NULL,									0,				0,	0,		NULL,	NULL,					0},
};

zcfgRet_t zcfgFeDalRipGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid,waniid;
	struct json_object *obj = NULL;
	struct json_object *wanobj = NULL;
	struct json_object *pramJobj = NULL;
	const char *Interface; 
#ifdef TAAAB_HGW
    const char *X_ZYXEL_SrvName = NULL, *X_TAAAB_SrvName = NULL;
    char *tmp_string = NULL;
    char newName[128] = {0};
#endif
	int count = 1;
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ROUTING_RIP_INTF_SET, &iid, &obj) == ZCFG_SUCCESS) {
		pramJobj = json_object_new_object();
		IID_INIT(waniid);
		waniid.level = 1;
		if(json_object_object_get(obj, "Interface") != NULL){
			Interface = json_object_get_string(json_object_object_get(obj,"Interface"));
			sscanf(Interface, "IP.Interface.%hhu", &waniid.idx[0]);
			if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &waniid, &wanobj) == ZCFG_SUCCESS){
				if(json_object_object_get(wanobj, "X_ZYXEL_SrvName") != NULL){
					json_object_object_add(pramJobj, "idx", json_object_new_int(iid.idx[0]));
#ifdef TAAAB_HGW
            X_ZYXEL_SrvName = json_object_get_string(json_object_object_get(wanobj, "X_ZYXEL_SrvName"));
			X_TAAAB_SrvName = json_object_get_string(json_object_object_get(wanobj, "X_TT_SrvName"));
			if(strcmp(X_ZYXEL_SrvName,"Default") == 0 || strcmp(X_ZYXEL_SrvName,"WWAN") == 0)
				json_object_object_add(pramJobj, "HGWStyleName", json_object_new_string(X_ZYXEL_SrvName));
			else{
				tmp_string = strrchr(X_TAAAB_SrvName,'_');
				strncpy(newName, X_TAAAB_SrvName, strlen(X_TAAAB_SrvName)-strlen(tmp_string));
				newName[strlen(X_TAAAB_SrvName)-strlen(tmp_string)] = '\0';
				json_object_object_add(pramJobj, "HGWStyleName", json_object_new_string(newName));	
			}
#endif
					json_object_object_add(pramJobj, "Interface",JSON_OBJ_COPY(json_object_object_get(wanobj, "X_ZYXEL_SrvName")));
					json_object_object_add(pramJobj, "Index", json_object_new_int(count));
					json_object_object_add(pramJobj, "Version", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_Version")));
					json_object_object_add(pramJobj, "RipOpMode", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_RipOpMode")));
					json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(obj, "Enable")));
					json_object_object_add(pramJobj, "DisableDefaultGw", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_DisableDefaultGw")));
					json_object_array_add(Jarray, pramJobj);
					count++;
				}
				zcfgFeJsonObjFree(wanobj);
			}
		}
		zcfgFeJsonObjFree(obj);
	}
	return ret;
}

void zcfgFeDalShowRip(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-7s %-20s %-12s %-12s %-12s %-20s\n",
		    "Index", "Interface", "Version", "RipOpMode", "Enable", "DisableDefaultGw");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj != NULL){
			printf("%-7s %-20s %-12s %-12s %-12d %-20d\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Interface")),
			json_object_get_string(json_object_object_get(obj, "Version")),
			json_object_get_string(json_object_object_get(obj, "RipOpMode")),
			json_object_get_boolean(json_object_object_get(obj, "Enable")),
			json_object_get_boolean(json_object_object_get(obj, "DisableDefaultGw")));
		}
	}
}

zcfgRet_t zcfgFeDalRipEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ripItfObj = NULL;
	struct json_object *ripObj = NULL;
	struct json_object *reqObject = NULL;
	struct json_object *tmpObj = NULL;
	objIndex_t ripItfIid = {0};
	objIndex_t ripIid = {0};
	bool Enable = false;
	bool Ripitf_Enable = false;
	const char *version;
	const char *ripOpMode;
	bool disableDefaultGw = false;
	int index = 0;
	const char *idx = 0;
	int len = 0;
	
	if(json_object_get_type(Jobj) == json_type_object){// set by dalcmd
		tmpObj = json_object_new_array();
		zcfgFeDalRipGet(NULL,tmpObj, NULL);
		IID_INIT(ripItfIid);
		index = json_object_get_int(json_object_object_get(Jobj,"Index"));
		
		len = json_object_array_length(tmpObj);
		for(int i=0;i<len;i++){
			reqObject = json_object_array_get_idx(tmpObj, i);
			if(index == atoi(json_object_get_string(json_object_object_get(reqObject, "Index")))){
				idx = json_object_get_string(json_object_object_get(reqObject, "idx"));
			}				
		}

		ripItfIid.level = 1;
		ripItfIid.idx[0] = atoi(idx);

		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ROUTING_RIP_INTF_SET, &ripItfIid, &ripItfObj) != ZCFG_SUCCESS) {
			if(replyMsg)
				sprintf(replyMsg, "Can't find RIP rule of index %d\n", index);
			return ZCFG_NOT_FOUND;
		}

		if((json_object_object_get(Jobj, "Enable") == NULL) && (json_object_object_get(Jobj, "Version") == NULL) && (json_object_object_get(Jobj, "RipOpMode") == NULL) && (json_object_object_get(Jobj, "DisableDefaultGw") == NULL)){
			strcpy(replyMsg, "Invalid parameters! Please enter correct parameters.");
			return ZCFG_REQUEST_REJECT;
		}

		if(json_object_object_get(Jobj, "Enable") != NULL){
			Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
			json_object_object_add(ripItfObj, "Enable", json_object_new_boolean(Enable));
		}

		if(json_object_object_get(Jobj, "Version") != NULL){
			version = json_object_get_string(json_object_object_get(Jobj, "Version"));
			if(!strcmp(version,"1"))
				json_object_object_add(ripItfObj, "X_ZYXEL_Version", json_object_new_string("RIPv1"));
			if(!strcmp(version,"2"))
				json_object_object_add(ripItfObj, "X_ZYXEL_Version", json_object_new_string("RIPv2"));
			if(!strcmp(version,"Both"))
				json_object_object_add(ripItfObj, "X_ZYXEL_Version", json_object_new_string("RIPv1v2"));	
		}

		if(json_object_object_get(Jobj, "RipOpMode") != NULL){
			ripOpMode = json_object_get_string(json_object_object_get(Jobj, "RipOpMode"));
			json_object_object_add(ripItfObj, "X_ZYXEL_RipOpMode", json_object_new_string(ripOpMode));
		}

		if(json_object_object_get(Jobj, "DisableDefaultGw") != NULL){
			disableDefaultGw = json_object_get_boolean(json_object_object_get(Jobj, "DisableDefaultGw"));
			json_object_object_add(ripItfObj, "X_ZYXEL_DisableDefaultGw", json_object_new_boolean(disableDefaultGw));
		}

		if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_ROUTING_RIP_INTF_SET, &ripItfIid, ripItfObj, NULL)) != ZCFG_SUCCESS){
			if(replyMsg)
				sprintf(replyMsg, "Set RIP rule fail!");
			return ret;
		}
		zcfgFeJsonObjFree(ripItfObj);
		return ret;
	}
	else{	//set by GUI
		len = json_object_array_length(Jobj);
		for(int reqObjIdx = 0;reqObjIdx<len;reqObjIdx++){
			reqObject = json_object_array_get_idx(Jobj, reqObjIdx);
			index = json_object_get_int(json_object_object_get(reqObject, "Index"));
			IID_INIT(ripItfIid);
			ripItfIid.idx[0] = index;
			ripItfIid.level = 1;
			ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ROUTING_RIP_INTF_SET, &ripItfIid, &ripItfObj);
			if(ret == ZCFG_SUCCESS){
				if(json_object_object_get(reqObject, "Enable") != NULL){
					Enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
					json_object_object_add(ripItfObj, "Enable", json_object_new_boolean(Enable));
				}

				if(json_object_object_get(reqObject, "Version") != NULL){
					version = json_object_get_string(json_object_object_get(reqObject, "Version"));
					json_object_object_add(ripItfObj, "X_ZYXEL_Version", json_object_new_string(version));
				}

				if(json_object_object_get(reqObject, "RipOpMode") != NULL){
					ripOpMode = json_object_get_string(json_object_object_get(reqObject, "RipOpMode"));
					json_object_object_add(ripItfObj, "X_ZYXEL_RipOpMode", json_object_new_string(ripOpMode));
				}

				if(json_object_object_get(reqObject, "DisableDefaultGw") != NULL){
					disableDefaultGw = json_object_get_boolean(json_object_object_get(reqObject, "DisableDefaultGw"));
					json_object_object_add(ripItfObj, "X_ZYXEL_DisableDefaultGw", json_object_new_boolean(disableDefaultGw));
				}

				//enable RIP rule
				if(json_object_object_get(reqObject, "Ripitf_Enable") != NULL){
					IID_INIT(ripIid);
					ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ROUTING_RIP, &ripIid, &ripObj);
					if(ret == ZCFG_SUCCESS){
						Ripitf_Enable = json_object_get_boolean(json_object_object_get(reqObject, "Ripitf_Enable"));
						json_object_object_add(ripObj, "Enable", json_object_new_boolean(Ripitf_Enable));
						if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_RIP, &ripIid, ripObj, NULL)) != ZCFG_SUCCESS){
							printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_ROUTING_RIP);
							return ZCFG_INTERNAL_ERROR;
						}
						zcfgFeJsonObjFree(ripObj);
					}
				}
				zcfgFeObjJsonBlockedSet(RDM_OID_ROUTING_RIP_INTF_SET, &ripItfIid, ripItfObj, NULL);
				zcfgFeJsonObjFree(ripItfObj);
			}
			else{
				printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_ROUTING_RIP_INTF_SET);
				return ZCFG_INTERNAL_ERROR;
			}
			#if 0 /* <JoannaSu>, <20191211>, <Avoid the invalid free as obj gets from json_object_array_get_idx> */
			zcfgFeJsonObjFree(reqObject);
			#endif
		}
		return ret;
	}
}

zcfgRet_t zcfgFeDalRip(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) 
		ret = zcfgFeDalRipEdit(Jobj, replyMsg);

	if(!strcmp(method, "GET")) 
		ret = zcfgFeDalRipGet(Jobj, Jarray, NULL);

	return ret;
}

