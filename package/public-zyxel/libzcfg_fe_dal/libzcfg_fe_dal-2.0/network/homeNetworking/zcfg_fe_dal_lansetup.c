#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>
#include <unistd.h>	//for sleep()

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_net.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_dal.h"
#include "zos_api.h"

dal_param_t LANSETUP_param[]={
	{"Name", dalType_string,	1,	128,	NULL, NULL, dal_Edit},
	//LAN IP
	{"IPAddress", dalType_v4Addr,	0,	0,	NULL},
	{"SubnetMask", dalType_v4Mask,	0,	0,	NULL},
	//IP Addressing Values
	{"DHCP_MinAddress", dalType_v4Addr,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	{"DHCP_MaxAddress", dalType_v4Addr,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	{"DHCP_AutoReserveLanIp", dalType_boolean,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	{"DHCP_RelayAddr", dalType_v4Addr,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	//DHCP Server State
	{"EnableDHCP", dalType_boolean,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	{"DHCPType", dalType_string,	0,	0,	NULL, "DHCPServer|DHCPRelay",hidden_extender_dalcmd_param},
	//DHCP Server Lease Time
	{"DHCP_LeaseTime", dalType_int,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	//DNS Values
	{"DNS_Type", dalType_string,	0,	0,	NULL, "DNSProxy|FromISP|Static",hidden_extender_dalcmd_param},
	{"DNS_Servers1", dalType_v4Addr,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	{"DNS_Servers2", dalType_v4Addr,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	//LAN IPv6 Mode Setup
	{"IPv6_LanEnable", dalType_boolean,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	//Link Local Address Type
	{"IPv6_LinkLocalAddressType", dalType_string,	0,	0,	NULL, "EUI64|Manual",hidden_extender_dalcmd_param},
	{"IPv6_LinkLocalAddress", dalType_v6Addr,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	//Lan Global Identifier Type
	{"IPv6_IdentifierType", 	dalType_string,	0,	0,	NULL, "EUI64|Manual",hidden_extender_dalcmd_param},
	{"IPv6_Identifier", dalType_string,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	//LAN IPv6 Prefix Setup
	{"IPv6_PrefixType", dalType_string,	0,	0,	NULL, "Static|DelegateFromWan",hidden_extender_dalcmd_param},
	{"IPv6_PrefixWan",			dalType_RtWan,	0,	0,	NULL, "Default",hidden_extender_dalcmd_param},
	{"IPv6_Prefix", 			dalType_v6AddrPrefix,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	//LAN IPv6 Address Assign Setup
	{"IPv6_LanAddrAssign", dalType_string,	0,	0,	NULL, "Stateless|Stateful"},
	//LAN IPv6 DNS Assign Setup
	{"IPv6_DNSAssign", dalType_string,	0,	0,	NULL, "RA|DHCP|RA_DHCP",hidden_extender_dalcmd_param},
	//IPv6 Address Values
	{"Ipv6_MinAddress", dalType_string,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	{"Ipv6_MaxAddress", dalType_string,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
	{"Ipv6_DNSSuffix", dalType_string,	0,	0,	NULL,NULL,hidden_extender_dalcmd_param},
//{"Ipv6Interface", dalType_string,	0,	0,	NULL},
	//IPv6 DNS Values
	{"Ipv6_DNSServer1", dalType_v6Addr,	0,	0,	NULL, "None|ISP|Proxy",hidden_extender_dalcmd_param},
	{"Ipv6_DNSServer2", dalType_v6Addr,	0,	0,	NULL, "None|ISP|Proxy",hidden_extender_dalcmd_param},
	{"Ipv6_DNSServer3", dalType_v6Addr,	0,	0,	NULL, "None|ISP|Proxy",hidden_extender_dalcmd_param},
	//DNS Query Scenario
	{"DNSQueryScenario", dalType_string,	0,	0,	NULL, "IPv4IPv6|IPv6Only|IPv4Only|IPv6First|IPv4First",hidden_extender_dalcmd_param},
	//Other
	{"IGMPSnoopingMode", dalType_string,	0,	0,	NULL, "Disabled|Standard|Blocking",	dalcmd_Forbid},
	{"MLDSnoopingMode", dalType_string,	0,	0,	NULL, "Disabled|Standard|Blocking",	dalcmd_Forbid},
#ifdef CONFIG_ABZQ_CUSTOMIZATION
	{"X_ZYXEL_WANMtuFlag", dalType_boolean, 0, 0, NULL},
#endif
#ifdef ZYXEL_OPAL_EXTENDER
	//DHCP Client
	{"EnableDHCPC", dalType_boolean,    0,  0,  NULL},
	{"IPv4_GW", dalType_v4Addr, 0,  0,  NULL},
	{"IPv6_GW", dalType_v6Addr, 0,  0,  NULL},
	{"IPv6_StaticAddress", dalType_v6Addr, 0,  0,  NULL},
#endif
	{NULL,					0,	0,	0,	NULL},
};

//GlobalLanObjs
static struct json_object *allIpIfaceObj = NULL;
static struct json_object *curIpIfaceObj = NULL;
static char ifacePath[32] = {0};
static char manualPrefixes[32] = {0};
static const char *selbrName = NULL;

static objIndex_t allIpIfaceIid = {0};
static objIndex_t curIpIfaceIid = {0};

void initGlobalLanObjs(){
	curIpIfaceObj = NULL;
	allIpIfaceObj = NULL;
	IID_INIT(curIpIfaceIid);
	IID_INIT(allIpIfaceIid);
}

void freeAllLanObjs(){
	zcfgFeJsonObjFree(allIpIfaceObj);
	//zcfgFeJsonObjFree(curIpIfaceObj); //curIpIfaceObj is a part of allIpIfaceObj, cant free it.
}

/*
 * Get Ipv6_DNSServer1 Ipv6_DNSServer2 Ipv6_DNSServer3 and curDnsList, curDnsServer
 * to construct dnsList and dnsServer
 *
 * The posiable of Ipv6_DNSServer is "ISP", "None", or IPv6 address. If it is IPv6 address, imply its Dns type is 'User Defined'.
 * e.g. curDnsList="None,User,User"   curDnsServer=",2001:123::1,2001:123::2"
 *        Ipv6_DNSServer1="2002:111::1"  Ipv6_DNSServer2=NULL  Ipv6_DNSServer3="ISP"
 *
 *      After convert, dnsList = "User,User,ISP"  dnsServer="2002:111::1,2001:123::1,"
*/
void convertIPv6DnsList(struct json_object *Jobj){
	struct json_object *radvdObj = NULL;
	objIndex_t iid = {0};
	const char *inface = NULL;
	bool found = false;
	char dnsList[32] = {0}, dnsServer[256] = {0};
	char  curDnsList[32] = {0}, curDnsServer[256] = {0};
	const char *DNSList1 = NULL, *DNSList2 = NULL, *DNSList3 = NULL;
	const char *DNSServer1 = NULL, *DNSServer2 = NULL, *DNSServer3 = NULL;
	const char *curDnsList1 = NULL, *curDnsList2 = NULL, *curDnsList3 = NULL;
	const char *curDNSServer1 = NULL, *curDNSServer2 = NULL, *curDNSServer3 = NULL;
	const char *Ipv6_DNSServer1 = NULL, *Ipv6_DNSServer2 = NULL, *Ipv6_DNSServer3 = NULL;
	char *tmp_ptr = NULL;
	
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_RT_ADV_INTF_SET, &iid, &radvdObj) == ZCFG_SUCCESS) {
		inface = json_object_get_string(json_object_object_get(radvdObj, "Interface"));
		if(!strcmp(inface, ifacePath)){
			found = true;
			strcpy(curDnsList, json_object_get_string(json_object_object_get(radvdObj, "X_ZYXEL_DNSList")));
			strcpy(curDnsServer, json_object_get_string(json_object_object_get(radvdObj, "X_ZYXEL_DNSServers")));
			zcfgFeJsonObjFree(radvdObj);
			break;
		}
		zcfgFeJsonObjFree(radvdObj);
	}

	if(found){
		curDnsList1 = strtok_r( curDnsList, ",", &tmp_ptr);
		curDnsList2 = strtok_r( NULL, ",", &tmp_ptr);
		curDnsList3 = strtok_r( NULL, ",", &tmp_ptr);

		curDNSServer1 = strtok_r( curDnsServer, ",", &tmp_ptr);
		curDNSServer2 = strtok_r( NULL, ",", &tmp_ptr);
		curDNSServer3 = strtok_r( NULL, ",", &tmp_ptr);
	}
	
	if(curDnsList1 == NULL) curDnsList1 = "FromISP";
	if(curDnsList2 == NULL) curDnsList3 = "FromISP";
	if(curDnsList3 == NULL) curDnsList3 = "FromISP";
	if(curDNSServer1 == NULL) curDNSServer1 = "";
	if(curDNSServer2 == NULL) curDNSServer2 = "";
	if(curDNSServer3 == NULL) curDNSServer3 = "";

	Ipv6_DNSServer1 = json_object_get_string(json_object_object_get(Jobj, "Ipv6_DNSServer1"));
	Ipv6_DNSServer2 = json_object_get_string(json_object_object_get(Jobj, "Ipv6_DNSServer2"));
	Ipv6_DNSServer3 = json_object_get_string(json_object_object_get(Jobj, "Ipv6_DNSServer3"));

	DNSList1 = DNSList2 = DNSList3 = "User";
	DNSServer1 = DNSServer2 = DNSServer3 = "";
	if(Ipv6_DNSServer1 == NULL){
		DNSList1 = curDnsList1;
		DNSServer1 = curDNSServer1;
	}
	else{
		if(!strcmp(Ipv6_DNSServer1, "None") || !strcmp(Ipv6_DNSServer1, "ISP") || !strcmp(Ipv6_DNSServer1, "Proxy"))
			DNSList1 = Ipv6_DNSServer1;
		else
			DNSServer1 = Ipv6_DNSServer1;	
	}
	if(Ipv6_DNSServer2 == NULL){
		DNSList2 = curDnsList2;
		DNSServer2 = curDNSServer2;
	}
	else{
		if(!strcmp(Ipv6_DNSServer2, "None") || !strcmp(Ipv6_DNSServer2, "ISP") || !strcmp(Ipv6_DNSServer2, "Proxy"))
			DNSList2 = Ipv6_DNSServer2;
		else
			DNSServer2 = Ipv6_DNSServer2;	
	}
	if(Ipv6_DNSServer3 == NULL){
		DNSList3 = curDnsList3;
		DNSServer3 = curDNSServer3;
	}
	else{
		if(!strcmp(Ipv6_DNSServer3, "None") || !strcmp(Ipv6_DNSServer3, "ISP") || !strcmp(Ipv6_DNSServer3, "Proxy"))
			DNSList3 = Ipv6_DNSServer3;
		else
			DNSServer3 = Ipv6_DNSServer3;	
	}
	
	sprintf(dnsList, "%s,%s,%s", DNSList1, DNSList2, DNSList3);
	sprintf(dnsServer, "%s,%s,%s", DNSServer1, DNSServer2, DNSServer3);

	json_object_object_add(Jobj, "Ipv6_DNSList", json_object_new_string(dnsList));
	json_object_object_add(Jobj, "Ipv6_DNSServers", json_object_new_string(dnsServer));

	return;
}

bool isLowerLayerUSBorEmpty(struct json_object *subIpIfaceObj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	static struct json_object *pppObj = NULL;
	static struct json_object *ethlinkObj = NULL;
	objIndex_t pppIid = {0};
	objIndex_t ethlinkIid = {0};
	uint8_t index = 0;
	const char *lowLayer = NULL;
	char ipIntLowLayer[32] = {0};

	IID_INIT(pppIid);
	strcpy(ipIntLowLayer, json_object_get_string(json_object_object_get(subIpIfaceObj, "LowerLayers")));

	if(!strncmp(ipIntLowLayer, "PPP.Interface.", 14)){
		sscanf(ipIntLowLayer, "PPP.Interface.%hhu", &index);
		pppIid.idx[0] = index;
		if((ret = zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &pppIid, &pppObj)) == ZCFG_SUCCESS){
			lowLayer = json_object_get_string(json_object_object_get(pppObj, "LowerLayers"));
			if((strlen(lowLayer) == 0) ||(!strncmp(lowLayer, "USB.", 4))){
				zcfgFeJsonObjFree(pppObj);
				return true;
			}
		}

	}else if(!strncmp(ipIntLowLayer, "Ethernet.Link.", 14)){
		sscanf(ipIntLowLayer, "Ethernet.Link.%hhu", &index);
		ethlinkIid.idx[0] = index;
		if((ret = zcfgFeObjJsonGet(RDM_OID_ETH_LINK, &ethlinkIid, &ethlinkObj)) == ZCFG_SUCCESS){
			lowLayer = json_object_get_string(json_object_object_get(ethlinkObj, "LowerLayers"));
			if((strlen(lowLayer) == 0) ||(!strncmp(lowLayer, "USB.", 4))){
				zcfgFeJsonObjFree(ethlinkObj);
				return true;
			}
		}
	}
	return false;
}

bool isNatEnable(){
	struct json_object *natObj = NULL;
	objIndex_t natIid = {0};
	char natIf[32] = {0};
	const char *groupWANIf = NULL;
	bool enable = false;
	uint8_t tmpIndex = 0;
	bool nat = false;
	int i = 0;

	IID_INIT(natIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_NAT_INTF_SETTING, &natIid, &natObj) == ZCFG_SUCCESS) {
		enable = json_object_get_boolean(json_object_object_get(natObj, "Enable"));
		strcpy(natIf, json_object_get_string(json_object_object_get(natObj, "Interface")));
		if(enable == true && strlen(natIf) > 0){
			sscanf(natIf, "IP.Interface.%hhu", &tmpIndex);
			for(i = 0; i < json_object_array_length(allIpIfaceObj); i++) {
				struct json_object *subIpIfaceObj = json_object_array_get_idx(allIpIfaceObj, i);
				if(tmpIndex == (i+1)){
					groupWANIf = json_object_get_string(json_object_object_get(curIpIfaceObj, "X_ZYXEL_Group_WAN_IpIface"));
					if(!isLowerLayerUSBorEmpty(subIpIfaceObj) || strstr(natIf,groupWANIf)){					
						nat = true;
						zcfgFeJsonObjFree(natObj);
						break;
					}
				}
			}
			if(nat == true){
				zcfgFeJsonObjFree(natObj);
				break;
			}
		}
	zcfgFeJsonObjFree(natObj);
	}
	return nat;
}

zcfgRet_t setNat(){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *natObj = NULL;
	objIndex_t natIid = {0};
	const char *natIf = NULL;
	const char *groupWANIf = NULL, *curGroupWANIf = NULL;
	const char *ifName = NULL;
	char *ipInt = NULL, *single_ipInt = NULL, *rest_str = NULL;
	bool enable = false;
	int i = 0;

	IID_INIT(natIid);
	if(isNatEnable()){
		while(zcfgFeObjJsonGetNext(RDM_OID_NAT_INTF_SETTING, &natIid, &natObj) == ZCFG_SUCCESS) {
			natIf = json_object_get_string(json_object_object_get(natObj, "Interface"));
			enable = json_object_get_boolean(json_object_object_get(natObj, "Enable"));
			if(enable == true && strlen(natIf) > 0){
				bool group = false;
				i = 0;
				for(i = 0; i < json_object_array_length(allIpIfaceObj); i++) {
					struct json_object *subIpIfaceObj = json_object_array_get_idx(allIpIfaceObj, i);
					ifName = json_object_get_string(json_object_object_get(subIpIfaceObj, "X_ZYXEL_IfName"));
					curGroupWANIf = json_object_get_string(json_object_object_get(curIpIfaceObj, "X_ZYXEL_Group_WAN_IpIface"));
					if(!strncmp(ifName, "br", 2)){
						groupWANIf = json_object_get_string(json_object_object_get(subIpIfaceObj, "X_ZYXEL_Group_WAN_IpIface"));
						ipInt = (char*)malloc(strlen(groupWANIf)+1);
						memset(ipInt, 0, strlen(groupWANIf)+1);
						memcpy(ipInt, groupWANIf, strlen(groupWANIf)+1);
						single_ipInt = strtok_r( ipInt, ",", &rest_str);
						while(single_ipInt != NULL){
							if(!strcmp(natIf, single_ipInt)){
								if(!strcmp(groupWANIf, curGroupWANIf)){
									json_object_object_add(natObj, "Enable", json_object_new_boolean(false));
									json_object_object_add(natObj, "X_ZYXEL_FullConeEnabled", json_object_new_boolean(false));
									ret = zcfgFeObjJsonSet(RDM_OID_NAT_INTF_SETTING, &natIid, natObj, NULL);
								}
								group = true;
							}
							single_ipInt = strtok_r(NULL, ",", &rest_str);
						}
						free(ipInt);
					}
				}//for

				if(group == false && !strcmp(curGroupWANIf, "")){
					json_object_object_add(natObj, "Enable", json_object_new_boolean(false));
					json_object_object_add(natObj, "X_ZYXEL_FullConeEnabled", json_object_new_boolean(false));
					ret = zcfgFeObjJsonSet(RDM_OID_NAT_INTF_SETTING, &natIid, natObj, NULL);
					group = true;
				}
			}	
			zcfgFeJsonObjFree(natObj);
		}//while
			
	}//if
	return ret;
}

zcfgRet_t setIgmpSnooping(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *igmpSnoopingObj = NULL;
	struct json_object *obj = NULL;
	const char *brtmpname = NULL;
	objIndex_t igmpSnoopingIid = {0};
	char *igmpSnooping = NULL;
	int brsnoopingenable = 0;
	const char *IGMPSnoopingMode = NULL;
	const char *brIfName = NULL;
	char *tmp = NULL;
	char newbrtmp[6] = {0};
	char buff[100] = {0};
	char *brname = NULL;
	char newbrstr[100] = {0};
	objIndex_t iid = {0};
	int len = 0;

	IGMPSnoopingMode = json_object_get_string(json_object_object_get(Jobj, "IGMPSnoopingMode"));
	if(IGMPSnoopingMode == NULL)
		return ret;

	if(!strcmp(IGMPSnoopingMode, "Disabled"))
		brsnoopingenable = 0;
	else if(!strcmp(IGMPSnoopingMode, "Standard"))
		brsnoopingenable = 1;
	else //if(!strcmp(IGMPSnoopingMode, "Blocking"))
		brsnoopingenable = 2;
	

	IID_INIT(iid);
	brtmpname = json_object_get_string(json_object_object_get(Jobj, "Name"));
	if(brtmpname && !strcmp(brtmpname, "Default")){
		brIfName = "br0";
	}
	else{
		while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &iid, &obj) == ZCFG_SUCCESS) {
			if(!strcmp(json_object_get_string(json_object_object_get(obj, "X_ZYXEL_SrvName")),brtmpname)){
				brIfName = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_IfName"));
				if(!strncmp(brIfName, "br", 2)) break;
			}
			zcfgFeJsonObjFree(obj);
		}		
	}
	IID_INIT(igmpSnoopingIid);
	if(brIfName && !strncmp(brIfName, "br", 2) && zcfgFeObjJsonGet(RDM_OID_ZY_IGMP, &igmpSnoopingIid, &igmpSnoopingObj) == ZCFG_SUCCESS){
		igmpSnooping = (char *)json_object_get_string(json_object_object_get(igmpSnoopingObj, "SnoopingBridgeIfName"));	
		if(igmpSnooping != NULL){
			strcpy(buff, igmpSnooping);
			brname = strtok_r(buff, ",", &tmp);
			while(brname != NULL){
				if(strncmp(brIfName,brname,3)){
					strcat(newbrstr, brname);
					strcat(newbrstr, ",");
				}
				else{
					sprintf(newbrtmp,"%s|%d",brIfName,brsnoopingenable);
					strcat(newbrstr, newbrtmp);
					strcat(newbrstr, ",");
				}
				brname = strtok_r(NULL, ",", &tmp);
			}
			len = strlen(newbrstr);
			if(newbrstr[len-1] == ',')
				newbrstr[len-1] = '\0';
		}
		else
			sprintf(newbrstr,"%s|%d", brIfName, brsnoopingenable);

		json_object_object_add(igmpSnoopingObj, "SnoopingBridgeIfName", json_object_new_string(newbrstr));
		zcfgFeObjJsonSet(RDM_OID_ZY_IGMP, &igmpSnoopingIid, igmpSnoopingObj, NULL);
		zcfgFeJsonObjFree(igmpSnoopingObj);
	}
	zcfgFeJsonObjFree(obj);
	return ret;	
}



zcfgRet_t setmldSnooping(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *mldSnoopingObj = NULL;
	struct json_object *obj = NULL;
	objIndex_t mldSnoopingIid = {0};
	char *mldSnooping = NULL;
	int brsnoopingenable = 0;
    const char *MLDSnoopingMode = NULL;
	const char *brtmpname = NULL;
	const char *brIfName = NULL;
	char buff[100] = {0};
	char *brname = NULL;
	char *tmp = NULL;
	char newbrstr[100] = {0};
	char newbrtmp[6] = {0};
	char mldBrSnooping[100] = {0};
	objIndex_t iid = {0};
	int len = 0;

    MLDSnoopingMode = json_object_get_string(json_object_object_get(Jobj, "MLDSnoopingMode"));
    if(MLDSnoopingMode == NULL)
        return ret;
    
    if(!strcmp(MLDSnoopingMode, "Disabled"))
        brsnoopingenable = 0;
    else if(!strcmp(MLDSnoopingMode, "Standard"))
        brsnoopingenable = 1;
    else //if(!strcmp(MLDSnoopingMode, "Blocking"))
        brsnoopingenable = 2;

	IID_INIT(iid);
	brtmpname = json_object_get_string(json_object_object_get(Jobj, "Name"));
	if(brtmpname && !strcmp(brtmpname, "Default")){
		brIfName = "br0";
	}
	else{
		while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &iid, &obj) == ZCFG_SUCCESS) {
			if(!strcmp(json_object_get_string(json_object_object_get(obj, "X_ZYXEL_SrvName")),brtmpname)){
				brIfName = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_IfName"));
				break;
			}
			zcfgFeJsonObjFree(obj);
		}		
	}
	IID_INIT(mldSnoopingIid);
	if(zcfgFeObjJsonGet(RDM_OID_ZY_MLD, &mldSnoopingIid, &mldSnoopingObj) == ZCFG_SUCCESS){
		mldSnooping = (char *)json_object_get_string(json_object_object_get(mldSnoopingObj, "SnoopingBridgeIfName"));	
		if(mldSnooping != NULL){
			strcpy(buff, mldSnooping);
			brname = strtok_r(buff, ",", &tmp);
			while(brname != NULL){
				if(!brIfName || strncmp(brIfName,brname,3)){
					strcat(newbrstr, brname);
					strcat(newbrstr, ",");
				}
				else if(brIfName){
					sprintf(newbrtmp, "%s|%d", brIfName, brsnoopingenable);
					strcat(newbrstr, newbrtmp);
					strcat(newbrstr, ",");
				}
				brname = strtok_r(NULL, ",", &tmp);
			}
			len = strlen(newbrstr);
			if(newbrstr[len-1] == ',')
				newbrstr[len-1] = '\0';
		}
		else
			sprintf(mldBrSnooping, "%s|%d", brIfName, brsnoopingenable);
		json_object_object_add(mldSnoopingObj, "SnoopingBridgeIfName", json_object_new_string(newbrstr));
		zcfgFeObjJsonSet(RDM_OID_ZY_MLD, &mldSnoopingIid, mldSnoopingObj, NULL);
		zcfgFeJsonObjFree(mldSnoopingObj);
	}
	zcfgFeJsonObjFree(obj);
	return ret;	
}

#ifdef ZYXEL_OPAL_EXTENDER
typedef struct def_ip_s {
	bool enableIPv4;
	char v4Addr[sizeof(((rdm_Ip_t *)0)->X_ZYXEL_IPv4AddrDefault)];
} def_ip_t;

/*!
 * @brief It is used to get extend default ip address parameter
 *
 * @param[in] def_ip_p  The buffer to save the default ip parameter
 *
 * @return No return value.
 */
void get_def_ip ( def_ip_t *def_ip_p )
{
	objIndex_t objIid;
	rdm_Ip_t *ip = NULL;
	
	IID_INIT(objIid);
	if ( zcfgFeObjStructGetWithoutUpdate(RDM_OID_IP, &objIid, (void **)(&ip) ) == ZCFG_SUCCESS) {
		def_ip_p->enableIPv4 = ip->X_ZYXEL_EnableIPv4Default;
		ZOS_STRNCPY(def_ip_p->v4Addr,ip->X_ZYXEL_IPv4AddrDefault,sizeof(def_ip_p->v4Addr));
		zcfgFeObjStructFree(ip);
	}
	else {
		def_ip_p->enableIPv4 = false;
		def_ip_p->v4Addr[0] = '\0';
	}
} /* get_def_ip */


zcfgRet_t setExtenderLANIP(struct json_object *Jobj)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    zcfg_offset_t oid;
    struct json_object *v4AddrObj = NULL;
    struct json_object *v4FwdObj = NULL;
    const char *IPAddress = NULL, *CurIPAddress = NULL;
    const char *SubnetMask = NULL, *CurSubnetMask = NULL;
    objIndex_t v4AddrIid = {0};
    objIndex_t v4FwdIid = {0};
    bool            enableDhcpc = true;
    def_ip_t def_ip;

    enableDhcpc = json_object_get_boolean(json_object_object_get(Jobj, "EnableDHCPC"));
    IID_INIT(v4AddrIid);
    v4AddrIid.idx[0] = curIpIfaceIid.idx[0];
    v4AddrIid.idx[1] = 1;
    v4AddrIid.level = 2;

    ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj);

    if(ret != ZCFG_SUCCESS)
    {
        printf("\nFail to get IP v4 Addr\n");
        return ret;
    }

    CurIPAddress = json_object_get_string(json_object_object_get(v4AddrObj, "IPAddress"));
    CurSubnetMask = json_object_get_string(json_object_object_get(v4AddrObj, "SubnetMask"));

    IPAddress = json_object_get_string(json_object_object_get(Jobj, "IPAddress"));
    SubnetMask = json_object_get_string(json_object_object_get(Jobj, "SubnetMask"));

    if (IPAddress == NULL)
    {
        IPAddress = CurIPAddress;
    }
    if (IPAddress == NULL)
    {
        IPAddress = CurIPAddress;
    }
    if (SubnetMask == NULL)
    {
        SubnetMask = CurSubnetMask;
    }

    json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));
    json_object_object_add(v4AddrObj, "AddressingType", json_object_new_string("Static"));

    if (enableDhcpc)
    {
        /* For disable case, do nothing. Because the code run to here only network is available.
           Thus it will have ip address.
           Let's keep the original setting.
           In addition, if link up then everything will be updated.
        */
        get_def_ip(&def_ip);
        if ( def_ip.enableIPv4) {
            json_object_object_add(v4AddrObj, "IPAddress", json_object_new_string(def_ip.v4Addr));
            json_object_object_add(v4AddrObj, "SubnetMask", json_object_new_string("255.255.255.0"));
        }
    }
    else //static
    {
        json_object_object_add(v4AddrObj, "IPAddress", json_object_new_string(IPAddress));
        json_object_object_add(v4AddrObj, "SubnetMask", json_object_new_string(SubnetMask));
    }

    zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, v4AddrObj, NULL);
    zcfgFeJsonObjFree(v4AddrObj);

    if (!enableDhcpc)
    {
        oid = RDM_OID_ROUTING_ROUTER_V4_FWD;
        if(!getSpecificObj(oid, "Interface", json_type_string, "IP.Interface.1", &v4FwdIid, &v4FwdObj)){
            IID_INIT(v4FwdIid);
            v4FwdIid.level = 1;
            v4FwdIid.idx[0] = 1;
            if(zcfgFeObjJsonAdd(oid, &v4FwdIid, NULL) == ZCFG_SUCCESS)
                zcfgFeObjJsonGetWithoutUpdate(oid, &v4FwdIid, &v4FwdObj);
        }
        if(v4FwdObj){
            json_object_object_add(v4FwdObj, "Enable", json_object_new_boolean(true));
            replaceParam(v4FwdObj, "GatewayIPAddress", Jobj, "IPv4_GW");
            zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V4_FWD, &v4FwdIid, v4FwdObj, NULL);
            zcfgFeJsonObjFree(v4FwdObj);
        }
    }

    return ret;
}
#endif
zcfgRet_t setLanIP(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
 	struct json_object *v4AddrObj = NULL;
	const char *IPAddress = NULL, *CurIPAddress = NULL;
	const char *SubnetMask = NULL, *CurSubnetMask = NULL;
	objIndex_t v4AddrIid = {0};

	IID_INIT(v4AddrIid);
	v4AddrIid.idx[0] = curIpIfaceIid.idx[0];
	v4AddrIid.idx[1] = 1;
	v4AddrIid.level = 2;

	ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj);

	if(ret != ZCFG_SUCCESS)
	{
		return ret;
	}

	CurIPAddress = json_object_get_string(json_object_object_get(v4AddrObj, "IPAddress"));
	CurSubnetMask = json_object_get_string(json_object_object_get(v4AddrObj, "SubnetMask"));

	IPAddress = json_object_get_string(json_object_object_get(Jobj, "IPAddress"));
	SubnetMask = json_object_get_string(json_object_object_get(Jobj, "SubnetMask"));

	if (IPAddress == NULL)
	{
		IPAddress = CurIPAddress;
	}
	if (SubnetMask == NULL)
	{
		SubnetMask = CurSubnetMask;
	}

	if (!strcmp(IPAddress,CurIPAddress) && !strcmp(SubnetMask,CurSubnetMask))
	{
		//ret = ZCFG_INVALID_PARAM_VALUE;
		goto done;
	}

	json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(v4AddrObj, "IPAddress", json_object_new_string(IPAddress));
	json_object_object_add(v4AddrObj, "SubnetMask", json_object_new_string(SubnetMask));
	zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, v4AddrObj, NULL);

