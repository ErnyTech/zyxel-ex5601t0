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
#include "zcfg_fe_dal.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_net.h"
#if defined(SAME_FW_FOR_TWO_BOARD)
#include "zyutil_cofw.h"
#endif
#include "zlog_api.h"
#include "zos_api.h"
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
#include <math.h>
#define OPT_61_VAL_ENCODE_ENCVAL 256
#define OPT_125_VAL_ENCODE_ENCVAL 256
#endif
//#define JSON_OBJ_COPY(json_object) json_tokener_parse(json_object_to_json_string(json_object))

dal_param_t WAN_param[]={
	{"Index",				dalType_int,	0,	0,	NULL, NULL, dal_Add_ignore|dal_Edit_ignore},
	{"ipIfacePath",			dalType_string,	1,	32,	NULL, NULL, dalcmd_Forbid}, //read only
	{"X_ZYXEL_IfName",		dalType_string,	0,	32,	NULL, NULL, dalcmd_Forbid}, //read only
	{"Name",				dalType_string,	1,	32,	NULL, NULL, dal_Add|dal_Edit|dal_Delete},
	{"Enable",				dalType_boolean,	0,	0,	NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	{"Type",				dalType_string,	3,	4,	NULL, "ATM|PTM|ETH|PON|SFP|CELL", dal_Add | dal_Edit_ignore},
#else
	{"Type",				dalType_string,	3,	4,	NULL, "ATM|PTM|ETH|PON|SFP", dal_Add | dal_Edit_ignore},
#endif
	{"Mode",				dalType_string,	9,	10,	NULL, "IP_Routed|IP_Bridged", dal_Add},
#ifdef ZYXEL_PPTPD_RELAY
	{"Encapsulation",		dalType_string,	4,	11,	NULL, "IPoE|IPoA|PPPoE|PPPoA|PPtPtoPPPoA"},
#else
	{"Encapsulation",		dalType_string,	4,	5,	NULL, "IPoE|IPoA|PPPoE|PPPoA"},
#endif
#ifdef ABUE_CUSTOMIZATION
	{"ipMode",				dalType_string,	4,	10,	NULL, "IPv4|DualStack|IPv6|AutoConfig"},
#else
	{"ipMode",				dalType_string,	4,	9,	NULL, "IPv4|DualStack|IPv6"},
#endif	
#ifdef ZYXEL_WAN_DHCPV6_MODE
	{"ip6Mode", 			dalType_string, 5,	6,	NULL, "SLAAC|DHCPv6|Static"},
#endif
#ifdef ZYXEL_PD_TYPE
	{"ip6StaticPrefix", 	dalType_string, 0,	0,	NULL},
	{"ip6PrefixEnable", 	dalType_boolean,	0,	0,	NULL},
	{"ip6PrefixStatic", 	dalType_boolean,	0,	0,	NULL},
#endif
	//atmLink
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	{"vpivci",				dalType_vpivci,	1,	256,	NULL}, // VPI/VCI
	{"AtmEncapsulation",	dalType_string,	1,	6,	NULL},
	{"QoSClass",			dalType_string,	3,	8,	NULL, "UBR|CBR|VBR-nrt|VBR-rt"},
	{"atmPeakCellRate",		dalType_int,	0,	255000,	NULL},
	{"atmSustainedCellRate",dalType_int,	0,	255000,	NULL},
	{"atmMaxBurstSize",		dalType_int,	0,	1000000,	NULL},
#endif
	//GPON
	//{"slidValue",			dalType_int,	0,	0,	NULL},
	//VLAN term
	{"VLANEnable",			dalType_boolean,	0,	0,	NULL},
	{"VLANID",				dalType_int,		-1,	4094,	NULL},
	{"VLANPriority",		dalType_int,		-1,	7,	NULL},
	//ppp Iface
#ifndef ABUU_CUSTOMIZATION
	{"pppUsername",			dalType_string,	1,	64,	NULL},
	{"pppPassword",			dalType_string,	1,	64,	NULL},
#else
	{"pppUsername",			dalType_string,	0,	64,	NULL},
	{"pppPassword",			dalType_string,	0,	64,	NULL},
#endif
	{"ConnectionTrigger",	dalType_string,	0,	0,	NULL, "AlwaysOn|OnDemand"},
	{"IdleDisconnectTime",	dalType_int,	0,	0,	NULL},
#ifdef ZYXEL_PPPoE_SERVICE_OPTION
	{"pppServicename",		dalType_string,	0,	0,	NULL},
#endif
	{"pppoePassThrough",	dalType_boolean,	0,	0,	NULL},
	//Nat
	{"NatEnable",			dalType_boolean,	0,	0,	NULL},
	{"FullConeEnabled",		dalType_boolean,	0,	0,	NULL},
	//IGMP/MLD
	{"IGMPEnable",			dalType_boolean,	0,	0,	NULL},
#ifdef ZyXEL_WEB_GUI_SHOW_IGMP_VERSION	
	{"IGMPVersion", 		dalType_int, 	    2, 	3, 	NULL,	NULL,	NULL},
#endif		
	{"MLDEnable",			dalType_boolean,	0,	0,	NULL},
	//DNS
	{"ipDnsStatic",			dalType_boolean,	0,	0,	NULL},
	{"DNSServer",			dalType_v4AddrList,	0,	0,	NULL},
	{"ip6DnsStatic",		dalType_boolean,	0,	0,	NULL},
	{"DNS6Server",			dalType_v6Addr,	0,	0,	NULL},
	//MAP
	{"mapEnable",			dalType_boolean,	0,	0,	NULL},
	{"Softwire46Enable",	dalType_boolean,	0,	0,	NULL},
	//{"domainEnable",		dalType_boolean,	0,	0,	NULL},
	{"TransportMode",		dalType_string,		0,	0,	NULL, "Translation|Encapsulation"},
	//{"WANInterface",		dalType_string,		0,	0,	NULL},
	{"BRIPv6Prefix",		dalType_string,		0,	0,	NULL},
	{"PSIDOffset",			dalType_int,		0,	0,	NULL},
	{"PSIDLength",			dalType_int,		0,	0,	NULL},
	{"PSID",				dalType_int,		0,	0,	NULL},
	//{"ruleEnable",			dalType_boolean,	0,	0,	NULL},
	{"IPv6Prefix",			dalType_string,		0,	0,	NULL},
	{"IPv4Prefix",			dalType_string,		0,	0,	NULL},
	{"EABitsLength",		dalType_int,		0,	0,	NULL},
	//router
	{"sysGwEnable",			dalType_boolean,	0,	0,	NULL},
	{"sysGw6Enable",		dalType_boolean,	0,	0,	NULL},
	//6RD
	{"Enable_6RD",			dalType_boolean,	0,	0,	NULL},
	{"v6RD_Type",			dalType_string,	0,	0,	NULL, "dhcp|static"},
	{"SPIPv6Prefix",		dalType_string,	0,	0,	NULL},
	{"IPv4MaskLength",		dalType_int,	0,	32,	NULL},
	{"BorderRelayIPv4Addresses",dalType_v4Addr,	0,	0,	NULL},
	//DSLite
	{"DSLiteEnable",		dalType_boolean,	0,	0,	NULL},
#ifdef ZYXEL_DHCPV6_OPTION64
	{"DSLiteType",			dalType_string,	0,	0,	NULL, "dhcp|static"},
#else
	{"DSLiteType",			dalType_string,	0,	0,	NULL, "static"},
#endif
	{"DSLiteRelayIPv6Addresses",	dalType_v6Addr,	0,	0,	NULL},
	//ipIface or pppIface
	{"MaxMTUSize",			dalType_int,	0,	0,	NULL},
	//Address, gateway, DHCP
	{"ipStatic",			dalType_boolean,	0,	0,	NULL},
	{"IPAddress",			dalType_v4Addr,	0,	0,	NULL},
	{"SubnetMask",			dalType_v4Mask,	0,	0,	NULL},
	{"GatewayIPAddress",	dalType_v4Addr,	0,	0,	NULL},
	{"ip6Static",			dalType_boolean,	0,	0,	NULL},
	{"IP6Address",			dalType_v6AddrPrefix,	0,	0,	NULL},
	{"NextHop",				dalType_v6Addr,	0,	0,	NULL},
     /* __ZyXEL__, Albert, 20180209,Support IPv6 option IA_NA and IA_PD  */
	{"IPv6Manual_IANA",		dalType_boolean,	0,	0,	NULL},
	{"IPv6Manual_IAPD",		dalType_boolean,	0,	0,	NULL},	
	//DHCP
	{"option42Enable",		dalType_boolean,	0,	0,	NULL},
	{"option42Value",		dalType_string,	0,	255,	NULL, NULL, dalcmd_Forbid},
	{"option43Enable",		dalType_boolean,	0,	0,	NULL},
	{"option120Enable",		dalType_boolean,	0,	0,	NULL},
	{"option121Enable",		dalType_boolean,	0,	0,	NULL},
	{"option60Enable",		dalType_boolean,	0,	0,	NULL},
	{"option60Value",		dalType_string,	0,	255,	NULL},
	{"option61Enable",		dalType_boolean,	0,	0,	NULL},
	{"option61Value",		dalType_string,	0,	255,	NULL, NULL, dalcmd_Forbid},
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
    {"option61Iaid",		dalType_string, 	0,	255,		NULL},
    {"option61Duid",		dalType_int, 		1,	3,			NULL},
    {"option61Enterprise",	dalType_int, 		0,	2147483647,	NULL},
    {"option61Identifier",	dalType_string, 	0,	255,		NULL},
#else
	{"option61Iaid",		dalType_string, 0,	255,	NULL},
	{"option61Duid",		dalType_string, 0,	255,	NULL},
#endif
	{"option125Enable",		dalType_boolean,	0,	0,	NULL},
	{"option125Value",		dalType_string,	0,	255,	NULL},
	//ConcurrentWan
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
	{"option125ManufacturerOUI",    dalType_string,	0,	255,	NULL},
	{"option125ProductClass",       dalType_string,	0,	255,	NULL},
	{"option125ModelName",          dalType_string,	0,	255,	NULL},
	{"option125SerialNumber",       dalType_string,	0,	255,	NULL},
#endif
	{"ConcurrentWan",		dalType_int,	0,	31,	NULL},
	{"ManufacturerOUI",     dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"SerialNumber",		dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"ProductClass",		dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
#ifdef ZYXEL_WAN_MAC
	{"WanMacType",		    dalType_string,	0,	0,	NULL, "Default|ClientIP|UserDefine"},
	{"WanMacIP",		    dalType_v4Addr,	0,	0,	NULL},
	{"WanMacAddr",		    dalType_string,	0,	0,	NULL},
#endif
	{"LantoWan",			dalType_boolean, 0,  0,  NULL},
	{NULL,					0,	0,	0,	NULL},
};

#if defined (CONFIG_ABZQ_CUSTOMIZATION)
#define OPTION61_HEADER_LENGTH      6
#define OPTION61_IAID_BYTES         4
#define OPTION61_DUID_BYTES         2
#define OPTION61_ENTERPRISE_BYTES   4
#endif

#if defined (CONFIG_ABZQ_CUSTOMIZATION)
#define OPTION125_ENTERPRISE_BYTES      4
#define OPTION125_DATA_BYTES            1
#define OPTION125_HEADER_BYTES          (OPTION125_DATA_BYTES + OPTION125_ENTERPRISE_BYTES)
#define OPTION125_SUBOPTION_CODE        1
#define OPTION125_SUBOPTION_HEADER      (OPTION125_SUBOPTION_CODE + OPTION125_DATA_BYTES)
#define OPTION125_ZYXEL_ID              890
#endif //CONFIG_ABZQ_CUSTOMIZATION


char currLowerLayers[128] = {0};
char ipIfacePath[32] = {0};

bool isAdd = false;
bool isDelete = false;
bool isPppQuick = false;

const char *Name, *Type, *Mode, *Encapsulation, *ipMode, *v6RD_Type, *deleteipIfacePath;
#ifdef ZYXEL_WAN_DHCPV6_MODE
const char *ip6Mode;
#endif
#ifdef ZYXEL_PD_TYPE
const char *ip6StaticPrefix;
bool ip6PrefixEnable,ip6PrefixStatic;	
#endif
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
const char *TAAABSrvName;
#ifdef TAAAB_HGW
const char *HGWStyleName;
#endif
#endif
#ifdef ZYXEL_WAN_MAC
const char *WanMacType, *WanMacAddr, *WanMacIP;
#endif
bool ipStatic, ip6Static, Enable_6RD, ip6DnsStatic, IPv6Manual_IANA, IPv6Manual_IAPD;

bool DSCPEnable;

char CurrType[16] = {0};
char CurrMode[16] = {0};
char CurrEncapsulation[16] = {0};
char CurrIpMode[16] = {0};
char CurrV6RD_Type[16] = {0};
bool CurrEnable = false, CurrIpStatic = false, CurrIp6Static = false, CurrPppStatic = false, CurrEnable_6RD = false;

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
struct json_object *atmLinkObj = NULL;
struct json_object *atmLinkQosObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
struct json_object *ptmLinkObj = NULL;
#endif
struct json_object *ethIfaceObj = NULL;
struct json_object *optIfaceObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
struct json_object *cellIfaceObj = NULL;
#endif
struct json_object *ethLinkObj = NULL;
struct json_object *vlanTermObj = NULL;
struct json_object *pppIfaceObj = NULL;
struct json_object *ipIfaceObj = NULL;
struct json_object *v4ClientObj = NULL;
struct json_object *opt42Obj = NULL;
struct json_object *opt43Obj = NULL;
struct json_object *opt120Obj = NULL;
struct json_object *opt121Obj = NULL;
struct json_object *opt212Obj = NULL;
struct json_object *opt60Obj = NULL;
struct json_object *opt61Obj = NULL;
struct json_object *opt125Obj = NULL;
struct json_object *igmpObj = NULL;
struct json_object *mldObj = NULL;
struct json_object *natIntfObj = NULL;
struct json_object *dnsClientSrv4Obj = NULL;
struct json_object *dnsClientSrv6Obj = NULL;
struct json_object *dynamicDnsClientSrv4Obj = NULL;
struct json_object *dynamicDnsClientSrv6Obj = NULL;
struct json_object *routerObj = NULL;
struct json_object *v4FwdObj = NULL;
struct json_object *v6FwdObj = NULL;
//struct json_object *dynamicV4FwdObj = NULL;
//struct json_object *dynamicV6FwdObj = NULL;
struct json_object *v4AddrObj = NULL;
struct json_object *v6AddrObj = NULL;
struct json_object *dnsObj = NULL;
struct json_object *mapObj = NULL;
struct json_object *domainObj = NULL;
struct json_object *ruleObj = NULL;

struct json_object *featureFlagObj = NULL;

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
objIndex_t atmLinkIid = {0};
objIndex_t atmLinkQosIid = {0};
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
objIndex_t ptmLinkIid = {0};
#endif
objIndex_t ethIfaceIid = {0};
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
objIndex_t optIfaceIid = {0};
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
objIndex_t cellIfaceIid = {0};
#endif
objIndex_t ethLinkIid = {0};
objIndex_t vlanTermIid = {0};
objIndex_t pppIfaceIid = {0};
objIndex_t ipIfaceIid = {0};
objIndex_t v4ClientIid = {0};
objIndex_t opt42Iid = {0};
objIndex_t opt43Iid = {0};
objIndex_t opt120Iid = {0};
objIndex_t opt121Iid = {0};
objIndex_t opt212Iid = {0};
objIndex_t opt60Iid = {0};
objIndex_t opt61Iid = {0};
objIndex_t opt125Iid = {0};
objIndex_t igmpIid = {0};
objIndex_t mldIid = {0};
objIndex_t natIntfIid = {0};
objIndex_t ripIid = {0};
objIndex_t dnsClientSrv4Iid = {0};
objIndex_t dnsClientSrv6Iid = {0};
objIndex_t dynamicDnsClientSrv4Iid = {0};
objIndex_t dynamicDnsClientSrv6Iid = {0};
objIndex_t routerIid = {0};
objIndex_t v4FwdIid = {0};
objIndex_t v6FwdIid = {0};
objIndex_t v4AddrIid = {0};
objIndex_t v6AddrIid = {0};
objIndex_t dnsIid = {0};
objIndex_t mapIid = {0};
objIndex_t domainIid = {0};
objIndex_t ruleIid = {0};

time_t t1, t2;

void zcfgFeDalShowWan(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj, *obj_idx = NULL;
	const char *mode = NULL;
	const char *encapsulation = NULL;
	const char *vlanpriority = NULL;
	const char *vlanID = NULL;
	const char *igmpEnable = NULL;
	const char *nat = NULL;
	const char *defaultgateway = NULL;
	char *dnsServer = NULL;
	char *dns6Server =NULL;
	char *primaryDns = NULL, *secondaryDns = NULL, *primary6Dns = NULL, *secondary6Dns = NULL;
	

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	len = json_object_array_length(Jarray);
	obj_idx = json_object_array_get_idx(Jarray, 0);
	if(len == 1 && json_object_get_boolean(json_object_object_get(obj_idx, "ShowDetail"))){
		if(json_object_get_boolean(json_object_object_get(obj_idx, "ipDnsStatic"))){
			dnsServer = (char *)json_object_get_string(json_object_object_get(obj_idx, "DNSServer"));
			primaryDns = strtok_r(dnsServer, ",", &secondaryDns);
		}
		if(json_object_get_boolean(json_object_object_get(obj_idx, "ip6DnsStatic"))){
			dns6Server = (char *)json_object_get_string(json_object_object_get(obj_idx, "DNS6Server"));
			primary6Dns = strtok_r(dns6Server, ",", &secondary6Dns);
		}
		printf("General: \n");
		printf("%-2s %-20s %-10s \n", "", "Active", json_object_get_string(json_object_object_get(obj_idx, "Enable")));
		printf("%-2s %-20s %-10s \n", "", "Name", json_object_get_string(json_object_object_get(obj_idx, "Name")));
		if(!strcmp("ATM",json_object_get_string(json_object_object_get(obj_idx, "Type"))))
			printf("%-2s %-20s %-10s \n", "", "Type", "ADSL over ATM.");
		else if(!strcmp("PTM",json_object_get_string(json_object_object_get(obj_idx, "Type"))))
			printf("%-2s %-20s %-10s \n", "", "Type", "ADSL/VDSL over PTM.");
		else if(!strcmp("ETH",json_object_get_string(json_object_object_get(obj_idx, "Type"))))
			printf("%-2s %-20s %-10s \n", "", "Type", "Ethnet.");
		else if(!strcmp("SFP",json_object_get_string(json_object_object_get(obj_idx, "Type"))))
			printf("%-2s %-20s %-10s \n", "", "Type", "SFP.");
		if(!strcmp("IP_Routed",json_object_get_string(json_object_object_get(obj_idx, "Mode"))))
			printf("%-2s %-20s %-10s \n", "", "Mode", "Routing");
		else if(!strcmp("IP_Bridged",json_object_get_string(json_object_object_get(obj_idx, "Mode"))))
			printf("%-2s %-20s %-10s \n", "", "Mode", "Bridge");
		if(!strcmp("IP_Routed",json_object_get_string(json_object_object_get(obj_idx, "Mode")))){
			printf("%-2s %-20s %-10s \n", "", "Mode", json_object_get_string(json_object_object_get(obj_idx, "Encapsulation")));
			printf("%-2s %-20s %-10s \n", "", "IPv4/IPv6 Mode", json_object_get_string(json_object_object_get(obj_idx, "ipMode")));
			printf("%-2s %-20s %-10s \n", "", "Static IP", json_object_get_string(json_object_object_get(obj_idx, "ipStatic")));
			if(json_object_get_boolean(json_object_object_get(obj_idx, "ipStatic"))){
				printf("%-2s %-20s %-10s \n", "", "IP Address", json_object_get_string(json_object_object_get(obj_idx, "IPAddress")));
				printf("%-2s %-20s %-10s \n", "", "Subnet Mask", json_object_get_string(json_object_object_get(obj_idx, "SubnetMask")));
				printf("%-2s %-20s %-10s \n", "", "Gateway IP Address", json_object_get_string(json_object_object_get(obj_idx, "GatewayIPAddress")));
				printf("%-2s %-20s %-10s \n", "", "DNS Server", json_object_get_string(json_object_object_get(obj_idx, "ipDnsStatic")));
				printf("%-2s %-25s %-10s \n", "", "Primary DNS Server", primaryDns);
				printf("%-2s %-25s %-10s \n", "", "Secondary DNS Server", secondaryDns);
			}
			else if(json_object_get_boolean(json_object_object_get(obj_idx, "ipDnsStatic"))){
				printf("%-2s %-20s %-10s \n", "", "DNS Server", json_object_get_string(json_object_object_get(obj_idx, "ipDnsStatic")));
				printf("%-2s %-25s %-10s \n", "", "Primary DNS Server", primaryDns);
				printf("%-2s %-25s %-10s \n", "", "Secondary DNS Server", secondaryDns);
			}
			else
				printf("%-2s %-20s %-10s \n", "", "DNS Server", json_object_get_string(json_object_object_get(obj_idx, "ipDnsStatic")));
		}
		if(!strcmp("ATM",json_object_get_string(json_object_object_get(obj_idx, "Type")))){
			printf("%-2s %-20s %-10s \n", "", "VPI/VCI", json_object_get_string(json_object_object_get(obj_idx, "vpivci")));
			if(!strcmp("LLC",json_object_get_string(json_object_object_get(obj_idx, "AtmEncapsulation"))))
				printf("%-2s %-20s %-10s \n", "", "Encapsulation", "LLC/SNAP-BRIDGING");
			else if(!strcmp("VCMUX",json_object_get_string(json_object_object_get(obj_idx, "AtmEncapsulation"))))
				printf("%-2s %-20s %-10s \n", "", "Encapsulation", "VC/MUX");
			if(!strcmp("UBR",json_object_get_string(json_object_object_get(obj_idx, "QoSClass"))))
				printf("%-2s %-20s %-10s \n", "", "Encapsulation", "UBR without PCR");
			else if(!strcmp("CBR",json_object_get_string(json_object_object_get(obj_idx, "QoSClass")))){
				printf("%-2s %-20s %-10s \n", "", "Encapsulation", "CBR");
				printf("%-4s %-20s %-10s \n", "", "Peak Cell Rate [cells/s]", json_object_get_string(json_object_object_get(obj_idx, "atmPeakCellRate")));
			}
			else if(!strcmp("VBR-nrt",json_object_get_string(json_object_object_get(obj_idx, "QoSClass")))){
				printf("%-2s %-20s %-10s \n", "", "Encapsulation", "Non Realtime VBR");
				printf("%-4s %-20s %-10s \n", "", "Peak Cell Rate [cells/s]", json_object_get_string(json_object_object_get(obj_idx, "atmPeakCellRate")));
				printf("%-4s %-20s %-10s \n", "", "Sustainable Cell Rate", json_object_get_string(json_object_object_get(obj_idx, "atmSustainedCellRate")));
				printf("%-4s %-20s %-10s \n", "", "Maximum Burst Size [cells]", json_object_get_string(json_object_object_get(obj_idx, "atmMaxBurstSize")));
			}
			else if(!strcmp("VBR-rt",json_object_get_string(json_object_object_get(obj_idx, "QoSClass")))){
				printf("%-2s %-20s %-10s \n", "", "Encapsulation", "Realtime VBR");
				printf("%-4s %-20s %-10s \n", "", "Peak Cell Rate [cells/s]", json_object_get_string(json_object_object_get(obj_idx, "atmPeakCellRate")));
				printf("%-4s %-20s %-10s \n", "", "Sustainable Cell Rate", json_object_get_string(json_object_object_get(obj_idx, "atmSustainedCellRate")));
				printf("%-4s %-20s %-10s \n", "", "Maximum Burst Size [cells]", json_object_get_string(json_object_object_get(obj_idx, "atmMaxBurstSize")));
			}
		}
		if(!strncmp(json_object_get_string(json_object_object_get(obj_idx, "Encapsulation")),"PPP",3)){
			printf("PPP Information \n");
			printf("%-2s %-25s %-10s \n", "", "PPP User Name", json_object_get_string(json_object_object_get(obj_idx, "pppUsername")));
			printf("%-2s %-25s %-10s \n", "", "PPP Password", json_object_get_string(json_object_object_get(obj_idx, "pppPassword")));
			if(!strcmp("AlwaysOn",json_object_get_string(json_object_object_get(obj_idx, "ConnectionTrigger"))))
				printf("%-2s %-25s %-10s \n", "", "PPP Connection Trigger", "Auto Connect");
			else if(!strcmp("OnDemand",json_object_get_string(json_object_object_get(obj_idx, "ConnectionTrigger")))){
				printf("%-2s %-25s %-10s \n", "", "PPP Connection Trigger", "On Demand");
				printf("%-2s %-25s %-10s \n", "", "Idle Timeout(sec.)", json_object_get_string(json_object_object_get(obj_idx, "IdleDisconnectTime")));
			}
			printf("%-2s %-25s %-10s \n", "", "PPPoE Passthrough", json_object_get_string(json_object_object_get(obj_idx, "pppoePassThrough")));
		}
		printf("VLAN: \n");
		printf("%-2s %-20s %-10s \n", "", "Active :", json_object_get_string(json_object_object_get(obj_idx, "VLANEnable")));
		if(!strcmp("-1",json_object_get_string(json_object_object_get(obj_idx, "VLANPriority"))))
			printf("%-2s %-20s \n", "", "802.1p :");
		else
			printf("%-2s %-20s %-10s \n", "", "802.1p :", json_object_get_string(json_object_object_get(obj_idx, "VLANPriority")));
		if(!strcmp("-1",json_object_get_string(json_object_object_get(obj_idx, "VLANID"))))
			printf("%-2s %-20s \n", "", "802.1q :");
		else 
			printf("%-2s %-20s %-10s \n", "", "802.1q :", json_object_get_string(json_object_object_get(obj_idx, "VLANID")));
		printf("%-20s %-10s \n", "MTU :", json_object_get_string(json_object_object_get(obj_idx, "MaxMTUSize")));
		printf("Routing Feature: \n");
		printf("%-2s %-30s %-10s \n", "", "NAT Enable :", json_object_get_string(json_object_object_get(obj_idx, "NatEnable")));
		if(json_object_get_boolean(json_object_object_get(obj_idx, "NatEnable")))
			printf("%-2s %-30s %-10s \n", "", "Fullcone NAT Enable :", json_object_get_string(json_object_object_get(obj_idx, "FullConeEnabled")));
		printf("%-2s %-30s %-10s \n", "", "IGMP Proxy Enable :", json_object_get_string(json_object_object_get(obj_idx, "IGMPEnable")));
		printf("%-2s %-30s %-10s \n", "", "Apply as Default Gateway :", json_object_get_string(json_object_object_get(obj_idx, "sysGwEnable")));
		if(!json_object_get_boolean(json_object_object_get(obj_idx, "ipStatic"))){
			printf("DHCPC Options: \n");
#ifdef ZyXEL_DHCP_OPTION42
			printf("%-2s %-20s %-10s %s\n", "", "options42", json_object_get_string(json_object_object_get(obj_idx, "option42Enable")), json_object_get_string(json_object_object_get(obj_idx, "option42Value")));
#endif
			printf("%-2s %-20s %-10s \n", "", "options43", json_object_get_string(json_object_object_get(obj_idx, "option43Enable")));
			printf("%-2s %-20s %-10s \n", "", "options60", json_object_get_string(json_object_object_get(obj_idx, "option60Enable")));
			printf("%-2s %-20s %-10s \n", "", "options61", json_object_get_string(json_object_object_get(obj_idx, "option61Enable")));
			printf("%-2s %-20s %-10s \n", "", "options120", json_object_get_string(json_object_object_get(obj_idx, "option120Enable")));
			printf("%-2s %-20s %-10s \n", "", "options121", json_object_get_string(json_object_object_get(obj_idx, "option121Enable")));
			printf("%-2s %-20s %-10s \n", "", "options125", json_object_get_string(json_object_object_get(obj_idx, "option125Enable")));
		}
		printf("IPv6 Address: \n");
		printf("%-2s %-20s %-10s \n", "", "IPv6 Address", json_object_get_string(json_object_object_get(obj_idx, "ip6Static")));
		if(json_object_get_boolean(json_object_object_get(obj_idx, "ip6Static"))){
			printf("%-2s %-20s %-10s \n", "", "Static IPv6 Address", json_object_get_string(json_object_object_get(obj_idx, "IP6Address")));
			printf("%-2s %-20s %-10s \n", "", "IPv6 Default Gateway", json_object_get_string(json_object_object_get(obj_idx, "NextHop")));
			printf("%-2s %-20s %-10s \n", "", "IPv6 DNS Server", json_object_get_string(json_object_object_get(obj_idx, "ip6DnsStatic")));
			printf("%-2s %-25s %-10s \n", "", "Primary DNS Server", primary6Dns);
			printf("%-2s %-25s %-10s \n", "", "Secondary DNS Server", secondary6Dns);
		}
		else if(json_object_get_boolean(json_object_object_get(obj_idx, "ipDnsStatic"))){
			printf("%-2s %-20s %-10s \n", "", "IPv6 DNS Server", json_object_get_string(json_object_object_get(obj_idx, "ip6DnsStatic")));
			printf("%-2s %-25s %-10s \n", "", "Primary DNS Server", primary6Dns);
			printf("%-2s %-25s %-10s \n", "", "Secondary DNS Server", secondary6Dns);
		}
		else
			printf("%-2s %-20s %-10s \n", "", "IPv6 DNS Server", json_object_get_string(json_object_object_get(obj_idx, "ip6DnsStatic")));
		printf("IPv6 Routing Feature: \n");
		printf("%-2s %-30s %-10s \n", "", "MLD Proxy Enable", json_object_get_string(json_object_object_get(obj_idx, "MLDEnable")));
		printf("%-2s %-30s %-10s \n", "", "Apply as Default Gateway", json_object_get_string(json_object_object_get(obj_idx, "sysGw6Enable")));
		if(json_object_get_boolean(json_object_object_get(obj_idx, "mapEnable")))
		{
			printf("IPv6 MAP: \n");
			printf("%-2s %-30s %-10d \n", "", "MAP Enable", json_object_get_boolean(json_object_object_get(obj_idx, "mapEnable")));
			printf("%-2s %-30s %-10s \n", "", "Transport Mode", json_object_get_string(json_object_object_get(obj_idx, "TransportMode")));
			//printf("%-2s %-30s %-10s \n", "", "WAN Interface", json_object_get_string(json_object_object_get(obj_idx, "WANInterface")));
			printf("%-2s %-30s %-10s \n", "", "BR IPv6 Prefix", json_object_get_string(json_object_object_get(obj_idx, "BRIPv6Prefix")));
			//printf("%-2s %-30s %-10s \n", "", "PSID Offset", json_object_get_int(json_object_object_get(obj_idx, "PSIDOffset")));
			//printf("%-2s %-30s %-10s \n", "", "PSID Length", json_object_get_int(json_object_object_get(obj_idx, "PSIDLength")));
			//printf("%-2s %-30s %-10s \n", "", "PSID", json_object_get_int(json_object_object_get(obj_idx, "PSID")));
			printf("%-2s %-30s %-10s \n", "", "IPv6 Prefix", json_object_get_string(json_object_object_get(obj_idx, "IPv6Prefix")));
			printf("%-2s %-30s %-10s \n", "", "IPv4 Prefix", json_object_get_string(json_object_object_get(obj_idx, "IPv4Prefix")));
			//printf("%-2s %-30s %-10s \n", "", "EA Bits Length", json_object_get_int(json_object_object_get(obj_idx, "EABitsLength")));
		}
	}
	else{
		printf("%-10s %-30s %-10s %-12s %-15s %-8s %-8s %-12s %-5s %-17s\n",
			    "Index", "Name", "Type", "Mode", "Encapsulation", "802.1p", "802.1q", "IGMP Proxy", "NAT", "Default Gateway");
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "Mode")),"IP_Routed")){
			mode = "Routing";
			encapsulation = json_object_get_string(json_object_object_get(obj, "Encapsulation"));
		}else{
			mode = "Bridge";
			encapsulation = "Bridge";
		}
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "VLANPriority")),"-1"))
			vlanpriority = "N/A";
		else
			vlanpriority = json_object_get_string(json_object_object_get(obj, "VLANPriority"));
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "VLANID")),"-1"))
			vlanID = "N/A";
		else
			vlanID = json_object_get_string(json_object_object_get(obj, "VLANID"));
		if(json_object_get_boolean(json_object_object_get(obj, "IGMPEnable")))
			igmpEnable = "Y";
		else
			igmpEnable = "N";
		if(json_object_get_boolean(json_object_object_get(obj, "NatEnable")))
			nat = "Y";
		else
			nat = "N";
		if(json_object_get_boolean(json_object_object_get(obj, "sysGwEnable")))
			defaultgateway = "Y";
		else
			defaultgateway = "N";

			printf("%-10s %-30s %-10s %-12s %-15s %-8s %-8s %-12s %-5s %-17s\n",
				json_object_get_string(json_object_object_get(obj, "index")),
		   json_object_get_string(json_object_object_get(obj, "Name")),
		   json_object_get_string(json_object_object_get(obj, "Type")),
			  	mode, encapsulation, vlanpriority, vlanID, igmpEnable, nat, defaultgateway);
		}
	}
	
}

int charStrToHexStr(char *str, char *hex){
	int i;
	for(i=0;i<strlen(str);i++){
		char a = str[i];
		int b = (int)a;
		char c[3]={0};
		sprintf(c,"%x%x",(b/16),(b%16));
		strcat(hex,c);
	}
	return 1;
}
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
void intToHexStr (int iNum, char *cBuff, int iBytes)
{
	int i, iOffSet, iValue, iSeprateTimes;

	char cPreBuff[128];

	iSeprateTimes = iBytes - 1;

	memset (cBuff, 0, iBytes);

	for ( i = 0 ; i <= iSeprateTimes ; i++ )
	{
		iOffSet = iSeprateTimes - i;

		iValue = ( (iNum >> (8*iOffSet)) & 0xff );

		if ( i == 0 )
		{
			snprintf (cBuff,128, "%02x", iValue);
		}
		else
		{
		 	strncpy(cPreBuff,cBuff,sizeof(cPreBuff)-1);
			snprintf (cBuff,128, "%s%02x", cPreBuff, iValue);
		}
	}
	return;
}
#else //CONFIG_ABZQ_CUSTOMIZATION
int intToHexStr(int num, char *hex){
	if(num>=(16*16)||num<0)
		return 0;
	sprintf(hex,"%x%x",(num/16),(num%16));
	return 1;
}
#endif //CONFIG_ABZQ_CUSTOMIZATION
void opt125ValGet(char *val){
	struct json_object *devInfoObj = NULL;
	objIndex_t objIid;
	char tmpHexStr[256] = {0};
	char ouiHexStr[256] = {0};
	char snHexStr[256] = {0};
	char productHexStr[256] = {0};
	int len = 0, dataLen = 0, lenOui = 0, lenSn = 0, lenProduct = 0;

	IID_INIT(objIid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DEV_INFO, &objIid, &devInfoObj) == ZCFG_SUCCESS){
		lenOui = strlen(json_object_get_string(json_object_object_get(devInfoObj, "ManufacturerOUI")));
		lenSn = strlen(json_object_get_string(json_object_object_get(devInfoObj, "SerialNumber")));
		lenProduct = strlen(json_object_get_string(json_object_object_get(devInfoObj, "ProductClass")));
		charStrToHexStr((char *)json_object_get_string(json_object_object_get(devInfoObj, "ManufacturerOUI")), ouiHexStr);
		charStrToHexStr((char *)json_object_get_string(json_object_object_get(devInfoObj, "SerialNumber")), snHexStr);
		charStrToHexStr((char *)json_object_get_string(json_object_object_get(devInfoObj, "ProductClass")), productHexStr);

		/* entprise(4 bytes) + dataLen(1 byte) + code1(1 byte) + len1(1 byte) + oui(string) + code2 + len2 + sn(string) + code3 + len3 + product(string) */
		len = 4 + 1 + 1 + 1 + lenOui + 1 + 1 + lenSn + 1 + 1 + lenProduct;
		/* -entiprise(4 bytes) - dataLen(1 byte)*/
		dataLen = len - 4 - 1; 

		/* Follow GUI: option125Value = 125 + len + entNum + dataLen + codeOui + lenOui + oui + codeSn + lenSn + sn + codeProduct + lenProduct + product */
		sprintf(tmpHexStr,"%02x%02x%08x%02x%02x%02x%s%02x%02x%s%02x%02x%s",125,len,3561,dataLen,1,lenOui,ouiHexStr,2,lenSn,snHexStr,3,lenProduct,productHexStr);
		strcat(val,tmpHexStr);
		
	}
	if(devInfoObj){
        zcfgFeJsonObjFree(devInfoObj);
    }
}
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
int opt125ValEncode(char *cOUI, char *cClass, char *cModel, char *cSN, char *encVal, int tag)
{
	if(encVal)
		strncpy(encVal,"",OPT_125_VAL_ENCODE_ENCVAL-1);

	int lenOUI = strlen(cOUI);
	int lenClass = strlen(cClass);
	int lenModel = strlen(cModel);
	int lenSn = strlen(cSN);
	int strLen = lenOUI + lenClass + lenModel + lenSn + 4 * OPTION125_SUBOPTION_HEADER;
	char cOptionStr[128] = {0};

	/* option 125 tag */
	intToHexStr (tag, cOptionStr, 1);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* total length */
	intToHexStr ((strLen + OPTION125_HEADER_BYTES), cOptionStr, OPTION125_DATA_BYTES);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* ENTERPRISE ID */
	intToHexStr (OPTION125_ZYXEL_ID, cOptionStr, OPTION125_ENTERPRISE_BYTES);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* total suboption length */
	intToHexStr (strLen, cOptionStr, OPTION125_DATA_BYTES);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* OUI index */
	intToHexStr (1, cOptionStr, OPTION125_SUBOPTION_CODE);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* OUI length */
	intToHexStr (lenOUI, cOptionStr, OPTION125_DATA_BYTES);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* OUI content */
	memset (cOptionStr, 0, 128);
	charStrToHexStr (cOUI, cOptionStr);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* Produce class index */
	intToHexStr (2, cOptionStr, OPTION125_SUBOPTION_CODE);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* Produce class length */
	intToHexStr (lenClass, cOptionStr, OPTION125_DATA_BYTES);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* Produce class content */
	memset (cOptionStr, 0, 128);
	charStrToHexStr (cClass, cOptionStr);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* Model name index */
	intToHexStr (3, cOptionStr, OPTION125_SUBOPTION_CODE);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* Model name length */
	intToHexStr (lenModel, cOptionStr, OPTION125_DATA_BYTES);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* Model name content */
	memset (cOptionStr, 0, 128);
	charStrToHexStr (cModel, cOptionStr);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* SN index */
	intToHexStr (4, cOptionStr, OPTION125_SUBOPTION_CODE);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* SN length */
	intToHexStr (lenSn, cOptionStr, OPTION125_DATA_BYTES);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	/* SN content */
	memset (cOptionStr, 0, 128);
	charStrToHexStr (cSN, cOptionStr);
	strncat (encVal, cOptionStr,OPT_125_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	return 1;
}

void opt125DefaultValGet (char *cOUI, char *cClass, char *cModel, char *cSN)
{
	int i, j, iLength;
	char totalMacAddr[32] = {0};
	char productName[32] = {0};

	zyUtilIGetProductName(productName);
	zyUtilIGetBaseMAC(totalMacAddr);

	iLength = strlen(totalMacAddr);
	for (i = 0; i < iLength; ++i)
	{
		if (isupper(totalMacAddr[i]))
		{
			totalMacAddr[i] = tolower(totalMacAddr[i]);
		}
	}

	strncpy (cSN, totalMacAddr, iLength);

	for (i = 0 ; i < 6 ; i++)
	{
		cOUI [i] = totalMacAddr[i];
	}

	iLength = strlen(productName);

	strncpy (cClass, productName, iLength);

	for (i = 0, j = 0 ; i < iLength ; i++)
	{
		if ( productName[i] !=  '-')
		{
			cModel [j++] = productName [i];
		}
	}
	cModel [j] = '\0';

	return;
}

void opt125ValDecode(char *val, char *cOUI, char *cClass, char *cModel, char *cSN)
{
	int i, iOffset, iSuboptionLength;
	char cOption [128] = {0};

	/* OUI */
	iOffset = (OPTION125_SUBOPTION_CODE + OPTION125_HEADER_BYTES + OPTION125_DATA_BYTES + OPTION125_SUBOPTION_CODE) << 1;

	strncpy (cOption, val+iOffset, OPTION125_DATA_BYTES << 1);
	iSuboptionLength = strtol(cOption, (char **)NULL, 16);

	iOffset += OPTION125_DATA_BYTES << 1;

	memset (cOption, 0, 128);
	strncpy (cOption, val+iOffset, iSuboptionLength << 1);
	hexStrToCharStr (cOption, cOUI);

	/* Produce class */
	iOffset += (iSuboptionLength + OPTION125_SUBOPTION_CODE) << 1;

	memset (cOption, 0, 128);
	strncpy (cOption, val+iOffset, OPTION125_DATA_BYTES << 1);
	iSuboptionLength = strtol(cOption, (char **)NULL, 16);

	iOffset += OPTION125_DATA_BYTES << 1;

	memset (cOption, 0, 128);
	strncpy (cOption, val+iOffset, iSuboptionLength << 1);
	hexStrToCharStr (cOption, cClass);

	/* Model name */
	iOffset += (iSuboptionLength + OPTION125_SUBOPTION_CODE) << 1;

	memset (cOption, 0, 128);
	strncpy (cOption, val+iOffset, OPTION125_DATA_BYTES << 1);
	iSuboptionLength = strtol(cOption, (char **)NULL, 16);

	iOffset += OPTION125_DATA_BYTES << 1;

	memset (cOption, 0, 128);
	strncpy (cOption, val+iOffset, iSuboptionLength << 1);
	hexStrToCharStr (cOption, cModel);

	/* SN */
	iOffset += (iSuboptionLength + OPTION125_SUBOPTION_CODE) << 1;

	memset (cOption, 0, 128);
	strncpy (cOption, val+iOffset, OPTION125_DATA_BYTES << 1);
	iSuboptionLength = strtol(cOption, (char **)NULL, 16);

	iOffset += OPTION125_DATA_BYTES << 1;

	memset (cOption, 0, 128);
	strncpy (cOption, val+iOffset, iSuboptionLength << 1);
	hexStrToCharStr (cOption, cSN);

	return;
}


int opt61ValEncode(char *iaid, int duid, int enterprise, char *identifier, char *encVal, int tag){
#else
int opt61ValEncode(char *iaid, char *duid, char *encVal, int tag){
#endif
	if(encVal)
		ZOS_STRCPY_M(encVal,"");

	//*(iaid+4)='\0';
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
	int strLen = OPTION61_IAID_BYTES + OPTION61_DUID_BYTES;
	char cOptionStr[128] = {0};
#else
	int strLen = 1+4+2+(strlen(duid)/2);
#endif
	char tagStr[2]={0};
	char hexLen[2]={0};
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
	intToHexStr (tag, tagStr, 1);

	strncat (encVal, tagStr, OPT_61_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	if ( duid == 2 )
	{
		strLen +=  OPTION61_ENTERPRISE_BYTES + strlen(identifier) + 1;
	}
	intToHexStr (strLen, hexLen, 1);
	strncat (encVal, hexLen, OPT_61_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	strncat (encVal, "ff", OPT_61_VAL_ENCODE_ENCVAL-strlen(encVal)-1);
	strncat (encVal, iaid, OPT_61_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	intToHexStr (duid, cOptionStr, OPTION61_DUID_BYTES);
	strncat (encVal, cOptionStr, OPT_61_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

	if ( duid == 2 )
	{
		intToHexStr (enterprise, cOptionStr, OPTION61_ENTERPRISE_BYTES);
		strncat (encVal, cOptionStr, OPT_61_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

		memset (cOptionStr, 0, 128);
		charStrToHexStr (identifier, cOptionStr);
		strncat (encVal, cOptionStr, OPT_61_VAL_ENCODE_ENCVAL-strlen(encVal)-1);
	}
#else //CONFIG_ABZQ_CUSTOMIZATION

	intToHexStr(tag,tagStr);
	strcat(encVal,tagStr);

	intToHexStr(strLen,hexLen);
	strcat(encVal,hexLen);

	strcat(encVal,"ff");
	strcat(encVal,iaid);
	strcat(encVal,"0002");
	strcat(encVal,duid);
#endif //CONFIG_ABZQ_CUSTOMIZATION
	return 1;
}

int opt60ValEncode(char *val, char *encVal, int tag){

	if(encVal)
		ZOS_STRCPY_M(encVal,"");

	int strLen = strlen(val);
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
	char tagStr[128]={0};
	char hexLen[128]={0};
#else	
	char tagStr[2]={0};
	char hexLen[2]={0};
#endif
	char charStr[256]={0};
	char hexStr[256]={0};
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
	intToHexStr(tag,tagStr, 1);
	strncat(encVal,tagStr, OPT_61_VAL_ENCODE_ENCVAL-strlen(encVal)-1);

    intToHexStr(strLen,hexLen, 1);
	strncat(encVal,hexLen, OPT_61_VAL_ENCODE_ENCVAL-strlen(encVal)-1);
#else
	intToHexStr(tag,tagStr);
	strcat(encVal,tagStr);

	intToHexStr(strLen,hexLen);
	strcat(encVal,hexLen);
#endif //CONFIG_ABZQ_CUSTOMIZATION
	strncpy(charStr,val,strLen);
	*(charStr+strLen)='\0';
	charStrToHexStr(charStr,hexStr);
	strcat(encVal,hexStr);

	return 1;
}

int hexStrToCharStr(char *hex, char* str){
	char a[2]={0};
	int i=0;
	int hexTokLen = 2;
	while(1){
		if(i>=strlen(hex)){
			break;
		}
		strncpy(a,hex+i,hexTokLen);
		*(a+hexTokLen)='\0';
		int hh = (int)strtol(a,NULL,16);
		char t[5]={0};
		sprintf(t,"%c",(char)hh);
		strcat(str,t);
		i=i+hexTokLen;
	}
	return 1;
}

int opt60ValDecode(char *val, char *decVal){
	char hexStr[256]={0};
	char charStr[256]={0};
	if(strlen(val)<=4){
		ZOS_STRCPY_M(decVal,"");
		return 1;
	}
	if(decVal)
		ZOS_STRCPY_M(decVal,"");
	ZOS_STRNCPY(hexStr,val+4,sizeof(hexStr));
	hexStrToCharStr(hexStr,charStr);
	ZOS_STRCPY_M(decVal,charStr);
	return 1;
}
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
int hexToInteger (int iChar)
{
	int iResult = 0;

	if ( '0' <= iChar && iChar <= '9' )
	{
		iResult = iChar - '0';
	}
	else if ( 'a' <= iChar && iChar <= 'f' )
	{
		iResult = 10 + iChar - 'a' ;
	}

	return iResult;
}

int opt61ValDecode(char *val, char *iaid, int *duid, int *enterprise, char *identifier){
	int i, iIAIDLength, iValue, iDUIDLength, iEnterpriseLength, iIdentifierLength, iOffset;
	char cOption [128] = {0};

	/* IAID */
	iOffset = OPTION61_HEADER_LENGTH;
	iIAIDLength = OPTION61_IAID_BYTES << 1;

	strncpy (iaid, val+iOffset, iIAIDLength);

	/* DUID */
	iOffset += iIAIDLength;
	memset (cOption, 0, 128);
	iDUIDLength = OPTION61_DUID_BYTES << 1;

	strncpy (cOption, val+iOffset, iDUIDLength);
	*duid = atoi(cOption);

	if ( *duid == 2 )
	{
		/* ENTERPRISE */
		iOffset += iDUIDLength;
		memset (cOption, 0, 128);
		iEnterpriseLength = OPTION61_ENTERPRISE_BYTES << 1;

		strncpy (cOption, val+iOffset, iEnterpriseLength);

		iValue = 0;
		for (i = 0 ; i < iEnterpriseLength ; i++)
		{
			iValue += hexToInteger ((int)cOption[i]) * pow(16, (iEnterpriseLength-i-1));
		}

		*enterprise = iValue;

		/* Identifier */
		iOffset += iEnterpriseLength;
		iIdentifierLength = strlen(val) - iOffset;
		memset (cOption, 0, 128);

		strncpy (cOption, val+iOffset, iIdentifierLength);

		hexStrToCharStr (cOption, identifier);
	}
	return 1;
}
#endif //CONFIG_ABZQ_CUSTOMIZATION

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
int UpWanTypeGet()
{
     objIndex_t ethIfaceIid;	
	 objIndex_t dslChannelIid;
	 struct json_object *ethIfaceObj = NULL;
	 struct json_object *dslChannelObj = NULL;
	 const char *ethType;
	 const char *ethStatus;
	 const char *dslType;
	 const char *dslStatus;
	 int WanConnectType = 0;
     
	 IID_INIT(ethIfaceIid);
	 while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &ethIfaceIid, &ethIfaceObj) == ZCFG_SUCCESS) {
		 ethType = json_object_get_string(json_object_object_get(ethIfaceObj, "X_ZYXEL_LanPort"));
		 ethStatus = json_object_get_string(json_object_object_get(ethIfaceObj, "Status"));
		 if(!strcmp(ethType,"ETHWAN") && !strcmp(ethStatus,"Up")){
		 	WanConnectType = 1;
			json_object_put(ethIfaceObj);
			break;
		 }

		 json_object_put(ethIfaceObj);
	 }
	 
	 IID_INIT(dslChannelIid);
	 while(zcfgFeObjJsonGetNext(RDM_OID_DSL_CHANNEL, &dslChannelIid, &dslChannelObj) == ZCFG_SUCCESS) {
		 dslType = json_object_get_string(json_object_object_get(dslChannelObj, "LinkEncapsulationUsed"));
		 dslStatus = json_object_get_string(json_object_object_get(dslChannelObj, "Status"));
		 if(strstr(dslType,"ATM") && !strcmp(dslStatus,"Up")){
		 	WanConnectType = 2;
			json_object_put(dslChannelObj);
			break;
		 }
		 else if(strstr(dslType,"PTM") && !strcmp(dslStatus,"Up")){
		 	WanConnectType = 3;
			json_object_put(dslChannelObj);
			break;
		 }
		 
		 json_object_put(dslChannelObj);	 
	 }	
	 	
	 return WanConnectType;
	 
}
#endif

//the length of delim must be 1 or function may not work well
bool genNewInterface(char *delim, char *newInterface, const char *oldInterface, const char *targetIntf, bool add){
	bool changed = false, firstEnter = true, findTarget = false;
	char buff[512] = {0}, *intf = NULL, *tmp_ptr = NULL;

	ZOS_STRCPY_M(buff, oldInterface);
	intf = strtok_r(buff, delim, &tmp_ptr);
	while(intf != NULL){
		if(!strcmp(intf, targetIntf))
			findTarget = true;
		intf = strtok_r(NULL, delim, &tmp_ptr);
	}
	
	if(add && !findTarget){
		ZOS_STRCPY_M(newInterface, oldInterface);
		if(newInterface[0] != '\0' && newInterface[strlen(newInterface)-1] != *delim)
			strcat(newInterface, delim);
		strcat(newInterface, targetIntf);
		changed = true;
	}
	else if(!add && findTarget){
		tmp_ptr = NULL;
		intf = NULL;
		memset(buff, 0, sizeof(buff));
		ZOS_STRCPY_M(buff, oldInterface);
			
		intf = strtok_r(buff, delim, &tmp_ptr);
		while(intf != NULL){
			if(*intf != '\0' && !strstr(intf, targetIntf)){
				if(!firstEnter && intf != NULL){
					strcat(newInterface, delim);
				}
				strcat(newInterface, intf);
				firstEnter = false;
			}
			intf = strtok_r(NULL, delim, &tmp_ptr);
		}
		changed = true;
	}

	return changed;
}

static zcfgRet_t addDefaultParam(struct json_object *Jobj, char *replyMsg){

	if(Type == NULL){
		json_object_object_add(Jobj, "Type", json_object_new_string("PTM"));
		Type = json_object_get_string(json_object_object_get(Jobj, "Type"));
	}
	else if(strcmp(Type, "ATM") && 
			strcmp(Type, "PTM") && 
			strcmp(Type, "SFP") &&
			strcmp(Type, "ETH") && 
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
			strcmp(Type, "CELL") &&
#endif
			strcmp(Type, "PON")){
		return ZCFG_INVALID_PARAM_VALUE;
	}

	if(!strcmp(Type, "ATM")){
		if(json_object_object_get(Jobj, "vpivci") == NULL){
			if(replyMsg)
				ZOS_STRCPY_M(replyMsg, "vpivci is Mandatory for ATM WAN");
			return ZCFG_INVALID_ARGUMENTS;
		}
		
		if(json_object_object_get(Jobj, "QoSClass") == NULL)
			json_object_object_add(Jobj, "QoSClass", json_object_new_string("UBR"));
	}

	if(Mode == NULL){
		json_object_object_add(Jobj, "Mode", json_object_new_string("IP_Routed"));
		Mode = json_object_get_string(json_object_object_get(Jobj, "Mode"));
	}
	else if(strcmp(Mode, "IP_Routed") && strcmp(Mode, "IP_Bridged")){
		return ZCFG_INVALID_PARAM_VALUE;
	}

	if(json_object_get_boolean(json_object_object_get(Jobj, "VLANEnable")) == false){
		json_object_object_add(Jobj, "VLANID", json_object_new_int(-1));
		json_object_object_add(Jobj, "VLANPriority", json_object_new_int(-1));
	}
	
	if(!strcmp(Mode, "IP_Bridged")){
		return ZCFG_SUCCESS;
	}

	if(Encapsulation == NULL || *Encapsulation == '\0'){
		json_object_object_add(Jobj, "Encapsulation", json_object_new_string("IPoE"));
		Encapsulation = json_object_get_string(json_object_object_get(Jobj, "Encapsulation"));
	}
	else if(strcmp(Encapsulation, "IPoE") &&
			strcmp(Encapsulation, "IPoA") &&
			strcmp(Encapsulation, "PPPoE") &&
			strcmp(Encapsulation, "PPPoA") &&
			strcmp(Encapsulation, TR181_PPtP2PPPOA)){
		return ZCFG_INVALID_PARAM_VALUE;
	}

	if(!strcmp(Encapsulation, "PPPoE") || !strcmp(Encapsulation, "PPPoA")  || !strcmp(Encapsulation, TR181_PPtP2PPPOA)){
		if(json_object_object_get(Jobj, "ConnectionTrigger") == NULL)
			json_object_object_add(Jobj, "ConnectionTrigger", json_object_new_string("AlwaysOn"));
		if(json_object_object_get(Jobj, "MaxMTUSize") == NULL)
			json_object_object_add(Jobj, "MaxMTUSize", json_object_new_int(1492));
	}
	else{ //IPoE/IPoA
		if(json_object_object_get(Jobj, "MaxMTUSize") == NULL)
			json_object_object_add(Jobj, "MaxMTUSize", json_object_new_int(1500));
	}

	if(ipMode == NULL){//IPv4/DualStack/IPv6/AutoConfig
		json_object_object_add(Jobj, "ipMode", json_object_new_string("IPv4"));
		ipMode = json_object_get_string(json_object_object_get(Jobj, "ipMode"));
	}
	else if(strcmp(ipMode, "IPv4") &&
			strcmp(ipMode, "IPv6") &&
#ifdef ABUE_CUSTOMIZATION			
			strcmp(ipMode, "AutoConfig") &&
#endif			
			strcmp(ipMode, "DualStack")){
		return ZCFG_INVALID_PARAM_VALUE;
	}
	if(json_object_object_get(Jobj, "NatEnable") == NULL)
		json_object_object_add(Jobj, "NatEnable", json_object_new_boolean(true));
	
	if(!strcmp(ipMode, "IPv4")){
		if(Enable_6RD){
			if(v6RD_Type == NULL){
				json_object_object_add(Jobj, "v6RD_Type", json_object_new_string("dhcp"));
				v6RD_Type = json_object_get_string(json_object_object_get(Jobj, "v6RD_Type"));
			}
			else if(strcmp(v6RD_Type, "dhcp") && strcmp(v6RD_Type, "static")){
				return ZCFG_INVALID_PARAM_VALUE;
			}

			if(!strcmp(v6RD_Type, "dhcp"))
				json_object_object_add(Jobj, "option212Enable", json_object_new_boolean(true));
		}
	}
	
	if(!strcmp(ipMode, "IPv6")){//dslite
		if(json_object_object_get(Jobj, "DSLiteType") == NULL)
			json_object_object_add(Jobj, "DSLiteType", json_object_new_string("static"));
			
		if(json_object_object_get(Jobj, "DSLiteRelayIPv6Addresses") == NULL)
			json_object_object_add(Jobj, "DSLiteRelayIPv6Addresses", json_object_new_string(""));
	}
#ifdef ABUE_CUSTOMIZATION
	if(!strcmp(ipMode, "AutoConfig")){//dslite
		if(json_object_object_get(Jobj, "DSLiteType") == NULL)
			json_object_object_add(Jobj, "DSLiteType", json_object_new_string("static"));
				
		if(json_object_object_get(Jobj, "DSLiteRelayIPv6Addresses") == NULL)
			json_object_object_add(Jobj, "DSLiteRelayIPv6Addresses", json_object_new_string(""));
	}
#endif

	return ZCFG_SUCCESS;
}

static bool isXTMLinkHasReference(struct json_object *Jobj){
	char atmLinkPath[16] = {0};
	const char *LowerLayer = NULL;
	zcfg_offset_t oid;
	objIndex_t iid;
	struct json_object *Obj;
	zcfg_offset_t currRefOid; 
	objIndex_t currRefIid;

	dbg_printf("isXTMLinkHasReference\n");

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	if(!strcmp(CurrType, "ATM") && (atmLinkObj != NULL))
		sprintf(atmLinkPath, "ATM.Link.%u", atmLinkIid.idx[0]);
	else
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	if(!strcmp(CurrType, "PTM") && (ptmLinkObj != NULL))
		sprintf(atmLinkPath, "PTM.Link.%u", ptmLinkIid.idx[0]);
	else
#endif
	{
		return true;
	}

	dbg_printf("atmLinkPath=%s\n", atmLinkPath);
	
	IID_INIT(currRefIid);
	if(!strcmp(CurrEncapsulation, "PPPoA") || !strcmp(CurrEncapsulation, TR181_PPtP2PPPOA)){
		currRefOid = RDM_OID_PPP_IFACE; 
		memcpy(&currRefIid, &pppIfaceIid, sizeof(objIndex_t));
	}
	else if(!strcmp(CurrEncapsulation, "IPoA")){
		currRefOid = RDM_OID_IP_IFACE; 
		memcpy(&currRefIid, &ipIfaceIid, sizeof(objIndex_t));
	}
	else{//IPoE or PPPoE
		currRefOid = RDM_OID_ETH_LINK; 
		memcpy(&currRefIid, &ethLinkIid, sizeof(objIndex_t));
	}

	dbg_printf("currRefOid=%d\n", currRefOid);

	oid = RDM_OID_ETH_LINK;
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &Obj) == ZCFG_SUCCESS) {
		LowerLayer = json_object_get_string(json_object_object_get(Obj, "LowerLayers"));
		if((LowerLayer != NULL) && !strcmp(LowerLayer, atmLinkPath)){
			if((oid != currRefOid) || memcmp(&iid, &currRefIid, sizeof(objIndex_t))){
				json_object_put(Obj);
				dbg_printf("line=%d\n", __LINE__);
				return true;
			}
		}
		json_object_put(Obj);
	}

	if(!strcmp(CurrType, "PTM")){
		printf("isXTMLinkHasReference false\n");
		return false;
	}

	oid = RDM_OID_PPP_IFACE;
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &Obj) == ZCFG_SUCCESS) {
		LowerLayer = json_object_get_string(json_object_object_get(Obj, "LowerLayers"));
		if((LowerLayer != NULL) && !strcmp(LowerLayer, atmLinkPath)){
			if((oid != currRefOid) || memcmp(&iid, &currRefIid, sizeof(objIndex_t))){
				json_object_put(Obj);
				dbg_printf("line=%d\n", __LINE__);
				return true;
			}
		}
		json_object_put(Obj);
	}

	oid = RDM_OID_IP_IFACE;
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &Obj) == ZCFG_SUCCESS) {
		LowerLayer = json_object_get_string(json_object_object_get(Obj, "LowerLayers"));
		if((LowerLayer != NULL) && !strcmp(LowerLayer, atmLinkPath)){
			if((oid != currRefOid) || memcmp(&iid, &currRefIid, sizeof(objIndex_t))){
				json_object_put(Obj);
				dbg_printf("line=%d\n", __LINE__);
				return true;
			}
		}
		json_object_put(Obj);
	}

	dbg_printf("isXTMLinkHasReference false\n");
	return false;
	
}

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
static zcfgRet_t editAtmObjects(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	bool isNewAtmObj = false;
	const char *vpivci = json_object_get_string(json_object_object_get(Jobj, "vpivci"));
	const char *currVpivci;

	if(isAdd && vpivci == NULL){ //add case
		printf("need vpivci\n");
		return ZCFG_INVALID_PARAM_VALUE;
	}
	
	if(!isAdd){ //edit case
		currVpivci = json_object_get_string(json_object_object_get(atmLinkObj, "DestinationAddress"));
		if(vpivci != NULL && strcmp(currVpivci, vpivci)){
			if(!isXTMLinkHasReference(Jobj)){
				//ensure the qos restart after edit vpivci when linktype is EOA
				if(ethLinkObj){
					json_object_object_add(ethLinkObj, "Enable", json_object_new_boolean(false));
					zcfgFeObjJsonSetWithoutApply(RDM_OID_ETH_LINK, &ethLinkIid, ethLinkObj, NULL);
					json_object_object_add(ethLinkObj, "Enable", json_object_new_boolean(true));
				} else {
					printf("RDM_OID_ETH_LINK Object is not exist!\n");
				}
				zcfgFeObjJsonDel(RDM_OID_ATM_LINK, &atmLinkIid, NULL);
			}
			zcfgFeJsonObjFree(atmLinkObj);
		}
		else{
			//do nothing. If vpivci == NULL or vpivci == currVpivci, it means vci/vpi are not changed
		}
	}
	
	if(!isAdd && atmLinkObj != NULL){
		//edit case and vci/vpi are not changed, no need to re-get atm object.
	}
	else if(!getSpecificObj(RDM_OID_ATM_LINK, "DestinationAddress", json_type_string, (void *)vpivci, &atmLinkIid, &atmLinkObj)){
		//edit or add case, need new atmLink obj.
		IID_INIT(atmLinkIid);
		if((ret = zcfgFeObjJsonAdd(RDM_OID_ATM_LINK, &atmLinkIid, NULL)) == ZCFG_SUCCESS){
			ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj);
		}
		if(ret != ZCFG_SUCCESS)
			return ret;
		isNewAtmObj = true;
		replaceParam(atmLinkObj, "DestinationAddress", Jobj, "vpivci");
	}
	sprintf(currLowerLayers, "ATM.Link.%u", atmLinkIid.idx[0]);

	if(atmLinkQosObj == NULL){
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK_QOS, &atmLinkIid, &atmLinkQosObj)) != ZCFG_SUCCESS)
			return ret;
	}
	
	if(isNewAtmObj){ //set defalult values
		json_object_object_add(atmLinkObj, "Enable", json_object_new_boolean(true));
		json_object_object_add(atmLinkObj, "Encapsulation", json_object_new_string("LLC"));
#ifdef SUPPORT_DSL_BONDING
		json_object_object_add(atmLinkObj, "LowerLayers", json_object_new_string("DSL.Channel.1,DSL.Channel.3,DSL.BondingGroup.1"));
#else
		json_object_object_add(atmLinkObj, "LowerLayers", json_object_new_string("DSL.Channel.1"));
#endif

		dbg_printf("%s: SetIpIfaceStackModified  \n", __FUNCTION__);
		if(ipIfaceObj)
			json_object_object_add(ipIfaceObj, "X_ZYXEL_AccessDescription", json_object_new_string("SetIpIfaceStackModified"));
	}

	/* set atmQos object first */
	replaceParam(atmLinkQosObj, "QoSClass", Jobj, "QoSClass");
	replaceParam(atmLinkQosObj, "MaximumBurstSize", Jobj, "atmMaxBurstSize");
	replaceParam(atmLinkQosObj, "SustainableCellRate", Jobj, "atmSustainedCellRate");
	replaceParam(atmLinkQosObj, "PeakCellRate", Jobj, "atmPeakCellRate");
	memcpy(&atmLinkQosIid, &atmLinkIid, sizeof(objIndex_t));

	if(!isAdd && Mode == NULL){
		Mode = CurrMode;
	}
	if(!strcmp(Mode, "IP_Bridged")){
		json_object_object_add(atmLinkObj, "LinkType", json_object_new_string("EoA"));
	}
	else{ //IP_Routed
		if(!strcmp(Encapsulation, "PPPoE") || !strcmp(Encapsulation, "IPoE")){
			json_object_object_add(atmLinkObj, "LinkType", json_object_new_string("EoA"));
		}
		else{
			json_object_object_add(atmLinkObj, "LinkType", json_object_new_string(Encapsulation));
		}
	}
	replaceParam(atmLinkObj, "Encapsulation", Jobj, "AtmEncapsulation");

	return ret;
}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
zcfgRet_t editPtmObjects(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!isAdd){ //do nothing for edit case
		sprintf(currLowerLayers, "PTM.Link.%u", ptmLinkIid.idx[0]);
		if(ptmLinkObj) {zcfgFeJsonObjFree(ptmLinkObj);}
		return ret;
	}
		
	IID_INIT(ptmLinkIid);
	if(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_PTM_LINK, &ptmLinkIid, &ptmLinkObj) != ZCFG_SUCCESS)
	{
		IID_INIT(ptmLinkIid);

		if((ret = zcfgFeObjJsonAdd(RDM_OID_PTM_LINK, &ptmLinkIid, NULL)) == ZCFG_SUCCESS){
			ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PTM_LINK, &ptmLinkIid, &ptmLinkObj);
		}
		if(ret != ZCFG_SUCCESS)
			return ret;

		json_object_object_add(ptmLinkObj, "Enable", json_object_new_boolean(true));
		json_object_object_add(ptmLinkObj, "X_ZYXEL_QueueShapingBurstSize", json_object_new_int(3000));
		json_object_object_add(ptmLinkObj, "X_ZYXEL_ConnectionMode", json_object_new_string("VlanMuxMode"));
		json_object_object_add(ptmLinkObj, "X_ZYXEL_SchedulerAlgorithm", json_object_new_string("WRR"));
		json_object_object_add(ptmLinkObj, "X_ZYXEL_EnableADSLPtm", json_object_new_boolean(true));
#ifdef SUPPORT_DSL_BONDING
		json_object_object_add(ptmLinkObj, "LowerLayers", json_object_new_string("DSL.Channel.2,DSL.Channel.4,DSL.BondingGroup.2"));
#else
		json_object_object_add(ptmLinkObj, "LowerLayers", json_object_new_string("DSL.Channel.2"));
#endif
	}
	sprintf(currLowerLayers, "PTM.Link.%u", ptmLinkIid.idx[0]);

	return ret;
}
#endif
#if 0
zcfgRet_t editGponObjects(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *opticalIntfObj = NULL;
	int Enable = 1;

	if(getSpecificObj(RDM_OID_ONU, "Upstream", json_type_boolean, &Enable, &iid, &opticalIntfObj))
		sprintf(currLowerLayers, "Optical.Interface.%u", iid.idx[0]);
	else
		return ZCFG_INTERNAL_ERROR;

	
	json_object_put(opticalIntfObj);
	return ret;
}
#endif

zcfgRet_t editPhyLayerObjects(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	int value;
	int valueSFP = 0;
	
	if(!strcmp(Type, "ATM")){
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
		ret = editAtmObjects(Jobj);
#else
		printf("%s : ATM not support\n", __func__);
		return ZCFG_INTERNAL_ERROR;
#endif
	}
	else if(!strcmp(Type, "PTM")){
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
		ret = editPtmObjects(Jobj);
#else
		printf("%s : PTM not support\n", __func__);
		return ZCFG_INTERNAL_ERROR;
#endif
	}
	else if(!strcmp(Type, "ETH")){
		value = 1;
		if((getSpecificObj(RDM_OID_ETH_IFACE, "Upstream", json_type_boolean, &value, &iid, NULL)) )
			sprintf(currLowerLayers, "Ethernet.Interface.%u", iid.idx[0]);
		else
			currLowerLayers[0] = '\0';
	}
	else if(!strcmp(Type, "SFP")){
		value = 1;
		valueSFP = 1;
		if((getSpecificObj(RDM_OID_ETH_IFACE, "Upstream", json_type_boolean, &value, &iid, NULL)) && (getSpecificObj(RDM_OID_ETH_IFACE, "X_ZYXEL_SFP", json_type_boolean, &valueSFP, &iid, NULL)) )
			sprintf(currLowerLayers, "Ethernet.Interface.%u", iid.idx[0]);
		else
			currLowerLayers[0] = '\0';
		printf("[%s][%d] currLowerLayers=[%s]\n",__FUNCTION__,__LINE__,currLowerLayers);
	}
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	else if(!strcmp(Type, "PON")){
		value = 1;
		if(getSpecificObj(RDM_OID_OPT_INTF, "Upstream", json_type_boolean, &value, &iid, NULL))
			sprintf(currLowerLayers, "Optical.Interface.%u", iid.idx[0]);
		else{
			printf("There is no GPON interface\n");
			ret = ZCFG_REQUEST_REJECT;
		}
	}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	else if(!strcmp(Type, "CELL")){
		sprintf(currLowerLayers, "Cellular.Interface.%u", cellIfaceIid.idx[0]);
	}
#endif
	else {
		printf("%s : %s not supported\n", __func__, Type);
	}

	return ret;
}

zcfgRet_t editEthLinkObject(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char *oldLowerLayers;
	if(!strcmp(Mode, "IP_Routed") && (!strcmp(Encapsulation, "IPoA") || !strcmp(Encapsulation, "PPPoA") || !strcmp(Encapsulation, TR181_PPtP2PPPOA)))
		return ret;

	if(isAdd || ethLinkObj == NULL){
		IID_INIT(ethLinkIid);
		if((ret = zcfgFeObjJsonAdd(RDM_OID_ETH_LINK, &ethLinkIid, NULL)) != ZCFG_SUCCESS)
			return ret;
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkObj)) != ZCFG_SUCCESS)
			return ret;
	}
	oldLowerLayers = (char *)json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
	printf("[%s][%d]oldLowerLayers=[%s]  to [%s]\n",__FUNCTION__,__LINE__,oldLowerLayers,currLowerLayers);
	json_object_object_add(ethLinkObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(ethLinkObj, "LowerLayers", json_object_new_string(currLowerLayers));
	if(Type) {
		if(!strcmp(Type, "ETH") || !strcmp(Type, "SFP")){
			json_object_object_add(ethLinkObj, "X_ZYXEL_OrigEthWAN", json_object_new_boolean(true));
		}
#ifdef ZYXEL_WAN_MAC
		if(!strcmp(Type, "ETH") || !strcmp(Type, "ATM") || !strcmp(Type, "PTM")|| !strcmp(Type, "SFP")){
			if(!strcmp(Mode, "IP_Routed") && (WanMacType != NULL))
			{
				json_object_object_add(ethLinkObj, "X_ZYXEL_WanMacType", json_object_new_string(WanMacType));
				if (!strcmp(WanMacType, WAN_MAC_CLIENTIP)) {
					dbg_printf("%s, set WanMacIP\n", __FUNCTION__);
					if (WanMacIP != NULL)
						json_object_object_add(ethLinkObj, "X_ZYXEL_WanMacClientIP", json_object_new_string(WanMacIP));
				} else if (!strcmp(WanMacType, WAN_MAC_USER)) {
					dbg_printf("%s, set WanMacAddr\n", __FUNCTION__);
					if (WanMacAddr != NULL)
						json_object_object_add(ethLinkObj, "X_ZYXEL_WanMacAddr", json_object_new_string(WanMacAddr));
				}
			}
		}
#endif
	}
	sprintf(currLowerLayers, "Ethernet.Link.%u", ethLinkIid.idx[0]);
	printf("[%s][%d]currLowerLayers=[%s]\n",__FUNCTION__,__LINE__,currLowerLayers);
	return ret;
}

zcfgRet_t editVlanTermObject(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;


	if(!strcmp(Mode, "IP_Routed") && (!strcmp(Encapsulation, "IPoA") || !strcmp(Encapsulation, "PPPoA") || !strcmp(Encapsulation, TR181_PPtP2PPPOA)))
		return ret;

#ifdef ZYXEL_WEB_GUI_SHOW_CELLWAN
	printf("[%d] %s Type: %s, CurrType: %s\n", __LINE__, __func__, Type, CurrType);
	if(strcmp(Type, "CELL") == 0)
		return ret;
#endif

	if(isAdd || vlanTermObj == NULL){
		IID_INIT(vlanTermIid);
		if((ret = zcfgFeObjJsonAdd(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, NULL)) != ZCFG_SUCCESS)
			return ret;
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, &vlanTermObj)) != ZCFG_SUCCESS)
			return ret;
	}
	
	json_object_object_add(vlanTermObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(vlanTermObj, "LowerLayers", json_object_new_string(currLowerLayers));

#ifdef ZCFG_VMG8825_B50B_WIND	
	if ((json_object_get_boolean(json_object_object_get(Jobj, "VLANEnable")) == true) && !strcmp(Encapsulation, "PPPoE")){
		if ((strstr(json_object_get_string(json_object_object_get(Jobj, "pppUsername")), "megaoffice.it") != NULL) ||
			(strstr(json_object_get_string(json_object_object_get(Jobj, "pppUsername")), "megaoffice-ws.it") != NULL)){
			json_object_object_add(Jobj, "VLANPriority", json_object_new_int(1));
		}
	}
#endif //ZCFG_VMG8825_B50B_WIND
	
	replaceParam(vlanTermObj, "X_ZYXEL_VLANEnable", Jobj, "VLANEnable");
	replaceParam(vlanTermObj, "VLANID", Jobj, "VLANID");
	replaceParam(vlanTermObj, "X_ZYXEL_VLANPriority", Jobj, "VLANPriority");
	sprintf(currLowerLayers, "Ethernet.VLANTermination.%u", vlanTermIid.idx[0]);
	
	return ret;
}

zcfgRet_t editNatSetting(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//objIndex_t iid;
	zcfg_offset_t oid;
	//struct json_object *obj = NULL;
	bool CurrNatEnabled=false, CurrFullConeEnabled=false;
	bool NatEnable=false, FullConeEnabled=false;
	struct json_object *pramJobj = NULL;
	if(natIntfObj != NULL){
		CurrNatEnabled = json_object_get_boolean(json_object_object_get(natIntfObj, "Enable"));
		CurrFullConeEnabled = json_object_get_boolean(json_object_object_get(natIntfObj, "X_ZYXEL_FullConeEnabled"));
	}

	pramJobj = json_object_object_get(Jobj, "NatEnable");
	if(pramJobj)
		NatEnable = json_object_get_boolean(pramJobj);
	else
		NatEnable = CurrNatEnabled;

	if(NatEnable){
		pramJobj = json_object_object_get(Jobj, "FullConeEnabled");
		if(pramJobj)
			FullConeEnabled = json_object_get_boolean(pramJobj);
		else
			FullConeEnabled = CurrFullConeEnabled;
	}

	dbg_printf("CurrNatEnabled=%d NatEnable=%d\n", CurrNatEnabled, NatEnable);
	dbg_printf("CurrFullConeEnabled=%d FullConeEnabled=%d\n", CurrFullConeEnabled, FullConeEnabled);

	if(CurrNatEnabled && NatEnable){ //enable to enable
		if(FullConeEnabled != CurrFullConeEnabled){
			json_object_object_add(natIntfObj, "X_ZYXEL_FullConeEnabled", json_object_new_boolean(FullConeEnabled));
		}else{
			if(natIntfObj) {zcfgFeJsonObjFree(natIntfObj);}
		}
	}
	else if(CurrNatEnabled && !NatEnable){ ////enable to disable
		json_object_object_add(natIntfObj, "Enable", json_object_new_boolean(false));
		json_object_object_add(natIntfObj, "FullConeEnabled", json_object_new_boolean(false));

	}
	else if(!CurrNatEnabled && NatEnable){ //disable to enable or add WAN case
		oid = RDM_OID_NAT_INTF_SETTING;
		if(natIntfObj == NULL && !getSpecificObj(oid, "Interface", json_type_string, "", &natIntfIid, &natIntfObj)){
			IID_INIT(natIntfIid);
			if(zcfgFeObjJsonAdd(oid, &natIntfIid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(oid, &natIntfIid, &natIntfObj);
		}
		if(natIntfObj){
			json_object_object_add(natIntfObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(natIntfObj, "Interface", json_object_new_string(ipIfacePath));
			replaceParam(natIntfObj, "X_ZYXEL_FullConeEnabled", Jobj, "FullConeEnabled");
		}

	}
	else{//disable to disable (or add WAN case without enable NAT, so need to add rip obj)
		if(natIntfObj){zcfgFeJsonObjFree(natIntfObj);}

	}

	return ret;
}

zcfgRet_t editDnsSetting(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfg_offset_t oid;
	struct json_object *pramJobj = NULL;
	bool isIp4DnsObjAdd = false;
	bool ipDnsStatic=false, ip6DnsStatic=false;
	bool CurrIpDnsStatic=false, CurrIp6DnsStatic=false;
	const char *DNSServer;
	const char *CurrDNSServer;

	if(dnsClientSrv4Obj != NULL) CurrIpDnsStatic = true;
	if(dnsClientSrv6Obj != NULL) CurrIp6DnsStatic = true;	

	pramJobj = json_object_object_get(Jobj, "ipDnsStatic");
	if(!strcmp(ipMode, "IPv6"))
		ipDnsStatic = false;
	else if(pramJobj)
		ipDnsStatic = json_object_get_boolean(pramJobj);
	else
		ipDnsStatic = CurrIpDnsStatic;

	pramJobj = json_object_object_get(Jobj, "ip6DnsStatic");
	if(!strcmp(ipMode, "IPv4"))
		ip6DnsStatic = false;
	else if(pramJobj)
		ip6DnsStatic = json_object_get_boolean(pramJobj);
	else
		ip6DnsStatic = CurrIp6DnsStatic;
	
	dbg_printf("CurrIpDnsStatic=%d ipDnsStatic=%d\n", CurrIpDnsStatic, ipDnsStatic);
	dbg_printf("CurrIp6DnsStatic=%d ip6DnsStatic=%d\n", CurrIp6DnsStatic, ip6DnsStatic);

	if(CurrIpDnsStatic && ipDnsStatic){ //enable to enable
		DNSServer = json_object_get_string(json_object_object_get(Jobj, "DNSServer"));
		CurrDNSServer = json_object_get_string(json_object_object_get(dnsClientSrv4Obj, "DNSServer"));
		if(json_object_object_get(Jobj, "DNSServer")){
			if(strcmp(DNSServer, CurrDNSServer))
				json_object_object_add(dnsClientSrv4Obj, "DNSServer", json_object_new_string(DNSServer));
		}
		else{
			if(dnsClientSrv4Obj) {zcfgFeJsonObjFree(dnsClientSrv4Obj);}
		}
	}
	else if(CurrIpDnsStatic && !ipDnsStatic){ ////enable to disable
		//json_object_object_add(dnsClientSrv4Obj, "Enable", json_object_new_boolean(false));
		json_object_object_add(dnsClientSrv4Obj, "Interface", json_object_new_string(""));
	}
	else if(!CurrIpDnsStatic && ipDnsStatic){ //disable to enable
		if(dnsClientSrv4Obj) {zcfgFeJsonObjFree(dnsClientSrv4Obj);} //should not happened, just to make sure no memory leak.

		isIp4DnsObjAdd = true;
		oid = RDM_OID_DNS_CLIENT_SRV;
		if(!getSpecificObj(oid, "Interface", json_type_string, "", &dnsClientSrv4Iid, &dnsClientSrv4Obj)){
			IID_INIT(dnsClientSrv4Iid);
			if(zcfgFeObjJsonAdd(oid, &dnsClientSrv4Iid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(oid, &dnsClientSrv4Iid, &dnsClientSrv4Obj);
		}
		if(dnsClientSrv4Obj){
			json_object_object_add(dnsClientSrv4Obj, "Enable", json_object_new_boolean(true));

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				objIndex_t tmpiid;
				struct json_object *tmpipIfObj = NULL;
				int upwantype = 0;
				const char *curTAAABSrvName;
				const char *tmpTAAABSrvName;
				char tempipIfacePath[32];
		
			upwantype = UpWanTypeGet();   
			curTAAABSrvName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_TT_SrvName"));
			
			if(upwantype == 2 && strstr(curTAAABSrvName,"_DSL")!=NULL){
				
				IID_INIT(tmpiid);
				while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &tmpiid, &tmpipIfObj) == ZCFG_SUCCESS){
					   tmpTAAABSrvName = json_object_get_string(json_object_object_get(tmpipIfObj, "X_TT_SrvName"));
					   if(!strcmp(curTAAABSrvName,tmpTAAABSrvName) && (tmpiid.idx[0] != ipIfaceIid.idx[0])){
							 sprintf(tempipIfacePath, "IP.Interface.%d", tmpiid.idx[0]);
							 zcfgFeJsonObjFree(tmpipIfObj);
							 break;
					   }
					   zcfgFeJsonObjFree(tmpipIfObj);
				}
				json_object_object_add(dnsClientSrv4Obj, "Interface", json_object_new_string(tempipIfacePath));

			}
			else
		json_object_object_add(dnsClientSrv4Obj, "Interface", json_object_new_string(ipIfacePath));
#else
			json_object_object_add(dnsClientSrv4Obj, "Interface", json_object_new_string(ipIfacePath));
#endif
			json_object_object_add(dnsClientSrv4Obj, "X_ZYXEL_Type", json_object_new_string("Static"));
			replaceParam(dnsClientSrv4Obj, "DNSServer", Jobj, "DNSServer");
		}
	}
	else{//do nothing for disable to disable
		if(dnsClientSrv4Obj) {zcfgFeJsonObjFree(dnsClientSrv4Obj);}
	}

	if(CurrIp6DnsStatic && ip6DnsStatic){ //enable to enable
		DNSServer = json_object_get_string(json_object_object_get(Jobj, "DNS6Server"));
		CurrDNSServer = json_object_get_string(json_object_object_get(dnsClientSrv6Obj, "DNSServer"));
		
		if(DNSServer == NULL || CurrDNSServer ==NULL || !strcmp(DNSServer, CurrDNSServer)){
			if(dnsClientSrv6Obj) {zcfgFeJsonObjFree(dnsClientSrv6Obj);}
		}
		else if(strcmp(DNSServer, CurrDNSServer)){
			json_object_object_add(dnsClientSrv6Obj, "DNSServer", json_object_new_string(DNSServer));
		}
	}
	else if(CurrIp6DnsStatic && !ip6DnsStatic){ ////enable to disable
		//json_object_object_add(dnsClientSrv6Obj, "Enable", json_object_new_boolean(false));
		json_object_object_add(dnsClientSrv6Obj, "Interface", json_object_new_string(""));
		
	}
	else if(!CurrIp6DnsStatic && ip6DnsStatic){ //disable to enable
		if(dnsClientSrv6Obj) {zcfgFeJsonObjFree(dnsClientSrv6Obj);} //should not happened, just to make sure no memory leak.
		oid = RDM_OID_DNS_CLIENT_SRV;
		if(isIp4DnsObjAdd){ // if v4dns obj was added before, do not try to get empty obj!
			IID_INIT(dnsClientSrv6Iid);
			if(zcfgFeObjJsonAdd(oid, &dnsClientSrv6Iid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(oid, &dnsClientSrv6Iid, &dnsClientSrv6Obj);
		}
		else if(!getSpecificObj(oid, "Interface", json_type_string, "", &dnsClientSrv6Iid, &dnsClientSrv6Obj)){
			IID_INIT(dnsClientSrv6Iid);
			if(zcfgFeObjJsonAdd(oid, &dnsClientSrv6Iid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(oid, &dnsClientSrv6Iid, &dnsClientSrv6Obj);
		}
		if(dnsClientSrv6Obj){
			json_object_object_add(dnsClientSrv6Obj, "Enable", json_object_new_boolean(true));
			json_object_object_add(dnsClientSrv6Obj, "Interface", json_object_new_string(ipIfacePath));
			json_object_object_add(dnsClientSrv6Obj, "X_ZYXEL_Type", json_object_new_string("Static"));
			replaceParam(dnsClientSrv6Obj, "DNSServer", Jobj, "DNS6Server");
		}
	}
	else{//do nothing for disable to disable
		if(dnsClientSrv6Obj) {zcfgFeJsonObjFree(dnsClientSrv6Obj);}
	}

	return ret;
}

zcfgRet_t addStaticAddrSetting(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//objIndex_t iid;
	zcfg_offset_t oid;
	//struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;
	//const char *Encapsulation, *ipMode;
	bool ipStatic = false, ip6Static = false;

	/* static ipv4*/
	pramJobj=json_object_object_get(Jobj, "ipStatic");
	if(pramJobj)
		ipStatic = json_object_get_boolean(pramJobj);
	if(strcmp(ipMode, "IPv6") && (!strcmp(Encapsulation, "IPoA")|| ipStatic)){
		oid = RDM_OID_ROUTING_ROUTER_V4_FWD;
		if(!getSpecificObj(oid, "Interface", json_type_string, "", &v4FwdIid, &v4FwdObj)){
			IID_INIT(v4FwdIid);
			v4FwdIid.level = 1;
			v4FwdIid.idx[0] = 1;
			if(zcfgFeObjJsonAdd(oid, &v4FwdIid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(oid, &v4FwdIid, &v4FwdObj);
		}
		if(v4FwdObj){
			json_object_object_add(v4FwdObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(v4FwdObj, "Interface", json_object_new_string(ipIfacePath));
			replaceParam(v4FwdObj, "GatewayIPAddress", Jobj, "GatewayIPAddress");
		}
		//if((ret = zcfgFeObjJsonSetWithoutApply(oid, &v4FwdIid, obj, NULL)) != ZCFG_SUCCESS)
		//	printf("set oid:%d fail, line=%d, ret=%d\n", oid, __LINE__, ret);
		//json_object_put(obj);

		oid = RDM_OID_IP_IFACE_V4_ADDR;
		IID_INIT(v4AddrIid);
		v4AddrIid.level = 1;
		sscanf(ipIfacePath, "IP.Interface.%hhu", &v4AddrIid.idx[0]);
		if(zcfgFeObjJsonAdd(oid, &v4AddrIid, NULL) == ZCFG_SUCCESS)
			zcfgFeObjJsonGetWithoutUpdate(oid, &v4AddrIid, &v4AddrObj);
		if(v4AddrObj){
			json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));
			replaceParam(v4AddrObj, "IPAddress", Jobj, "IPAddress");
			replaceParam(v4AddrObj, "SubnetMask", Jobj, "SubnetMask");
		}
		//if((ret = zcfgFeObjJsonSetWithoutApply(oid, &v4AddrIid, obj, NULL)) != ZCFG_SUCCESS)
		//	printf("set oid:%d fail, line=%d, ret=%d\n", oid, __LINE__, ret);
		//json_object_put(obj);
	}

	/* static ipv6*/
	pramJobj=json_object_object_get(Jobj, "ip6Static");
	if(pramJobj)
		ip6Static = json_object_get_boolean(pramJobj);
	if(strcmp(ipMode, "IPv4") && ip6Static){
		oid = RDM_OID_ROUTING_ROUTER_V6_FWD;
		if(!getSpecificObj(oid, "Interface", json_type_string, "", &v6FwdIid, &v6FwdObj)){
			IID_INIT(v6FwdIid);
			v6FwdIid.level = 1;
			v6FwdIid.idx[0] = 1;
			if(zcfgFeObjJsonAdd(oid, &v6FwdIid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(oid, &v6FwdIid, &v6FwdObj);
		}
		if(v6FwdObj){
			json_object_object_add(v6FwdObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(v6FwdObj, "Interface", json_object_new_string(ipIfacePath));
			json_object_object_add(v6FwdObj, "DestIPPrefix", json_object_new_string("::/0"));
			replaceParam(v6FwdObj, "NextHop", Jobj, "NextHop");
		}
		//if((ret = zcfgFeObjJsonSetWithoutApply(oid, &v6FwdIid, v6FwdObj, NULL)) != ZCFG_SUCCESS)
		//	printf("set oid:%d fail, line=%d, ret=%d\n", oid, __LINE__, ret);
		//json_object_put(v6FwdObj);

		oid = RDM_OID_IP_IFACE_V6_ADDR;
		IID_INIT(v6AddrIid);
		v6AddrIid.level = 1;
		sscanf(ipIfacePath, "IP.Interface.%hhu", &v6AddrIid.idx[0]);
		if(zcfgFeObjJsonAdd(oid, &v6AddrIid, NULL) == ZCFG_SUCCESS)
			zcfgFeObjJsonGetWithoutUpdate(oid, &v6AddrIid, &v6AddrObj);
		if(v6AddrObj){
			json_object_object_add(v6AddrObj, "Enable", json_object_new_boolean(true));
			replaceParam(v6AddrObj, "IPAddress", Jobj, "IP6Address");
		}
		//if((ret = zcfgFeObjJsonSetWithoutApply(oid, &v6AddrIid, v6AddrObj, NULL)) != ZCFG_SUCCESS)
		//	printf("set oid:%d fail, line=%d, ret=%d\n", oid, __LINE__, ret);
		//json_object_put(v6AddrObj);
	}

	return ret;
}


//This function is only for IP mode, not deal with static PPP address
zcfgRet_t editStaticAddrSetting(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfg_offset_t oid;
	struct json_object *pramJobj = NULL;
	bool ipStatic = false, ip6Static = false;
	const char *IPAddress, *SubnetMask, *GatewayIPAddress;
	const char *CurrIPAddress, *CurrSubnetMask, *CurrGatewayIPAddress;
	const char *IP6Address;
	const char *CurrIP6Address;
	// const char *NextHop, *CurrNextHop;
	bool ipdnsmodify = false;

	pramJobj = json_object_object_get(Jobj, "ipStatic");
	if(!strcmp(ipMode, "IPv6"))
		ipStatic = false;
	else if(pramJobj)
		ipStatic = json_object_get_boolean(pramJobj);
	else
		ipStatic = CurrIpStatic;

	pramJobj = json_object_object_get(Jobj, "ip6Static");
	if(!strcmp(ipMode, "IPv4"))
		ip6Static = false;
	else if(pramJobj)
		ip6Static = json_object_get_boolean(pramJobj);
	else
		ip6Static = CurrIp6Static;

	if(strstr(Encapsulation, "PPP") || strstr(Encapsulation, "PPtP"))
		ipStatic = false;

	dbg_printf("CurrIpStatic=%d ipStatic=%d\n", CurrIpStatic, ipStatic);
	dbg_printf("CurrIp6Static=%d ip6Static=%d\n", CurrIp6Static, ip6Static);

	if(CurrIpStatic){
		if(v4AddrObj == NULL){
			IID_INIT(v4AddrIid);
			v4AddrIid.level = 1;
			sscanf(ipIfacePath, "IP.Interface.%hhu", &v4AddrIid.idx[0]);
			if(zcfgFeObjJsonAdd(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj);
		}
		if(v4AddrObj)
			json_object_object_add(v4AddrObj, "AddressingType", json_object_new_string("Static"));
	}

	//static v4ip
	if(CurrIpStatic && ipStatic){ //enable to enable
		//v4 addr
		IPAddress = json_object_get_string(json_object_object_get(Jobj, "IPAddress"));
		SubnetMask = json_object_get_string(json_object_object_get(Jobj, "SubnetMask"));
		GatewayIPAddress = json_object_get_string(json_object_object_get(Jobj, "GatewayIPAddress"));
		if(GatewayIPAddress == NULL){GatewayIPAddress = "";}
		
		CurrIPAddress = json_object_get_string(json_object_object_get(v4AddrObj, "IPAddress"));
		CurrSubnetMask = json_object_get_string(json_object_object_get(v4AddrObj, "SubnetMask"));
		CurrGatewayIPAddress = json_object_get_string(json_object_object_get(v4FwdObj, "GatewayIPAddress"));
			
		if(json_object_object_get(Jobj, "IPAddress")){
			if(strcmp(IPAddress, CurrIPAddress)){
				json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));
				json_object_object_add(v4AddrObj, "IPAddress", json_object_new_string(IPAddress));
				ipdnsmodify = true;
			}
		}
		if(json_object_object_get(Jobj, "SubnetMask")){
			if(strcmp(SubnetMask, CurrSubnetMask)){
				json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));
				json_object_object_add(v4AddrObj, "SubnetMask", json_object_new_string(SubnetMask));
				ipdnsmodify = true;
			}
		}
		if(!ipdnsmodify){
			if(v4AddrObj) {zcfgFeJsonObjFree(v4AddrObj);}
		}

		//v4 fwd
		if(v4FwdObj){
            //json_object_object_add(v4FwdObj, "StaticRoute", json_object_new_boolean(true)); //readonly parameter, cant set from front end.
			if( (strlen(GatewayIPAddress) >0) && strcmp(GatewayIPAddress, CurrGatewayIPAddress)){
				//json_object_object_add(v4FwdObj, "Interface", json_object_new_string(ipIfacePath));
				json_object_object_add(v4FwdObj, "GatewayIPAddress", json_object_new_string(GatewayIPAddress));
			}
			else{
				if(v4FwdObj) {zcfgFeJsonObjFree(v4FwdObj);}
			}
		}
	}
	else if(CurrIpStatic && !ipStatic){ ////enable to disable
		//v4 addr, disable or delete or do nothing?
		if(v4AddrObj)
		{	json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(false));	}
		//v4 fwd
		if(v4FwdObj)
		{
			json_object_object_add(v4FwdObj, "Interface", json_object_new_string(""));
			//json_object_object_add(v4FwdObj, "StaticRoute", json_object_new_boolean(false));
		}
	}
	else if(!CurrIpStatic && ipStatic){ //disable to enable
		if(v4AddrObj == NULL){
			oid = RDM_OID_IP_IFACE_V4_ADDR;
			IID_INIT(v4AddrIid);
			v4AddrIid.level = 1;
			sscanf(ipIfacePath, "IP.Interface.%hhu", &v4AddrIid.idx[0]);
			if(zcfgFeObjJsonAdd(oid, &v4AddrIid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(oid, &v4AddrIid, &v4AddrObj);
		}
		if(v4AddrObj){
			json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(v4AddrObj, "AddressingType", json_object_new_string("Static"));
			replaceParam(v4AddrObj, "IPAddress", Jobj, "IPAddress");
			replaceParam(v4AddrObj, "SubnetMask", Jobj, "SubnetMask");
		}


		if(v4FwdObj == NULL){
			oid = RDM_OID_ROUTING_ROUTER_V4_FWD;
			if(!getSpecificObj(oid, "Interface", json_type_string, "", &v4FwdIid, &v4FwdObj)){
				IID_INIT(v4FwdIid);
				v4FwdIid.level = 1;
				v4FwdIid.idx[0] = 1;
				if(zcfgFeObjJsonAdd(oid, &v4FwdIid, NULL) == ZCFG_SUCCESS){
					zcfgFeObjJsonGetWithoutUpdate(oid, &v4FwdIid, &v4FwdObj);
				}
			}
		}
		if(v4FwdObj){
			json_object_object_add(v4FwdObj, "Enable", json_object_new_boolean(true));
			//json_object_object_add(v4FwdObj, "StaticRoute", json_object_new_boolean(true));
			json_object_object_add(v4FwdObj, "Origin", json_object_new_string("Static"));
			json_object_object_add(v4FwdObj, "Interface", json_object_new_string(ipIfacePath));
			json_object_object_add(v4FwdObj, "DestIPAddress", json_object_new_string(""));
			json_object_object_add(v4FwdObj, "DestSubnetMask", json_object_new_string(""));
			replaceParam(v4FwdObj, "GatewayIPAddress", Jobj, "GatewayIPAddress");
		}
	}
	else{//do nothing for disable to disable
		if(v4AddrObj) {zcfgFeJsonObjFree(v4AddrObj);}
		if(v4FwdObj) {zcfgFeJsonObjFree(v4FwdObj);}
	}

	//static v6ip
	if(CurrIp6Static && ip6Static){ //enable to enable
		//v6 addr
		IP6Address = json_object_get_string(json_object_object_get(Jobj, "IP6Address"));
		// NextHop = json_object_get_string(json_object_object_get(Jobj, "NextHop"));
		CurrIP6Address = json_object_get_string(json_object_object_get(v6AddrObj, "IPAddress"));
		// CurrNextHop = json_object_get_string(json_object_object_get(v6FwdObj, "NextHop"));
		
		if(strcmp(IP6Address, CurrIP6Address)){
			json_object_object_add(v6AddrObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(v6AddrObj, "IPAddress", json_object_new_string(IP6Address));
		}else{
			if(v6AddrObj) {zcfgFeJsonObjFree(v6AddrObj);}
		}

		//v6 fwd
		if(v6FwdObj == NULL){
			oid = RDM_OID_ROUTING_ROUTER_V6_FWD;
			if(!getSpecificObj(oid, "Interface", json_type_string, "", &v6FwdIid, &v6FwdObj)){
				IID_INIT(v6FwdIid);
				v6FwdIid.level = 1;
				v6FwdIid.idx[0] = 1;
				if(zcfgFeObjJsonAdd(oid, &v6FwdIid, NULL) == ZCFG_SUCCESS)
					zcfgFeObjJsonGetWithoutUpdate(oid, &v6FwdIid, &v6FwdObj);
			}
		}
		if(v6FwdObj){
			json_object_object_add(v6FwdObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(v6FwdObj, "Origin", json_object_new_string("Static"));
			json_object_object_add(v6FwdObj, "Interface", json_object_new_string(ipIfacePath));
			json_object_object_add(v6FwdObj, "DestIPPrefix", json_object_new_string("::/0"));
			replaceParam(v6FwdObj, "NextHop", Jobj, "NextHop");
		}
	}
	else if(CurrIp6Static && !ip6Static){ ////enable to disable
		//v6 addr, disable or delete or do nothing?
		if(v6AddrObj)
		{	json_object_object_add(v6AddrObj, "Enable", json_object_new_boolean(false));	}
		//v6 fwd
		if(v6FwdObj)
		{   json_object_object_add(v6FwdObj, "Interface", json_object_new_string(""));	}
	}
	else if(!CurrIp6Static && ip6Static){ //disable to enable
		if(v6AddrObj == NULL){
			oid = RDM_OID_IP_IFACE_V6_ADDR;
			IID_INIT(v6AddrIid);
			v6AddrIid.level = 1;
			sscanf(ipIfacePath, "IP.Interface.%hhu", &v6AddrIid.idx[0]);
			if(zcfgFeObjJsonAdd(oid, &v6AddrIid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(oid, &v6AddrIid, &v6AddrObj);
		}
		if(v6AddrObj){
			json_object_object_add(v6AddrObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(v6AddrObj, "Origin", json_object_new_string("Static"));
			replaceParam(v6AddrObj, "IPAddress", Jobj, "IP6Address");
		}


		if(v6FwdObj == NULL){
			oid = RDM_OID_ROUTING_ROUTER_V6_FWD;
			if(!getSpecificObj(oid, "Interface", json_type_string, "", &v6FwdIid, &v6FwdObj)){
				IID_INIT(v6FwdIid);
				v6FwdIid.level = 1;
				v6FwdIid.idx[0] = 1;
				if(zcfgFeObjJsonAdd(oid, &v6FwdIid, NULL) == ZCFG_SUCCESS)
					zcfgFeObjJsonGetWithoutUpdate(oid, &v6FwdIid, &v6FwdObj);
			}
		}
		if(v6FwdObj){
			json_object_object_add(v6FwdObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(v6FwdObj, "Origin", json_object_new_string("Static"));
			json_object_object_add(v6FwdObj, "Interface", json_object_new_string(ipIfacePath));
			json_object_object_add(v6FwdObj, "DestIPPrefix", json_object_new_string("::/0"));
			replaceParam(v6FwdObj, "NextHop", Jobj, "NextHop");
		}
	}
	else{//do nothing for disable to disable
		if(v6AddrObj) {zcfgFeJsonObjFree(v6AddrObj);}
		if(v6FwdObj) {zcfgFeJsonObjFree(v6FwdObj);}
	}

	return ret;
}

zcfgRet_t getDhcpv4ClientOpt(objIndex_t *dhcpv4ClinetIid){
	zcfg_offset_t oid;
	objIndex_t iid;
	struct json_object *obj = NULL;
	int tag;
	
	IID_INIT(iid);
	oid = RDM_OID_DHCPV4_REQ_OPT;
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(oid, dhcpv4ClinetIid, &iid, &obj)== ZCFG_SUCCESS) {
		tag = json_object_get_int(json_object_object_get(obj, "Tag"));
		if(tag == 42){
			memcpy(&opt42Iid, &iid, sizeof(objIndex_t));
			opt42Obj = obj;
		}
		else if(tag == 43){
			memcpy(&opt43Iid, &iid, sizeof(objIndex_t));
			opt43Obj = obj;
		}
		else if(tag == 120){
			memcpy(&opt120Iid, &iid, sizeof(objIndex_t));
			opt120Obj = obj;
		}
		else if(tag == 121){
			memcpy(&opt121Iid, &iid, sizeof(objIndex_t));
			opt121Obj = obj;
		}
		else if(tag == 212){
			memcpy(&opt212Iid, &iid, sizeof(objIndex_t));
			opt212Obj = obj;
		}
		else
			json_object_put(obj);
	}

	IID_INIT(iid);
	oid = RDM_OID_DHCPV4_SENT_OPT;
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(oid, dhcpv4ClinetIid, &iid, &obj)== ZCFG_SUCCESS) {
		tag = json_object_get_int(json_object_object_get(obj, "Tag"));
		if(tag == 60){
			memcpy(&opt60Iid, &iid, sizeof(objIndex_t));
			opt60Obj = obj;
		}
		else if(tag == 61){
			memcpy(&opt61Iid, &iid, sizeof(objIndex_t));
			opt61Obj = obj;
		}
		else if(tag == 125){
			memcpy(&opt125Iid, &iid, sizeof(objIndex_t));
			opt125Obj = obj;
		}
		else
			json_object_put(obj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t setDhcpReqOption(struct json_object *Jobj, struct json_object **optObj, objIndex_t *optIid, objIndex_t *dhcpClientIid, int Tag, char *Alias){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//char optEnableName[32] = {0};
	zcfg_offset_t oid;
	//const char *ipMode, *v6RD_Type;
	//bool Enable_6RD, Enable = false;

	oid = RDM_OID_DHCPV4_REQ_OPT;
	
#if 0
	sprintf(optEnableName, "option%dEnable", Tag); //ex. option43Enable
	ipMode = json_object_get_string(json_object_object_get(Jobj, "ipMode"));
	Enable_6RD = json_object_get_boolean(json_object_object_get(Jobj, "Enable_6RD"));
	v6RD_Type = json_object_get_string(json_object_object_get(Jobj, "v6RD_Type"));

	if(Tag == 212){ //6RD
		if(!strcmp(ipMode, "IPv4") && Enable_6RD && !strcmp(v6RD_Type, "dhcp"))
			Enable = true;
	}else{
		Enable = json_object_get_boolean(json_object_object_get(Jobj, optEnableName));
	}
#endif

	if(*optObj){
		//if(Enable)
		json_object_object_add(*optObj, "Enable", json_object_new_boolean(true));
		json_object_object_add(*optObj, "Tag", json_object_new_int(Tag));
		json_object_object_add(*optObj, "Alias", json_object_new_string(Alias));
		//else
		//json_object_object_add(*optObj, "Enable", json_object_new_boolean(false));
		
		//zcfgFeObjJsonSetWithoutApply(oid, optIid, *optObj, NULL);
	}
	else{
		//if(Enable){
			memcpy(optIid, dhcpClientIid, sizeof(objIndex_t));
			zcfgFeObjJsonAdd(oid, optIid, NULL);
			zcfgFeObjJsonGetWithoutUpdate(oid, optIid, optObj);
			json_object_object_add(*optObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(*optObj, "Tag", json_object_new_int(Tag));
			json_object_object_add(*optObj, "Alias", json_object_new_string(Alias));
			//zcfgFeObjJsonSetWithoutApply(oid, optIid, *optObj, NULL);
		//}
	}

	return ret;
}

zcfgRet_t setDhcpSentOption(struct json_object *Jobj, struct json_object **optObj, objIndex_t *optIid, objIndex_t *dhcpClientIid, int Tag){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//char optEnableName[32] = {0};
	char optValueName[32] = {0};
	zcfg_offset_t oid;
	//bool Enable = false;
	
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
	char tmp[OPT_125_VAL_ENCODE_ENCVAL]={0};
	char Value[OPT_61_VAL_ENCODE_ENCVAL]={0};
#else
	char tmp[256]={0};
	char Value[256]={0};
#endif
	char *opt60Val = NULL;
	
	oid = RDM_OID_DHCPV4_SENT_OPT;
	//sprintf(optEnableName, "option%dEnable", Tag); //ex. option60Enable
	sprintf(optValueName, "option%dValue", Tag); //ex. option60Value
	//Enable = json_object_get_boolean(json_object_object_get(Jobj, optEnableName));

	if(Tag==60){
		opt60Val = (char *)json_object_get_string(json_object_object_get(Jobj, "option60Value"));
		ZOS_STRCPY_M(tmp,opt60Val);
		opt60ValEncode(tmp,Value,Tag);
		json_object_object_add(Jobj,"option60Value",json_object_new_string(Value));
	}
	else if(Tag==61){
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
		bool bParameter = true;
		char *tmpIaid = NULL;
		int tmpDuid = 0;
		int tmpEnterprise = 0;
		char *tmpIdentifier = NULL;
		if(json_object_object_get(Jobj, "option61Iaid"))
			tmpIaid = json_object_get_string(json_object_object_get(Jobj, "option61Iaid"));
		else
			bParameter = false;

		if(json_object_object_get(Jobj, "option61Duid"))
			tmpDuid = json_object_get_int(json_object_object_get(Jobj, "option61Duid"));
		else
			bParameter = false;

		if(json_object_object_get(Jobj, "option61Enterprise"))
			tmpEnterprise = json_object_get_int(json_object_object_get(Jobj, "option61Enterprise"));
		else
			bParameter = false;

		if(json_object_object_get(Jobj, "option61Identifier"))
			tmpIdentifier = json_object_get_string(json_object_object_get(Jobj, "option61Identifier"));
		else
			bParameter = false;

		if(bParameter){
			opt61ValEncode(tmpIaid,tmpDuid,tmpEnterprise,tmpIdentifier,Value,Tag);
			json_object_object_add(Jobj,"option61Value",json_object_new_string(Value));
		}
#else //CONFIG_ABZQ_CUSTOMIZATION

#ifndef ABUU_CUSTOMIZATION
		const char *dhcpOption61Mode = featureFlagObj ? json_object_get_string(json_object_object_get(featureFlagObj, "DHCPOption61Mode")) : NULL;
		char *tmpIaid = NULL;
		char *tmpDuid = NULL;
		if(dhcpOption61Mode && !strcmp(dhcpOption61Mode, "ADDRDOMAIN")){
			json_object_object_add(Jobj, "option61Value", json_object_new_string(""));
		}else{
			// default DUIDEN
			if(json_object_object_get(Jobj, "option61Iaid"))
				tmpIaid = (char *)json_object_get_string(json_object_object_get(Jobj, "option61Iaid"));
			if(json_object_object_get(Jobj, "option61Duid"))
				tmpDuid = (char *)json_object_get_string(json_object_object_get(Jobj, "option61Duid"));
			if(json_object_object_get(Jobj, "option61Iaid")&&json_object_object_get(Jobj, "option61Duid")){
				opt61ValEncode(tmpIaid,tmpDuid,Value,Tag);
				json_object_object_add(Jobj,"option61Value",json_object_new_string(Value));
			}
		}
#endif
#endif //CONFIG_ABZQ_CUSTOMIZATION
	}
    else if(Tag == 125){
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
	bool bParameter = true;
	char *tmpOUI = NULL;
	char *tmpClass = NULL;
	char *tmpModel = NULL;
	char *tmpSN = NULL;

	if(json_object_object_get(Jobj, "option125ManufacturerOUI"))
		tmpOUI = json_object_get_string(json_object_object_get(Jobj, "option125ManufacturerOUI"));
	else
		bParameter = false;
	if(json_object_object_get(Jobj, "option125ProductClass"))
		tmpClass = json_object_get_string(json_object_object_get(Jobj, "option125ProductClass"));
	else
		bParameter = false;
	if(json_object_object_get(Jobj, "option125ModelName"))
		tmpModel = json_object_get_string(json_object_object_get(Jobj, "option125ModelName"));
	else
		bParameter = false;
	if(json_object_object_get(Jobj, "option125SerialNumber"))
		tmpSN = json_object_get_string(json_object_object_get(Jobj, "option125SerialNumber"));
	else
		bParameter = false;

	if(bParameter){
		strncpy(tmp,"",OPT_125_VAL_ENCODE_ENCVAL);
		opt125ValEncode (tmpOUI,tmpClass,tmpModel,tmpSN,tmp,Tag);
		strncpy(Value, tmp,OPT_125_VAL_ENCODE_ENCVAL);
	}
#else
        opt125ValGet(Value);
#endif //CONFIG_ABZQ_CUSTOMIZATION

        json_object_object_add(Jobj,"option125Value",json_object_new_string(Value));
    }

	if(*optObj){
		json_object_object_add(*optObj, "Enable", json_object_new_boolean(true));
		replaceParam(*optObj, "Value", Jobj, optValueName);
		//zcfgFeObjJsonSetWithoutApply(oid, optIid, *optObj, NULL);
	}
	else{
		//if(Enable){
			memcpy(optIid, dhcpClientIid, sizeof(objIndex_t));
			zcfgFeObjJsonAdd(oid, optIid, NULL);
			zcfgFeObjJsonGetWithoutUpdate(oid, optIid, optObj);
			json_object_object_add(*optObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(*optObj, "Tag", json_object_new_int(Tag));
			replaceParam(*optObj, "Value", Jobj, optValueName);
			//zcfgFeObjJsonSetWithoutApply(oid, optIid, *optObj, NULL);
		//}
	}

	return ret;
}

zcfgRet_t addDhcpSetting(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfg_offset_t oid;
	objIndex_t iid;
	//objIndex_t v4ClientIid;
	//objIndex_t opt43Iid = {0}, opt120Iid = {0}, opt121Iid = {0}, opt212Iid = {0};
	//objIndex_t opt60Iid = {0}, opt61Iid = {0}, opt125Iid = {0};
	struct json_object *obj = NULL;
	//struct json_object *v4ClientObj = NULL;
	//struct json_object *opt43obj = NULL, *opt120obj = NULL, *opt121obj = NULL, *opt212obj = NULL;
	//struct json_object *opt60obj = NULL, *opt61obj = NULL, *opt125obj = NULL;
	//const char *Encapsulation, *ipMode;
	//bool ipStatic;
	bool option42Enable=false,option43Enable=false, option120Enable=false, option121Enable=false, option212Enable=false;
	bool option60Enable=false, option61Enable=false, option125Enable=false;
	int tag;

	if(!strcmp(Encapsulation, "PPPoA") || !strcmp(Encapsulation, "PPPoE") || !strcmp(Encapsulation, TR181_PPtP2PPPOA))
		return ret;

	option42Enable = json_object_get_boolean(json_object_object_get(Jobj, "option42Enable"));
	option43Enable = json_object_get_boolean(json_object_object_get(Jobj, "option43Enable"));
	option120Enable = json_object_get_boolean(json_object_object_get(Jobj, "option120Enable"));
	option121Enable = json_object_get_boolean(json_object_object_get(Jobj, "option121Enable"));
	if(!strcmp(ipMode, "IPv4") && Enable_6RD && !strcmp(v6RD_Type, "dhcp"))
		option212Enable = json_object_get_boolean(json_object_object_get(Jobj, "option212Enable"));
	
	option60Enable = json_object_get_boolean(json_object_object_get(Jobj, "option60Enable"));
	option61Enable = json_object_get_boolean(json_object_object_get(Jobj, "option61Enable"));
	option125Enable = json_object_get_boolean(json_object_object_get(Jobj, "option125Enable"));
	
	if(strcmp(ipMode, "IPv6") && strcmp(Encapsulation, "IPoA") && !ipStatic){
		if(!getSpecificObj(RDM_OID_DHCPV4_CLIENT, "Interface", json_type_string, "", &v4ClientIid, &v4ClientObj))
		{
			IID_INIT(v4ClientIid);

			if(zcfgFeObjJsonAdd(RDM_OID_DHCPV4_CLIENT, &v4ClientIid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DHCPV4_CLIENT, &v4ClientIid, &v4ClientObj);
		}

		//option 42, 43, 120, 121, 212
		IID_INIT(iid);
		oid = RDM_OID_DHCPV4_REQ_OPT;
		while(zcfgFeSubInObjJsonGetNextWithoutUpdate(oid, &v4ClientIid, &iid, &obj)== ZCFG_SUCCESS) {
			tag = json_object_get_int(json_object_object_get(obj, "Tag"));
			if(tag == 42 && option42Enable){
				memcpy(&opt42Iid, &iid, sizeof(objIndex_t));
				opt42Obj = obj;
			}
			else if(tag == 43 && option43Enable){
				memcpy(&opt43Iid, &iid, sizeof(objIndex_t));
				opt43Obj = obj;
			}
			else if(tag == 120 && option120Enable){
				memcpy(&opt120Iid, &iid, sizeof(objIndex_t));
				opt120Obj = obj;
			}
			else if(tag == 121 && option121Enable){
				memcpy(&opt121Iid, &iid, sizeof(objIndex_t));
				opt121Obj = obj;
			}
			else if(tag == 212 && option212Enable){
				memcpy(&opt212Iid, &iid, sizeof(objIndex_t));
				opt212Obj = obj;
			}
			else
				json_object_put(obj);
		}

		if(option42Enable) setDhcpReqOption(Jobj, &opt42Obj, &opt42Iid, &v4ClientIid, 42, "ntpsrv");
		if(option43Enable) setDhcpReqOption(Jobj, &opt43Obj, &opt43Iid, &v4ClientIid, 43, "acsinfo");
		if(option120Enable) setDhcpReqOption(Jobj, &opt120Obj, &opt120Iid, &v4ClientIid, 120, "sipsrv");
		if(option121Enable) setDhcpReqOption(Jobj, &opt121Obj, &opt121Iid, &v4ClientIid, 121, "sroute");
		if(option212Enable) setDhcpReqOption(Jobj, &opt212Obj, &opt212Iid, &v4ClientIid, 212, "");
		
		//if(opt43Obj) json_object_put(opt43Obj);
		//if(opt120Obj) json_object_put(opt120Obj);
		//if(opt121Obj) json_object_put(opt121Obj);
		//if(opt212Obj) json_object_put(opt212Obj);


		//option 60, 61, 125
		IID_INIT(iid);
		oid = RDM_OID_DHCPV4_SENT_OPT;
		while(zcfgFeSubInObjJsonGetNextWithoutUpdate(oid, &v4ClientIid, &iid, &obj)== ZCFG_SUCCESS) {
			tag = json_object_get_int(json_object_object_get(obj, "Tag"));
			if(tag == 60 && option60Enable){
				memcpy(&opt60Iid, &iid, sizeof(objIndex_t));
				opt60Obj = obj;
			}
			else if(tag == 61 && option61Enable){
				memcpy(&opt61Iid, &iid, sizeof(objIndex_t));
				opt61Obj = obj;
			}
			else if(tag == 125 && option125Enable){
				memcpy(&opt125Iid, &iid, sizeof(objIndex_t));
				opt125Obj = obj;
			}
			else
				json_object_put(obj);
		}

		if(option60Enable) setDhcpSentOption(Jobj, &opt60Obj, &opt60Iid, &v4ClientIid, 60);
		if(option61Enable) setDhcpSentOption(Jobj, &opt61Obj, &opt61Iid, &v4ClientIid, 61);
		if(option125Enable) setDhcpSentOption(Jobj, &opt125Obj, &opt125Iid, &v4ClientIid, 125);
		
		//if(opt60Obj) json_object_put(opt60Obj);
		//if(opt61Obj) json_object_put(opt61Obj);
		//if(opt125Obj) json_object_put(opt125Obj);

		//set DHCPv4 Client
		json_object_object_add(v4ClientObj, "Enable", json_object_new_boolean(true));
		json_object_object_add(v4ClientObj, "Interface", json_object_new_string(ipIfacePath));
		//ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_CLIENT, &v4ClientIid, v4ClientObj, NULL);
		//json_object_put(v4ClientObj);
	}

	return ret;
}


zcfgRet_t editDhcpReqOption(struct json_object *Jobj, struct json_object **optObj, objIndex_t *optIid, objIndex_t *dhcpClientIid, int Tag, char *Alias){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char optEnableName[32] = {0};
	zcfg_offset_t oid;
	//const char *ipMode, *v6RD_Type;
	bool optEnable = false, CurrOptEnable = false;
	struct json_object *pramJobj = NULL;
	bool needSet = false;

	oid = RDM_OID_DHCPV4_REQ_OPT;

	if(*optObj){
		if(json_object_get_boolean(json_object_object_get(*optObj, "Enable")))
			CurrOptEnable = true;
	}
	
	sprintf(optEnableName, "option%dEnable", Tag); //ex. option43Enable
	pramJobj = json_object_object_get(Jobj, optEnableName);
	if(pramJobj)
		optEnable = json_object_get_boolean(pramJobj);
	else
		optEnable = CurrOptEnable;

	if(CurrOptEnable && !optEnable){ //enable to disable
		if(*optObj){
			json_object_object_add(*optObj, "Enable", json_object_new_boolean(false));
			json_object_object_add(*optObj, "Value", json_object_new_string(""));
			needSet = true;
		}
	}
	else if(!CurrOptEnable && optEnable){ //disable to enable
		if(*optObj){
			json_object_object_add(*optObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(*optObj, "Alias", json_object_new_string(Alias));
		}
		else{
			memcpy(optIid, dhcpClientIid, sizeof(objIndex_t));
			if(zcfgFeObjJsonAdd(oid, optIid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(oid, optIid, optObj);
			if(*optObj){
				json_object_object_add(*optObj, "Enable", json_object_new_boolean(true));
				json_object_object_add(*optObj, "Tag", json_object_new_int(Tag));
				json_object_object_add(*optObj, "Alias", json_object_new_string(Alias));
			}
		}
		needSet = true;
	}

	if(!needSet){
		if(*optObj) {zcfgFeJsonObjFree(*optObj);}
	}

	return ret;
}

zcfgRet_t editDhcpSentOption(struct json_object *Jobj, struct json_object **optObj, objIndex_t *optIid, objIndex_t *dhcpClientIid, int Tag){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char optEnableName[32] = {0};
	char optValueName[32] = {0};
	zcfg_offset_t oid;
	//const char *ipMode, *v6RD_Type;
	bool optEnable = false, CurrOptEnable = false;
	const char *CurrValue = "";
	struct json_object *pramJobj = NULL;
	bool needSet = false;
	char tmp[256]={0}, Value[256]={0};
	char *newValue = NULL;


	oid = RDM_OID_DHCPV4_SENT_OPT;
	const char *dhcpOption61Mode = featureFlagObj ? json_object_get_string(json_object_object_get(featureFlagObj, "DHCPOption61Mode")) : NULL;
	
	if(*optObj){
		CurrOptEnable = json_object_get_boolean(json_object_object_get(*optObj, "Enable"));
		CurrValue = (char *)json_object_get_string(json_object_object_get(*optObj, "Value"));
		if(CurrValue == NULL)
			CurrValue = "";
	}
	
	sprintf(optEnableName, "option%dEnable", Tag); //ex. option60Enable
	pramJobj = json_object_object_get(Jobj, optEnableName);
	if(pramJobj)
		optEnable = json_object_get_boolean(pramJobj);
	else
		optEnable = CurrOptEnable;

	sprintf(optValueName, "option%dValue", Tag); //ex. option60Value
	pramJobj = json_object_object_get(Jobj, optValueName);
	if(pramJobj)
		newValue = (char *)json_object_get_string(pramJobj);
	else
		newValue = (char *)CurrValue;

	if(Tag==60 && optEnable){
		if(json_object_object_get(Jobj, "option60Value") == NULL) {
			//no need to encrypt for curr val which is already encrypted
			ZOS_STRCPY_M(Value, CurrValue);
		}
		else {
			ZOS_STRCPY_M(tmp,newValue);
			opt60ValEncode(tmp,Value,Tag);
		}
	}
	else if(Tag==61 && optEnable){
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
		bool bParameter = true;
		char *tmpIaid = NULL;
		int tmpDuid = 0;
		int tmpEnterprise = 0;
		char *tmpIdentifier = NULL;

		if(json_object_object_get(Jobj, "option61Iaid"))
			tmpIaid = json_object_get_string(json_object_object_get(Jobj, "option61Iaid"));
		else
			bParameter = false;

		if(json_object_object_get(Jobj, "option61Duid"))
			tmpDuid = json_object_get_int(json_object_object_get(Jobj, "option61Duid"));
		else
			bParameter = false;

		if(json_object_object_get(Jobj, "option61Enterprise"))
			tmpEnterprise = json_object_get_int(json_object_object_get(Jobj, "option61Enterprise"));
		else
			bParameter = false;

		if(json_object_object_get(Jobj, "option61Identifier"))
			tmpIdentifier = json_object_get_string(json_object_object_get(Jobj, "option61Identifier"));
		else
			bParameter = false;

		if(bParameter){
			strncpy(tmp,"", (sizeof(tmp)-1));
			opt61ValEncode(tmpIaid,tmpDuid,tmpEnterprise,tmpIdentifier,tmp,Tag);
			strncpy(Value, tmp,(sizeof(tmp)-1));
		}
#else //CONFIG_ABZQ_CUSTOMIZATION

#ifndef ABUU_CUSTOMIZATION
		char *tmpIaid = NULL;
		char *tmpDuid = NULL;
#endif
		if(dhcpOption61Mode && !strcmp(dhcpOption61Mode, "ADDRDOMAIN")){
			ZOS_STRCPY_M(Value, CurrValue);
		}else{
#ifndef ABUU_CUSTOMIZATION
			if(json_object_object_get(Jobj, "option61Iaid"))
				tmpIaid = (char *)json_object_get_string(json_object_object_get(Jobj, "option61Iaid"));
			if(json_object_object_get(Jobj, "option61Duid"))
				tmpDuid = (char *)json_object_get_string(json_object_object_get(Jobj, "option61Duid"));
			if(json_object_object_get(Jobj, "option61Iaid")&&json_object_object_get(Jobj, "option61Duid")){
				ZOS_STRCPY_M(tmp,"");
				opt61ValEncode(tmpIaid,tmpDuid,tmp,Tag);
				ZOS_STRCPY_M(Value, tmp);
			}
#endif
		}
#endif //CONFIG_ABZQ_CUSTOMIZATION
	}
	else if(Tag==125 && optEnable){
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
		bool bParameter = true;
		char *tmpOUI = NULL;
		char *tmpClass = NULL;
		char *tmpModel = NULL;
		char *tmpSN = NULL;

		if(json_object_object_get(Jobj, "option125ManufacturerOUI"))
			tmpOUI = json_object_get_string(json_object_object_get(Jobj, "option125ManufacturerOUI"));
		else
			bParameter = false;
		if(json_object_object_get(Jobj, "option125ProductClass"))
			tmpClass = json_object_get_string(json_object_object_get(Jobj, "option125ProductClass"));
		else
			bParameter = false;
		if(json_object_object_get(Jobj, "option125ModelName"))
			tmpModel = json_object_get_string(json_object_object_get(Jobj, "option125ModelName"));
		else
			bParameter = false;
		if(json_object_object_get(Jobj, "option125SerialNumber"))
			tmpSN = json_object_get_string(json_object_object_get(Jobj, "option125SerialNumber"));
		else
			bParameter = false;

		if(bParameter){
			strncpy(tmp,"",OPT_125_VAL_ENCODE_ENCVAL-1);
			opt125ValEncode (tmpOUI,tmpClass,tmpModel,tmpSN,tmp,Tag);
			strncpy(Value, tmp,OPT_125_VAL_ENCODE_ENCVAL-1);
		}
#else //CONFIG_ABZQ_CUSTOMIZATION

        if(strlen(newValue) == 0){
            opt125ValGet(Value);
        }
        else{
            snprintf(Value, sizeof(Value), "%s", newValue);
        }
#endif //CONFIG_ABZQ_CUSTOMIZATION

    }

	if(CurrOptEnable && optEnable){//enable to enable
		if(*optObj && strcmp(CurrValue, Value)){
			json_object_object_add(*optObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(*optObj, "Value", json_object_new_string(Value));
			needSet = true;
		}
	}
	else if(CurrOptEnable && !optEnable){ //enable to disable
		if(*optObj){
			json_object_object_add(*optObj, "Enable", json_object_new_boolean(false));
			json_object_object_add(*optObj, "Value", json_object_new_string(""));
			needSet = true;
		}
	}
	else if(!CurrOptEnable && optEnable){ //disable to enable
		if(*optObj){
			json_object_object_add(*optObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(*optObj, "Value", json_object_new_string(Value));
		}
		else{
			memcpy(optIid, dhcpClientIid, sizeof(objIndex_t));
			zcfgFeObjJsonAdd(oid, optIid, NULL);
			zcfgFeObjJsonGetWithoutUpdate(oid, optIid, optObj);
			json_object_object_add(*optObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(*optObj, "Tag", json_object_new_int(Tag));
			json_object_object_add(*optObj, "Value", json_object_new_string(Value));

			memcpy(optIid, dhcpClientIid, sizeof(objIndex_t));
			zcfgFeObjJsonAdd(oid, optIid, NULL);
			zcfgFeObjJsonGetWithoutUpdate(oid, optIid, optObj);
			json_object_object_add(*optObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(*optObj, "Tag", json_object_new_int(Tag));
			replaceParam(*optObj, "Value", Jobj, optValueName);
		}
		needSet = true;
	}

	if(!needSet)
		if(*optObj) {zcfgFeJsonObjFree(*optObj);}

	if(*optObj) printf("%s:%d : *optObj = %s\n", __FUNCTION__, __LINE__, json_object_to_json_string(*optObj));
	return ret;
}

void editDhcpOption(struct json_object *Jobj, int action){

	if(action == dal_Delete){
		json_object_object_add(Jobj, "option42Enable", json_object_new_boolean(false));
        json_object_object_add(Jobj, "option43Enable", json_object_new_boolean(false));
		json_object_object_add(Jobj, "option120Enable", json_object_new_boolean(false));
		json_object_object_add(Jobj, "option121Enable", json_object_new_boolean(false));
		json_object_object_add(Jobj, "option212Enable", json_object_new_boolean(false));
		json_object_object_add(Jobj, "option60Enable", json_object_new_boolean(false));
		json_object_object_add(Jobj, "option61Enable", json_object_new_boolean(false));
		json_object_object_add(Jobj, "option125Enable", json_object_new_boolean(false));
	}

	editDhcpReqOption(Jobj, &opt42Obj, &opt42Iid, &v4ClientIid, 42, "ntpsrv");
	editDhcpReqOption(Jobj, &opt43Obj, &opt43Iid, &v4ClientIid, 43, "acsinfo");
	editDhcpReqOption(Jobj, &opt120Obj, &opt120Iid, &v4ClientIid, 120, "sipsrv");
	editDhcpReqOption(Jobj, &opt121Obj, &opt121Iid, &v4ClientIid, 121, "sroute");
	editDhcpReqOption(Jobj, &opt212Obj, &opt212Iid, &v4ClientIid, 212, "");
	editDhcpSentOption(Jobj, &opt60Obj, &opt60Iid, &v4ClientIid, 60);
	editDhcpSentOption(Jobj, &opt61Obj, &opt61Iid, &v4ClientIid, 61);
	editDhcpSentOption(Jobj, &opt125Obj, &opt125Iid, &v4ClientIid, 125);
	
}

zcfgRet_t editDhcpSetting(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfg_offset_t oid;
	struct json_object *pramJobj = NULL;
	bool dhcpEnable = false;
	bool CrrrDhcpEnable = false;
	bool setObj = false;

	if(v4ClientObj != NULL) CrrrDhcpEnable = true;

	pramJobj = json_object_object_get(Jobj, "ipStatic");
	if(pramJobj)
		dhcpEnable = !json_object_get_boolean(pramJobj);
	else if(strstr(CurrEncapsulation, "PPP") != NULL && !strcmp(Encapsulation, "IPoE")) //PPPoX to IPoE and not set static IP
		dhcpEnable = true;
	else
		dhcpEnable = CrrrDhcpEnable;

	if(!strcmp(Encapsulation, "PPPoE") || !strcmp(Encapsulation, "PPPoA") || !strcmp(Encapsulation, TR181_PPtP2PPPOA) || !strcmp(ipMode, "IPv6"))
		dhcpEnable = false;

	dbg_printf("CrrrDhcpEnable=%d dhcpEnable=%d\n", CrrrDhcpEnable, dhcpEnable);

	if(CrrrDhcpEnable && dhcpEnable){
		json_object_object_add(v4ClientObj, "Enable", json_object_new_boolean(true));
		editDhcpOption(Jobj, dal_Edit);
	}
	else if(CrrrDhcpEnable && !dhcpEnable){
		editDhcpOption(Jobj, dal_Delete);
		json_object_object_add(v4ClientObj, "Interface", json_object_new_string(""));
		setObj = true;
	}
	else if(!CrrrDhcpEnable && dhcpEnable){
		oid = RDM_OID_DHCPV4_CLIENT;
		if(!getSpecificObj(oid, "Interface", json_type_string, "", &v4ClientIid, &v4ClientObj)){
			IID_INIT(v4ClientIid);
			if(zcfgFeObjJsonAdd(oid, &v4ClientIid, NULL) == ZCFG_SUCCESS)
				zcfgFeObjJsonGetWithoutUpdate(oid, &v4ClientIid, &v4ClientObj);
		}
		if(v4ClientObj){
			json_object_object_add(v4ClientObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(v4ClientObj, "Interface", json_object_new_string(ipIfacePath));
			setObj = true;
		}

		getDhcpv4ClientOpt(&v4ClientIid);
		editDhcpOption(Jobj, dal_Add);
	}

	if(!setObj){
		if(v4ClientObj) {zcfgFeJsonObjFree(v4ClientObj);}
	}

	return ret;
}

zcfgRet_t editIgmpMldSetting(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//objIndex_t iid;
	//struct json_object *obj = NULL;
	const char *Interface = NULL;
	char newInterface[512] = {0};
	struct json_object *pramJobj = NULL;
	bool igmpEnable = false, mldEnable = false;
	bool CurrIgmpEnable = false, CurrMldEnable = false;
#ifdef ZyXEL_WEB_GUI_SHOW_IGMP_VERSION	
	int igmpVersion = 0;
#endif	
	if(igmpObj == NULL){ //add case
		IID_INIT(igmpIid);
		zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IGMP, &igmpIid, &igmpObj);
	}
	if(mldObj == NULL){ //add case
		IID_INIT(mldIid);
		zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MLD, &mldIid, &mldObj);
	}
		
	Interface = json_object_get_string(json_object_object_get(igmpObj, "Interface"));
	if(strstr(Interface, ipIfacePath))
		CurrIgmpEnable = true;

	Interface = json_object_get_string(json_object_object_get(mldObj, "Interface"));
	if(strstr(Interface, ipIfacePath))
		CurrMldEnable = true;

	pramJobj = json_object_object_get(Jobj, "IGMPEnable");
	if(pramJobj)
		igmpEnable = json_object_get_boolean(pramJobj);
	else
		igmpEnable = CurrIgmpEnable;

	pramJobj = json_object_object_get(Jobj, "MLDEnable");
	if(pramJobj)
		mldEnable = json_object_get_boolean(pramJobj);
	else
		mldEnable = CurrMldEnable;
	

	/* IGMP */
#ifdef ZyXEL_WEB_GUI_SHOW_IGMP_VERSION	
	igmpVersion = json_object_get_int(json_object_object_get(Jobj, "IGMPVersion"));
#endif	
	Interface = json_object_get_string(json_object_object_get(igmpObj, "Interface"));
	if(CurrIgmpEnable && igmpEnable){ //enable to enable
#ifdef ZyXEL_WEB_GUI_SHOW_IGMP_VERSION	
		json_object_object_add(igmpObj, "IgmpVersion", json_object_new_int(igmpVersion));
#else	
		if(igmpObj) {zcfgFeJsonObjFree(igmpObj);}
#endif		
	}
	else if(CurrIgmpEnable && !igmpEnable){ ////enable to disable
		if(genNewInterface(",", newInterface, Interface, ipIfacePath, false)){
			json_object_object_add(igmpObj, "Interface", json_object_new_string(newInterface));
		}
		else{
			if(igmpObj) {zcfgFeJsonObjFree(igmpObj);}
		}
	}
	else if(!CurrIgmpEnable && igmpEnable){ //disable to enable
#ifndef ZyXEL_WEB_GUI_SINGLE_IGMP_MLD
		if(genNewInterface(",", newInterface, Interface, ipIfacePath, true)){
			json_object_object_add(igmpObj, "Interface", json_object_new_string(newInterface));
#ifdef ZyXEL_WEB_GUI_SHOW_IGMP_VERSION			
			json_object_object_add(igmpObj, "IgmpVersion", json_object_new_int(igmpVersion));
#endif			
		}
#else
		char IgmpIntf[512] = {0};
		DisableSameWanTypeIgmpMld(IgmpIntf, Interface, ipIfacePath, 0);
		if(genNewInterface(",", newInterface, IgmpIntf, ipIfacePath, true)){
			json_object_object_add(igmpObj, "Interface", json_object_new_string(newInterface));
#ifdef ZyXEL_WEB_GUI_SHOW_IGMP_VERSION			
			json_object_object_add(igmpObj, "IgmpVersion", json_object_new_int(igmpVersion));
#endif			
		}
#endif	
		else{
			if(igmpObj) {zcfgFeJsonObjFree(igmpObj);}
		}
	}
	else{//do nothing for disable to disable
		if(igmpObj) {zcfgFeJsonObjFree(igmpObj);}
	}

#ifdef ACAU_CUSTOMIZATION /* Egypt TE */
		char cmd[64];

		snprintf(cmd, sizeof(cmd)-1, "echo %d > /var/snoop%d", json_object_get_boolean(json_object_object_get(Jobj, "IGMPSnooping")) ? 1 : 0, ipIfaceIid.idx[0]);
		system(cmd);
#endif

	/* MLD */
	Interface = json_object_get_string(json_object_object_get(mldObj, "Interface"));
	if(CurrMldEnable && mldEnable){ //enable to enable
		if(mldObj) {zcfgFeJsonObjFree(mldObj);}
	}
	else if(CurrMldEnable && !mldEnable){ ////enable to disable
		if(genNewInterface(",", newInterface, Interface, ipIfacePath, false)){
			json_object_object_add(mldObj, "Interface", json_object_new_string(newInterface));
		}
		else{
			if(mldObj) {zcfgFeJsonObjFree(mldObj);}
		}
	}
	else if(!CurrMldEnable && mldEnable){ //disable to enable
#ifndef ZyXEL_WEB_GUI_SINGLE_IGMP_MLD
		if(genNewInterface(",", newInterface, Interface, ipIfacePath, true)){
			json_object_object_add(mldObj, "Interface", json_object_new_string(newInterface));
		}
#else
		char MldIntf[512] = {0};
		DisableSameWanTypeIgmpMld(MldIntf, Interface, ipIfacePath, 0);	
		if(genNewInterface(",", newInterface, MldIntf, ipIfacePath, true)){
			json_object_object_add(mldObj, "Interface", json_object_new_string(newInterface));
		}
#endif
		else{
			if(mldObj) {zcfgFeJsonObjFree(mldObj);}
		}
	}
	else{//do nothing for disable to disable
		if(mldObj) {zcfgFeJsonObjFree(mldObj);}
	}

	return ret;
}

zcfgRet_t IpIfaceWanTypeGet(char *Ifname, char *WanType){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *HigherLayer;
	const char *LowerLayer;
	int count = 0;	
	
	//memset(WanType, 0, 16/*sizeof(WanType)*/); // already clean by caller.

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	struct json_object *atmObj = NULL;
	objIndex_t atmIid = {0};
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	struct json_object *ptmObj = NULL;
	objIndex_t ptmIid = {0};
#endif
	struct json_object *ethObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	struct json_object *optObj = NULL;
	objIndex_t optIid = {0};
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	struct json_object *cellObj = NULL;
#endif
	struct json_object *ethlink = NULL;
	struct json_object *vlanterm = NULL;
	struct json_object *pppObj = NULL;
	struct json_object *ipObj = NULL;
	objIndex_t ethIid = {0};
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	objIndex_t cellIid = {0};
#endif
	objIndex_t ethlinkIid = {0};
	objIndex_t vlantermIid = {0};
	objIndex_t pppIid = {0};	
	objIndex_t ipIid = {0};	
	
	IID_INIT(ipIid);
	ipIid.level = 1;
	sscanf(Ifname, "IP.Interface.%hhu", &ipIid.idx[0]);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &ipIid, &ipObj) != ZCFG_SUCCESS)
		return ZCFG_REQUEST_REJECT;		

	LowerLayer = json_object_get_string(json_object_object_get(ipObj, "LowerLayers"));		
	HigherLayer = "IP.Interface.";
	ZOS_STRCPY_M(CurrMode, json_object_get_string(json_object_object_get(ipObj, "X_ZYXEL_ConnectionType")));

	while(count < 4)
	{
		count++;
		if(!strncmp(HigherLayer, "IP.Interface.", 13))
		{
			if(!strncmp(LowerLayer, "PPP.Interface.", 14))
			{
				IID_INIT(pppIid);
				pppIid.level = 1;
				sscanf(LowerLayer, "PPP.Interface.%hhu", &pppIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PPP_IFACE, &pppIid, &pppObj);
				HigherLayer = LowerLayer;
				LowerLayer = json_object_get_string(json_object_object_get(pppObj, "LowerLayers"));
			}
			else if(!strncmp("Ethernet.VLANTermination.", LowerLayer, 25))
			{
				IID_INIT(vlantermIid);
				vlantermIid.level = 1;
				sscanf(LowerLayer, "Ethernet.VLANTermination.%hhu", &vlantermIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_VLAN_TERM, &vlantermIid, &vlanterm);
				HigherLayer = LowerLayer;
				LowerLayer = json_object_get_string(json_object_object_get(vlanterm, "LowerLayers"));
			}
			else if(!strncmp("Ethernet.Link.", LowerLayer, 14))
			{
				IID_INIT(ethlinkIid);
				ethlinkIid.level = 1;
				sscanf(LowerLayer, "Ethernet.Link.%hhu", &ethlinkIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_LINK, &ethlinkIid, &ethlink);
				HigherLayer = LowerLayer;
				LowerLayer = json_object_get_string(json_object_object_get(ethlink, "LowerLayers"));
			}
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
			else if(!strncmp("ATM.Link.", LowerLayer, 9) && !strcmp("IP_Routed", CurrMode))
			{
				IID_INIT(atmIid);
				atmIid.level = 1;
				sscanf(LowerLayer, "ATM.Link.%hhu", &atmIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmIid, &atmObj);		
				ZOS_STRCPY_M(WanType, "ATM");
				break;
			}
#endif
			else{
				ret = ZCFG_REQUEST_REJECT;
				goto out;
			}
		}
		else if(!strncmp(HigherLayer, "PPP.Interface.", 14))
		{
			if(!strncmp("Ethernet.VLANTermination.", LowerLayer, 25))
			{
				IID_INIT(vlantermIid);
				vlantermIid.level = 1;
				sscanf(LowerLayer, "Ethernet.VLANTermination.%hhu", &vlantermIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_VLAN_TERM, &vlantermIid, &vlanterm);
				HigherLayer = LowerLayer;
				LowerLayer = json_object_get_string(json_object_object_get(vlanterm, "LowerLayers"));		
			}
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
			else if(!strncmp("ATM.Link.", LowerLayer, 9))
			{
				IID_INIT(atmIid);
				atmIid.level = 1;
				sscanf(LowerLayer, "ATM.Link.%hhu", &atmIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmIid, &atmObj);
				ZOS_STRCPY_M(WanType, "ATM");
				break;
			}
#endif
			else
			{
				ret = ZCFG_REQUEST_REJECT;
				goto out;
			}
		}
		else if(!strncmp("Ethernet.VLANTermination.", HigherLayer, 25))
		{
			if(!strncmp("Ethernet.Link.", LowerLayer, 14))
			{
				IID_INIT(ethlinkIid);
				ethlinkIid.level = 1;
				sscanf(LowerLayer, "Ethernet.Link.%hhu", &ethlinkIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_LINK, &ethlinkIid, &ethlink);
				HigherLayer = LowerLayer;
				LowerLayer = json_object_get_string(json_object_object_get(ethlink, "LowerLayers"));
			}
			else
			{
				ret = ZCFG_REQUEST_REJECT;
				goto out;
			}
		}
		else if(!strncmp("Ethernet.Link.", HigherLayer, 14))
		{
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
			if(!strncmp("ATM.Link.", LowerLayer, 9))
			{
				IID_INIT(atmIid);
				atmIid.level = 1;
				sscanf(LowerLayer, "ATM.Link.%hhu", &atmIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmIid, &atmObj);
				ZOS_STRCPY_M(WanType, "ATM");
				break;
			}
			else
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
			if(!strncmp("PTM.Link.", LowerLayer, 9))
			{
				IID_INIT(ptmIid);
				ptmIid.level = 1;
				sscanf(LowerLayer, "PTM.Link.%hhu", &ptmIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PTM_LINK, &ptmIid, &ptmObj);
				ZOS_STRCPY_M(WanType, "PTM");
				break;
			}
			else
#endif
			if(!strncmp("Ethernet.Interface.", LowerLayer, 19))
			{
				IID_INIT(ethIid);
				ethIid.level = 1;
				sscanf(LowerLayer, "Ethernet.Interface.%hhu", &ethIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_IFACE, &ethIid, &ethObj);
				ZOS_STRCPY_M(WanType, "ETH");
				break;
			}
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
			else if(!strncmp("Optical.Interface.", LowerLayer, 18))
			{
				IID_INIT(optIid);
				optIid.level = 1;
				sscanf(LowerLayer, "Optical.Interface.%hhu", &optIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_OPT_INTF, &optIid, &optObj);				
				ZOS_STRCPY_M(WanType, "PON");
				break;
			}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
			else if(!strncmp("Cellular.Interface.", LowerLayer, 19))
			{
				IID_INIT(cellIid);
				cellIid.level = 1;
				sscanf(LowerLayer, "Cellular.Interface.%hhu", &cellIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF, &cellIid, &cellObj);
				ZOS_STRCPY_M(WanType, "CELL");
				break;
			}
#endif
			else if(!strcmp("", LowerLayer)){
				if(json_object_get_boolean(json_object_object_get(ethlink, "X_ZYXEL_OrigEthWAN")) == false)
				{
					ret = ZCFG_REQUEST_REJECT;
					goto out;
				}
			}
			else{
				ret = ZCFG_REQUEST_REJECT;
				goto out;
			}
		}
	}

	//release	
out:
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	if(atmObj) json_object_put(atmObj);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	if(ptmObj) json_object_put(ptmObj);
#endif
	if(ethObj) json_object_put(ethObj);
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	if(optObj) json_object_put(optObj);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	if(cellObj) json_object_put(cellObj);
#endif
	if(ethlink) json_object_put(ethlink);
	if(vlanterm) json_object_put(vlanterm);
	if(pppObj) json_object_put(pppObj);
	if(ipObj) json_object_put(ipObj);
	
	return ret;
}

zcfgRet_t DisableSameWanTypeIgmpMld(char *newInterface, const char *oldInterface, char *Ifname, int IgmpMld)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char buff[512] = {0}, *intf = NULL, *tmp_ptr = NULL;
	char ipWanType[16] = {0}, otherWanType[16] = {0};	

	memset(ipWanType, 0, sizeof(ipWanType));

	ZOS_STRNCPY(ipWanType, Type, sizeof(ipWanType));	
	
	//disable other same wan type igmp and mld proxy
	ZOS_STRCPY_M(buff, oldInterface);
	intf = strtok_r(buff, ",", &tmp_ptr);
	while(intf != NULL)
	{
		memset(otherWanType, 0, sizeof(otherWanType));
		IpIfaceWanTypeGet(intf, otherWanType);
			
		if(strcmp(ipWanType,otherWanType) == 0)
		{			
				genNewInterface(",", newInterface, oldInterface, intf, false);			
	
			return ret;
		}
		
		intf = strtok_r(NULL, ",", &tmp_ptr);
	}

	ZOS_STRCPY_M(newInterface, oldInterface);
	return ret;
}

zcfgRet_t editDefaultGwSetting(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//objIndex_t iid;
	//struct json_object *obj = NULL;
	const char *Interface = NULL;
	char newInterface[512] = {0};
	struct json_object *pramJobj = NULL;
	bool v4GwEnable = false, v6GwEnable = false;
	bool CurrV4GwEnable = false, CurrV6GwEnable = false;
	bool needSet = false;
	
	if(routerObj == NULL){ //add case
		IID_INIT(routerIid);
		routerIid.level = 1;
		routerIid.idx[0] = 1;
		zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ROUTING_ROUTER, &routerIid, &routerObj);
	}
		
	Interface = json_object_get_string(json_object_object_get(routerObj, "X_ZYXEL_ActiveDefaultGateway"));
	if(Interface && strstr(Interface, ipIfacePath))
		CurrV4GwEnable = true;

	Interface = json_object_get_string(json_object_object_get(routerObj, "X_ZYXEL_ActiveV6DefaultGateway"));
	if(Interface && strstr(Interface, ipIfacePath))
		CurrV6GwEnable = true;

	pramJobj = json_object_object_get(Jobj, "sysGwEnable");
	if(pramJobj)
		v4GwEnable = json_object_get_boolean(pramJobj);
	else
		v4GwEnable = CurrV4GwEnable;

	pramJobj = json_object_object_get(Jobj, "sysGw6Enable");
	if(pramJobj)
		v6GwEnable = json_object_get_boolean(pramJobj);
	else
		v6GwEnable = CurrV6GwEnable;

#ifdef ZYXEL_SYNC_CCWAN_TO_ALL_DEFGW
	if(v4GwEnable)
		json_object_object_add(ipIfaceObj, "X_ZYXEL_DefaultGatewayIface", json_object_new_boolean(true));
	else
		json_object_object_add(ipIfaceObj, "X_ZYXEL_DefaultGatewayIface", json_object_new_boolean(false));
#endif

	dbg_printf("CurrV4GwEnable=%d v4GwEnable=%d\n", CurrV4GwEnable, v4GwEnable);
	dbg_printf("CurrV6GwEnable=%d v6GwEnable=%d\n", CurrV6GwEnable, v6GwEnable);

	Interface = json_object_get_string(json_object_object_get(routerObj, "X_ZYXEL_ActiveDefaultGateway"));
	Interface = Interface ? Interface : "";

	if(CurrV4GwEnable && !v4GwEnable){ ////enable to disable
		if(genNewInterface(",", newInterface, Interface, ipIfacePath, false)){
			json_object_object_add(routerObj, "X_ZYXEL_ActiveDefaultGateway", json_object_new_string(newInterface));
			needSet = true;
		}
	}
	else if(!CurrV4GwEnable && v4GwEnable){ //disable to enable
		if(genNewInterface(",", newInterface, Interface, ipIfacePath, true)){
			json_object_object_add(routerObj, "X_ZYXEL_ActiveDefaultGateway", json_object_new_string(newInterface));
			needSet = true;
		}
	}

	memset(newInterface, 0, sizeof(newInterface));

	Interface = json_object_get_string(json_object_object_get(routerObj, "X_ZYXEL_ActiveV6DefaultGateway"));
	Interface = Interface ? Interface : "";

	if(CurrV6GwEnable && !v6GwEnable){ ////enable to disable
		if(genNewInterface(",", newInterface, Interface, ipIfacePath, false)){
			json_object_object_add(routerObj, "X_ZYXEL_ActiveV6DefaultGateway", json_object_new_string(newInterface));
			needSet = true;
		}
	}
	else if(!CurrV6GwEnable && v6GwEnable){ //disable to enable
		if(genNewInterface(",", newInterface, Interface, ipIfacePath, true)){
			json_object_object_add(routerObj, "X_ZYXEL_ActiveV6DefaultGateway", json_object_new_string(newInterface));
			needSet = true;
		}
	}

	if(!needSet){
		if(routerObj) {zcfgFeJsonObjFree(routerObj);}
	}

	return ret;
}

#ifdef ZYXEL_TAAAB_CUSTOMIZATION
//Sync PPP Username & Password to all PPP Interface
zcfgRet_t syncPppAccountSetting(struct json_object *Jobj, struct json_object *ipIfaceObj){
   zcfgRet_t ret = ZCFG_SUCCESS;
   objIndex_t pppIfaceIid;
   struct json_object *pppIfaceObj = NULL;
   int pppindex = 0;
   const char *LowerLayer;

	  LowerLayer = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
	  sscanf(LowerLayer, "PPP.Interface.%d", &pppindex);
	  IID_INIT(pppIfaceIid);
	  while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIfaceObj) == ZCFG_SUCCESS){
		  if(pppindex == pppIfaceIid.idx[0]){
			 zcfgFeJsonObjFree(pppIfaceObj);
			 continue;
		  }
		  
		  replaceParam(pppIfaceObj, "Username", Jobj, "pppUsername");
		  replaceParam(pppIfaceObj, "Password", Jobj, "pppPassword");

		  if((ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_PPP_IFACE, &pppIfaceIid, pppIfaceObj, NULL)) != ZCFG_SUCCESS) {
			  zcfgLog(ZCFG_LOG_ERR, "%s : Set PPP.Interface.%d fail\n", __FUNCTION__, pppIfaceIid.idx[0]);
		  }
		  zcfgFeJsonObjFree(pppIfaceObj);
	  }

	  return ret;
}
#endif

zcfgRet_t editPppIfaceObject(struct json_object *Jobj, struct json_object *ipIfaceObj, objIndex_t *ipIfaceIid){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *pramJobj = NULL;
	bool pppStatic = false;
	bool isNewPPP = false;
	int idleDisconnectTime = 0;
#ifdef IAAAA_CUSTOMIZATION //Adam
	int mark;
#endif
#ifdef ABUU_CUSTOMIZATION
	int connectionid = 0, ipr2_mark = 0;
#endif
	pramJobj = json_object_object_get(Jobj, "ipStatic");
	if(pramJobj)
		pppStatic = json_object_get_boolean(pramJobj);
	else
		pppStatic = CurrPppStatic;

	if(isAdd || pppIfaceObj == NULL){ //add case
		isNewPPP = true;
		IID_INIT(pppIfaceIid);
		if((ret = zcfgFeObjJsonAdd(RDM_OID_PPP_IFACE, &pppIfaceIid, NULL)) != ZCFG_SUCCESS)
			return ret;
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIfaceObj)) != ZCFG_SUCCESS)
			return ret;
	}
	
	//set ppp obj defalult values
	//json_object_object_add(pppIfaceObj, "Enable", json_object_new_boolean(true));
#ifdef IAAAA_CUSTOMIZATION //Adam
	mark = json_object_get_int(json_object_object_get(ipIfaceObj, "X_ZYXEL_IPR2_MARKING"));
	//printf("%s(%d)  mark value %d\n",__FUNCTION__, __LINE__, mark);
#endif
#ifdef ABUU_CUSTOMIZATION
	connectionid = json_object_get_int(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionId"));
	ipr2_mark = json_object_get_int(json_object_object_get(ipIfaceObj, "X_ZYXEL_IPR2_MARKING"));
#endif

#ifdef ABUU_CUSTOMIZATION
	json_object_object_add(pppIfaceObj, "X_ZYXEL_ConnectionId", json_object_new_int(connectionid));
#else
	json_object_object_add(pppIfaceObj, "X_ZYXEL_ConnectionId", json_object_new_int(ipIfaceIid->idx[0] - 1));
#endif

#ifdef IAAAA_CUSTOMIZATION //Adam
	json_object_object_add(pppIfaceObj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(mark));
#elif defined (ABUU_CUSTOMIZATION)
	json_object_object_add(pppIfaceObj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(ipr2_mark));
#else
	json_object_object_add(pppIfaceObj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(ipIfaceIid->idx[0] - 1));
#endif

	if(isNewPPP && json_object_object_get(Jobj, "MaxMTUSize") == NULL){
#ifdef SUPPORT_PPPOE_MTU_1500
		json_object_object_add(pppIfaceObj, "MaxMRUSize", json_object_new_int(1500));
#else
		json_object_object_add(pppIfaceObj, "MaxMRUSize", json_object_new_int(1492));
#endif	
	}

	json_object_object_add(pppIfaceObj, "LowerLayers", json_object_new_string(currLowerLayers));

	// need to set ipIface object first
	sprintf(currLowerLayers, "PPP.Interface.%u", pppIfaceIid.idx[0]);
	const char *lowerLayers = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
	if(lowerLayers && strcmp(lowerLayers, currLowerLayers))
		json_object_object_add(ipIfaceObj, "X_ZYXEL_AccessDescription", json_object_new_string("SetIpIfaceStackModified"));

	json_object_object_add(ipIfaceObj, "LowerLayers", json_object_new_string(currLowerLayers));
	
	sprintf(currLowerLayers, "IP.Interface.%u", ipIfaceIid->idx[0]);
	
	//set ppp parameter
	if(!replaceParam(pppIfaceObj, "Enable", Jobj, "Enable"))
		json_object_object_add(pppIfaceObj, "Enable", json_object_new_boolean(CurrEnable));
		
	replaceParam(pppIfaceObj, "Username", Jobj, "pppUsername");
	replaceParam(pppIfaceObj, "Password", Jobj, "pppPassword");
	replaceParam(pppIfaceObj, "MaxMRUSize", Jobj, "MaxMTUSize");
#ifdef ZYXEL_PPPoE_SERVICE_OPTION
	replaceParam(pppIfaceObj, "X_ZYXEL_ServiceName", Jobj, "pppServicename");
#endif
	replaceParam(pppIfaceObj, "ConnectionTrigger", Jobj, "ConnectionTrigger");
	idleDisconnectTime = json_object_get_int(json_object_object_get(Jobj, "IdleDisconnectTime"));
	if(json_object_object_get(Jobj, "IdleDisconnectTime")){
		json_object_object_add(pppIfaceObj, "IdleDisconnectTime", json_object_new_int(60*idleDisconnectTime));
	}
	if(json_object_get_boolean(json_object_object_get(Jobj, "pppoePassThrough")) == true)
		json_object_object_add(pppIfaceObj, "X_ZYXEL_ConnectionType", json_object_new_string("PPPoE_Bridged"));
	else
		json_object_object_add(pppIfaceObj, "X_ZYXEL_ConnectionType", json_object_new_string("IP_Routed"));
	
	if(pppStatic)
		replaceParam(pppIfaceObj, "X_ZYXEL_LocalIPAddress", Jobj, "IPAddress");
	else
		json_object_object_add(pppIfaceObj, "X_ZYXEL_LocalIPAddress", json_object_new_string("0.0.0.0"));

	if(!strcmp(ipMode, "IPv4")){
		json_object_object_add(pppIfaceObj, "IPv6CPEnable", json_object_new_boolean(false));
		json_object_object_add(pppIfaceObj, "IPCPEnable", json_object_new_boolean(true));
	}
	else if(!strcmp(ipMode, "IPv6")){
		json_object_object_add(pppIfaceObj, "IPv6CPEnable", json_object_new_boolean(true));
		json_object_object_add(pppIfaceObj, "IPCPEnable", json_object_new_boolean(false));
	}else{//Dual stack
		json_object_object_add(pppIfaceObj, "IPv6CPEnable", json_object_new_boolean(true));
		json_object_object_add(pppIfaceObj, "IPCPEnable", json_object_new_boolean(true));
	}

#ifdef ZYXEL_TAAAB_CUSTOMIZATION
   syncPppAccountSetting(Jobj,ipIfaceObj);
#endif
	return ret;
}

zcfgRet_t editMAPSetting(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char wanIntfPath[MAX_INTF_PATH_LENGTH] = {0};
	char ipv6PrefixPath[MAX_INTF_PATH_LENGTH] = {0};
	bool found = false;

	ZLOG_INFO("%s: Enter\n", __FUNCTION__);

	if(mapObj == NULL){
		ZLOG_INFO("%s: init mapObj\n", __FUNCTION__);
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MAP, &mapIid, &mapObj)) != ZCFG_SUCCESS)
			return ret;
	}
	if(domainObj == NULL){
		ZLOG_INFO("%s: init domainObj\n", __FUNCTION__);

		/* find domain object */
		snprintf(wanIntfPath, MAX_INTF_PATH_LENGTH, "Device.%s",  ipIfacePath);
		IID_INIT(domainIid);
		while(!found && zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MAP_DOMAIN, &domainIid, &domainObj) == ZCFG_SUCCESS)
		{
			// const char *DomainWANIntfPath = json_object_get_string(json_object_object_get(domainObj, "WANInterface"));
			// ZLOG_DEBUG("%s : domain idx[0]=%d DWANIntfPath=%s WANIntfPath=%s\n", __FUNCTION__, domainIid.idx[0], DomainWANIntfPath, wanIntfPath);

			if(strcmp(wanIntfPath, json_object_get_string(json_object_object_get(domainObj, "WANInterface"))) == 0){
				found = true;
				break ;
			}
			zcfgFeJsonObjFree(domainObj);
		}
		/* If there is no domain object can be found, add a new one */
		if(!found){
			ZLOG_INFO("%s: domainObj\n", __FUNCTION__);
			IID_INIT(domainIid);
			if((ret = zcfgFeObjJsonAdd(RDM_OID_MAP_DOMAIN, &domainIid, NULL)) != ZCFG_SUCCESS)
				ZLOG_ERROR("%s: domainObj zcfgFeObjJsonAdd fail, errno=%d\n", __FUNCTION__, ret);
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MAP_DOMAIN, &domainIid, &domainObj)) != ZCFG_SUCCESS)
				ZLOG_ERROR("%s: domainObj zcfgFeObjJsonGetWithoutUpdate fail, errno=%d\n", __FUNCTION__, ret);
			ZLOG_DEBUG("%s : domain idx[0]=%d \n", __FUNCTION__, domainIid.idx[0]);
		}
	}
	if(ruleObj == NULL){
		ZLOG_INFO("%s: init ruleObj\n", __FUNCTION__);
		/* get first rule in domain object */
		ZLOG_DEBUG("%s : rule1 idx[0]=%d idx[1]=%d domain idx[0]=%d\n", __FUNCTION__, ruleIid.idx[0], ruleIid.idx[1], domainIid.idx[0]);
		IID_INIT(ruleIid);
		if( zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_MAP_DOMAIN_RULE, &domainIid, &ruleIid, &ruleObj) != ZCFG_SUCCESS){
			/* if first rule object is empty, add a new one */
			IID_INIT(ruleIid);
			ruleIid.level = 1;
			ruleIid.idx[0] = domainIid.idx[0];
			if((ret = zcfgFeObjJsonAdd(RDM_OID_MAP_DOMAIN_RULE, &ruleIid, NULL)) != ZCFG_SUCCESS)
				ZLOG_ERROR("%s: ruleObj zcfgFeObjJsonAdd fail, errno=%d\n", __FUNCTION__, ret);
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MAP_DOMAIN_RULE, &ruleIid, &ruleObj)) != ZCFG_SUCCESS)
				ZLOG_ERROR("%s: ruleObj zcfgFeObjJsonGetWithoutUpdate fail, errno=%d\n", __FUNCTION__, ret);
			ZLOG_DEBUG("%s : rule2 idx[0]=%d idx[1]=%d domain idx[0]=%d\n", __FUNCTION__, ruleIid.idx[0], ruleIid.idx[1], domainIid.idx[0]);
		}
	}

#if 0
	/* check MAP enable */
	if(json_object_get_boolean(json_object_object_get(obj, "mapEnable")) == false)
		return ret;
#endif
	/* MAP enable */
	if(!strcmp(ipMode, "IPv6"))
	{
		replaceParam(mapObj, "Enable", Jobj, "mapEnable");
		replaceParam(domainObj, "Enable", Jobj, "mapEnable");
		replaceParam(ruleObj, "Enable", Jobj, "mapEnable");
	} else
	{
		// Set MAP to disable when there is other ipMode to avoid core trace
		json_object_object_add(mapObj, "Enable", json_object_new_boolean(false));
		json_object_object_add(domainObj, "Enable", json_object_new_boolean(false));
		json_object_object_add(ruleObj, "Enable", json_object_new_boolean(false));
	}

	/* MAP Domain object */
	replaceParam(domainObj, "TransportMode", Jobj, "TransportMode");

	// Set WAN Interface when MAP doamin enable on new WAN
	if(!found)
	{
		ZLOG_INFO("%s : Set WAN Interface\n", __FUNCTION__);
		snprintf(ipv6PrefixPath, MAX_INTF_PATH_LENGTH, "Device.%s.IPv6Prefix.1", ipIfacePath); //temporary set to IPv6Prefix.1
		json_object_object_add(domainObj, "WANInterface", json_object_new_string(wanIntfPath));
		json_object_object_add(domainObj, "IPv6Prefix", json_object_new_string(ipv6PrefixPath));
	}
	replaceParam(domainObj, "X_ZYXEL_Softwire46Enable", Jobj, "Softwire46Enable");
	ZLOG_DEBUG("%s : Domain %s %s S46=%d\n", __FUNCTION__
				, json_object_get_string(json_object_object_get(domainObj, "WANInterface"))
				, json_object_get_string(json_object_object_get(domainObj, "IPv6Prefix"))
				, json_object_get_boolean(json_object_object_get(domainObj, "X_ZYXEL_Softwire46Enable")) );
	// Set MAP rule by DHCP S46 option and don't need duplicate setting
	if(json_object_get_boolean(json_object_object_get(domainObj, "X_ZYXEL_Softwire46Enable")) == true)
		return ret;

	replaceParam(domainObj, "BRIPv6Prefix", Jobj, "BRIPv6Prefix");
	replaceParam(domainObj, "PSIDOffset", Jobj, "PSIDOffset");
	replaceParam(domainObj, "PSIDLength", Jobj, "PSIDLength");
	replaceParam(domainObj, "PSID", Jobj, "PSID");

	ZLOG_DEBUG("%s : Domain %d %s %s PSIDOffset=%lld PSIDLength=%lld PSID=%lld S46=%d\n", __FUNCTION__
				, json_object_get_boolean(json_object_object_get(domainObj, "Enable"))
				, json_object_get_string(json_object_object_get(domainObj, "TransportMode"))
				, json_object_get_string(json_object_object_get(domainObj, "BRIPv6Prefix"))
				, json_object_get_int(json_object_object_get(domainObj, "PSIDOffset"))
				, json_object_get_int(json_object_object_get(domainObj, "PSIDLength"))
				, json_object_get_int(json_object_object_get(domainObj, "PSID"))
				, json_object_get_boolean(json_object_object_get(domainObj, "X_ZYXEL_Softwire46Enable")) );

	/* MAP Domain Rule object */
	replaceParam(ruleObj, "IPv6Prefix", Jobj, "IPv6Prefix");
	replaceParam(ruleObj, "IPv4Prefix", Jobj, "IPv4Prefix");
	replaceParam(ruleObj, "EABitsLength", Jobj, "EABitsLength");
	replaceParam(ruleObj, "PSIDOffset", Jobj, "PSIDOffset");
	replaceParam(ruleObj, "PSIDLength", Jobj, "PSIDLength");
	replaceParam(ruleObj, "PSID", Jobj, "PSID");

	ZLOG_DEBUG("%s : Rule %d %s %s %lld\n", __FUNCTION__
				, json_object_get_boolean(json_object_object_get(ruleObj, "Enable"))
				, json_object_get_string(json_object_object_get(ruleObj, "IPv6Prefix"))
				, json_object_get_string(json_object_object_get(ruleObj, "IPv4Prefix"))
				, json_object_get_int(json_object_object_get(ruleObj, "EABitsLength")) );

	ZLOG_INFO("%s : End\n", __FUNCTION__);
	return ret;
}

zcfgRet_t addLayer3Objects(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char ipIfacePath_tmp[32] = {0};
	//objIndex_t ipIfaceIid;
	//struct json_object *ipIfaceObj = NULL;
	//const char *Name, *Mode, *Encapsulation, *ipMode;
	//objIndex_t dnsIid;
	//struct json_object *dnsObj = NULL;
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
   char copy_name[33] = {0};
   char new_dupName[33] = {0};
   char *tmp_string = NULL;
#endif

	dbg_printf("line:%d\n", __LINE__);

	IID_INIT(ipIfaceIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE, &ipIfaceIid, NULL)) != ZCFG_SUCCESS)
		return ret;
	
	sprintf(ipIfacePath_tmp, "IP.Interface.%u", ipIfaceIid.idx[0]);
	zcfgFeDalSetReturnValue(Jobj, json_type_string, "ipIfacePath", ipIfacePath_tmp);
	
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj)) != ZCFG_SUCCESS)
		return ret;

	dbg_printf("line:%d\n", __LINE__);

	//Name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	//Mode = json_object_get_string(json_object_object_get(Jobj, "Mode"));

	//set defalult values
	//json_object_object_add(ipIfaceObj, "Enable", json_object_new_boolean(true));//set to false here, trigger to true after WAN add comlete
	replaceParam(ipIfaceObj, "Enable", Jobj, "Enable");
	json_object_object_add(ipIfaceObj, "X_ZYXEL_ConnectionId", json_object_new_int(ipIfaceIid.idx[0] - 1));
	json_object_object_add(ipIfaceObj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(ipIfaceIid.idx[0] - 1));

	//set Name and Mode
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
#ifdef TAAAB_HGW
    if(HGWStyleName){
			strncpy(copy_name, HGWStyleName, sizeof(copy_name));
			strncat(copy_name,"_WAN", sizeof(copy_name));
		    json_object_object_add(ipIfaceObj, "X_TT_SrvName", json_object_new_string(copy_name));
    }
	else{
		tmp_string = strrchr(Name,'_');
		if(  tmp_string != 0)
		{
			if( (strcmp(tmp_string,"_ADSL") == 0) || (strcmp(tmp_string,"_VDSL") == 0))
				strncpy(new_dupName, Name, (strlen(Name)-5 ));
			else if( (strcmp(tmp_string,"_ETHWAN") == 0))
				strncpy(new_dupName, Name, (strlen(Name)-7 ));
			else
				strncpy(new_dupName, Name, strlen(Name));
		}
		else
			strncpy(new_dupName, Name, strlen(Name));

       	if(!strcmp(Type, "ATM") || !strcmp(Type, "PTM") ){		
			strncpy(copy_name, new_dupName, sizeof(copy_name));
			strncat(copy_name,"_DSL", sizeof(copy_name));
		    json_object_object_add(ipIfaceObj, "X_TT_SrvName", json_object_new_string(copy_name));
	    }
	    else if(!strcmp(Type, "ETH")){
			strncpy(copy_name, new_dupName, sizeof(copy_name));
			strncat(copy_name,"_WAN", sizeof(copy_name));
		    json_object_object_add(ipIfaceObj, "X_TT_SrvName", json_object_new_string(copy_name));
	    }
	}
#else
    if(TAAABSrvName){
       	if(!strcmp(Type, "ATM") || !strcmp(Type, "PTM") ){
			strncpy(copy_name, TAAABSrvName, sizeof(copy_name));
			strncat(copy_name,"_DSL", sizeof(copy_name));
		    json_object_object_add(ipIfaceObj, "X_TT_SrvName", json_object_new_string(copy_name));
	    }
	    else if(!strcmp(Type, "ETH")){
			strncpy(copy_name, TAAABSrvName, sizeof(copy_name));
			strncat(copy_name,"_WAN", sizeof(copy_name));
		    json_object_object_add(ipIfaceObj, "X_TT_SrvName", json_object_new_string(copy_name));
	    }
    }
	else{
		tmp_string = strrchr(Name,'_');
		if(  tmp_string != 0)
		{
			if( (strcmp(tmp_string,"_ADSL") == 0) || (strcmp(tmp_string,"_VDSL") == 0))
				strncpy(new_dupName, Name, (strlen(Name)-5 ));
			else if( (strcmp(tmp_string,"_ETHWAN") == 0))
				strncpy(new_dupName, Name, (strlen(Name)-7 ));
			else
				strncpy(new_dupName, Name, strlen(Name));
		}
		else
			strncpy(new_dupName, Name, strlen(Name));

       	if(!strcmp(Type, "ATM") || !strcmp(Type, "PTM") ){		
			strncpy(copy_name, new_dupName, sizeof(copy_name));
			strncat(copy_name,"_DSL", sizeof(copy_name));
		    json_object_object_add(ipIfaceObj, "X_TT_SrvName", json_object_new_string(copy_name));
	    }
	    else if(!strcmp(Type, "ETH")){
			strncpy(copy_name, new_dupName, sizeof(copy_name));
			strncat(copy_name,"_WAN", sizeof(copy_name));
		    json_object_object_add(ipIfaceObj, "X_TT_SrvName", json_object_new_string(copy_name));
	    }
	}
#endif
#endif
	json_object_object_add(ipIfaceObj, "X_ZYXEL_SrvName", json_object_new_string(Name));
	json_object_object_add(ipIfaceObj, "X_ZYXEL_ConnectionType", json_object_new_string(Mode));

	dbg_printf("line:%d\n", __LINE__);

	if(!strcmp(Mode, "IP_Bridged")){
		json_object_object_add(ipIfaceObj, "LowerLayers", json_object_new_string(currLowerLayers));
		//ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ipIfaceIid, ipIfaceObj, NULL);
		//json_object_put(ipIfaceObj);

		if(DSCPEnable)
			replaceParam(ipIfaceObj, "X_ZYXEL_DSCPMark", Jobj, "DSCP");
		else
			json_object_object_add(ipIfaceObj, "X_ZYXEL_DSCPMark", json_object_new_int(-1));

		replaceParam(ipIfaceObj, "MaxMTUSize", Jobj, "MaxMTUSize");

		return ret;
	}

	replaceParam(ipIfaceObj, "X_ZYXEL_ConcurrentWan", Jobj, "ConcurrentWan");
		
	dbg_printf("line:%d\n", __LINE__);
	sprintf(ipIfacePath, "IP.Interface.%u", ipIfaceIid.idx[0]);

	//routting mode, need to set Nat and Static DNS before set IP and PPP.
	editNatSetting(Jobj);
	editDnsSetting(Jobj);
	
	dbg_printf("line:%d\n", __LINE__);
	if(!strcmp(ipMode, "IPv4")){
		json_object_object_add(ipIfaceObj, "IPv4Enable", json_object_new_boolean(true));
		replaceParam(ipIfaceObj, "X_ZYXEL_Enable_6RD", Jobj, "Enable_6RD");
		replaceParam(ipIfaceObj, "X_ZYXEL_6RD_Type", Jobj, "v6RD_Type");
		replaceParam(ipIfaceObj, "X_ZYXEL_SPIPv6Prefix", Jobj, "SPIPv6Prefix");
		replaceParam(ipIfaceObj, "X_ZYXEL_IPv4MaskLength", Jobj, "IPv4MaskLength");
		replaceParam(ipIfaceObj, "X_ZYXEL_BorderRelayIPv4Addresses", Jobj, "BorderRelayIPv4Addresses");

		if(DSCPEnable)
			replaceParam(ipIfaceObj, "X_ZYXEL_DSCPMark", Jobj, "DSCP");
		else
			json_object_object_add(ipIfaceObj, "X_ZYXEL_DSCPMark", json_object_new_int(-1));
	}
	if(!strcmp(ipMode, "IPv6")){
		json_object_object_add(ipIfaceObj, "IPv6Enable", json_object_new_boolean(true));
		replaceParam(ipIfaceObj, "X_ZYXEL_Enable_DSLite", Jobj, "DSLiteEnable");
		replaceParam(ipIfaceObj, "X_ZYXEL_DSLite_Type", Jobj, "DSLiteType");
		replaceParam(ipIfaceObj, "X_ZYXEL_DSLiteRelayIPv6Addresses", Jobj, "DSLiteRelayIPv6Addresses");
         /* __ZyXEL__, Albert, 20180209,Support IPv6 option IA_NA and IA_PD  */
        replaceParam(ipIfaceObj, "X_ZYXEL_AdvManagedFlag_Manual", Jobj, "IPv6Manual_IANA");
        replaceParam(ipIfaceObj, "X_ZYXEL_AdvOtherConfigFlag_Manual", Jobj, "IPv6Manual_IAPD");       
	}
	else if(!strcmp(ipMode, "DualStack")){
		json_object_object_add(ipIfaceObj, "IPv4Enable", json_object_new_boolean(true));
		json_object_object_add(ipIfaceObj, "IPv6Enable", json_object_new_boolean(true));
        /* __ZyXEL__, Albert, 20180209,Support IPv6 option IA_NA and IA_PD  */
        replaceParam(ipIfaceObj, "X_ZYXEL_AdvManagedFlag_Manual", Jobj, "IPv6Manual_IANA");
        replaceParam(ipIfaceObj, "X_ZYXEL_AdvOtherConfigFlag_Manual", Jobj, "IPv6Manual_IAPD");

		if(DSCPEnable)
			replaceParam(ipIfaceObj, "X_ZYXEL_DSCPMark", Jobj, "DSCP");
		else
			json_object_object_add(ipIfaceObj, "X_ZYXEL_DSCPMark", json_object_new_int(-1));
	}
#ifdef ABUE_CUSTOMIZATION
	else if(!strcmp(ipMode, "AutoConfig")){
		json_object_object_add(ipIfaceObj, "IPv4Enable", json_object_new_boolean(true));
		json_object_object_add(ipIfaceObj, "IPv6Enable", json_object_new_boolean(true));
		replaceParam(ipIfaceObj, "X_ZYXEL_Enable_DSLite", Jobj, "DSLiteEnable");
		replaceParam(ipIfaceObj, "X_ZYXEL_DSLite_Type", Jobj, "DSLiteType");
		replaceParam(ipIfaceObj, "X_ZYXEL_DSLiteRelayIPv6Addresses", Jobj, "DSLiteRelayIPv6Addresses");
		/* __ZyXEL__, Albert, 20180209,Support IPv6 option IA_NA and IA_PD	*/
		replaceParam(ipIfaceObj, "X_ZYXEL_AdvManagedFlag_Manual", Jobj, "IPv6Manual_IANA");
		replaceParam(ipIfaceObj, "X_ZYXEL_AdvOtherConfigFlag_Manual", Jobj, "IPv6Manual_IAPD");
	}
#endif
	json_object_object_add(ipIfaceObj, "X_ZYXEL_AccessDescription", json_object_new_string("AddSetIpIface"));
#ifdef ZYXEL_TAAAG_FIREWALL_WANINTERFACE
	replaceParam(ipIfaceObj, "X_ZYXEL_Enable_Firewall", Jobj, "FirewallEnable");
#endif
	if(!strcmp(Encapsulation, "PPPoE") || !strcmp(Encapsulation, "PPPoA") || !strcmp(Encapsulation, TR181_PPtP2PPPOA)){
		ret = editPppIfaceObject(Jobj, ipIfaceObj, &ipIfaceIid);
	}
	else{
		replaceParam(ipIfaceObj, "MaxMTUSize", Jobj, "MaxMTUSize");
		json_object_object_add(ipIfaceObj, "LowerLayers", json_object_new_string(currLowerLayers));
		//ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ipIfaceIid, ipIfaceObj, NULL);
		dbg_printf("line:%d\n", __LINE__);
		sprintf(currLowerLayers, "IP.Interface.%u", ipIfaceIid.idx[0]);
		addStaticAddrSetting(Jobj);
		addDhcpSetting(Jobj);
    	dbg_printf("line=%d\n", __LINE__);
	}
	//json_object_put(ipIfaceObj);

	editIgmpMldSetting(Jobj);
	editDefaultGwSetting(Jobj);
#ifdef ZYXEL_IPV6_MAP
	editMAPSetting(Jobj);
#endif
	
	IID_INIT(dnsIid);
	zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DNS, &dnsIid, &dnsObj);
	//ret = zcfgFeObjJsonSet(RDM_OID_DNS, &dnsIid, dnsObj, NULL);
	//json_object_put(dnsObj);

	dbg_printf("line=%d\n", __LINE__);
	
	return ret;
}

zcfgRet_t editLayer3Objects(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;

	dbg_printf("line:%d\n", __LINE__);
#ifdef IAAAA_CUSTOMIZATION //Adam
	int mark;
#endif
#ifdef ABUU_CUSTOMIZATION
	int connectionid = 0, ipr2_mark = 0;
#endif
	if(isAdd){
		IID_INIT(ipIfaceIid);
		if((ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE, &ipIfaceIid, NULL)) != ZCFG_SUCCESS)
			return ret;
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj)) != ZCFG_SUCCESS)
			return ret;

		sprintf(ipIfacePath, "IP.Interface.%u", ipIfaceIid.idx[0]);

		IID_INIT(dnsIid);
		zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DNS, &dnsIid, &dnsObj);
	}

	dbg_printf("line:%d\n", __LINE__);
#ifdef IAAAA_CUSTOMIZATION //Adam
	mark = json_object_get_int(json_object_object_get(ipIfaceObj, "X_ZYXEL_IPR2_MARKING"));
	//printf("%s(%d)  mark value %d\n",__FUNCTION__, __LINE__, mark);
#endif
#ifdef ABUU_CUSTOMIZATION
	connectionid = json_object_get_int(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionId"));
	ipr2_mark = json_object_get_int(json_object_object_get(ipIfaceObj, "X_ZYXEL_IPR2_MARKING"));
#endif
	//json_object_object_add(ipIfaceObj, "Enable", json_object_new_boolean(true));
	if(!replaceParam(ipIfaceObj, "Enable", Jobj, "Enable"))
		json_object_object_add(ipIfaceObj, "Enable", json_object_new_boolean(CurrEnable));

#ifdef ABUU_CUSTOMIZATION
	json_object_object_add(ipIfaceObj, "X_ZYXEL_ConnectionId", json_object_new_int(connectionid));
#else
	json_object_object_add(ipIfaceObj, "X_ZYXEL_ConnectionId", json_object_new_int(ipIfaceIid.idx[0] - 1));
#endif

#ifdef IAAAA_CUSTOMIZATION //Adam
	json_object_object_add(ipIfaceObj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(mark));
#elif defined (ABUU_CUSTOMIZATION)
	json_object_object_add(ipIfaceObj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(ipr2_mark));
#else
	json_object_object_add(ipIfaceObj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(ipIfaceIid.idx[0] - 1));
#endif
	
	json_object_object_add(ipIfaceObj, "X_ZYXEL_SrvName", json_object_new_string(Name));
	json_object_object_add(ipIfaceObj, "X_ZYXEL_ConnectionType", json_object_new_string(Mode));

	dbg_printf("line:%d\n", __LINE__);

	if(!strcmp(Mode, "IP_Bridged")){
		json_object_object_add(ipIfaceObj, "LowerLayers", json_object_new_string(currLowerLayers));


		replaceParam(ipIfaceObj, "MaxMTUSize", Jobj, "MaxMTUSize");

		if(DSCPEnable)
			replaceParam(ipIfaceObj, "X_ZYXEL_DSCPMark", Jobj, "DSCP");
		else
			json_object_object_add(ipIfaceObj, "X_ZYXEL_DSCPMark", json_object_new_int(-1));

		return ret;
	}

	replaceParam(ipIfaceObj, "X_ZYXEL_ConcurrentWan", Jobj, "ConcurrentWan");
	
	dbg_printf("line:%d\n", __LINE__);
	if(!strcmp(ipMode, "IPv4")){
		json_object_object_add(ipIfaceObj, "IPv4Enable", json_object_new_boolean(true));
		json_object_object_add(ipIfaceObj, "IPv6Enable", json_object_new_boolean(false));
		replaceParam(ipIfaceObj, "X_ZYXEL_Enable_6RD", Jobj, "Enable_6RD");
		replaceParam(ipIfaceObj, "X_ZYXEL_6RD_Type", Jobj, "v6RD_Type");
		replaceParam(ipIfaceObj, "X_ZYXEL_SPIPv6Prefix", Jobj, "SPIPv6Prefix");
		replaceParam(ipIfaceObj, "X_ZYXEL_IPv4MaskLength", Jobj, "IPv4MaskLength");
		replaceParam(ipIfaceObj, "X_ZYXEL_BorderRelayIPv4Addresses", Jobj, "BorderRelayIPv4Addresses");
		json_object_object_add(ipIfaceObj, "X_ZYXEL_Enable_DSLite", json_object_new_boolean(false));

		if(DSCPEnable)
			replaceParam(ipIfaceObj, "X_ZYXEL_DSCPMark", Jobj, "DSCP");
		else
			json_object_object_add(ipIfaceObj, "X_ZYXEL_DSCPMark", json_object_new_int(-1));
	}
	if(!strcmp(ipMode, "IPv6")){
		json_object_object_add(ipIfaceObj, "IPv4Enable", json_object_new_boolean(false));
		json_object_object_add(ipIfaceObj, "IPv6Enable", json_object_new_boolean(true));
		replaceParam(ipIfaceObj, "X_ZYXEL_Enable_DSLite", Jobj, "DSLiteEnable");
		replaceParam(ipIfaceObj, "X_ZYXEL_DSLite_Type", Jobj, "DSLiteType");
		replaceParam(ipIfaceObj, "X_ZYXEL_DSLiteRelayIPv6Addresses", Jobj, "DSLiteRelayIPv6Addresses");
		json_object_object_add(ipIfaceObj, "X_ZYXEL_Enable_6RD", json_object_new_boolean(false));
        /* __ZyXEL__, Albert, 20180209,Support IPv6 option IA_NA and IA_PD  */
        replaceParam(ipIfaceObj, "X_ZYXEL_AdvManagedFlag_Manual", Jobj, "IPv6Manual_IANA");
        replaceParam(ipIfaceObj, "X_ZYXEL_AdvOtherConfigFlag_Manual", Jobj, "IPv6Manual_IAPD");
#ifdef ZYXEL_PD_TYPE
		if(json_object_get_boolean(json_object_object_get(Jobj, "IPv6Manual_IAPD")))
		{
			if(ip6PrefixStatic)
			{
				json_object_object_add(ipIfaceObj, "X_ZYXEL_IPv6Origin", json_object_new_string(TR181_PREFIX_STATIC));
				replaceParam(ipIfaceObj, "X_ZYXEL_IPv6Prefix", Jobj, "ip6StaticPrefix");
			}else{
				json_object_object_add(ipIfaceObj, "X_ZYXEL_IPv6Origin", json_object_new_string(""));
				json_object_object_add(ipIfaceObj, "X_ZYXEL_IPv6Prefix", json_object_new_string(""));
			}
		}else{
				json_object_object_add(ipIfaceObj, "X_ZYXEL_IPv6Origin", json_object_new_string(""));
				json_object_object_add(ipIfaceObj, "X_ZYXEL_IPv6Prefix", json_object_new_string(""));
		}
#endif
	}
	else if(!strcmp(ipMode, "DualStack")){
		json_object_object_add(ipIfaceObj, "IPv4Enable", json_object_new_boolean(true));
		json_object_object_add(ipIfaceObj, "IPv6Enable", json_object_new_boolean(true));
		json_object_object_add(ipIfaceObj, "X_ZYXEL_Enable_6RD", json_object_new_boolean(false));
		json_object_object_add(ipIfaceObj, "X_ZYXEL_Enable_DSLite", json_object_new_boolean(false));
        /* __ZyXEL__, Albert, 20180209,Support IPv6 option IA_NA and IA_PD  */
        replaceParam(ipIfaceObj, "X_ZYXEL_AdvManagedFlag_Manual", Jobj, "IPv6Manual_IANA");
        replaceParam(ipIfaceObj, "X_ZYXEL_AdvOtherConfigFlag_Manual", Jobj, "IPv6Manual_IAPD");      
#ifdef ZYXEL_PD_TYPE
		if(json_object_get_boolean(json_object_object_get(Jobj, "IPv6Manual_IAPD")))
		{
			if(ip6PrefixStatic)
			{
				json_object_object_add(ipIfaceObj, "X_ZYXEL_IPv6Origin", json_object_new_string(TR181_PREFIX_STATIC));
				replaceParam(ipIfaceObj, "X_ZYXEL_IPv6Prefix", Jobj, "ip6StaticPrefix");
			}else{
				json_object_object_add(ipIfaceObj, "X_ZYXEL_IPv6Origin", json_object_new_string(""));
				json_object_object_add(ipIfaceObj, "X_ZYXEL_IPv6Prefix", json_object_new_string(""));
			}
		}else{
				json_object_object_add(ipIfaceObj, "X_ZYXEL_IPv6Origin", json_object_new_string(""));
				json_object_object_add(ipIfaceObj, "X_ZYXEL_IPv6Prefix", json_object_new_string(""));
		}    
#endif
		if(DSCPEnable)
			replaceParam(ipIfaceObj, "X_ZYXEL_DSCPMark", Jobj, "DSCP");
		else
			json_object_object_add(ipIfaceObj, "X_ZYXEL_DSCPMark", json_object_new_int(-1));
	}
#ifdef ABUE_CUSTOMIZATION	
	else if(!strcmp(ipMode, "AutoConfig")){
		json_object_object_add(ipIfaceObj, "IPv4Enable", json_object_new_boolean(true));
		json_object_object_add(ipIfaceObj, "IPv6Enable", json_object_new_boolean(true));
		json_object_object_add(ipIfaceObj, "X_ZYXEL_Enable_6RD", json_object_new_boolean(false));
		replaceParam(ipIfaceObj, "X_ZYXEL_Enable_DSLite", Jobj, "DSLiteEnable");
		replaceParam(ipIfaceObj, "X_ZYXEL_DSLite_Type", Jobj, "DSLiteType");
		replaceParam(ipIfaceObj, "X_ZYXEL_DSLiteRelayIPv6Addresses", Jobj, "DSLiteRelayIPv6Addresses");
		/* __ZyXEL__, Albert, 20180209,Support IPv6 option IA_NA and IA_PD	*/
		replaceParam(ipIfaceObj, "X_ZYXEL_AdvManagedFlag_Manual", Jobj, "IPv6Manual_IANA");
		replaceParam(ipIfaceObj, "X_ZYXEL_AdvOtherConfigFlag_Manual", Jobj, "IPv6Manual_IAPD");
	}
#endif
#ifdef ZYXEL_TAAAG_FIREWALL_WANINTERFACE
		replaceParam(ipIfaceObj, "X_ZYXEL_Enable_Firewall", Jobj, "FirewallEnable");
#endif

	if(!strcmp(Encapsulation, "PPPoE") || !strcmp(Encapsulation, "PPPoA") || !strcmp(Encapsulation, TR181_PPtP2PPPOA)){
		ret = editPppIfaceObject(Jobj, ipIfaceObj, &ipIfaceIid);
	}
	else{
		json_object_object_add(ipIfaceObj, "MaxMTUSize", json_object_new_int(1500));
		replaceParam(ipIfaceObj, "MaxMTUSize", Jobj, "MaxMTUSize");
		const char *lowerLayers = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
		if(lowerLayers && strcmp(lowerLayers, currLowerLayers))
			json_object_object_add(ipIfaceObj, "X_ZYXEL_AccessDescription", json_object_new_string("SetIpIfaceStackModified"));

		json_object_object_add(ipIfaceObj, "LowerLayers", json_object_new_string(currLowerLayers));
		sprintf(currLowerLayers, "IP.Interface.%u", ipIfaceIid.idx[0]);
	}

   	dbg_printf("line=%d\n", __LINE__);
	if(0==isPppQuick){
		editNatSetting(Jobj);
		editDnsSetting(Jobj);
		editStaticAddrSetting(Jobj);
		editDhcpSetting(Jobj);
		editIgmpMldSetting(Jobj);
		editDefaultGwSetting(Jobj);
	}
	isPppQuick = false; //runtime isPppQuick should be reset to inital value
#ifdef ZYXEL_IPV6_MAP
	editMAPSetting(Jobj);
#endif

	dbg_printf("line=%d\n", __LINE__);
	
	return ret;
}

zcfgRet_t setAllObjects(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	if(atmLinkQosObj) zcfgFeObjJsonSet(RDM_OID_ATM_LINK_QOS, &atmLinkQosIid, atmLinkQosObj, NULL);
	if(atmLinkObj)zcfgFeObjJsonSet(RDM_OID_ATM_LINK, &atmLinkIid, atmLinkObj, NULL);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	if(ptmLinkObj)zcfgFeObjJsonSet(RDM_OID_PTM_LINK, &ptmLinkIid, ptmLinkObj, NULL);
#endif
	if(ethLinkObj)zcfgFeObjJsonSet(RDM_OID_ETH_LINK, &ethLinkIid, ethLinkObj, NULL);
	
	if(vlanTermObj)
	{  
        //When editing PPPoE or PPPoA wan, do not set RDM_OID_ETH_VLAN_TERM to avoid to trigger pppoe daemon twice
		//Trigger pppoe daemon only in setting RDM_OID_PPP_IFACE
		if(!isAdd && (Encapsulation != NULL))
		{
	        if((!strcmp(Encapsulation, "PPPoE"))){
				if(!strcmp(CurrEncapsulation, "PPPoA") || !strcmp(CurrEncapsulation, "IPoA") || !strcmp(CurrEncapsulation, TR181_PPtP2PPPOA)){
	       			zcfgFeObjJsonSet(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, vlanTermObj, NULL);
				}
				else{
	        		zcfgFeObjJsonSetWithoutApply(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, vlanTermObj, NULL);
				}
	        }
		    else{
				zcfgFeObjJsonSet(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, vlanTermObj, NULL);
    		}
    	}
		else 
	    {
	        zcfgFeObjJsonSet(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, vlanTermObj, NULL);
	    }
	}
	if(!strcmp(Mode, "IP_Bridged")){
		dbg_printf("IP_Bridged\n");
#if 0
		if(!isAdd && !strcmp(CurrMode, "IP_Routed")){ //route to bridge case , need to restart or reload service?
			if(igmpObj)zcfgFeObjJsonSet(RDM_OID_ZY_IGMP, &igmpIid, igmpObj, NULL);
			if(mldObj)zcfgFeObjJsonSet(RDM_OID_ZY_MLD, &mldIid, mldObj, NULL);
			if(dnsObj)zcfgFeObjJsonSet(RDM_OID_DNS, &dnsIid, dnsObj, NULL);
		}
#endif
		if(v4AddrObj)zcfgFeObjJsonSetWithoutApply(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, v4AddrObj, NULL);
        if(ipIfaceObj)zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ipIfaceIid, ipIfaceObj, NULL);
		return ret;
	}
	
	if(natIntfObj)zcfgFeObjJsonSet(RDM_OID_NAT_INTF_SETTING, &natIntfIid, natIntfObj, NULL);
	if(dnsClientSrv4Obj)zcfgFeObjJsonSetWithoutApply(RDM_OID_DNS_CLIENT_SRV, &dnsClientSrv4Iid, dnsClientSrv4Obj, NULL);
	if(dnsClientSrv6Obj)zcfgFeObjJsonSetWithoutApply(RDM_OID_DNS_CLIENT_SRV, &dnsClientSrv6Iid, dnsClientSrv6Obj, NULL);
	if(mapObj)zcfgFeObjJsonSetWithoutApply(RDM_OID_MAP, &mapIid, mapObj, NULL);
	if(domainObj)zcfgFeObjJsonSetWithoutApply(RDM_OID_MAP_DOMAIN, &domainIid, domainObj, NULL);
	if(ruleObj)zcfgFeObjJsonSetWithoutApply(RDM_OID_MAP_DOMAIN_RULE, &ruleIid, ruleObj, NULL);
	if(v4FwdObj)zcfgFeObjJsonSetWithoutApply(RDM_OID_ROUTING_ROUTER_V4_FWD, &v4FwdIid, v4FwdObj, NULL);
	if(v4AddrObj)zcfgFeObjJsonSetWithoutApply(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, v4AddrObj, NULL);
	if(v6FwdObj)zcfgFeObjJsonSetWithoutApply(RDM_OID_ROUTING_ROUTER_V6_FWD, &v6FwdIid, v6FwdObj, NULL);
	if(v6AddrObj)zcfgFeObjJsonSetWithoutApply(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, v6AddrObj, NULL);
	if(ipIfaceObj)zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ipIfaceIid, ipIfaceObj, NULL);
	if(pppIfaceObj)zcfgFeObjJsonSet(RDM_OID_PPP_IFACE, &pppIfaceIid, pppIfaceObj, NULL);
	if(opt42Obj)zcfgFeObjJsonSetWithoutApply(RDM_OID_DHCPV4_REQ_OPT, &opt42Iid, opt42Obj, NULL);
	if(opt43Obj)zcfgFeObjJsonSetWithoutApply(RDM_OID_DHCPV4_REQ_OPT, &opt43Iid, opt43Obj, NULL);
	if(opt120Obj)zcfgFeObjJsonSetWithoutApply(RDM_OID_DHCPV4_REQ_OPT, &opt120Iid, opt120Obj, NULL);
	if(opt121Obj)zcfgFeObjJsonSetWithoutApply(RDM_OID_DHCPV4_REQ_OPT, &opt121Iid, opt121Obj, NULL);
	if(opt212Obj)zcfgFeObjJsonSetWithoutApply(RDM_OID_DHCPV4_REQ_OPT, &opt212Iid, opt212Obj, NULL);
	if(opt60Obj)zcfgFeObjJsonSetWithoutApply(RDM_OID_DHCPV4_SENT_OPT, &opt60Iid, opt60Obj, NULL);
	if(opt61Obj)zcfgFeObjJsonSetWithoutApply(RDM_OID_DHCPV4_SENT_OPT, &opt61Iid, opt61Obj, NULL);
	if(opt125Obj)zcfgFeObjJsonSetWithoutApply(RDM_OID_DHCPV4_SENT_OPT, &opt125Iid, opt125Obj, NULL);
	if(v4ClientObj)zcfgFeObjJsonSet(RDM_OID_DHCPV4_CLIENT, &v4ClientIid, v4ClientObj, NULL);
	if(igmpObj)zcfgFeObjJsonSet(RDM_OID_ZY_IGMP, &igmpIid, igmpObj, NULL);
	if(mldObj)zcfgFeObjJsonSet(RDM_OID_ZY_MLD, &mldIid, mldObj, NULL);
	if(routerObj)
	{
	  if(isAdd && Enable_6RD)
      {
             //Avoid to overwrite the v6 default gw
             //For 6RD case, update the default gw via zcfg_be
             zcfgFeObjJsonSetWithoutApply(RDM_OID_ROUTING_ROUTER, &routerIid, routerObj, NULL);
      }
      else
      {
          zcfgFeObjJsonBlockedSet(RDM_OID_ROUTING_ROUTER, &routerIid, routerObj, NULL);
      }
    }
	if(dnsObj)zcfgFeObjJsonSet(RDM_OID_DNS, &dnsIid, dnsObj, NULL);

	return ret;
}

zcfgRet_t freeAllObjects(){
	zcfgRet_t ret = ZCFG_SUCCESS;

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	if(atmLinkQosObj) zcfgFeJsonObjFree(atmLinkQosObj);
	if(atmLinkObj) zcfgFeJsonObjFree(atmLinkObj);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	if(ptmLinkObj) zcfgFeJsonObjFree(ptmLinkObj);
#endif
	if(ethLinkObj) zcfgFeJsonObjFree(ethLinkObj);
	if(vlanTermObj) zcfgFeJsonObjFree(vlanTermObj);
	if(pppIfaceObj) zcfgFeJsonObjFree(pppIfaceObj);
	if(ipIfaceObj) zcfgFeJsonObjFree(ipIfaceObj);
	if(natIntfObj) zcfgFeJsonObjFree(natIntfObj);
	if(dnsClientSrv4Obj) zcfgFeJsonObjFree(dnsClientSrv4Obj);
	if(dnsClientSrv6Obj) zcfgFeJsonObjFree(dnsClientSrv6Obj);
	if(v4FwdObj) zcfgFeJsonObjFree(v4FwdObj);
	if(v4AddrObj) zcfgFeJsonObjFree(v4AddrObj);
	if(v6FwdObj) zcfgFeJsonObjFree(v6FwdObj);
	if(v6AddrObj) zcfgFeJsonObjFree(v6AddrObj);
	if(opt42Obj) zcfgFeJsonObjFree(opt42Obj);
	if(opt43Obj) zcfgFeJsonObjFree(opt43Obj);
	if(opt120Obj) zcfgFeJsonObjFree(opt120Obj);
	if(opt121Obj) zcfgFeJsonObjFree(opt121Obj);
	if(opt212Obj) zcfgFeJsonObjFree(opt212Obj);
	if(opt60Obj) zcfgFeJsonObjFree(opt60Obj);
	if(opt61Obj) zcfgFeJsonObjFree(opt61Obj);
	if(opt125Obj) zcfgFeJsonObjFree(opt125Obj);
	if(v4ClientObj) zcfgFeJsonObjFree(v4ClientObj);
	if(igmpObj) zcfgFeJsonObjFree(igmpObj);
	if(mldObj) zcfgFeJsonObjFree(mldObj);
	if(routerObj) zcfgFeJsonObjFree(routerObj);
	if(dnsObj) zcfgFeJsonObjFree(dnsObj);
	if(mapObj) zcfgFeJsonObjFree(mapObj);
	if(domainObj) zcfgFeJsonObjFree(domainObj);
	if(ruleObj) zcfgFeJsonObjFree(ruleObj);
	if(ethIfaceObj) zcfgFeJsonObjFree(ethIfaceObj);
	if(optIfaceObj) zcfgFeJsonObjFree(optIfaceObj);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	if(cellIfaceObj) zcfgFeJsonObjFree(cellIfaceObj);
#endif
	if(dynamicDnsClientSrv4Obj) zcfgFeJsonObjFree(dynamicDnsClientSrv4Obj);
	if(dynamicDnsClientSrv6Obj) zcfgFeJsonObjFree(dynamicDnsClientSrv6Obj);
	
	if(featureFlagObj) zcfgFeJsonObjFree(featureFlagObj);

	return ret;
}


void getBasicInfo(struct json_object *Jobj){
	Name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	Type = json_object_get_string(json_object_object_get(Jobj, "Type"));
	Mode = json_object_get_string(json_object_object_get(Jobj, "Mode"));
	Encapsulation = json_object_get_string(json_object_object_get(Jobj, "Encapsulation"));
	DSCPEnable = json_object_get_boolean(json_object_object_get(Jobj, "DSCPEnable"));
	ipMode = json_object_get_string(json_object_object_get(Jobj, "ipMode"));
#ifdef ZYXEL_WAN_DHCPV6_MODE
	ip6Mode = json_object_get_string(json_object_object_get(Jobj, "ip6Mode"));		
#endif
#ifdef ZYXEL_PD_TYPE
	ip6StaticPrefix = json_object_get_string(json_object_object_get(Jobj, "ip6StaticPrefix"));
	ip6PrefixEnable = json_object_get_boolean(json_object_object_get(Jobj, "ip6PrefixEnable"));
	ip6PrefixStatic = json_object_get_boolean(json_object_object_get(Jobj, "ip6PrefixStatic"));		
#endif
	ipStatic = json_object_get_boolean(json_object_object_get(Jobj, "ipStatic"));
	ip6Static = json_object_get_boolean(json_object_object_get(Jobj, "ip6Static"));
	ip6DnsStatic = json_object_get_boolean(json_object_object_get(Jobj, "ip6DnsStatic"));
	Enable_6RD = json_object_get_boolean(json_object_object_get(Jobj, "Enable_6RD"));
	v6RD_Type = json_object_get_string(json_object_object_get(Jobj, "v6RD_Type"));
#ifdef ZYXEL_WAN_MAC
	WanMacType = json_object_get_string(json_object_object_get(Jobj, "WanMacType"));
	WanMacAddr = json_object_get_string(json_object_object_get(Jobj, "WanMacAddr"));
	WanMacIP = json_object_get_string(json_object_object_get(Jobj, "WanMacIP"));
#endif
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
#ifdef TAAAB_HGW
	HGWStyleName  = json_object_get_string(json_object_object_get(Jobj, "HGWStyleName"));
#else
	TAAABSrvName = json_object_get_string(json_object_object_get(Jobj, "TTSrvName"));
#endif
#endif
	if(json_object_object_get(Jobj, "isPppQuick")){
		isPppQuick = json_object_get_boolean(json_object_object_get(Jobj, "isPppQuick"));
	}

	if(isDelete)
	{	deleteipIfacePath = json_object_get_string(json_object_object_get(Jobj, "Del_ipIfacePath"));	}
}

void initGlobalObjects(){
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	atmLinkObj = NULL;
	atmLinkQosObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	ptmLinkObj = NULL;
#endif
	ethIfaceObj = NULL;
	optIfaceObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	cellIfaceObj = NULL;
#endif
	ethLinkObj = NULL;
	vlanTermObj = NULL;
	pppIfaceObj = NULL;
	ipIfaceObj = NULL;
	v4ClientObj = NULL;
	opt42Obj = NULL;
	opt43Obj = NULL;
	opt120Obj = NULL;
	opt121Obj = NULL;
	opt212Obj = NULL;
	opt60Obj = NULL;
	opt61Obj = NULL;
	opt125Obj = NULL;
	igmpObj = NULL;
	mldObj = NULL;
	natIntfObj = NULL;
	dnsClientSrv4Obj = NULL;
	dnsClientSrv6Obj = NULL;
	dynamicDnsClientSrv4Obj = NULL;
	dynamicDnsClientSrv6Obj = NULL;
	routerObj = NULL;
	v4FwdObj = NULL;
	v6FwdObj = NULL;
	//dynamicV4FwdObj = NULL;
	//dynamicV6FwdObj = NULL;
	v4AddrObj = NULL;
	v6AddrObj = NULL;
	dnsObj = NULL;
	mapObj = NULL;
	domainObj = NULL;
	ruleObj = NULL;

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	IID_INIT(atmLinkIid);
	IID_INIT(atmLinkQosIid);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	IID_INIT(ptmLinkIid);
#endif
	IID_INIT(ethIfaceIid);
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	IID_INIT(optIfaceIid);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	IID_INIT(cellIfaceIid);
#endif
	IID_INIT(ethLinkIid);
	IID_INIT(vlanTermIid);
	IID_INIT(pppIfaceIid);
	IID_INIT(ipIfaceIid);
	IID_INIT(v4ClientIid);
	IID_INIT(opt42Iid);
	IID_INIT(opt43Iid);
	IID_INIT(opt120Iid);
	IID_INIT(opt121Iid);
	IID_INIT(opt212Iid);
	IID_INIT(opt60Iid);
	IID_INIT(opt61Iid);
	IID_INIT(opt125Iid);
	IID_INIT(igmpIid);
	IID_INIT(mldIid);
	IID_INIT(natIntfIid);
	IID_INIT(ripIid);
	IID_INIT(dnsClientSrv4Iid);
	IID_INIT(dnsClientSrv6Iid);
	IID_INIT(dynamicDnsClientSrv4Iid);
	IID_INIT(dynamicDnsClientSrv6Iid);
	IID_INIT(routerIid);
	IID_INIT(v4FwdIid);
	IID_INIT(v6FwdIid);
	IID_INIT(v4AddrIid);
	IID_INIT(v6AddrIid);
	IID_INIT(dnsIid);
	IID_INIT(mapIid);
	IID_INIT(domainIid);
	IID_INIT(ruleIid);

	memset(CurrType, 0, sizeof(CurrType));
	memset(CurrMode, 0, sizeof(CurrMode));
	memset(CurrEncapsulation, 0, sizeof(CurrEncapsulation));
	memset(CurrIpMode, 0, sizeof(CurrIpMode));
	memset(CurrV6RD_Type, 0, sizeof(CurrV6RD_Type));
	
	CurrIpStatic = false;
	CurrIp6Static = false;
	CurrPppStatic = false;
	CurrEnable_6RD = false;
	
}

zcfgRet_t getCurrentConfig(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *HigherLayer;
	const char *LowerLayer;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;
	objIndex_t iid;
	int count = 0;
	bool v4enable, v6enable, Enable;
#ifdef ABUE_CUSTOMIZATION	
	bool Enable_DSLite;
#endif	
	const char *Interface;
	const char *AddressingType;
	const char *DestIPAddress;
	const char *DestSubnetMask;
	const char *Origin;
	//bool staticRoute = false;
	const char *DestIPPrefix;
	const char *DNSServer;
	const char *X_ZYXEL_Type;
	const char *X_ZYXEL_LocalIPAddress;
	bool found = false;

	//check if target WAN exist
	if(isDelete == false)
	{
		if(!getSpecificObj(RDM_OID_IP_IFACE, "X_ZYXEL_SrvName", json_type_string, (void *)Name, &ipIfaceIid, &ipIfaceObj))
			return ZCFG_REQUEST_REJECT;
	}
	else
	{
		if(deleteipIfacePath != NULL)
		{
			if(strstr(deleteipIfacePath, "IP.Interface.") != NULL)
			{
				ipIfaceIid.level = 1;
				sscanf(deleteipIfacePath, "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj);
				if(ipIfaceObj)
				{	found = true;	}
			}
		}
		else{
			if(!getSpecificObj(RDM_OID_IP_IFACE, "X_ZYXEL_SrvName", json_type_string, (void *)Name, &ipIfaceIid, &ipIfaceObj))
				return ZCFG_REQUEST_REJECT;
			found = true;
		}
		
		if(!found)
		{	return ZCFG_REQUEST_REJECT;   }
	}
	
	sprintf(ipIfacePath, "IP.Interface.%u", ipIfaceIid.idx[0]);
	LowerLayer = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
	ZOS_STRCPY_M(CurrMode, json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionType")));

	HigherLayer = ipIfacePath;
	while(count < 4){
		dbg_printf("count=%d\n", count);
		dbg_printf("HigherLayer=%s\n", HigherLayer);
		dbg_printf("LowerLayer=%s\n", LowerLayer);
		
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
				ZOS_STRCPY_M(CurrEncapsulation, "IPoE");
			}
			else if(!strncmp("Ethernet.Link.", LowerLayer, 14)){
				ethLinkIid.level = 1;
				sscanf(LowerLayer, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkObj);
				HigherLayer = LowerLayer;
				LowerLayer = json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
				ZOS_STRCPY_M(CurrEncapsulation, "IPoE");
			}
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
			else if(!strncmp("ATM.Link.", LowerLayer, 9) && !strcmp("IP_Routed", CurrMode)){
				atmLinkIid.level = 1;
				sscanf(LowerLayer, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj);
				ZOS_STRCPY_M(CurrEncapsulation, "IPoA");
				ZOS_STRCPY_M(CurrType, "ATM");
				memcpy(&atmLinkQosIid, &atmLinkIid, sizeof(objIndex_t));
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK_QOS, &atmLinkQosIid, &atmLinkQosObj);
				break;
			}
#endif
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
				ZOS_STRCPY_M(CurrEncapsulation, "PPPoE");
			}
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
			else if(!strncmp("ATM.Link.", LowerLayer, 9)){
				atmLinkIid.level = 1;
				sscanf(LowerLayer, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj);
#ifdef ZYXEL_PPTPD_RELAY
				ZOS_STRCPY_M(CurrEncapsulation, json_object_get_string(json_object_object_get(atmLinkObj, "LinkType")));
#else
				ZOS_STRCPY_M(CurrEncapsulation, "PPPoA");
#endif
				
				ZOS_STRCPY_M(CurrType, "ATM");
				memcpy(&atmLinkQosIid, &atmLinkIid, sizeof(objIndex_t));
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK_QOS, &atmLinkQosIid, &atmLinkQosObj);
				break;
			}
#endif
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
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
			if(!strncmp("ATM.Link.", LowerLayer, 9)){
				atmLinkIid.level = 1;
				sscanf(LowerLayer, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj);
				ZOS_STRCPY_M(CurrType, "ATM");
				memcpy(&atmLinkQosIid, &atmLinkIid, sizeof(objIndex_t));
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK_QOS, &atmLinkQosIid, &atmLinkQosObj);
				break;
			}
			else
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
			if(!strncmp("PTM.Link.", LowerLayer, 9)){
				ptmLinkIid.level = 1;
				sscanf(LowerLayer, "PTM.Link.%hhu", &ptmLinkIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PTM_LINK, &ptmLinkIid, &ptmLinkObj);
				ZOS_STRCPY_M(CurrType, "PTM");
				break;
			}
			else
#endif
			if(!strncmp("Ethernet.Interface.", LowerLayer, 19)){
				ethIfaceIid.level = 1;
				sscanf(LowerLayer, "Ethernet.Interface.%hhu", &ethIfaceIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_IFACE, &ethIfaceIid, &ethIfaceObj);
				if(json_object_get_boolean(json_object_object_get(ethIfaceObj, "X_ZYXEL_SFP")) == false)
				ZOS_STRCPY_M(CurrType, "ETH");
				else
					ZOS_STRCPY_M(CurrType, "SFP");
				if(json_object_get_boolean(json_object_object_get(ethIfaceObj, "Upstream")) == false){
					if(json_object_get_boolean(json_object_object_get(ethLinkObj, "X_ZYXEL_OrigEthWAN")) == false){
						return ZCFG_REQUEST_REJECT;
					}
				}
				break;
			}
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
			else if(!strncmp("Optical.Interface.", LowerLayer, 18)){
				optIfaceIid.level = 1;
				sscanf(LowerLayer, "Optical.Interface.%hhu", &optIfaceIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_OPT_INTF, &optIfaceIid, &optIfaceObj);
				if(json_object_get_boolean(json_object_object_get(optIfaceObj, "Upstream")) == false){
					printf("The LowerLayer Optical Interface is a LAN interface.\n");
					return ZCFG_REQUEST_REJECT;
				}
				ZOS_STRCPY_M(CurrType, "PON");
				break;
			}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
			else if(!strncmp("Cellular.Interface.", LowerLayer, 19)){
				cellIfaceIid.level = 1;
				sscanf(LowerLayer, "Cellular.Interface.%hhu", &cellIfaceIid.idx[0]);
				zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF, &cellIfaceIid, &cellIfaceObj);
				if(json_object_get_boolean(json_object_object_get(cellIfaceObj, "Upstream")) == false){
					printf("The LowerLayer Cellular Interface is a LAN interface.\n");
					return ZCFG_REQUEST_REJECT;
				}
				ZOS_STRCPY_M(CurrType, "CELL");
				break;
			}
#endif
			else if(!strcmp("", LowerLayer)){
				if(json_object_get_boolean(json_object_object_get(ethLinkObj, "X_ZYXEL_OrigEthWAN")) == false){
					return ZCFG_REQUEST_REJECT;
				}
				ZOS_STRCPY_M(CurrType, "ETH");
				break;
			}
			else{
				return ZCFG_REQUEST_REJECT;
			}
		}
	}

	if(isDelete)
		return ZCFG_SUCCESS;

	if(Type == NULL)
		Type = CurrType;
	if(Mode == NULL)
		Mode = CurrMode;

	//dont allow to change Type
	if(strcmp(CurrType, Type))
		return ZCFG_REQUEST_REJECT;

	if(!strcmp(Mode, "IP_Bridged")){
		json_object_object_add(Jobj, "IGMPEnable", json_object_new_boolean(false));
		json_object_object_add(Jobj, "MLDEnable", json_object_new_boolean(false));
		json_object_object_add(Jobj, "sysGwEnable", json_object_new_boolean(false));
		json_object_object_add(Jobj, "sysGw6Enable", json_object_new_boolean(false));

        /*Consider static ip/routing mode to bridge mode*/
		v4AddrIid.level = 2;
		v4AddrIid.idx[0] = ipIfaceIid.idx[0];
		v4AddrIid.idx[1] = 1;
		zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj);
		if(v4AddrObj){
			AddressingType = json_object_get_string(json_object_object_get(v4AddrObj, "AddressingType"));
			Enable = json_object_get_boolean(json_object_object_get(v4AddrObj, "Enable"));
			if(Enable == true && !strcmp(AddressingType, "Static") && strstr(CurrEncapsulation, "IP")){
				json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(false));
			}
		}

		v6AddrIid.level = 2;
		v6AddrIid.idx[0] = ipIfaceIid.idx[0];
		v6AddrIid.idx[1] = 1;
		zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, &v6AddrObj);
		if(v6AddrObj){
			Origin = json_object_get_string(json_object_object_get(v6AddrObj, "Origin"));
			Enable = json_object_get_boolean(json_object_object_get(v6AddrObj, "Enable"));
			if(Enable == true && !strcmp(Origin, "Static")){
				json_object_object_add(v6AddrObj, "Enable", json_object_new_boolean(false));
			}
		}

		return ret;
	}

	if(!strcmp(CurrEncapsulation, "IPoE") || !strcmp(CurrEncapsulation, "IPoA")){
		v4enable = json_object_get_boolean(json_object_object_get(ipIfaceObj, "IPv4Enable"));
		v6enable = json_object_get_boolean(json_object_object_get(ipIfaceObj, "IPv6Enable"));
	}
	else{ //PPPoE or PPPoA
		v4enable = json_object_get_boolean(json_object_object_get(pppIfaceObj, "IPCPEnable"));
		v6enable = json_object_get_boolean(json_object_object_get(pppIfaceObj, "IPv6CPEnable"));
	}

    /* __ZyXEL__, Albert, 20180209,Support IPv6 option IA_NA and IA_PD  */
    IPv6Manual_IANA = json_object_get_boolean(json_object_object_get(ipIfaceObj, "IPv6Manual_IANA"));
    IPv6Manual_IAPD = json_object_get_boolean(json_object_object_get(ipIfaceObj, "IPv6Manual_IAPD"));
#ifdef ABUE_CUSTOMIZATION
	Enable_DSLite = json_object_get_boolean(json_object_object_get(ipIfaceObj, "X_ZYXEL_Enable_DSLite"));
#endif
#ifdef ABUE_CUSTOMIZATION
	if(v4enable && v6enable && !Enable_DSLite)
		ZOS_STRCPY_M(CurrIpMode, "DualStack");
	else if(v4enable && v6enable && Enable_DSLite)
		ZOS_STRCPY_M(CurrIpMode, "AutoConfig");
#else
	if(v4enable && v6enable)
		ZOS_STRCPY_M(CurrIpMode, "DualStack");
#endif	
	else if(v4enable)
		ZOS_STRCPY_M(CurrIpMode, "IPv4");
	else if(v6enable)
		ZOS_STRCPY_M(CurrIpMode, "IPv6");

	if(Encapsulation == NULL)
		Encapsulation = CurrEncapsulation;
	if(ipMode == NULL)
		ipMode = CurrIpMode;

	getSpecificObj(RDM_OID_DHCPV4_CLIENT, "Interface", json_type_string, ipIfacePath, &v4ClientIid, &v4ClientObj);
	if(v4ClientObj){
		getDhcpv4ClientOpt(&v4ClientIid);
		if(!strcmp(CurrIpMode, "IPv4"))
			CurrEnable_6RD = json_object_get_boolean(json_object_object_get(ipIfaceObj, "X_ZYXEL_Enable_6RD"));
		
		if(CurrEnable_6RD)
			ZOS_STRCPY_M(CurrV6RD_Type, json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_6RD_Type")));
	}

	Enable_6RD = false;
	if(!strcmp(ipMode, "IPv4")){
		pramJobj = json_object_object_get(Jobj, "Enable_6RD");
		if(pramJobj)
			Enable_6RD = json_object_get_boolean(pramJobj);
		else
			Enable_6RD = CurrEnable_6RD;

		if(Enable_6RD){
			pramJobj = json_object_object_get(Jobj, "v6RD_Type");
			if(pramJobj){
				v6RD_Type = json_object_get_string(json_object_object_get(Jobj, "v6RD_Type"));
			}
			else{
				if(strcmp(CurrV6RD_Type, ""))
					v6RD_Type = CurrV6RD_Type;
				else{
					ZOS_STRCPY_M(CurrV6RD_Type, "dhcp");
					v6RD_Type = CurrV6RD_Type;
				}
			}

			if(!strcmp(v6RD_Type, "dhcp"))
				json_object_object_add(Jobj, "option212Enable", json_object_new_boolean(true));
		}
	}

	//NAT
	getSpecificObj(RDM_OID_NAT_INTF_SETTING, "Interface", json_type_string, ipIfacePath, &natIntfIid, &natIntfObj);
	//Default Gateway
	IID_INIT(routerIid);
	routerIid.level = 1;
	routerIid.idx[0] = 1;
	zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ROUTING_ROUTER, &routerIid, &routerObj);
	
	//static ipv4 Addr
	IID_INIT(v4AddrIid);
#if 1
	v4AddrIid.level = 2;
	v4AddrIid.idx[0] = ipIfaceIid.idx[0];
	v4AddrIid.idx[1] = 1;
	zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj);
	if(v4AddrObj){
		AddressingType = json_object_get_string(json_object_object_get(v4AddrObj, "AddressingType"));
		Enable = json_object_get_boolean(json_object_object_get(v4AddrObj, "Enable"));
		if(Enable == true && !strcmp(AddressingType, "Static") && strstr(CurrEncapsulation, "IP")){
			CurrIpStatic = true;
		}
	}
#else
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &ipIfaceIid, &v4AddrIid, &v4AddrObj) == ZCFG_SUCCESS){
		AddressingType = json_object_get_string(json_object_object_get(v4AddrObj, "AddressingType"));
		if(!strcmp(AddressingType, "Static")){
			break;
		}
		zcfgFeJsonObjFree(v4AddrObj);
	}
#endif
	if(CurrIpStatic){
		IID_INIT(v4FwdIid);
		while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, &routerIid, &v4FwdIid, &v4FwdObj) == ZCFG_SUCCESS){
			Interface = json_object_get_string(json_object_object_get(v4FwdObj, "Interface"));
			//Origin = json_object_get_string(json_object_object_get(v4FwdObj, "Origin"));
			//staticRoute =  json_object_get_boolean(json_object_object_get(v4FwdObj, "StaticRoute"));
			//if(!strcmp(Interface, ipIfacePath) && !strcmp(Origin, "Static")){
			if(!strcmp(Interface, ipIfacePath)){
				DestSubnetMask = json_object_get_string(json_object_object_get(v4FwdObj, "DestSubnetMask"));
				DestIPAddress = json_object_get_string(json_object_object_get(v4FwdObj, "DestIPAddress"));
				if(!strcmp(DestIPAddress, "") && !strcmp(DestSubnetMask, "")){
					break;
				}
			}
			zcfgFeJsonObjFree(v4FwdObj);
		}
	}

	//static ppp
	if(strstr(CurrEncapsulation, "PPP") && pppIfaceObj != NULL){
		X_ZYXEL_LocalIPAddress = json_object_get_string(json_object_object_get(pppIfaceObj, "X_ZYXEL_LocalIPAddress"));
		if(strcmp(X_ZYXEL_LocalIPAddress, "0.0.0.0") && strcmp(X_ZYXEL_LocalIPAddress, "")){
			CurrPppStatic = true;
		}
	}

	//static ipv6 Addr
	IID_INIT(v6AddrIid);
#if 1
	v6AddrIid.level = 2;
	v6AddrIid.idx[0] = ipIfaceIid.idx[0];
	v6AddrIid.idx[1] = 1;
	zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, &v6AddrObj);
	if(v6AddrObj){
		Origin = json_object_get_string(json_object_object_get(v6AddrObj, "Origin"));
		Enable = json_object_get_boolean(json_object_object_get(v6AddrObj, "Enable"));
		if(Enable == true && !strcmp(Origin, "Static")){
			CurrIp6Static = true;
		}
	}
#else
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE_V6_ADDR, &ipIfaceIid, &v6AddrIid, &v6AddrObj) == ZCFG_SUCCESS){
		Origin = json_object_get_string(json_object_object_get(v6AddrObj, "Origin"));
		if(!strcmp(Origin, "Static")){
			CurrIp6Static = true;
			break;
		}
		zcfgFeJsonObjFree(v6AddrObj);
	}
#endif
	//if(CurrIp6Static){
		IID_INIT(v6FwdIid);
		while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &routerIid, &v6FwdIid, &v6FwdObj) == ZCFG_SUCCESS){
			Interface = json_object_get_string(json_object_object_get(v6FwdObj, "Interface"));
			// Origin = json_object_get_string(json_object_object_get(v6FwdObj, "Origin"));
			//if(!strcmp(Interface, ipIfacePath) && !strcmp(Origin, "Static")){
			if(!strcmp(Interface, ipIfacePath)){
				DestIPPrefix = json_object_get_string(json_object_object_get(v6FwdObj, "DestIPPrefix"));
				if(!strcmp(DestIPPrefix, "::/0") || !strcmp(DestIPPrefix, "")){
					break;
				}
			}
			zcfgFeJsonObjFree(v6FwdObj);
		}
	//}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		objIndex_t tmpiid;
		struct json_object *tmpipIfObj = NULL;
		int upwantype = 0;
		const char *curTAAABSrvName;
		const char *tmpTAAABSrvName;
		char tempipIfacePath[32];

    upwantype = UpWanTypeGet();   
	curTAAABSrvName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_TT_SrvName"));
	
    if(upwantype == 2 && strstr(curTAAABSrvName,"_DSL")!=NULL){
		
        IID_INIT(tmpiid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &tmpiid, &tmpipIfObj) == ZCFG_SUCCESS){
               tmpTAAABSrvName = json_object_get_string(json_object_object_get(tmpipIfObj, "X_TT_SrvName"));
			   if(!strcmp(curTAAABSrvName,tmpTAAABSrvName) && (tmpiid.idx[0] != ipIfaceIid.idx[0])){
                     sprintf(tempipIfacePath, "IP.Interface.%d", tmpiid.idx[0]);
					 zcfgFeJsonObjFree(tmpipIfObj);
					 break;
			   }
			   zcfgFeJsonObjFree(tmpipIfObj);
		}

	//static DNS
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DNS_CLIENT_SRV, &iid, &obj) == ZCFG_SUCCESS){
		Interface = json_object_get_string(json_object_object_get(obj, "Interface"));
		X_ZYXEL_Type = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Type"));
		if(!strcmp(Interface, tempipIfacePath) && !strcmp(X_ZYXEL_Type, "Static")){
			DNSServer = json_object_get_string(json_object_object_get(obj, "DNSServer"));
			if((strchr(DNSServer, ':') == NULL) && (dnsClientSrv4Obj == NULL)){
				dnsClientSrv4Obj = obj;
				memcpy(&dnsClientSrv4Iid, &iid, sizeof(objIndex_t));
			}
			else if(dnsClientSrv6Obj == NULL){
				dnsClientSrv6Obj = obj;
				memcpy(&dnsClientSrv6Iid, &iid, sizeof(objIndex_t));
			}
			else{
				zcfgFeJsonObjFree(obj);
			}
		}
		else if(!strcmp(Interface, tempipIfacePath) && strcmp(X_ZYXEL_Type, "Static")){
			DNSServer = json_object_get_string(json_object_object_get(obj, "DNSServer"));
			if((strchr(DNSServer, ':') == NULL) && (dynamicDnsClientSrv4Obj == NULL)){
				dynamicDnsClientSrv4Obj = obj;
				memcpy(&dynamicDnsClientSrv4Iid, &iid, sizeof(objIndex_t));
			}
			else if(dynamicDnsClientSrv6Obj == NULL){
				dynamicDnsClientSrv6Obj = obj;
				memcpy(&dynamicDnsClientSrv6Iid, &iid, sizeof(objIndex_t));
			}
			else{
				zcfgFeJsonObjFree(obj);
			}
		}
		else{
			zcfgFeJsonObjFree(obj);
		}
	}

    }
	else{
		

	//static DNS
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DNS_CLIENT_SRV, &iid, &obj) == ZCFG_SUCCESS){
		Interface = json_object_get_string(json_object_object_get(obj, "Interface"));
		X_ZYXEL_Type = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Type"));
		if(!strcmp(Interface, ipIfacePath) && !strcmp(X_ZYXEL_Type, "Static")){
			DNSServer = json_object_get_string(json_object_object_get(obj, "DNSServer"));
			if((strchr(DNSServer, ':') == NULL) && (dnsClientSrv4Obj == NULL)){
				dnsClientSrv4Obj = obj;
				memcpy(&dnsClientSrv4Iid, &iid, sizeof(objIndex_t));
			}
			else if(dnsClientSrv6Obj == NULL){
				dnsClientSrv6Obj = obj;
				memcpy(&dnsClientSrv6Iid, &iid, sizeof(objIndex_t));
			}
			else{
				zcfgFeJsonObjFree(obj);
			}
		}
		else if(!strcmp(Interface, ipIfacePath) && strcmp(X_ZYXEL_Type, "Static")){
			DNSServer = json_object_get_string(json_object_object_get(obj, "DNSServer"));
			if((strchr(DNSServer, ':') == NULL) && (dynamicDnsClientSrv4Obj == NULL)){
				dynamicDnsClientSrv4Obj = obj;
				memcpy(&dynamicDnsClientSrv4Iid, &iid, sizeof(objIndex_t));
			}
			else if(dynamicDnsClientSrv6Obj == NULL){
				dynamicDnsClientSrv6Obj = obj;
				memcpy(&dynamicDnsClientSrv6Iid, &iid, sizeof(objIndex_t));
			}
			else{
				zcfgFeJsonObjFree(obj);
			}
		}
		else{
			zcfgFeJsonObjFree(obj);
		}
	}

		}
#else
	//static DNS
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DNS_CLIENT_SRV, &iid, &obj) == ZCFG_SUCCESS){
		Interface = json_object_get_string(json_object_object_get(obj, "Interface"));
		X_ZYXEL_Type = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Type"));
		if(!strcmp(Interface, ipIfacePath) && !strcmp(X_ZYXEL_Type, "Static")){
			DNSServer = json_object_get_string(json_object_object_get(obj, "DNSServer"));
			if((strchr(DNSServer, ':') == NULL) && (dnsClientSrv4Obj == NULL)){
				dnsClientSrv4Obj = obj;
				memcpy(&dnsClientSrv4Iid, &iid, sizeof(objIndex_t));
			}
			else if(dnsClientSrv6Obj == NULL){
				dnsClientSrv6Obj = obj;
				memcpy(&dnsClientSrv6Iid, &iid, sizeof(objIndex_t));
			}
			else{
				zcfgFeJsonObjFree(obj);
			}
		}
		else if(!strcmp(Interface, ipIfacePath) && strcmp(X_ZYXEL_Type, "Static")){
			DNSServer = json_object_get_string(json_object_object_get(obj, "DNSServer"));
			if((strchr(DNSServer, ':') == NULL) && (dynamicDnsClientSrv4Obj == NULL)){
				dynamicDnsClientSrv4Obj = obj;
				memcpy(&dynamicDnsClientSrv4Iid, &iid, sizeof(objIndex_t));
			}
			else if(dynamicDnsClientSrv6Obj == NULL){
				dynamicDnsClientSrv6Obj = obj;
				memcpy(&dynamicDnsClientSrv6Iid, &iid, sizeof(objIndex_t));
			}
			else{
				zcfgFeJsonObjFree(obj);
			}
		}
		else{
			zcfgFeJsonObjFree(obj);
		}
	}
#endif
	
	//IGMP, MLD
	IID_INIT(igmpIid);
	zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IGMP, &igmpIid, &igmpObj);
	IID_INIT(mldIid);
	zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MLD, &mldIid, &mldObj);

	IID_INIT(dnsIid);
	zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DNS, &dnsIid, &dnsObj);

	return ret;
}

#ifdef MSTC_TAAAG_MULTI_USER
zcfgRet_t UnnecessarymodifyObj(struct json_object *Jobj){	
	zcfgRet_t ret = ZCFG_SUCCESS;
	char Name[64] = {0};
	struct json_object *TTuserobj = NULL;
	objIndex_t TAAABuserIid;
	char userName[512] = {0};
	char TAAABuserame[512] = {0};
	int curloginLevel = 0;
	struct json_object *atmlinkobj = NULL;
	objIndex_t atmlinkIid;
	const char *vpivci = json_object_get_string(json_object_object_get(Jobj, "vpivci"));
	const char *currVpivci = NULL;
	const char *atmLinkType = json_object_get_string(json_object_object_get(Jobj, "AtmEncapsulation"));
	const char *curratmLinkType = NULL;
	struct json_object *atmQosobj = NULL;
	objIndex_t atmQosIid;
	const char *QoSClass = json_object_get_string(json_object_object_get(Jobj, "QoSClass"));
	const char *currQoSClass = NULL;
	struct json_object *InternetDSLVLANTermobj = NULL;
	objIndex_t InternetDSLVLANTermIid;
	struct json_object *InternetWANVLANTermobj = NULL;
	objIndex_t InternetWANVLANTermIid;
	struct json_object *IPTVDSLVLANTermobj = NULL;
	objIndex_t IPTVDSLVLANTermIid;
	struct json_object *IPTVWANVLANTermobj = NULL;
	objIndex_t IPTVWANVLANTermIid;
	struct json_object *MANAGEMENTVLANTermobj = NULL;
	objIndex_t MANAGEMENTVLANTermIid;
	bool VlanEnable = json_object_get_boolean(json_object_object_get(Jobj, "VLANEnable"));
	bool currInternetDSLVlanEnable = false;
	bool currInternetWANVlanEnable = false;
	bool currIPTVDSLVlanEnable = false;
	bool currIPTVWANVlanEnable = false;
	bool currMANAGEMENTVlanEnable = false;
	int VlanID = json_object_get_int(json_object_object_get(Jobj, "VLANID"));
	int currInternetDSLVlanID = 0;
	int currInternetWANVlanID = 0;
	int currIPTVDSLVlanID = 0;
	int currIPTVWANVlanID = 0;
	int currMANAGEMENTVlanID = 0;
	int VlanPri = json_object_get_int(json_object_object_get(Jobj, "VLANPriority"));
	int currInternetDSLVlanPri = 0;
	int currInternetWANVlanPri = 0;
	int currIPTVDSLVlanPri = 0;
	int currIPTVWANVlanPri = 0;
	int currMANAGEMENTVlanPri = 0;
	struct json_object *IPTVDSLintfobj = NULL;
	objIndex_t IPTVDSLintfIid;
	struct json_object *IPTVWANintfobj = NULL;
	objIndex_t IPTVWANintfIid;
	struct json_object *MANAGEMENTintfobj = NULL;
	objIndex_t MANAGEMENTintfIid;
	uint32_t MaxMTUSize = json_object_get_int(json_object_object_get(Jobj, "MaxMTUSize"));
	uint32_t currIPTVDSLMaxMTUSize = 0;
	uint32_t currIPTVWANMaxMTUSize = 0;
	uint32_t currMANAGEMENTMaxMTUSize = 0;
	struct json_object *IPTVDSLethlinkobj = NULL;
	objIndex_t IPTVDSLethlinkIid;
	struct json_object *IPTVWANethlinkobj = NULL;
	objIndex_t IPTVWANethlinkIid;
	struct json_object *MANAGEMENTethlinkobj = NULL;
	objIndex_t MANAGEMENTethlinkIid;
#ifdef ZYXEL_WAN_MAC
	//char *WanMacType = json_object_get_string(json_object_object_get(Jobj, "WanMacType"));
	char *currIPTVDSLWanMacType = NULL;
	char *currIPTVWANWanMacType = NULL;
	char *currMANAGEMENTWanMacType = NULL;
#endif	
	bool ipDnsStatic = false;
	int index = 0;
    struct json_object *IPTV_DSLJarray = NULL;
    struct json_object *IPTV_WANJarray = NULL;
	struct json_object *ManagementJarray = NULL;
	objIndex_t IPTV_DSLIid,IPTV_WANIid,ManagementIid;
	bool IPTV_DSLLimitation = true;
	bool IPTV_WANLimitation = true;
	bool ManagementLimitation = true;

	ZOS_STRCPY_M(Name, json_object_get_string(json_object_object_get(Jobj, "Name")));
	index = json_object_get_int(json_object_object_get(Jobj, "index"));

	IID_INIT(TAAABuserIid);
	ZOS_STRCPY_M(userName, json_object_get_string(json_object_object_get(Jobj, "DAL_LOGIN_USERNAME")));
	if (userName != NULL && strlen(userName) > 0){
		while(zcfgFeObjJsonGetNext(RDM_OID_TAAAB_USER, &TTuserIid,&TTuserobj) == ZCFG_SUCCESS){
			ZOS_STRCPY_M(TAAABuserame, Jgets(TAAABuserobj, "Username"));
			if(strcmp(userName, TAAABuserame) == 0){
				curloginLevel = Jgeti(TAAABuserobj, "Level");
				json_object_put(TAAABuserobj);
				break;
			}
			json_object_put(TAAABuserobj);
		}
	}

	IID_INIT(IPTV_DSLIid);
	if ( zcfgFeObjJsonGet(RDM_OID_IPTV_DSL, &IPTV_DSLIid,&IPTV_DSLJarray) == ZCFG_SUCCESS)
	{
		IPTV_DSLLimitation = Jgetb(IPTV_DSLJarray, "Limitation");
		zcfgFeJsonObjFree(IPTV_DSLJarray);
	}
	IID_INIT(IPTV_WANIid);
	if ( zcfgFeObjJsonGet(RDM_OID_IPTV_WAN, &IPTV_WANIid,&IPTV_WANJarray) == ZCFG_SUCCESS)
	{
		IPTV_WANLimitation = Jgetb(IPTV_WANJarray, "Limitation");
		zcfgFeJsonObjFree(IPTV_WANJarray);
	}
	IID_INIT(ManagementIid);
	if ( zcfgFeObjJsonGet(RDM_OID_MANAGEMENT_WAN, &ManagementIid,&ManagementJarray) == ZCFG_SUCCESS)
	{
		ManagementLimitation = Jgetb(ManagementJarray, "Limitation");
		zcfgFeJsonObjFree(ManagementJarray);
	}

	if(curloginLevel == 1){
		if(((index == 4 && IPTV_DSLLimitation == true) || (strstr(Name,"IPTV_ETHWAN") != NULL && IPTV_WANLimitation == true) || (strstr(Name,"ETH-MANAGEMENT") != NULL && ManagementLimitation == true)) && !isAdd){
			if(Mode == NULL)
				Mode = CurrMode;

			//dont allow to change Mode
			if(strcmp(CurrMode, Mode))
				Mode = CurrMode;

			if(Encapsulation == NULL)
				Encapsulation = CurrEncapsulation;

			if(strcmp(CurrEncapsulation, Encapsulation))
				Encapsulation = CurrEncapsulation;
			
			if(ipMode == NULL)
				ipMode = CurrIpMode;
			
			if(strcmp(CurrIpMode, ipMode))
				ipMode = CurrIpMode;

			if(index == 4){
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
				IID_INIT(atmlinkIid);
				atmlinkIid.level = 1;
				atmlinkIid.idx[0] = 2;

				if(zcfgFeObjJsonGet(RDM_OID_ATM_LINK, &atmlinkIid,&atmlinkobj) != ZCFG_SUCCESS){
					printf("cannot get RDM_OID_ATM_LINK obj");
				}		
				currVpivci = json_object_get_string(json_object_object_get(atmlinkobj, "DestinationAddress"));
				
				if(vpivci != NULL && strcmp(currVpivci, vpivci)){
					vpivci = currVpivci;
					json_object_object_add(Jobj, "vpivci", json_object_new_string(currVpivci));
				}

				curratmLinkType = json_object_get_string(json_object_object_get(atmlinkobj, "Encapsulation"));
				
				if(atmLinkType != NULL && strcmp(atmLinkType, curratmLinkType)){
					atmLinkType = curratmLinkType;
					json_object_object_add(Jobj, "AtmEncapsulation", json_object_new_string(atmLinkType));
				}
				zcfgFeJsonObjFree(atmlinkobj);

				IID_INIT(atmQosIid);
				atmQosIid.level = 1;
				atmQosIid.idx[0] = 2;
				if(zcfgFeObjJsonGet(RDM_OID_ATM_LINK_QOS, &atmQosIid,&atmQosobj) != ZCFG_SUCCESS){
					printf("cannot get RDM_OID_ATM_LINK_QOS obj");
				}		
				currQoSClass = json_object_get_string(json_object_object_get(atmQosobj, "QoSClass"));
				
				if(QoSClass != NULL && strcmp(QoSClass, currQoSClass)){
					QoSClass = currQoSClass;
					json_object_object_add(Jobj, "QoSClass", json_object_new_string(QoSClass));
				}
				zcfgFeJsonObjFree(atmQosobj);
#else
				printf("%s : ATM not support\n", __func__);
				return ZCFG_INTERNAL_ERROR; 
#endif
			}
			IID_INIT(IPTVDSLintfIid);
			IPTVDSLintfIid.level = 1;
			IPTVDSLintfIid.idx[0] = 6;
			if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &IPTVDSLintfIid,&IPTVDSLintfobj) != ZCFG_SUCCESS){
				printf("cannot get RDM_OID_IP_IFACE obj");
			}

			IID_INIT(IPTVWANintfIid);
			IPTVWANintfIid.level = 1;
			IPTVWANintfIid.idx[0] = 7;
			if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &IPTVWANintfIid,&IPTVWANintfobj) != ZCFG_SUCCESS){
				printf("cannot get RDM_OID_IP_IFACE obj");
			}

			IID_INIT(MANAGEMENTintfIid);
			MANAGEMENTintfIid.level = 1;
			MANAGEMENTintfIid.idx[0] = 9;
			if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &MANAGEMENTintfIid,&MANAGEMENTintfobj) != ZCFG_SUCCESS){
				printf("cannot get RDM_OID_IP_IFACE obj");
			}
			currIPTVDSLMaxMTUSize = json_object_get_int(json_object_object_get(IPTVDSLintfobj, "MaxMTUSize"));
			currIPTVWANMaxMTUSize = json_object_get_int(json_object_object_get(IPTVWANintfobj, "MaxMTUSize"));		
			currMANAGEMENTMaxMTUSize = json_object_get_int(json_object_object_get(MANAGEMENTintfobj, "MaxMTUSize"));
			if(index == 4){
				if(MaxMTUSize != currIPTVDSLMaxMTUSize){
					MaxMTUSize = currIPTVDSLMaxMTUSize;
					json_object_object_add(Jobj, "MaxMTUSize", json_object_new_int(MaxMTUSize));
				}
			}
			else if(index == 5){
				if(MaxMTUSize != currIPTVWANMaxMTUSize){
					MaxMTUSize = currIPTVWANMaxMTUSize;
					json_object_object_add(Jobj, "MaxMTUSize", json_object_new_int(MaxMTUSize));
				}
			}
			else if(index == 7){
				if(MaxMTUSize != currMANAGEMENTMaxMTUSize){
					MaxMTUSize = currMANAGEMENTMaxMTUSize;
					json_object_object_add(Jobj, "MaxMTUSize", json_object_new_int(MaxMTUSize));
				}
			}
			zcfgFeJsonObjFree(IPTVDSLintfobj);
			zcfgFeJsonObjFree(IPTVWANintfobj);
			zcfgFeJsonObjFree(MANAGEMENTintfobj);

			IID_INIT(IPTVDSLethlinkIid);
			IPTVDSLethlinkIid.level = 1;
			IPTVDSLethlinkIid.idx[0] = 6;
			if(zcfgFeObjJsonGet(RDM_OID_ETH_LINK, &IPTVDSLethlinkIid,&IPTVDSLethlinkobj) != ZCFG_SUCCESS){
				printf("cannot get RDM_OID_ETH_LINK obj");
			}
			IID_INIT(IPTVWANethlinkIid);
			IPTVWANethlinkIid.level = 1;
			IPTVWANethlinkIid.idx[0] = 7;
			if(zcfgFeObjJsonGet(RDM_OID_ETH_LINK, &IPTVWANethlinkIid,&IPTVWANethlinkobj) != ZCFG_SUCCESS){
				printf("cannot get RDM_OID_ETH_LINK obj");
			}
			IID_INIT(MANAGEMENTethlinkIid);
			MANAGEMENTethlinkIid.level = 1;
			MANAGEMENTethlinkIid.idx[0] = 9;
			if(zcfgFeObjJsonGet(RDM_OID_ETH_LINK, &MANAGEMENTethlinkIid,&MANAGEMENTethlinkobj) != ZCFG_SUCCESS){
				printf("cannot get RDM_OID_ETH_LINK obj");
			}
#ifdef ZYXEL_WAN_MAC
			currIPTVDSLWanMacType = json_object_get_string(json_object_object_get(IPTVDSLethlinkobj, "X_ZYXEL_WanMacType"));
			currIPTVWANWanMacType = json_object_get_string(json_object_object_get(IPTVWANethlinkobj, "X_ZYXEL_WanMacType"));
			currMANAGEMENTWanMacType = json_object_get_string(json_object_object_get(MANAGEMENTethlinkobj, "X_ZYXEL_WanMacType"));
			if(index == 4){
				if(WanMacType != NULL && strcmp(WanMacType, currIPTVDSLWanMacType)){
					WanMacType = currIPTVDSLWanMacType;
					json_object_object_add(Jobj, "WanMacType", json_object_new_string(WanMacType));
				}
			}
			else if(index == 5){
				if(WanMacType != NULL && strcmp(WanMacType, currIPTVWANWanMacType)){
					WanMacType = currIPTVWANWanMacType;
					json_object_object_add(Jobj, "WanMacType", json_object_new_string(WanMacType));
				}
			}
			else if(index == 7){
				if(WanMacType != NULL && strcmp(WanMacType, currMANAGEMENTWanMacType)){
					WanMacType = currMANAGEMENTWanMacType;
					json_object_object_add(Jobj, "WanMacType", json_object_new_string(WanMacType));
				}
			}
#endif
			zcfgFeJsonObjFree(IPTVDSLethlinkobj);
			zcfgFeJsonObjFree(IPTVWANethlinkobj);
			zcfgFeJsonObjFree(MANAGEMENTethlinkobj);

			if(CurrIpStatic != ipStatic){
				ipStatic = CurrIpStatic;
				json_object_object_add(Jobj, "ipStatic", json_object_new_boolean(ipStatic));
			}	
		}

		if(!strcmp(Mode, "IP_Routed")){		
			if(index == 1){
				IID_INIT(InternetDSLVLANTermIid);
				InternetDSLVLANTermIid.level = 1;
				InternetDSLVLANTermIid.idx[0] = 1;
				if(zcfgFeObjJsonGet(RDM_OID_ETH_VLAN_TERM, &InternetDSLVLANTermIid,&InternetDSLVLANTermobj) != ZCFG_SUCCESS){
					printf("cannot get RDM_OID_ETH_VLAN_TERM obj");
				}
				//currInternetDSLVlanEnable = json_object_get_boolean(json_object_object_get(InternetDSLVLANTermobj, "X_ZYXEL_VLANEnable"));
				currInternetDSLVlanID = json_object_get_int(json_object_object_get(InternetDSLVLANTermobj, "VLANID"));
				currInternetDSLVlanPri =  json_object_get_int(json_object_object_get(InternetDSLVLANTermobj, "X_ZYXEL_VLANPriority"));

				if(VlanID != -1){
					if(VlanID != currInternetDSLVlanID || VlanPri != currInternetDSLVlanPri){
						VlanID = currInternetDSLVlanID;
						VlanPri = currInternetDSLVlanPri;	
						json_object_object_add(Jobj, "VLANID", json_object_new_int(VlanID));
						json_object_object_add(Jobj, "VLANPriority", json_object_new_int(VlanPri));
					}
				}
				zcfgFeJsonObjFree(InternetDSLVLANTermobj);
			}
			else if(index == 2){
				IID_INIT(InternetWANVLANTermIid);
				InternetWANVLANTermIid.level = 1;
				InternetWANVLANTermIid.idx[0] = 2;
				if(zcfgFeObjJsonGet(RDM_OID_ETH_VLAN_TERM, &InternetWANVLANTermIid,&InternetWANVLANTermobj) != ZCFG_SUCCESS){
					printf("cannot get RDM_OID_ETH_VLAN_TERM obj");
				}
				//currInternetWANVlanEnable = json_object_get_boolean(json_object_object_get(InternetWANVLANTermobj, "X_ZYXEL_VLANEnable"));
				currInternetWANVlanID = json_object_get_int(json_object_object_get(InternetWANVLANTermobj, "VLANID"));
				currInternetWANVlanPri =  json_object_get_int(json_object_object_get(InternetWANVLANTermobj, "X_ZYXEL_VLANPriority"));
				if(VlanID != -1){
					if(VlanID != currInternetWANVlanID || VlanPri != currInternetWANVlanPri){
						VlanID = currInternetWANVlanID;
						VlanPri = currInternetWANVlanPri;						
						json_object_object_add(Jobj, "VLANID", json_object_new_int(VlanID));
						json_object_object_add(Jobj, "VLANPriority", json_object_new_int(VlanPri));
					}
				}
				zcfgFeJsonObjFree(InternetWANVLANTermobj);
			}
			else if(index == 4){
				IID_INIT(IPTVDSLVLANTermIid);
				IPTVDSLVLANTermIid.level = 1;
				IPTVDSLVLANTermIid.idx[0] = 4;
				if(zcfgFeObjJsonGet(RDM_OID_ETH_VLAN_TERM, &IPTVDSLVLANTermIid,&IPTVDSLVLANTermobj) != ZCFG_SUCCESS){
					printf("cannot get RDM_OID_ETH_VLAN_TERM obj");
				}
				//currIPTVDSLVlanEnable = json_object_get_boolean(json_object_object_get(IPTVDSLVLANTermobj, "X_ZYXEL_VLANEnable"));
				currIPTVDSLVlanID = json_object_get_int(json_object_object_get(IPTVDSLVLANTermobj, "VLANID"));
				currIPTVDSLVlanPri =  json_object_get_int(json_object_object_get(IPTVDSLVLANTermobj, "X_ZYXEL_VLANPriority"));
				if(VlanID != -1){
					if(VlanID != currIPTVDSLVlanID || VlanPri != currIPTVDSLVlanPri){
						VlanID = currIPTVDSLVlanID;
						VlanPri = currIPTVDSLVlanPri;	
						json_object_object_add(Jobj, "VLANID", json_object_new_int(VlanID));
						json_object_object_add(Jobj, "VLANPriority", json_object_new_int(VlanPri));
					}
				}
				zcfgFeJsonObjFree(IPTVDSLVLANTermobj);
			}
			else if(index == 5){
				IID_INIT(IPTVWANVLANTermIid);
				IPTVWANVLANTermIid.level = 1;
				IPTVWANVLANTermIid.idx[0] = 5;
				if(zcfgFeObjJsonGet(RDM_OID_ETH_VLAN_TERM, &IPTVWANVLANTermIid,&IPTVWANVLANTermobj) != ZCFG_SUCCESS){
					printf("cannot get RDM_OID_ETH_VLAN_TERM obj");
				}
				//currIPTVWANVlanEnable = json_object_get_boolean(json_object_object_get(IPTVWANVLANTermobj, "X_ZYXEL_VLANEnable"));
				currIPTVWANVlanID = json_object_get_int(json_object_object_get(IPTVWANVLANTermobj, "VLANID"));
				currIPTVWANVlanPri =  json_object_get_int(json_object_object_get(IPTVWANVLANTermobj, "X_ZYXEL_VLANPriority"));
				if(VlanID != -1){
					if(VlanID != currIPTVWANVlanID || VlanPri != currIPTVWANVlanPri){
						VlanID = currIPTVWANVlanID;
						VlanPri = currIPTVWANVlanPri;	
						json_object_object_add(Jobj, "VLANID", json_object_new_int(VlanID));
						json_object_object_add(Jobj, "VLANPriority", json_object_new_int(VlanPri));
					}
				}
				zcfgFeJsonObjFree(IPTVWANVLANTermobj);
			}
			else if(index == 7){
				IID_INIT(MANAGEMENTVLANTermIid);
				MANAGEMENTVLANTermIid.level = 1;
				MANAGEMENTVLANTermIid.idx[0] = 7;
				if(zcfgFeObjJsonGet(RDM_OID_ETH_VLAN_TERM, &MANAGEMENTVLANTermIid,&MANAGEMENTVLANTermobj) != ZCFG_SUCCESS){
					printf("cannot get RDM_OID_ETH_VLAN_TERM obj");
				}
				//currMANAGEMENTVlanEnable = json_object_get_boolean(json_object_object_get(MANAGEMENTVLANTermobj, "X_ZYXEL_VLANEnable"));
				currMANAGEMENTVlanID = json_object_get_int(json_object_object_get(MANAGEMENTVLANTermobj, "VLANID"));
				currMANAGEMENTVlanPri =  json_object_get_int(json_object_object_get(MANAGEMENTVLANTermobj, "X_ZYXEL_VLANPriority"));
				if(VlanID != -1){
					if(VlanID != currMANAGEMENTVlanID || VlanPri != currMANAGEMENTVlanPri){
						VlanID = currMANAGEMENTVlanID;
						VlanPri = currMANAGEMENTVlanPri;
						json_object_object_add(Jobj, "VLANID", json_object_new_int(VlanID));
						json_object_object_add(Jobj, "VLANPriority", json_object_new_int(VlanPri));
					}
				}
				zcfgFeJsonObjFree(MANAGEMENTVLANTermobj);
			}
		}

		ipDnsStatic = json_object_get_boolean(json_object_object_get(Jobj, "ipDnsStatic"));
		if(ipDnsStatic != false){
			ipDnsStatic = false;
			json_object_object_add(Jobj, "ipDnsStatic", json_object_new_boolean(ipDnsStatic));
		}
	}

	return ret;
}	
#endif

void deleteUnnecessaryObj(){

	bool delPpp = false, delVlanEthLink = false;

	dbg_printf("CurrMode=%s, Mode=%s\n", CurrMode, Mode);

	if(!strcmp(CurrMode, "IP_Routed") && !strcmp(Mode, "IP_Bridged")){
		if(strstr(CurrEncapsulation, "PPP") || strstr(CurrEncapsulation, "PPtP"))
			delPpp = true;
	}
	else if(!strcmp(CurrMode, "IP_Routed") && !strcmp(Mode, "IP_Routed")){
		if((!strcmp(CurrEncapsulation, "PPPoA") || !strcmp(CurrEncapsulation, "PPPoE") || !strcmp(CurrEncapsulation, TR181_PPtP2PPPOA)) &&
		   (!strcmp(Encapsulation, "IPoA") || !strcmp(Encapsulation, "IPoE"))){
		   delPpp = true;
		}
		if((!strcmp(CurrEncapsulation, "PPPoE") || !strcmp(CurrEncapsulation, "IPoE")) &&
		   (!strcmp(Encapsulation, "PPPoA") || !strcmp(Encapsulation, "IPoA") || !strcmp(Encapsulation, TR181_PPtP2PPPOA))){
		   delVlanEthLink = true;
		}
	}
	else if(!strcmp(CurrMode, "IP_Bridged") && !strcmp(Mode, "IP_Routed")){
		if(!strcmp(Encapsulation, "PPPoA") || !strcmp(Encapsulation, "IPoA") || !strcmp(Encapsulation, TR181_PPtP2PPPOA))
			delVlanEthLink = true;
	}

	if(delPpp){
		dbg_printf("delPpp\n");
		json_object_object_add(ipIfaceObj, "X_ZYXEL_AccessDescription", json_object_new_string("SetIpIfaceStackModified"));
		zcfgFeObjJsonDel(RDM_OID_PPP_IFACE, &pppIfaceIid, NULL);
		zcfgFeJsonObjFree(pppIfaceObj);
	}

	if(delVlanEthLink){
		dbg_printf("delVlanEthLink\n");
		json_object_object_add(ipIfaceObj, "X_ZYXEL_AccessDescription", json_object_new_string("SetIpIfaceStackModified"));
		zcfgFeObjJsonDel(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, NULL);
		zcfgFeObjJsonDel(RDM_OID_ETH_LINK, &ethLinkIid, NULL);
		zcfgFeJsonObjFree(vlanTermObj);
		zcfgFeJsonObjFree(ethLinkObj);
	}
	
}


/*
The same type of WAN interface can be added up to 8.
* Call getWanLanList return Jarray:
*    Jarray.i.Type : LAN|WAN
*    Jarray.i.LinkType = ATM|PTM|ETH|PON|USB (For WAN only)
* Inuput Parameter:
*    currObj.Type : ATM|PTM|ETH (WAN type be checked)
*    isAdd : 0 for edit WAN, 1 for add new WAN
*/
bool checkWanInterfaceLimit(struct json_object * currObj, struct json_object *wanLanList, int isAdd)
{
    struct json_object *listObj = NULL;

    int i, wanIntfCount = 0, len = 0;
    int MaxSameWANInterface = WAN_INTERFACES;


    len = json_object_array_length(wanLanList);
	if(json_object_object_get(currObj, "Type")){
	    for(i=0; i<len; i++){
	        listObj = json_object_array_get_idx(wanLanList, i);
	        if(!strcmp("WAN",json_object_get_string(json_object_object_get(listObj, "Type"))))
	        {
	            if(!strcmp(json_object_get_string(json_object_object_get(currObj, "Type")),json_object_get_string(json_object_object_get(listObj, "LinkType"))))
	            {
	                wanIntfCount ++;
	            }
	        }
	    }
	}
    if(isAdd)
        wanIntfCount++;
    
    if(MaxSameWANInterface < wanIntfCount)
    {
        dbg_printf("Type %s WAN interface count would be %d, over threshold %d\n",json_object_get_string(json_object_object_get(currObj, "Type")), wanIntfCount,MaxSameWANInterface);
        return false;
    }
    return true;
}

#ifdef ACAU_CUSTOMIZATION
void modifyAllPPPInterfaceUserPassword (void)
{
	objIndex_t pppIid = {0};
	struct json_object *pppObj = NULL;
	const char *lowerLayers = NULL, *userName = NULL;
	char *pIgnoreUserName = "wifihomespot"; // Cannot modify PPPoE interface. It is request from TE.

	IID_INIT(pppIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_PPP_IFACE, &pppIid, &pppObj) == ZCFG_SUCCESS)
	{
		lowerLayers = Jgets(pppObj, "LowerLayers");
		userName = Jgets(pppObj, "Username");

		if ( !(strstr (lowerLayers, "USB")) && !(strstr (userName, pIgnoreUserName)) && (pppIid.idx[0] != pppIfaceIid.idx[0]) )
		{
			json_object_object_add(pppObj, "Username", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Username")));
			json_object_object_add(pppObj, "Password", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Password")));

			zcfgFeObjJsonSet(RDM_OID_PPP_IFACE, &pppIid, pppObj, NULL);
		}

		zcfgFeJsonObjFree (pppObj);
	}

	return;
}
#endif

bool isVlanRepeat(char *type, struct json_object *Jobj, struct json_object *wanLanList, int isAdd)
{
    struct json_object *wanLanObj = NULL;
	struct json_object *curWanObj = NULL;
    int i, len = 0;
	const char *LinkType, *VpiVci, *Name, *LinkMode;
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
	const char *Encapsulation, *Mode;
#endif
	int VLANID;

	/*support duplicate vlan by config. start*/
	objIndex_t iid;
	//struct json_object *obj = NULL;
	bool checkDuplicateVlan = true;
	IID_INIT(iid);
	iid.idx[0] = 0;
	if(!featureFlagObj && ZCFG_SUCCESS==zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FEATURE_FLAG, &iid, &featureFlagObj)){
		checkDuplicateVlan = json_object_get_boolean(json_object_object_get(featureFlagObj, "checkDuplicateVlan"));
		if(false ==checkDuplicateVlan){
			//if(obj) zcfgFeJsonObjFree(obj);
			return false;
		}
	}
	//if(obj) zcfgFeJsonObjFree(obj);
	/*support duplicate vlan by config. end*/

	Name = Jgets(Jobj, "Name");
	if(Name == NULL)
		return false;
	
	if(isAdd){
		if((LinkType = Jgets(Jobj, "Type")) == NULL)
			LinkType = "";
		if((VpiVci = Jgets(Jobj, "vpivci")) == NULL)
			VpiVci = "";
		if((LinkMode = Jgets(Jobj,"Mode")) == NULL)
			LinkMode = "";
		if(Jget(Jobj, "VLANID"))
			VLANID = Jgeti(Jobj, "VLANID");
		else
			VLANID = -1;
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
		if((Mode = Jgets(Jobj, "Mode")) == NULL)
			Mode ="IP_Routed";
		if((Encapsulation = Jgets(Jobj, "Encapsulation")) == NULL)
			Encapsulation = "IPoE";
#endif
	}
	else{
		len = json_object_array_length(wanLanList);
		for(i=0; i<len; i++){
			wanLanObj = json_object_array_get_idx(wanLanList, i);
			if(!strcmp(Name, Jgets(wanLanObj, "Name")) && !strcmp("WAN", Jgets(wanLanObj, "Type"))){
				curWanObj = wanLanObj;
				break;
			}
		}
		if(curWanObj == NULL)
			return false;

		if((LinkType = Jgets(Jobj, "Type")) == NULL)
			LinkType = Jgets(curWanObj, "LinkType");
		if((VpiVci = Jgets(Jobj, "vpivci")) == NULL)
			VpiVci = Jgets(curWanObj, "VpiVci");
		if((LinkMode = Jgets(Jobj,"Mode")) == NULL)
			LinkMode = Jgets(curWanObj, "Mode");
		if(Jget(Jobj, "VLANID"))
			VLANID = Jgeti(Jobj, "VLANID");
		else
			VLANID = Jgeti(curWanObj, "VLANID");
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
		if((Mode = Jgets(Jobj, "Mode")) == NULL)
			Mode = Jgets(curWanObj, "ConnectionType");
		if((Encapsulation = Jgets(Jobj, "Encapsulation")) == NULL)
			Encapsulation = Jgets(curWanObj, "Encapsulation");
#endif
	}

	if(LinkType != NULL)
		ZOS_STRCPY_M(type, LinkType);

	len = json_object_array_length(wanLanList);
	for(i=0; i<len; i++){
		wanLanObj = json_object_array_get_idx(wanLanList, i);
		if(strcmp("WAN", Jgets(wanLanObj, "Type")) || !strcmp(Name, Jgets(wanLanObj, "Name"))){
			continue;
		}
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
		if(Jgets(wanLanObj, "LinkType") && !strcmp("USB", Jgets(wanLanObj, "LinkType"))){
			continue;
		}

		if(!strcmp(LinkType, Jgets(wanLanObj, "LinkType")) && (VLANID == Jgeti(wanLanObj, "VLANID"))&&!strcmp(Encapsulation, Jgets(wanLanObj, "Encapsulation"))&& !strcmp(Mode,Jgets(wanLanObj, "ConnectionType"))){

#else		
		if(!strcmp(LinkType, Jgets(wanLanObj, "LinkType")) && (VLANID == Jgeti(wanLanObj, "VLANID")) && !strcmp(LinkMode, Jgets(wanLanObj, "ConnectionType"))){
#endif
#ifdef ZYXEL_USA_PRODUCT
			if(strcmp(LinkType, "ATM")&& (VLANID !=-1))
#else
 			if(strcmp(LinkType, "ATM"))
#endif
			{
				if(!strcmp(LinkType, "CELL"))
					return false;
				else
 					return true;
			}
#ifdef ZYXEL_USA_PRODUCT
			else if(!strcmp(VpiVci, Jgets(wanLanObj, "VpiVci"))&& strcmp(VpiVci,""))
#else
 			else if(!strcmp(VpiVci, Jgets(wanLanObj, "VpiVci")))
#endif	
				return true;
		}
	}

	return false;
}

zcfgRet_t zcfgFeCheckIPDNS(struct json_object *Jobj, char *replyMsg);
/*
 *  input: Jobj
 *  output: _retJobj in Jobj
 *             replyMsg
*/
zcfgRet_t zcfgFeDalWanAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *wanLanList = NULL, *wanLanObj = NULL;
	int i = 0, len = 0, wanNum = 0;
	char type[16] = {0};

	isAdd = true;
	isDelete = false;
	initGlobalObjects();
	getBasicInfo(Jobj);

#if defined(SAME_FW_FOR_TWO_BOARD)
    if (strcmp(Type, "ETH") == 0)
    {
        /* Not support ETHWAN */
        if (zyutil_cofw_modelNameGet() == MODEL_VMG1312_T20A)
        {
            return ZCFG_REQUEST_REJECT;
        }
    }
#endif

	getWanLanList(&wanLanList);

	//check if duplicate Name exist
	len = json_object_array_length(wanLanList);
	for(i=0; i<len; i++){
		wanLanObj = json_object_array_get_idx(wanLanList, i);
		if(!strcmp(Name, Jgets(wanLanObj, "Name")) && !strcmp("WAN", Jgets(wanLanObj, "Type"))){
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_name"));
			if(replyMsg)
				ZOS_STRCPY_M(replyMsg,"Name is duplicate.");

			json_object_put(wanLanList);
			return ZCFG_REQUEST_REJECT;
		}

		if(strcmp("WWAN", Jgets(wanLanObj, "Name")) != 0 && !strcmp("WAN", Jgets(wanLanObj, "Type")))
		{
			wanNum++;
		}
	}

	if(wanNum >= WAN_INTERFACES)
	{
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Broadband.Broadband.warning.exceed_wan_limit"));
		if(replyMsg)
			ZOS_STRCPY_M(replyMsg,"The number of WAN interfaces exceeds the limit.");

		json_object_put(wanLanList);
		return ZCFG_REQUEST_REJECT;
	}

	if(checkWanInterfaceLimit(Jobj, wanLanList, 1) == false){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Broadband.Broadband.warning.exceed_wan_limit"));
		if(replyMsg)
			ZOS_STRCPY_M(replyMsg,"The number of WAN interfaces exceeds the limit.");

		json_object_put(wanLanList);
		return ZCFG_REQUEST_REJECT;
	}

	if(isVlanRepeat(type, Jobj, wanLanList, 1)){
		if(!strcmp(type, "ATM"))
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Broadband.Broadband.warning.vlan_pvc_repeat"));
		else
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Broadband.Broadband.warning.vlan_repeat"));
		if(replyMsg)
			ZOS_STRCPY_M(replyMsg,"The Vlan is repeat.");

		json_object_put(wanLanList);
		return ZCFG_REQUEST_REJECT;
	}
	json_object_put(wanLanList);
	
	ret = zcfgFeCheckIPDNS(Jobj, replyMsg);
	if((ret = addDefaultParam(Jobj, replyMsg)) != ZCFG_SUCCESS)//for CLI
		return ret;

	if((ret = editPhyLayerObjects(Jobj)) != ZCFG_SUCCESS)
		return ret;

	if((ret = editEthLinkObject(Jobj)) != ZCFG_SUCCESS)
		return ret;

	if((ret = editVlanTermObject(Jobj)) != ZCFG_SUCCESS)
		return ret;

	if((ret = addLayer3Objects(Jobj)) != ZCFG_SUCCESS)
		return ret;
	setAllObjects(Jobj);
#ifdef ZYXEL_SYNC_CCWAN_TO_ALL_DEFGW
	syncCCWanSetting(Jobj);
#endif
	freeAllObjects();
	
	return ret;
}

zcfgRet_t objectStepStackLowerLayer(const char *startLayer, const char *destLowLayer, char *higherLayer, char *lowerLayer)
{
	objIndex_t iid;
	zcfg_offset_t oid;
	const char *ptr = NULL;
	struct json_object *tmpJobj = NULL;
	char tr181Name[64] = {0};
	char buf[64];
	char *bufptr;

	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	if(!startLayer || !destLowLayer || !higherLayer)
		return ZCFG_INTERNAL_ERROR;

	higherLayer[0] = '\0';

	ZOS_STRNCPY(tr181Name, startLayer, sizeof(tr181Name));
	IID_INIT(iid);
	oid = zcfgFeObjNameToObjId(tr181Name, &iid);
	if((int)oid < 0)
		return ZCFG_NO_SUCH_OBJECT;

	while(zcfgFeObjJsonGetWithoutUpdate(oid, &iid, &tmpJobj) == ZCFG_SUCCESS){
		ptr = json_object_get_string(json_object_object_get(tmpJobj, "LowerLayers"));

		if (ptr == NULL || !strcmp(ptr, "")){
			json_object_put(tmpJobj);
			return ZCFG_NO_SUCH_OBJECT;
		}

		//prevent multi lowerlayers case
		ZOS_STRNCPY(buf, ptr, sizeof(buf));
		bufptr = strchr(buf, ',');
		if(bufptr)
			*bufptr = '\0';
		ZOS_STRCPY_M(lowerLayer, buf);

		json_object_put(tmpJobj);

		if(!strcmp(lowerLayer, destLowLayer)){
			ZOS_STRCPY_M(higherLayer, tr181Name);
			return ZCFG_SUCCESS;
		}
		else{
			ZOS_STRCPY_M(tr181Name, lowerLayer);
			IID_INIT(iid);
			oid = zcfgFeObjNameToObjId(tr181Name, &iid);
			if((int)oid < 0)
				return ZCFG_NO_SUCH_OBJECT;
		}
	}

	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t configEthIfaceWan(struct json_object *Jobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char higherLayers[64] = {0}, lowerLayers[64] = {0};
	bool enable = json_object_get_boolean(json_object_object_get(Jobj, (const char *)"EnableEthWAN"));
	const char *ethIfaceName = json_object_get_string(json_object_object_get(Jobj, (const char *)"EthIfaceName"));
	struct json_object *ipIntObj = NULL, *ethIntfObj = NULL;
	objIndex_t ipIntIid, ethIntfIid;

	if(!ethIfaceName || !strstr(ethIfaceName, "Ethernet")){
		return ZCFG_INVALID_OBJECT;
	}

	dbg_printf("%s: enable/disable %d EthWAN %s\n", __FUNCTION__, (int)enable, ethIfaceName);

	IID_INIT(ethIntfIid);
	ethIntfIid.level = 1;
	sscanf(ethIfaceName, "Ethernet.Interface.%hhu", &ethIntfIid.idx[0]);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_IFACE, &ethIntfIid, &ethIntfObj)) != ZCFG_SUCCESS)
		return ret;

	json_object_object_add(ethIntfObj, "X_ZYXEL_Upstream", json_object_new_boolean(enable));
	if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_ETH_IFACE, &ethIntfIid, ethIntfObj, NULL)) != ZCFG_SUCCESS){
		json_object_put(ethIntfObj);
		return ret;
	}
	json_object_put(ethIntfObj);

	if(enable != true)
		return ZCFG_SUCCESS;

	IID_INIT(ipIntIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIntIid, &ipIntObj) == ZCFG_SUCCESS) {
		const char *ipIfaceLowerLayers = json_object_get_string(json_object_object_get(ipIntObj, "LowerLayers"));
		if(ipIfaceLowerLayers && strlen(ipIfaceLowerLayers)){
			if(objectStepStackLowerLayer(ipIfaceLowerLayers, ethIfaceName, higherLayers, lowerLayers) == ZCFG_SUCCESS){
				break;
			}
			json_object_put(ipIntObj);
			ipIntObj = NULL;
		}
	}

	if(ipIntObj){
		json_object_object_add(ipIntObj, "X_ZYXEL_AccessDescription", json_object_new_string("SetIpIfaceStackModified"));
		if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_IP_IFACE, &ipIntIid, ipIntObj, NULL)) != ZCFG_SUCCESS){
			json_object_put(ipIntObj);
			return ret;
		}
		json_object_put(ipIntObj);
	}

	return ZCFG_SUCCESS;
}


zcfgRet_t zcfgFeDalWanEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *wanLanList = NULL;
	char type[16] = {0};
	
	isAdd = false;
	isDelete = false;
	initGlobalObjects();
	getBasicInfo(Jobj);

	/*
	if(json_object_object_get(Jobj, "EnableEthWAN")){
		// it's EthWAN enable/disable
		if(configEthIfaceWan(Jobj) == ZCFG_SUCCESS)
			zcfgFeReqReboot(NULL);

		goto dalwan_edit_fail;
	}
	else
	*/
	if(0==isPppQuick)
	{
		ret = zcfgFeCheckIPDNS(Jobj, replyMsg);

	
		getWanLanList(&wanLanList);
		if(checkWanInterfaceLimit(Jobj, wanLanList, 0) == false){
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Broadband.Broadband.warning.exceed_wan_limit"));
			if(replyMsg)
				ZOS_STRCPY_M(replyMsg,"The number of WAN interfaces exceeds the limit.");
			json_object_put(wanLanList);
			ret = ZCFG_REQUEST_REJECT;
			goto dalwan_edit_fail;
		}
		if(isVlanRepeat(type, Jobj, wanLanList, 0)){
			if(!strcmp(type, "ATM"))
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Broadband.Broadband.warning.vlan_pvc_repeat"));
			else
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Broadband.Broadband.warning.vlan_repeat"));
			if(replyMsg)
				ZOS_STRCPY_M(replyMsg,"The Vlan is repeat.");
			json_object_put(wanLanList);
			ret = ZCFG_REQUEST_REJECT;
			goto dalwan_edit_fail;
		}
		json_object_put(wanLanList);
	}
	if ((ret = getCurrentConfig(Jobj)) != ZCFG_SUCCESS)
		goto dalwan_edit_fail;

#ifdef MSTC_TAAAG_MULTI_USER
	if ((ret = UnnecessarymodifyObj(Jobj)) != ZCFG_SUCCESS)
		goto dalwan_edit_fail;
#endif

	//disable ipIface pppIface first
	CurrEnable = json_object_get_boolean(json_object_object_get(ipIfaceObj, "Enable"));
	json_object_object_add(ipIfaceObj, "Enable", json_object_new_boolean(false));
	zcfgFeObjJsonBlockedSet(RDM_OID_IP_IFACE, &ipIfaceIid, ipIfaceObj, NULL);

	if(pppIfaceObj){
		json_object_object_add(pppIfaceObj, "Enable", json_object_new_boolean(false));
		zcfgFeObjJsonSet(RDM_OID_PPP_IFACE, &pppIfaceIid, pppIfaceObj, NULL);
	}

	//Always clean dynamic dns
	if(dynamicDnsClientSrv4Obj){
		json_object_object_add(dynamicDnsClientSrv4Obj, "Interface", json_object_new_string(""));
		json_object_object_add(dynamicDnsClientSrv4Obj, "X_ZYXEL_Type", json_object_new_string("Static"));
		json_object_object_add(dynamicDnsClientSrv4Obj, "DNSServer", json_object_new_string(""));
		json_object_object_add(dynamicDnsClientSrv4Obj, "X_ZYXEL_Ifname", json_object_new_string(""));
		json_object_object_add(dynamicDnsClientSrv4Obj, "X_ZYXEL_GwAddr", json_object_new_string(""));
		zcfgFeObjJsonSet(RDM_OID_DNS_CLIENT_SRV, &dynamicDnsClientSrv4Iid, dynamicDnsClientSrv4Obj, NULL);
		zcfgFeJsonObjFree(dynamicDnsClientSrv4Obj);
	}
	if(dynamicDnsClientSrv6Obj){
		json_object_object_add(dynamicDnsClientSrv6Obj, "Interface", json_object_new_string(""));
		json_object_object_add(dynamicDnsClientSrv6Obj, "X_ZYXEL_Type", json_object_new_string("Static"));
		json_object_object_add(dynamicDnsClientSrv6Obj, "DNSServer", json_object_new_string(""));
		json_object_object_add(dynamicDnsClientSrv6Obj, "X_ZYXEL_Ifname", json_object_new_string(""));
		json_object_object_add(dynamicDnsClientSrv6Obj, "X_ZYXEL_GwAddr", json_object_new_string(""));
		zcfgFeObjJsonSet(RDM_OID_DNS_CLIENT_SRV, &dynamicDnsClientSrv6Iid, dynamicDnsClientSrv6Obj, NULL);
		zcfgFeJsonObjFree(dynamicDnsClientSrv6Obj);
	}

/* __ZYXEL__, Chi-Hsiang When WAN IPv6 address mode change from dynamic ip to static ip,
	may encounter some data model error that BE has deleted RDM_OID_ROUTING_ROUTER_V6_FWD
	and RDM_OID_IP_IFACE_V6_ADDR, which results in setAllObjects() couldn't find curObj to set.
	Sympton: atfer set IPv6 address from dynamic ip to static and apply, web GUI still shows
	dynamic ip.
*/
	if(ip6Static){
		zcfgFeObjJsonDel(RDM_OID_ROUTING_ROUTER_V6_FWD, &v6FwdIid, NULL);
		zcfgFeJsonObjFree(v6FwdObj);
		v6FwdObj = NULL;
		
		if(!CurrIp6Static){
			zcfgFeObjJsonDel(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, NULL);
			zcfgFeJsonObjFree(v6AddrObj);
			v6AddrObj = NULL;
		}
	}

	if(!ip6Static){
		if(CurrIp6Static){
			zcfgFeObjJsonDel(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, NULL);
		}
	}
	
	if((ip6DnsStatic)){
		zcfgFeObjJsonDel(RDM_OID_DNS_CLIENT_SRV, &dynamicDnsClientSrv6Iid, NULL);
		zcfgFeJsonObjFree(dynamicDnsClientSrv6Obj);
		dynamicDnsClientSrv6Obj = NULL;
	}
	deleteUnnecessaryObj();
	if((ret = editPhyLayerObjects(Jobj)) != ZCFG_SUCCESS)
		goto dalwan_edit_fail;
	if((ret = editEthLinkObject(Jobj)) != ZCFG_SUCCESS)
		goto dalwan_edit_fail;

	if((ret = editVlanTermObject(Jobj)) != ZCFG_SUCCESS)
		goto dalwan_edit_fail;
	
	if((ret = editLayer3Objects(Jobj)) != ZCFG_SUCCESS)
		goto dalwan_edit_fail;

	setAllObjects(Jobj);

#ifdef ACAU_CUSTOMIZATION
	if(pppIfaceObj)
	{
		modifyAllPPPInterfaceUserPassword ();
	}
#endif

#ifdef ZYXEL_SYNC_CCWAN_TO_ALL_DEFGW
	syncCCWanSetting(Jobj);
#endif

	zcfgFeLanCheckWanDhcpServer(NULL);

dalwan_edit_fail:
	freeAllObjects();
	
	return ret;
}

zcfgRet_t zcfgFeDalWanDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	isDelete = true;
	initGlobalObjects();
	getBasicInfo(Jobj);
	getCurrentConfig(Jobj);

	if(ipIfaceObj)
		zcfgFeObjJsonDel(RDM_OID_IP_IFACE, &ipIfaceIid, NULL);

	if(pppIfaceObj)
		zcfgFeObjJsonDel(RDM_OID_PPP_IFACE, &pppIfaceIid, NULL);

	if(vlanTermObj)
		zcfgFeObjJsonDel(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, NULL);

	if(ethLinkObj)
		zcfgFeObjJsonDel(RDM_OID_ETH_LINK, &ethLinkIid, NULL);

	/*if(ptmLinkObj){
		if(!isXTMLinkHasReference(Jobj))
			zcfgFeObjJsonDel(RDM_OID_PTM_LINK, &ptmLinkIid, NULL);
	}*/

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	if(atmLinkObj){
		if(!isXTMLinkHasReference(Jobj))
			zcfgFeObjJsonDel(RDM_OID_ATM_LINK, &atmLinkIid, NULL);
	}
#endif
	freeAllObjects();
	zcfgFeLanCheckWanDhcpServer(NULL);
	return ret;
}

zcfgRet_t parseIpIfaceObj(struct json_object *pramJobj,objIndex_t ipIfaceIid, struct json_object *ipIfaceObj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *HigherLayer;
	const char *LowerLayer;
	int count = 0;

	sprintf(ipIfacePath, "IP.Interface.%u", ipIfaceIid.idx[0]);
	//printf("%s: enter , ipIfacePath = %s\n",__FUNCTION__,ipIfacePath);

	LowerLayer = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
	ZOS_STRCPY_M(CurrMode, json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionType")));
	
		HigherLayer = ipIfacePath;
		while(count < 4){
			//printf("count=%d\n", count);
			//printf("HigherLayer=%s\n", HigherLayer);
			//printf("LowerLayer=%s\n", LowerLayer);
			
			count++;

			if ((HigherLayer==NULL) || (LowerLayer==NULL)){
				return ZCFG_REQUEST_REJECT;
			}			
			
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
					ZOS_STRCPY_M(CurrEncapsulation, "IPoE");
				}
				else if(!strncmp("Ethernet.Link.", LowerLayer, 14)){
					ethLinkIid.level = 1;
					sscanf(LowerLayer, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkObj);
					HigherLayer = LowerLayer;
					LowerLayer = json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
					ZOS_STRCPY_M(CurrEncapsulation, "IPoE");
				}
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
				else if(!strncmp("ATM.Link.", LowerLayer, 9) && !strcmp("IP_Routed", CurrMode)){
					atmLinkIid.level = 1;
					sscanf(LowerLayer, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj);
					ZOS_STRCPY_M(CurrEncapsulation, "IPoA");
					ZOS_STRCPY_M(CurrType, "ATM");
					memcpy(&atmLinkQosIid, &atmLinkIid, sizeof(objIndex_t));
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK_QOS, &atmLinkQosIid, &atmLinkQosObj);
					break;
				}
#endif
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
					ZOS_STRCPY_M(CurrEncapsulation, "PPPoE");
				}
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
				else if(!strncmp("ATM.Link.", LowerLayer, 9)){
					atmLinkIid.level = 1;
					sscanf(LowerLayer, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj);
#ifdef ZYXEL_PPTPD_RELAY
					ZOS_STRCPY_M(CurrEncapsulation, json_object_get_string(json_object_object_get(atmLinkObj, "LinkType")));
#else
					ZOS_STRCPY_M(CurrEncapsulation, "PPPoA");
#endif
					ZOS_STRCPY_M(CurrType, "ATM");
					memcpy(&atmLinkQosIid, &atmLinkIid, sizeof(objIndex_t));
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK_QOS, &atmLinkQosIid, &atmLinkQosObj);
					break;
				}
#endif
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
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
				if(!strncmp("ATM.Link.", LowerLayer, 9)){
					atmLinkIid.level = 1;
					sscanf(LowerLayer, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj);
					ZOS_STRCPY_M(CurrType, "ATM");
					memcpy(&atmLinkQosIid, &atmLinkIid, sizeof(objIndex_t));
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK_QOS, &atmLinkQosIid, &atmLinkQosObj);
					break;
				}
				else
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
				if(!strncmp("PTM.Link.", LowerLayer, 9)){
					ptmLinkIid.level = 1;
					sscanf(LowerLayer, "PTM.Link.%hhu", &ptmLinkIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PTM_LINK, &ptmLinkIid, &ptmLinkObj);
					ZOS_STRCPY_M(CurrType, "PTM");
					break;
				}
				else
#endif
				if(!strncmp("Ethernet.Interface.", LowerLayer, 19)){
					ethIfaceIid.level = 1;
					sscanf(LowerLayer, "Ethernet.Interface.%hhu", &ethIfaceIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_IFACE, &ethIfaceIid, &ethIfaceObj);
					if(json_object_get_boolean(json_object_object_get(ethIfaceObj, "X_ZYXEL_SFP")) == false)
					ZOS_STRCPY_M(CurrType, "ETH");
					else
						ZOS_STRCPY_M(CurrType, "SFP");
					if(json_object_get_boolean(json_object_object_get(ethIfaceObj, "Upstream")) == false){
						if(json_object_get_boolean(json_object_object_get(ethLinkObj, "X_ZYXEL_OrigEthWAN")) == false){
							return ZCFG_REQUEST_REJECT;
						}
					}
					break;
				}
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
				else if(!strncmp("Optical.Interface.", LowerLayer, 18)){
					optIfaceIid.level = 1;
					sscanf(LowerLayer, "Optical.Interface.%hhu", &optIfaceIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_OPT_INTF, &optIfaceIid, &optIfaceObj);
					if(json_object_get_boolean(json_object_object_get(optIfaceObj, "Upstream")) == false){
						printf("The LowerLayer Optical Interface is a LAN interface.\n");
						return ZCFG_REQUEST_REJECT;
					}
					ZOS_STRCPY_M(CurrType, "PON");
					break;
				}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
				else if(!strncmp("Cellular.Interface.", LowerLayer, 19)){
					cellIfaceIid.level = 1;
					sscanf(LowerLayer, "Cellular.Interface.%hhu", &cellIfaceIid.idx[0]);
					zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF, &cellIfaceIid, &cellIfaceObj);
					if(json_object_get_boolean(json_object_object_get(cellIfaceObj, "Upstream")) == false){
						printf("The LowerLayer Cellular Interface is a LAN interface.\n");
						return ZCFG_REQUEST_REJECT;
					}
					ZOS_STRCPY_M(CurrType, "CELL");
					break;
				}
#endif
				else if(!strcmp("", LowerLayer)){
					if(json_object_get_boolean(json_object_object_get(ethLinkObj, "X_ZYXEL_OrigEthWAN")) == false){
						return ZCFG_REQUEST_REJECT;
					}
					ZOS_STRCPY_M(CurrType, "ETH");
					break;
				}
				else{
					return ZCFG_REQUEST_REJECT;
				}
			}
		}

	//Basic param
	json_object_object_add(pramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_SrvName")));
	json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "Enable")));
	json_object_object_add(pramJobj, "Type", json_object_new_string(CurrType));
	json_object_object_add(pramJobj, "Mode", json_object_new_string(CurrMode));
	json_object_object_add(pramJobj, "VLANID", JSON_OBJ_COPY(json_object_object_get(vlanTermObj, "VLANID")));
	json_object_object_add(pramJobj, "VLANEnable", JSON_OBJ_COPY(json_object_object_get(vlanTermObj, "X_ZYXEL_VLANEnable")));
	json_object_object_add(pramJobj, "VLANPriority", JSON_OBJ_COPY(json_object_object_get(vlanTermObj, "X_ZYXEL_VLANPriority")));
	json_object_object_add(pramJobj, "Iid", json_object_new_int(ipIfaceIid.idx[0]));
	if(vlanTermObj == NULL){
		json_object_object_add(pramJobj, "VLANID", json_object_new_int(-1));
		json_object_object_add(pramJobj, "VLANPriority", json_object_new_int(-1));
	}

    /* __ZyXEL__, Albert, 20180209,Support IPv6 option IA_NA and IA_PD  */
	json_object_object_add(pramJobj, "IPv6Manual_IANA", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_AdvManagedFlag_Manual")));
	json_object_object_add(pramJobj, "IPv6Manual_IAPD", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_AdvOtherConfigFlag_Manual")));
#ifdef ZYXEL_WAN_DHCPV6_MODE
	IPv6Manual_IANA = json_object_get_boolean(json_object_object_get(ipIfaceObj, "X_ZYXEL_AdvManagedFlag_Manual"));
#endif
#ifdef ZYXEL_PD_TYPE
	json_object_object_add(pramJobj, "ip6PrefixEnable", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_AdvOtherConfigFlag_Manual")));
	if(!strcmp(json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_IPv6Origin")),TR181_PREFIX_STATIC))
	{
		json_object_object_add(pramJobj, "ip6StaticPrefix", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_IPv6Prefix")));
		json_object_object_add(pramJobj, "ip6PrefixStatic", json_object_new_boolean(true));
	}
	else{
		json_object_object_add(pramJobj, "ip6PrefixStatic", json_object_new_boolean(false));
	}
#endif
#ifdef ZYXEL_TAAAG_FIREWALL_WANINTERFACE
	json_object_object_add(pramJobj, "FirewallEnable", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_Enable_Firewall")));
#endif

	if(!strcmp(CurrType, "ATM")){
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
		json_object_object_add(pramJobj, "vpivci", JSON_OBJ_COPY(json_object_object_get(atmLinkObj, "DestinationAddress")));
		json_object_object_add(pramJobj, "AtmEncapsulation", JSON_OBJ_COPY(json_object_object_get(atmLinkObj, "Encapsulation")));
		json_object_object_add(pramJobj, "QoSClass", JSON_OBJ_COPY(json_object_object_get(atmLinkQosObj, "QoSClass")));
		json_object_object_add(pramJobj, "atmMaxBurstSize", JSON_OBJ_COPY(json_object_object_get(atmLinkQosObj, "MaximumBurstSize")));
		json_object_object_add(pramJobj, "atmSustainedCellRate",JSON_OBJ_COPY(json_object_object_get(atmLinkQosObj, "SustainableCellRate")));
		json_object_object_add(pramJobj, "atmPeakCellRate", JSON_OBJ_COPY(json_object_object_get(atmLinkQosObj, "PeakCellRate")));
#endif
	}

#ifdef ZYXEL_WAN_MAC
	if( strcmp(CurrEncapsulation, "PPPoA") && strcmp(CurrEncapsulation, "IPoA")){   //Not PPPoA and not IPoA
		if(!strcmp(CurrType,"ETH") || !strcmp(CurrType,"ATM") || !strcmp(CurrType,"PTM") ){
			json_object_object_add(pramJobj, "WanMacType", JSON_OBJ_COPY(json_object_object_get(ethLinkObj, "X_ZYXEL_WanMacType")));
			json_object_object_add(pramJobj, "WanMacAddr", JSON_OBJ_COPY(json_object_object_get(ethLinkObj, "X_ZYXEL_WanMacAddr")));
			json_object_object_add(pramJobj, "WanMacIP", JSON_OBJ_COPY(json_object_object_get(ethLinkObj, "X_ZYXEL_WanMacClientIP")));
		}
	}
#endif

	return ret;
}

static zcfgRet_t getWanInfo(struct json_object *pramJobj,objIndex_t ipIfaceIid, struct json_object *ipIfaceObj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	bool v4enable, v6enable;
#ifdef ABUE_CUSTOMIZATION
	bool Enable_DSLite;
#endif
#ifdef ZyXEL_WEB_GUI_SHOW_IGMP_VERSION
	int igmpVersion = 0;
#endif
	const char *Interface;
	const char *pppPassTh;
	bool enable_6RD = false;
	char v6RD_Type[16] = {0};
	const char *lanport = NULL;
	bool found = false;
	char tmpStr[MAX_INTF_PATH_LENGTH] = {0};

	if(!strcmp(CurrEncapsulation, "IPoE") || !strcmp(CurrEncapsulation, "IPoA")){
		v4enable = json_object_get_boolean(json_object_object_get(ipIfaceObj, "IPv4Enable"));
		v6enable = json_object_get_boolean(json_object_object_get(ipIfaceObj, "IPv6Enable"));
		json_object_object_add(pramJobj, "MaxMTUSize", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "MaxMTUSize")));
	}
	else{ //PPPoE or PPPoA
		v4enable = json_object_get_boolean(json_object_object_get(pppIfaceObj, "IPCPEnable"));
		v6enable = json_object_get_boolean(json_object_object_get(pppIfaceObj, "IPv6CPEnable"));
		pppPassTh = json_object_get_string(json_object_object_get(pppIfaceObj, "X_ZYXEL_ConnectionType"));

		if(!strcmp(pppPassTh, "PPPoE_Bridged"))	
			json_object_object_add(pramJobj, "pppoePassThrough",json_object_new_boolean(true));
		else
			json_object_object_add(pramJobj, "pppoePassThrough",json_object_new_boolean(false));
		
		json_object_object_add(pramJobj, "MaxMTUSize", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "MaxMRUSize")));
		json_object_object_add(pramJobj, "IdleDisconnectTime",JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "IdleDisconnectTime")));
		json_object_object_add(pramJobj, "pppUsername", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Username")));
		json_object_object_add(pramJobj, "pppPassword", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Password")));
		json_object_object_add(pramJobj, "ConnectionTrigger", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "ConnectionTrigger")));
#ifdef ZYXEL_PPPoE_SERVICE_OPTION
		json_object_object_add(pramJobj, "pppServicename", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "X_ZYXEL_ServiceName")));
#endif

	}
#ifdef ABUE_CUSTOMIZATION
	Enable_DSLite = json_object_get_boolean(json_object_object_get(ipIfaceObj, "X_ZYXEL_Enable_DSLite"));
	if(v4enable && v6enable && !Enable_DSLite)
		ZOS_STRCPY_M(CurrIpMode, "DualStack");
	else if(v4enable && v6enable && Enable_DSLite)
		ZOS_STRCPY_M(CurrIpMode, "AutoConfig");
#else
	if(v4enable && v6enable)
		ZOS_STRCPY_M(CurrIpMode, "DualStack");
#endif	
	else if(v4enable)
		ZOS_STRCPY_M(CurrIpMode, "IPv4");
	else if(v6enable)
		ZOS_STRCPY_M(CurrIpMode, "IPv6");

	json_object_object_add(pramJobj, "Encapsulation", json_object_new_string(CurrEncapsulation));
	json_object_object_add(pramJobj, "ipMode", json_object_new_string(CurrIpMode));
	json_object_object_add(pramJobj, "ConcurrentWan", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConcurrentWan")));
	json_object_object_add(pramJobj, "Enable_6RD",json_object_new_boolean(false));
	
#if 0
	if(!strcmp(CurrEncapsulation, "PPPoE") || !strcmp(CurrEncapsulation, "PPPoA")){
		if(!strcmp(CurrIpMode, "IPv4")){
			enable_6RD = json_object_get_boolean(json_object_object_get(ipIfaceObj, "X_ZYXEL_Enable_6RD"));
			ZOS_STRCPY_M(v6RD_Type, json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_6RD_Type")));
		}
		if(enable_6RD && !strcmp(v6RD_Type, "static")){
			json_object_object_add(pramJobj, "Enable_6RD",json_object_new_boolean(enable_6RD));
			json_object_object_add(pramJobj, "v6RD_Type", json_object_new_string(v6RD_Type));
			json_object_object_add(pramJobj, "SPIPv6Prefix", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_SPIPv6Prefix")));
			json_object_object_add(pramJobj, "IPv4MaskLength", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_IPv4MaskLength")));
			json_object_object_add(pramJobj, "BorderRelayIPv4Addresses", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_BorderRelayIPv4Addresses")));
		}
	}
#endif

	if(v4ClientObj) zcfgFeJsonObjFree(v4ClientObj);
	getSpecificObj(RDM_OID_DHCPV4_CLIENT, "Interface", json_type_string, ipIfacePath, &v4ClientIid, &v4ClientObj);
	if(v4ClientObj){
		getDhcpv4ClientOpt(&v4ClientIid);
	}
	
	if(!strcmp(CurrIpMode, "IPv4")){
		enable_6RD = json_object_get_boolean(json_object_object_get(ipIfaceObj, "X_ZYXEL_Enable_6RD"));
		json_object_object_add(pramJobj, "Enable_6RD",json_object_new_boolean(enable_6RD));
	}
	if(enable_6RD){
		ZOS_STRCPY_M(v6RD_Type, json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_6RD_Type")));
		json_object_object_add(pramJobj, "v6RD_Type", json_object_new_string(v6RD_Type));
		if(!strcmp(v6RD_Type, "static")){
			json_object_object_add(pramJobj, "SPIPv6Prefix", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_SPIPv6Prefix")));
			json_object_object_add(pramJobj, "IPv4MaskLength", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_IPv4MaskLength")));
			json_object_object_add(pramJobj, "BorderRelayIPv4Addresses", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_BorderRelayIPv4Addresses")));
		}
	}
	
	
	if(!strcmp(CurrIpMode, "IPv6")){
		json_object_object_add(pramJobj, "DSLiteEnable",JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_Enable_DSLite")));
		json_object_object_add(pramJobj, "DSLiteType",JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_DSLite_Type")));
		json_object_object_add(pramJobj, "DSLiteRelayIPv6Addresses",JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_DSLiteRelayIPv6Addresses")));

		//MAP
		if(mapObj == NULL)
		{
			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MAP, &mapIid, &mapObj) == ZCFG_SUCCESS)
				json_object_object_add(pramJobj, "mapEnable",JSON_OBJ_COPY(json_object_object_get(mapObj, "Enable")));
		}
		if(domainObj == NULL)
		{
			snprintf(tmpStr, MAX_INTF_PATH_LENGTH, "Device.%s",  ipIfacePath);
			IID_INIT(domainIid);
			while(!found && zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MAP_DOMAIN, &domainIid, &domainObj) == ZCFG_SUCCESS)
			{
				if(strcmp(tmpStr, json_object_get_string(json_object_object_get(domainObj, "WANInterface"))) == 0){
					found = true;

					json_object_object_add(pramJobj, "TransportMode",JSON_OBJ_COPY(json_object_object_get(domainObj, "TransportMode")));
					json_object_object_add(pramJobj, "BRIPv6Prefix",JSON_OBJ_COPY(json_object_object_get(domainObj, "BRIPv6Prefix")));
					json_object_object_add(pramJobj, "PSIDOffset",JSON_OBJ_COPY(json_object_object_get(domainObj, "PSIDOffset")));
					json_object_object_add(pramJobj, "PSIDLength",JSON_OBJ_COPY(json_object_object_get(domainObj, "PSIDLength")));
					json_object_object_add(pramJobj, "PSID",JSON_OBJ_COPY(json_object_object_get(domainObj, "PSID")));
					json_object_object_add(pramJobj, "Softwire46Enable",JSON_OBJ_COPY(json_object_object_get(domainObj, "X_ZYXEL_Softwire46Enable")));

					ZLOG_DEBUG("%s : Domain %s PSIDOffset=%lld PSIDLength=%lld PSID=%lld S46=%d\n", __FUNCTION__
						, json_object_get_string(json_object_object_get(domainObj, "BRIPv6Prefix"))
						, json_object_get_int(json_object_object_get(domainObj, "PSIDOffset"))
						, json_object_get_int(json_object_object_get(domainObj, "PSIDLength"))
						, json_object_get_int(json_object_object_get(domainObj, "PSID"))
						, json_object_get_boolean(json_object_object_get(domainObj, "X_ZYXEL_Softwire46Enable")) );
					break ;
				}
				zcfgFeJsonObjFree(domainObj);
			}
		}
		if(ruleObj == NULL)
		{
			IID_INIT(ruleIid);
			if(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_MAP_DOMAIN_RULE, &domainIid, &ruleIid, &ruleObj) == ZCFG_SUCCESS)
			{
				json_object_object_add(pramJobj, "IPv6Prefix",JSON_OBJ_COPY(json_object_object_get(ruleObj, "IPv6Prefix")));
				json_object_object_add(pramJobj, "IPv4Prefix",JSON_OBJ_COPY(json_object_object_get(ruleObj, "IPv4Prefix")));

				ZLOG_DEBUG("%s : Rule %s %s \n", __FUNCTION__
					, json_object_get_string(json_object_object_get(ruleObj, "IPv6Prefix"))
					, json_object_get_string(json_object_object_get(ruleObj, "IPv4Prefix")) );
			}
		}
		//MAP end
	}
#ifdef ABUE_CUSTOMIZATION	
	if(!strcmp(CurrIpMode, "AutoConfig")){
		json_object_object_add(pramJobj, "DSLiteEnable",JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_Enable_DSLite")));
		json_object_object_add(pramJobj, "DSLiteType",JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_DSLite_Type")));
		json_object_object_add(pramJobj, "DSLiteRelayIPv6Addresses",JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_DSLiteRelayIPv6Addresses")));
	}
#endif
	//NAT
	if(natIntfObj) zcfgFeJsonObjFree(natIntfObj);
	if (getSpecificObj(RDM_OID_NAT_INTF_SETTING, "Interface", json_type_string, ipIfacePath, &natIntfIid, &natIntfObj)) {
		json_object_object_add(pramJobj, "NatEnable",JSON_OBJ_COPY(json_object_object_get(natIntfObj, "Enable")));
		json_object_object_add(pramJobj, "FullConeEnabled",JSON_OBJ_COPY(json_object_object_get(natIntfObj, "X_ZYXEL_FullConeEnabled")));
	}
	else{
		Jaddb(pramJobj, "NatEnable", false);
		Jaddb(pramJobj, "FullConeEnabled", false);
	}

	if(routerObj) zcfgFeJsonObjFree(routerObj);
	//Default Gateway
	IID_INIT(routerIid);
	routerIid.level = 1;
	routerIid.idx[0] = 1;
	zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ROUTING_ROUTER, &routerIid, &routerObj);
	Interface = routerObj ? json_object_get_string(json_object_object_get(routerObj, "X_ZYXEL_ActiveDefaultGateway")) : NULL;
	if(Interface && strstr(Interface, ipIfacePath))
		json_object_object_add(pramJobj, "sysGwEnable", json_object_new_boolean(true));
	else	
		json_object_object_add(pramJobj, "sysGwEnable", json_object_new_boolean(false));

	
	Interface = routerObj ? json_object_get_string(json_object_object_get(routerObj, "X_ZYXEL_ActiveV6DefaultGateway")) : NULL;
	if(Interface && strstr(Interface, ipIfacePath))
		json_object_object_add(pramJobj, "sysGw6Enable", json_object_new_boolean(true));
	else
		json_object_object_add(pramJobj, "sysGw6Enable", json_object_new_boolean(false));

	//IGMP
	IID_INIT(igmpIid);
	if(igmpObj) zcfgFeJsonObjFree(igmpObj);
	zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IGMP, &igmpIid, &igmpObj);
	Interface = igmpObj ? json_object_get_string(json_object_object_get(igmpObj, "Interface")) : NULL;
#ifdef ZyXEL_WEB_GUI_SHOW_IGMP_VERSION		
	igmpVersion = json_object_get_int(json_object_object_get(igmpObj, "IgmpVersion"));
	json_object_object_add(pramJobj, "IGMPVersion", json_object_new_int(igmpVersion));
#endif	
	if(Interface && strstr(Interface, ipIfacePath))
		json_object_object_add(pramJobj, "IGMPEnable", json_object_new_boolean(true));
	else
		json_object_object_add(pramJobj, "IGMPEnable", json_object_new_boolean(false));

#ifdef ACAU_CUSTOMIZATION /* Egypt TE */
		char path[64];
		char c;
		FILE *fp;

		snprintf(path, sizeof(path)-1, "/var/snoop%d", ipIfaceIid.idx[0]);
		fp = fopen(path, "r");
		if (fp)
		{
			c = fgetc(fp);
			fclose(fp);
			fp = NULL;

			if (c == '1')
				json_object_object_add(pramJobj, "IGMPSnooping", json_object_new_boolean(true));
			else
				json_object_object_add(pramJobj, "IGMPSnooping", json_object_new_boolean(false));
		}
		else
		{
			if(strstr(Interface, ipIfacePath))
				json_object_object_add(pramJobj, "IGMPSnooping", json_object_new_boolean(true));
			else
				json_object_object_add(pramJobj, "IGMPSnooping", json_object_new_boolean(false));
		}
#endif

	//MLD
	IID_INIT(mldIid);
	if(mldObj) zcfgFeJsonObjFree(mldObj);
	zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MLD, &mldIid, &mldObj);
	Interface = mldObj ? json_object_get_string(json_object_object_get(mldObj, "Interface")) : NULL;
	if(Interface && strstr(Interface, ipIfacePath))
		json_object_object_add(pramJobj, "MLDEnable", json_object_new_boolean(true));
	else
		json_object_object_add(pramJobj, "MLDEnable", json_object_new_boolean(false));

	//Wan Port Check	
	IID_INIT(ethIfaceIid);
	ethIfaceIid.level = 1;
	ethIfaceIid.idx[0] = 4;
	if(ethIfaceObj) zcfgFeJsonObjFree(ethIfaceObj);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_IFACE, &ethIfaceIid, &ethIfaceObj)== ZCFG_SUCCESS){
		lanport = json_object_get_string(json_object_object_get(ethIfaceObj, "X_ZYXEL_LanPort"));
		if(!strcmp(lanport,"ETHWAN")){
			json_object_object_add(pramJobj, "LantoWan", json_object_new_boolean(true));
		}
		else{
			json_object_object_add(pramJobj, "LantoWan", json_object_new_boolean(false));
		}
	}
	
	dbg_printf("====testH %s: exit\n",__FUNCTION__);
	return ret;
}

zcfgRet_t getIpGatewayDnsInfo(struct json_object *pramJobj,objIndex_t ipIfaceIid, struct json_object *ipIfaceObj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t iid;
	bool Enable;
	const char *Interface;
	const char *AddressingType;
	const char *Origin;
	//bool staticRoute = false;
	const char *DNSServer;
	const char *X_ZYXEL_Type;
	const char *X_ZYXEL_LocalIPAddress;
	const char *GatewayIPAddress;
	const char *DestIPAddress;
	const char *DestIPPrefix;
	const char *value = NULL;
	const char *DevOUI;
	const char *DevSN;
	const char *DevPClass;
	bool ipStatic = false;
	//bool ip6Static = false;
	bool ipDnsStatic = false;
	bool ip6DnsStatic = false;
	int tag = 0;
	bool opt = false;
	
	objIndex_t featureFlagIid;
	IID_INIT(featureFlagIid);
	if(!featureFlagObj)
		zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FEATURE_FLAG, &featureFlagIid, &featureFlagObj);

	//Default
	json_object_object_add(pramJobj, "ipStatic", json_object_new_boolean(false));
	json_object_object_add(pramJobj, "ip6Static", json_object_new_boolean(false));
	json_object_object_add(pramJobj, "option42Enable", json_object_new_boolean(false));
	json_object_object_add(pramJobj, "option43Enable", json_object_new_boolean(false));
	json_object_object_add(pramJobj, "option120Enable", json_object_new_boolean(false));
	json_object_object_add(pramJobj, "option121Enable", json_object_new_boolean(false));
	json_object_object_add(pramJobj, "option60Enable", json_object_new_boolean(false));
	json_object_object_add(pramJobj, "option61Enable", json_object_new_boolean(false));
	json_object_object_add(pramJobj, "option125Enable", json_object_new_boolean(false));

	Jadds(pramJobj, "IPAddress", "");
	Jadds(pramJobj, "SubnetMask", "");
	Jadds(pramJobj, "GatewayIPAddress", "");
	Jadds(pramJobj, "DNSServer", "");
	Jadds(pramJobj, "IP6Address", "");
	Jadds(pramJobj, "NextHop", "");
	Jadds(pramJobj, "DNS6Server", "");

	//ipv4 Addr
	IID_INIT(v4AddrIid);
	v4AddrIid.level = 2;
	v4AddrIid.idx[0] = ipIfaceIid.idx[0];
	v4AddrIid.idx[1] = 1;
	if(v4AddrObj) zcfgFeJsonObjFree(v4AddrObj);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj);
	if(ret == ZCFG_SUCCESS && v4AddrObj){
		AddressingType = json_object_get_string(json_object_object_get(v4AddrObj, "AddressingType"));
		Enable = json_object_get_boolean(json_object_object_get(v4AddrObj, "Enable"));
        if(Enable == true){
			replaceParam(pramJobj, "IPAddress", v4AddrObj, "IPAddress");
			replaceParam(pramJobj, "SubnetMask", v4AddrObj, "SubnetMask");
    		if(!strcmp(AddressingType, "Static") && strstr(CurrEncapsulation, "IP")){
    			ipStatic = true;
    			json_object_object_add(pramJobj, "ipStatic", json_object_new_boolean(true));
    		}
        }
	}

	if(ipStatic)
	{
		//GatewayIPAddress
		IID_INIT(v4FwdIid);
		if(v4FwdObj) zcfgFeJsonObjFree(v4FwdObj);
		while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, &routerIid, &v4FwdIid, &v4FwdObj) == ZCFG_SUCCESS){
			Interface = json_object_get_string(json_object_object_get(v4FwdObj, "Interface"));
			DestIPAddress = Jgets(v4FwdObj, "DestIPAddress");
			//staticRoute =  json_object_get_boolean(json_object_object_get(v4FwdObj, "StaticRoute"));
			if(!strcmp(Interface, ipIfacePath) && !strcmp(DestIPAddress, "")){
				GatewayIPAddress= json_object_get_string(json_object_object_get(v4FwdObj, "GatewayIPAddress"));
				json_object_object_add(pramJobj, "GatewayIPAddress", json_object_new_string(GatewayIPAddress));
				zcfgFeJsonObjFree(v4FwdObj);
				break;
			}
			zcfgFeJsonObjFree(v4FwdObj);
		}
	}else{
		//GatewayIPAddress
		IID_INIT(v4FwdIid);
		if(v4FwdObj) zcfgFeJsonObjFree(v4FwdObj);
		while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, &routerIid, &v4FwdIid, &v4FwdObj) == ZCFG_SUCCESS){
			Interface = json_object_get_string(json_object_object_get(v4FwdObj, "Interface"));
			DestIPAddress = Jgets(v4FwdObj, "DestIPAddress");
			if(!strcmp(Interface, ipIfacePath) && !strcmp(DestIPAddress, "")){
				GatewayIPAddress= json_object_get_string(json_object_object_get(v4FwdObj, "GatewayIPAddress"));
				json_object_object_add(pramJobj, "GatewayIPAddress", json_object_new_string(GatewayIPAddress));
				zcfgFeJsonObjFree(v4FwdObj);
				break;
			}
			zcfgFeJsonObjFree(v4FwdObj);
		}

		const char *dhcpOption61Mode = featureFlagObj ? json_object_get_string(json_object_object_get(featureFlagObj, "DHCPOption61Mode")) : NULL;

		//DHCP REQ OPT
		IID_INIT(iid);
		while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_DHCPV4_REQ_OPT, &v4ClientIid, &iid, &obj) == ZCFG_SUCCESS){
			tag = json_object_get_int(json_object_object_get(obj, "Tag"));
			opt = json_object_get_boolean(json_object_object_get(obj, "Enable"));
			value = json_object_get_string(json_object_object_get(obj, "Value"));
			if(tag == 42 ){
				json_object_object_add(pramJobj, "option42Enable", json_object_new_boolean(opt));
				if(opt)
					json_object_object_add(pramJobj, "option42Value", json_object_new_string(value));
			}
			else if(tag == 43){
				json_object_object_add(pramJobj, "option43Enable", json_object_new_boolean(opt));
			}else if(tag == 120 ){
				json_object_object_add(pramJobj, "option120Enable", json_object_new_boolean(opt));
			}else if(tag == 121 ){
				json_object_object_add(pramJobj, "option121Enable", json_object_new_boolean(opt));
			}
			zcfgFeJsonObjFree(obj);
   		}
		//DHCP SENT OPT
		IID_INIT(iid);
		while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_DHCPV4_SENT_OPT, &v4ClientIid, &iid, &obj) == ZCFG_SUCCESS){
			tag = json_object_get_int(json_object_object_get(obj, "Tag"));
			opt = json_object_get_boolean(json_object_object_get(obj, "Enable"));
			value = json_object_get_string(json_object_object_get(obj, "Value"));
			if(tag == 60 ){
				json_object_object_add(pramJobj, "option60Enable", json_object_new_boolean(opt));
				if(strlen(value)>=4){
					char decVal[256]={0};
					opt60ValDecode((char *)value,decVal);
					json_object_object_add(pramJobj, "option60Value", json_object_new_string(decVal));
				}
				else{
					json_object_object_add(pramJobj, "option60Value", json_object_new_string(""));
				}
			}else if(tag == 61 ){
				json_object_object_add(pramJobj, "option61Enable", json_object_new_boolean(opt));
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
				char iaidVal[256] = {0};
				int duidVal = 0;
				int enterpriseVal = 0;
				char identifierVal[256]={0};
				if(opt){
					opt61ValDecode(value,&iaidVal,&duidVal,&enterpriseVal,identifierVal);
				}
				json_object_object_add(pramJobj, "option61Iaid", json_object_new_string(iaidVal));
				if (duidVal)
					json_object_object_add(pramJobj, "option61Duid", json_object_new_int(duidVal));
				if (enterpriseVal)
					json_object_object_add(pramJobj, "option61Enterprise", json_object_new_int(enterpriseVal));
				json_object_object_add(pramJobj, "option61Identifier", json_object_new_string(identifierVal));
#else //CONFIG_ABZQ_CUSTOMIZATION

				if(dhcpOption61Mode && !strcmp(dhcpOption61Mode, "ADDRDOMAIN")){
					char addrDomain[256] = {0};

					json_object_object_add(pramJobj, "option61Value", json_object_new_string(addrDomain));
				}else{ // default
					char iaidVal[256]={0};
					char duidVal[256]={0};
					if(opt){ //opt61ValDecode
						strncpy(iaidVal,value+6,8); 
						ZOS_STRNCPY(duidVal,value+18,sizeof(duidVal));
					}
					json_object_object_add(pramJobj, "option61Iaid", json_object_new_string(iaidVal));
					json_object_object_add(pramJobj, "option61Duid", json_object_new_string(duidVal));
				}
#endif //CONFIG_ABZQ_CUSTOMIZATION
			}else if(tag == 125 ){
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
				char cOUI[256] = {0};
				char cClass[256] = {0};
				char cModel[256] = {0};
				char cSN[256] = {0};
#endif
				json_object_object_add(pramJobj, "option125Enable", json_object_new_boolean(opt));
#if defined (CONFIG_ABZQ_CUSTOMIZATION)
				if ( strlen(value) != 0 )
				{
					opt125ValDecode (value,&cOUI,&cClass,&cModel,cSN);
				}

				json_object_object_add(pramJobj, "option125ManufacturerOUI", json_object_new_string(cOUI));
				json_object_object_add(pramJobj, "option125ProductClass", json_object_new_string(cClass));
				json_object_object_add(pramJobj, "option125ModelName", json_object_new_string(cModel));
				json_object_object_add(pramJobj, "option125SerialNumber", json_object_new_string(cSN));
#else
				json_object_object_add(pramJobj, "option125Value", json_object_new_string(value));
#endif //CONFIG_ABZQ_CUSTOMIZATION
			}
			zcfgFeJsonObjFree(obj);
		}                                                                                                
	}

	//static ppp
	if(strstr(CurrEncapsulation, "PPP") && pppIfaceObj != NULL){
		X_ZYXEL_LocalIPAddress = json_object_get_string(json_object_object_get(pppIfaceObj, "X_ZYXEL_LocalIPAddress"));
		if(strcmp(X_ZYXEL_LocalIPAddress, "0.0.0.0") && strcmp(X_ZYXEL_LocalIPAddress, "")){
			CurrPppStatic = true;
			json_object_object_add(pramJobj, "IPAddress", json_object_new_string(X_ZYXEL_LocalIPAddress));
			json_object_object_add(pramJobj, "ipStatic", json_object_new_boolean(true));
		}
	}
	//static ipv6 Addr
	IID_INIT(v6AddrIid);
	v6AddrIid.level = 2;
	v6AddrIid.idx[0] = ipIfaceIid.idx[0];
	v6AddrIid.idx[1] = 1;
	if(v6AddrObj) zcfgFeJsonObjFree(v6AddrObj);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, &v6AddrObj);
	if(ret == ZCFG_SUCCESS && v6AddrObj){
		Origin = json_object_get_string(json_object_object_get(v6AddrObj, "Origin"));
		Enable = json_object_get_boolean(json_object_object_get(v6AddrObj, "Enable"));
		if(Enable == true){
            replaceParam(pramJobj, "IP6Address", v6AddrObj, "IPAddress");
            if(!strcmp(Origin, "Static")){
			    //ip6Static = true;
			    json_object_object_add(pramJobj, "ip6Static", json_object_new_boolean(true));
            }
		}
	}

	IID_INIT(v6FwdIid);
	if(v6FwdObj) zcfgFeJsonObjFree(v6FwdObj);
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &routerIid, &v6FwdIid, &v6FwdObj) == ZCFG_SUCCESS){
		Interface = json_object_get_string(json_object_object_get(v6FwdObj, "Interface"));
		// Origin = json_object_get_string(json_object_object_get(v6FwdObj, "Origin"));
		DestIPPrefix = Jgets(v6FwdObj, "DestIPPrefix");
		if(strstr(CurrEncapsulation, "IP")){
			if(!strcmp(Interface, ipIfacePath) && (!strcmp(DestIPPrefix, "::/0") || !strcmp(DestIPPrefix, ""))){
				json_object_object_add(pramJobj, "NextHop", JSON_OBJ_COPY(json_object_object_get(v6FwdObj, "NextHop")));
				zcfgFeJsonObjFree(v6FwdObj);
				break;
			}
		}
		zcfgFeJsonObjFree(v6FwdObj);
	}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
    objIndex_t tmpiid;
    struct json_object *tmpipIfObj = NULL;
	int upwantype = 0;
	const char *curTAAABSrvName;
	const char *tmpTAAABSrvName;
	char tempipIfacePath[32];
	
    upwantype = UpWanTypeGet();   
	curTAAABSrvName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_TT_SrvName"));
	
	json_object_object_add(pramJobj, "upwantype", json_object_new_int(upwantype));
	
    if(upwantype == 2 && strstr(curTAAABSrvName,"_DSL")!=NULL){
		
        IID_INIT(tmpiid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &tmpiid, &tmpipIfObj) == ZCFG_SUCCESS){
               tmpTAAABSrvName = json_object_get_string(json_object_object_get(tmpipIfObj, "X_TT_SrvName"));
			   if(!strcmp(curTAAABSrvName,tmpTAAABSrvName) && (tmpiid.idx[0] != ipIfaceIid.idx[0])){
                     sprintf(tempipIfacePath, "IP.Interface.%d", tmpiid.idx[0]);
					 zcfgFeJsonObjFree(tmpipIfObj);
					 break;
			   }
			   zcfgFeJsonObjFree(tmpipIfObj);
		}

	//static DNS
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DNS_CLIENT_SRV, &iid, &obj) == ZCFG_SUCCESS){
		Interface = json_object_get_string(json_object_object_get(obj, "Interface"));
		X_ZYXEL_Type = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Type"));
		if(!strcmp(Interface, tempipIfacePath)){
			DNSServer = json_object_get_string(json_object_object_get(obj, "DNSServer"));
			if((strchr(DNSServer, ':') == NULL)) { //&& (dnsClientSrv4Obj == NULL)){
				json_object_object_add(pramJobj, "DNSServer", json_object_new_string(DNSServer));
				json_object_object_add(pramJobj, "ipDnsStatic", json_object_new_boolean(true));
				if(!strcmp(X_ZYXEL_Type, "Static"))
					ipDnsStatic = true;
			}
			else {//if(dnsClientSrv6Obj == NULL){
				json_object_object_add(pramJobj, "DNS6Server", json_object_new_string(DNSServer));
				json_object_object_add(pramJobj, "ip6DnsStatic", json_object_new_boolean(true));
				if(!strcmp(X_ZYXEL_Type, "Static"))
					ip6DnsStatic = true;
			}
		}
		/*else{
			json_object_object_add(pramJobj, "ipDnsStatic", json_object_new_boolean(false));
			json_object_object_add(pramJobj, "ip6DnsStatic", json_object_new_boolean(false));
			zcfgFeJsonObjFree(obj);
		}*/
		zcfgFeJsonObjFree(obj);
	}

	if(ipDnsStatic == false)
	{	json_object_object_add(pramJobj, "ipDnsStatic", json_object_new_boolean(false));   }

	if(ip6DnsStatic == false)
	{	json_object_object_add(pramJobj, "ip6DnsStatic", json_object_new_boolean(false));   }		

	}
	else{
		//static DNS
		IID_INIT(iid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DNS_CLIENT_SRV, &iid, &obj) == ZCFG_SUCCESS){
			Interface = json_object_get_string(json_object_object_get(obj, "Interface"));
			X_ZYXEL_Type = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Type"));
			if(!strcmp(Interface, ipIfacePath)){
				DNSServer = json_object_get_string(json_object_object_get(obj, "DNSServer"));
				if((strchr(DNSServer, ':') == NULL)) { //&& (dnsClientSrv4Obj == NULL)){
					json_object_object_add(pramJobj, "DNSServer", json_object_new_string(DNSServer));
					json_object_object_add(pramJobj, "ipDnsStatic", json_object_new_boolean(true));
					if(!strcmp(X_ZYXEL_Type, "Static"))
						ipDnsStatic = true;
				}
				else {//if(dnsClientSrv6Obj == NULL){
					json_object_object_add(pramJobj, "DNS6Server", json_object_new_string(DNSServer));
					json_object_object_add(pramJobj, "ip6DnsStatic", json_object_new_boolean(true));
					if(!strcmp(X_ZYXEL_Type, "Static"))
						ip6DnsStatic = true;
				}
			}
			/*else{
				json_object_object_add(pramJobj, "ipDnsStatic", json_object_new_boolean(false));
				json_object_object_add(pramJobj, "ip6DnsStatic", json_object_new_boolean(false));
				zcfgFeJsonObjFree(obj);
			}*/
			zcfgFeJsonObjFree(obj);
		}
		
		if(ipDnsStatic == false)
		{	json_object_object_add(pramJobj, "ipDnsStatic", json_object_new_boolean(false));   }

		if(ip6DnsStatic == false)
		{	json_object_object_add(pramJobj, "ip6DnsStatic", json_object_new_boolean(false));	}
	}
#else
	//static DNS
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DNS_CLIENT_SRV, &iid, &obj) == ZCFG_SUCCESS){
		Interface = json_object_get_string(json_object_object_get(obj, "Interface"));
		X_ZYXEL_Type = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Type"));
		if(!strcmp(Interface, ipIfacePath)){
			DNSServer = json_object_get_string(json_object_object_get(obj, "DNSServer"));
			if((strchr(DNSServer, ':') == NULL)) { //&& (dnsClientSrv4Obj == NULL)){
				json_object_object_add(pramJobj, "DNSServer", json_object_new_string(DNSServer));
				json_object_object_add(pramJobj, "ipDnsStatic", json_object_new_boolean(true));
				if(!strcmp(X_ZYXEL_Type, "Static"))
					ipDnsStatic = true;
			}
			else {//if(dnsClientSrv6Obj == NULL){
				json_object_object_add(pramJobj, "DNS6Server", json_object_new_string(DNSServer));
				json_object_object_add(pramJobj, "ip6DnsStatic", json_object_new_boolean(true));
				if(!strcmp(X_ZYXEL_Type, "Static"))
					ip6DnsStatic = true;
			}
		}
		/*else{
			json_object_object_add(pramJobj, "ipDnsStatic", json_object_new_boolean(false));
			json_object_object_add(pramJobj, "ip6DnsStatic", json_object_new_boolean(false));
			zcfgFeJsonObjFree(obj);
		}*/
		zcfgFeJsonObjFree(obj);
	}

	if(ipDnsStatic == false)
	{	json_object_object_add(pramJobj, "ipDnsStatic", json_object_new_boolean(false));   }

	if(ip6DnsStatic == false)
	{	json_object_object_add(pramJobj, "ip6DnsStatic", json_object_new_boolean(false));   }
#endif

    //Dev info
	IID_INIT(iid);
	iid.level = 0;
	iid.idx[0] = 0;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DEV_INFO, &iid, &obj) == ZCFG_SUCCESS){

		DevOUI = json_object_get_string(json_object_object_get(obj, "ManufacturerOUI"));
		DevSN = json_object_get_string(json_object_object_get(obj, "SerialNumber"));
		DevPClass = json_object_get_string(json_object_object_get(obj, "ProductClass"));

		json_object_object_add(pramJobj, "ManufacturerOUI", json_object_new_string(DevOUI));
		json_object_object_add(pramJobj, "SerialNumber", json_object_new_string(DevSN));
		json_object_object_add(pramJobj, "ProductClass", json_object_new_string(DevPClass));

		zcfgFeJsonObjFree(obj);
	}

	return ret;
}

#ifdef ZYXEL_SYNC_CCWAN_TO_ALL_DEFGW
zcfgRet_t restartRunningWan(objIndex_t ccWanIid, struct json_object *ccWanObj){
	zcfgRet_t ret = ZCFG_SUCCESS;

	printf("%s: Restart running wan!\n",__FUNCTION__);
	json_object_object_add(ccWanObj, "Enable", json_object_new_boolean(false));
	zcfgFeObjJsonBlockedSet(RDM_OID_IP_IFACE, &ccWanIid, ccWanObj, NULL);
	json_object_object_add(ccWanObj, "Enable", json_object_new_boolean(true));
	zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ccWanIid, ccWanObj, NULL);

	return ret;
}
zcfgRet_t syncCCWanSetting(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	bool defaultGatewayIface = false;
	int newCCWan = 0, CurCCWan = 0;
	char *status = NULL;
	struct json_object *subIpIfaceObj =   NULL;
	struct json_object *ccWanObj = NULL;
	objIndex_t ccWanIid = {0};
	bool wanEnable = false, reRunningWan = false;
	bool sysGwEnable = false;

	newCCWan = json_object_get_int(json_object_object_get(Jobj, "ConcurrentWan"));
	sysGwEnable = json_object_get_int(json_object_object_get(Jobj, "sysGwEnable"));

	if(!sysGwEnable)
		return ret;

	/*Sync other default WAN*/
	IID_INIT(ccWanIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ccWanIid, &ccWanObj) == ZCFG_SUCCESS){
		status = json_object_get_string(json_object_object_get(ccWanObj, "Status"));
		defaultGatewayIface = json_object_get_boolean(json_object_object_get(ccWanObj, "X_ZYXEL_DefaultGatewayIface"));
		wanEnable = json_object_get_boolean(json_object_object_get(ccWanObj, "Enable"));
		CurCCWan = json_object_get_int(json_object_object_get(ccWanObj, "X_ZYXEL_ConcurrentWan"));

		if(ccWanIid.idx[0] == ipIfaceIid.idx[0] || ccWanIid.idx[0] <=2){
			zcfgFeJsonObjFree(ccWanObj);
			continue;
		}

		replaceParam(ccWanObj, "X_ZYXEL_ConcurrentWan", Jobj, "ConcurrentWan");
		if(defaultGatewayIface && !strcmp("Up", status)){
			if(wanEnable) {//restart running wan!
				restartRunningWan(ccWanIid, ccWanObj);
			}else{
				zcfgFeObjJsonSetWithoutApply(RDM_OID_IP_IFACE, &ccWanIid, ccWanObj, NULL);
			}
		}else if (defaultGatewayIface && strcmp("Up", status)){
			zcfgFeObjJsonSetWithoutApply(RDM_OID_IP_IFACE, &ccWanIid, ccWanObj, NULL);
		}else{
			if(!defaultGatewayIface && CurCCWan !=0){
				json_object_object_add(ccWanObj, "X_ZYXEL_ConcurrentWan", json_object_new_int(0));
				zcfgFeObjJsonSetWithoutApply(RDM_OID_IP_IFACE, &ccWanIid, ccWanObj, NULL);
			}
		}
		zcfgFeJsonObjFree(ccWanObj);
	}

	return ret;
}
#endif
zcfgRet_t zcfgFeDalWanGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int count = 0, index = 0;
#ifdef ABUU_CUSTOMIZATION
	char *ifname = NULL;
#endif
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
    const char *BindName;
    const char *TT_SrvName;
	char *tmp_string = NULL;
#endif
#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER)
	const char *wanname=NULL;
#endif
	struct json_object *pramJobj = NULL;
	struct json_object *ipIfObj = NULL;
	
#ifdef ZYXEL_WAN_DHCPV6_MODE
	const char *Origin;
	struct json_object *v6AddrObj = NULL;
#endif
	int hideEth = 0;

#if defined(SAME_FW_FOR_TWO_BOARD)
	/* Not support ETHWAN */
	if (zyutil_cofw_modelNameGet() == MODEL_VMG1312_T20A)
		hideEth += 1;
#endif
#if defined(ZYXEL_SUPPORT_LAN_AS_ETHWAN)
	struct json_object *jarr = json_object_new_array();
	if (ZCFG_SUCCESS == zcfgFeDalHandler("ethwanlan", "GET", NULL, jarr, NULL))
	{
		struct json_object *jobj = json_object_array_get_idx(jarr, 0);
		if (jobj && !json_object_get_boolean(json_object_object_get(jobj, "Enable")))
			hideEth += 1;
	}
	zcfgFeJsonObjFree(jarr);
#endif

	//initGlobalObjects();

	objIndex_t featureFlagIid;
	IID_INIT(featureFlagIid);
	if(!featureFlagObj)
		zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FEATURE_FLAG, &featureFlagIid, &featureFlagObj);

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfObj) == ZCFG_SUCCESS) {
		
		pramJobj = json_object_new_object();
		ret = parseIpIfaceObj(pramJobj, ipIfaceIid, ipIfObj);
	
		if( ret == ZCFG_SUCCESS )
		{
			count++;
			json_object_object_add(pramJobj, "index", json_object_new_int(ipIfaceIid.idx[0]));
			Jadds(pramJobj, "ipIfacePath", ipIfacePath);
            replaceParam(pramJobj, "X_ZYXEL_IfName", ipIfObj, "X_ZYXEL_IfName");

        	struct json_object *ipIfaceDSCPObj = json_object_object_get(ipIfObj, "X_ZYXEL_DSCPMark");
        	int ipIfaceDSCPMark = ipIfaceDSCPObj ? json_object_get_int(ipIfaceDSCPObj) : -1;
        	json_object_object_add(pramJobj, "DSCPEnable", (ipIfaceDSCPMark >= 0) ? json_object_new_boolean(true) : json_object_new_boolean(false));
        	json_object_object_add(pramJobj, "DSCP", json_object_new_int(ipIfaceDSCPMark));

#ifdef ZYXEL_WAN_DHCPV6_MODE
			IID_INIT(v6AddrIid);
			v6AddrIid.level = 2;
			v6AddrIid.idx[0] = ipIfaceIid.idx[0];
			v6AddrIid.idx[1] = 1;
			if(IPv6Manual_IANA )
				json_object_object_add(pramJobj, "ip6Mode", json_object_new_string("DHCPv6"));
			else{
				if(v6AddrObj) zcfgFeJsonObjFree(v6AddrObj);
				ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, &v6AddrObj);
				if(ret == ZCFG_SUCCESS && v6AddrObj){
					Origin = json_object_get_string(json_object_object_get(v6AddrObj, "Origin"));
					if(!strcmp(Origin,TR181_V6_ADDR_DHCPV6))
						json_object_object_add(pramJobj, "ip6Mode", json_object_new_string("DHCPv6"));
					else if(!strcmp(Origin,TR181_V6_ADDR_AUTO))	
						json_object_object_add(pramJobj, "ip6Mode", json_object_new_string("SLAAC"));
					else
						json_object_object_add(pramJobj, "ip6Mode", json_object_new_string("Static"));
					
					zcfgFeJsonObjFree(v6AddrObj);
				}
			}
#endif

            if (hideEth && !strcmp("ETH", json_object_get_string(json_object_object_get(pramJobj, "Type"))))
            {
				zcfgFeJsonObjFree(pramJobj);
				zcfgFeJsonObjFree(ipIfObj);
				freeAllObjects();
				continue;
            }

			if(strcmp(CurrMode, "IP_Bridged")){
				getWanInfo(pramJobj, ipIfaceIid, ipIfObj);
				getIpGatewayDnsInfo(pramJobj, ipIfaceIid, ipIfObj);
			}else{
				json_object_object_add(pramJobj, "MaxMTUSize", JSON_OBJ_COPY(json_object_object_get(ipIfObj, "MaxMTUSize")));
			}
			if(json_object_object_get(Jobj, "Index")){
				index = json_object_get_int(json_object_object_get(Jobj, "Index"));
				if(index == ipIfaceIid.idx[0]){
					json_object_object_add(pramJobj, "ShowDetail", json_object_new_boolean(true));
					json_object_array_add(Jarray, JSON_OBJ_COPY(pramJobj));
					zcfgFeJsonObjFree(pramJobj);
					zcfgFeJsonObjFree(ipIfObj);
					freeAllObjects();
					break;
				}
			}
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
            else if(json_object_object_get(Jobj, "BindName")){
				BindName = json_object_get_string(json_object_object_get(Jobj, "BindName"));
				TT_SrvName = json_object_get_string(json_object_object_get(ipIfObj, "X_TT_SrvName"));
				if(!strcmp(BindName,TT_SrvName)){
					tmp_string = strrchr(TAAAB_SrvName,'_');
					if(!strcmp(tmp_string,"_DSL"))
						json_object_object_add(pramJobj, "EditType", json_object_new_string("DSL"));
					else if(!strcmp(tmp_string,"_WAN")){
						json_object_object_add(pramJobj, "EditType", json_object_new_string("ETH"));
#ifdef TAAAB_HGW
                    char newName[128] = {0};
					strncpy(newName, TAAAB_SrvName, strlen(TAAAB_SrvName)-strlen(tmp_string));
                    newName[strlen(TAAAB_SrvName)-strlen(tmp_string)] = '\0';
					json_object_object_add(pramJobj, "HGWStyleName", json_object_new_string(newName));
#endif
					}
					json_object_object_add(pramJobj, "TTSrvName", json_object_new_string(TAAAB_SrvName));
					json_object_array_add(Jarray, JSON_OBJ_COPY(pramJobj));
				}
#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER)
				wanname=Jgets(Jobj, "BindName");
				FILE *wannamefs;
				wannamefs = fopen( "/var/wannamefile", "a+");
				if(wannamefs != NULL){
					if(wanname !=NULL){			
						fprintf(wannamefs, "%s\n", wanname);
					}	
					fclose(wannamefs);
				}
#endif
			}
#endif
#ifdef ABUU_CUSTOMIZATION
			else if(json_object_object_get(Jobj, "Name")){
				ifname = (char *)json_object_get_string(json_object_object_get(Jobj, "Name"));
				if(!strcmp(ifname, json_object_get_string(json_object_object_get(ipIfObj, "X_ZYXEL_SrvName")))){
					json_object_object_add(pramJobj, "ShowDetail", json_object_new_boolean(true));
					json_object_array_add(Jarray, JSON_OBJ_COPY(pramJobj));
				}
			}
#endif
            else
            {
			    json_object_array_add(Jarray, JSON_OBJ_COPY(pramJobj));
            }
		}
		zcfgFeJsonObjFree(pramJobj);
		zcfgFeJsonObjFree(ipIfObj);
		freeAllObjects();
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeCheckIPDNS(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *Jarray = NULL, *obj = NULL;
	const char *curname = NULL;
	int len = 0, i;
	bool editmode = false, ipv4static = false, dnsv4static = false, ipv6static = false, dnsv6static = false;
	bool oriipv4static = false,  oriipv6static = false;
	//bool oridnsv4static = false, oridnsv6static = false;

	Jarray = json_object_new_array();
	curname = json_object_get_string(json_object_object_get(Jobj, "Name"));
	ipv4static = json_object_get_boolean(json_object_object_get(Jobj, "ipStatic"));
	dnsv4static = json_object_get_boolean(json_object_object_get(Jobj, "ipDnsStatic"));
	ipv6static = json_object_get_boolean(json_object_object_get(Jobj, "ip6Static"));
	dnsv6static = json_object_get_boolean(json_object_object_get(Jobj, "ip6DnsStatic"));
	zcfgFeDalWanGet(Jobj, Jarray, NULL);
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(!strcmp(curname, json_object_get_string(json_object_object_get(obj, "Name")))){
			oriipv4static = json_object_get_boolean(json_object_object_get(obj, "ipStatic"));
			//oridnsv4static = json_object_get_boolean(json_object_object_get(obj, "ipDnsStatic"));
			oriipv6static = json_object_get_boolean(json_object_object_get(obj, "ip6Static"));
			//oridnsv6static = json_object_get_boolean(json_object_object_get(obj, "ip6DnsStatic"));
			editmode = true;
			break;
		}
	}
	if(editmode){	//edit case
		if(json_object_object_get(Jobj, "ipStatic") && json_object_object_get(Jobj, "ipDnsStatic")){
			if(ipv4static && !dnsv4static){
				if(replyMsg)
					ZOS_STRCPY_M(replyMsg, "--ipDnsStatic should be set as 1.");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
		}
		else if(!json_object_object_get(Jobj, "ipStatic") && json_object_object_get(Jobj, "ipDnsStatic")){
			if(oriipv4static && !dnsv4static){
				if(replyMsg)
					ZOS_STRCPY_M(replyMsg, "--ipDnsStatic should be set as 1.");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
		}

		if(json_object_object_get(Jobj, "ip6Static") && json_object_object_get(Jobj, "ip6DnsStatic")){
			if(ipv6static && !dnsv6static){
				if(replyMsg)
					ZOS_STRCPY_M(replyMsg, "--ip6DnsStatic should be set as 1.");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
		}
		else if(!json_object_object_get(Jobj, "ip6Static") && json_object_object_get(Jobj, "ip6DnsStatic")){
			if(oriipv6static && !dnsv6static){
				if(replyMsg)
					ZOS_STRCPY_M(replyMsg, "--ip6DnsStatic should be set as 1.");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
		}
	}
	else{	//add case
		if(ipv4static){	//for ipv4 case
			if(!json_object_object_get(Jobj, "ipDnsStatic")){
				if(replyMsg)
					ZOS_STRCPY_M(replyMsg, "Please set --ipDnsStatic parameter.");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
			else if(!dnsv4static){
				if(replyMsg)
					ZOS_STRCPY_M(replyMsg, "--ipDnsStatic should be set as 1.");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
		}
		if(ipv6static){	//for ipv6 case
			if(!json_object_object_get(Jobj, "ip6DnsStatic")){
				if(replyMsg)
					ZOS_STRCPY_M(replyMsg, "Please set --i6pDnsStatic parameter.");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
			else if(!dnsv6static){
				if(replyMsg)
					ZOS_STRCPY_M(replyMsg, "--ip6DnsStatic should be set as 1.");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
		}
	}

	zcfgFeJsonObjFree(Jarray);
	return ret;
}

/*
 * input: method, Jobj, replyMsg
 * output(GET case):
 *			Jarray: return on wan obj in Jarray[0] or return wan array
 *
 * output(POST case)
 *                 There will be a _retJobj added in Jobj, the ipIfacePath will be added in _retJobj
 *                  Jobj:{_retJobj:{ipIfacePath:{value}}}
 *
*/
zcfgRet_t zcfgFeDalWan(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "POST"))
		ret = zcfgFeDalWanAdd(Jobj, replyMsg);
	else if(!strcmp(method, "PUT"))
		ret = zcfgFeDalWanEdit(Jobj, replyMsg);
	else if(!strcmp(method, "DELETE"))
		ret = zcfgFeDalWanDelete(Jobj, NULL);
	else if(!strcmp(method, "GET")){
		initGlobalObjects();

		ret = zcfgFeDalWanGet(Jobj, Jarray, NULL);

		freeAllObjects();
	}
	else
		printf("Unknown method:%s\n", method);

	return ret;
}



