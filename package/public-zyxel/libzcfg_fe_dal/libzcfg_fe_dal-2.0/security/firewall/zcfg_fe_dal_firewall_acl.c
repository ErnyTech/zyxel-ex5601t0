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
#include "zyutil.h"	//for str_replace()

#define LEVEL_NAME_UD "UserDefined"


dal_param_t FIREWALL_ACL_param[] =
{
	{"Index", 				dalType_int, 	 0, 	0,		NULL,	NULL, 							dal_Edit|dal_Delete},	
	{"Enable",				dalType_boolean, 0,	    0,	    NULL,	NULL,	                        dal_Add},
	{"Name",		 		dalType_string,  1, 	128,	NULL,	NULL, 							dal_Add},
	{"Order", 				dalType_int, 	 0, 	0, 		NULL,	NULL, 							dal_Add},
	{"Target", 				dalType_string,  0, 	0, 		NULL,	"Accept|Drop|Reject", 			0},
	{"SourceIPExclude", 	dalType_string,	 0,		0,		NULL,	NULL, 							dalcmd_Forbid},
	{"DestIPExclude", 		dalType_string,	 0,		0,		NULL,	NULL, 							dalcmd_Forbid},
	/*protocol*/
	{"Protocol", 			dalType_string,  0, 	0,		NULL,	"ALL|TCP|UDP|TCPUDP|ICMP|ICMPv6", dal_Add},
	{"ProtocolExclude", 	dalType_int, 	 0, 	0, 		NULL,	NULL, 							dalcmd_Forbid},
	{"IPVersion", 			dalType_string,	 0, 	0, 		NULL,	"IPv4|IPv6", 					dal_Add},
	{"MACAddress",    dalType_MacAddrHyphen,	0,	0,	NULL,	"AnyMACAddress",	   		dalcmd_Forbid},
	/*src des ip*/
	{"SourceIP", 			dalType_v4v6Addr,  0, 	0, 		NULL,	"AnyIP", 						dalcmd_Forbid},
	{"SourceMask", 			dalType_string,  0, 	0, 		NULL,	NULL, 							dalcmd_Forbid},
	{"SourceIPPrefix", 		dalType_IPPrefix,  0, 	0, 		NULL,	"AnyIP", 						dal_Add},
	{"DestIP", 				dalType_v4v6Addr,  0, 	0, 		NULL,	"AnyIP", 						dalcmd_Forbid},
	{"DestMask", 			dalType_string,  0, 	0, 		NULL,	NULL, 							dalcmd_Forbid},
	{"DestIPPrefix", 		dalType_IPPrefix,  0, 	0, 		NULL,	"AnyIP", 						dal_Add},
	/*Port*/
	{"DestPortExclude", 	dalType_int, 	 0, 	0, 		NULL,	NULL, 							dalcmd_Forbid},
	{"DestPort", 			dalType_int, 	-1, 	65535, 	NULL,	NULL, 							dalcmd_Forbid},
	{"DestPortRangeMax", 	dalType_int, 	-1, 	65535, 	NULL,	NULL, 							dalcmd_Forbid},
	{"DestPortRange", 		dalType_PortRange, 	0, 	0, 	NULL,	NULL, 								0},
	{"SourcePortExclude", 	dalType_int, 	 0, 	0, 		NULL,	NULL, 							dalcmd_Forbid},
	{"SourcePort", 			dalType_int, 	-1, 	65535, 	NULL,	NULL, 							dalcmd_Forbid},
	{"SourcePortRangeMax", 	dalType_int, 	-1, 	65535, 	NULL,	NULL, 							dalcmd_Forbid},
	{"SourcePortRange", 	dalType_PortRange, 	-1, 	65535, 	NULL,	NULL, 						0},
	/*TCP flag*/
	{"TCPFlag", 			dalType_string,  0, 	0, 		NULL,	"SYN|ACK|URG|PSH|RST|FIN",		0},	//for dalcmd TCP flag
	{"X_ZYXEL_TCPFlagComp", dalType_string,  0, 	0, 		NULL,	NULL, 							dalcmd_Forbid},
	{"X_ZYXEL_TCPFlagMask", dalType_string,  0, 	0, 		NULL,	NULL, 							dalcmd_Forbid},
	{"X_ZYXEL_ServiceIndex",dalType_int, 	 0, 	0, 		NULL,	NULL, 							dalcmd_Forbid},
	{"Direction", 			dalType_string,  0, 	0, 		NULL,	"WAN_TO_LAN|LAN_TO_WAN|WAN_TO_ROUTER|LAN_TO_ROUTER", dal_Add},
	/*rate limit*/
	{"LimitRate", 			dalType_int, 	 0, 	512, 		NULL,	NULL, 						0},
	{"LimitRateUnit",		dalType_string,  0, 	0, 		NULL,	"minute|second", 				0},
	{"X_ZYXEL_ScheduleRule",dalType_int, 	 0, 	0, 		NULL,	NULL, 							dalcmd_Forbid},
	{"IPv6header", 			dalType_string,  0, 	0, 		NULL,	NULL, 							0},
	{"IPv6headertypes",     dalType_string,  0, 	0, 		NULL,	NULL, 							0},
	{"ICMPv6Type", 			dalType_ICMPv6,  0, 	0, 		NULL,	NULL, 							0},
	{"ICMPType", 			dalType_int, 	-1, 	143, 	NULL,	NULL, 							dalcmd_Forbid},//for GUI only
	{"ICMPTypeCode",		dalType_int, 	-1, 	16, 	NULL,	NULL, 							dalcmd_Forbid},//for GUI only
	{NULL, 					0, 				 0, 	0, 		NULL,	NULL, 							0}
};
char* ICMP_type( int icmptype, int icmptypecode){
	char *type = "";
	if(icmptype == 0)
		type = "0/Echo-reply";
	else if(icmptype == 3 && icmptypecode == -1)
		type = "3/Destination-Unreachable";
	else if(icmptype == 3 && icmptypecode == 0)
		type = "3/Network-Unreachable";
	else if(icmptype == 3 && icmptypecode == 1)
		type = "3/Host-Unreachable";
	else if(icmptype == 3 && icmptypecode == 2)
		type = "3/Protocol-Unreachable";
	else if(icmptype == 3 && icmptypecode == 3)
		type = "3/Port-Unreachable";
	else if(icmptype == 3 && icmptypecode == 4)
		type = "3/Fragmentation-needed";
	else if(icmptype == 3 && icmptypecode == 5)
		type = "3/Source-routing-failed";
	else if(icmptype == 3 && icmptypecode == 6)
		type = "3/Network-unknown";
	else if(icmptype == 3 && icmptypecode == 7)
		type = "3/Host-unknown";
	else if(icmptype == 3 && icmptypecode == 9)
		type = "3/Network-prohibited";
	else if(icmptype == 3 && icmptypecode == 10)
		type = "3/Host-prohibited";
	else if(icmptype == 3 && icmptypecode == 11)
		type = "3/TOS-Network-unreachable";
	else if(icmptype == 3 && icmptypecode == 12)
		type = "3/TOS-Host-unreachable";
	else if(icmptype == 3 && icmptypecode == 13)
		type = "3/Communication-prohibited";
	else if(icmptype == 3 && icmptypecode == 14)
		type = "3/Host-precedence-violation";
	else if(icmptype == 3 && icmptypecode == 15)
		type = "3/Precedence-cutoff";
	else if(icmptype == 4)
		type = "4/Source-quench";
	else if(icmptype == 5 && icmptypecode == -1)
		type = "5/Redirect";
	else if(icmptype == 5 && icmptypecode == 0)
		type = "5/Redirect-network";
	else if(icmptype == 5 && icmptypecode == 1)
		type = "5/Redirect-host";
	else if(icmptype == 5 && icmptypecode == 2)
		type = "5/TOS-network-redirect";
	else if(icmptype == 5 && icmptypecode == 3)
		type = "5/TOS-host-redirect";
	else if(icmptype == 8)
		type = "8/Echo-request";
	else if(icmptype == 9)
		type = "9/Router-advertisement";
	else if(icmptype == 10)
		type = "10/Router-solicitation";
	else if(icmptype == 11 && icmptypecode == -1)
		type = "11/Time-exceeded";
	else if(icmptype == 11 && icmptypecode == 0)
		type = "11/TTL-zero-during-transit";
	else if(icmptype == 11 && icmptypecode == 1)
		type = "11/TTL-zero-during-reassembly";
	else if(icmptype == 12 && icmptypecode == -1)
		type = "12/Parameter-problem";
	else if(icmptype == 12 && icmptypecode == 0)
		type = "12/IP-header-bad";
	else if(icmptype == 12 && icmptypecode == 1)
		type = "12/Required-options-missing";
	else if(icmptype == 13)
		type = "13/Timestamp-request";
	else if(icmptype == 14)
		type = "14/Timestamp-reply";
	else if(icmptype == 17)
		type = "17/Address-mask-request";
	else if(icmptype == 18)
		type = "18/Address-mask-reply";	

	return type;
}

