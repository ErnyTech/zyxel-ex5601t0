#include <ctype.h>
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_net.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

#define JSON_OBJ_COPY(json_object) json_tokener_parse(json_object_to_json_string(json_object))

dal_param_t VPNLite_param[]={
	{"Enable",				dalType_boolean,	0,	0,	NULL},
	{"Username",			dalType_string,		0,	64,	NULL},
	{"Password",			dalType_string,		0,	64,	NULL},	
	{"IPAddress",           dalType_v4Addr,		0,	0,	NULL},
	{"SubnetMask",          dalType_v4Mask,	    0,	0,	NULL},
	{"DHCP_MinAddress",		dalType_v4Addr,		0,	0,	NULL},
	{"DHCP_MaxAddress",		dalType_v4Addr,		0,	0,	NULL}
};

zcfgRet_t zcfgFeDalVPNLiteSet(json_object *Jobj, char *replyMsg){
	struct json_object *ipIfaceObj = NULL;
	struct json_object *VPNLitepppIfaceObj = NULL;
	struct json_object *VPNLitev4AddrObj = NULL;
	struct json_object *dhcpv4fwdObj = NULL;
	struct json_object *dhcpv4srvObj = NULL;
	const char *SrvName = NULL, *pppIface = NULL, *MaxDHCP = NULL, *MinDHCP = NULL;
	bool dchprelay = false;
	char currintf[32] = {0};
	
	objIndex_t ipIfaceIid, VPNLitepppIfaceIid, VPNLitev4AddrIid, dhcpv4fwdIid, dhcpv4srvIid;
	IID_INIT(ipIfaceIid);
	IID_INIT(VPNLitepppIfaceIid);
	IID_INIT(VPNLitev4AddrIid);
	IID_INIT(dhcpv4fwdIid);
	IID_INIT(dhcpv4srvIid);	
	
    //Set VPNLite Interface
    VPNLitepppIfaceIid.level = 1;
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS){
		SrvName = json_object_get_string(json_object_object_get(ipIfaceObj,"X_ZYXEL_SrvName"));
		if(SrvName != NULL && strstr(SrvName,"VPN-lite") != NULL){
			pppIface = json_object_get_string(json_object_object_get(ipIfaceObj,"LowerLayers"));
			sscanf(pppIface,"PPP.Interface.%hhu", &VPNLitepppIfaceIid.idx[0]);
			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PPP_IFACE, &VPNLitepppIfaceIid, &VPNLitepppIfaceObj) == ZCFG_SUCCESS){
				replaceParam(VPNLitepppIfaceObj, "Enable", Jobj, "Enable");
				replaceParam(VPNLitepppIfaceObj, "Username", Jobj, "Username");
				replaceParam(VPNLitepppIfaceObj, "Password", Jobj, "Password");	
				zcfgFeObjJsonSet(RDM_OID_PPP_IFACE, &VPNLitepppIfaceIid, VPNLitepppIfaceObj, NULL);
				zcfgFeJsonObjFree(VPNLitepppIfaceObj);
			}
		}
		if(!strncmp(json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_IfName")), "br0", 3)){
			VPNLitev4AddrIid.level = 2;
			VPNLitev4AddrIid.idx[0] = ipIfaceIid.idx[0];
			VPNLitev4AddrIid.idx[1] = 1;
			sprintf(currintf,"IP.Interface.%u",ipIfaceIid.idx[0]);
		}
		zcfgFeJsonObjFree(ipIfaceObj);
	}
    //Set LanIP
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &VPNLitev4AddrIid, &VPNLitev4AddrObj) == ZCFG_SUCCESS){
		replaceParam(VPNLitev4AddrObj, "IPAddress", Jobj, "IPAddress");
		replaceParam(VPNLitev4AddrObj, "SubnetMask", Jobj, "SubnetMask");
		zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &VPNLitev4AddrIid, VPNLitev4AddrObj, NULL);
		zcfgFeJsonObjFree(VPNLitev4AddrObj);		
	}
	
	MaxDHCP = json_object_get_string(json_object_object_get(Jobj, "DHCP_MaxAddress"));
	MinDHCP = json_object_get_string(json_object_object_get(Jobj, "DHCP_MinAddress"));
    if(MaxDHCP == NULL && MinDHCP == NULL)
		return ZCFG_SUCCESS;
	while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_RELAY_FWD, &dhcpv4fwdIid, &dhcpv4fwdObj) == ZCFG_SUCCESS){
		if(!strcmp(json_object_get_string(json_object_object_get(dhcpv4fwdObj, "Interface")),currintf)){
			dchprelay = true;
			zcfgFeJsonObjFree(dhcpv4fwdObj);
			break;
		}
		zcfgFeJsonObjFree(dhcpv4fwdObj);
	}
	while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_SRV_POOL, &dhcpv4srvIid, &dhcpv4srvObj) == ZCFG_SUCCESS){
		if(!strcmp(json_object_get_string(json_object_object_get(dhcpv4srvObj, "Interface")),currintf) && !dchprelay){
			replaceParam(dhcpv4srvObj, "MinAddress", Jobj, "DHCP_MinAddress");
			replaceParam(dhcpv4srvObj, "MaxAddress", Jobj, "DHCP_MaxAddress");
			zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL, &dhcpv4srvIid, dhcpv4srvObj, NULL);
			zcfgFeJsonObjFree(dhcpv4srvObj);
			break;
		}
		zcfgFeJsonObjFree(dhcpv4srvObj);	
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeDalVPNLiteGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	struct json_object *paramJobj = NULL;
	struct json_object *staticRouteJarray = NULL;
	struct json_object *staticRouteJobj = NULL;	
	struct json_object *ipIfaceObj = NULL;	
	struct json_object *pppIfaceObj = NULL;
	struct json_object *ipv4addrObj = NULL;
	struct json_object *staticRouteObj = NULL;
	const char *SrvName = NULL, *status = NULL, *pppIface = NULL;
	const char *Interface = NULL, *Alias = NULL, *DestIPAddress = NULL;
	//const char *Origin = NULL, *DestIPPrefix = NULL;
	bool findVPNLite = false, findLan = false, StaticRoute;
	char VPNLiteIpIface[32] = {0};
	int count = 0;
	objIndex_t ipIfaceIid, pppIfaceIid, ipv4addrIid, staticRouteIid;
	IID_INIT(ipIfaceIid);
	IID_INIT(pppIfaceIid);
	IID_INIT(ipv4addrIid);
	IID_INIT(staticRouteIid);
	pppIfaceIid.level = 1;
	paramJobj = json_object_new_object();
	staticRouteJarray = json_object_new_array();
	
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS){
	    //Get IP Interface for VPNLite		
		if(!findVPNLite){
			SrvName = json_object_get_string(json_object_object_get(ipIfaceObj,"X_ZYXEL_SrvName"));
			if(SrvName != NULL && strstr(SrvName,"VPN-lite") != NULL){
				json_object_object_add(paramJobj, "IfaceName", json_object_new_string(SrvName));
				sprintf(VPNLiteIpIface, "IP.Interface.%u", ipIfaceIid.idx[0]);
				json_object_object_add(paramJobj, "IfacePath", json_object_new_string(VPNLiteIpIface));
				pppIface = json_object_get_string(json_object_object_get(ipIfaceObj,"LowerLayers"));
				sscanf(pppIface,"PPP.Interface.%hhu", &pppIfaceIid.idx[0]);

				status = json_object_get_string(json_object_object_get(ipIfaceObj,"status"));
				if(status != NULL && strstr(status,"Up") != NULL){
					findVPNLite = true;
				}
			}
		}
		//Get LanIP Information	
		if(!findLan && !strncmp(json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_IfName")), "br0", 3)){
			findLan = true;
			ipv4addrIid.level = 2;
			ipv4addrIid.idx[0] = ipIfaceIid.idx[0];
			ipv4addrIid.idx[1] = 1;
			if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipv4addrIid, &ipv4addrObj) == ZCFG_SUCCESS){
				json_object_object_add(paramJobj, "IPAddress", JSON_OBJ_COPY(json_object_object_get(ipv4addrObj, "IPAddress")));
				json_object_object_add(paramJobj, "SubnetMask", JSON_OBJ_COPY(json_object_object_get(ipv4addrObj, "SubnetMask")));

				zcfgFeJsonObjFree(ipv4addrObj);
			}
		}
		zcfgFeJsonObjFree(ipIfaceObj);
		if(findVPNLite && findLan)
			break;
	}
    //Get PPP Information for VPNLite	
	if(zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIfaceObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Enable")));
		json_object_object_add(paramJobj, "Username", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Username")));
		json_object_object_add(paramJobj, "Password", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Password")));
		zcfgFeJsonObjFree(pppIfaceObj);		
	}
	//Get Static Route for VPNLite
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, &staticRouteIid, &staticRouteObj) == ZCFG_SUCCESS) {
		Interface = json_object_get_string(json_object_object_get(staticRouteObj, "Interface"));		
		Alias = json_object_get_string(json_object_object_get(staticRouteObj, "Alias"));
		DestIPAddress = json_object_get_string(json_object_object_get(staticRouteObj, "DestIPAddress"));
		StaticRoute = json_object_get_boolean(json_object_object_get(staticRouteObj, "StaticRoute"));
		if(StaticRoute && strcmp(Alias, "") && strcmp(DestIPAddress, "")){
			count++;
			if(strcmp(Interface, VPNLiteIpIface)){
				zcfgFeJsonObjFree(staticRouteObj);
				continue;
			}
			staticRouteJobj = json_object_new_object();
			json_object_object_add(staticRouteJobj, "Index", json_object_new_int(count));
			json_object_object_add(staticRouteJobj, "Name", json_object_new_string(Alias));
			json_object_object_add(staticRouteJobj, "DestIPAddress", json_object_new_string(DestIPAddress));
			json_object_object_add(staticRouteJobj, "StaticRoute", json_object_new_boolean(StaticRoute));
			json_object_object_add(staticRouteJobj, "ipver", json_object_new_string("IPv4"));
			json_object_object_add(staticRouteJobj, "GatewayIPAddress", JSON_OBJ_COPY(json_object_object_get(staticRouteObj, "GatewayIPAddress")));
			json_object_object_add(staticRouteJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(staticRouteObj, "Enable")));
			json_object_object_add(staticRouteJobj, "DestSubnetMask", JSON_OBJ_COPY(json_object_object_get(staticRouteObj, "DestSubnetMask")));
			json_object_object_add(staticRouteJobj, "Interface", json_object_new_string(Interface));
			json_object_object_add(staticRouteJobj, "idx", json_object_new_int(staticRouteIid.idx[1]));		
			json_object_array_add(staticRouteJarray, JSON_OBJ_COPY(staticRouteJobj));
		}
		zcfgFeJsonObjFree(staticRouteObj);
	}
	/*
	IID_INIT(staticRouteIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &staticRouteIid, &staticRouteObj) == ZCFG_SUCCESS) {
		Interface = json_object_get_string(json_object_object_get(staticRouteObj, "Interface"));
		Alias = json_object_get_string(json_object_object_get(staticRouteObj, "Alias"));
		DestIPPrefix = json_object_get_string(json_object_object_get(staticRouteObj, "DestIPPrefix"));
		Origin = json_object_get_string(json_object_object_get(staticRouteObj, "Origin"));
		if(!strcmp(Origin, "Static") && strcmp(Alias, "") && strcmp(DestIPPrefix, "")){
			count++;
			if(strcmp(Interface, VPNLiteIpIface)){
				zcfgFeJsonObjFree(staticRouteObj);
				continue;
			}
			staticRouteJobj = json_object_new_object();
			json_object_object_add(staticRouteJobj, "Index", json_object_new_int(count));
			json_object_object_add(staticRouteJobj, "Name", json_object_new_string(Alias));
			json_object_object_add(staticRouteJobj, "DestIPPrefix", json_object_new_string(DestIPPrefix));
			json_object_object_add(staticRouteJobj, "ipver", json_object_new_string("IPv6"));
			json_object_object_add(staticRouteJobj, "NextHop", JSON_OBJ_COPY(json_object_object_get(staticRouteObj, "NextHop")));
			json_object_object_add(staticRouteJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(staticRouteObj, "Enable")));
			json_object_object_add(staticRouteJobj, "Interface", json_object_new_string(Interface));
			json_object_object_add(staticRouteJobj, "idx", json_object_new_int(staticRouteIid.idx[1]));
			json_object_array_add(staticRouteJarray, JSON_OBJ_COPY(staticRouteJobj));
		}
		zcfgFeJsonObjFree(staticRouteObj);
	}
	*/
	json_object_object_add(paramJobj, "StaticRoute", JSON_OBJ_COPY(staticRouteJarray));
	zcfgFeJsonObjFree(staticRouteJarray);

	json_object_array_add(Jarray, JSON_OBJ_COPY(paramJobj));
	zcfgFeJsonObjFree(paramJobj);		
	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeDalVPNLite(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalVPNLiteSet(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalVPNLiteGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