done:
	zcfgFeJsonObjFree(v4AddrObj);
	return ret;
}

#ifdef ZYXEL_OPAL_EXTENDER
zcfgRet_t setDHCPCState(struct json_object *Jobj)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    struct          json_object *dhcpClientObj = NULL;
    objIndex_t      dhcpClientIid = {0};
    bool            enableDhcpc = true;

    //IPv4
    enableDhcpc = json_object_get_boolean(json_object_object_get(Jobj, "EnableDHCPC"));
    IID_INIT(dhcpClientIid);
    dhcpClientIid.idx[0] = 1;
    dhcpClientIid.level = 1;

    if (zcfgFeObjJsonGet(RDM_OID_DHCPV4_CLIENT, &dhcpClientIid, &dhcpClientObj) != ZCFG_SUCCESS)
    {
        return ZCFG_INTERNAL_ERROR;
    }

    json_object_object_add(dhcpClientObj, "Enable", json_object_new_boolean(enableDhcpc));
    if (enableDhcpc)
    {
        json_object_object_add(dhcpClientObj, "Status", json_object_new_string("Enabled"));
    }
    else
    {
        json_object_object_add(dhcpClientObj, "Status", json_object_new_string("Disabled"));
    }

    zcfgFeObjJsonSet(RDM_OID_DHCPV4_CLIENT, &dhcpClientIid, dhcpClientObj, NULL);
    zcfgFeJsonObjFree(dhcpClientObj);
    return ret;
}

