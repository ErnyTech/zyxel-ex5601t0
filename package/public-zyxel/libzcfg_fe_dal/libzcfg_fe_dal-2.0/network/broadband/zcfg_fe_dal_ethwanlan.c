#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_webgui_flag.h"


dal_param_t ETHWANLAN_param[]={
	{"Enable", 					dalType_boolean,	0,	0,	NULL,		NULL,			dal_Edit},
	//{"BCM63138_EthWanAsLan", 	dalType_boolean,	0,	0,	NULL,		NULL,			dalcmd_Forbid},
	//{"EthWanAsLan", 			dalType_boolean,	0,	0,	NULL,		NULL,			dalcmd_Forbid},
	//{"X_ZYXEL_Upstream", 		dalType_boolean,	0,	0,	NULL,		NULL,			dalcmd_Forbid},
	{NULL,	0,	0,	0,	NULL,	NULL,	0},

};

zcfgRet_t getEthObj(struct json_object **output, int *index){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t objIid;
	int intfId, lastIntfId;
	char *name = NULL;
	char *lanPort = NULL;
	
	*output = NULL;
	lastIntfId = -1;
	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &objIid, &obj) == ZCFG_SUCCESS){

		lanPort = (char *)json_object_get_string(json_object_object_get(obj, "X_ZYXEL_LanPort"));
		if(!strcmp(lanPort,"ETHWAN")){
			*output = obj;
			*index = (int)objIid.idx[0];
			break;
		}
		
		name = (char *)json_object_get_string(json_object_object_get(obj, "Name"));
#if defined(ECONET_PLATFORM)
		sscanf(name,"eth0.%d",&intfId);
#else
		sscanf(name,"eth%d",&intfId);
#endif

		if(intfId > lastIntfId){
			lastIntfId = intfId;

			if(*output!=NULL)
				zcfgFeJsonObjFree(*output);
			
			*output = obj;
			*index = (int)objIid.idx[0];
		}

		if(obj != *output)
			zcfgFeJsonObjFree(obj);

	}

	return ret;
}

void zcfgFeDalShowEthWanLan(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("Ethernet WAN status : ");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj != NULL){
			if(json_object_get_boolean(json_object_object_get(obj, "Enable")))
				printf("Enabled");
			else
				printf("Disabled");
		}
	}
	printf("\n");
}

zcfgRet_t zcfgFeDal_ETHWanLAN_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

#if (defined(CONFIG_BCM963138) || defined (CONFIG_BCM963148)) && defined(ZYXEL_SUPPORT_ETHWAN_AS_LAN)
	struct json_object *custObj = NULL;
	objIndex_t custIid;
	bool ethWanAsLan;
#endif

	// else
	struct json_object *pramJobj = NULL;
	struct json_object *ethObj = NULL;
	int iidIndex;

	pramJobj = json_object_new_object();

#if (defined(CONFIG_BCM963138) || defined (CONFIG_BCM963148)) && defined(ZYXEL_SUPPORT_ETHWAN_AS_LAN)

	IID_INIT(custIid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GUI_CUSTOMIZATION, &custIid, &custObj)) != ZCFG_SUCCESS)
		return ret;

	ethWanAsLan = json_object_get_boolean(json_object_object_get(custObj,"EthWanAsLan"));

	if(ethWanAsLan)
		json_object_object_add(pramJobj, "Enable", json_object_new_boolean(false));
	else
		json_object_object_add(pramJobj, "Enable", json_object_new_boolean(true));

	zcfgFeJsonObjFree(custObj);

#else
	getEthObj(&ethObj,&iidIndex);
	json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(ethObj,"X_ZYXEL_Upstream")));
	zcfgFeJsonObjFree(ethObj);

#endif

	json_object_array_add(Jarray, JSON_OBJ_COPY(pramJobj));
	zcfgFeJsonObjFree(pramJobj);
	return ret;

}

#if !((defined(CONFIG_BCM963138) || defined (CONFIG_BCM963148)) && defined(ZYXEL_SUPPORT_ETHWAN_AS_LAN))
zcfgRet_t objectStepStackLowerLayer(const char *startLayer, const char *destLowLayer, char *higherLayer, char *lowerLayer);
#endif
zcfgRet_t zcfgFeDal_ETHWanLAN_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//struct json_object *obj = NULL;
	//bool X_ZYXEL_Upstream = false;
#if (defined(CONFIG_BCM963138) || defined (CONFIG_BCM963148)) && defined(ZYXEL_SUPPORT_ETHWAN_AS_LAN)
	struct json_object *ethWanLanObj = NULL;
	objIndex_t ethWanLanObjIid = {0};
	bool EthWanAsLan = false;
#endif
	//bool wanPort = false;

	bool enable;
	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));

