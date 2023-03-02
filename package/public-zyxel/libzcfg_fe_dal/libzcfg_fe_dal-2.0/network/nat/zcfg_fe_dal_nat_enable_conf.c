
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

#define LEVEL_NAME_UD "UserDefined" //Telia Estonia Firewall

dal_param_t NAT_ENABLE_CONF_param[] =
{
	{"Nat",								dalType_boolean,	0,	0,		NULL,	NULL,					0},
};

zcfgRet_t zcfgFeDalNatEnableConfGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *natEnableJobj = NULL;	
	objIndex_t objIid;
	
	IID_INIT(objIid);
	paramJobj = json_object_new_object();
	while( zcfgFeObjJsonGetNext(RDM_OID_NAT_INTF_SETTING, &objIid, &natEnableJobj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "NAT_Enable", JSON_OBJ_COPY(json_object_object_get(natEnableJobj, "Enable")));
		//printf("%s: json_object_get_string(json_object_object_get(natEnableJobj, Enable)) =%d\n", __FUNCTION__, json_object_get_boolean(json_object_object_get(paramJobj, "Enable")));
		zcfgFeJsonObjFree(natEnableJobj);
	}

	json_object_array_add(Jarray, paramJobj);
	return ret;
}

void zcfgFeDalShowNatEnableConf(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("NAT Enable Configuration\n");
		printf("%-50s %d \n","NAT Enable :", json_object_get_boolean(json_object_object_get(obj, "NAT_Enable")));
	}
}

