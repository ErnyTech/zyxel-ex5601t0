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

dal_param_t QUICK_START_param[]={		//RDM_OID_REMO_SRV
	{"Enable", 	 dalType_boolean,	0,	0,	NULL},
	{"X_ZYXEL_TimeZone", 		 dalType_string,	0,	0,	NULL},
	{"X_ZYXEL_Location", 		 dalType_string,	0,	0,	NULL},
	{"Interface", 		 dalType_string,	0,	0,	NULL},
	{"AutoShowQuickStart", 	 dalType_boolean,	0,	0,	NULL},
	{"DNSServer", 		 dalType_string,	0,	0,	NULL},
	{"X_ZYXEL_Type", 		 dalType_string,	0,	0,	NULL},
	{"IPAddress", 		 dalType_string,	0,	0,	NULL},
	{"SubnetMask", 		 dalType_string,	0,	0,	NULL},
	{"AddressingType", 		 dalType_string,	0,	0,	NULL},
	{"StaticRoute", 	 dalType_boolean,	0,	0,	NULL},
	{"GatewayIPAddress", 		 dalType_string,	0,	0,	NULL},
	{"ForwardingPolicy", 	 dalType_int,	0,	0,	NULL},
	{"X_ZYXEL_ActiveDefaultGateway", 		 dalType_string,	0,	0,	NULL},
	{"Username", 		 dalType_string,	0,	0,	NULL},
	{"Password", 		 dalType_string,	0,	0,	NULL},
	{"X_ZYXEL_LocalIPAddress", 		 dalType_string,	0,	0,	NULL},

	{NULL,					0,	0,	0,	NULL},

};

zcfgRet_t Time(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *timeObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t timeIid = {0};
	boolean enable = false;
	const char *timezone = NULL;
	const char *location = NULL;

	IID_INIT(timeIid);
	ret = zcfgFeObjJsonGet(RDM_OID_TIME, &timeIid, &timeObj);
	reqObject = json_object_array_get_idx(Jobj, 0);
	enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
	timezone = json_object_get_string(json_object_object_get(reqObject, "X_ZYXEL_TimeZone"));
	location = json_object_get_string(json_object_object_get(reqObject, "X_ZYXEL_Location"));

	json_object_object_add(timeObj, "Enable", json_object_new_boolean(enable));
	json_object_object_add(timeObj, "X_ZYXEL_TimeZone", json_object_new_string(timezone));
	json_object_object_add(timeObj, "X_ZYXEL_Location", json_object_new_string(location));
	
	ret = zcfgFeObjJsonSet(RDM_OID_TIME, &timeIid, timeObj, NULL);
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(timeObj);
	return ret;
}

zcfgRet_t DNS(struct json_object *Jobj, const char *Action){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dnsObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t dnsIid = {0};
	boolean enable = false;
	boolean found = true;
	const char *interface = NULL;
	const char *dnsserver = NULL;
	const char *origdnsserver = NULL;
	const char *type = NULL;
	const char *origInterface = NULL;
	int index = 0;
	
	IID_INIT(dnsIid);
	reqObject = json_object_array_get_idx(Jobj, 1);
	if(reqObject!=NULL){
		interface = json_object_get_string(json_object_object_get(reqObject, "Interface"));

		if(!strcmp(Action,"POST")){
			while((ret = zcfgFeObjJsonGetNext(RDM_OID_DNS_CLIENT_SRV, &dnsIid, &dnsObj)) == ZCFG_SUCCESS){
				origInterface = json_object_get_string(json_object_object_get(dnsObj, "Interface"));
				origdnsserver = json_object_get_string(json_object_object_get(dnsObj, "DNSServer")); 
				if(!strcmp(interface,origInterface) && (strchr(origdnsserver, ':') == NULL)){
					ret = zcfgFeObjJsonDel(RDM_OID_DNS_CLIENT_SRV, &dnsIid, NULL);
					ret = zcfgFeObjJsonAdd(RDM_OID_DNS_CLIENT_SRV, &dnsIid, NULL);
					if(ret == ZCFG_SUCCESS){
						ret = zcfgFeObjJsonGet(RDM_OID_DNS_CLIENT_SRV, &dnsIid, &dnsObj);
						found = false;
						break;
					}
				}
			}
			if(found){
				IID_INIT(dnsIid);
			ret = zcfgFeObjJsonAdd(RDM_OID_DNS_CLIENT_SRV, &dnsIid, NULL);
			if(ret == ZCFG_SUCCESS){
				ret = zcfgFeObjJsonGet(RDM_OID_DNS_CLIENT_SRV, &dnsIid, &dnsObj);
			}
		}
		}
		else if(!strcmp(Action,"PUT")){
			index = json_object_get_int(json_object_object_get(reqObject, "Index"));
			dnsIid.idx[0] = index;
			dnsIid.level = 1;
			ret = zcfgFeObjJsonGet(RDM_OID_DNS_CLIENT_SRV, &dnsIid, &dnsObj);
			}

		if(ret == ZCFG_SUCCESS){
			enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
			dnsserver = json_object_get_string(json_object_object_get(reqObject, "DNSServer"));
			type = json_object_get_string(json_object_object_get(reqObject, "X_ZYXEL_Type"));
			json_object_object_add(dnsObj, "Enable", json_object_new_boolean(enable));
			json_object_object_add(dnsObj, "Interface", json_object_new_string(interface));
			json_object_object_add(dnsObj, "DNSServer", json_object_new_string(dnsserver));

			if(type != NULL){
				json_object_object_add(dnsObj, "X_ZYXEL_Type", json_object_new_string(type));
			}
			ret = zcfgFeObjJsonSet(RDM_OID_DNS_CLIENT_SRV, &dnsIid, dnsObj, NULL);
		}
	}
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(dnsObj);
	return ret;
}