/* original: when set static IP, web cannot set dns server */
/* fixed: when set static IP, web can set set dns server */
/* Yumi Wu 2022/01/21 */
zcfgRet_t setEXTENDERDns(struct json_object *Jobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfg_offset_t oid;
	const char *DNSServer;
	bool DhcpStatus = false;
	struct json_object *dnsClientSrv4Obj = NULL;
	struct json_object *dnsObj = NULL;
	struct json_object *dhcpv4ClientObj = NULL;
	objIndex_t dnsClientSrv4Iid = {0};
	objIndex_t dnsIid = {0};
	objIndex_t dhcpv4ClientIid = {0};

	dbg_printf(" %s: enter\n", __FUNCTION__);

	oid = RDM_OID_DNS_CLIENT_SRV;
	if(!getSpecificObj(oid, "Interface", json_type_string, "IP.Interface.1", &dnsClientSrv4Iid, &dnsClientSrv4Obj)){
		IID_INIT(dnsClientSrv4Iid);
		if(zcfgFeObjJsonAdd(oid, &dnsClientSrv4Iid, NULL) == ZCFG_SUCCESS){;
			zcfgFeObjJsonGetWithoutUpdate(oid, &dnsClientSrv4Iid, &dnsClientSrv4Obj);
		}
	}

	IID_INIT(dhcpv4ClientIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_CLIENT, &dhcpv4ClientIid, &dhcpv4ClientObj) == ZCFG_SUCCESS){
		if(!strcmp((Jgets(dhcpv4ClientObj, "Interface")), "IP.Interface.1")){
			DhcpStatus = json_object_get_boolean(json_object_object_get(dhcpv4ClientObj, "Enable"));
		}
		zcfgFeJsonObjFree(dhcpv4ClientObj);
	}

	if(!DhcpStatus ){ /*static*/
		if(dnsClientSrv4Obj){
			json_object_object_add(dnsClientSrv4Obj, "Enable", json_object_new_boolean(true));
			json_object_object_add(dnsClientSrv4Obj, "X_ZYXEL_Type", json_object_new_string("Static"));
			replaceParam(dnsClientSrv4Obj, "DNSServer", Jobj, "DNSServer");
			zcfgFeObjJsonSet(RDM_OID_DNS_CLIENT_SRV, &dnsClientSrv4Iid, dnsClientSrv4Obj, NULL);
			zcfgFeJsonObjFree(dnsClientSrv4Obj);
		}
	}
	else{/*enable DHCP*/
		if(dnsClientSrv4Obj) {
			zcfgFeJsonObjFree(dnsClientSrv4Obj);
		}
		IID_INIT(dnsClientSrv4Iid);
		while(zcfgFeObjJsonGetNext(RDM_OID_DNS_CLIENT_SRV, &dnsClientSrv4Iid, &dnsClientSrv4Obj) == ZCFG_SUCCESS){
			if(!strcmp((Jgets(dnsClientSrv4Obj, "Interface")), "IP.Interface.1")){
				if(zcfgFeObjJsonDel(RDM_OID_DNS_CLIENT_SRV, &dnsClientSrv4Iid, NULL) != ZCFG_SUCCESS){
					dbg_printf("%s: fail to delete RDM_OID_DNS_CLIENT_SRV obj \n", __FUNCTION__);
				}
			}
			zcfgFeJsonObjFree(dnsClientSrv4Obj);
		}
	}

	IID_INIT(dnsIid);
	if(zcfgFeObjJsonGet(RDM_OID_DNS, &dnsIid, &dnsObj) == ZCFG_SUCCESS){
		zcfgFeObjJsonSet(RDM_OID_DNS, &dnsIid, dnsObj, NULL);
		zcfgFeJsonObjFree(dnsObj);
	}
	return ret;
}
#endif

zcfgRet_t setDHCPState(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dhcpSrvObj = NULL;
	struct json_object *dhcpRelayObj = NULL;
	struct json_object *dhcpServerObj = NULL; // RDM_OID_DHCPV4_SRV
	objIndex_t dhcpSrvIid = {0};
	objIndex_t dhcpRelayIid = {0};
	objIndex_t dhcpServerIid = {0}; // RDM_OID_DHCPV4_SRV
	const char *dhcpIfacePath = NULL, *dhcpRelayPath = NULL;
	const char *dhcpRelayAddr = NULL, *curdhcpRelayAddr = NULL;
	const char *dhcpMinAdr = NULL, *curDhcpMinAdr = NULL;
	const char *dhcpMaxAdr = NULL, *curDhcpMaxAdr = NULL;
	const char *dnsServer1 = NULL, *dnsServer2 = NULL, *curDnsServer = NULL;
	const char *dnsType = NULL, *curDnsType = NULL;
	bool enableDhcp = false, curEnableDhcp = false;
	bool curRelayDhcp = false;
	bool dhcpAutoReserve = false, curDhcpAutoReserve = false;
	int leaseTime = 0, curLeaseTime=0;
	bool setParam = false;
	const char *DHCPType = NULL;
	bool dhcpServerEnable = false;
	char dnsServer[64] = {0};

	// RDM_OID_DHCPV4_SRV
	IID_INIT(dhcpServerIid);
	if(zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV, &dhcpServerIid, &dhcpServerObj) == ZCFG_SUCCESS){
		dhcpServerEnable = json_object_get_boolean(json_object_object_get(dhcpServerObj, "Enable"));
	}

	// RDM_OID_DHCPV4_SRV_POOL
	IID_INIT(dhcpSrvIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_SRV_POOL, &dhcpSrvIid, &dhcpSrvObj) == ZCFG_SUCCESS) {
		dhcpIfacePath = json_object_get_string(json_object_object_get(dhcpSrvObj, "Interface"));
		if(!strcmp(ifacePath, dhcpIfacePath)){
			curEnableDhcp = json_object_get_boolean(json_object_object_get(dhcpSrvObj, "Enable"));
			curDhcpMinAdr = json_object_get_string(json_object_object_get(dhcpSrvObj, "MinAddress"));
			curDhcpMaxAdr = json_object_get_string(json_object_object_get(dhcpSrvObj, "MaxAddress"));
			curDnsServer = json_object_get_string(json_object_object_get(dhcpSrvObj, "DNSServers"));
			curDnsType = json_object_get_string(json_object_object_get(dhcpSrvObj, "X_ZYXEL_DNS_Type"));
			curLeaseTime = json_object_get_int(json_object_object_get(dhcpSrvObj, "LeaseTime"));
			curDhcpAutoReserve = json_object_get_boolean(json_object_object_get(dhcpSrvObj, "X_ZYXEL_AutoReserveLanIp"));
			break;
		}
		zcfgFeJsonObjFree(dhcpSrvObj);
	}
	if(json_object_object_get(Jobj, "EnableDHCP") != NULL){
		enableDhcp = json_object_get_boolean(json_object_object_get(Jobj, "EnableDHCP"));
	}
	else
		enableDhcp = curEnableDhcp;
	
	
	IID_INIT(dhcpRelayIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_RELAY_FWD, &dhcpRelayIid, &dhcpRelayObj) == ZCFG_SUCCESS) {
		dhcpRelayPath = json_object_get_string(json_object_object_get(dhcpRelayObj, "Interface"));
		if(!strcmp(ifacePath, dhcpRelayPath)){
			curRelayDhcp = json_object_get_boolean(json_object_object_get(dhcpRelayObj, "Enable"));
			curdhcpRelayAddr = json_object_get_string(json_object_object_get(dhcpRelayObj, "DHCPServerIPAddress"));
			break;
		}
		zcfgFeJsonObjFree(dhcpRelayObj);
	}

	DHCPType = json_object_get_string(json_object_object_get(Jobj, "DHCPType"));
	if(DHCPType == NULL){
		if(enableDhcp){
			printf("\nInvalid parameter: DHCPType\n");
			ret = ZCFG_INVALID_PARAM_VALUE;
			goto out;
		}
		else{
			if(curRelayDhcp)
				DHCPType = "DHCPRelay";
			else
				DHCPType = "DHCPServer";
		}
	}

	if(enableDhcp && !strcmp(DHCPType, "DHCPRelay")){
		dhcpRelayAddr = json_object_get_string(json_object_object_get(Jobj, "DHCP_RelayAddr"));
		if(dhcpRelayAddr == NULL){
			if(curdhcpRelayAddr == NULL || curdhcpRelayAddr[0] == '\0'){
				printf("\nInvalid parameter: DHCP_RelayAddr\n");
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto out;
			}
			else{
				/*
					For editing LanInfo in Lan card page,
					it will not send dhcpRelayAddr information here,
					so use value stored in json.
				*/
				dhcpRelayAddr = curdhcpRelayAddr;
			}
				
		}

		setNat();
		
		//original enable dhcp, now disable dhcp server but not delete object
		if(curEnableDhcp){
			json_object_object_add(dhcpSrvObj, "Enable", json_object_new_boolean(false));
			zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL, &dhcpSrvIid, dhcpSrvObj, NULL);
		}

		//orignial enable dhcp relay, set dhcp relay
		if(curRelayDhcp){
			if(strcmp(dhcpRelayAddr, curdhcpRelayAddr)){
				setParam = true;
			}
		}else{//orignial disable dhcp relay, add dhcp relay, POST
			IID_INIT(dhcpRelayIid);
			zcfgFeJsonObjFree(dhcpRelayObj);
			ret = zcfgFeObjJsonAdd(RDM_OID_DHCPV4_RELAY_FWD, &dhcpRelayIid, NULL);
			ret = (ret == ZCFG_SUCCESS) ? zcfgFeObjJsonGet(RDM_OID_DHCPV4_RELAY_FWD, &dhcpRelayIid, &dhcpRelayObj) : ret;
			if(ret == ZCFG_SUCCESS){
				//zcfgFeObjJsonGet(RDM_OID_DHCPV4_RELAY_FWD, &dhcpRelayIid, &dhcpRelayObj);
				json_object_object_add(dhcpRelayObj, "Interface", json_object_new_string(ifacePath));
				setParam = true;
			}
		}
		if(setParam){
			json_object_object_add(dhcpRelayObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(dhcpRelayObj, "DHCPServerIPAddress", json_object_new_string(dhcpRelayAddr));
			zcfgFeObjJsonSet(RDM_OID_DHCPV4_RELAY_FWD, &dhcpRelayIid, dhcpRelayObj, NULL);
		}
	}
	else if(enableDhcp && !strcmp(DHCPType, "DHCPServer")){
		dhcpMinAdr = json_object_get_string(json_object_object_get(Jobj, "DHCP_MinAddress"));
		dhcpMaxAdr = json_object_get_string(json_object_object_get(Jobj, "DHCP_MaxAddress"));
		dnsServer1 = json_object_get_string(json_object_object_get(Jobj, "DNS_Servers1"));
		dnsServer2 = json_object_get_string(json_object_object_get(Jobj, "DNS_Servers2"));
		dnsType = json_object_get_string(json_object_object_get(Jobj, "DNS_Type"));
		leaseTime = json_object_get_int(json_object_object_get(Jobj, "DHCP_LeaseTime"));
		dhcpAutoReserve = json_object_get_boolean(json_object_object_get(Jobj, "DHCP_AutoReserveLanIp"));
		
		//original enable dhcp relay, disable dhcp relay server
		if(curRelayDhcp){
			ret = zcfgFeObjJsonDel(RDM_OID_DHCPV4_RELAY_FWD, &dhcpRelayIid, NULL);
			if(dnsType == NULL){
				if(!strcmp(curDnsType, "From ISP"))
					dnsType = "FromISP";
				else if(!strcmp(curDnsType, "DNS Proxy"))
					dnsType = "DNSProxy";
				else //if(!strcmp(curDnsType, "Static"))
					dnsType = "Static";
			}
			if(dnsServer1 == NULL)
				strcat(dnsServer,curDnsServer);
		}
		else{
			if(dhcpMinAdr == NULL || dhcpMaxAdr == NULL || leaseTime == 0 || dnsType == NULL ){
				printf("\nInvalid parameters.\n");
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto out;
			}
		}
		if(!strcmp(dnsType,"Static")){
			if(dnsServer1 == NULL){
				printf("\nInvalid parameter: DNS_Servers1\n");
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto out;
			}
			strcat(dnsServer, dnsServer1);
			if(json_object_object_get(Jobj, "DNS_Servers2")){
				strcat(dnsServer, ",");
				strcat(dnsServer, dnsServer2);
			}
		}

		//original enable dhcp, set dhcp server pool object
		if(curEnableDhcp){
			if((strcmp(dhcpMinAdr, curDhcpMinAdr)) || (strcmp(dhcpMaxAdr, curDhcpMaxAdr)) ||
				(leaseTime != curLeaseTime) || (strcmp(dnsType, curDnsType)) ||
				(dnsServer != NULL && strcmp(dnsServer, curDnsServer)) ||
				(dhcpAutoReserve != curDhcpAutoReserve)){
				setParam = true;
			}
		}
		else{//original disable dhcp, add dhcp server pool object
			setParam = true;
		}

		// 20170725 Max Add
		if(dhcpServerEnable == false){
			json_object_object_add(dhcpServerObj, "Enable", json_object_new_boolean(true));
			zcfgFeObjJsonSetWithoutApply(RDM_OID_DHCPV4_SRV, &dhcpServerIid, dhcpServerObj, NULL);
			sleep(3); // make sure RDM_OID_DHCPV4_SRV already set
		}

		if(setParam){
			fflush(stdout);
			if(!strcmp(dnsType, "DNSProxy"))
				json_object_object_add(dhcpSrvObj, "X_ZYXEL_DNS_Type", json_object_new_string("DNS Proxy"));
			else if(!strcmp(dnsType, "FromISP"))
				json_object_object_add(dhcpSrvObj, "X_ZYXEL_DNS_Type", json_object_new_string("From ISP"));
			else 
				json_object_object_add(dhcpSrvObj, "X_ZYXEL_DNS_Type", json_object_new_string(dnsType));	
			json_object_object_add(dhcpSrvObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(dhcpSrvObj, "MinAddress", json_object_new_string(dhcpMinAdr));
			json_object_object_add(dhcpSrvObj, "MaxAddress", json_object_new_string(dhcpMaxAdr));
			json_object_object_add(dhcpSrvObj, "DNSServers", json_object_new_string(dnsServer));
			json_object_object_add(dhcpSrvObj, "LeaseTime", json_object_new_int(leaseTime));
			json_object_object_add(dhcpSrvObj, "X_ZYXEL_AutoReserveLanIp", json_object_new_boolean(dhcpAutoReserve));
			zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL, &dhcpSrvIid, dhcpSrvObj, NULL);
		}
	}else{
		//original enable dhcp, now disable dhcp server but not delete object
		if(curEnableDhcp){
			setParam = true;
			json_object_object_add(dhcpSrvObj, "Enable", json_object_new_boolean(false));
			zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL, &dhcpSrvIid, dhcpSrvObj, NULL);
		}
		
		//original enable dhcp relay, delete dhcp relay server
		if(curRelayDhcp){
			ret = zcfgFeObjJsonDel(RDM_OID_DHCPV4_RELAY_FWD, &dhcpRelayIid, NULL);
		}
	}

