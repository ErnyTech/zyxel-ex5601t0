
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"
#include "zos_api.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t NatPortFwd_param[] =
{
	{"Index",						dalType_int,		0,	0,		NULL,	NULL,				dal_Edit|dal_Delete},
	{"Description",					dalType_string,		1,	256,	NULL,	NULL,				dal_Add},
	{"Enable",						dalType_boolean,	0,	0,		NULL,	NULL,				0},
#ifndef ABOG_CUSTOMIZATION
	{"Interface",					dalType_LanRtWanWWan,	0,	0,		NULL,	NULL,				dal_Add},
	{"X_ZYXEL_AutoDetectWanStatus",	dalType_boolean,	0,	0,		NULL,	NULL,				dalcmd_Forbid},
#endif
	{"ExternalPortStart",			dalType_int,		1,	65535,	NULL,	NULL,				dal_Add},
	{"ExternalPortEnd",				dalType_int,		1,	65535,	NULL,	NULL,				dal_Add},
	{"InternalPortStart",			dalType_int,		1,	65535,	NULL,	NULL,				dal_Add},
	//{"X_ZYXEL_InternalPortEndRange",dalType_int,		1,	63737,	NULL,	NULL,	0},
	{"InternalPortEnd",				dalType_int,		1,	65535,	NULL,	NULL,				dal_Add},
	//{"X_ZYXEL_SetOriginatingIP",	dalType_boolean, 	0, 0, 		NULL,	NULL,	0},
	{"SetOriginatingIP",			dalType_boolean, 	0,	0, 		NULL,	NULL,				dal_Add},
	{"OriginatingIpAddress",		dalType_v4Addr, 	0, 	0, 		NULL,	NULL,				0},
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
	{"OriginatingSubnetMask",		dalType_v4Addr, 	0,	0,		NULL,	NULL,				0},
#endif
	{"X_ZYXEL_AutoDetectWanStatus",	dalType_boolean,	0,	0,		NULL,	NULL,				0},
#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
	{"WANIP",				dalType_v4Addr,		0,	0,		NULL,	NULL,				0},
#endif
	{"Protocol",					dalType_string,		0,	0,		NULL,	"TCP|UDP|ALL",		dal_Add},
	{"InternalClient",				dalType_v4Addr,		0,	256,	NULL,	NULL,				dal_Add},
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	{"SetFromACS",					dalType_boolean,	0,	0,		NULL,	NULL,				0},	
#endif
	{NULL,							0,					0,	0,		NULL,	NULL,				0}
};

zcfgRet_t zcfgFeDalNATPortRangeCheck(bool isDSL,bool isEdit,struct json_object *Jobj,int index,int portStart,int portEnd,const char *newOriginatingIpAddress,char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t Iid = {0};
	int externalport = portStart;
	int externalportendrange = portEnd;
	const char *existOriginatingIpAddress = NULL;

#ifdef CONFIG_TAAAB_DSL_BINDING
	struct json_object *checkObj = NULL; 
	const char *Interface = NULL;
	objIndex_t ipIfaceIid = {0};
	const char *description=NULL;
	char X_TAAAB_SrvName[32] = {0};
	char X_ZYXEL_SrvName[32] = {0};
	bool found = false;
	char sameDSLIntfPath[32];
	if(isEdit&&isDSL){
		Interface = Jgets(Jobj, "Interface");
		description = Jgets(Jobj, "Description");
		IID_INIT(Iid);
		Iid.level = 1;
		sscanf(Interface, "IP.Interface.%hhu", &Iid.idx[0]);
		zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &Iid, &obj);
		ZOS_STRCPY_M(X_TAAAB_SrvName, Jgets(obj, "X_TT_SrvName"));
		ZOS_STRCPY_M(X_ZYXEL_SrvName, Jgets(obj, "X_ZYXEL_SrvName"));
		zcfgFeJsonObjFree(obj);
			
		IID_INIT(ipIfaceIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &obj) == ZCFG_SUCCESS){
		
		
			if(ipIfaceIid.idx[0] == Iid.idx[0]){
				zcfgFeJsonObjFree(obj);
				continue;
			}
			
			if(!strcmp(X_TAAAB_SrvName, Jgets(obj, "X_TT_SrvName"))&&strcmp(X_ZYXEL_SrvName,Jgets(obj, "X_ZYXEL_SrvName"))){
				               
				found = true;
				sprintf(sameDSLIntfPath, "IP.Interface.%d", ipIfaceIid.idx[0]);
				zcfgFeJsonObjFree(obj);
				break;
			}
		
		}
	}