char* ICMPv6_type( int icmptype, int icmptypecode){
	char *v6type = "";
	if(icmptype == 1 && icmptypecode == 0)
		v6type = "1/No-Rute";
	else if(icmptype == 1 && icmptypecode == 1)
		v6type = "1/Communication-Prohibited";
	else if(icmptype == 1 && icmptypecode == 3)
		v6type = "1/Address-Unreachable";
	else if(icmptype == 1 && icmptypecode == 4)
		v6type = "1/Port-Unreachable";
	else if(icmptype == 2 && icmptypecode == 0)
		v6type = "2/Packet too Big";
	else if(icmptype == 3 && icmptypecode == 0)
		v6type = "3/Time Exceeded (ttl-zero-during-transit)";
	else if(icmptype == 3 && icmptypecode == 1)
		v6type = "3/Time Exceeded (ttl-zero-during-reassembly)";
	else if(icmptype == 4 && icmptypecode == 0)
		v6type = "4/Parameter Problem (bad-header)";
	else if(icmptype == 4 && icmptypecode == 1)
		v6type = "4/Parameter Problem (unknown-header-type)";
	else if(icmptype == 4 && icmptypecode == 2)
		v6type = "4/Parameter Problem (unknown-option)";
	else if(icmptype == 128)
		v6type = "128/Echo Request";
	else if(icmptype == 129)
		v6type = "129/Echo Response";
	else if(icmptype == 130)
		v6type = "130/Listener Query";
	else if(icmptype == 131)
		v6type = "131/Listener Report";
	else if(icmptype == 132)
		v6type = "132/Listener Done";
	else if(icmptype == 143)
		v6type = "143/Listener Report v2";
	else if(icmptype == 133)
		v6type = "133/Router Solicitation";
	else if(icmptype == 134)
		v6type = "134/Router Advertisement";
	else if(icmptype == 135)
		v6type = "135/Neighbor Solicitation";
	else if(icmptype == 136)
		v6type = "136/Neighbor Advertisement";
	else if(icmptype == 137)
		v6type = "137/Redirect";
	return v6type;
}