out:
	zcfgFeJsonObjFree(dhcpSrvObj);
	zcfgFeJsonObjFree(dhcpRelayObj);
	zcfgFeJsonObjFree(dhcpServerObj);
	return ret;
}

zcfgRet_t setRadvd(struct json_object *Jobj, bool ip6PreTypeStatic, bool isRTADVApply){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *radvdObj = NULL;
	struct json_object *obj = NULL;
	objIndex_t radvdIid = {0};
	objIndex_t iid = {0};
	const char *inface = NULL;
	bool found = false;
	const char *ip6LanAddrAssign = NULL;
	const char *ip6DnsAssign = NULL;
	const char *dnsList = NULL, *dnsServer = NULL;
	
	IID_INIT(radvdIid);
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_RT_ADV_INTF_SET, &iid, &obj) == ZCFG_SUCCESS) {
		inface = json_object_get_string(json_object_object_get(obj, "Interface"));
		if(!strcmp(inface, ifacePath)){
			memcpy(&radvdIid, &iid, sizeof(objIndex_t));
			radvdObj = obj;
			found = true;
			break;
		}
		zcfgFeJsonObjFree(obj);
	}

	if(!found){//POST
		ret = zcfgFeObjJsonAdd(RDM_OID_RT_ADV_INTF_SET, &radvdIid, NULL);
		if(ret == ZCFG_SUCCESS){
			ret = zcfgFeObjJsonGet(RDM_OID_RT_ADV_INTF_SET, &radvdIid, &radvdObj);
			found = (ret == ZCFG_SUCCESS) ? true : false;
		}
	}

	if(found){
		json_object_object_add(radvdObj, "Enable", json_object_new_boolean(true));
		json_object_object_add(radvdObj, "Interface", json_object_new_string(ifacePath));
		
		ip6LanAddrAssign = json_object_get_string(json_object_object_get(Jobj, "IPv6_LanAddrAssign"));
		if(json_object_object_get(Jobj, "IPv6_LanAddrAssign")){
			if(ip6LanAddrAssign && !strcmp(ip6LanAddrAssign, "Stateless")){
				json_object_object_add(radvdObj, "AdvManagedFlag", json_object_new_boolean(false));
			}
			else{ //if(!strcmp(ip6LanAddrAssign, "Stateful")){
				json_object_object_add(radvdObj, "AdvManagedFlag", json_object_new_boolean(true));
			}
		}

		ip6DnsAssign = json_object_get_string(json_object_object_get(Jobj, "IPv6_DNSAssign"));
		if(json_object_object_get(Jobj, "IPv6_DNSAssign")){
			if(ip6DnsAssign && !strcmp(ip6DnsAssign, "DHCP"))//From DHCP6 server
				json_object_object_add(radvdObj, "AdvOtherConfigFlag", json_object_new_boolean(true));
			else if(ip6DnsAssign && !strcmp(ip6DnsAssign, "RA_DHCP")){//From DHCP6 server and RA
				json_object_object_add(radvdObj, "AdvOtherConfigFlag", json_object_new_boolean(true));
				json_object_object_add(radvdObj, "X_ZYXEL_RAandDHCP6S", json_object_new_boolean(true));
			}
			else{ //if(!strcmp(ip6DnsAssign, "RA")){
				json_object_object_add(radvdObj, "AdvOtherConfigFlag", json_object_new_boolean(false));
			}
		}
		
        if(ip6PreTypeStatic){
			json_object_object_add(radvdObj, "ManualPrefixes", json_object_new_string(manualPrefixes));
        }
        else
            json_object_object_add(radvdObj, "ManualPrefixes", json_object_new_string(""));

 		dnsList = json_object_get_string(json_object_object_get(Jobj, "Ipv6_DNSList"));
		dnsServer = json_object_get_string(json_object_object_get(Jobj, "Ipv6_DNSServers"));

		json_object_object_add(radvdObj, "X_ZYXEL_DNSList", json_object_new_string(dnsList));
		json_object_object_add(radvdObj, "X_ZYXEL_DNSServers", json_object_new_string(dnsServer));
		if(json_object_object_get(Jobj, "IPv6_DNSAssign")){
		if(ip6DnsAssign && !strcmp(ip6DnsAssign, "RA_DHCP"))
			json_object_object_add(radvdObj, "X_ZYXEL_RAandDHCP6S", json_object_new_boolean(true));
        else
			json_object_object_add(radvdObj, "X_ZYXEL_RAandDHCP6S", json_object_new_boolean(false));
		}
		
#ifdef CONFIG_ABZQ_CUSTOMIZATION
		if(json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_WANMtuFlag")))
			json_object_object_add(radvdObj, "X_ZYXEL_WANMtuFlag", json_object_new_boolean(true));
		else
			json_object_object_add(radvdObj, "X_ZYXEL_WANMtuFlag", json_object_new_boolean(false));
#endif		
		zcfgFeObjJsonSet(RDM_OID_RT_ADV_INTF_SET, &radvdIid, radvdObj, NULL);

	}
	zcfgFeJsonObjFree(radvdObj);
	return ret;
}

zcfgRet_t setIp6PreTypeStatic_Disable(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	bool isRTADVApply = false;
	const char *prefixWanName = NULL;
	const char *srvName = NULL;
	const char *iP6PrefixDelegateWAN = NULL;
	char selIPIntf[32] = {0};
	int i = 0;
	
	prefixWanName = json_object_get_string(json_object_object_get(Jobj, "IPv6_PrefixWan"));
	iP6PrefixDelegateWAN = json_object_get_string(json_object_object_get(curIpIfaceObj, "X_ZYXEL_IPv6PrefixDelegateWAN"));
	if(json_object_object_get(Jobj, "IPv6_PrefixWan")){	
    if((prefixWanName && strcmp(prefixWanName, "Default"))){
        for(i = 0; i < json_object_array_length(allIpIfaceObj); i++) {
			struct json_object *subIpIfaceObj = json_object_array_get_idx(allIpIfaceObj, i);
			srvName = json_object_get_string(json_object_object_get(subIpIfaceObj, "X_ZYXEL_SrvName"));
			if(!strcmp(prefixWanName, srvName)){ 
				sprintf(selIPIntf,"IP.Interface.%d",(i+1));		
				if(!strcmp(selIPIntf, iP6PrefixDelegateWAN)){
					isRTADVApply = true;
					break;
				}
        	}
		}//while
    }else{
		if(prefixWanName && iP6PrefixDelegateWAN && !strcmp(prefixWanName, iP6PrefixDelegateWAN)){
			isRTADVApply = true;
    	}
    }
	}
	setRadvd(Jobj, false, isRTADVApply);
	return ret;
}

zcfgRet_t setIp6refixTypeDelegated(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *ip6Origin = NULL;
	const char *IPv6_PrefixWan = NULL;
	const char *ip6Prefix = NULL;
	const char *ip6Id = NULL;
	const char *IPv6_IdentifierType = NULL;
	const char *ip6LanAdrAssign = NULL;
	const char *ip6DNSAssign = NULL;

	ip6Origin = json_object_get_string(json_object_object_get(Jobj, "IPv6_PrefixType"));
	json_object_object_add(curIpIfaceObj, "IPv6Enable", json_object_new_boolean(true));
	if(json_object_object_get(Jobj, "IPv6_PrefixType"))
	json_object_object_add(curIpIfaceObj, "X_ZYXEL_IPv6Origin", json_object_new_string(ip6Origin));
	if(json_object_object_get(Jobj, "IPv6_PrefixType")){
	if(ip6Origin && !strcmp(ip6Origin, "DelegateFromWan")){
		IPv6_PrefixWan = json_object_get_string(json_object_object_get(Jobj, "IPv6_PrefixWan"));
		json_object_object_add(curIpIfaceObj, "X_ZYXEL_IPv6PrefixDelegateWAN", json_object_new_string(IPv6_PrefixWan));
	}else{
		ip6Prefix = json_object_get_string(json_object_object_get(Jobj, "IPv6_Prefix"));
		if(ip6Prefix != NULL)
			json_object_object_add(curIpIfaceObj, "X_ZYXEL_IPv6Prefix", json_object_new_string(ip6Prefix));
	}
	}

	IPv6_IdentifierType = json_object_get_string(json_object_object_get(Jobj, "IPv6_IdentifierType"));
	if(json_object_object_get(Jobj, "IPv6_IdentifierType")){
	if(IPv6_IdentifierType && !strcmp(IPv6_IdentifierType, "EUI64"))
		json_object_object_add(curIpIfaceObj, "X_ZYXEL_IPv6IdentifierType", json_object_new_string("EUI-64"));
	else{ //if(!strcmp(IPv6_IdentifierType, "Manual"))
		ip6Id = json_object_get_string(json_object_object_get(Jobj, "IPv6_Identifier"));
		json_object_object_add(curIpIfaceObj, "X_ZYXEL_IPv6IdentifierType", json_object_new_string("Manual"));
		json_object_object_add(curIpIfaceObj, "X_ZYXEL_IPv6Identifier", json_object_new_string(ip6Id));
	}
	}

	ip6LanAdrAssign = json_object_get_string(json_object_object_get(Jobj, "IPv6_LanAddrAssign"));
	ip6DNSAssign = json_object_get_string(json_object_object_get(Jobj, "IPv6_DNSAssign"));
	if(json_object_object_get(Jobj, "IPv6_LanAddrAssign")){
	if(ip6LanAdrAssign && !strcmp(ip6LanAdrAssign, "Stateless"))
		json_object_object_add(curIpIfaceObj, "X_ZYXEL_AdvManagedFlag", json_object_new_int(0));
	else //if(!strcmp(ip6LanAdrAssign, "Stateful")){
		json_object_object_add(curIpIfaceObj, "X_ZYXEL_AdvManagedFlag", json_object_new_int(1));
	}

	if(json_object_object_get(Jobj, "IPv6_DNSAssign")){
		if(ip6DNSAssign && !strcmp(ip6DNSAssign, "DHCP")){
			json_object_object_add(curIpIfaceObj, "X_ZYXEL_AdvOtherConfigFlag", json_object_new_int(1));
			json_object_object_add(curIpIfaceObj, "X_ZYXEL_RAandDHCP6S", json_object_new_boolean(false));
		}	
		else if(ip6DNSAssign && !strcmp(ip6DNSAssign, "RA_DHCP")){
			json_object_object_add(curIpIfaceObj, "X_ZYXEL_AdvOtherConfigFlag", json_object_new_int(1));
			json_object_object_add(curIpIfaceObj, "X_ZYXEL_RAandDHCP6S", json_object_new_boolean(true));
		}	
		else{//if(!strcmp(ip6DNSAssign, "RA"))
			json_object_object_add(curIpIfaceObj, "X_ZYXEL_AdvOtherConfigFlag", json_object_new_int(0));
			json_object_object_add(curIpIfaceObj, "X_ZYXEL_RAandDHCP6S", json_object_new_boolean(false));
		} 
			
	}
	zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &curIpIfaceIid, curIpIfaceObj, NULL);
	return ret;
}

zcfgRet_t setIp6LocalStaticAddress(struct json_object *Jobj)
{
    zcfgRet_t                   ret = ZCFG_SUCCESS;
    struct json_object          *v6AddrObj = NULL;
    struct json_object          *v6FwdObj = NULL;
    objIndex_t                  v6AddrIid = {0};
    objIndex_t                  v6FwdIid = {0};
    const char                  *ip6Origin = NULL;
    const char                  *ip6FwdOrigin = NULL;
    const char                  *ip6StaticLocalAdr = NULL;
    const char                  *ip6PrefixType = NULL;
    //const char                  *ip6Gateway = NULL;
    zcfg_offset_t               oid;
    bool                        found = false;
    bool                        isStatic = false;
    //char                        sysCmd[128] = {0};

    ip6StaticLocalAdr = json_object_get_string(json_object_object_get(Jobj, "IPv6_StaticAddress"));
    ip6PrefixType = json_object_get_string(json_object_object_get(Jobj, "IPv6_PrefixType"));
    //ip6Gateway = json_object_get_string(json_object_object_get(Jobj, "IPv6_GW"));
    if (ip6PrefixType)
    {
        if (strcmp(ip6PrefixType,"Static") == 0)
        {
            isStatic = true;
        }
    }

    if (isStatic)
    {
        while (zcfgFeSubInObjJsonGetNext(RDM_OID_IP_IFACE_V6_ADDR, &curIpIfaceIid, &v6AddrIid, &v6AddrObj)== ZCFG_SUCCESS)
        {
            ip6Origin = json_object_get_string(json_object_object_get(v6AddrObj, "Origin"));
            if (ip6StaticLocalAdr == NULL)
            {
                zcfgFeJsonObjFree(v6AddrObj);
                return ret;
            }
            if (strcmp(ip6Origin, "Static") == 0)
            {
                found = true;
                json_object_object_add(v6AddrObj, "Enable", json_object_new_boolean(isStatic));
                json_object_object_add(v6AddrObj, "IPAddress", json_object_new_string(ip6StaticLocalAdr));
                zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, v6AddrObj, NULL);
            }
            zcfgFeJsonObjFree(v6AddrObj);
        }

        if (!found)
        {   //Post
            IID_INIT(v6AddrIid);
            memcpy(&v6AddrIid, &curIpIfaceIid, sizeof(objIndex_t));
            v6AddrIid.level = 1;
            ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, NULL);
            if (ret == ZCFG_SUCCESS)
            {
                ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, &v6AddrObj);
                if (ret == ZCFG_SUCCESS)
                {
                    json_object_object_add(v6AddrObj, "Enable", json_object_new_boolean(isStatic));
                    json_object_object_add(v6AddrObj, "Origin", json_object_new_string("Static"));
                    json_object_object_add(v6AddrObj, "IPAddress", json_object_new_string(ip6StaticLocalAdr));
                    zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, v6AddrObj, NULL);
                    zcfgFeJsonObjFree(v6AddrObj);
                }
            }
        }
        oid = RDM_OID_ROUTING_ROUTER_V6_FWD;

        IID_INIT(v6FwdIid);
        found = false;

        while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &v6FwdIid, &v6FwdObj) == ZCFG_SUCCESS)
        {
            ip6FwdOrigin = json_object_get_string(json_object_object_get(v6FwdObj, "Origin"));
            if (strcmp(ip6FwdOrigin,"Static") == 0)
            {
                found = true;
                break;
            }
            zcfgFeJsonObjFree(v6FwdObj);
        }

        if (!found)
        {
            v6FwdIid.level = 1;
            v6FwdIid.idx[0] = 1;
            zcfgFeObjJsonAdd(oid, &v6FwdIid, NULL);
            zcfgFeObjJsonGetWithoutUpdate(oid, &v6FwdIid, &v6FwdObj);
        }
        json_object_object_add(v6FwdObj, "Enable", json_object_new_boolean(true));
        json_object_object_add(v6FwdObj, "Origin", json_object_new_string("Static"));
        json_object_object_add(v6FwdObj, "Interface", json_object_new_string("IP.Interface.1"));
        json_object_object_add(v6FwdObj, "DestIPPrefix", json_object_new_string("::/0"));
        replaceParam(v6FwdObj, "NextHop", Jobj, "IPv6_GW");
        zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V6_FWD, &v6FwdIid, v6FwdObj, NULL);
        zcfgFeJsonObjFree(v6FwdObj);
    }
    else //!isStatic
    {
        IID_INIT(v6AddrIid);
        while (zcfgFeSubInObjJsonGetNext(RDM_OID_IP_IFACE_V6_ADDR, &curIpIfaceIid, &v6AddrIid, &v6AddrObj)== ZCFG_SUCCESS)
        {
            ip6Origin = json_object_get_string(json_object_object_get(v6AddrObj, "Origin"));
            if (strcmp(ip6Origin, "Static") == 0)
            {
                if (zcfgFeObjJsonDel(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, NULL) != ZCFG_SUCCESS)
                {
                    dbg_printf("%s: fail to delete RDM_OID_IP_IFACE_V6_ADDR obj \n", __FUNCTION__);
                }
            }
            zcfgFeJsonObjFree(v6AddrObj);
        }

        IID_INIT(v6FwdIid);
        while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &v6FwdIid, &v6FwdObj) == ZCFG_SUCCESS)
        {
            ip6FwdOrigin = json_object_get_string(json_object_object_get(v6FwdObj, "Origin"));
            if (strcmp(ip6FwdOrigin,"Static") == 0)
            {
                if (zcfgFeObjJsonDel(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, NULL) != ZCFG_SUCCESS)
                {
                    dbg_printf("%s: fail to delete RDM_OID_IP_IFACE_V6_ADDR obj \n", __FUNCTION__);
                }
            }
            zcfgFeJsonObjFree(v6FwdObj);
        }
    }


    return ret;
}