#endif	
	
	IID_INIT(Iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &Iid, &obj) == ZCFG_SUCCESS){
		//new rule for port rule(one to one)
		//new rule for port rule(many to one)
		//printf("test:%d %d %lld %lld\n",externalport,externalportendrange,json_object_get_int(json_object_object_get(obj, "ExternalPort")),json_object_get_int(json_object_object_get(obj, "ExternalPortEndRange")));
		if(index == Iid.idx[0])
	    {
			zcfgFeJsonObjFree(obj);
			continue;
		}
#ifdef CONFIG_TAAAB_DSL_BINDING
		else if(!strcmp(sameDSLIntfPath,json_object_get_string(json_object_object_get(obj, "Interface")))){
			zcfgFeJsonObjFree(obj);
			continue;
		}
#endif
		else{
			existOriginatingIpAddress = json_object_get_string(json_object_object_get(obj, "OriginatingIpAddress"));
			if(newOriginatingIpAddress == NULL)
				newOriginatingIpAddress = "";
			if(!strcmp(existOriginatingIpAddress,newOriginatingIpAddress)){
				if(externalport==json_object_get_int(json_object_object_get(obj, "ExternalPort"))){
					if(replyMsg != NULL){
						strncat(replyMsg, "Port is duplicate.", sizeof(replyMsg));	
						json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.NAT.PortForwarding.error.port_duplicate"));
					}
           	   		zcfgFeJsonObjFree(obj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
			
				if(externalport>json_object_get_int(json_object_object_get(obj, "ExternalPortEndRange"))||externalportendrange<json_object_get_int(json_object_object_get(obj, "ExternalPort"))){
					zcfgFeJsonObjFree(obj);
				}
				else{
					if(replyMsg != NULL){	
						strncat(replyMsg, "Port is duplicate.", sizeof(replyMsg));
						json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.NAT.PortForwarding.error.port_duplicate"));
					}
          	 		zcfgFeJsonObjFree(obj);
          	 		return ZCFG_INVALID_PARAM_VALUE;
				}
			}
		}
	}
	return ret;
}




void zcfgFeDalShowPortFwd(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	const char *origIP = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
#ifdef ABOG_CUSTOMIZATION
	printf("%-5s %-15s %-7s %-15s %-17s %-10s %-10s %-25s %-25s %-10s\n",
		"Index", "Description", "Enable", "Originating IP", "Server IP Address", "Start Port", "End Port", "Translation Start Port", "Translation End Port", "Protocol");
#else
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
	printf("%-5s %-15s %-7s %-15s %-15s %-17s %-15s %-10s %-10s %-25s %-25s %-10s\n",
		"Index", "Description", "Enable", "Originating IP", "Originating SubnetMask", "Server IP Address", "WAN Interface", "Start Port", "End Port", "Translation Start Port", "Translation End Port", "Protocol");
#else
	printf("%-5s %-15s %-7s %-15s %-17s %-15s %-10s %-10s %-25s %-25s %-10s\n",
		"Index", "Description", "Enable", "Originating IP", "Server IP Address", "WAN Interface", "Start Port", "End Port", "Translation Start Port", "Translation End Port", "Protocol");
#endif //ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
#endif
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(!json_object_get_boolean(json_object_object_get(obj, "SetOriginatingIP")) ||
			!strcmp(json_object_get_string(json_object_object_get(obj, "OriginatingIpAddress")),"")){
			origIP = "N/A";
		}
		else
			origIP = json_object_get_string(json_object_object_get(obj, "OriginatingIpAddress"));
#ifdef ABOG_CUSTOMIZATION
		printf("%-5s %-15s %-7d %-15s %-17s %-10s %-10s %-25s %-25s %-10s\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Description")),
			json_object_get_boolean(json_object_object_get(obj, "Enable")),
			origIP,
			json_object_get_string(json_object_object_get(obj, "InternalClient")),
			json_object_get_string(json_object_object_get(obj, "ExternalPortStart")),
			json_object_get_string(json_object_object_get(obj, "ExternalPortEnd")),
			json_object_get_string(json_object_object_get(obj, "InternalPortStart")),
			json_object_get_string(json_object_object_get(obj, "InternalPortEnd")),
			json_object_get_string(json_object_object_get(obj, "Protocol")));

#else
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
		printf("%-5s %-15s %-7d %-15s %-15s %-17s %-15s %-10s %-10s %-25s %-25s %-10s\n",
#else
		printf("%-5s %-15s %-7d %-15s %-17s %-15s %-10s %-10s %-25s %-25s %-10s\n",
#endif
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Description")),
			json_object_get_boolean(json_object_object_get(obj, "Enable")),
			origIP,
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK		
			json_object_get_string(json_object_object_get(obj, "OriginatingSubnetMask")),
#endif			
			json_object_get_string(json_object_object_get(obj, "InternalClient")),
			json_object_get_string(json_object_object_get(obj, "Interface")),
			json_object_get_string(json_object_object_get(obj, "ExternalPortStart")),
			json_object_get_string(json_object_object_get(obj, "ExternalPortEnd")),
			json_object_get_string(json_object_object_get(obj, "InternalPortStart")),
			json_object_get_string(json_object_object_get(obj, "InternalPortEnd")),
			json_object_get_string(json_object_object_get(obj, "Protocol")));
#endif
	}
}

zcfgRet_t zcfgFeDalNatPortMappingAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatPortMappingJobj = NULL;
	objIndex_t objIid = {0};

	bool enable = false;
	const char *interface = NULL;
	const char *originatingipaddress = NULL;
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
	const char *originatingSubnetMask = NULL;
#endif
	int externalport = 0;
	int externalportendrange = 0;
	int internalport = 0;
	int internalportendrange = 0;
	bool setoriginatingip = false;
	const char *protocol = NULL;
	const char *internalclient = NULL;
	const char *description = NULL;
	bool autoDetectWanStatus = false;
	bool isEdit=false;
	
#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
	const char *wanip = NULL;
#endif
#ifdef CONFIG_TAAAB_DSL_BINDING
		objIndex_t ipIfaceIid;
		struct json_object *ipIfaceObj = NULL;
		objIndex_t referobjIid = {0};
		char New_SrvName[64] = {0};
		char New_TAAAB_SrvName[64] = {0};
		char newreferinterface[64] = {0};
		char atmdescription[128] = {0};
		int newreferindex = -1;
