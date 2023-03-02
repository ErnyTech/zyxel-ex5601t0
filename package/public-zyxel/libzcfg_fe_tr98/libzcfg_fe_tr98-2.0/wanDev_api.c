#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcmd_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_msg.h"
#include "zcfg_net.h"
#include "zcfg_debug.h"
#include "zcfg_fe_tr98.h"
#include "zos_api.h"

int portMappingNumGet(char *ipIface)
{
	int num = 0;
    objIndex_t objIid;
	rdm_NatPortMapping_t *portMapObj = NULL;

    IID_INIT(objIid);
    while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &objIid, (void **)&portMapObj) == ZCFG_SUCCESS) {
        if(!strcmp(portMapObj->Interface, ipIface)) {
			num++;
        }
        zcfgFeObjStructFree(portMapObj);
    }

    return num;
}

zcfgRet_t nextHopInfoSet(char *ipIface, struct json_object *tr98Jobj)
{
	objIndex_t ipv6FwdIid;
	rdm_RoutingRouterV6Fwd_t *v6FwdObj = NULL;

	IID_INIT(ipv6FwdIid);
	while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &ipv6FwdIid, (void **)&v6FwdObj) == ZCFG_SUCCESS) {
		if(strstr(ipIface, v6FwdObj->Interface)) {
			json_object_object_add(tr98Jobj, "NextHop", json_object_new_string(v6FwdObj->NextHop));
			zcfgFeObjStructFree(v6FwdObj);
			return ZCFG_SUCCESS;
		}
		zcfgFeObjStructFree(v6FwdObj);
	}

	/*Default value*/
	json_object_object_add(tr98Jobj, "NextHop", json_object_new_string(""));

	return ZCFG_SUCCESS;
}

zcfgRet_t natInfoSet(char *ipIface, struct json_object *tr98Jobj)
{
	objIndex_t natIntfIid;
	rdm_NatIntfSetting_t *natIntfObj = NULL;

//	printf("%s : Enter\n", __FUNCTION__);
	
	IID_INIT(natIntfIid);
	while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_NAT_INTF_SETTING, &natIntfIid, (void **)&natIntfObj) == ZCFG_SUCCESS) {
		if(!strcmp(natIntfObj->Interface, ipIface)) {
			json_object_object_add(tr98Jobj, "NATEnabled", json_object_new_boolean(natIntfObj->Enable));
			json_object_object_add(tr98Jobj, "X_ZYXEL_FullConeEnabled", json_object_new_boolean(natIntfObj->X_ZYXEL_FullConeEnabled));			
			zcfgFeObjStructFree(natIntfObj);
			return ZCFG_SUCCESS;
		}
		zcfgFeObjStructFree(natIntfObj);
	}

	/*Default value*/
	json_object_object_add(tr98Jobj, "NATEnabled", json_object_new_boolean(false));
	json_object_object_add(tr98Jobj, "X_ZYXEL_FullConeEnabled", json_object_new_boolean(false));

	return ZCFG_SUCCESS;
}

zcfgRet_t retrieveJobjWithIpIface(const char *ipIface, zcfg_offset_t oid, objIndex_t *objIid, struct json_object **jobj)
{
	if(!ipIface || !objIid || !jobj)
		return ZCFG_NO_SUCH_OBJECT;

	IID_INIT(*objIid);
	while(zcfgFeObjJsonGetNext(oid, objIid, (struct json_object **)jobj) == ZCFG_SUCCESS) {
		const char *intf = json_object_get_string(json_object_object_get(*jobj, "Interface"));
		if(!strcmp(ipIface, intf))
			return ZCFG_SUCCESS;

		json_object_put(*jobj);
		*jobj = NULL;
	}

	return ZCFG_NO_SUCH_OBJECT;
}