zcfgRet_t setIp6LinkLocalEUI64(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *v6AddrObj = NULL;
	objIndex_t v6AddrIid = {0};
	const char *IPv6_LinkLocalAddressType = NULL;
	const char *ip6Origin = NULL;
	const char *ip6LinkLocalAdr = NULL, *curIp6LinkLocalAdr = NULL;
	bool found = false;
	char tmp[5] = {0};

	IPv6_LinkLocalAddressType = json_object_get_string(json_object_object_get(Jobj, "IPv6_LinkLocalAddressType"));

	IID_INIT(v6AddrIid);
	if(IPv6_LinkLocalAddressType != NULL && !strcmp(IPv6_LinkLocalAddressType, "EUI64")){
		while(zcfgFeSubInObjJsonGetNext(RDM_OID_IP_IFACE_V6_ADDR, &curIpIfaceIid, &v6AddrIid, &v6AddrObj)== ZCFG_SUCCESS){
			ip6Origin = json_object_get_string(json_object_object_get(v6AddrObj, "Origin"));
			curIp6LinkLocalAdr = json_object_get_string(json_object_object_get(v6AddrObj, "IPAddress"));
			ZOS_STRNCPY(tmp,curIp6LinkLocalAdr,sizeof(tmp));  //The tmp size should be 4+1
			if(!strcmp(ip6Origin, "Static") && !strcasecmp(tmp, "fe80")){
				ret = zcfgFeObjJsonDel(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, NULL);
			}
			zcfgFeJsonObjFree(v6AddrObj);
		}
	}else{
		ip6LinkLocalAdr = json_object_get_string(json_object_object_get(Jobj, "IPv6_LinkLocalAddress"));
		if(ip6LinkLocalAdr == NULL)
			return ret;
		else{
			invalidCharFilter((char *)ip6LinkLocalAdr, 26);
			if(IPv6Filter(ip6LinkLocalAdr) == false){
				printf("invalid local ipv6 address...\n");
				return false;
			}
		}
		
		while(zcfgFeSubInObjJsonGetNext(RDM_OID_IP_IFACE_V6_ADDR, &curIpIfaceIid, &v6AddrIid, &v6AddrObj)== ZCFG_SUCCESS){
			ip6Origin = json_object_get_string(json_object_object_get(v6AddrObj, "Origin"));
			curIp6LinkLocalAdr = json_object_get_string(json_object_object_get(v6AddrObj, "IPAddress"));
			ZOS_STRNCPY(tmp,curIp6LinkLocalAdr,sizeof(tmp));  //The tmp size should be 4+1
			if(!strcmp(ip6Origin, "Static") && !strcasecmp(tmp, "fe80")){
				found = true;
				json_object_object_add(v6AddrObj, "Enable", json_object_new_boolean(true));
				json_object_object_add(v6AddrObj, "IPAddress", json_object_new_string(ip6LinkLocalAdr));
				zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, v6AddrObj, NULL);
			}
			zcfgFeJsonObjFree(v6AddrObj);
		}

		if(!found){//Post
			IID_INIT(v6AddrIid);
			zcfgFeJsonObjFree(v6AddrObj);
			memcpy(&v6AddrIid, &curIpIfaceIid, sizeof(objIndex_t));
			v6AddrIid.level = 1;
			ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, NULL);
			ret = (ret == ZCFG_SUCCESS) ? zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, &v6AddrObj) : ret;
			if(ret == ZCFG_SUCCESS){
				//zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, &v6AddrObj);
				json_object_object_add(v6AddrObj, "Enable", json_object_new_boolean(true));
				json_object_object_add(v6AddrObj, "IPAddress", json_object_new_string(ip6LinkLocalAdr));
				zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, v6AddrObj, NULL);
				zcfgFeJsonObjFree(v6AddrObj);
			}
		}
	}
	
	return ret;
}

zcfgRet_t setIp6PreTypeNonStatic(struct json_object *Jobj, bool isEnable)
{
    zcfgRet_t                   ret = ZCFG_SUCCESS;
    struct json_object          *v6prefixObj = NULL;
    struct json_object          *v6FwdObj = NULL;
    objIndex_t                  v6prefixIid = {0};
    objIndex_t                  v6FwdIid = {0};
    const char                  *ipv6Origin = NULL;
    const char                  *ipv6FwdOrigin = NULL;
    bool                        isCurPrefixEnable = true;
    bool                        found = false;

    IID_INIT(v6prefixIid);
    while (zcfgFeSubInObjJsonGetNext(RDM_OID_IP_IFACE_V6_PREFIX, &curIpIfaceIid, &v6prefixIid, &v6prefixObj)== ZCFG_SUCCESS){
        ipv6Origin = json_object_get_string(json_object_object_get(v6prefixObj, "Origin"));
        isCurPrefixEnable = json_object_get_boolean(json_object_object_get(v6prefixObj, "Enable"));
        if (isCurPrefixEnable &&
           ((strcmp(ipv6Origin, TR181_PREFIX_RA) == 0) ||
            (strcmp(ipv6Origin, TR181_PREFIX_AUTO) == 0)))
        {
            found = true;
            break;
        }
        zcfgFeJsonObjFree(v6prefixObj);
    }

    if (found)
    {
        json_object_object_add(v6prefixObj, "Enable", json_object_new_boolean(isEnable));
        zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_PREFIX, &v6prefixIid, v6prefixObj, NULL);
        zcfgFeJsonObjFree(v6prefixObj);
    }



    if (isEnable == false)
    {
        IID_INIT(v6FwdIid);
        while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &v6FwdIid, &v6FwdObj) == ZCFG_SUCCESS)
        {
            ipv6FwdOrigin = json_object_get_string(json_object_object_get(v6FwdObj, "Origin"));
            if ((strcmp(ipv6FwdOrigin, TR181_FWD_RA) == 0) ||
                (strcmp(ipv6FwdOrigin, TR181_FWD_DHCPV6) == 0))
            {
                zcfgFeJsonObjFree(v6FwdObj);
                zcfgFeObjJsonDel(RDM_OID_ROUTING_ROUTER_V6_FWD, &v6FwdIid, NULL);
                break;
            }
            zcfgFeJsonObjFree(v6FwdObj);
        }
    }

    return ret;
}

zcfgRet_t setIp6PreTypeStatic_Enable(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *v6prefixObj = NULL;
	objIndex_t v6prefixIid = {0};
	const char * v6PrefixAddress = NULL;
	bool found = false;
	const char *ip6LanAdrAssign = NULL;
	const char *ipv6Origin = NULL;
	IID_INIT(v6prefixIid);
	while(zcfgFeSubInObjJsonGetNext(RDM_OID_IP_IFACE_V6_PREFIX, &curIpIfaceIid, &v6prefixIid, &v6prefixObj)== ZCFG_SUCCESS)
	{
		ipv6Origin = json_object_get_string(json_object_object_get(v6prefixObj, "Origin"));
		if (ipv6Origin && !strcmp(ipv6Origin, "Static")) {
			found = true;
			break;
		}
		zcfgFeJsonObjFree(v6prefixObj);
	}

	if(!found){//Post
		IID_INIT(v6prefixIid);
		zcfgFeJsonObjFree(v6prefixObj);
		memcpy(&v6prefixIid, &curIpIfaceIid, sizeof(objIndex_t));
		ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE_V6_PREFIX, &v6prefixIid, NULL);
		if(ret == ZCFG_SUCCESS){
			zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_PREFIX, &v6prefixIid, &v6prefixObj);
			found = true;
		}
	}

	if(found){
		sprintf(manualPrefixes, "IP.Interface.%hhu.IPv6Prefix.%hhu", v6prefixIid.idx[0], v6prefixIid.idx[1]);
		
		v6PrefixAddress = json_object_get_string(json_object_object_get(Jobj, "IPv6_Prefix"));
		json_object_object_add(v6prefixObj, "Enable", json_object_new_boolean(true));
		json_object_object_add(v6prefixObj, "Prefix", json_object_new_string(v6PrefixAddress));

		if(json_object_object_get(Jobj, "IPv6_LanAddrAssign")){
			ip6LanAdrAssign = json_object_get_string(json_object_object_get(Jobj, "IPv6_LanAddrAssign"));
		
			if(ip6LanAdrAssign && !strcmp(ip6LanAdrAssign, "Stateless"))
				json_object_object_add(v6prefixObj, "Autonomous", json_object_new_boolean(true));
			else //if(!strcmp(ip6LanAdrAssign, "Stateful"))
				json_object_object_add(v6prefixObj, "Autonomous", json_object_new_boolean(false));
		}

		zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_PREFIX, &v6prefixIid, v6prefixObj, NULL);
	}

	setRadvd(Jobj, true, true);
	
	zcfgFeJsonObjFree(v6prefixObj);
	return ret;
}

zcfgRet_t setDHCPv6(struct json_object *Jobj, bool ip6PreTypeStatic){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dhcp6SrvObj = NULL;;
	objIndex_t dhcp6sPoolIid = {0};
	const char *inface = NULL;
	bool found = false;
	const char *ip6LanAddrAssign = NULL;
	const char *ip6DnsAssign = NULL;
	const char *dnsList = NULL, *dnsServer = NULL;
	const char *minAddress = NULL, *maxAddress = NULL;
	const char *ipv6DomainName = NULL;

	IID_INIT(dhcp6sPoolIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV6_SRV_POOL, &dhcp6sPoolIid, &dhcp6SrvObj) == ZCFG_SUCCESS) {
		inface = json_object_get_string(json_object_object_get(dhcp6SrvObj, "Interface"));
		if(!strcmp(inface, ifacePath)){
			found = true;
			break;
		}
		zcfgFeJsonObjFree(dhcp6SrvObj);
	}

	if(!found){//Post
		IID_INIT(dhcp6sPoolIid);
		ret = zcfgFeObjJsonAdd(RDM_OID_DHCPV6_SRV_POOL, &dhcp6sPoolIid, NULL);
		if(ret == ZCFG_SUCCESS){
			zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV_POOL, &dhcp6sPoolIid, &dhcp6SrvObj);
			found = true;
		}
	}

	if(found){
		json_object_object_add(dhcp6SrvObj, "Enable", json_object_new_boolean(true));
		json_object_object_add(dhcp6SrvObj, "Interface", json_object_new_string(ifacePath));

		ip6LanAddrAssign = json_object_get_string(json_object_object_get(Jobj, "IPv6_LanAddrAssign"));
		if(json_object_object_get(Jobj, "IPv6_LanAddrAssign")){
		if(ip6LanAddrAssign && !strcmp(ip6LanAddrAssign, "Stateless")){
			json_object_object_add(dhcp6SrvObj, "X_ZYXEL_MinAddress", json_object_new_string(""));
			json_object_object_add(dhcp6SrvObj, "X_ZYXEL_MaxAddress", json_object_new_string(""));
		}
		else{ //if(!strcmp(ip6LanAddrAssign, "Stateful")){
			minAddress = json_object_get_string(json_object_object_get(Jobj, "Ipv6_MinAddress"));
			maxAddress = json_object_get_string(json_object_object_get(Jobj, "Ipv6_MaxAddress"));
			if(minAddress != NULL)
				json_object_object_add(dhcp6SrvObj, "X_ZYXEL_MinAddress", json_object_new_string(minAddress));
			if(maxAddress != NULL)
				json_object_object_add(dhcp6SrvObj, "X_ZYXEL_MaxAddress", json_object_new_string(maxAddress));
		}
		}
		
		ip6DnsAssign = json_object_get_string(json_object_object_get(Jobj, "IPv6_DNSAssign"));
		if(ip6DnsAssign != NULL && !strcmp(ip6DnsAssign, "RA_DHCP")){
			json_object_object_add(dhcp6SrvObj, "X_ZYXEL_RAandDHCP6S", json_object_new_boolean(true));
		}else
			json_object_object_add(dhcp6SrvObj, "X_ZYXEL_RAandDHCP6S", json_object_new_boolean(false));

		ipv6DomainName = json_object_get_string(json_object_object_get(Jobj, "Ipv6_DNSSuffix"));

		if(ipv6DomainName != NULL)
			json_object_object_add(dhcp6SrvObj, "X_ZYXEL_DNSSuffix", json_object_new_string(ipv6DomainName));
	
		dnsList = json_object_get_string(json_object_object_get(Jobj, "Ipv6_DNSList"));
		dnsServer = json_object_get_string(json_object_object_get(Jobj, "Ipv6_DNSServers"));

		json_object_object_add(dhcp6SrvObj, "X_ZYXEL_DNSList", json_object_new_string(dnsList));
		json_object_object_add(dhcp6SrvObj, "X_ZYXEL_DNSServers", json_object_new_string(dnsServer));
		json_object_object_add(dhcp6SrvObj, "IANAEnable", json_object_new_boolean(true));

		if(ip6PreTypeStatic)
			json_object_object_add(dhcp6SrvObj, "IANAManualPrefixes", json_object_new_string(dnsList));
		else
			json_object_object_add(dhcp6SrvObj, "IANAManualPrefixes", json_object_new_string(""));

		zcfgFeObjJsonBlockedSet(RDM_OID_DHCPV6_SRV_POOL, &dhcp6sPoolIid, dhcp6SrvObj, NULL);
	}
	
	zcfgFeJsonObjFree(dhcp6SrvObj);
	return ret;
}