#endif

	//Don' t allow Description prefix is "APP:", it is used for NAT applications rule.
	if(json_object_object_get(Jobj, "Description") != NULL){
		if(!strncmp(json_object_get_string(json_object_object_get(Jobj, "Description")), "APP:", 4)){
			if(replyMsg != NULL)
				ZOS_STRCPY_M(replyMsg, "Invalid NatPortFwd Description.");
			return ZCFG_REQUEST_REJECT;
		}
	}
	description = json_object_get_string(json_object_object_get(Jobj, "Description"));
	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &objIid, &NatPortMappingJobj) == ZCFG_SUCCESS){
		if(strncmp(json_object_get_string(json_object_object_get(Jobj, "Description")), "APP:", 4)){
			if(!strcmp(description, json_object_get_string(json_object_object_get(NatPortMappingJobj, "Description")))){
				zcfgFeJsonObjFree(NatPortMappingJobj);
				strcat(replyMsg, "Description is duplicate.");
				return ZCFG_INVALID_PARAM_VALUE;
			}
			zcfgFeJsonObjFree(NatPortMappingJobj);
		}
	}

	//declare for new port rule
	externalport = json_object_get_int(json_object_object_get(Jobj, "ExternalPortStart"));
	externalportendrange = json_object_get_int(json_object_object_get(Jobj, "ExternalPortEnd"));
	originatingipaddress = json_object_get_string(json_object_object_get(Jobj, "OriginatingIpAddress"));
	if ((ret = zcfgFeDalNATPortRangeCheck(NULL,isEdit,Jobj,0,externalport,externalportendrange,originatingipaddress, replyMsg)) != ZCFG_SUCCESS)
		return ret;


	IID_INIT(objIid);
	ret = zcfgFeObjJsonAdd(RDM_OID_NAT_PORT_MAPPING, &objIid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGet(RDM_OID_NAT_PORT_MAPPING, &objIid, &NatPortMappingJobj);
	}
	if(ret == ZCFG_SUCCESS){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
		originatingSubnetMask = json_object_get_string(json_object_object_get(Jobj, "OriginatingSubnetMask"));	
		//printf("#### ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK #### Add originatingSubnetMask = %s \n", originatingSubnetMask);
#endif
		externalport = json_object_get_int(json_object_object_get(Jobj, "ExternalPortStart"));
		externalportendrange = json_object_get_int(json_object_object_get(Jobj, "ExternalPortEnd"));
		internalport = json_object_get_int(json_object_object_get(Jobj, "InternalPortStart"));
		internalportendrange = json_object_get_int(json_object_object_get(Jobj, "InternalPortEnd"));
		setoriginatingip = json_object_get_boolean(json_object_object_get(Jobj, "SetOriginatingIP"));
		protocol = json_object_get_string(json_object_object_get(Jobj, "Protocol"));
		internalclient = json_object_get_string(json_object_object_get(Jobj, "InternalClient"));
		description = json_object_get_string(json_object_object_get(Jobj, "Description"));
#ifdef ABOG_CUSTOMIZATION
		autoDetectWanStatus = true;
#else
		interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
		autoDetectWanStatus = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_AutoDetectWanStatus"));
		if(!strcmp(interface, "")){
			autoDetectWanStatus = true;
		}
#endif
#ifdef CONFIG_TAAAB_DSL_BINDING	   
					IID_INIT(ipIfaceIid);
					ipIfaceIid.level = 1;
					sscanf(interface, "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
					if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS)
					{
						ZOS_STRCPY_M(New_SrvName,Jgets(ipIfaceObj, "X_ZYXEL_SrvName"));
						ZOS_STRCPY_M(New_TAAAB_SrvName,Jgets(ipIfaceObj, "X_TT_SrvName"));
						zcfgFeJsonObjFree(ipIfaceObj);
					}
					
            if(strstr(New_TAAAB_SrvName,"_WAN") != NULL)
			{       
			   newreferindex = -1;
			}
			else if(strstr(New_TAAAB_SrvName,"_DSL") != NULL)
			{
			   IID_INIT(referobjIid);
			   ret = zcfgFeObjJsonAdd(RDM_OID_NAT_PORT_MAPPING, &referobjIid, NULL);
			   if(ret == ZCFG_SUCCESS){
				   newreferindex = referobjIid.idx[0];
			   }			
			}
#endif

#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
		wanip = json_object_get_string(json_object_object_get(Jobj, "WANIP"));
		if(!strcmp(wanip, "")){
			autoDetectWanStatus = true;
			ZOS_STRCPY_M(interface,"");
		}
		else
			autoDetectWanStatus = false;
#endif

#ifndef ZYXEL_USA_PRODUCT
		//check port range
		if(internalport > internalportendrange){
			if(replyMsg != NULL)
				ZOS_STRCPY_M(replyMsg, "Internal port range is invalid.");

			zcfgFeJsonObjFree(NatPortMappingJobj);
			zcfgFeObjJsonDel(RDM_OID_NAT_PORT_MAPPING, &objIid, NULL);
			return ZCFG_INVALID_PARAM_VALUE;
		}
		if(externalport > externalportendrange){
			if(replyMsg != NULL)
				ZOS_STRCPY_M(replyMsg, "External port range is invalid.");

			zcfgFeJsonObjFree(NatPortMappingJobj);
			zcfgFeObjJsonDel(RDM_OID_NAT_PORT_MAPPING, &objIid, NULL);
			return ZCFG_INVALID_PARAM_VALUE;
		}

#endif

		if(json_object_object_get(Jobj, "Enable") != NULL)
			json_object_object_add(NatPortMappingJobj, "Enable", json_object_new_boolean(enable));
#ifdef ABOG_CUSTOMIZATION
			json_object_object_add(NatPortMappingJobj, "Interface", json_object_new_string(""));
			json_object_object_add(NatPortMappingJobj, "X_ZYXEL_AutoDetectWanStatus", json_object_new_boolean(autoDetectWanStatus));
#else
		if(json_object_object_get(Jobj, "Interface") != NULL)
			json_object_object_add(NatPortMappingJobj, "Interface", json_object_new_string(interface));
		if(json_object_object_get(Jobj, "X_ZYXEL_AutoDetectWanStatus") != NULL)
			json_object_object_add(NatPortMappingJobj, "X_ZYXEL_AutoDetectWanStatus", json_object_new_boolean(autoDetectWanStatus));
#endif
		if(json_object_object_get(Jobj, "ExternalPortStart") != NULL)
			json_object_object_add(NatPortMappingJobj, "ExternalPort", json_object_new_int(externalport));
		if(json_object_object_get(Jobj, "ExternalPortEnd") != NULL)
			json_object_object_add(NatPortMappingJobj, "ExternalPortEndRange", json_object_new_int(externalportendrange));
		if(json_object_object_get(Jobj, "InternalPortStart") != NULL)
			json_object_object_add(NatPortMappingJobj, "InternalPort", json_object_new_int(internalport));
		if(json_object_object_get(Jobj, "InternalPortEnd") != NULL)
			json_object_object_add(NatPortMappingJobj, "X_ZYXEL_InternalPortEndRange", json_object_new_int(internalportendrange));
		if(json_object_object_get(Jobj, "SetOriginatingIP") != NULL)
			json_object_object_add(NatPortMappingJobj, "X_ZYXEL_SetOriginatingIP", json_object_new_boolean(setoriginatingip));
		if(json_object_object_get(Jobj, "OriginatingIpAddress") != NULL)
			json_object_object_add(NatPortMappingJobj, "OriginatingIpAddress", json_object_new_string(originatingipaddress));
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
		if(json_object_object_get(Jobj, "OriginatingSubnetMask") != NULL)
			json_object_object_add(NatPortMappingJobj, "OriginatingSubnetMask", json_object_new_string(originatingSubnetMask));
#endif
		if(json_object_object_get(Jobj, "Protocol") != NULL)
			json_object_object_add(NatPortMappingJobj, "Protocol", json_object_new_string(protocol));
		if(json_object_object_get(Jobj, "InternalClient") != NULL)
			json_object_object_add(NatPortMappingJobj, "InternalClient", json_object_new_string(internalclient));
		if(json_object_object_get(Jobj, "Description") != NULL)
			json_object_object_add(NatPortMappingJobj, "Description", json_object_new_string(description));

#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
		if(json_object_object_get(Jobj, "WANIP") != NULL)
			json_object_object_add(NatPortMappingJobj, "X_ZYXEL_WANIP", json_object_new_string(wanip));
#endif
#ifdef CONFIG_TAAAB_DSL_BINDING
			json_object_object_add(NatPortMappingJobj, "X_ZYXEL_REFERENCE", json_object_new_int(newreferindex));
#endif
		ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_MAPPING, &objIid, NatPortMappingJobj, NULL);