zcfgRet_t natIntfGetJobj(const char *ipIface, objIndex_t *natIntfIid, struct json_object **natIntfJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *natObj = NULL, *emptyObj = NULL;
	objIndex_t natIid, emptyIid;

	if(!ipIface || !natIntfIid || !natIntfJobj) return ZCFG_INTERNAL_ERROR;

	IID_INIT(natIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_NAT_INTF_SETTING, &natIid, (struct json_object **)&natObj) == ZCFG_SUCCESS) {
		const char *natIntfName = json_object_get_string(json_object_object_get(natObj, "Interface"));
		if(!strcmp(natIntfName, "")) {
 			if(!emptyObj) {
				emptyObj = natObj;
				natObj = NULL;
				memcpy(&emptyIid, &natIid, sizeof(objIndex_t));
				continue;
			}
		}
		else if(!strcmp(natIntfName, ipIface)) {
			*natIntfJobj = natObj;
			memcpy(natIntfIid, &natIid, sizeof(objIndex_t));
			if(emptyObj) {
				json_object_put(emptyObj);
			}
			printf("%s: Nat interface retrieved\n", __FUNCTION__);
			return ZCFG_SUCCESS;
		}
		json_object_put(natObj);
	}

	if(emptyObj) {
		*natIntfJobj = emptyObj;
		memcpy(natIntfIid, &emptyIid, sizeof(objIndex_t));
		json_object_object_add(emptyObj, "Interface", json_object_new_string(ipIface));
		return ZCFG_SUCCESS;
	}

	printf("%s: There is not empty nat interface! Add\n", __FUNCTION__);

	IID_INIT(emptyIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_NAT_INTF_SETTING, &emptyIid, NULL)) != ZCFG_SUCCESS) {
		printf("%s: Add nat interface setting fail!\n", __FUNCTION__);
		return ret;
	}

	natObj = NULL;
	if(zcfgFeObjJsonGet(RDM_OID_NAT_INTF_SETTING, &emptyIid, (struct json_object **)&natObj) == ZCFG_SUCCESS) {
		*natIntfJobj = natObj;
		memcpy(natIntfIid, &emptyIid, sizeof(objIndex_t));
		json_object_object_add(natObj, "Interface", json_object_new_string(ipIface));
		return ZCFG_SUCCESS;
	}

	printf("%s: Retrieve nat interface setting fail!\n", __FUNCTION__);

	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t defaultRtGetObj(const char *ipIface, objIndex_t *rtIpIid, struct json_object **rtIpObj, const char *addrType)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *rtObj = NULL, *emptyObj = NULL;
	objIndex_t rtIid, emptyIid;
	struct json_object *rtIntfObj = NULL;
	objIndex_t rtIntfIid;
	int hasStaticRt = 0, hasDhcpRt = 0;

	if(!ipIface || !rtIpIid || !rtIpObj) return ZCFG_INTERNAL_ERROR;

	*rtIpObj = NULL;


	IID_INIT(rtIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ROUTING_ROUTER_V4_FWD, &rtIid, (struct json_object **)&rtObj) == ZCFG_SUCCESS) {
		const char *rtIntfName = json_object_get_string(json_object_object_get(rtObj, "Interface"));
		const char *dstIP = json_object_get_string(json_object_object_get(rtObj, "DestIPAddress"));
		const char *dstSubnetMask = json_object_get_string(json_object_object_get(rtObj, "DestSubnetMask"));
		const char *routAdder = json_object_get_string(json_object_object_get(rtObj, "X_ZYXEL_RoutAdder"));
		const char *rtIP = json_object_get_string(json_object_object_get(rtObj, "GatewayIPAddress"));
		bool rtStaticRoute = json_object_get_boolean(json_object_object_get(rtObj, "StaticRoute"));
		if(!strcmp(rtIntfName, ipIface) && !strcmp(dstIP, "") && !strcmp(dstSubnetMask, "") && strcmp(routAdder, "acsaddobject")) {
			if(!strcmp(addrType, "Static") && rtStaticRoute == true && dstIP[0] == '\0' && strlen(rtIP))
				hasStaticRt = 1;
			else if(!strcmp(addrType, "DHCP") && rtStaticRoute != true && dstIP[0] == '\0' && strlen(rtIP))
				hasDhcpRt = 1;
			else if(!rtIntfObj){
				rtIntfObj = rtObj;
				rtObj = NULL;
				memcpy(&rtIntfIid, &rtIid, sizeof(objIndex_t));
				continue;
			}

			if(hasStaticRt || hasDhcpRt){
			*rtIpObj = rtObj;
			memcpy(rtIpIid, &rtIid, sizeof(objIndex_t));
				json_object_put(emptyObj);
				json_object_put(rtIntfObj);
			printf("%s: Ip route retrieved\n", __FUNCTION__);
			return ZCFG_SUCCESS;
		}
			continue;
		}
		else if(!strcmp(rtIntfName, "") && strcmp(routAdder, "acsaddobject")) {
 			if(!emptyObj) {
				emptyObj = rtObj;
				rtObj = NULL;
				memcpy(&emptyIid, &rtIid, sizeof(objIndex_t));
				continue;
			}
		}

		json_object_put(rtObj);
	}

	if(rtIntfObj) {
		*rtIpObj = rtObj = rtIntfObj;
		memcpy(rtIpIid, &rtIntfIid, sizeof(objIndex_t));
		json_object_put(emptyObj);
		printf("%s: Ip route retrieved\n", __FUNCTION__);
		return ZCFG_SUCCESS;
	}
	else if(emptyObj) {
		*rtIpObj = rtObj = emptyObj;
		memcpy(rtIpIid, &emptyIid, sizeof(objIndex_t));
	}
	else {
		printf("%s: There is not empty ip route! Add\n", __FUNCTION__);

		IID_INIT(emptyIid);
		emptyIid.idx[0] = 1;
		emptyIid.level = 1;
		if((ret = zcfgFeObjJsonAdd(RDM_OID_ROUTING_ROUTER_V4_FWD, &emptyIid, NULL)) != ZCFG_SUCCESS) {
			printf("%s: Add ip route fail!\n", __FUNCTION__);
			return ret;
		}
	
		rtObj = NULL;
		if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER_V4_FWD, &emptyIid, (struct json_object **)&rtObj)) != ZCFG_SUCCESS) {
			printf("%s: Retrieve ip route fail!\n", __FUNCTION__);
			return ret;
		}

		*rtIpObj = rtObj;
		memcpy(rtIpIid, &emptyIid, sizeof(objIndex_t));
	}
	json_object_object_add(rtObj, "Interface", json_object_new_string(ipIface));
	ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_ROUTING_ROUTER_V4_FWD, rtIpIid, rtObj, NULL);
	if(ret != ZCFG_SUCCESS && ret != ZCFG_DELAY_APPLY)
		return ret;

	printf("%s: Ip route return\n", __FUNCTION__);

	return ZCFG_SUCCESS;
}

char *activeDefaultGt_Strip(const char *activeDefaultGt, const char *ipIface)
{
	char *new_activeDefaultGt = NULL;
	char *str_1 = NULL, *str_2 = NULL;
	int activeDefaultGtLen = strlen(activeDefaultGt);

	if(!activeDefaultGt || !ipIface || (activeDefaultGtLen <= 0))
		return NULL;

	str_1 = strstr(activeDefaultGt, ipIface);
	if(!str_1) {
		return NULL;
	}

	new_activeDefaultGt = malloc(activeDefaultGtLen);
	new_activeDefaultGt[0]='\0';

	if(str_1 == activeDefaultGt) {
		str_2 = str_1 + strlen(ipIface);
		if(str_2[0] != '\0') {
			str_2++;
			ZOS_STRCPY_M(new_activeDefaultGt, str_2);
		}
	}
	else {
		str_2 = str_1 + strlen(ipIface);
		ZOS_STRNCPY(new_activeDefaultGt, activeDefaultGt, (str_1-activeDefaultGt));
		if(str_2[0] != '\0') {
			strcat(new_activeDefaultGt, str_2);
		}
	}

	return new_activeDefaultGt;
}

char *activeDefaultGt_Add(const char *activeDefaultGt, const char *ipIface)
{
	char *new_activeDefaultGt = NULL;
	char *str_1 = NULL;
	int activeDefaultGtLen = strlen(activeDefaultGt);

	if(!activeDefaultGt || !ipIface || (activeDefaultGtLen <= 0))
		return NULL;

	str_1 = strstr(activeDefaultGt, ipIface);
	if(str_1) {
		return NULL;
	}

	activeDefaultGtLen += strlen(ipIface) + 1 + 1;
	new_activeDefaultGt = malloc(activeDefaultGtLen);
	memset(new_activeDefaultGt, 0, activeDefaultGtLen);

	ZOS_STRCPY_M(new_activeDefaultGt, activeDefaultGt);
	new_activeDefaultGt[strlen(activeDefaultGt)] = ',';
	strcat(new_activeDefaultGt, ipIface);

	return new_activeDefaultGt;
}