void zcfgFeDalShowFirewallACL(struct json_object *Jarray){
	int i, len = 0, SourcePort = 0, SourcePortRange = 0, DestPort = 0, DestPortRange = 0, icmptype = 0, icmptypecode = 0;
	int ServiceIndex = 0;
	const char *protocol = NULL;
	const char *icmp = NULL;
	const char *icmpv6 = NULL;
	char tmp[64] = {0}, srcport[32] = {0}, dstport[32] = {0}, srcip[32] = {0}, dstip[32] = {0};
	const char *SourceIP = NULL;
	const char *DestIP = NULL;
	const char *SourceMask = NULL;
	const char *DestMask = NULL;
	//const char *MACAddress = NULL;
	struct json_object *obj = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	
	printf("%-6s %-6s %-6s %-13s %-41s %-41s %-40s %-12s\n",
		    "Index", "Enable", "Order", "Name", "Source IP", "Destination IP", "Service", "Action");

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		ServiceIndex = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_ServiceIndex"));
		protocol = json_object_get_string(json_object_object_get(obj, "Protocol"));
		SourceIP = json_object_get_string(json_object_object_get(obj, "SourceIP"));
		SourceMask = json_object_get_string(json_object_object_get(obj, "SourceMask"));
		DestIP = json_object_get_string(json_object_object_get(obj, "DestIP"));
		DestMask = json_object_get_string(json_object_object_get(obj, "DestMask"));
		//MACAddress = json_object_get_string(json_object_object_get(obj, "MACAddress"));
		tmp[0] = '\0';
			printf("%-6s %-6s %-6s %-13s ",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Enable")),			
		    json_object_get_string(json_object_object_get(obj, "Order")),
		    json_object_get_string(json_object_object_get(obj, "Name")));

		if(!strcmp(SourceMask,"")){
			strcpy(srcip,SourceIP);
		}
		else{
			sprintf(srcip, "%s/%s", SourceIP, SourceMask);
		}
		if(!strcmp(DestMask,"")){
			strcpy(dstip,DestIP);
		}
		else{
			sprintf(dstip, "%s/%s", DestIP , DestMask);
		}
		printf("%-41s %-41s ",srcip,dstip);
		if(ServiceIndex != 0){
			printf("%-40s ",json_object_get_string(json_object_object_get(obj, "ServiceName")));
		}
		else{
			if(!strcmp(protocol,"TCP/UDP") ||!strcmp(protocol,"TCP") ||!strcmp(protocol,"UDP")){
				SourcePort = json_object_get_int(json_object_object_get(obj, "SourcePort"));
				SourcePortRange = json_object_get_int(json_object_object_get(obj, "SourcePortRangeMax"));
				DestPort = json_object_get_int(json_object_object_get(obj, "DestPort"));
				DestPortRange = json_object_get_int(json_object_object_get(obj, "DestPortRangeMax"));
				if(SourcePortRange != -1)
					sprintf(srcport,"%d:%d",SourcePort,SourcePortRange);
				else if(SourcePort != -1)
					sprintf(srcport,"%d",SourcePort);
				else
					strcpy(srcport,"Any");
				if(DestPortRange != -1)
					sprintf(dstport,"%d:%d",DestPort,DestPortRange);
				else if(DestPort != -1)
					sprintf(dstport,"%d",DestPort);
				else
					strcpy(dstport,"Any");

				sprintf(tmp, "%s:%s-->%s",protocol,srcport,dstport);	
				printf("%-40s ",tmp);
			}
			else if(!strcmp(protocol,"ICMP")){
				icmptype = json_object_get_int(json_object_object_get(obj,"ICMPType"));
				icmptypecode = json_object_get_int(json_object_object_get(obj,"ICMPTypeCode"));
				icmp = ICMP_type(icmptype,icmptypecode);
				sprintf(tmp, "ICMP:%s", icmp);
				printf("%-40s ",tmp);
			}
			else if(!strcmp(protocol,"ICMPv6")){
				icmptype = json_object_get_int(json_object_object_get(obj,"ICMPType"));
				icmptypecode = json_object_get_int(json_object_object_get(obj,"ICMPTypeCode"));
				icmpv6 = ICMPv6_type(icmptype,icmptypecode);
				sprintf(tmp, "ICMPv6:%s", icmpv6);
				printf("%-40s ",tmp);
			}
			else{
				printf("%-40s ",json_object_get_string(json_object_object_get(obj, "Protocol")));
			}
		}
		printf("%-12s\n",json_object_get_string(json_object_object_get(obj, "Target")));
	}
}