#ifdef ZYXEL_USA_PRODUCT
		if(ret != ZCFG_SUCCESS){
			if(replyMsg != NULL)
				ZOS_STRCPY_M(replyMsg, "Port mapping parameters are invalid");

			//printf("JJJJJJ delete empty\n");
			zcfgFeObjJsonDel(RDM_OID_NAT_PORT_MAPPING, &objIid, NULL);
		}
#endif
		zcfgFeJsonObjFree(NatPortMappingJobj);
	}

//TAAAB sync entry
#ifdef CONFIG_TAAAB_DSL_BINDING
				if(newreferindex != -1){
						IID_INIT(referobjIid);
						referobjIid.level = 1;
						referobjIid.idx[0] = newreferindex;
						zcfgFeObjJsonGetWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &referobjIid, &NatPortMappingJobj);
	
							IID_INIT(ipIfaceIid);
							while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS) {
								if(!strcmp(Jgets(ipIfaceObj, "X_TT_SrvName"),New_TAAAB_SrvName) && strcmp(Jgets(ipIfaceObj, "X_ZYXEL_SrvName"),New_SrvName) && strstr(Jgets(ipIfaceObj, "X_ZYXEL_SrvName"),"_ADSL"))
								{
										sprintf(newreferinterface, "IP.Interface.%u", ipIfaceIid.idx[0]);
										json_object_object_add(NatPortMappingJobj, "Interface", json_object_new_string(newreferinterface));
										json_object_object_add(NatPortMappingJobj, "X_ZYXEL_REFERENCE", json_object_new_int(-1));
										zcfgFeJsonObjFree(ipIfaceObj);
										break;
								}
								zcfgFeJsonObjFree(ipIfaceObj);
							}
	
							
							enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	
							autoDetectWanStatus = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_AutoDetectWanStatus"));
							if(!strcmp(interface, "")){
								autoDetectWanStatus = true;
							}
							else{
								autoDetectWanStatus = false;
							}
	
							originatingipaddress = json_object_get_string(json_object_object_get(Jobj, "OriginatingIpAddress"));
							externalport = json_object_get_int(json_object_object_get(Jobj, "ExternalPortStart"));
							externalportendrange = json_object_get_int(json_object_object_get(Jobj, "ExternalPortEnd"));
							internalport = json_object_get_int(json_object_object_get(Jobj, "InternalPortStart"));
							internalportendrange = json_object_get_int(json_object_object_get(Jobj, "InternalPortEnd"));
							setoriginatingip = json_object_get_boolean(json_object_object_get(Jobj, "SetOriginatingIP"));
							protocol = json_object_get_string(json_object_object_get(Jobj, "Protocol"));
							internalclient = json_object_get_string(json_object_object_get(Jobj, "InternalClient"));
							ZOS_STRCPY_M(atmdescription,json_object_get_string(json_object_object_get(Jobj, "Description")));
							strcat(atmdescription,"_ATM");
#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
							wanip = json_object_get_string(json_object_object_get(Jobj, "WANIP"));
#endif
							if(json_object_object_get(Jobj, "Enable") != NULL)
								json_object_object_add(NatPortMappingJobj, "Enable", json_object_new_boolean(enable));
#ifndef ABOG_CUSTOMIZATION
							if(json_object_object_get(Jobj, "X_ZYXEL_AutoDetectWanStatus") != NULL)
								json_object_object_add(NatPortMappingJobj, "X_ZYXEL_AutoDetectWanStatus", json_object_new_boolean(autoDetectWanStatus));
#endif
							if(json_object_object_get(Jobj, "ExternalPortStart") != NULL)
								json_object_object_add(NatPortMappingJobj, "ExternalPort", json_object_new_int(externalport));
							if(json_object_object_get(Jobj, "ExternalPortEnd") != NULL)
								json_object_object_add(NatPortMappingJobj, "ExternalPortEndRange", json_object_new_int(externalportendrange));
							if(json_object_object_get(Jobj, "InternalPortStart") != NULL)
								json_object_object_add(NatPortMappingJobj, "InternalPort", json_object_new_int(internalport));
							if(json_object_object_get(Jobj, "InternalPortEnd") != NULL)
								json_object_object_add(NatPortMappingJobj, "X_ZYXEL_InternalPortEndRange", json_object_new_int(internalportendrange));
							if(json_object_object_get(Jobj, "SetOriginatingIP") != NULL)
								json_object_object_add(NatPortMappingJobj, "X_ZYXEL_SetOriginatingIP", json_object_new_boolean(setoriginatingip));
							if(json_object_object_get(Jobj, "OriginatingIpAddress") != NULL)
								json_object_object_add(NatPortMappingJobj, "OriginatingIpAddress", json_object_new_string(originatingipaddress));
							if(json_object_object_get(Jobj, "Protocol") != NULL)
								json_object_object_add(NatPortMappingJobj, "Protocol", json_object_new_string(protocol));
							if(json_object_object_get(Jobj, "InternalClient") != NULL)
								json_object_object_add(NatPortMappingJobj, "InternalClient", json_object_new_string(internalclient));
							if(json_object_object_get(Jobj, "Description") != NULL)
								json_object_object_add(NatPortMappingJobj, "Description", json_object_new_string(atmdescription));