zcfgRet_t defaultGwInfoSet(char *ipIface, struct json_object *tr98Jobj)
{
	objIndex_t ipv4FwdIid;
	rdm_RoutingRouterV4Fwd_t *v4FwdObj = NULL;

//	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(ipv4FwdIid);
	while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, &ipv4FwdIid, (void **)&v4FwdObj) == ZCFG_SUCCESS) {
		if(!strcmp(v4FwdObj->Interface, ipIface)) {
			json_object_object_add(tr98Jobj, "DefaultGateway", json_object_new_string(v4FwdObj->GatewayIPAddress));
			zcfgFeObjStructFree(v4FwdObj);
			return ZCFG_SUCCESS;
		}
		zcfgFeObjStructFree(v4FwdObj);
	}

	/*Default value*/
	json_object_object_add(tr98Jobj, "DefaultGateway", json_object_new_string(""));

	return ZCFG_SUCCESS;
}
#if 0
zcfgRet_t dnsInfoSet(char *ipIface, struct json_object *tr98Jobj)
{
	objIndex_t dnsIid;
	rdm_DnsClientSrv_t *dnsSrvObj = NULL;

//	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(dnsIid);
	while(zcfgFeObjStructGetNext(RDM_OID_DNS_CLIENT_SRV, &dnsIid, (void **)&dnsSrvObj) == ZCFG_SUCCESS) {
		if(!strcmp(dnsSrvObj->Interface, ipIface)) {
			json_object_object_add(tr98Jobj, "DNSEnabled", json_object_new_boolean(dnsSrvObj->Enable));
			json_object_object_add(tr98Jobj, "DNSServers", json_object_new_string(dnsSrvObj->DNSServer));
			json_object_object_add(tr98Jobj, "X_ZYXEL_DNSType", json_object_new_string(dnsSrvObj->X_ZYXEL_Type));
			zcfgFeObjStructFree(dnsSrvObj);
			return ZCFG_SUCCESS;
		}
		zcfgFeObjStructFree(dnsSrvObj);
	}

	/*Default value*/
	json_object_object_add(tr98Jobj, "DNSEnabled", json_object_new_boolean(true));
	json_object_object_add(tr98Jobj, "DNSServers", json_object_new_string(""));
	json_object_object_add(tr98Jobj, "X_ZYXEL_DNSType", json_object_new_string(""));

	return ZCFG_SUCCESS;
}
#else
zcfgRet_t dnsInfoGetJobj(char *ipIface, objIndex_t *dnsIid, struct json_object **dnsSrvJobj, bool ipv6, bool updateFlag){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *interface = NULL;
	const char *type = NULL;
	const char *dnsServers = NULL;
	const char *iP_Type = NULL;
	
	IID_INIT(*dnsIid);
	while((ret = feObjJsonGetNext(RDM_OID_DNS_CLIENT_SRV, dnsIid, dnsSrvJobj, updateFlag)) == ZCFG_SUCCESS)
	{
		interface = json_object_get_string(json_object_object_get((*dnsSrvJobj), "Interface"));
		type = json_object_get_string(json_object_object_get((*dnsSrvJobj), "Type"));
		dnsServers = json_object_get_string(json_object_object_get((*dnsSrvJobj), "DNSServer"));
		iP_Type = json_object_get_string(json_object_object_get((*dnsSrvJobj), "IP_Type"));

		if(!strcmp(ipIface, interface))
		{
			if (ipv6)
			{
				if (!strcmp("IPv6", iP_Type) || !strcmp("DHCPv6", type) || !strcmp("RouterAdvertisement", type) || strchr(dnsServers, ':'))
				{
					return ret;
				}
			}
			else
			{
				if (!strcmp("IPv4", iP_Type) || !strcmp("DHCPv4", type) || !strchr(dnsServers, ':'))
				{
					return ret;
				}
			}
		}
		json_object_put(*dnsSrvJobj);
		*dnsSrvJobj = NULL;
	}	
	return ret;
}

zcfgRet_t dnsAddJobj(char *ipIface, objIndex_t *dnsIid, struct json_object **dnsSrvJobj, bool ipv6){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *interface = NULL;
	bool found = false;
	const char *iP_Type = NULL;

	//if(!dnsIid || !dnsSrvJobj)
	
	*dnsSrvJobj = NULL;

	IID_INIT(*dnsIid);
	while((ret = feObjJsonGetNext(RDM_OID_DNS_CLIENT_SRV, dnsIid, dnsSrvJobj, false)) == ZCFG_SUCCESS) {
		interface = json_object_get_string(json_object_object_get((*dnsSrvJobj), "Interface"));
		iP_Type = json_object_get_string(json_object_object_get((*dnsSrvJobj), "IP_Type"));
		if(!strcmp(interface,"")){
			found = true;
			break;
		}
		json_object_put(*dnsSrvJobj);
		*dnsSrvJobj = NULL;
	}	
	if(!found){
		IID_INIT(*dnsIid);
		if((ret = zcfgFeObjStructAdd(RDM_OID_DNS_CLIENT_SRV, dnsIid, NULL)) != ZCFG_SUCCESS){
			return ret;
		}
		//*dnsSrvJobj = json_object_new_object();
		if((ret = zcfgFeObjJsonGet(RDM_OID_DNS_CLIENT_SRV, dnsIid, dnsSrvJobj)) != ZCFG_SUCCESS){
			return ret;
		}
	}

	if(ipIface && strstr(ipIface, "IP.Interface"))
	{
		json_object_object_add(*dnsSrvJobj, "Interface", json_object_new_string(ipIface));
		if (ipv6)
		{
			json_object_object_add(*dnsSrvJobj, "IP_Type", json_object_new_string("IPv6"));
		}
		else
		{
			json_object_object_add(*dnsSrvJobj, "IP_Type", json_object_new_string("IPv4"));
		}
	}

	//json_object_object_add(*dnsSrvJobj, "X_ZYXEL_Type", json_object_new_string(""));

	zcfgFeObjJsonSetWithoutApply(RDM_OID_DNS_CLIENT_SRV, dnsIid, *dnsSrvJobj, NULL);

	return ret;
}
#endif

bool isIGMPProxyEnable(char *ipIface)
{
	bool enable = false;
    objIndex_t objIid;
	rdm_ZyIgmp_t *igmpObj = NULL;

    IID_INIT(objIid);
    if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_ZY_IGMP, &objIid, (void **)&igmpObj) == ZCFG_SUCCESS) {
        if(igmpObj->Enable && strstr(igmpObj->Interface, ipIface)) {
			enable = true;
        }
        zcfgFeObjStructFree(igmpObj);
    }

    return enable;
}

bool isMLDProxyEnable(char *ipIface)
{
	bool enable = false;
    objIndex_t objIid;
	rdm_ZyMld_t *mldObj = NULL;

    IID_INIT(objIid);
    if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_ZY_MLD, &objIid, (void **)&mldObj) == ZCFG_SUCCESS) {
        if(strstr(mldObj->Interface, ipIface)) {
			enable = true;
        }
        zcfgFeObjStructFree(mldObj);
    }

    return enable;
}