zcfgRet_t setDNSQueryScenario(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dnsObj = NULL;
	objIndex_t dnsIid = {0};
	int dnsQueryScenario = 0;
	const char *DNSQueryScenario = NULL;
	if((DNSQueryScenario = json_object_get_string(json_object_object_get(Jobj, "DNSQueryScenario"))) == NULL)
		return ret;
	
	if(!strcmp(DNSQueryScenario, "IPv4IPv6"))
		dnsQueryScenario = 0;
	else if(!strcmp(DNSQueryScenario, "IPv6Only"))
		dnsQueryScenario = 1;
	else if(!strcmp(DNSQueryScenario, "IPv4Only"))
		dnsQueryScenario = 2;
	else if(!strcmp(DNSQueryScenario, "IPv6First"))
		dnsQueryScenario = 3;
	else //if(!strcmp(DNSQueryScenario, "IPv4First"))
		dnsQueryScenario = 4;

	IID_INIT(dnsIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DNS, &dnsIid, &dnsObj);
	if(ret == ZCFG_SUCCESS){
		json_object_object_add(dnsObj, "X_ZYXEL_DNSQueryScenario", json_object_new_int(dnsQueryScenario));
		zcfgFeObjJsonBlockedSet(RDM_OID_DNS, &dnsIid, dnsObj, NULL);
		zcfgFeJsonObjFree(dnsObj);
	}

	return ret;
}

zcfgRet_t getLanInfo(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *bridgingObj = NULL;
	struct json_object *obj = NULL;
	objIndex_t bridgingIid = {0};
	objIndex_t iid = {0};
	const char *brName = NULL;
	const char *IfName = NULL;
	int selectIndex = 0;
	int brIndex = 0;
	int i = 0;

	IID_INIT(bridgingIid);
	selbrName = json_object_get_string(json_object_object_get(Jobj, "Name"));
	while(zcfgFeObjJsonGetNext(RDM_OID_BRIDGING_BR, &bridgingIid, &bridgingObj) == ZCFG_SUCCESS) {
		brName = json_object_get_string(json_object_object_get(bridgingObj, "X_ZYXEL_BridgeName"));
		if(!strcmp(selbrName, brName)){
			zcfgFeJsonObjFree(bridgingObj);
			break;
		}
			selectIndex++;                                           
		zcfgFeJsonObjFree(bridgingObj);
    }

	allIpIfaceObj = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &iid, &obj) == ZCFG_SUCCESS) {
		json_object_array_add(allIpIfaceObj, obj);
	}

	for(i = 0; i < json_object_array_length(allIpIfaceObj); i++) {
		struct json_object *subIpIfaceObj = json_object_array_get_idx(allIpIfaceObj, i);
		IfName = json_object_get_string(json_object_object_get(subIpIfaceObj, "X_ZYXEL_IfName"));
		if(!strncmp(IfName, "br", 2)){
			if(selectIndex == brIndex){//found
				curIpIfaceObj = subIpIfaceObj;
				curIpIfaceIid.idx[0] = (i+1);
				curIpIfaceIid.level = 1;
				sprintf(ifacePath, "IP.Interface.%hhu", curIpIfaceIid.idx[0]);
				break;
			}
			else
				brIndex++;
		}
	}
	return ret;
}

#if 0
zcfgRet_t zcfgFeDal_LanSetup_Get(struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t bridgingIid;
	struct json_object *bridgingObj = NULL;
	struct json_object *pramJobj = NULL;
	const char *Name = NULL;
	
	IID_INIT(bridgingIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_BRIDGING_BR, &bridgingIid, &bridgingObj) == ZCFG_SUCCESS) {
		pramJobj = json_object_new_object();
		Name = json_object_get_string(json_object_object_get(bridgingObj, "X_ZYXEL_BridgeName"));
		                                         
		zcfgFeJsonObjFree(bridgingObj);
    }
}
#endif

zcfgRet_t zcfgFeDal_LanSetup_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	bool ipv6LanEnable = false;
	bool ip6PreTypeStatic = false;
	const char *IPv6_PrefixType = NULL;
	
#ifdef CONFIG_ABZQ_CUSTOMIZATION
	struct json_object *radvdObj = NULL;
	objIndex_t radvdIid = {0};
#endif

#ifdef ZYXEL_OPAL_EXTENDER
	const char *ip6LanAdrAssign = NULL;
#endif

#ifdef ZYXEL_TAAAB_CUSTOMIZATION_V6
	struct json_object *ttIpV6LanObj = NULL;
	objIndex_t objIid;
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_V6_LAN, &objIid, &ttIpV6LanObj)) != ZCFG_SUCCESS){
			printf("%s: retrieve RDM_OID_IP_V6_LAN object fail\n", __FUNCTION__);
			return ret;
	}
#endif

	initGlobalLanObjs();
	getLanInfo(Jobj);
	json_object_object_add(curIpIfaceObj, "X_ZYXEL_SrvName", json_object_new_string(selbrName));

#ifdef ZYXEL_OPAL_EXTENDER
	ret = setExtenderLANIP(Jobj);
	if(ret != ZCFG_SUCCESS)
	    goto out;
#else
	ret = setLanIP(Jobj);
	if(ret != ZCFG_SUCCESS)
		goto out;
#endif
    if(json_object_object_get(Jobj, "IPv6_LanEnable"))
        ipv6LanEnable = json_object_get_boolean(json_object_object_get(Jobj, "IPv6_LanEnable"));
    else
        ipv6LanEnable = json_object_get_boolean(json_object_object_get(curIpIfaceObj, "IPv6Enable"));

    if (ipv6LanEnable)
    {
        IPv6_PrefixType = json_object_get_string(json_object_object_get(Jobj, "IPv6_PrefixType"));
        if(json_object_object_get(Jobj, "IPv6_PrefixType")){
        if(!strcmp(IPv6_PrefixType, "Static"))
            ip6PreTypeStatic = true;
        }
    }


#ifdef CONFIG_ABZQ_CUSTOMIZATION

IID_INIT(radvdIid);

while(zcfgFeObjJsonGetNext(RDM_OID_RT_ADV_INTF_SET, &radvdIid, &radvdObj) == ZCFG_SUCCESS)
{		
		if(json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_WANMtuFlag"))){
			json_object_object_add(radvdObj, "X_ZYXEL_WANMtuFlag", json_object_new_boolean(true));
			}
		else{
			json_object_object_add(radvdObj, "X_ZYXEL_WANMtuFlag", json_object_new_boolean(false));
			}
}

zcfgFeObjJsonSet(RDM_OID_RT_ADV_INTF_SET, &radvdIid, radvdObj, NULL);
zcfgFeJsonObjFree(radvdObj);

#endif	


	

#ifdef ZYXEL_OPAL_EXTENDER
	ret = setDHCPCState(Jobj);
    if(ret != ZCFG_SUCCESS)
        goto out;
    if (ipv6LanEnable && (json_object_object_get(Jobj, "IPv6_PrefixType")))
    {
        if(ip6PreTypeStatic)
        {
            json_object_object_add(curIpIfaceObj, "X_ZYXEL_IPv6Origin", json_object_new_string("Static"));
            zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &curIpIfaceIid, curIpIfaceObj, NULL);
            //disable auto
            setIp6PreTypeNonStatic(Jobj,false);
            //enable static , change NextHop
            setIp6LocalStaticAddress(Jobj);
        }
        else
        {
            //IPv6_LanAddrAssign : Stateful or Stateless
            //disable static , disable NextHop
            setIp6LocalStaticAddress(Jobj);

            ip6LanAdrAssign = json_object_get_string(json_object_object_get(Jobj, "IPv6_LanAddrAssign"));
            if (!strcmp(ip6LanAdrAssign, "Stateless"))
            {
                json_object_object_add(curIpIfaceObj, "X_ZYXEL_AdvManagedFlag", json_object_new_int(0));
            }
            else //if(!strcmp(ip6LanAdrAssign, "Stateful")){
            {
                json_object_object_add(curIpIfaceObj, "X_ZYXEL_AdvManagedFlag", json_object_new_int(1));
            }
            json_object_object_add(curIpIfaceObj, "X_ZYXEL_IPv6Origin", json_object_new_string("DelegateFromWan"));
            zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &curIpIfaceIid, curIpIfaceObj, NULL);
        }
    }
#else //!ZYXEL_OPAL_EXTENDER
	ret = setDHCPState(Jobj);
	if(ret != ZCFG_SUCCESS)
		goto out;

	ret = setIgmpSnooping(Jobj);
	if(ret != ZCFG_SUCCESS)
		goto out;

/*
	if(json_object_object_get(Jobj, "IPv6_LanEnable"))
		ipv6LanEnable = json_object_get_boolean(json_object_object_get(Jobj, "IPv6_LanEnable"));
	else
		ipv6LanEnable = json_object_get_boolean(json_object_object_get(curIpIfaceObj, "IPv6Enable"));
*/
	if(ipv6LanEnable){
		convertIPv6DnsList(Jobj);
/*
		IPv6_PrefixType = json_object_get_string(json_object_object_get(Jobj, "IPv6_PrefixType"));
		if(json_object_object_get(Jobj, "IPv6_PrefixType")){
		if(!strcmp(IPv6_PrefixType, "Static"))
			ip6PreTypeStatic = true;
		}
*/
		setmldSnooping(Jobj);
#ifdef ZYXEL_TAAAB_CUSTOMIZATION_V6
		json_object_object_add(ttIpV6LanObj, "Enabled", json_object_new_boolean(true));
		zcfgFeObjJsonSetWithoutApply(RDM_OID_IP_V6_LAN, &objIid, ttIpV6LanObj, NULL);
#endif
		setIp6refixTypeDelegated(Jobj);

		if(!ip6PreTypeStatic){
			//setIp6PreTypeStatic_Disable(Jobj);
		}
		else{	
			setIp6PreTypeStatic_Enable(Jobj);
		}
		setIp6LinkLocalEUI64(Jobj);

		setDHCPv6(Jobj,ip6PreTypeStatic);
		setDNSQueryScenario(Jobj);
	}else{
#ifdef ZYXEL_TAAAB_CUSTOMIZATION_V6
		json_object_object_add(ttIpV6LanObj, "Enabled", json_object_new_boolean(false));
		zcfgFeObjJsonSetWithoutApply(RDM_OID_IP_V6_LAN, &objIid, ttIpV6LanObj, NULL);
#endif		
		json_object_object_add(curIpIfaceObj, "IPv6Enable", json_object_new_boolean(false));
		zcfgFeObjJsonBlockedSet(RDM_OID_IP_IFACE, &curIpIfaceIid, curIpIfaceObj, NULL);
	}
#endif //#ifdef ZYXEL_OPAL_EXTENDER

#ifdef ZYXEL_OPAL_EXTENDER
	/* original: when set static IP, web cannot set dns server */
	/* fixed: when set static IP, web can set set dns server */
	/* Yumi Wu 2022/01/21 */
	/* do this after SetDHCPCState() */
	ret = setEXTENDERDns(Jobj);
	if(ret != ZCFG_SUCCESS)
	    goto out;
#endif

out:
#ifdef ZYXEL_TAAAB_CUSTOMIZATION_V6
	zcfgFeJsonObjFree(ttIpV6LanObj);
#endif
	freeAllLanObjs();
	return ret;
}

zcfgRet_t zcfgFeDal_LanSetup_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *ipifaceObj = NULL;
    struct json_object *ethLinkObj = NULL;
    struct json_object *bridgeBrObj = NULL;
	struct json_object *ipv4addrObj= NULL;
	struct json_object *ipv6addrObj= NULL;
	struct json_object *dhcpv4fwdObj = NULL;
	struct json_object *dhcpv4srvObj = NULL;
	struct json_object *dhcpv6srvObj = NULL;
	struct json_object *dnsObj = NULL;
	struct json_object *rtadvObj = NULL;
	struct json_object *igmpSnoopingObj = NULL;
	struct json_object *mldSnoopingObj = NULL;
	objIndex_t ipifaceIid = {0};
    objIndex_t ethLinkIid = {0};
    objIndex_t bridgeBrIid = {0};
	objIndex_t ipv4addrIid = {0};
	objIndex_t ipv6addrIid = {0};
	objIndex_t dhcpv4fwdIid = {0};
	objIndex_t dhcpv4srvIid = {0};
	objIndex_t dhcpv6srvIid = {0};
	objIndex_t dnsIid = {0};
	objIndex_t rtadvIid = {0};
	objIndex_t igmpSnoopingIid = {0};
	objIndex_t mldSnoopingIid = {0};
	bool dchprelay = false;
	bool showdetail = false;
	int DNSQueryScenario = 0;
	const char *dnstype = NULL, *dnsserver = NULL, *IPv6_IdentifierType = NULL, *IPv6_PrefixType = NULL, *v6DNSServer = NULL, *v6DNSList = NULL, *Name = NULL, *curName = NULL;
	const char *igmpSnooping = NULL, *sub_igmpSnooping = NULL, *mldSnooping = NULL, *sub_mldSnooping = NULL,*ifName = NULL;
	char currintf[32] = {0}, buff[32] = {0}, v6DNSServerTmp[256] = {0}, v6DNSListTmp[256] = {0}, brName[100]={0}, igmpType[100] = {0}, mldType[8] = {0};
	char *dns1 = NULL, *dns2 = NULL, *v6dnstype1 = NULL, *v6dnstype2 = NULL, *v6dnstype3 = NULL, *v6dnsServer1 = NULL, *v6dnsServer2 = NULL, *v6dnsServer3 = NULL, *v6dnstypetmp = NULL, *ptr = NULL, *brPtr = NULL;
	const char *ip6Origin = NULL, *v6IPAddress = NULL, *LowerLayers = NULL;

#ifdef ZYXEL_OPAL_EXTENDER
    struct json_object          *dhcpv4ClientObj = NULL;
    struct json_object          *v6FwdObj = NULL;
    struct json_object          *v4FwdObj = NULL;
    objIndex_t                  v6FwdIid = {0};
    objIndex_t                  dhcpv4ClientIid = {0};
    objIndex_t                  v4FwdIid = {0};
    const char                  *dhcpcIntf = NULL;
    const char                  *v6Origin = NULL;
	struct json_object          *DNSSrvCObj = NULL;
	objIndex_t                  DNSSrvCIid = {0};