bool getChainRuleIid(objIndex_t *chainIid){
	struct json_object *levelObj = NULL;
	objIndex_t levelIid = {0};
	const char *name = NULL, *chain = NULL;

	IID_INIT(*chainIid);
	IID_INIT(levelIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_FIREWALL_LEVEL, &levelIid, &levelObj) == ZCFG_SUCCESS) {
		name = json_object_get_string(json_object_object_get(levelObj, "Name"));
		if(!strcmp(name, LEVEL_NAME_UD)){
			chain = json_object_get_string(json_object_object_get(levelObj, "Chain"));
			chainIid->level = 1;
			if (sscanf(chain, "Firewall.Chain.%hhu", &(chainIid->idx[0])) == 1){
				zcfgFeJsonObjFree(levelObj);
				return true;
			}
			zcfgFeJsonObjFree(levelObj);
			break;
		}
		zcfgFeJsonObjFree(levelObj);
	}
	return false;
}
zcfgRet_t zcfgFeDal_Firewall_ACL_Add(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *aclObj = NULL;
	struct json_object *paramObj = NULL;
 	objIndex_t aclIid = {0};
	objIndex_t chainIid = {0};
	zcfg_offset_t oid;
	const char *charVal = NULL;
	const char *protocol = NULL;
	const char *name = NULL;
	bool enable;
	char ip[64] = {0};
	char mask[8] = {0};
	char MACAddress[18] = {0};
	int startPort, endPort, ServiceIndex;
	objIndex_t protocolIid = {0};
	IID_INIT(protocolIid);

	IID_INIT(aclIid);
	oid = RDM_OID_FIREWALL_CHAIN_RULE;
 		
	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	ServiceIndex = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_ServiceIndex"));

	ret = dalcmdParamCheck(NULL, name, oid, "Description", "Name",  replyMsg);
	if(ret != ZCFG_SUCCESS){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_name"));
		return ret;
	}
	if(getChainRuleIid(&chainIid)){
		ret = zcfgFeObjJsonBlockedAdd(RDM_OID_FIREWALL_CHAIN_RULE, &chainIid, NULL);
		if(ret == ZCFG_SUCCESS){
			memcpy(&aclIid, &chainIid, sizeof(objIndex_t));
			ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FIREWALL_CHAIN_RULE, &aclIid, &aclObj);
		}

		if(ret == ZCFG_SUCCESS){
#if 0//IChia 20190918
			json_object_object_foreach(Jobj, key, val){
				if(!strcmp(key, "Index")){
					continue;
				}
				json_object_object_add(aclObj, key, val);
			}
#else
			json_object_object_foreach(Jobj, key, val)
			{
				if(!strcmp(key, "Index"))
				{
					continue;
				}
				json_object_object_add(aclObj, key, JSON_OBJ_COPY(val));
			}
#endif
			
			if((paramObj = json_object_object_get(Jobj, "IPVersion")) != NULL){
				charVal = json_object_get_string(paramObj);
				if(!strcmp(charVal, "IPv4"))
					json_object_object_add(aclObj, "IPVersion", json_object_new_int(4));
				else if(!strcmp(charVal, "IPv6"))
					json_object_object_add(aclObj, "IPVersion", json_object_new_int(6));
			}
			if((paramObj = json_object_object_get(Jobj, "TCPFlag")) != NULL){
				protocol = json_object_get_string(json_object_object_get(Jobj, "Protocol"));
				if(!strcmp(protocol, "TCP") || !strcmp(protocol, "TCPUDP")){
					json_object_object_add(aclObj, "X_ZYXEL_TCPFlagMask", JSON_OBJ_COPY(paramObj));
					json_object_object_add(aclObj, "X_ZYXEL_TCPFlagComp", JSON_OBJ_COPY(paramObj));
					json_object_object_del(aclObj, "TCPFlag");
				}
				else{
					strcpy(replyMsg, "TCP flag should choose Protocol TCP or TCP/UDP.");
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			if((paramObj = json_object_object_get(Jobj, "Protocol")) != NULL){
				charVal = json_object_get_string(paramObj);
				if(!strcmp(charVal, "ALL"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(-1));
				else if(!strcmp(charVal, "TCP"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(6));
				else if(!strcmp(charVal, "UDP"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(17));
				else if(!strcmp(charVal, "TCPUDP"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(-2));
				else if(!strcmp(charVal, "ICMP"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(1));
				else if(!strcmp(charVal, "ICMPv6"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(58));
			}
			
			if((paramObj = json_object_object_get(Jobj, "SourceIPPrefix")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "SourceIPPrefix")), "AnyIP")){
					json_object_object_add(aclObj, "SourceIP", json_object_new_string(""));
					json_object_object_add(aclObj, "SourceMask", json_object_new_string("0"));
				}
				else{
					charVal = json_object_get_string(paramObj);
					sscanf(charVal, "%[^/]/%s", ip, mask);
					//printf("SourceIPPrefix ip=%s mask=%s\n", ip, mask);
					json_object_object_add(aclObj, "SourceIP", json_object_new_string(ip));
					json_object_object_add(aclObj, "SourceMask", json_object_new_string(mask));
				}
				json_object_object_del(aclObj, "SourceIPPrefix");
			}
			if((paramObj = json_object_object_get(Jobj, "MACAddress")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MACAddress")), "AnyMACAddress")){
					json_object_object_add(aclObj, "MACAddress", json_object_new_string(""));				
				}
				else{
					charVal = json_object_get_string(paramObj);
					sscanf(charVal, "%s", MACAddress);
					str_replace(MACAddress, "-",":");
					json_object_object_add(aclObj, "MACAddress", json_object_new_string(MACAddress));		
				}
			}
			//else
			//	json_object_object_add(aclObj, "SourceMask", json_object_new_string("32"));
			
			if((paramObj = json_object_object_get(Jobj, "DestIPPrefix")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "DestIPPrefix")), "AnyIP")){
					json_object_object_add(aclObj, "DestIP", json_object_new_string(""));
					json_object_object_add(aclObj, "DestMask", json_object_new_string("0"));
				}
				else{
					charVal = json_object_get_string(paramObj);
					sscanf(charVal, "%[^/]/%s", ip, mask);
					//printf("DestIPPrefix ip=%s mask=%s\n", ip, mask);
					json_object_object_add(aclObj, "DestIP", json_object_new_string(ip));
					json_object_object_add(aclObj, "DestMask", json_object_new_string(mask));
				}
				json_object_object_del(aclObj, "DestIPPrefix");
			}
			//else
			//	json_object_object_add(aclObj, "DestMask", json_object_new_string("32"));

			if((paramObj = json_object_object_get(Jobj, "DestPortRange")) != NULL){
				charVal = json_object_get_string(paramObj);
				if(strchr(charVal, ':') != NULL){
					sscanf(charVal, "%d:%d", &startPort, &endPort);
					if(startPort == -1)
						endPort = -1;
				}
				else{
					startPort = atoi(charVal);
					endPort = -1;
				}
				
				//printf("DestPortRange startPort=%d endPort=%d\n", startPort, endPort);
				json_object_object_add(aclObj, "DestPort", json_object_new_int(startPort));
				json_object_object_add(aclObj, "DestPortRangeMax", json_object_new_int(endPort));
				json_object_object_del(aclObj, "DestPortRange");
			}
			else if(json_object_object_get(Jobj, "DestPort") == NULL){//Protocol == ALL or not from WEGUI, and DestPortRange == NULL add default value
				json_object_object_add(aclObj, "DestPort", json_object_new_int(-1));
				json_object_object_add(aclObj, "DestPortRangeMax", json_object_new_int(-1));
			}

			if((paramObj = json_object_object_get(Jobj, "SourcePortRange")) != NULL){
				charVal = json_object_get_string(paramObj);
				if(strchr(charVal, ':') != NULL){
					sscanf(charVal, "%d:%d", &startPort, &endPort);
					if(startPort == -1)
						endPort = -1;
				}
				else{
					startPort = atoi(charVal);
					endPort = -1;
				}
				
				//printf("SourcePortRange startPort=%d endPort=%d\n", startPort, endPort);
				json_object_object_add(aclObj, "SourcePort", json_object_new_int(startPort));
				json_object_object_add(aclObj, "SourcePortRangeMax", json_object_new_int(endPort));
				json_object_object_del(aclObj, "SourcePortRange");
			}
			else if(json_object_object_get(Jobj, "SourcePort") == NULL){//Protocol == ALL or not from WEGUI, and SourcePort == NULL, add default value
				json_object_object_add(aclObj, "SourcePort", json_object_new_int(-1));
				json_object_object_add(aclObj, "SourcePortRangeMax", json_object_new_int(-1));
			}
#ifdef ZYXEL_TAAAG_FIREWALL_WANINTERFACE//wan interface of Firewall
			char *Direction = NULL;
			char *Interface = NULL;
			Interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
			Direction = json_object_get_string(json_object_object_get(Jobj, "Direction"));
			//printf("zcfgFeDal_Firewall_ACL_Add  --%s--%s--\r\n",Interface,Direction);
			if(!strcmp(Direction,"WAN_TO_LAN") || !strcmp(Direction,"WAN_TO_ROUTER"))
			{
				if(!strcmp(Interface,"ALL"))
					json_object_object_add(aclObj, "SourceAllInterfaces",json_object_new_boolean(true));
				else
					json_object_object_add(aclObj, "SourceInterface", JSON_OBJ_COPY(json_object_object_get(Jobj, "Interface")));
			}
			else if(!strcmp(Direction,"LAN_TO_WAN"))
			{
				if(!strcmp(Interface,"ALL"))
					json_object_object_add(aclObj, "DestAllInterfaces",json_object_new_boolean(true));
				else
					json_object_object_add(aclObj, "DestInterface", JSON_OBJ_COPY(json_object_object_get(Jobj, "Interface")));
			}
#endif			
			if((paramObj = json_object_object_get(Jobj, "Direction")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_Direction", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "Direction");
			}
			if((paramObj = json_object_object_get(Jobj, "IPv6header")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_IPv6header", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "IPv6header");
			}
			if((paramObj = json_object_object_get(Jobj, "IPv6headertypes")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_IPv6headertypes", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "IPv6headertypes");
			}
			if((paramObj = json_object_object_get(Jobj, "ICMPType")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_ICMPType", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "ICMPType");
			}
			if((paramObj = json_object_object_get(Jobj, "ICMPTypeCode")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_ICMPTypeCode", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "ICMPTypeCode");
			}
			if((paramObj = json_object_object_get(Jobj, "MACAddress")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MACAddress")), "AnyMACAddress"))
				json_object_object_add(aclObj, "MACAddress", json_object_new_string(""));
			}
			if((paramObj = json_object_object_get(Jobj, "SourceIP")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "SourceIP")), "AnyIP"))
				json_object_object_add(aclObj, "SourceIP", json_object_new_string(""));
			}
			if((paramObj = json_object_object_get(Jobj, "DestIP")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "DestIP")), "AnyIP"))
				json_object_object_add(aclObj, "DestIP", json_object_new_string(""));
			}
			if((paramObj = json_object_object_get(Jobj, "LimitRate")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_LimitRate", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "LimitRate");
			}
			if((paramObj = json_object_object_get(Jobj, "LimitRateUnit")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_LimitRateUnit", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "LimitRateUnit");
			}
			
			if(json_object_object_get(Jobj, "Enable"))
				json_object_object_add(aclObj, "Enable", json_object_new_boolean(enable));
					
			if(json_object_object_get(Jobj, "Name"))
				json_object_object_add(aclObj, "Description", json_object_new_string(name));

			ret = convertIndextoIid(ServiceIndex, &protocolIid, RDM_OID_NETWORK_SRV_GP, NULL, NULL, replyMsg);

			if (json_object_object_get(Jobj, "X_ZYXEL_ServiceIndex"))
				json_object_object_add(aclObj, "X_ZYXEL_ServiceIndex", json_object_new_int(protocolIid.idx[0]));

			ret = zcfgFeObjJsonBlockedSet(RDM_OID_FIREWALL_CHAIN_RULE, &aclIid, aclObj, NULL);
		}
	}

	zcfgFeJsonObjFree(aclObj);
	return ret;
}

zcfgRet_t zcfgFeDal_Firewall_ACL_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *aclObj = NULL;
	objIndex_t aclIid = {0};
	struct json_object *paramObj = NULL;
	const char *charVal = NULL;
	const char *protocol = NULL;
	const char *name = NULL;	
	bool enable;
	char ip[64] = {0};
	char mask[8] = {0};
	char MACAddress[18] = {0};
	int startPort, endPort, ServiceIndex;
	objIndex_t protocolIid = {0};
	IID_INIT(protocolIid);

	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	ServiceIndex = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_ServiceIndex"));
	
	if(getChainRuleIid(&aclIid)){
		aclIid.level = 2;
		aclIid.idx[1] = json_object_get_int(json_object_object_get(Jobj, "Index"));
		if(json_object_object_get(Jobj, "Name")){
			ret = dalcmdParamCheck(&aclIid, name, RDM_OID_FIREWALL_CHAIN_RULE, "Description", "Name",  replyMsg);
			if(ret != ZCFG_SUCCESS){
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_name"));
				return ret;
			}
		}
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FIREWALL_CHAIN_RULE, &aclIid, &aclObj)) == ZCFG_SUCCESS){
#if 0 //IChia 20190917
			json_object_object_foreach(Jobj, key, val){
				if(!strcmp(key, "Index")){
					continue;
				}
				json_object_object_add(aclObj, key, val);
			}
#else
			json_object_object_foreach(Jobj, key, val)
			{
				if(!strcmp(key, "Index"))
				{
					continue;
				}
				json_object_object_add(aclObj, key, JSON_OBJ_COPY(val));
			}
#endif
			if((paramObj = json_object_object_get(Jobj, "IPVersion")) != NULL){
				charVal = json_object_get_string(paramObj);
				if(!strcmp(charVal, "IPv4"))
					json_object_object_add(aclObj, "IPVersion", json_object_new_int(4));
				else if(!strcmp(charVal, "IPv6"))
					json_object_object_add(aclObj, "IPVersion", json_object_new_int(6));
			}
			if((paramObj = json_object_object_get(Jobj, "TCPFlag")) != NULL){
				protocol = json_object_get_string(json_object_object_get(Jobj, "Protocol"));
				if(!strcmp(protocol, "TCP") || !strcmp(protocol, "TCPUDP")){
					json_object_object_add(aclObj, "X_ZYXEL_TCPFlagMask", JSON_OBJ_COPY(paramObj));
					json_object_object_add(aclObj, "X_ZYXEL_TCPFlagComp", JSON_OBJ_COPY(paramObj));
					json_object_object_del(aclObj, "TCPFlag");
				}
				else{
					strcpy(replyMsg, "TCP flag should choose Protocol TCP or TCP/UDP.");
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			if((paramObj = json_object_object_get(Jobj, "Protocol")) != NULL){
				charVal = json_object_get_string(paramObj);
				if(!strcmp(charVal, "ALL"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(-1));
				else if(!strcmp(charVal, "TCP"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(6));
				else if(!strcmp(charVal, "UDP"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(17));
				else if(!strcmp(charVal, "TCPUDP"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(-2));
				else if(!strcmp(charVal, "ICMP"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(1));
				else if(!strcmp(charVal, "ICMPv6"))
					json_object_object_add(aclObj, "Protocol", json_object_new_int(58));
			}
			
			if((paramObj = json_object_object_get(Jobj, "SourceIPPrefix")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "SourceIPPrefix")), "AnyIP")){
					json_object_object_add(aclObj, "SourceIP", json_object_new_string(""));
					json_object_object_add(aclObj, "SourceMask", json_object_new_string("0"));
				}
				else{
					charVal = json_object_get_string(paramObj);
					sscanf(charVal, "%[^/]/%s", ip, mask);
					
					//printf("SourceIPPrefix ip=%s mask=%s\n", ip, mask);
					json_object_object_add(aclObj, "SourceIP", json_object_new_string(ip));
					json_object_object_add(aclObj, "SourceMask", json_object_new_string(mask));
				}
				json_object_object_del(aclObj, "SourceIPPrefix");
			}
			if((paramObj = json_object_object_get(Jobj, "MACAddress")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MACAddress")), "AnyMACAddress")){
					json_object_object_add(aclObj, "MACAddress", json_object_new_string(""));				
				}
				else{
					charVal = json_object_get_string(paramObj);
					sscanf(charVal, "%s", MACAddress);
					str_replace(MACAddress, "-",":");
					json_object_object_add(aclObj, "MACAddress", json_object_new_string(MACAddress));		
				}
			}
			if((paramObj = json_object_object_get(Jobj, "DestIPPrefix")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "DestIPPrefix")), "AnyIP")){
					json_object_object_add(aclObj, "DestIP", json_object_new_string(""));
					json_object_object_add(aclObj, "DestMask", json_object_new_string("0"));
				}
				else{
					charVal = json_object_get_string(paramObj);
					sscanf(charVal, "%[^/]/%s", ip, mask);
					
					//printf("DestIPPrefix ip=%s mask=%s\n", ip, mask);
					json_object_object_add(aclObj, "DestIP", json_object_new_string(ip));
					json_object_object_add(aclObj, "DestMask", json_object_new_string(mask));
				}
				json_object_object_del(aclObj, "DestIPPrefix");
			}
			if((paramObj = json_object_object_get(Jobj, "DestPortRange")) != NULL){
				charVal = json_object_get_string(paramObj);
				if(strchr(charVal, ':') != NULL){
					sscanf(charVal, "%d:%d", &startPort, &endPort);
					if(startPort == -1)
						endPort = -1;
				}
				else{
					startPort = atoi(charVal);
					endPort = -1;
				}
				
				//printf("DestPortRange startPort=%d endPort=%d\n", startPort, endPort);
				json_object_object_add(aclObj, "DestPort", json_object_new_int(startPort));
				json_object_object_add(aclObj, "DestPortRangeMax", json_object_new_int(endPort));
				json_object_object_del(aclObj, "DestPortRange");
			}
			if((paramObj = json_object_object_get(Jobj, "SourcePortRange")) != NULL){
				charVal = json_object_get_string(paramObj);
				if(strchr(charVal, ':') != NULL){
					sscanf(charVal, "%d:%d", &startPort, &endPort);
					if(startPort == -1)
						endPort = -1;
				}
				else{
					startPort = atoi(charVal);
					endPort = -1;
				}
				
				//printf("SourcePortRange startPort=%d endPort=%d\n", startPort, endPort);
				json_object_object_add(aclObj, "SourcePort", json_object_new_int(startPort));
				json_object_object_add(aclObj, "SourcePortRangeMax", json_object_new_int(endPort));
				json_object_object_del(aclObj, "SourcePortRange");
			}
#ifdef ZYXEL_TAAAG_FIREWALL_WANINTERFACE//wan interface of Firewall
						char *Direction = NULL;
						char *Interface = NULL;
						Interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
						Direction = json_object_get_string(json_object_object_get(Jobj, "Direction"));
						//printf("zcfgFeDal_Firewall_ACL_Edit  --%s--%s--\r\n",Interface,Direction);
						if(!strcmp(Direction,"WAN_TO_LAN") || !strcmp(Direction,"WAN_TO_ROUTER"))
						{
							if(!strcmp(Interface,"ALL"))
								json_object_object_add(aclObj, "SourceAllInterfaces",json_object_new_boolean(true));
							else{
								json_object_object_add(aclObj, "SourceAllInterfaces",json_object_new_boolean(false));
								json_object_object_add(aclObj, "SourceInterface", JSON_OBJ_COPY(json_object_object_get(Jobj, "Interface")));
							}
						}
						else if(!strcmp(Direction,"LAN_TO_WAN"))
						{
							if(!strcmp(Interface,"ALL"))
								json_object_object_add(aclObj, "DestAllInterfaces",json_object_new_boolean(true));
							else{
								json_object_object_add(aclObj, "DestAllInterfaces",json_object_new_boolean(false));
								json_object_object_add(aclObj, "DestInterface", JSON_OBJ_COPY(json_object_object_get(Jobj, "Interface")));
							}
						}
#endif			
			if((paramObj = json_object_object_get(Jobj, "Direction")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_Direction", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "Direction");
			}
			if((paramObj = json_object_object_get(Jobj, "IPv6header")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_IPv6header", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "IPv6header");
			}
			if((paramObj = json_object_object_get(Jobj, "IPv6headertypes")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_IPv6headertypes", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "IPv6headertypes");
			}
			if((paramObj = json_object_object_get(Jobj, "ICMPType")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_ICMPType", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "ICMPType");
			}
			if((paramObj = json_object_object_get(Jobj, "ICMPTypeCode")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_ICMPTypeCode", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "ICMPTypeCode");
			}
			if((paramObj = json_object_object_get(Jobj, "MACAddress")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MACAddress")), "AnyMACAddress"))
				json_object_object_add(aclObj, "MACAddress", json_object_new_string(""));
			}
			if((paramObj = json_object_object_get(Jobj, "SourceIP")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "SourceIP")), "AnyIP"))
				json_object_object_add(aclObj, "SourceIP", json_object_new_string(""));
			}
			if((paramObj = json_object_object_get(Jobj, "DestIP")) != NULL){
				if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "DestIP")), "AnyIP"))
				json_object_object_add(aclObj, "DestIP", json_object_new_string(""));
			}
			if((paramObj = json_object_object_get(Jobj, "LimitRate")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_LimitRate", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "LimitRate");
			}
			if((paramObj = json_object_object_get(Jobj, "LimitRateUnit")) != NULL){
				json_object_object_add(aclObj, "X_ZYXEL_LimitRateUnit", JSON_OBJ_COPY(paramObj));
				json_object_object_del(aclObj, "LimitRateUnit");
			}

			if(json_object_object_get(Jobj, "Enable"))
				json_object_object_add(aclObj, "Enable", json_object_new_boolean(enable));
		
			if(json_object_object_get(Jobj, "Name"))
				json_object_object_add(aclObj, "Description", json_object_new_string(name));

			ret = convertIndextoIid(ServiceIndex, &protocolIid, RDM_OID_NETWORK_SRV_GP, NULL, NULL, replyMsg);

			if (json_object_object_get(Jobj, "X_ZYXEL_ServiceIndex"))
				json_object_object_add(aclObj, "X_ZYXEL_ServiceIndex", json_object_new_int(protocolIid.idx[0]));

			ret = zcfgFeObjJsonBlockedSet(RDM_OID_FIREWALL_CHAIN_RULE, &aclIid, aclObj, NULL);
			zcfgFeJsonObjFree(aclObj);
		}
	}
	else
		ret = ZCFG_NOT_FOUND;

	return ret;
	
}