zcfgRet_t multiObjEthLinkAdd(char *ethLinkPathName, char *lowerLayer, struct json_object *multiObj)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ethLinkObj = NULL;

	printf("%s: %s\n", __FUNCTION__, lowerLayer);

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_ETH_LINK, &objIid, NULL)) != ZCFG_SUCCESS) {
		return ZCFG_INTERNAL_ERROR;
	}

	sprintf(ethLinkPathName, "Ethernet.Link.%d", objIid.idx[0]);

	/*Set LowerLayer for Ethernet.Link*/
	if(zcfgFeObjJsonGet(RDM_OID_ETH_LINK, &objIid, &ethLinkObj) != ZCFG_SUCCESS){
		return ZCFG_INTERNAL_ERROR;
	}

	json_object_object_add(ethLinkObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(ethLinkObj, "LowerLayers", json_object_new_string(lowerLayer));
	json_object_object_add(ethLinkObj, "X_ZYXEL_OrigEthWAN", json_object_new_boolean(true));
	
	zcfgFeJsonMultiObjAppend(RDM_OID_ETH_LINK, &objIid, multiObj, ethLinkObj);
	zcfgFeJsonMultiObjAddParam(multiObj, RDM_OID_ETH_LINK, &objIid, "apply", json_object_new_boolean(true));
	json_object_put(ethLinkObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t multiObjVlanTermAdd(char *vlanTermPathName, char *lowerLayer, struct json_object *multiObj)
{
	objIndex_t objIid;
	struct json_object *vlanTermObj = NULL;

	printf("%s: %s\n", __FUNCTION__, lowerLayer);

	IID_INIT(objIid);
	if(zcfgFeObjStructAdd(RDM_OID_ETH_VLAN_TERM, &objIid, NULL) != ZCFG_SUCCESS) {
		return ZCFG_INTERNAL_ERROR;
	}

	sprintf(vlanTermPathName, "Ethernet.VLANTermination.%d", objIid.idx[0]);

	if(zcfgFeObjJsonGet(RDM_OID_ETH_VLAN_TERM, &objIid, &vlanTermObj) != ZCFG_SUCCESS){
		return ZCFG_INTERNAL_ERROR;
	}

	json_object_object_add(vlanTermObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(vlanTermObj, "LowerLayers", json_object_new_string(lowerLayer));
	json_object_object_add(vlanTermObj, "VLANID", json_object_new_int(-1));

	zcfgFeJsonMultiObjAppend(RDM_OID_ETH_VLAN_TERM, &objIid, multiObj, vlanTermObj);
	zcfgFeJsonMultiObjAddParam(multiObj, RDM_OID_ETH_VLAN_TERM, &objIid, "apply", json_object_new_boolean(true));
	json_object_put(vlanTermObj);

	return ZCFG_SUCCESS;
}

#ifdef CONFIG_TAAAB_CUSTOMIZATION
#define MAX_ACS_IPIFACE  2

bool isSrvNameRepeat(char *targetname, int myindex){
		objIndex_t ipIfaceIid;
		struct json_object *ipIfaceJobj = NULL;

     	IID_INIT(ipIfaceIid);
	    while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceJobj) == ZCFG_SUCCESS){
		if(myindex == ipIfaceIid.idx[0]){
			json_object_put(ipIfaceJobj);
			continue;
		}
		if(!strcmp(json_object_get_string(json_object_object_get(ipIfaceJobj, "X_ZYXEL_SrvName")), targetname)){
			json_object_put(ipIfaceJobj);
			return true;
		}
		json_object_put(ipIfaceJobj);
	    }

		return false;
}
#endif

#ifdef CONFIG_TAAAB_CUSTOMIZATION
zcfgRet_t multiObjIpIfaceAdd(char *ipIfacePathName, char *lowerLayer, struct json_object *multiObj, int wantype)
#else
zcfgRet_t multiObjIpIfaceAdd(char *ipIfacePathName, char *lowerLayer, struct json_object *multiObj)
#endif
{
	objIndex_t objIid;
	struct json_object *ipIfaceObj = NULL;
#ifdef CONFIG_TAAAB_CUSTOMIZATION
    int addindex = 0;
	char SrvName[128] = {0};
	char TAAABSrvName[128] = {0};
	bool isAddFromTr69 = false;
#endif

	printf("%s: %s\n", __FUNCTION__, lowerLayer);

	IID_INIT(objIid);
	if(zcfgFeObjStructAdd(RDM_OID_IP_IFACE, &objIid, NULL) != ZCFG_SUCCESS) {
		return ZCFG_INTERNAL_ERROR;
	}

	sprintf(ipIfacePathName, "IP.Interface.%d", objIid.idx[0]);

	if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &objIid, &ipIfaceObj) != ZCFG_SUCCESS){
		return ZCFG_INTERNAL_ERROR;
	}

	json_object_object_add(ipIfaceObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(ipIfaceObj, "LowerLayers", json_object_new_string(lowerLayer));
	json_object_object_add(ipIfaceObj, "X_ZYXEL_ConnectionId", json_object_new_int(objIid.idx[0] - 1));	
	json_object_object_add(ipIfaceObj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(objIid.idx[0] - 1));
#ifdef CONFIG_TAAAB_CUSTOMIZATION
	if(strstr(lowerLayer, "PPP") != NULL)
		json_object_object_add(ipIfaceObj, "MaxMTUSize", json_object_new_int(1492));
	else
		json_object_object_add(ipIfaceObj, "MaxMTUSize", json_object_new_int(1500));
	
    for(addindex = 0; addindex < MAX_ACS_IPIFACE; addindex++){ 
		sprintf(SrvName,"AcsIPIface%d",addindex+1);
		ZOS_STRCPY_M(TAAABSrvName,SrvName);
		if(wantype == 1){
			strcat(SrvName,"_ETHWAN");
			strcat(TAAABSrvName,"_WAN");
		}
		else if(wantype == 2){
			strcat(SrvName,"_ADSL");
			strcat(TAAABSrvName,"_DSL");
		}
		else if(wantype == 3){
			strcat(SrvName,"_VDSL");
			strcat(TAAABSrvName,"_DSL");
		}

		isAddFromTr69 = true;

		if(isSrvNameRepeat(SrvName,objIid.idx[0]))
			continue;
		else
			break;
    }
	printf("\r\n multiObjIpIfaceAdd,srvname is %s,ttname is %s isAddFromTr69 %d\r\n",SrvName,TAAABSrvName,isAddFromTr69);
	json_object_object_add(ipIfaceObj, "X_ZYXEL_SrvName", json_object_new_string(SrvName));
	json_object_object_add(ipIfaceObj, "X_TT_SrvName", json_object_new_string(TAAABSrvName));
	json_object_object_add(ipIfaceObj, "isAddFromTr69", json_object_new_boolean(isAddFromTr69));
#endif

	zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE, &objIid, multiObj, ipIfaceObj);
	zcfgFeJsonMultiObjAddParam(multiObj, RDM_OID_IP_IFACE, &objIid, "apply", json_object_new_boolean(true));
	json_object_put(ipIfaceObj);

	return ZCFG_SUCCESS;
}

