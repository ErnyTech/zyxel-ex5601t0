#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t ETH_CTL_param[] = 
{
#ifdef ABQA_CUSTOMIZATION
    {"intf",		dalType_string,       0, 0, NULL,		"LAN1",							dal_Edit},
	{"media-type",	dalType_string,       0, 0, NULL,		"Auto|1000FD|1000HD|100FD|100HD|10FD|10HD", 	0},
#else
    {"intf",		dalType_string,       0, 0, NULL,		"LAN1|LAN2|LAN3|LAN4",			dal_Edit},
	{"media-type",	dalType_string,       0, 0, NULL,		"Auto|100FD|100HD|10FD|10HD", 	0},
#endif
	{"enable",		dalType_boolean,   	  0, 0, NULL, 	     NULL,					        0},
	{NULL,						  0,	  0, 0, NULL, 	     NULL,					        0}
};


void zcfgFeDalShowEthCtl(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	struct json_object *obj1 = NULL;
	struct json_object *ethctlListObj = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("setting for eth ports\n");
	printf("%-15s %-15s %-15s %-15s \n","Interface","Duplex","Speed","Enable");
	obj = json_object_array_get_idx(Jarray, 0);
	ethctlListObj = json_object_object_get(obj, "EthctlList");
	len = json_object_array_length(ethctlListObj);
	for(i=0;i<len;i++){
		obj1 = json_object_array_get_idx(ethctlListObj, i);
		printf("%-15s %-15s %-15s %-15d \n",
			json_object_get_string(json_object_object_get(obj1, "intf")),
			json_object_get_string(json_object_object_get(obj1, "duplexSetting")),
			json_object_get_string(json_object_object_get(obj1, "speedSetting")),
			json_object_get_boolean(json_object_object_get(obj1, "enable")));
	}
	printf("status for eth ports\n");
#ifdef ZYXEL_WEB_GUI_ETH_UNI
	printf("%-15s %-25s %-15s %-15s %-15s %-15s \n","Interface", "Administrative Status", "Link State", "Duplex","Speed","Enable");
#else
	printf("%-15s %-15s %-15s %-15s \n","Interface","Duplex","Speed","Enable");
#endif
	obj = json_object_array_get_idx(Jarray, 0);
	ethctlListObj = json_object_object_get(obj, "EthctlList");
	len = json_object_array_length(ethctlListObj);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(ethctlListObj, i);
#ifdef ZYXEL_WEB_GUI_ETH_UNI
			printf("%-15s %-25s %-15s %-15s %-15s %-15d \n",
				json_object_get_string(json_object_object_get(obj1, "intf")),
				json_object_get_string(json_object_object_get(obj1, "intfStatus")),
				json_object_get_string(json_object_object_get(obj1, "linkStatus")),
				json_object_get_string(json_object_object_get(obj1, "duplexStatus")),
				json_object_get_string(json_object_object_get(obj1, "speedStatus")),
				json_object_get_boolean(json_object_object_get(obj1, "enable")));
#else
			if (strcmp(json_object_get_string(json_object_object_get(obj, "duplexStatus")), "Down") == 0||json_object_get_boolean(json_object_object_get(obj, "enable"))==0)
				printf("%-15s %-15s\n",
					json_object_get_string(json_object_object_get(obj, "intf")),
					"Down");
			else
				printf("%-15s %-15s %-15s %-15d \n",
					json_object_get_string(json_object_object_get(obj, "intf")),
					json_object_get_string(json_object_object_get(obj, "duplexStatus")),
					json_object_get_string(json_object_object_get(obj, "speedStatus")),
					json_object_get_boolean(json_object_object_get(obj, "enable")));
#endif
	}
}
zcfgRet_t zcfgFeDalEthCtlGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ethObj = NULL;
	struct json_object *paramJobj = NULL;
	struct json_object *ethctlListObj = NULL, *tmpListObj = NULL;
	const char *name = NULL;
	objIndex_t ethIid = {0};
	ethctlListObj = json_object_new_object();
	tmpListObj = json_object_new_array();
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &ethIid, &ethObj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		name = json_object_get_string(json_object_object_get(ethObj, "X_ZYXEL_LanPort"));
		if(strcmp(name,"ETHWAN") != 0){
			json_object_object_add(paramJobj, "intf", JSON_OBJ_COPY(json_object_object_get(ethObj, "X_ZYXEL_LanPort")));
			json_object_object_add(paramJobj, "duplexSetting", JSON_OBJ_COPY(json_object_object_get(ethObj, "X_ZYXEL_DuplexMode")));
			json_object_object_add(paramJobj, "speedSetting", JSON_OBJ_COPY(json_object_object_get(ethObj, "X_ZYXEL_MaxBitRate")));
			json_object_object_add(paramJobj, "duplexStatus", JSON_OBJ_COPY(json_object_object_get(ethObj, "DuplexMode")));
			json_object_object_add(paramJobj, "speedStatus", JSON_OBJ_COPY(json_object_object_get(ethObj, "MaxBitRate")));
			json_object_object_add(paramJobj, "enable", JSON_OBJ_COPY(json_object_object_get(ethObj, "IntfEnable")));
#ifdef ZYXEL_WEB_GUI_ETH_UNI	
			json_object_object_add(paramJobj, "linkStatus", JSON_OBJ_COPY(json_object_object_get(ethObj, "Status")));
			json_object_object_add(paramJobj, "intfStatus", JSON_OBJ_COPY(json_object_object_get(ethObj, "IntfStatus")));
#endif
			json_object_array_add(tmpListObj, paramJobj);
		}
		else{
			zcfgFeJsonObjFree(paramJobj);
		}
		zcfgFeJsonObjFree(ethObj);
	}
	json_object_object_add(ethctlListObj, "EthctlList", tmpListObj);
	json_object_array_add(Jarray, ethctlListObj);
	return ret;
}
zcfgRet_t setEthctlIntf(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ethObj = NULL;
	objIndex_t ethIid = {0};
	const char *targetName = NULL;
	const char *name = NULL;
	const char * media_type = NULL;
	int index = 0;
	bool newEnable = false;
	bool curEnable = false;
	bool found = false;
	targetName = json_object_get_string(json_object_object_get(Jobj, "intf"));
	sscanf(targetName, "LAN%d", &index);
	
	IID_INIT(ethIid);
	ethIid.level = 1;
	ethIid.idx[0] = index;
	
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_IFACE, &ethIid, &ethObj) == ZCFG_SUCCESS) {
		name = json_object_get_string(json_object_object_get(ethObj, "X_ZYXEL_LanPort"));
		if((targetName != NULL) && !strcmp(targetName,name)){
			curEnable  =  json_object_get_boolean(json_object_object_get(ethObj, "IntfEnable"));
			found = true;
		}
	}

	if(found) {
		if((json_object_object_get(Jobj, "media-type") == NULL) && (json_object_object_get(Jobj, "enable") == NULL) ){
#ifdef ABQA_CUSTOMIZATION
			strcpy(replyMsg, "Invalid parameters! Please enter --intf  <interface> --media-type <Auto|1000FD|1000HD|100FD|100HD|10FD|10HD>  or --enable <1|0>");
#else
			strcpy(replyMsg, "Invalid parameters! Please enter --intf  <interface> --media-type <Auto|100FD|100HD|10FD|10HD>  or --enable <1|0>");
#endif
			zcfgFeJsonObjFree(ethObj);
			return ZCFG_REQUEST_REJECT;
		}else{
			//Enable/disable
			if(json_object_object_get(Jobj, "enable") != NULL){
				newEnable = json_object_get_int(json_object_object_get(Jobj, "enable"));
				json_object_object_add(ethObj, "IntfEnable", json_object_new_boolean(newEnable));
			}
		
			//If disable, ignore
			if(json_object_object_get(Jobj, "media-type") != NULL){
				if(!curEnable && !newEnable){
					strcpy(replyMsg, "The port is disable.");
					// return ZCFG_REQUEST_REJECT;
				}
				//media-type
				media_type = json_object_get_string(json_object_object_get(Jobj, "media-type"));
				 if(!strcmp(media_type,"Auto")){
					json_object_object_add(ethObj, "X_ZYXEL_DuplexMode", json_object_new_string(media_type));
					json_object_object_add(ethObj, "X_ZYXEL_MaxBitRate", json_object_new_int(-1));
				}else if(!strcmp(media_type,"100FD")){
					json_object_object_add(ethObj, "X_ZYXEL_DuplexMode", json_object_new_string("Full"));
					json_object_object_add(ethObj, "X_ZYXEL_MaxBitRate", json_object_new_int(100));
				}else if(!strcmp(media_type,"100HD")){
					json_object_object_add(ethObj, "X_ZYXEL_DuplexMode", json_object_new_string("Half"));
					json_object_object_add(ethObj, "X_ZYXEL_MaxBitRate", json_object_new_int(100));
				}else if(!strcmp(media_type,"10FD")){
					json_object_object_add(ethObj, "X_ZYXEL_DuplexMode", json_object_new_string("Full"));
					json_object_object_add(ethObj, "X_ZYXEL_MaxBitRate", json_object_new_int(10));
				}else if(!strcmp(media_type,"10HD")){
					json_object_object_add(ethObj, "X_ZYXEL_DuplexMode", json_object_new_string("Half"));
					json_object_object_add(ethObj, "X_ZYXEL_MaxBitRate", json_object_new_int(10));
				}
#ifdef ABQA_CUSTOMIZATION
				else if(!strcmp(media_type,"1000HD")){
					json_object_object_add(ethObj, "X_ZYXEL_DuplexMode", json_object_new_string("Half"));
					json_object_object_add(ethObj, "X_ZYXEL_MaxBitRate", json_object_new_int(1000));
				}
				else if(!strcmp(media_type,"1000FD")){
					json_object_object_add(ethObj, "X_ZYXEL_DuplexMode", json_object_new_string("Full"));
					json_object_object_add(ethObj, "X_ZYXEL_MaxBitRate", json_object_new_int(1000));
				}
#endif
				else{
#ifdef ABQA_CUSTOMIZATION
					strcpy(replyMsg, "Media type should be Auto,1000FD,1000HD,100FD,100HD,10FD or 10HD.");
#else
					strcpy(replyMsg, "Duplex should be Full, Half or Auto. Speed should be 10 or 100.");
#endif
					zcfgFeJsonObjFree(ethObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}

			ret = zcfgFeObjJsonSet(RDM_OID_ETH_IFACE, &ethIid, ethObj, NULL);
		}
	}
	else{
		zcfgFeJsonObjFree(ethObj);
		return ZCFG_NOT_FOUND;
	}

	zcfgFeJsonObjFree(ethObj);
	return ret;
}
zcfgRet_t zcfgFeDalEthCtlEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *reqObject = NULL;
	int reqObjIdx = 0;
	if(json_object_get_type(Jobj) == json_type_object){// set by dalcmd
		ret = setEthctlIntf(Jobj, replyMsg);
	}
	else{
		while((reqObject = json_object_array_get_idx(Jobj, reqObjIdx)) !=NULL){
			ret = setEthctlIntf(reqObject, replyMsg);
			if(ret != ZCFG_SUCCESS){
				return ret;
			}
			reqObjIdx++;
		}
	}
	return ret;
}

zcfgRet_t zcfgFeDalEthCtl(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) 
		ret = zcfgFeDalEthCtlEdit(Jobj, replyMsg);

	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalEthCtlGet(Jobj, Jarray, replyMsg);
	else 
		printf("Unknown method:%s\n", method);
	return ret;
}