zcfgRet_t IPIFACE(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ipifaceObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t ipifaceIid = {0};
	boolean enable = false;
	const char *ipaddress = NULL;
	const char *subnetmask = NULL;
    /* __ZyXEL__, PingLin, 20170523, #36685 [Quick Start] From Quick Start to configure static IP address manually, but broadband webpage did not match. */
    const char *addressingtype = NULL;
	int index0 = 0;
	int index1 = 0;
	const char *action = NULL;
	
	IID_INIT(ipifaceIid);
	reqObject = json_object_array_get_idx(Jobj, 2);
	if(reqObject!=NULL){		
		index0 = json_object_get_int(json_object_object_get(reqObject, "Index0"));
		index1 = json_object_get_int(json_object_object_get(reqObject, "Index1"));
		action = json_object_get_string(json_object_object_get(reqObject, "Action"));

		if(!strcmp(action,"POST")){
			ipifaceIid.idx[0] = index0;
			ipifaceIid.level = 1;
			ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE_V4_ADDR, &ipifaceIid, NULL);
			if(ret == ZCFG_SUCCESS){
				ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipifaceIid, &ipifaceObj);
			}
		}
		else if(!strcmp(action,"PUT")){
			ipifaceIid.idx[0] = index0;
			ipifaceIid.idx[1] = index1;
			ipifaceIid.level = 2;
			ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipifaceIid, &ipifaceObj);
		}
		if(ret == ZCFG_SUCCESS){
			enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
			ipaddress = json_object_get_string(json_object_object_get(reqObject, "IPAddress"));
			subnetmask = json_object_get_string(json_object_object_get(reqObject, "SubnetMask"));
            /* __ZyXEL__, PingLin, 20170523, #36685 [Quick Start] From Quick Start to configure static IP address manually, but broadband webpage did not match. */
            addressingtype = json_object_get_string(json_object_object_get(reqObject, "AddressingType"));
			
			json_object_object_add(ipifaceObj, "Enable", json_object_new_boolean(enable));
			json_object_object_add(ipifaceObj, "IPAddress", json_object_new_string(ipaddress));
			json_object_object_add(ipifaceObj, "SubnetMask", json_object_new_string(subnetmask));
            /* __ZyXEL__, PingLin, 20170523, #36685 [Quick Start] From Quick Start to configure static IP address manually, but broadband webpage did not match. */
            json_object_object_add(ipifaceObj, "AddressingType", json_object_new_string(addressingtype));

			ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &ipifaceIid, ipifaceObj, NULL);
		}
	}
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(ipifaceObj);
	return ret;
}