#ifdef CONFIG_TAAAB_CUSTOMIZATION
zcfgRet_t multiObjPppIfaceAdd(char *pppIfacePathName, char *lowerLayer, struct json_object *multiObj, int wantype)
#else
zcfgRet_t multiObjPppIfaceAdd(char *pppIfacePathName, char *lowerLayer, struct json_object *multiObj)
#endif
{
	objIndex_t objIid;
	objIndex_t ipObjIid;
	struct json_object *pppIfaceObj = NULL;
	struct json_object *newPppIfaceObj = NULL;
	char ipIfacePathName[32] = {0};

	printf("%s: %s\n", __FUNCTION__, lowerLayer);

	IID_INIT(objIid);
	if(zcfgFeObjStructAdd(RDM_OID_PPP_IFACE, &objIid, NULL) != ZCFG_SUCCESS) {
		return ZCFG_INTERNAL_ERROR;
	}

	sprintf(pppIfacePathName, "PPP.Interface.%d", objIid.idx[0]);

	if(zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &objIid, &pppIfaceObj) != ZCFG_SUCCESS){
		return ZCFG_INTERNAL_ERROR;
	}

	json_object_object_add(pppIfaceObj, "IPCPEnable", json_object_new_boolean(true));
	json_object_object_add(pppIfaceObj, "LowerLayers", json_object_new_string(lowerLayer));

	newPppIfaceObj = zcfgFeJsonMultiObjAppend(RDM_OID_PPP_IFACE, &objIid, multiObj, pppIfaceObj);
	zcfgFeJsonMultiObjAddParam(multiObj, RDM_OID_PPP_IFACE, &objIid, "apply", json_object_new_boolean(true));
	json_object_put(pppIfaceObj);

        #ifdef CONFIG_TAAAB_CUSTOMIZATION
	if(multiObjIpIfaceAdd(ipIfacePathName, pppIfacePathName, multiObj, wantype) != ZCFG_SUCCESS)
	#else
	if(multiObjIpIfaceAdd(ipIfacePathName, pppIfacePathName, multiObj) != ZCFG_SUCCESS)
	#endif
		return ZCFG_INTERNAL_ERROR;

	zcfgFeObjNameToObjId( ipIfacePathName, &ipObjIid);
	json_object_object_add(newPppIfaceObj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(ipObjIid.idx[0] - 1));

	return ZCFG_SUCCESS;
}

zcfgRet_t ethLinkAdd(char *ethLinkPathName, char *lowerLayer)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	rdm_EthLink_t *ethLinkObj = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_ETH_LINK, &objIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}

	sprintf(ethLinkPathName, "Ethernet.Link.%d", objIid.idx[0]);

	/*Set LowerLayer for Ethernet.Link*/
	if((ret = zcfgFeObjStructGet(RDM_OID_ETH_LINK, &objIid, (void **)&ethLinkObj)) == ZCFG_SUCCESS) {
		ethLinkObj->Enable = true;
		ZOS_STRCPY_M(ethLinkObj->LowerLayers, lowerLayer);
		if((ret = zcfgFeObjStructSet(RDM_OID_ETH_LINK, &objIid, (void *)ethLinkObj, NULL)) != ZCFG_SUCCESS) {
			printf("%s : Set Ethernet.Link LowerLayers Fail\n", __FUNCTION__);
		}

		zcfgFeObjStructFree(ethLinkObj);
	}
	else {
		return ret;
	}

	return ret;	
}

zcfgRet_t vlanTermAdd(char *vlanTermPathName, char *lowerLayer)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	rdm_EthVlanTerm_t *vlanTermObj = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_ETH_VLAN_TERM, &objIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}

	sprintf(vlanTermPathName, "Ethernet.VLANTermination.%d", objIid.idx[0]);

	/*Set LowerLayer for Ethernet.Link*/
	if((ret = zcfgFeObjStructGet(RDM_OID_ETH_VLAN_TERM, &objIid, (void **)&vlanTermObj)) == ZCFG_SUCCESS) {
		vlanTermObj->Enable = true;
		ZOS_STRCPY_M(vlanTermObj->LowerLayers, lowerLayer);
		vlanTermObj->VLANID = -1;
		if((ret = zcfgFeObjStructSet(RDM_OID_ETH_VLAN_TERM, &objIid, (void *)vlanTermObj, NULL)) != ZCFG_SUCCESS) {
			printf("%s : Set Ethernet.VLANTermination LowerLayers Fail\n", __FUNCTION__);
		}

		zcfgFeObjStructFree(vlanTermObj);
	}
	else {
		return ret;
	}

	return ret;	
}

zcfgRet_t ipIfaceAdd(char *ipIfacePathName, char *lowerLayer)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	rdm_IpIface_t *ipIfaceObj = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE, &objIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}

	sprintf(ipIfacePathName, "IP.Interface.%d", objIid.idx[0] );

	/*Set LowerLayer for IP.Interface */
	if((ret = zcfgFeObjStructGet(RDM_OID_IP_IFACE, &objIid, (void **)&ipIfaceObj)) == ZCFG_SUCCESS) {
		ipIfaceObj->Enable = true;
		ZOS_STRCPY_M(ipIfaceObj->LowerLayers, lowerLayer);
		if((ret = zcfgFeObjStructSet(RDM_OID_IP_IFACE, &objIid, (void *)ipIfaceObj, NULL)) != ZCFG_SUCCESS) {
			printf("%s : Set IP.Interface LowerLayers Fail\n", __FUNCTION__);
		}
		else {
			printf("%s : Set IP.Interface LowerLayers Success\n", __FUNCTION__);
		}

		zcfgFeObjStructFree(ipIfaceObj);
	}
	else {
		printf("%s : Get IP.Interface fail\n", __FUNCTION__);
		return ret;
	}
	
	return ret;
}

zcfgRet_t pppIfaceAdd(char *pppIfacePathName, char *lowerLayer)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	rdm_PppIface_t *pppIfaceObj = NULL;
	char ipIfacePathName[32] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_PPP_IFACE, &objIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}

	sprintf(pppIfacePathName, "PPP.Interface.%d", objIid.idx[0] );

	/*Set LowerLayer for PPP.Interface */
	if((ret = zcfgFeObjStructGet(RDM_OID_PPP_IFACE, &objIid, (void **)&pppIfaceObj)) == ZCFG_SUCCESS) {
		//pppIfaceObj->Enable = true;
		ZOS_STRCPY_M(pppIfaceObj->LowerLayers, lowerLayer);
		pppIfaceObj->IPCPEnable = 1;
		if((ret = zcfgFeObjStructSet(RDM_OID_PPP_IFACE, &objIid, (void *)pppIfaceObj, NULL)) != ZCFG_SUCCESS) {
			printf("%s : Set PPP.Interface LowerLayers Fail\n", __FUNCTION__);
		}

		zcfgFeObjStructFree(pppIfaceObj);
	}
	else {
		return ret;
	}

	/*Add Device.IP.Interface.i*/
	ret = ipIfaceAdd(ipIfacePathName, pppIfacePathName);

	return ret;
}