#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
							if(json_object_object_get(Jobj, "WANIP") != NULL)
								json_object_object_add(NatPortMappingJobj, "X_ZYXEL_WANIP", json_object_new_string(wanip));
#endif
							ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_MAPPING, &referobjIid, NatPortMappingJobj, NULL);
							zcfgFeJsonObjFree(NatPortMappingJobj);
				}	
#endif

	return ret;
}

zcfgRet_t zcfgFeDalNatPortMappingEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatPortMappingJobj = NULL;
	struct json_object *obj = NULL;
	objIndex_t objIid = {0};
	objIndex_t Iid = {0};
	bool enable = false;
	const char *interface = NULL;
	const char *originatingipaddress = NULL;
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
	const char *originatingSubnetMask = NULL;
#endif
	int externalport = 0;
	int externalportendrange = 0;
	int internalport = 0;
	int internalportendrange = 0;
	bool setoriginatingip = false;
	const char *protocol = NULL;
	const char *internalclient = NULL;
	const char *description = NULL;
	int index = 0;
	bool found = false;
	bool autoDetectWanStatus = false;
	bool isEdit=true;
	bool isDSL=false;
#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
	const char *wanip = NULL;
#endif
#ifdef CONFIG_TAAAB_DSL_BINDING
	objIndex_t ipIfaceIid;
	struct json_object *ipIfaceObj = NULL;
    objIndex_t oldipIfaceIid;
	struct json_object *oldipIfaceObj = NULL;
	objIndex_t newipIfaceIid;
	struct json_object *newipIfaceObj = NULL;
	objIndex_t referobjIid;
	const char *New_SrvName = NULL;
	const char *New_TAAAB_SrvName = NULL;
	const char *Old_SrvName = NULL;
	const char *Old_TAAAB_SrvName = NULL;
    char newreferinterface[64] = {0};
	char atmdescription[128] = {0};
	int newreferindex = -1;
#endif

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	description = json_object_get_string(json_object_object_get(Jobj, "Description"));
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = index;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &objIid, &NatPortMappingJobj) == ZCFG_SUCCESS){
		if(strncmp(json_object_get_string(json_object_object_get(NatPortMappingJobj, "Description")),"APP:",4)){
			found = true;
		}
	}
	if(json_object_object_get(Jobj, "Description")){
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &Iid, &obj) == ZCFG_SUCCESS){
			if(strncmp(json_object_get_string(json_object_object_get(Jobj, "Description")), "APP:", 4)){
				if(!strcmp(description, json_object_get_string(json_object_object_get(obj, "Description")))){
					if(index == Iid.idx[0]){
						zcfgFeJsonObjFree(obj);
					}
					else{
						zcfgFeJsonObjFree(obj);
						strcat(replyMsg, "Description is duplicate.");
						return ZCFG_INVALID_PARAM_VALUE;
					}
				}
			}
			zcfgFeJsonObjFree(obj);
		}
	}
	if(!found){
		if(replyMsg != NULL)
			ZOS_STRCPY_M(replyMsg, "Can not find the rule.");
		if(NatPortMappingJobj != NULL)
			zcfgFeJsonObjFree(NatPortMappingJobj);
		return ZCFG_NOT_FOUND;
	}

	if(found){
		//Don' t allow Description prefix is "APP:", it is used for NAT applications rule.
		if(json_object_object_get(Jobj, "Description") != NULL){
			if(!strncmp(json_object_get_string(json_object_object_get(Jobj, "Description")), "APP:", 4)){
				if(replyMsg != NULL)
					ZOS_STRCPY_M(replyMsg, "Invalid NatPortFwd Description");

				zcfgFeJsonObjFree(NatPortMappingJobj);
				return ZCFG_REQUEST_REJECT;
			}
		}

		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
#ifndef ABOG_CUSTOMIZATION
		interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
		autoDetectWanStatus = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_AutoDetectWanStatus"));
		if(!strcmp(interface, "")){
			autoDetectWanStatus = true;
		}
		else{
			autoDetectWanStatus = false;
		}
#endif
#ifdef CONFIG_TAAAB_DSL_BINDING	   
							IID_INIT(newipIfaceIid);
							newipIfaceIid.level = 1;
							sscanf(interface, "IP.Interface.%hhu", &newipIfaceIid.idx[0]);
							if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &newipIfaceIid, &newipIfaceObj) == ZCFG_SUCCESS)
							{
								New_SrvName = Jgets(newipIfaceObj, "X_ZYXEL_SrvName");
								New_TAAAB_SrvName = Jgets(newipIfaceObj, "X_TT_SrvName");
							}
							
							if(strstr(New_TAAAB_SrvName,"_DSL")){isDSL=true;}
							
							IID_INIT(oldipIfaceIid);
							oldipIfaceIid.level = 1;
							sscanf(Jgets(NatPortMappingJobj,"Interface"), "IP.Interface.%hhu", &oldipIfaceIid.idx[0]);
							if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &oldipIfaceIid, &oldipIfaceObj) == ZCFG_SUCCESS)
							{
								Old_SrvName = Jgets(oldipIfaceObj, "X_ZYXEL_SrvName");
								Old_TAAAB_SrvName = Jgets(oldipIfaceObj, "X_TT_SrvName");
							}	   
							
				if(strcmp(Jgets(NatPortMappingJobj,"Interface"),interface)){
					if(strstr(Old_TAAAB_SrvName,"_DSL") != NULL && strstr(New_TAAAB_SrvName,"_DSL") != NULL)
					{
						newreferindex = json_object_get_int(json_object_object_get(Jobj, "referenceIndex"));
					}
					else if(strstr(Old_TAAAB_SrvName,"_DSL") != NULL && strstr(New_TAAAB_SrvName,"_WAN") != NULL)
					{
					   IID_INIT(referobjIid);
					   referobjIid.level = 1;
					   referobjIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "referenceIndex"));
					   zcfgFeObjJsonDel(RDM_OID_NAT_PORT_MAPPING, &referobjIid, NULL);			 
					   newreferindex = -1;
		
					}
					else if(strstr(Old_TAAAB_SrvName,"_WAN") != NULL && strstr(New_TAAAB_SrvName,"_DSL") != NULL)
					{
					   IID_INIT(referobjIid);
					   ret = zcfgFeObjJsonAdd(RDM_OID_NAT_PORT_MAPPING, &referobjIid, NULL);
					   if(ret == ZCFG_SUCCESS){
						   newreferindex = referobjIid.idx[0];
					   }			
					}
					else if(strstr(Old_TAAAB_SrvName,"_WAN") != NULL && strstr(New_TAAAB_SrvName,"_WAN") != NULL)
					{
						newreferindex = -1;
					}
				}
				else{
					if(strstr(New_TAAAB_SrvName,"_DSL") != NULL)
					{
						newreferindex = json_object_get_int(json_object_object_get(Jobj, "referenceIndex"));
					}
					else if(strstr(New_TAAAB_SrvName,"_WAN") != NULL)
					{
						newreferindex = -1;
					}
				}