zcfgRet_t zcfgFeDal_Firewall_ACL_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *FiremallACLJobj = NULL;
	struct json_object *protocolobj = NULL;
	int Protocol = 0;
	const char *SourceIP = NULL;
	const char *DestIP = NULL;
	const char *MACAddress = NULL;
	int ServiceIndex = 0;
	int count = 1;
	objIndex_t FiremallACLIid = {0};
	objIndex_t protocolIid = {0};
	IID_INIT(FiremallACLIid);
	IID_INIT(protocolIid);
		
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_FIREWALL_CHAIN_RULE, &FiremallACLIid, &FiremallACLJobj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index", json_object_new_int(count));
		
		Protocol = json_object_get_int(json_object_object_get(FiremallACLJobj, "Protocol"));
		switch(Protocol){
			case -2: //TCPUDP
				json_object_object_add(paramJobj, "Protocol", json_object_new_string("TCP/UDP"));
				break;
			case 6:	 //TPC
				json_object_object_add(paramJobj, "Protocol", json_object_new_string("TCP"));
				break;
			case 17: //UDP
				json_object_object_add(paramJobj, "Protocol", json_object_new_string("UDP"));
				break;
			case -1: //ALL
				json_object_object_add(paramJobj, "Protocol", json_object_new_string("ALL"));
				break;
			case 1: //ICMP
				json_object_object_add(paramJobj, "Protocol", json_object_new_string("ICMP"));
				break;
			case 58: //ICMPv6
				json_object_object_add(paramJobj, "Protocol", json_object_new_string("ICMPv6"));
				break;
			default:
				json_object_object_add(paramJobj, "Protocol", json_object_new_string(""));
				break;
		}

		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "Enable")));
		json_object_object_add(paramJobj, "Order", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "Order")));
		json_object_object_add(paramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "Description")));
		json_object_object_add(paramJobj, "SourcePort", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "SourcePort")));
		json_object_object_add(paramJobj, "SourcePortRangeMax", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "SourcePortRangeMax")));
		json_object_object_add(paramJobj, "DestPort", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "DestPort")));
		json_object_object_add(paramJobj, "DestPortRangeMax", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "DestPortRangeMax")));
		json_object_object_add(paramJobj, "Direction", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "X_ZYXEL_Direction")));
		json_object_object_add(paramJobj, "IPVersion", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "IPVersion")));
		json_object_object_add(paramJobj, "X_ZYXEL_ScheduleRule", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "X_ZYXEL_ScheduleRule")));
		json_object_object_add(paramJobj, "LimitRate", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "X_ZYXEL_LimitRate")));
		json_object_object_add(paramJobj, "LimitRateUnit", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "X_ZYXEL_LimitRateUnit")));
		json_object_object_add(paramJobj, "X_ZYXEL_TCPFlagMask", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "X_ZYXEL_TCPFlagMask")));
		json_object_object_add(paramJobj, "X_ZYXEL_TCPFlagComp", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "X_ZYXEL_TCPFlagComp")));