#endif

	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipifaceIid, &ipifaceObj) == ZCFG_SUCCESS){
		if(showdetail){
			break;
		}
		if(!strcmp("", Jgets(ipifaceObj,"X_ZYXEL_SrvName")))
			Jadds(ipifaceObj, "X_ZYXEL_SrvName", "Default");
	
		if(!strncmp(json_object_get_string(json_object_object_get(ipifaceObj, "X_ZYXEL_IfName")), "br", 2)){
            dchprelay = false;
			paramJobj = json_object_new_object();
			curName = json_object_get_string(json_object_object_get(ipifaceObj, "X_ZYXEL_SrvName"));
			ifName = json_object_get_string(json_object_object_get(ipifaceObj, "X_ZYXEL_IfName"));
            
            //For Bridge interface, curName should be Device.Bridging.Bridge.i.X_ZYXEL_BridgeName
            LowerLayers = Jgets(ipifaceObj, "LowerLayers");
            sscanf(LowerLayers, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
            ethLinkIid.level = 1;
            if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkObj) == ZCFG_SUCCESS)
            {
                LowerLayers = Jgets(ethLinkObj, "LowerLayers");
                sscanf(LowerLayers, "Bridging.Bridge.%hhu.Port.", &bridgeBrIid.idx[0]);
                bridgeBrIid.level = 1;
                zcfgFeJsonObjFree(ethLinkObj);
                if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_BRIDGING_BR, &bridgeBrIid, &bridgeBrObj) == ZCFG_SUCCESS)
                {
                    curName = Jgets(bridgeBrObj, "X_ZYXEL_BridgeName");
                    Jadds(ipifaceObj, "X_ZYXEL_SrvName", curName);
                    zcfgFeJsonObjFree(bridgeBrObj);
                }
            }
			
			if(json_object_object_get(Jobj, "Name")){
				Name = json_object_get_string(json_object_object_get(Jobj, "Name"));
				if(!strcmp(Name,curName))
					showdetail = true;
			}
			json_object_object_add(paramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(ipifaceObj, "X_ZYXEL_SrvName")));
#ifdef ZYXEL_OPAL_EXTENDER
			IID_INIT(dhcpv4ClientIid);
			dhcpv4ClientIid.idx[0] = 1;
			dhcpv4ClientIid.level = 1;
			if (zcfgFeObjJsonGet(RDM_OID_DHCPV4_CLIENT, &dhcpv4ClientIid, &dhcpv4ClientObj) == ZCFG_SUCCESS)
			{
			    dhcpcIntf = Jgets(dhcpv4ClientObj, "Interface");
			    if (strcmp(dhcpcIntf, "IP.Interface.1") == 0)
			    {
                    json_object_object_add(paramJobj, "EnableDHCPC", JSON_OBJ_COPY(json_object_object_get(dhcpv4ClientObj, "Enable")));
                    //json_object_object_add(paramJobj, "IPv4_GW", JSON_OBJ_COPY(json_object_object_get(dhcpv4ClientObj, "IPRouters")));
			    }
			    zcfgFeJsonObjFree(dhcpv4ClientObj);
			}
			else
			{
			    dbg_printf("%s: fail to get RDM_OID_DHCPV4_CLIENT obj \n", __FUNCTION__);
			}

			if(!getSpecificObj(RDM_OID_ROUTING_ROUTER_V4_FWD, "Interface", json_type_string, "IP.Interface.1", &v4FwdIid, &v4FwdObj))
			{
			    json_object_object_add(paramJobj, "IPv4_GW", json_object_new_string(""));
			}
			else
			{
			    json_object_object_add(paramJobj, "IPv4_GW", JSON_OBJ_COPY(json_object_object_get(v4FwdObj, "GatewayIPAddress")));
			}
			if(!getSpecificObj(RDM_OID_DNS_CLIENT_SRV, "Interface", json_type_string, "IP.Interface.1", &DNSSrvCIid, &DNSSrvCObj))
			{
			    json_object_object_add(paramJobj, "DNSServer", json_object_new_string(""));
			}
			else
			{
			    json_object_object_add(paramJobj, "DNSServer", JSON_OBJ_COPY(json_object_object_get(DNSSrvCObj, "DNSServer")));
				if (DNSSrvCObj)zcfgFeJsonObjFree(DNSSrvCObj);
			}
#endif
			ipv4addrIid.level = 2;
			ipv4addrIid.idx[0] = ipifaceIid.idx[0];
			ipv4addrIid.idx[1] = 1;
			if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipv4addrIid, &ipv4addrObj) == ZCFG_SUCCESS){
				json_object_object_add(paramJobj, "IPAddress", JSON_OBJ_COPY(json_object_object_get(ipv4addrObj, "IPAddress")));
				json_object_object_add(paramJobj, "SubnetMask", JSON_OBJ_COPY(json_object_object_get(ipv4addrObj, "SubnetMask")));

				zcfgFeJsonObjFree(ipv4addrObj);
			}
			sprintf(currintf,"IP.Interface.%u",ipifaceIid.idx[0]);	// IP.Interface.1 , IP.Interface.6 , IP.Interface.7
			IID_INIT(dhcpv4fwdIid);
			while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_RELAY_FWD, &dhcpv4fwdIid, &dhcpv4fwdObj) == ZCFG_SUCCESS){
				if(!strcmp(json_object_get_string(json_object_object_get(dhcpv4fwdObj, "Interface")),currintf)){
					json_object_object_add(paramJobj, "DHCPType", json_object_new_string("DHCPRelay"));
					json_object_object_add(paramJobj, "DHCP_RelayAddr", JSON_OBJ_COPY(json_object_object_get(dhcpv4fwdObj, "DHCPServerIPAddress")));
					json_object_object_add(paramJobj, "EnableDHCP", json_object_new_boolean(true));
					dchprelay = true;
					zcfgFeJsonObjFree(dhcpv4fwdObj);
					break;
				}
				zcfgFeJsonObjFree(dhcpv4fwdObj);
			}

			IID_INIT(dhcpv4srvIid);
			while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_SRV_POOL, &dhcpv4srvIid, &dhcpv4srvObj) == ZCFG_SUCCESS){
				if(!strcmp(json_object_get_string(json_object_object_get(dhcpv4srvObj, "Interface")),currintf) && !dchprelay){
					json_object_object_add(paramJobj, "DHCPType", json_object_new_string("DHCPServer"));
					if(json_object_get_boolean(json_object_object_get(dhcpv4srvObj, "Enable")))
						json_object_object_add(paramJobj, "EnableDHCP", json_object_new_boolean(true));
					else
						json_object_object_add(paramJobj, "EnableDHCP", json_object_new_boolean(false));

					json_object_object_add(paramJobj, "DHCP_MinAddress", JSON_OBJ_COPY(json_object_object_get(dhcpv4srvObj, "MinAddress")));
					json_object_object_add(paramJobj, "DHCP_MaxAddress", JSON_OBJ_COPY(json_object_object_get(dhcpv4srvObj, "MaxAddress")));
					json_object_object_add(paramJobj, "DHCP_AutoReserveLanIp", JSON_OBJ_COPY(json_object_object_get(dhcpv4srvObj, "X_ZYXEL_AutoReserveLanIp")));
					json_object_object_add(paramJobj, "DHCP_LeaseTime", JSON_OBJ_COPY(json_object_object_get(dhcpv4srvObj, "LeaseTime")));
					dnstype = json_object_get_string(json_object_object_get(dhcpv4srvObj, "X_ZYXEL_DNS_Type"));
					if(!strcmp(dnstype, "DNS Proxy") )
						json_object_object_add(paramJobj, "DNS_Type", json_object_new_string("DNSProxy"));
					else if(!strcmp(dnstype, "Static") || !strcmp(dnstype, "From ISP")){
						if(!strcmp(dnstype, "Static")){
							json_object_object_add(paramJobj, "DNS_Type", json_object_new_string("Static"));
							dnsserver = json_object_get_string(json_object_object_get(dhcpv4srvObj, "DNSServers"));
							strcpy(buff, dnsserver);
							dns1 = strtok_r(buff, ",", &dns2);
							json_object_object_add(paramJobj, "DNS_Servers1", json_object_new_string(dns1));
							if(dns2 != NULL){
								json_object_object_add(paramJobj, "DNS_Servers2", json_object_new_string(dns2));
							}
						}
						else if(!strcmp(dnstype, "From ISP")){
							json_object_object_add(paramJobj, "DNS_Type", json_object_new_string("FromISP"));
						}
					}
					zcfgFeJsonObjFree(dhcpv4srvObj);
					break;

				}
				zcfgFeJsonObjFree(dhcpv4srvObj);	
			}

			//IGMP snooping
			IID_INIT(igmpSnoopingIid);
			if(zcfgFeObjJsonGet(RDM_OID_ZY_IGMP, &igmpSnoopingIid, &igmpSnoopingObj) == ZCFG_SUCCESS){
				igmpSnooping = json_object_get_string(json_object_object_get(igmpSnoopingObj, "SnoopingBridgeIfName"));	
				if(igmpSnooping != NULL){
					memset(buff, 0, sizeof(buff));
					brPtr = NULL;
					strcpy(buff, igmpSnooping);
					sub_igmpSnooping = strtok_r(buff, ",", &brPtr);
					while(sub_igmpSnooping != NULL){
						if(!strncmp(ifName,sub_igmpSnooping, 3)){
							sscanf(sub_igmpSnooping, "%[^|]%s", brName, igmpType); 
							if(!strcmp(igmpType,"|0"))
								json_object_object_add(paramJobj, "IGMPSnoopingMode", json_object_new_string("Disabled"));
							else if(!strcmp(igmpType,"|1"))
								json_object_object_add(paramJobj, "IGMPSnoopingMode", json_object_new_string("Standard"));
							else 
								json_object_object_add(paramJobj, "IGMPSnoopingMode", json_object_new_string("Blocking"));
							break;

						}
						sub_igmpSnooping = strtok_r(NULL, ",", &brPtr);
					}
				}
				zcfgFeJsonObjFree(igmpSnoopingObj);
			}

			json_object_object_add(paramJobj, "IPv6_LanEnable", JSON_OBJ_COPY(json_object_object_get(ipifaceObj, "IPv6Enable")));
			if(json_object_get_boolean(json_object_object_get(ipifaceObj, "IPv6Enable"))){	//IPv6
				IID_INIT(ipv6addrIid);
				ipv6addrIid.level = 2;
				ipv6addrIid.idx[0] = ipifaceIid.idx[0];
				json_object_object_add(paramJobj, "IPv6_LinkLocalAddressType", json_object_new_string("EUI64"));
				while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE_V6_ADDR, &ipv6addrIid, &ipv6addrObj) == ZCFG_SUCCESS){
					ip6Origin = json_object_get_string(json_object_object_get(ipv6addrObj, "Origin"));
					v6IPAddress = json_object_get_string(json_object_object_get(ipv6addrObj, "IPAddress"));
#ifdef ZYXEL_OPAL_EXTENDER
					if(!strcmp(ip6Origin, "Static"))
					{
					    json_object_object_add(paramJobj, "IPv6_StaticAddress", JSON_OBJ_COPY(json_object_object_get(ipv6addrObj, "IPAddress")));
					}
#endif
					if(!strcmp(ip6Origin, "Static") && !strncasecmp(v6IPAddress, "fe80", 4)){
						json_object_object_add(paramJobj, "IPv6_LinkLocalAddressType", json_object_new_string("Manual"));
						json_object_object_add(paramJobj, "IPv6_LinkLocalAddress", JSON_OBJ_COPY(json_object_object_get(ipv6addrObj, "IPAddress")));
						zcfgFeJsonObjFree(ipv6addrObj);
						break;
					}
					zcfgFeJsonObjFree(ipv6addrObj);
				}

				IPv6_IdentifierType = json_object_get_string(json_object_object_get(ipifaceObj, "X_ZYXEL_IPv6IdentifierType"));
				json_object_object_add(paramJobj, "IPv6_IdentifierType", json_object_new_string(IPv6_IdentifierType));
				if(!strcmp(IPv6_IdentifierType, "Manual"))
					json_object_object_add(paramJobj, "IPv6_Identifier", JSON_OBJ_COPY(json_object_object_get(ipifaceObj, "X_ZYXEL_IPv6Identifier")));
				IPv6_PrefixType = json_object_get_string(json_object_object_get(ipifaceObj, "X_ZYXEL_IPv6Origin"));
				json_object_object_add(paramJobj, "IPv6_PrefixType", json_object_new_string(IPv6_PrefixType));
				if(!strcmp(IPv6_PrefixType ,"Static"))
					json_object_object_add(paramJobj, "IPv6_Prefix", JSON_OBJ_COPY(json_object_object_get(ipifaceObj, "X_ZYXEL_IPv6Prefix")));
				else if(!strcmp(IPv6_PrefixType, "DelegateFromWan")){
					json_object_object_add(paramJobj, "IPv6_PrefixWan", JSON_OBJ_COPY(json_object_object_get(ipifaceObj, "X_ZYXEL_IPv6PrefixDelegateWAN")));
				}

#ifdef ZYXEL_OPAL_EXTENDER
				IID_INIT(v6FwdIid);
				while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &v6FwdIid, &v6FwdObj) == ZCFG_SUCCESS)
				{
				    v6Origin = json_object_get_string(json_object_object_get(v6FwdObj, "Origin"));
				    if (json_object_get_boolean(json_object_object_get(v6FwdObj, "Enable")) == true)
				    {
				        if (strcmp (v6Origin, "Static") == 0)
				        {
				            json_object_object_add(paramJobj, "IPv6_GW", JSON_OBJ_COPY(json_object_object_get(v6FwdObj, "NextHop")));
				            zcfgFeJsonObjFree(v6FwdObj);
				            break;
				        }
				    }
				    zcfgFeJsonObjFree(v6FwdObj);
				}