#endif
		originatingipaddress = json_object_get_string(json_object_object_get(Jobj, "OriginatingIpAddress"));
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
		originatingSubnetMask = json_object_get_string(json_object_object_get(Jobj, "OriginatingSubnetMask"));		
#endif
		externalport = json_object_get_int(json_object_object_get(Jobj, "ExternalPortStart"));
		externalportendrange = json_object_get_int(json_object_object_get(Jobj, "ExternalPortEnd"));
		internalport = json_object_get_int(json_object_object_get(Jobj, "InternalPortStart"));
		internalportendrange = json_object_get_int(json_object_object_get(Jobj, "InternalPortEnd"));
		setoriginatingip = json_object_get_boolean(json_object_object_get(Jobj, "SetOriginatingIP"));
		protocol = json_object_get_string(json_object_object_get(Jobj, "Protocol"));
		internalclient = json_object_get_string(json_object_object_get(Jobj, "InternalClient"));
		description = json_object_get_string(json_object_object_get(Jobj, "Description"));
#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
		wanip = json_object_get_string(json_object_object_get(Jobj, "WANIP"));
		if(!strcmp(wanip, "")){
			autoDetectWanStatus = true;
			ZOS_STRCPY_M(interface,"");
		}
		else
			autoDetectWanStatus = false;
#endif
		if(json_object_object_get(Jobj, "Enable") != NULL)
			json_object_object_add(NatPortMappingJobj, "Enable", json_object_new_boolean(enable));
#ifndef ABOG_CUSTOMIZATION
		if(json_object_object_get(Jobj, "Interface") != NULL)
			json_object_object_add(NatPortMappingJobj, "Interface", json_object_new_string(interface));
		if(json_object_object_get(Jobj, "X_ZYXEL_AutoDetectWanStatus") != NULL)
			json_object_object_add(NatPortMappingJobj, "X_ZYXEL_AutoDetectWanStatus", json_object_new_boolean(autoDetectWanStatus));
#endif
		if(json_object_object_get(Jobj, "ExternalPortStart") != NULL)
			json_object_object_add(NatPortMappingJobj, "ExternalPort", json_object_new_int(externalport));
		if(json_object_object_get(Jobj, "ExternalPortEnd") != NULL)
			json_object_object_add(NatPortMappingJobj, "ExternalPortEndRange", json_object_new_int(externalportendrange));
		if(json_object_object_get(Jobj, "InternalPortStart") != NULL)
			json_object_object_add(NatPortMappingJobj, "InternalPort", json_object_new_int(internalport));
		if(json_object_object_get(Jobj, "InternalPortEnd") != NULL)
			json_object_object_add(NatPortMappingJobj, "X_ZYXEL_InternalPortEndRange", json_object_new_int(internalportendrange));
		if(json_object_object_get(Jobj, "SetOriginatingIP") != NULL)
			json_object_object_add(NatPortMappingJobj, "X_ZYXEL_SetOriginatingIP", json_object_new_boolean(setoriginatingip));
		if(json_object_object_get(Jobj, "OriginatingIpAddress") != NULL)
			json_object_object_add(NatPortMappingJobj, "OriginatingIpAddress", json_object_new_string(originatingipaddress));
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
		if(json_object_object_get(Jobj, "OriginatingSubnetMask") != NULL)
			json_object_object_add(NatPortMappingJobj, "OriginatingSubnetMask", json_object_new_string(originatingSubnetMask));
#endif
		if(json_object_object_get(Jobj, "Protocol") != NULL)
			json_object_object_add(NatPortMappingJobj, "Protocol", json_object_new_string(protocol));
		if(json_object_object_get(Jobj, "InternalClient") != NULL)
			json_object_object_add(NatPortMappingJobj, "InternalClient", json_object_new_string(internalclient));
		if(json_object_object_get(Jobj, "Description") != NULL)
			json_object_object_add(NatPortMappingJobj, "Description", json_object_new_string(description));
#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
		if(json_object_object_get(Jobj, "WANIP") != NULL)
			json_object_object_add(NatPortMappingJobj, "X_ZYXEL_WANIP", json_object_new_string(wanip));
#endif
#ifdef CONFIG_TAAAB_DSL_BINDING
            json_object_object_add(NatPortMappingJobj, "X_ZYXEL_REFERENCE", json_object_new_int(newreferindex));