#ifdef ZYXEL_TAAAG_FIREWALL_WANINTERFACE//wan interface of Firewall
		char *Direction = NULL;
		Direction = json_object_get_string(json_object_object_get(FiremallACLJobj, "X_ZYXEL_Direction"));
		if(!strcmp(Direction,"WAN_TO_LAN") || !strcmp(Direction,"WAN_TO_ROUTER"))
		{
			if(json_object_get_boolean(json_object_object_get(FiremallACLJobj, "SourceAllInterfaces")))
					json_object_object_add(paramJobj, "Interface",json_object_new_string("ALL"));
			else{
					json_object_object_add(paramJobj, "Interface",JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "SourceInterface")));
				}
		}
		else if(!strcmp(Direction,"LAN_TO_WAN"))
		{
			if(json_object_get_boolean(json_object_object_get(FiremallACLJobj, "DestAllInterfaces")))
					json_object_object_add(paramJobj, "Interface",json_object_new_string("ALL"));
			else{
					json_object_object_add(paramJobj, "Interface",JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "DestInterface")));
				}

		}
#endif
		
		SourceIP = json_object_get_string(json_object_object_get(FiremallACLJobj, "SourceIP"));
		DestIP = json_object_get_string(json_object_object_get(FiremallACLJobj, "DestIP"));
		MACAddress = json_object_get_string(json_object_object_get(FiremallACLJobj, "MACAddress"));
		str_replace((char *)MACAddress, "-",":"); 
		if(!strcmp(MACAddress,"")){
			json_object_object_add(paramJobj, "MACAddress", json_object_new_string("AnyMACAddress"));
		}
		else{
			json_object_object_add(paramJobj, "MACAddress", json_object_new_string(MACAddress));
		}
		
		if(!strcmp(SourceIP,"")){
			json_object_object_add(paramJobj, "SourceIP", json_object_new_string("Any"));
		}
		else{
			json_object_object_add(paramJobj, "SourceIP", json_object_new_string(SourceIP));
		}
		json_object_object_add(paramJobj, "SourceMask", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "SourceMask")));
		
		if(!strcmp(DestIP,"")){

			json_object_object_add(paramJobj, "DestIP", json_object_new_string("Any"));
		}
		else{
			json_object_object_add(paramJobj, "DestIP", json_object_new_string(DestIP));
		}
		json_object_object_add(paramJobj, "DestMask", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "DestMask")));
		json_object_object_add(paramJobj, "IPv6header", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "X_ZYXEL_IPv6header")));
		json_object_object_add(paramJobj, "IPv6headertypes", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "X_ZYXEL_IPv6headertypes")));
		json_object_object_add(paramJobj, "ICMPType", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "X_ZYXEL_ICMPType")));
		json_object_object_add(paramJobj, "ICMPTypeCode", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "X_ZYXEL_ICMPTypeCode")));
		json_object_object_add(paramJobj, "Target", JSON_OBJ_COPY(json_object_object_get(FiremallACLJobj, "Target")));

		ServiceIndex = json_object_get_int(json_object_object_get(FiremallACLJobj, "X_ZYXEL_ServiceIndex"));
		json_object_object_add(paramJobj, "X_ZYXEL_ServiceIndex", json_object_new_int(ServiceIndex));
		if(ServiceIndex != 0){
			protocolIid.level = 1;
			protocolIid.idx[0] = ServiceIndex;
			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_NETWORK_SRV_GP, &protocolIid, &protocolobj) == ZCFG_SUCCESS){
				json_object_object_add(paramJobj, "ServiceName", JSON_OBJ_COPY(json_object_object_get(protocolobj, "Name")));
				zcfgFeJsonObjFree(protocolobj);
			}
		}
		json_object_object_add(paramJobj, "idx", json_object_new_int(FiremallACLIid.idx[1]));
		json_object_array_add(Jarray, paramJobj);
		count++;

		zcfgFeJsonObjFree(FiremallACLJobj);
	}	
	return ret;
}