#endif
				//MLD snooping
				IID_INIT(mldSnoopingIid);
				if(zcfgFeObjJsonGet(RDM_OID_ZY_MLD, &mldSnoopingIid, &mldSnoopingObj) == ZCFG_SUCCESS){
					mldSnooping = json_object_get_string(json_object_object_get(mldSnoopingObj, "SnoopingBridgeIfName")); 
					if(mldSnooping != NULL){
						memset(buff, 0, sizeof(buff));
						brPtr = NULL;
						strcpy(buff, mldSnooping);
						sub_mldSnooping = strtok_r(buff, ",", &brPtr);
						while(sub_mldSnooping != NULL){
							if(!strncmp(ifName,sub_mldSnooping, 3)){
								sscanf(sub_mldSnooping, "%[^|]%s", brName, mldType); 
								if(!strcmp(mldType,"|0"))
									json_object_object_add(paramJobj, "MLDSnoopingMode", json_object_new_string("Disabled"));
								else if(!strcmp(mldType,"|1"))
									json_object_object_add(paramJobj, "MLDSnoopingMode", json_object_new_string("Standard"));
								else 
									json_object_object_add(paramJobj, "MLDSnoopingMode", json_object_new_string("Blocking"));
								break;
							}
							sub_mldSnooping = strtok_r(NULL, ",", &brPtr);

						}
					}
					zcfgFeJsonObjFree(mldSnoopingObj);
				}

				IID_INIT(rtadvIid);
				while(zcfgFeObjJsonGetNext(RDM_OID_RT_ADV_INTF_SET, &rtadvIid, &rtadvObj) == ZCFG_SUCCESS){
					if(!strcmp(json_object_get_string(json_object_object_get(rtadvObj, "Interface")),currintf)){
						
#ifdef CONFIG_ABZQ_CUSTOMIZATION			

						if(json_object_get_boolean(json_object_object_get(rtadvObj, "X_ZYXEL_WANMtuFlag"))){
							json_object_object_add(paramJobj, "X_ZYXEL_WANMtuFlag", json_object_new_boolean(true));
							}
						else{
							json_object_object_add(paramJobj, "X_ZYXEL_WANMtuFlag", json_object_new_boolean(false));
							}
#endif

						if(json_object_get_boolean(json_object_object_get(rtadvObj, "AdvOtherConfigFlag"))){
							if(json_object_get_boolean(json_object_object_get(rtadvObj, "X_ZYXEL_RAandDHCP6S")))
								json_object_object_add(paramJobj, "IPv6_DNSAssign", json_object_new_string("RA_DHCP"));
							else
								json_object_object_add(paramJobj, "IPv6_DNSAssign", json_object_new_string("DHCP"));
						}
						else
							json_object_object_add(paramJobj, "IPv6_DNSAssign", json_object_new_string("RA"));
						zcfgFeJsonObjFree(rtadvObj);
						break;
					}
					zcfgFeJsonObjFree(rtadvObj);
				}

				IID_INIT(rtadvIid);
				while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV6_SRV_POOL, &dhcpv6srvIid, &dhcpv6srvObj) == ZCFG_SUCCESS){
					if(!strcmp(currintf, json_object_get_string(json_object_object_get(dhcpv6srvObj, "Interface")))){ 
					if(json_object_get_int(json_object_object_get(ipifaceObj, "X_ZYXEL_AdvManagedFlag")) == 1){
						json_object_object_add(paramJobj, "IPv6_LanAddrAssign", json_object_new_string("Stateful"));
						json_object_object_add(paramJobj, "Ipv6_MinAddress", JSON_OBJ_COPY(json_object_object_get(dhcpv6srvObj, "X_ZYXEL_MinAddress")));
						json_object_object_add(paramJobj, "Ipv6_MaxAddress", JSON_OBJ_COPY(json_object_object_get(dhcpv6srvObj, "X_ZYXEL_MaxAddress")));
						json_object_object_add(paramJobj, "Ipv6_DNSSuffix", JSON_OBJ_COPY(json_object_object_get(dhcpv6srvObj, "X_ZYXEL_DNSSuffix")));
					}
					else
						json_object_object_add(paramJobj, "IPv6_LanAddrAssign", json_object_new_string("Stateless"));
					
					v6DNSServer = json_object_get_string(json_object_object_get(dhcpv6srvObj, "X_ZYXEL_DNSServers"));
					v6DNSList = json_object_get_string(json_object_object_get(dhcpv6srvObj, "X_ZYXEL_DNSList")); 							
					strcpy(v6DNSServerTmp,v6DNSServer);
					strcpy(v6DNSListTmp,v6DNSList);
					v6dnstype1 = strtok_r(v6DNSListTmp, ",", &v6dnstypetmp);
					v6dnstype2 = strtok_r(NULL, ",", &v6dnstypetmp);
					v6dnstype3 = strtok_r(NULL, ",", &v6dnstypetmp);
					v6dnsServer1=v6DNSServerTmp;

					if(strlen(v6dnsServer1) >0)
					{

						*(ptr = strchr(v6dnsServer1, ',')) = '\0';
						v6dnsServer2=ptr+1;
						*(ptr = strchr(v6dnsServer2, ',')) = '\0';
						v6dnsServer3=ptr+1;
					}
					
					if(strlen(v6DNSListTmp) >0)
					{
						if(!strcmp(v6dnstype1, "None") || !strcmp(v6dnstype1, "ISP") ||!strcmp(v6dnstype1, "Proxy"))
							json_object_object_add(paramJobj, "Ipv6_DNSServer1", json_object_new_string(v6dnstype1));
						else
							json_object_object_add(paramJobj, "Ipv6_DNSServer1", json_object_new_string(v6dnsServer1));
						if(!strcmp(v6dnstype2, "None") || !strcmp(v6dnstype2, "ISP") || !strcmp(v6dnstype2, "Proxy"))
							json_object_object_add(paramJobj, "Ipv6_DNSServer2", json_object_new_string(v6dnstype2));
						else
							json_object_object_add(paramJobj, "Ipv6_DNSServer2", json_object_new_string(v6dnsServer2));
						if(!strcmp(v6dnstype3, "None") || !strcmp(v6dnstype3, "ISP") || !strcmp(v6dnstype3, "Proxy"))
							json_object_object_add(paramJobj, "Ipv6_DNSServer3", json_object_new_string(v6dnstype3));
						else
							json_object_object_add(paramJobj, "Ipv6_DNSServer3", json_object_new_string(v6dnsServer3));
					}
					else
					{
						json_object_object_add(paramJobj, "Ipv6_DNSServer1", json_object_new_string(""));
						json_object_object_add(paramJobj, "Ipv6_DNSServer2", json_object_new_string(""));
						json_object_object_add(paramJobj, "Ipv6_DNSServer3", json_object_new_string(""));
					}
					zcfgFeJsonObjFree(dhcpv6srvObj);
					break;
				}
					zcfgFeJsonObjFree(dhcpv6srvObj);
				}
				if(zcfgFeObjJsonGet(RDM_OID_DNS, &dnsIid, &dnsObj) == ZCFG_SUCCESS){
					DNSQueryScenario = json_object_get_int(json_object_object_get(dnsObj, "X_ZYXEL_DNSQueryScenario"));
					if(DNSQueryScenario == 0)
						json_object_object_add(paramJobj, "DNSQueryScenario", json_object_new_string("IPv4IPv6"));
					else if(DNSQueryScenario == 1)
						json_object_object_add(paramJobj, "DNSQueryScenario", json_object_new_string("IPv6Only"));
					else if(DNSQueryScenario == 2)
						json_object_object_add(paramJobj, "DNSQueryScenario", json_object_new_string("IPv4Only"));
					else if(DNSQueryScenario == 3)
						json_object_object_add(paramJobj, "DNSQueryScenario", json_object_new_string("IPv6First"));
					else if(DNSQueryScenario == 4)
						json_object_object_add(paramJobj, "DNSQueryScenario", json_object_new_string("IPv4First"));
					zcfgFeJsonObjFree(dnsObj);
				}
			}
			if(json_object_object_get(Jobj, "Name")){
				if(showdetail){
					json_object_object_add(paramJobj, "ShowDetail", json_object_new_boolean(true));
					json_object_array_add(Jarray, paramJobj);
				}
			}
			else
			json_object_array_add(Jarray, paramJobj);
		}
		zcfgFeJsonObjFree(ipifaceObj);
	}

	zcfgFeJsonObjFree(ipifaceObj);
	return ret;

}

void zcfgFeDalShowLanSetup(struct json_object *Jarray){
	struct json_object *obj = NULL, *obj_idx = NULL;
	int len = 0, i;
	int leasetime = 0, min = 0, hour = 0, day = 0;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	len = json_object_array_length(Jarray);
	obj_idx = json_object_array_get_idx(Jarray, 0);
	if(len == 1 && json_object_get_boolean(json_object_object_get(obj_idx, "ShowDetail"))){
	obj = json_object_array_get_idx(Jarray, 0);
	printf("%-15s %s \n","Group Name",json_object_get_string(json_object_object_get(obj, "Name")));
	printf("%-15s %s \n","IP Address",json_object_get_string(json_object_object_get(obj, "IPAddress")));
	printf("%-15s %s \n","Subnet Mask",json_object_get_string(json_object_object_get(obj, "SubnetMask")));

	
	if(!strcmp("DHCPRelay",json_object_get_string(json_object_object_get(obj, "DHCPType")))){
		printf("%-15s %s \n","DHCP","DHCP Relay");
		printf("%-20s %-10s\n", "DHCP Relay Address", json_object_get_string(json_object_object_get(obj, "DHCP_RelayAddr")));
	} else {
		if(json_object_get_boolean(json_object_object_get(obj, "EnableDHCP"))){
			printf("%-15s %s \n","DHCP","Enable");
			printf("%-2s %-25s %s \n", "", "Beginning IP Address",json_object_get_string(json_object_object_get(obj, "DHCP_MinAddress")));
			printf("%-2s %-25s %s \n", "", "Ending IP Address",json_object_get_string(json_object_object_get(obj, "DHCP_MaxAddress")));
			printf("%-2s %-25s %s \n", "", "AutoReserveLanIp",json_object_get_string(json_object_object_get(obj, "DHCP_AutoReserveLanIp")));
			leasetime = json_object_get_int(json_object_object_get(obj, "DHCP_LeaseTime"));
			leasetime = leasetime / 60;
			min = leasetime % 60;
			leasetime = (leasetime - (leasetime % 60)) / 60;
			hour = leasetime % 24;
			leasetime = (leasetime - (leasetime % 24)) / 24;
			day = leasetime;
			printf("%-2s %-25s %d %s %d %s %d %s \n", "", "DHCP Server Lease Time",day,"Days",hour,"Hours",min,"Minutes");
			printf("%-2s %-15s %s \n", "", "DNS Values",json_object_get_string(json_object_object_get(obj, "DNS_Type")));
		}else{
			printf("%-15s %s \n","DHCP","Disable");
		}
	}
	printf("%-15s %s \n","IPv6 Active",json_object_get_string(json_object_object_get(obj, "IPv6_LanEnable")));
	if(json_object_get_boolean(json_object_object_get(obj, "IPv6_LanEnable"))){
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPv6_LinkLocalAddressType")),"Manual"))
			printf("%-30s %-10s %-10s\n", "Link Local Address Type", json_object_get_string(json_object_object_get(obj, "IPv6_LinkLocalAddressType")),json_object_get_string(json_object_object_get(obj, "IPv6_LinkLocalAddress")));
		else if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPv6_LinkLocalAddressType")),"EUI64"))
			printf("%-30s %-10s\n", "Link Local Address Type", json_object_get_string(json_object_object_get(obj, "IPv6_LinkLocalAddressType")));
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPv6_IdentifierType")),"Manual"))
				printf("%-30s %-10s %-10s\n", "LAN Global Identifier Type", json_object_get_string(json_object_object_get(obj, "IPv6_IdentifierType")),json_object_get_string(json_object_object_get(obj, "IPv6_Identifier")));
			else if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPv6_IdentifierType")),"EUI-64"))
				printf("%-30s %-10s\n", "LAN Global Identifier Type", json_object_get_string(json_object_object_get(obj, "IPv6_IdentifierType")));
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPv6_PrefixType")),"DelegateFromWan"))
			printf("%-30s %-30s %-10s\n", "LAN IPv6 Prefix Setup", "Delegate prefix from WAN", json_object_get_string(json_object_object_get(obj, "IPv6_PrefixWan")));
		else if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPv6_PrefixType")),"Static"))
			printf("%-30s %-10s %-10s\n", "LAN IPv6 Prefix Setup", "Static", json_object_get_string(json_object_object_get(obj, "IPv6_Prefix")));
		printf("%-30s %-10s\n", "LAN IPv6 Address Assign Setup", json_object_get_string(json_object_object_get(obj, "IPv6_LanAddrAssign")));
		printf("%-30s %-10s\n", "LAN IPv6 DNS Assign Setup", json_object_get_string(json_object_object_get(obj, "IPv6_DNSAssign")));
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPv6_DNSAssign")), "RA"))
			printf("%-30s %-10s\n", "DHCPv6 Active", "DHCPv6 Disable");
		else
			printf("%-30s %-10s\n", "DHCPv6 Active", "DHCPv6 Server");
			if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPv6_LanAddrAssign")),"Stateful")){
				printf("%-30s %-10s\n", "IPv6 Start Address", json_object_get_string(json_object_object_get(obj, "IPv6_MinAddress")));
				printf("%-30s %-10s\n", "IPv6 End Address", json_object_get_string(json_object_object_get(obj, "Ipv6_MaxAddress")));
				printf("%-30s %-10s\n", "IPv6 Domain Name Server", json_object_get_string(json_object_object_get(obj, "Ipv6_DNSSuffix")));
			}
			printf("IPv6 DNS Values \n");
			if(!strcmp(json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer1")),"None") || !strcmp(json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer1")),"ISP")){
				printf("%-2s %-25s %-12s\n", "", "IPv6 DNS Server 1", json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer1")));
			} else {
				printf("%-2s %-25s %-12s %-10s\n", "", "IPv6 DNS Server 1", "User Defined", json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer1")));
			}
			if(!strcmp(json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer2")),"None") || !strcmp(json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer2")),"ISP")){
				printf("%-2s %-25s %-12s\n", "", "IPv6 DNS Server 2", json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer2")));
			} else {
				printf("%-2s %-25s %-12s %-10s\n", "", "IPv6 DNS Server 2", "User Defined", json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer2")));
			}
			if(!strcmp(json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer3")),"None") || !strcmp(json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer3")),"ISP")){
				printf("%-2s %-25s %-12s\n", "", "IPv6 DNS Server 3", json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer3")));
			} else {
				printf("%-2s %-25s %-12s %-10s\n", "", "IPv6 DNS Server 3", "User Defined", json_object_get_string(json_object_object_get(obj, "Ipv6_DNSServer3")));
			}
			printf("%-30s ", "DNS Query Scenario");		
			if(!strcmp(json_object_get_string(json_object_object_get(obj, "DNSQueryScenario")), "IPv4IPv6"))
				printf("%-10s\n", "IPv4/IPv6 DNS Server");		
			else if(!strcmp(json_object_get_string(json_object_object_get(obj, "DNSQueryScenario")), "IPv6Only"))
				printf("%-10s\n", "IPv6 DNS Server Only");		
			else if(!strcmp(json_object_get_string(json_object_object_get(obj, "DNSQueryScenario")), "IPv4Only"))
				printf("%-10s\n", "IPv4 DNS Server Only");		
			else if(!strcmp(json_object_get_string(json_object_object_get(obj, "DNSQueryScenario")), "IPv6First"))
				printf("%-10s\n", "IPv6 DNS Server First");		
			else //if(!strcmp(json_object_get_string(json_object_object_get(obj, "DNSQueryScenario")), "IPv4First"))
				printf("%-10s\n", "IPv4 DNS Server First");		

		}
	}
	else{
		printf("%-15s %-20s %-20s %-15s \n", "Group Name", "LAN IP Address", "IPv4 DHCP Server", "IPv6 Enable");
		for(i=0;i<len;i++){
			obj = json_object_array_get_idx(Jarray, i);
			if(json_object_get_boolean(json_object_object_get(obj, "EnableDHCP"))){
				printf("%-15s %-20s %-20s %-15s \n", 
					json_object_get_string(json_object_object_get(obj, "Name")),
					json_object_get_string(json_object_object_get(obj, "IPAddress")),
					"Enable",
					json_object_get_string(json_object_object_get(obj, "IPv6_LanEnable")));
			}else{
				printf("%-15s %-20s %-20s %-15s \n", 
					json_object_get_string(json_object_object_get(obj, "Name")),
					json_object_get_string(json_object_object_get(obj, "IPAddress")),
					"Disable",
					json_object_get_string(json_object_object_get(obj, "IPv6_LanEnable")));
			}
		}
	}
}




zcfgRet_t zcfgFeDalLanSetup(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDal_LanSetup_Edit(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_LanSetup_Get(Jobj, Jarray, NULL); 
	else
		printf("Unknown method:%s\n", method);
	//else if(!strcmp(method, "GET"))
		//ret = zcfgFeDal_LanSetup_Get(Jobj, NULL);
	
	
	return ret;
}