zcfgRet_t addDhcpObj(char *ipIface, struct json_object *multiJobj)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	//rdm_Dhcpv4Client_t *dhcpv4Obj = NULL;
	struct json_object *dhcpv4Obj = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_CLIENT, &objIid, &dhcpv4Obj)) == ZCFG_SUCCESS) {
		/*Search a DHCPv4 Client Object whose value of Interface is empty*/
		const char *paramStr = json_object_get_string(json_object_object_get(dhcpv4Obj, "Interface"));
		if(paramStr && !strcmp(paramStr, "")) {
			json_object_object_add(dhcpv4Obj, "Enable", json_object_new_boolean(true));
			json_object_object_add(dhcpv4Obj, "Interface", json_object_new_string(ipIface));
			if(multiJobj)
				zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_CLIENT, &objIid, multiJobj, dhcpv4Obj);
			else{
				ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_CLIENT, &objIid, dhcpv4Obj, NULL);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					printf("Set DHCPv4.Client Fail\n");
				}
			}
			json_object_put(dhcpv4Obj);
			return ret;
		}
	}

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_DHCPV4_CLIENT, &objIid, NULL)) == ZCFG_SUCCESS) {
		printf("Add DHCPv4 Client Success\n");
		if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_CLIENT, &objIid, &dhcpv4Obj)) == ZCFG_SUCCESS) {
			json_object_object_add(dhcpv4Obj, "Enable", json_object_new_boolean(true));
			json_object_object_add(dhcpv4Obj, "Interface", json_object_new_string(ipIface));
			if(multiJobj)
				zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_CLIENT, &objIid, multiJobj, dhcpv4Obj);
			else{
				ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_CLIENT, &objIid, (void *)dhcpv4Obj, NULL);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					printf("Set DHCPv4 Client Fail\n");
					return ret;
				}
			}
			json_object_put(dhcpv4Obj);
		}
		else {
			printf("Get DHCPv4 Client Fail\n");
			return ret;
		}
	}
	else {
		printf("Add DHCPv4 Client Fail\n");
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t setDhcpObj(char *ipIface, bool enable, struct json_object *multiJobj)
{
	zcfgRet_t ret;
	objIndex_t dhcpcv4Iid;
	//rdm_Dhcpv4Client_t *dhcpc4Obj = NULL;
	struct json_object *dhcpcv4Obj = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(dhcpcv4Iid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_CLIENT, &dhcpcv4Iid, &dhcpcv4Obj)) == ZCFG_SUCCESS) {
		const char *paramStr = json_object_get_string(json_object_object_get(dhcpcv4Obj, "Interface"));
		if(paramStr && !strcmp(paramStr, ipIface)) {
			if(!enable)
				json_object_object_add(dhcpcv4Obj, "Interface", json_object_new_string(""));
			else
				json_object_object_add(dhcpcv4Obj, "Enable", json_object_new_boolean(enable));

			if(multiJobj)
				zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_CLIENT, &dhcpcv4Iid, multiJobj, dhcpcv4Obj);
			else{
				ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_CLIENT, &dhcpcv4Iid, (void *)dhcpcv4Obj, NULL);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					printf("Set DHCPv4.Client Fail\n");
				}
			}
			json_object_put(dhcpcv4Obj);
			
			return ret;
		}
		json_object_put(dhcpcv4Obj);
	}
	
	return ZCFG_NO_SUCH_OBJECT;
}