zcfgRet_t zcfgFeDal_Firewall_ACL_Delete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL, *objtmp = NULL;
	struct json_object *aclObj = NULL;
	objIndex_t aclIid = {0};
	int index = 0, len = 0, i = 0, idx = 0;
	if(getChainRuleIid(&aclIid)){
		obj = json_object_new_array();
		zcfgFeDal_Firewall_ACL_Get(NULL, obj, NULL);
		len = json_object_array_length(obj);
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		for(i=0;i<len;i++){
			objtmp = json_object_array_get_idx(obj, i);
			if(index == atoi(json_object_get_string(json_object_object_get(objtmp, "Index")))){
				idx = json_object_get_int(json_object_object_get(objtmp, "idx"));
				break;
			}
		}
		aclIid.level = 2;
		aclIid.idx[0] = 1;
		aclIid.idx[1] = idx;
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FIREWALL_CHAIN_RULE, &aclIid, &aclObj)) == ZCFG_SUCCESS){
			ret = zcfgFeObjJsonDel(RDM_OID_FIREWALL_CHAIN_RULE, &aclIid, NULL);
			zcfgFeJsonObjFree(aclObj);
		}
	}
	else{
		ret = ZCFG_NOT_FOUND;
	}
	return ret;
}

zcfgRet_t zcfgFeDalFirewallACL(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_Firewall_ACL_Edit(Jobj, replyMsg);
	}else if(!strcmp(method, "POST")) {
		ret = zcfgFeDal_Firewall_ACL_Add(Jobj, replyMsg);
	}else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDal_Firewall_ACL_Delete(Jobj, NULL);
	}else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_Firewall_ACL_Get(Jobj, Jarray, NULL);
	}else
		printf("Unknown method:%s\n", method);

	return ret;
}