zcfgRet_t RoutingFwd(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *fwdObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t fwdIid = {0};
	boolean enable = false;
	boolean staticroute = false;
	const char *gatewayipaddress = NULL;
	const char *interface = NULL;
	const char *origInterface = NULL;
	int forwardingpolicy = 0;

	IID_INIT(fwdIid);
	reqObject = json_object_array_get_idx(Jobj, 3);
	if(reqObject!=NULL){
		interface = json_object_get_string(json_object_object_get(reqObject, "Interface"));	
		while((ret = zcfgFeObjJsonGetNext(RDM_OID_ROUTING_ROUTER_V4_FWD, &fwdIid, &fwdObj)) == ZCFG_SUCCESS){
			origInterface = json_object_get_string(json_object_object_get(fwdObj, "Interface"));
			if(!strcmp(origInterface,"")){
				ret = zcfgFeObjJsonDel(RDM_OID_ROUTING_ROUTER_V4_FWD, &fwdIid, NULL);
			}
			if(!strcmp(interface,origInterface)){
				ret = zcfgFeObjJsonDel(RDM_OID_ROUTING_ROUTER_V4_FWD, &fwdIid, NULL);
				break;
			}
		}
		IID_INIT(fwdIid);
		fwdIid.idx[0] = 1;
		fwdIid.level = 1;
	
		ret = zcfgFeObjJsonAdd(RDM_OID_ROUTING_ROUTER_V4_FWD, &fwdIid, NULL);
			if(ret == ZCFG_SUCCESS){
				ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER_V4_FWD, &fwdIid, &fwdObj);
			}
		if(ret == ZCFG_SUCCESS){
			enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
			staticroute = json_object_get_boolean(json_object_object_get(reqObject, "StaticRoute"));
			gatewayipaddress = json_object_get_string(json_object_object_get(reqObject, "GatewayIPAddress"));
			forwardingpolicy = json_object_get_int(json_object_object_get(reqObject, "ForwardingPolicy"));
			json_object_object_add(fwdObj, "Enable", json_object_new_boolean(enable));	
			json_object_object_add(fwdObj, "StaticRoute", json_object_new_boolean(staticroute));
			if(gatewayipaddress!=NULL){
				json_object_object_add(fwdObj, "GatewayIPAddress", json_object_new_string(gatewayipaddress));
			}
			json_object_object_add(fwdObj, "Interface", json_object_new_string(interface));
			json_object_object_add(fwdObj, "ForwardingPolicy", json_object_new_int(forwardingpolicy));
			ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V4_FWD, &fwdIid, fwdObj, NULL);
		}
	}
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(fwdObj); 
	return ret;
}

zcfgRet_t Routing(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *routingObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t routingIid = {0};
	boolean enable = false;
	const char *activedefaultgateway = NULL;
	
	IID_INIT(routingIid);
	reqObject = json_object_array_get_idx(Jobj, 4);
	if(reqObject!=NULL){
		routingIid.idx[0] = 1;
		routingIid.level = 1;
		ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER, &routingIid, &routingObj);
		if(ret == ZCFG_SUCCESS){
			enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
			activedefaultgateway = json_object_get_string(json_object_object_get(reqObject, "X_ZYXEL_ActiveDefaultGateway"));
			json_object_object_add(routingObj, "Enable", json_object_new_boolean(enable));
			json_object_object_add(routingObj, "X_ZYXEL_ActiveDefaultGateway", json_object_new_string(activedefaultgateway));
			ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER, &routingIid, routingObj, NULL);
		}
	}
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(routingObj);
	return ret;
}

zcfgRet_t DHCPV4Client(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dhcpv4Obj = NULL;
	struct json_object *reqObject = NULL;
	struct json_object *fwdObj = NULL;
	struct json_object *ipifaceObj = NULL;
	objIndex_t dhcpv4Iid = {0};
	objIndex_t fwdIid = {0};
	objIndex_t ipifaceIid = {0};
	boolean enable = false;
	const char *interface = NULL;
	const char *origInterface = NULL;
	
	IID_INIT(dhcpv4Iid);
	IID_INIT(fwdIid);
	IID_INIT(ipifaceIid);
	reqObject = json_object_array_get_idx(Jobj, 5);
	if(reqObject!=NULL){
		enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
		if(enable){
			ret = zcfgFeObjJsonAdd(RDM_OID_DHCPV4_CLIENT, &dhcpv4Iid, NULL);
			if(ret == ZCFG_SUCCESS){
				ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_CLIENT, &dhcpv4Iid, &dhcpv4Obj);
				enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
				interface = json_object_get_string(json_object_object_get(reqObject, "Interface"));
				json_object_object_add(dhcpv4Obj, "Enable", json_object_new_boolean(enable));
				json_object_object_add(dhcpv4Obj, "Interface", json_object_new_string(interface));
				ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_CLIENT, &dhcpv4Iid, dhcpv4Obj, NULL);
			}
			while((ret = zcfgFeObjJsonGetNext(RDM_OID_ROUTING_ROUTER_V4_FWD, &fwdIid, &fwdObj)) == ZCFG_SUCCESS){
				origInterface = json_object_get_string(json_object_object_get(fwdObj, "Interface"));
				if(!strcmp(interface,origInterface)){
					json_object_object_add(fwdObj, "Interface", json_object_new_string(""));
					ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V4_FWD, &fwdIid, fwdObj, NULL);
					break;
				}

			}
			ipifaceIid.level = 2;
			sscanf(interface, "IP.Interface.%hhu", &ipifaceIid.idx[0]);
			ipifaceIid.idx[1] = 1;
			ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipifaceIid, &ipifaceObj);
			if(ret == ZCFG_SUCCESS){
				json_object_object_add(ipifaceObj, "Enable", json_object_new_boolean(false));
				ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &ipifaceIid, ipifaceObj, NULL);
			}
		}
	}
	zcfgFeJsonObjFree(fwdObj);
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(dhcpv4Obj);
	zcfgFeJsonObjFree(ipifaceObj);
	return ret;
}