#if (defined(CONFIG_BCM963138) || defined (CONFIG_BCM963148)) && defined(ZYXEL_SUPPORT_ETHWAN_AS_LAN)

	IID_INIT(ethWanLanObjIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GUI_CUSTOMIZATION, &ethWanLanObjIid, &ethWanLanObj);
	if(ret == ZCFG_SUCCESS){

		if(enable)
			json_object_object_add(ethWanLanObj, "EthWanAsLan", json_object_new_boolean(false));
		else
			json_object_object_add(ethWanLanObj, "EthWanAsLan", json_object_new_boolean(true));

		zcfgFeObjJsonSet(RDM_OID_GUI_CUSTOMIZATION, &ethWanLanObjIid, ethWanLanObj, NULL);
	}

	zcfgFeJsonObjFree(ethWanLanObj);
		
#else

	struct json_object *ethObj = NULL;
	objIndex_t ethObjIid;
	int iidIndex;

	getEthObj(&ethObj,&iidIndex);
	json_object_object_add(ethObj, "X_ZYXEL_Upstream", json_object_new_boolean(enable));
	IID_INIT(ethObjIid);
	ethObjIid.level = 1;
	ethObjIid.idx[0]=iidIndex;
	zcfgFeObjJsonBlockedSet(RDM_OID_ETH_IFACE, &ethObjIid, ethObj, NULL);
	zcfgFeJsonObjFree(ethObj);

	if(enable){
		struct json_object *ipIntObj = NULL;
		objIndex_t ipIntIid;
		char higherLayers[64] = {0}, lowerLayers[64] = {0};
		char ethIfaceName[64] = {0};
		sprintf(ethIfaceName,"Ethernet.Interface.%d",iidIndex);

		IID_INIT(ipIntIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIntIid, &ipIntObj) == ZCFG_SUCCESS) {
			const char *ipIfaceLowerLayers = json_object_get_string(json_object_object_get(ipIntObj, "LowerLayers"));
			if(ipIfaceLowerLayers && strlen(ipIfaceLowerLayers)){
				if(objectStepStackLowerLayer(ipIfaceLowerLayers, ethIfaceName, higherLayers, lowerLayers) == ZCFG_SUCCESS){
					if(ipIntObj){
						json_object_object_add(ipIntObj, "X_ZYXEL_AccessDescription", json_object_new_string("SetIpIfaceStackModified"));
						if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_IP_IFACE, &ipIntIid, ipIntObj, NULL)) != ZCFG_SUCCESS){
							zcfgLog(ZCFG_LOG_ERR, "%s(): set ipIntObj->X_ZYXEL_AccessDescription Fail.\n", __FUNCTION__);
						}
					}
				}
				json_object_put(ipIntObj);
				ipIntObj = NULL;
			}
		}
		
	}

	/*
		X_ZYXEL_Upstream = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_Upstream"));

		while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &objIid, &obj) == ZCFG_SUCCESS){
			ethName = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_LanPort"));
			memcpy(&ethWanLanObjIid, &objIid, sizeof(objIndex_t));
			if(ethName!=NULL){
				if(!strcmp(ethName,"ETHWAN")){
					printf("ETHWAN name found!\n", __FUNCTION__);
					ethWanLanObj = obj;
					wanPort = true;
					break;
				}
			}
			json_object_put(obj);
		}

		if(!wanPort){
			//Last Port
			zcfgFeObjJsonGet(RDM_OID_ETH_IFACE, &ethWanLanObjIid, &ethWanLanObj);
		}
		
		if(ethWanLanObj!=NULL){
			json_object_object_add(ethWanLanObj, "X_ZYXEL_Upstream", json_object_new_boolean(X_ZYXEL_Upstream));
			zcfgFeObjJsonSet(RDM_OID_ETH_IFACE, &ethWanLanObjIid, ethWanLanObj, NULL);
		}
	*/
#endif

	if(ret==ZCFG_SUCCESS)
		zcfgFeReqReboot(NULL);

	return ret;
}

/*
 * This function is support for CLI to distinguish this modle has WAN port or not.
 * e.g. VMG1312 VMG5313 do not have WAN port for Eth, thus ethwanlan need to change Lan4 port to Wan port
 */

bool DeviceIntfNumofEntry(){
	struct json_object *obj = NULL;
	objIndex_t Iid = {0};
	int flag1 = 0;
	
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GUI_CUSTOMIZATION, &Iid, &obj) == ZCFG_SUCCESS){
		flag1 = json_object_get_int(json_object_object_get(obj, "Flag1"));
		zcfgFeJsonObjFree(obj);
		if(flag1 & HIDE_ETH_WAN_TAB)
			return false;
		else
			return true;
	}
	else
		return false;
}

zcfgRet_t zcfgFeDalEthWanLan(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if(DeviceIntfNumofEntry()){
		if(!strcmp(method, "PUT"))
			ret = zcfgFeDal_ETHWanLAN_Edit(Jobj, NULL);
		else if(!strcmp(method, "GET"))
			ret = zcfgFeDal_ETHWanLAN_Get(Jobj, Jarray, NULL);
		else
			printf("Unknown method:%s\n", method);
	}
	else
		printf("This device do not support Ethernet LAN4 port change to WAN port.");

	return ret;
}