zcfgRet_t ipaddrSet(char *ipIface, objIndex_t *ipIfIid, json_object **ipIfaceJobj, const char *addrType, struct json_object *tr98Jobj, struct json_object *multiJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t v4AddrIid;
	//rdm_IpIfaceV4Addr_t *v4AddrObj = NULL;
	struct json_object *v4AddrObj = NULL;
	struct json_object *paramValue = NULL;
	bool ipv4Enabled = false, ipv4AddrAddDelete = false;
	bool reqSetAddrObj = false;

	printf("%s : Enter\n", __FUNCTION__);

	printf("AddrType: %s\n", addrType);

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	objIndex_t dnsSrvIid;
	struct json_object *dnsSrvJobj = NULL;
	const char *dnsaddr = NULL, *ipaddr=NULL;
	if(dnsInfoGetJobj(ipIface, &dnsSrvIid, (struct json_object **)&dnsSrvJobj, false, false) == ZCFG_SUCCESS){ //dns v4
		 dnsaddr = json_object_get_string(json_object_object_get(dnsSrvJobj, "DNSServer"));
		 ipaddr = json_object_get_string(json_object_object_get(tr98Jobj, "ExternalIPAddress"));
		 if(dnsaddr && ipaddr && strstr(dnsaddr,ipaddr)){
		 	printf("\r\n%s,go to error set,ip is %s,dns is %s\r\n", "ipaddrSet",ipaddr,dnsaddr);
		 	json_object_put(dnsSrvJobj);
			ret = ZCFG_REQUEST_REJECT;
			return ret;
		 }
		 else
		 	json_object_put(dnsSrvJobj);
	}
#endif

	IID_INIT(v4AddrIid);
	if(zcfgFeSubInObjJsonGetNext(RDM_OID_IP_IFACE_V4_ADDR, ipIfIid, &v4AddrIid, &v4AddrObj) == ZCFG_SUCCESS) {
		reqSetAddrObj = false;
		/*IPv4Address already exist*/
		printf("%s : IPv4Address already exists\n", __FUNCTION__);
		char const *addressingType = json_object_get_string(json_object_object_get(v4AddrObj, "AddressingType"));
		if(addressingType && (!strcmp(addressingType, "Static") || !strcmp(addressingType, ""))) {
			if(!strcmp(addrType, "DHCP")) {
				json_object_object_add(v4AddrObj, "AddressingType", json_object_new_string("DHCP"));
				/*Static to DHCP*/
				if((ret = setDhcpObj(ipIface, true, multiJobj)) == ZCFG_NO_SUCH_OBJECT) {
					ret = addDhcpObj(ipIface, multiJobj);
				}
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					json_object_put(v4AddrObj);
					return ret;
				}
				json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));
				reqSetAddrObj = true;
			}
			else if(!strcmp(addrType, "Static")){
				const char *addr = NULL, *nmask = NULL;
				json_object_object_add(v4AddrObj, "AddressingType", json_object_new_string("Static"));
				/*Static to Static*/
				json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));
				paramValue = json_object_object_get(tr98Jobj, "ExternalIPAddress");
				addr = json_object_get_string(paramValue);
				paramValue = json_object_object_get(tr98Jobj, "SubnetMask");
				nmask = json_object_get_string(paramValue);
				if(!addr || (strlen(addr) == 0)) {
					json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(false));
				}
				//if(strcmp((const char *)v4AddrObj->IPAddress, addr) ||
				//    strcmp((const char *)v4AddrObj->SubnetMask, nmask)) {
					reqSetAddrObj = true;
				//}
				json_object_object_add(v4AddrObj, "IPAddress", json_object_new_string(addr));
				json_object_object_add(v4AddrObj, "SubnetMask", json_object_new_string(nmask));
			}

			if(reqSetAddrObj == true) {
				if(multiJobj)
					zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, multiJobj, v4AddrObj);
				else{
					ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, v4AddrObj, NULL);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						printf("%s : Set IPv4Address Fail\n", __FUNCTION__);
						json_object_put(v4AddrObj);
						return ret;
					}
				}
				if((json_object_get_boolean(json_object_object_get(v4AddrObj, "Enable"))) == true)
					ipv4Enabled = true;
			}
			//json_object_put(v4AddrObj);
		}
		else if(addressingType && !strcmp(addressingType, "DHCP")){
			if(!strcmp(addrType, "DHCP")) {
				printf("%s : DHCP to DHCP\n", __FUNCTION__);
			}
			else if(!strcmp(addrType, "Static")){
				/*DHCP to Static*/
				/*Disable DHCP Client, the DHCP backend will delete IPv4Address automatically*/
				setDhcpObj(ipIface, false, multiJobj);

				/*Add IPv4Address*/
				//memcpy(&v4AddrIid, ipIfIid, sizeof(objIndex_t));
				//if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, NULL)) == ZCFG_SUCCESS) {
					json_object_object_add(v4AddrObj, "AddressingType", json_object_new_string("Static"));
					//printf("%s : Add IPv4Address Success\n", __FUNCTION__);
					//if((ret = zcfgFeObjStructGet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, (void **)&v4AddrObj)) == ZCFG_SUCCESS) {
					{
						const char *addr = NULL;
						paramValue = json_object_object_get(tr98Jobj, "ExternalIPAddress");
						if(paramValue != NULL) {
							addr = json_object_get_string(paramValue);
							json_object_object_add(v4AddrObj, "IPAddress", json_object_new_string(addr));
						}
						if(!addr || (strlen(addr) == 0))
							json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(false));
						else
							json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));

						paramValue = json_object_object_get(tr98Jobj, "SubnetMask");
						if(paramValue != NULL) {
							json_object_object_add(v4AddrObj, "SubnetMask", json_object_new_string(json_object_get_string(paramValue)));
						}

						if(multiJobj)
							zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, multiJobj, v4AddrObj);
						else{
							ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, v4AddrObj, NULL);
							if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
								printf("%s : Set IPv4Address Fail\n", __FUNCTION__);
								json_object_put(v4AddrObj);
								return ret;
							}
						}
						ipv4AddrAddDelete = true;

						if((json_object_get_boolean(json_object_object_get(v4AddrObj, "Enable"))) == true){
							ipv4Enabled = true;
						}
						//json_object_put(v4AddrObj);
					}
					//else {
					//	printf("%s : Get IPv4Address Fail\n", __FUNCTION__);
					//	return ret;
					//}
				//}
				//else {
				//	printf("%s : Add IPv4Address Fail\n", __FUNCTION__);
				//	return ret;
				//}
			}
		}
		else {
			printf("%s : Other : %s to %s", __FUNCTION__, addressingType, addrType);
		}

		json_object_put(v4AddrObj);
	}
	else {
		reqSetAddrObj = false;
		printf("%s : IPv4Address does not exist\n", __FUNCTION__);
		memcpy(&v4AddrIid, ipIfIid, sizeof(objIndex_t));
		if((ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, NULL)) == ZCFG_SUCCESS) {
			printf("%s : Add IPv4Address Success\n", __FUNCTION__);
			if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj)) == ZCFG_SUCCESS) {
				printf("%s : Get IPv4Address Success\n", __FUNCTION__);
			}
			else {
				printf("%s : Get IPv4Address Fail\n", __FUNCTION__);
				return ret;
			}
			ipv4AddrAddDelete = true;
		}
		else {
			printf("%s : Add IPv4Address Fail\n", __FUNCTION__);
			return ret;
		}

		if(!strcmp(addrType, "DHCP")) {
			json_object_object_add(v4AddrObj, "AddressingType", json_object_new_string("DHCP"));
			if((ret = setDhcpObj(ipIface, true, multiJobj)) == ZCFG_NO_SUCH_OBJECT) {
				ret = addDhcpObj(ipIface, multiJobj);
			}
			if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
				json_object_put(v4AddrObj);
				return ret;
			}
			reqSetAddrObj = true;
			json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));
		}
		else if(!strcmp(addrType, "Static")) {
			const char *addr = NULL, *nmask = NULL;

			setDhcpObj(ipIface, false, multiJobj);
				
			/*Add IPv4Address*/
			printf("%s : Add Static IPv4Address\n", __FUNCTION__);
			json_object_object_add(v4AddrObj, "AddressingType", json_object_new_string("Static"));

			paramValue = json_object_object_get(tr98Jobj, "ExternalIPAddress");
			if(paramValue != NULL) {
				addr = json_object_get_string(paramValue);
				json_object_object_add(v4AddrObj, "IPAddress", json_object_new_string(addr));
			}

			paramValue = json_object_object_get(tr98Jobj, "SubnetMask");
			if(paramValue != NULL) {
				nmask = json_object_get_string(paramValue);
				json_object_object_add(v4AddrObj, "SubnetMask", json_object_new_string(nmask));
			}
			if(!addr || (strlen(addr) == 0))
				json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(false));
			else
				json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));

			if(addr || nmask)
				reqSetAddrObj = true;
		}

		if(reqSetAddrObj == true) {
			if(multiJobj)
				zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, multiJobj, v4AddrObj);
			else{
				ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, (void *)v4AddrObj, NULL);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					printf("%s : Set IPv4Address Fail\n", __FUNCTION__);
					json_object_put(v4AddrObj);
					return ret;
				}
			}
			printf("%s : Set IPv4Address Success\n", __FUNCTION__);
			if((json_object_get_boolean(json_object_object_get(v4AddrObj, "Enable"))) == true)
				ipv4Enabled = true;
		}
		json_object_put(v4AddrObj);
	}

	if(ipv4AddrAddDelete == true) {
		struct json_object *ipObj = NULL;
		if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE, ipIfIid, (struct json_object **)&ipObj)) != ZCFG_SUCCESS)
			return ret;

		json_object_put(*ipIfaceJobj);
		*ipIfaceJobj = ipObj;
#ifndef ABUU_CUSTOMIZATION
		json_object_object_add(*ipIfaceJobj, "X_ZYXEL_ConnectionId", json_object_new_int(ipIfIid->idx[0] - 1));
		json_object_object_add(*ipIfaceJobj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(ipIfIid->idx[0] - 1));
#endif
	}

	if(ipv4Enabled == true) {
		bool objIpv4Enabled = json_object_get_boolean(json_object_object_get(*ipIfaceJobj, "IPv4Enable"));
		if(objIpv4Enabled != true)
			json_object_object_add(*ipIfaceJobj, "IPv4Enable", json_object_new_boolean(true));
	}

	return ret;
}