zcfgRet_t qkPPP(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *qkPPPObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t qkPPPIid = {0};
	boolean enable = false;
	int index = 0;
	
	IID_INIT(qkPPPIid);
	reqObject = json_object_array_get_idx(Jobj, 2);
	if(reqObject!=NULL){
		index = json_object_get_int(json_object_object_get(reqObject, "Index"));
		if(index != 0){
		qkPPPIid.idx[0] = index;
		qkPPPIid.level = 1;
		ret = zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &qkPPPIid, &qkPPPObj);
		}
		else{
			ret = zcfgFeObjJsonAdd(RDM_OID_PPP_IFACE, &qkPPPIid, NULL);
			if(ret == ZCFG_SUCCESS){
				ret = zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &qkPPPIid, &qkPPPObj);
			}
		}
		json_object_object_add(qkPPPObj, "Enable", json_object_new_boolean(enable));
		ret = zcfgFeObjJsonBlockedSet(RDM_OID_PPP_IFACE, &qkPPPIid, qkPPPObj, NULL);
	}
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(qkPPPObj);
	return ret;
}

zcfgRet_t qkEnable(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *qkEnableObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t qkEnableIid = {0};
	boolean enable = false;
	const char *username = NULL;
	const char *password = NULL;
	const char *localipaddress = NULL;
	int index = 0;
	
	IID_INIT(qkEnableIid);
	reqObject = json_object_array_get_idx(Jobj, 3);
	if(reqObject!=NULL){
		enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
		index = json_object_get_int(json_object_object_get(reqObject, "Index"));
		qkEnableIid.idx[0] = index;
		qkEnableIid.level = 1;
		ret = zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &qkEnableIid, &qkEnableObj);
		
		username = json_object_get_string(json_object_object_get(reqObject, "Username"));
		password = json_object_get_string(json_object_object_get(reqObject, "Password"));
		localipaddress = json_object_get_string(json_object_object_get(reqObject, "X_ZYXEL_LocalIPAddress"));
		json_object_object_add(qkEnableObj, "Enable", json_object_new_boolean(enable));
		json_object_object_add(qkEnableObj, "Username", json_object_new_string(username));
		json_object_object_add(qkEnableObj, "Password", json_object_new_string(password));
		json_object_object_add(qkEnableObj, "X_ZYXEL_LocalIPAddress", json_object_new_string(localipaddress));
		ret = zcfgFeObjJsonSet(RDM_OID_PPP_IFACE, &qkEnableIid, qkEnableObj, NULL);
	}
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(qkEnableObj);
	return ret;
}

zcfgRet_t WifiRadio(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *wifiradioObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t wifiradioIid = {0};
	boolean enable = false;
	
	IID_INIT(wifiradioIid);
	reqObject = json_object_array_get_idx(Jobj, 6);
	if(reqObject!=NULL){
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
		ret = zcfgFeObjJsonGetNext(RDM_OID_WIFI_RADIO, &wifiradioIid, &wifiradioObj);
		enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
		json_object_object_add(wifiradioObj, "Enable", json_object_new_boolean(enable));
		ret = zcfgFeObjJsonSet(RDM_OID_WIFI_RADIO, &wifiradioIid, wifiradioObj, NULL);
#endif
	}
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(wifiradioObj);
	return ret;
}