zcfgRet_t zcfgFeDalNatEnableConfEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	json_object *natIntfObj = NULL;
	bool nat_enable = false;
		
	IID_INIT(objIid);
	while( zcfgFeObjJsonGetNext(RDM_OID_NAT_INTF_SETTING, &objIid, &natIntfObj) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "Nat") != NULL){
			nat_enable = json_object_get_boolean(json_object_object_get(Jobj, "Nat"));
			//printf("nat_enable=%d\n", nat_enable);
			json_object_object_add(natIntfObj, "Enable", json_object_new_boolean(nat_enable));
			
			/* When NAT is from Disabled to Enabled, three parameters should be set
			"Device.NAT.InterfaceSetting.1.Interface": "Device.IP.Interface.2",
			"Device.NAT.InterfaceSetting.1.Enable": true,
			"Device.IP.Interface.2.Reset" : true
			*/
			if(1== nat_enable){
				//"Device.NAT.InterfaceSetting.1.Interface": "Device.IP.Interface.2",
				struct json_object *routerObj = NULL;
				objIndex_t routerIid;
				char *Interface= NULL;
				char nat_interface[64] = {0};

				routerIid.level = 1;
                routerIid.idx[0] = 1;
                if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ROUTING_ROUTER, &routerIid, &routerObj)==ZCFG_SUCCESS){
					Interface = (char *)json_object_get_string(json_object_object_get(routerObj, "X_ZYXEL_ActiveDefaultGateway"));
					
					if(Interface)
						snprintf(nat_interface, sizeof(nat_interface), "%s", Interface );

					if(routerObj) 
						zcfgFeJsonObjFree(routerObj);
				}
				else {
					printf("%s: Get ROUTING_ROUTER fail.\n", __FUNCTION__);
					return ret;
				}
				
				json_object_object_add(natIntfObj, "Interface", json_object_new_string(nat_interface));
				
				//"Device.IP.Interface.2.Reset" : true
				objIndex_t IpIfaceIid;
				struct json_object *IpIfaceObj = NULL;
				bool reset = false;
				
				IID_INIT(IpIfaceIid);
				IpIfaceIid.level = 1;
				IpIfaceIid.idx[0] =2; //hardcode here ..
				
				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &IpIfaceIid, &IpIfaceObj)==ZCFG_SUCCESS){
					reset = json_object_get_boolean(json_object_object_get(IpIfaceObj, "Reset"));
					//printf("reset=%d\n", reset);
					reset = true;
					json_object_object_add(IpIfaceObj, "Reset", json_object_new_boolean(reset));
				}
				else {
					printf("%s: Set IP_IFACE fail.\n", __FUNCTION__);
					return ret;
				}

				if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &IpIfaceIid, IpIfaceObj, NULL)) != ZCFG_SUCCESS){
					printf("%s: Write IP_IFACE object fail\n", __FUNCTION__);
					return ret;
				}
				if(IpIfaceObj)
					zcfgFeJsonObjFree(IpIfaceObj);

				/* Telia Estonia Firewall
				When AIP service (this a service with public IP on the lan side and with nat disabled) is used, 
				then incoming connections are blocked. 
				We need to change firewall level (web gui: /Firewall) from "Medium" to "Low" in order to solve this issue. 
				Please provide TR069 parameter for controlling this function OR make this switch (Medium<->Low) automatic 
				whenever NAT is enabled/disabled.
				*/
				objIndex_t levelIid;
				struct json_object *levelObj = NULL;
				const char *name = NULL;
				char firewall_level[64] = {0};

				IID_INIT(levelIid);
				while(zcfgFeObjJsonGetNext(RDM_OID_FIREWALL_LEVEL, &levelIid, &levelObj) == ZCFG_SUCCESS) {
					name = json_object_get_string(json_object_object_get(levelObj, "Name"));
					if(!strcmp(name, LEVEL_NAME_UD)){
						strcpy(firewall_level, "Low");//X_ZYXEL_DefaultConfig Low == (GUI Firewall Level) Medium
						json_object_object_add(levelObj, "X_ZYXEL_DefaultConfig", json_object_new_string(firewall_level));
						break;
					}
				}

				if((ret = zcfgFeObjJsonSet(RDM_OID_FIREWALL_LEVEL, &levelIid, levelObj, NULL)) != ZCFG_SUCCESS){
					printf("%s: Write FIREWALL_LEVEL object fail\n", __FUNCTION__);
					return ret;
				}
				if(levelObj)
					zcfgFeJsonObjFree(levelObj);
			} 			
			else {
				/* Telia Estonia Firewall
				When AIP service (this a service with public IP on the lan side and with nat disabled) is used, 
				then incoming connections are blocked. 
				We need to change firewall level (web gui: /Firewall) from "Medium" to "Low" in order to solve this issue. 
				Please provide TR069 parameter for controlling this function OR make this switch (Medium<->Low) automatic 
				whenever NAT is enabled/disabled.
				*/
				objIndex_t levelIid;
				struct json_object *levelObj = NULL;
				const char *name = NULL;
				char firewall_level[64] = {0};

				IID_INIT(levelIid);
				while(zcfgFeObjJsonGetNext(RDM_OID_FIREWALL_LEVEL, &levelIid, &levelObj) == ZCFG_SUCCESS) {
					name = json_object_get_string(json_object_object_get(levelObj, "Name"));
					if(!strcmp(name, LEVEL_NAME_UD)){
						strcpy(firewall_level, "Off");//X_ZYXEL_DefaultConfig Off == (GUI Firewall Level) Low
						json_object_object_add(levelObj, "X_ZYXEL_DefaultConfig", json_object_new_string(firewall_level));
						break;
					}
				}

				if((ret = zcfgFeObjJsonSet(RDM_OID_FIREWALL_LEVEL, &levelIid, levelObj, NULL)) != ZCFG_SUCCESS){
					printf("%s: Write FIREWALL_LEVEL object fail\n", __FUNCTION__);
					return ret;
				}
				if(levelObj)
					zcfgFeJsonObjFree(levelObj);
			}
			
			//if nat_enable is false, only set "Device.NAT.InterfaceSetting.1.Enable": false,
			if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_INTF_SETTING, &objIid, natIntfObj, NULL)) != ZCFG_SUCCESS){
				printf("%s: Set NAT_INTF_SETTING object fail\n", __FUNCTION__);
				return ret;
			}
			if(natIntfObj)
				zcfgFeJsonObjFree(natIntfObj);
		}
	}

	return ret;
}

zcfgRet_t zcfgFeDalNatEnableConf(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;
	if(!strcmp(method, "PUT")) 
		ret = zcfgFeDalNatEnableConfEdit(Jobj, replyMsg);
	if(!strcmp(method, "GET")) 
		ret = zcfgFeDalNatEnableConfGet(Jobj, Jarray, NULL);
	return ret;
}