zcfgRet_t DhcpcIidGet(char *tr98FullPathName, objIndex_t *dhcpcIid)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	rdm_Dhcpv4Client_t* dhcpcObj = NULL;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ipIface[32] = {0};
	char *ptr = NULL;
	bool found = false;

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	
	ptr = strstr(tr98TmpName, ".DHCPClient");
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	memset(dhcpcIid, 0, sizeof(dhcpcIid));
	while(!found && (ret = zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_DHCPV4_CLIENT, dhcpcIid, (void **)&dhcpcObj)) == ZCFG_SUCCESS) {
		if(!strcmp(dhcpcObj->Interface, ipIface)) {
			found = true;
		}
		zcfgFeObjStructFree(dhcpcObj);
	}

	if(!found){
		memset(dhcpcIid, 0, sizeof(dhcpcIid));
		return ZCFG_NOT_FOUND;
	}
	
	return ZCFG_SUCCESS;
}
#ifdef IAAAA_CUSTOMIZATION
bool getSpecificObj(zcfg_offset_t oid, char *key, json_type type, void *value, objIndex_t *outIid, struct json_object **outJobj)
{
	objIndex_t iid;
	struct json_object *Obj = NULL;
	struct json_object *pramJobj;
	bool found = false;

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &Obj) == ZCFG_SUCCESS) {
		pramJobj = json_object_object_get(Obj, key);
		if(pramJobj != NULL){
			if(type == json_type_string){
				if(!strcmp((const char*)value, json_object_get_string(pramJobj))){
					found = true;
				}
			}
			else{
				if(*(int*)value == json_object_get_int(pramJobj))
					found = true;
			}
			
			if(found){
				if(outIid) memcpy(outIid, &iid, sizeof(objIndex_t));
				if(outJobj) *outJobj = Obj;
				else json_object_put(Obj);
				break;
			}
		}
		json_object_put(Obj);
	}

	return found;
}
#endif

#ifdef ZYXEL_RUNNINGWAN_CUSTOMIZATION
zcfgRet_t checkWANType(objIndex_t ipIfaceIid, rdm_IpIface_t* ipIfaceObj, char* currType, uint8_t *iid){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *HigherLayer;
	const char *LowerLayer;
	char CurrMode[16] = {0};
	char ipIfacePath[32] = {0};
	int count = 0;
	struct json_object *vlanTermObj = NULL;
	struct json_object *pppIfaceObj = NULL;
	struct json_object *ethLinkObj = NULL;
	struct json_object *ethIfaceObj = NULL;
	objIndex_t vlanTermIid;
	objIndex_t pppIfaceIid;
	objIndex_t ethLinkIid;
	objIndex_t ethIfaceIid;

	snprintf(ipIfacePath, sizeof(ipIfacePath),  "IP.Interface.%hhu", ipIfaceIid.idx[0]);
	printf("%s: enter , ipIfacePath = %s\n",__FUNCTION__,ipIfacePath);
	//LowerLayer = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
	LowerLayer = ipIfaceObj->LowerLayers;
	printf("LowerLayer first=%s\n", LowerLayer);
	//strncpy(CurrMode, json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionType")));
	strncpy(CurrMode, ipIfaceObj->X_ZYXEL_ConnectionType, sizeof(CurrMode));
	printf("CurrMode=%s\n", CurrMode);
	HigherLayer = ipIfacePath;

	while(count < 4){
		printf("count=%d\n", count);
		printf("HigherLayer=%s\n", HigherLayer);
		printf("LowerLayer=%s\n", LowerLayer);
		count++;
		if(!strncmp(HigherLayer, "IP.Interface.", 13)){
			if(!strncmp(LowerLayer, "PPP.Interface.", 14)){
				pppIfaceIid.level = 1;
				sscanf(LowerLayer, "PPP.Interface.%hhu", &pppIfaceIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIfaceObj);
				HigherLayer = LowerLayer;
				LowerLayer = json_object_get_string(json_object_object_get(pppIfaceObj, "LowerLayers"));
			}
			else if(!strncmp("Ethernet.VLANTermination.", LowerLayer, 25)){
				vlanTermIid.level = 1;
				sscanf(LowerLayer, "Ethernet.VLANTermination.%hhu", &vlanTermIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, &vlanTermObj);
				HigherLayer = LowerLayer;
				LowerLayer = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
			}
			else if(!strncmp("ATM.Link.", LowerLayer, 9) && !strcmp("IP_Routed", CurrMode)){
				strncpy(currType, "ADSL", 10);
				sscanf(LowerLayer, "ATM.Link.%hhu", iid);
				break;
			}
			else{
				return ZCFG_REQUEST_REJECT;
			}
		}
		else if(!strncmp(HigherLayer, "PPP.Interface.", 14)){
			if(!strncmp("Ethernet.VLANTermination.", LowerLayer, 25)){
				vlanTermIid.level = 1;
				sscanf(LowerLayer, "Ethernet.VLANTermination.%hhu", &vlanTermIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, &vlanTermObj);
				HigherLayer = LowerLayer;
				LowerLayer = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
			}
			else if(!strncmp("ATM.Link.", LowerLayer, 9)){
				strncpy(currType, "ADSL", 10);
				sscanf(LowerLayer, "ATM.Link.%hhu", iid);
				break;
			}
			else if(!strncmp("USB.Interface.", LowerLayer, 14)){
				strncpy(currType, "3G/4G", 10);
				break;
			}
			else{
				return ZCFG_REQUEST_REJECT;
			}
		}
		else if(!strncmp("Ethernet.VLANTermination.", HigherLayer, 25)){
			if(!strncmp("Ethernet.Link.", LowerLayer, 14)){
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
				strncpy(currType, "ADSL", 10);
				sscanf(LowerLayer, "ATM.Link.%hhu", iid);
				break;
			}
			else if(!strncmp("PTM.Link.", LowerLayer, 9)){
				strncpy(currType, "VDSL", 10);
				break;
			}
			else if(!strncmp("Ethernet.Interface.", LowerLayer, 19)){
				ethIfaceIid.level = 1;
				sscanf(LowerLayer, "Ethernet.Interface.%hhu", &ethIfaceIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_IFACE, &ethIfaceIid, &ethIfaceObj);
				if(json_object_get_boolean(json_object_object_get(ethIfaceObj, "X_ZYXEL_SFP")) == false)
				strncpy(currType, "Ethernet", 10);
				else
					strncpy(currType, "SFP", 10);
				break;
			}
			else if(!strncmp("Optical.Interface.", LowerLayer, 18)){
				strncpy(currType, "PON", 10);
				break;
			}
			else if(!strcmp("", LowerLayer)){
				if(json_object_get_boolean(json_object_object_get(ethLinkObj, "X_ZYXEL_OrigEthWAN")) == false){
					return ZCFG_REQUEST_REJECT;
				}
				strncpy(currType, "Ethernet", 10);
				break;
			}
			else{
				return ZCFG_REQUEST_REJECT;
			}
		}
	}
	return ret;
}
#endif