#endif
		//check prot range
		externalport = json_object_get_int(json_object_object_get(NatPortMappingJobj, "ExternalPort"));
		externalportendrange = json_object_get_int(json_object_object_get(NatPortMappingJobj, "ExternalPortEndRange"));
		internalport = json_object_get_int(json_object_object_get(NatPortMappingJobj, "InternalPort"));
		internalportendrange = json_object_get_int(json_object_object_get(NatPortMappingJobj, "X_ZYXEL_InternalPortEndRange"));

		if(internalport > internalportendrange){
			if(replyMsg != NULL)
				ZOS_STRCPY_M(replyMsg, "Internal port range is invalid.");
			zcfgFeJsonObjFree(NatPortMappingJobj);
			return ZCFG_INVALID_PARAM_VALUE;
		}
		if(externalport > externalportendrange){
			if(replyMsg != NULL)
				ZOS_STRCPY_M(replyMsg, "External port range is invalid.");
			zcfgFeJsonObjFree(NatPortMappingJobj);
			return ZCFG_INVALID_PARAM_VALUE;
		}
	//declare for new port rule
	externalport = json_object_get_int(json_object_object_get(Jobj, "ExternalPortStart"));
	externalportendrange = json_object_get_int(json_object_object_get(Jobj, "ExternalPortEnd"));
	IID_INIT(Iid);
	if ((ret = zcfgFeDalNATPortRangeCheck(isDSL,isEdit,Jobj,index,externalport,externalportendrange,originatingipaddress,replyMsg)) != ZCFG_SUCCESS)
		return ret;
	
		ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_MAPPING, &objIid, NatPortMappingJobj, NULL);
		zcfgFeJsonObjFree(NatPortMappingJobj);
//TAAAB sync entry
#ifdef CONFIG_TAAAB_DSL_BINDING
			if(newreferindex != -1){
					IID_INIT(referobjIid);
					referobjIid.level = 1;
					referobjIid.idx[0] = newreferindex;
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &referobjIid, &NatPortMappingJobj);

				 		IID_INIT(ipIfaceIid);
		                while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS) {
							if(!strcmp(Jgets(ipIfaceObj, "X_TT_SrvName"),New_TAAAB_SrvName) && strcmp(Jgets(ipIfaceObj, "X_ZYXEL_SrvName"),New_SrvName) && strstr(Jgets(ipIfaceObj, "X_ZYXEL_SrvName"),"_ADSL"))
							{
									sprintf(newreferinterface, "IP.Interface.%u", ipIfaceIid.idx[0]);
									json_object_object_add(NatPortMappingJobj, "Interface", json_object_new_string(newreferinterface));
									json_object_object_add(NatPortMappingJobj, "X_ZYXEL_REFERENCE", json_object_new_int(-1));
									zcfgFeJsonObjFree(ipIfaceObj);
                                    break;
							}
							zcfgFeJsonObjFree(ipIfaceObj);
			            }

						
						enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));

						autoDetectWanStatus = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_AutoDetectWanStatus"));
						if(!strcmp(interface, "")){
							autoDetectWanStatus = true;
						}
						else{
							autoDetectWanStatus = false;
						}

						originatingipaddress = json_object_get_string(json_object_object_get(Jobj, "OriginatingIpAddress"));
						externalport = json_object_get_int(json_object_object_get(Jobj, "ExternalPortStart"));
						externalportendrange = json_object_get_int(json_object_object_get(Jobj, "ExternalPortEnd"));
						internalport = json_object_get_int(json_object_object_get(Jobj, "InternalPortStart"));
						internalportendrange = json_object_get_int(json_object_object_get(Jobj, "InternalPortEnd"));
						setoriginatingip = json_object_get_boolean(json_object_object_get(Jobj, "SetOriginatingIP"));
						protocol = json_object_get_string(json_object_object_get(Jobj, "Protocol"));
						internalclient = json_object_get_string(json_object_object_get(Jobj, "InternalClient"));
						ZOS_STRCPY_M(atmdescription,json_object_get_string(json_object_object_get(Jobj, "Description")));
						strcat(atmdescription,"_ATM");
#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
						wanip = json_object_get_string(json_object_object_get(Jobj, "WANIP"));
#endif
						if(json_object_object_get(Jobj, "Enable") != NULL)
							json_object_object_add(NatPortMappingJobj, "Enable", json_object_new_boolean(enable));
#ifndef ABOG_CUSTOMIZATION
						if(json_object_object_get(Jobj, "X_ZYXEL_AutoDetectWanStatus") != NULL)
							json_object_object_add(NatPortMappingJobj, "X_ZYXEL_AutoDetectWanStatus", json_object_new_boolean(autoDetectWanStatus));
#endif
						if(json_object_object_get(Jobj, "ExternalPortStart") != NULL)
							json_object_object_add(NatPortMappingJobj, "ExternalPort", json_object_new_int(externalport));
						if(json_object_object_get(Jobj, "ExternalPortEnd") != NULL)
							json_object_object_add(NatPortMappingJobj, "ExternalPortEndRange", json_object_new_int(externalportendrange));
						if(json_object_object_get(Jobj, "InternalPortStart") != NULL)
							json_object_object_add(NatPortMappingJobj, "InternalPort", json_object_new_int(internalport));
						if(json_object_object_get(Jobj, "InternalPortEnd") != NULL)
							json_object_object_add(NatPortMappingJobj, "X_ZYXEL_InternalPortEndRange", json_object_new_int(internalportendrange));
						if(json_object_object_get(Jobj, "SetOriginatingIP") != NULL)
							json_object_object_add(NatPortMappingJobj, "X_ZYXEL_SetOriginatingIP", json_object_new_boolean(setoriginatingip));
						if(json_object_object_get(Jobj, "OriginatingIpAddress") != NULL)
							json_object_object_add(NatPortMappingJobj, "OriginatingIpAddress", json_object_new_string(originatingipaddress));
						if(json_object_object_get(Jobj, "Protocol") != NULL)
							json_object_object_add(NatPortMappingJobj, "Protocol", json_object_new_string(protocol));
						if(json_object_object_get(Jobj, "InternalClient") != NULL)
							json_object_object_add(NatPortMappingJobj, "InternalClient", json_object_new_string(internalclient));
						if(json_object_object_get(Jobj, "Description") != NULL)
							json_object_object_add(NatPortMappingJobj, "Description", json_object_new_string(atmdescription));
#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
						if(json_object_object_get(Jobj, "WANIP") != NULL)
							json_object_object_add(NatPortMappingJobj, "X_ZYXEL_WANIP", json_object_new_string(wanip));
#endif
						ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_MAPPING, &referobjIid, NatPortMappingJobj, NULL);
						zcfgFeJsonObjFree(NatPortMappingJobj);
			}	

			if(newipIfaceObj != NULL)
			     zcfgFeJsonObjFree(newipIfaceObj);
		    if(oldipIfaceObj != NULL)
			     zcfgFeJsonObjFree(oldipIfaceObj);
#endif

	}

	return ret;
}