zcfgRet_t WifiSSID(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *wifissidObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t wifissidIid = {0};
	boolean enable = false;
	int index = 0;
	
	IID_INIT(wifissidIid);
	reqObject = json_object_array_get_idx(Jobj, 7);
	index = json_object_get_int(json_object_object_get(reqObject, "Index"));
	wifissidIid.idx[0] = index;
	wifissidIid.level = 1;
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	ret = zcfgFeObjJsonGet(RDM_OID_WIFI_SSID, &wifissidIid, &wifissidObj);
	enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
	json_object_object_add(wifissidObj, "Enable", json_object_new_boolean(enable));
	ret = zcfgFeObjJsonSet(RDM_OID_WIFI_SSID, &wifissidIid, wifissidObj, NULL);
#endif
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(wifissidObj);
	return ret;
	
}

zcfgRet_t AutoShowQuickStart(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *autoObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t autoIid = {0};
	boolean enable = false;
	int index0 = 0;
	int index1 = 0;
	
	IID_INIT(autoIid);
	reqObject = json_object_array_get_idx(Jobj, 8);
	index0 = json_object_get_int(json_object_object_get(reqObject, "Index0"));
	index1 = json_object_get_int(json_object_object_get(reqObject, "Index1"));
	autoIid.idx[0] = index0;
	autoIid.idx[1] = index1;
	autoIid.level = 2;
	ret = zcfgFeObjJsonGet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &autoIid, &autoObj);
	enable = json_object_get_boolean(json_object_object_get(reqObject, "AutoShowQuickStart"));
	json_object_object_add(autoObj, "AutoShowQuickStart", json_object_new_boolean(enable));
	ret = zcfgFeObjJsonSet(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &autoIid, autoObj, NULL);
	zcfgFeJsonObjFree(reqObject);
	zcfgFeJsonObjFree(autoObj);
	return ret;
	
}

zcfgRet_t EthVlanTerm(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ethvlantermObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t ethvlantermIid = {0};
	boolean enable = false;
	boolean vlanenable = false;
	int vlanid = 0;
	int vlanpriority = 0;
	int index = 0;

	IID_INIT(ethvlantermIid);
	reqObject = json_object_array_get_idx(Jobj, 10);
	if(reqObject!=NULL){
		index = json_object_get_boolean(json_object_object_get(reqObject, "Index"));
		vlanenable = json_object_get_boolean(json_object_object_get(reqObject, "X_ZYXEL_VLANEnable"));
		if(vlanenable){
			enable = json_object_get_boolean(json_object_object_get(reqObject, "Enable"));
			vlanid = json_object_get_int(json_object_object_get(reqObject, "VLANID"));
			vlanpriority = json_object_get_int(json_object_object_get(reqObject, "X_ZYXEL_VLANPriority"));
			
			ethvlantermIid.idx[0] = index;
			ethvlantermIid.level = 1;
			ret = zcfgFeObjJsonGet(RDM_OID_ETH_VLAN_TERM, &ethvlantermIid, &ethvlantermObj);
			if(ret == ZCFG_SUCCESS){
				json_object_object_add(ethvlantermObj, "Enable", json_object_new_boolean(enable));
				json_object_object_add(ethvlantermObj, "X_ZYXEL_VLANEnable", json_object_new_boolean(vlanenable));
				json_object_object_add(ethvlantermObj, "VLANID", json_object_new_int(vlanid));
				json_object_object_add(ethvlantermObj, "X_ZYXEL_VLANPriority", json_object_new_int(vlanpriority));
				ret = zcfgFeObjJsonSet(RDM_OID_ETH_VLAN_TERM, &ethvlantermIid, ethvlantermObj, NULL);
			}
		}
	}
	zcfgFeJsonObjFree(ethvlantermObj);
	return ret;
}

zcfgRet_t DHCPV4ClientDelete(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dhcpv4deleteObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t dhcpv4deleteIid = {0};
	int index = 0;

	IID_INIT(dhcpv4deleteIid);
	reqObject = json_object_array_get_idx(Jobj, 9);
	if(reqObject!=NULL){
		index = json_object_get_int(json_object_object_get(reqObject, "DeleteIdx"));
		dhcpv4deleteIid.idx[0] = index;
		dhcpv4deleteIid.level = 1;
		ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_CLIENT, &dhcpv4deleteIid, &dhcpv4deleteObj);
		ret = zcfgFeObjJsonDel(RDM_OID_DHCPV4_CLIENT, &dhcpv4deleteIid, NULL);
		zcfgFeJsonObjFree(reqObject);
		zcfgFeJsonObjFree(dhcpv4deleteObj);
		return ret;
	}
	return ret;
}