zcfgRet_t zcfgFeDalNatPortMappingDelete(struct json_object *Jobj, char *replyMsg)
{

	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *NatPortMappingJobj = NULL;
	objIndex_t objIid = {0};
	bool found = false;
#ifdef CONFIG_TAAAB_DSL_BINDING
		int newreferindex = -1;
#endif

	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "Index"));
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &objIid, &NatPortMappingJobj) == ZCFG_SUCCESS) {
		if(strncmp(json_object_get_string(json_object_object_get(NatPortMappingJobj, "Description")),"APP:",4)){
			found = true;
		}
#ifdef CONFIG_TAAAB_DSL_BINDING
        newreferindex = json_object_get_int(json_object_object_get(NatPortMappingJobj, "X_ZYXEL_REFERENCE"));
#endif
		zcfgFeJsonObjFree(NatPortMappingJobj);
	}

	if(found){
		ret = zcfgFeObjJsonDel(RDM_OID_NAT_PORT_MAPPING, &objIid, NULL);
	}
	else{
		if(replyMsg != NULL)
			ZOS_STRCPY_M(replyMsg, "Can't find rule");
		return ZCFG_NOT_FOUND;
	}

//TAAAB sync entry
#ifdef CONFIG_TAAAB_DSL_BINDING
		if(newreferindex != -1){
			IID_INIT(objIid);
			objIid.level = 1;
			objIid.idx[0] = newreferindex;
			ret = zcfgFeObjJsonDel(RDM_OID_NAT_PORT_MAPPING, &objIid, NULL);
		}
#endif

	return ret;

}

zcfgRet_t zcfgFeDalNatPortMappingGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *PortFwdJobj = NULL;
	struct json_object *indexObj = NULL;
	objIndex_t PortFwdIid = {0};
	int indexGet = -1;
#ifdef CONFIG_TAAAB_DSL_BINDING	
  	char SrvName[64];
	objIndex_t ipIfaceIid;
	struct json_object *ipIfaceObj = NULL;  
#endif

	if ((indexObj = json_object_object_get(Jobj, "Index")) != NULL)
	{
		indexGet = json_object_get_int(indexObj);
	}

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &PortFwdIid, &PortFwdJobj) == ZCFG_SUCCESS){
		if(strncmp(json_object_get_string(json_object_object_get(PortFwdJobj, "Description")),"APP:",4)){
#ifdef CONFIG_TAAAB_DSL_BINDING	   
 			IID_INIT(ipIfaceIid);
			ipIfaceIid.level = 1;
			sscanf(Jgets(PortFwdJobj,"Interface"), "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS)
			{
				ZOS_STRCPY_M(SrvName,Jgets(ipIfaceObj,"X_ZYXEL_SrvName"));
				zcfgFeJsonObjFree(ipIfaceObj);
			}          
			
           if(strstr(SrvName,"_ETHWAN") || strstr(SrvName,"_VDSL")){
#endif
			if (indexGet == -1 || PortFwdIid.idx[0] == indexGet)
			{
				paramJobj = json_object_new_object();
				json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "Enable")));
				json_object_object_add(paramJobj, "Protocol", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "Protocol")));
				json_object_object_add(paramJobj, "Description", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "Description")));
				json_object_object_add(paramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "Interface")));
				json_object_object_add(paramJobj, "ExternalPortStart", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "ExternalPort")));
				json_object_object_add(paramJobj, "ExternalPortEnd", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "ExternalPortEndRange")));
				json_object_object_add(paramJobj, "InternalPortStart", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "InternalPort")));
				json_object_object_add(paramJobj, "InternalPortEnd", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "X_ZYXEL_InternalPortEndRange")));
				json_object_object_add(paramJobj, "InternalClient", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "InternalClient")));
				json_object_object_add(paramJobj, "SetOriginatingIP", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "X_ZYXEL_SetOriginatingIP")));
				json_object_object_add(paramJobj, "OriginatingIpAddress", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "OriginatingIpAddress")));
#ifdef ZYXEL_NAT_PORTFORWARD_SOURCE_IP_SUBNETMASK
				json_object_object_add(paramJobj, "OriginatingSubnetMask", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "OriginatingSubnetMask")));
#endif
				json_object_object_add(paramJobj, "Index", json_object_new_int(PortFwdIid.idx[0]));
				json_object_object_add(paramJobj, "X_ZYXEL_AutoDetectWanStatus", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "X_ZYXEL_AutoDetectWanStatus")));
#ifdef CONFIG_TAAAB_DSL_BINDING
				json_object_object_add(paramJobj, "referenceIndex", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "X_ZYXEL_REFERENCE")));
#endif
#ifdef EAAAA_NAT_PORTFORWARD_CUSTOMIZATIONN
				json_object_object_add(paramJobj, "WANIP", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "X_ZYXEL_WANIP")));
#endif
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				json_object_object_add(paramJobj, "SetFromACS", JSON_OBJ_COPY(json_object_object_get(PortFwdJobj, "X_ZYXEL_SetFromACS")));                
#endif
				json_object_array_add(Jarray, paramJobj);
			}
#ifdef CONFIG_TAAAB_DSL_BINDING
           }
#endif
		}
		zcfgFeJsonObjFree(PortFwdJobj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalNatPortMapping(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	//arrary obj add/set, for TAAAB easymenu port mapping page
	if((json_object_get_type(Jobj) == json_type_array) && (!strcmp(method, "PUT") || !strcmp(method, "POST"))) {
		int idx = 0, arrLen = 0;
		struct json_object *obj = NULL;
		arrLen = json_object_array_length(Jobj);
		if(arrLen == 1 && !strcmp(method, "POST")) {
			ret = zcfgFeDalNatPortMappingAdd(Jobj, replyMsg);
		}		
		else {			
			for (idx = 0; idx < arrLen; idx++) {
				obj = json_object_array_get_idx(Jobj,idx);
				ret = zcfgFeDalNatPortMappingEdit(obj, replyMsg);
			}
		}
		return ret;
	}
	
	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalNatPortMappingEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalNatPortMappingAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalNatPortMappingDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDalNatPortMappingGet(Jobj, Jarray, replyMsg);
	}

	return ret;
}