zcfgRet_t DNSDelete(struct json_object *Jobj, const char *iptype){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dnsdeleteObj = NULL;
	struct json_object *reqObject = NULL;
	struct json_object *reqObject1 = NULL;
	objIndex_t dnsdeleteIid = {0};
	const char *originterface = NULL;
	const char *interface = NULL;
	// const char *type = NULL;
	int index = 0;
	IID_INIT(dnsdeleteIid);
	reqObject = json_object_array_get_idx(Jobj, 9);
	if(reqObject!=NULL){
		if(!strcmp(iptype,"PPP dynamic")){
			reqObject1 = json_object_array_get_idx(Jobj, 1);
			interface = json_object_get_string(json_object_object_get(reqObject1, "Interface"));
			// type = json_object_get_string(json_object_object_get(reqObject1, "X_ZYXEL_Type"));
			while((ret = zcfgFeObjJsonGetNext(RDM_OID_DNS_CLIENT_SRV, &dnsdeleteIid, &dnsdeleteObj)) == ZCFG_SUCCESS){
				originterface = json_object_get_string(json_object_object_get(dnsdeleteObj, "Interface"));
				if(!strcmp(interface,originterface)){
					ret = zcfgFeObjJsonDel(RDM_OID_DNS_CLIENT_SRV, &dnsdeleteIid, NULL);
				}
			}
		}
		else{
			index = json_object_get_int(json_object_object_get(reqObject, "DeleteIdx"));
			dnsdeleteIid.idx[0] = index;
			dnsdeleteIid.level = 1;
			ret = zcfgFeObjJsonGet(RDM_OID_DNS_CLIENT_SRV, &dnsdeleteIid, &dnsdeleteObj);
			ret = zcfgFeObjJsonDel(RDM_OID_DNS_CLIENT_SRV, &dnsdeleteIid, NULL);
		}
		zcfgFeJsonObjFree(reqObject);
		zcfgFeJsonObjFree(reqObject1);
		zcfgFeJsonObjFree(dnsdeleteObj);
		return ret;
	}
	return ret;
}

zcfgRet_t zcfgFeDal_QuickStart_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS; 
	struct json_object *reqObject = NULL;
	const char *iptype = NULL;
	const char *protocol = NULL;
	int index = 0;
	reqObject = json_object_array_get_idx(Jobj,9);
	if(reqObject!=NULL){
		protocol = json_object_get_string(json_object_object_get(reqObject,"Protocol"));
		iptype = json_object_get_string(json_object_object_get(reqObject,"ipType"));
		if((!strcmp(protocol,"IPoE")) || (!strcmp(protocol,"IPoA"))){
			if(!strcmp(iptype,"DHCP to static IP")){
				DHCPV4ClientDelete(Jobj);
				DNSDelete(Jobj,iptype);
				DNS(Jobj,"POST");
				IPIFACE(Jobj);
				RoutingFwd(Jobj);
				Routing(Jobj);
			}
			else if(!strcmp(iptype, "static IP to static IP")){
				DNS(Jobj,"PUT");
				IPIFACE(Jobj);
				RoutingFwd(Jobj);
				Routing(Jobj);				
			}
			else if(!strcmp(iptype, "static IP to DHCP")){
				DNSDelete(Jobj,iptype);
				DHCPV4Client(Jobj);
			}
		}
		else if(!strcmp(protocol,"PPP")){
			if(!strcmp(iptype,"static DNS")){
				index = json_object_get_int(json_object_object_get(reqObject, "DeleteIdx"));
				if(index != 0){
					DNSDelete(Jobj,iptype);
					DNS(Jobj,"POST");
				}
				DNS(Jobj,"PUT");
			}
			else if(!strcmp(iptype,"dynamic DNS")){
				DNSDelete(Jobj,iptype);
			}
			if(!strcmp(iptype,"PPP static"))
				DNS(Jobj,"POST");
			else if(!strcmp(iptype,"PPP dynamic"))
				DNSDelete(Jobj,iptype);
			qkPPP(Jobj);
			qkEnable(Jobj);
		}
#ifdef ZyXEL_WEB_GUI_SHOW_PON
		if(!strcmp(protocol,"IPoE") || !strcmp(protocol,"PPPoE")){
			EthVlanTerm(Jobj);
		}
#endif
	}
	Time(Jobj);
	WifiRadio(Jobj);
	WifiSSID(Jobj);
	AutoShowQuickStart(Jobj);
	zcfgFeJsonObjFree(reqObject);
	return ret;

}

zcfgRet_t zcfgFeDalQuickStart(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_QuickStart_Edit(Jobj, NULL);
		}
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

