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
#include "zcfg_fe_dal.h"

dal_param_t SUBNET_param[]={
	{"GroupName",				dalType_string,		0,	0,	NULL,	NULL,	dal_Edit},
	{"ipAlias_Enable", 			dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"ipAlias_IPAddress", 		dalType_v4Addr,		0,	0,	NULL,	NULL,	0},
	{"ipAlias_SubnetMask", 		dalType_v4Mask,		0,	0,	NULL,	NULL,	0},
#ifdef ZYXEL_SECOND_LAN_IP_ALIAS
	{"GroupName_2",				dalType_string,		0,	0,	NULL,	NULL,	dal_Edit},
	{"ipAlias_Enable_2", 			dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"ipAlias_IPAddress_2", 		dalType_v4Addr,		0,	0,	NULL,	NULL,	0},
	{"ipAlias_SubnetMask_2", 		dalType_v4Mask,		0,	0,	NULL,	NULL,	0},
#endif
	{"PubLan_Enable",			dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"PubLan_IP", 				dalType_v4Addr,		0,	0,	NULL,	NULL,	0},
	{"PubLan_SubnetMask", 		dalType_v4Mask,		0,	0,	NULL,	NULL,	0},
	{"PubLan_DHCP", 			dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"PubLan_ARP", 				dalType_boolean,	0,	0,	NULL,	NULL,	0},
#ifdef ZYXEL_IP_UNNUMBER
	{"SubnetState",		    dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"IP_unmber_Enable",		    dalType_boolean,	0,	0,	NULL,		NULL,			0},	
	{"Interface",			dalType_string, 	0,	0,	NULL,		NULL,			0},
	{"Multiple_interface",			dalType_string, 	0,	0,	NULL,		NULL,			0},	
	{"MinAddress",			dalType_string, 	0,	0,	NULL,		NULL,			0},
	{"MaxAddress",			dalType_string, 	0,	0,	NULL,		NULL,			0},
	{"SubnetMask",			dalType_string, 	0,	0,	NULL,		NULL,			0},
	{"LANIP",				dalType_string, 	0,	0,	NULL,		NULL,			0},
	{"subnetObjAction",		dalType_string, 	0,	0,	NULL,		NULL,			0},	
#endif
	{NULL,						0,					0,	0,	NULL,	NULL,	0},
};
#ifdef ZYXEL_IP_UNNUMBER
static const char* subnetDHCPObjAction;
objIndex_t DHCPSubnetIid = {0};
#endif
void zcfgFeDalShowAddnl(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj = NULL;
	const char *IPAddress = NULL;
	const char *SubnetMask = NULL;
	const char *X_ZYXEL_PubLan_IP = NULL;
	const char *X_ZYXEL_PubLan_Mask = NULL;
	
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-15s %-42s %-70s \n","", "IP Alias Setup", "Public LAN");
	printf("%-15s %-10s %-15s %-15s %-10s %-15s %-15s %-25s %-20s\n",
		    "Group Name", "Active", "IPv4 Arrdess", "Subnet mask", "Active", "IPv4 Arrdess", "Subnet mask", "Offer Public IP by DHCP", "Enable ARP Proxy");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);

		if(!strcmp(json_object_get_string(json_object_object_get(obj, "ipAlias_IPAddress")),""))
			IPAddress = "N/A";
		else
			IPAddress = json_object_get_string(json_object_object_get(obj, "ipAlias_IPAddress"));
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "ipAlias_SubnetMask")),""))
			SubnetMask = "N/A";
		else
			SubnetMask = json_object_get_string(json_object_object_get(obj, "ipAlias_SubnetMask"));
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "PubLan_IP")),""))
			X_ZYXEL_PubLan_IP = "N/A";
		else
			X_ZYXEL_PubLan_IP = json_object_get_string(json_object_object_get(obj, "PubLan_IP"));
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "PubLan_SubnetMask")),""))
			X_ZYXEL_PubLan_Mask = "N/A";
		else
			X_ZYXEL_PubLan_Mask = json_object_get_string(json_object_object_get(obj, "PubLan_SubnetMask"));

		printf("%-15s %-10d %-15s %-15s %-10d %-15s %-15s %-25d %-20d\n",
			json_object_get_string(json_object_object_get(obj, "GroupName")),
			json_object_get_boolean(json_object_object_get(obj, "ipAlias_Enable")),
			IPAddress,
			SubnetMask,
			json_object_get_boolean(json_object_object_get(obj, "PubLan_Enable")),
			X_ZYXEL_PubLan_IP,
			X_ZYXEL_PubLan_Mask,
			json_object_get_boolean(json_object_object_get(obj, "PubLan_DHCP")),
			json_object_get_boolean(json_object_object_get(obj, "PubLan_ARP")));
#ifdef ZYXEL_IP_UNNUMBER
	printf("%-10s %-10s %-15s %-15s %-10s %-15s %-15s %-25s\n",
		    "SubnetState", "IP_unmber_Enable", "Interface", "Multiple_interface", "MinAddress", "MaxAddress", "SubnetMask", "LANIP");
	printf("%-10s %-10d %-15s %-15s %-10s %-15s %-15s %-25s\n",
			json_object_get_string(json_object_object_get(obj, "SubnetState")),
			json_object_get_boolean(json_object_object_get(obj, "IP_unmber_Enable")),
			json_object_get_string(json_object_object_get(obj, "Interface")),
			json_object_get_string(json_object_object_get(obj, "Multiple_interface")),
			json_object_get_string(json_object_object_get(obj, "MinAddress")),
			json_object_get_string(json_object_object_get(obj, "MaxAddress")),
			json_object_get_string(json_object_object_get(obj, "SubnetMask")),
			json_object_get_string(json_object_object_get(obj, "LANIP")));
#endif
	}
}

zcfgRet_t setIPAlias(struct json_object *Jobj, objIndex_t *ipIntfIid){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ipAliasObj = NULL;
	struct json_object *ipv4AddrObj = NULL;
	objIndex_t ipAliasIid = {0};
	objIndex_t ipv4AddrIid = {0};
	bool aliasEnable = false, curAlias = false;
	bool found = false, setParam = false;
	const char *aliasAddr = NULL, *curAliasAddr = NULL;
	const char *aliasSubMask = NULL, *curAliasSubMask = NULL;
	char *X_ZYXEL_Dhcp4Subnet_Ref = NULL;
#ifdef ZYXEL_SECOND_LAN_IP_ALIAS
	objIndex_t ipAliasIid_2 = {0};
	bool aliasEnable_2 = false, curAlias_2 = false;
	bool found_2 = false, setParam_2 = false, onesetting = false;
	const char *aliasAddr_2 = NULL, *curAliasAddr_2 = NULL;
	const char *aliasSubMask_2 = NULL, *curAliasSubMask_2 = NULL;
	aliasEnable_2 = json_object_get_boolean(json_object_object_get(Jobj, "ipAlias_Enable_2"));
#endif
	aliasEnable = json_object_get_boolean(json_object_object_get(Jobj, "ipAlias_Enable"));

	IID_INIT(ipAliasIid);
	IID_INIT(ipv4AddrIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE_V4_ADDR, &ipv4AddrIid, &ipv4AddrObj) == ZCFG_SUCCESS) {
		curAlias = json_object_get_boolean(json_object_object_get(ipv4AddrObj, "X_ZYXEL_Alias"));
//		X_ZYXEL_Dhcp4Subnet_Ref = (char *)json_object_get_string(json_object_object_get(ipv4AddrObj, "X_ZYXEL_Dhcp4Subnet_Ref"));
#ifdef ZYXEL_IP_UNNUMBER		
		X_ZYXEL_Dhcp4Subnet_Ref = json_object_get_string(json_object_object_get(ipv4AddrObj, "X_ZYXEL_Dhcp4Subnet_Ref"));
#else
		X_ZYXEL_Dhcp4Subnet_Ref = (char *)json_object_get_string(json_object_object_get(ipv4AddrObj, "X_ZYXEL_Dhcp4Subnet_Ref"));
#endif
		if((ipv4AddrIid.idx[0] != ipIntfIid->idx[0]) || (curAlias == false) || (strstr(X_ZYXEL_Dhcp4Subnet_Ref, "DHCPv4.Server.X_ZYXEL_Subnet"))){
			//should prevent double free
			zcfgFeJsonObjFree(ipv4AddrObj);
			continue;
		}
		#ifdef ZYXEL_SECOND_LAN_IP_ALIAS
		if(ipv4AddrIid.idx[1] == 3)
			break;
		#endif
		curAliasAddr = json_object_get_string(json_object_object_get(ipv4AddrObj, "IPAddress"));
		curAliasSubMask = json_object_get_string(json_object_object_get(ipv4AddrObj, "SubnetMask"));
		found = true;
		memcpy(&ipAliasIid, &ipv4AddrIid, sizeof(objIndex_t));
		ipAliasObj = ipv4AddrObj;
		break;                                          
	}
	if(json_object_object_get(Jobj, "ipAlias_Enable")){
	if(aliasEnable){
		aliasAddr = json_object_get_string(json_object_object_get(Jobj, "ipAlias_IPAddress"));
		aliasSubMask = json_object_get_string(json_object_object_get(Jobj, "ipAlias_SubnetMask"));

		if(found){
			if(strcmp(curAliasAddr, aliasAddr) || strcmp(curAliasSubMask, aliasSubMask)){
				setParam = true;
			}
		}else{//POST
			memcpy(&ipAliasIid, ipIntfIid, sizeof(objIndex_t));
			ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE_V4_ADDR, &ipAliasIid, NULL);
			if(ret == ZCFG_SUCCESS){
				zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipAliasIid, &ipAliasObj);
				setParam = true;
			}
		}
#ifdef ZYXEL_IP_UNNUMBER
		if(!json_object_object_get(Jobj, "X_ZYXEL_Dhcp4Subnet_Ref")){
			setParam = true;		
		}	
#endif
		if(setParam){
			json_object_object_add(ipAliasObj, "Enable", json_object_new_boolean(aliasEnable));
			json_object_object_add(ipAliasObj, "X_ZYXEL_Alias", json_object_new_boolean(true));
			if(json_object_object_get(Jobj, "ipAlias_IPAddress"))
			json_object_object_add(ipAliasObj, "IPAddress", json_object_new_string(aliasAddr));
			if(json_object_object_get(Jobj, "ipAlias_SubnetMask"))
			json_object_object_add(ipAliasObj, "SubnetMask", json_object_new_string(aliasSubMask));
#ifdef ZYXEL_IP_UNNUMBER
json_object_object_add(ipAliasObj, "X_ZYXEL_Dhcp4Subnet_Ref", json_object_new_string("X_Zyxel_PublicLAN_Enhanced"));
#endif
			zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &ipAliasIid, ipAliasObj, NULL);
			zcfgFeJsonObjFree(ipAliasObj);
#ifdef ZYXEL_SECOND_LAN_IP_ALIAS
			onesetting = true;
#endif
		}
	}else{
		zcfgFeJsonObjFree(ipAliasObj);
		ipv4AddrObj = NULL;
		if(found){
			ret = zcfgFeObjJsonDel(RDM_OID_IP_IFACE_V4_ADDR, &ipAliasIid, NULL);
		}
	}
	}

#ifdef ZYXEL_SECOND_LAN_IP_ALIAS
	//Add Second Alias IP
	if(!onesetting){
	aliasAddr_2 = json_object_get_string(json_object_object_get(Jobj, "ipAlias_IPAddress_2"));
	aliasSubMask_2 = json_object_get_string(json_object_object_get(Jobj, "ipAlias_SubnetMask_2"));

	IID_INIT(ipAliasIid_2);
	IID_INIT(ipv4AddrIid);
	ipv4AddrIid.level = 2;
	ipv4AddrIid.idx[0] = 1;
	ipv4AddrIid.idx[1] = 2;
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE_V4_ADDR, &ipv4AddrIid, &ipv4AddrObj) == ZCFG_SUCCESS) {
		curAlias_2 = json_object_get_boolean(json_object_object_get(ipv4AddrObj, "X_ZYXEL_Alias"));
		X_ZYXEL_Dhcp4Subnet_Ref = json_object_get_string(json_object_object_get(ipv4AddrObj, "X_ZYXEL_Dhcp4Subnet_Ref"));
		if((ipv4AddrIid.idx[0] != ipIntfIid->idx[0]) || (curAlias_2 == false) || (strstr(X_ZYXEL_Dhcp4Subnet_Ref, "DHCPv4.Server.X_ZYXEL_Subnet"))){
			zcfgFeJsonObjFree(ipv4AddrObj);
			continue;
		}
        if(ipv4AddrIid.idx[1] == 2 && (curAlias_2 == true)){
            zcfgFeJsonObjFree(ipv4AddrObj);
		    continue;
        }
		
		curAliasAddr_2 = json_object_get_string(json_object_object_get(ipv4AddrObj, "IPAddress"));
		curAliasSubMask_2 = json_object_get_string(json_object_object_get(ipv4AddrObj, "SubnetMask"));
		found_2 = true;
		memcpy(&ipAliasIid_2, &ipv4AddrIid, sizeof(objIndex_t));
		ipAliasObj = ipv4AddrObj;
		break;                                          
	}

	if(json_object_object_get(Jobj, "ipAlias_Enable_2")){
		if(aliasEnable_2){
			aliasAddr_2 = json_object_get_string(json_object_object_get(Jobj, "ipAlias_IPAddress_2"));
			aliasSubMask_2 = json_object_get_string(json_object_object_get(Jobj, "ipAlias_SubnetMask_2"));
			if(found_2){
				if(strcmp(curAliasAddr_2, aliasAddr_2) || strcmp(curAliasSubMask_2, aliasSubMask_2)){
					setParam_2 = true;
				}
			}else{//POST
				memcpy(&ipAliasIid_2, ipIntfIid, sizeof(objIndex_t));
				ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE_V4_ADDR, &ipAliasIid_2, NULL);
				if(ret == ZCFG_SUCCESS){
					zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipAliasIid_2, &ipAliasObj);
					setParam_2 = true;
				}
			}

			if(setParam_2){
				json_object_object_add(ipAliasObj, "Enable", json_object_new_boolean(aliasEnable_2));
				json_object_object_add(ipAliasObj, "X_ZYXEL_Alias", json_object_new_boolean(true));
				if(json_object_object_get(Jobj, "ipAlias_IPAddress_2"))
				json_object_object_add(ipAliasObj, "IPAddress", json_object_new_string(aliasAddr_2));
				if(json_object_object_get(Jobj, "ipAlias_SubnetMask_2"))
				json_object_object_add(ipAliasObj, "SubnetMask", json_object_new_string(aliasSubMask_2));

				zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &ipAliasIid_2, ipAliasObj, NULL);
				zcfgFeJsonObjFree(ipAliasObj);
			}
		}else{
			zcfgFeJsonObjFree(ipAliasObj);
			if(found_2){
				ret = zcfgFeObjJsonDel(RDM_OID_IP_IFACE_V4_ADDR, &ipAliasIid_2, NULL);
			}
		}
	}
	}
#endif	
#if 0
//should prevent double free
	zcfgFeJsonObjFree(ipv4AddrObj);
#endif
	return ret;
}

zcfgRet_t setPublicLan(struct json_object *Jobj, struct json_object *ipIntfObj, objIndex_t *ipIntfIid){
	zcfgRet_t ret = ZCFG_SUCCESS;
#if 0
	bool pubLan = false, curPubLan = false;
	bool pubLanDHCP = false, curPubDHCP = false;
	bool pubLanARP = false, curPubARP = false;
	const char *pubLanIP = NULL, *curPubLanIP = NULL;
	const char *pubLanMask = NULL, *curPubLanMask = NULL;
#endif
	bool pubLan = false;
	bool pubLanDHCP = false;
	bool pubLanARP = false;
	const char *pubLanIP = NULL;
	const char *pubLanMask = NULL;
#ifdef ZYXEL_IP_UNNUMBER
	int IPv4AddressNumberOfEntries = 0;
#endif	
	pubLan = json_object_get_boolean(json_object_object_get(Jobj, "PubLan_Enable"));
	pubLanIP = json_object_get_string(json_object_object_get(Jobj, "PubLan_IP"));
	pubLanMask = json_object_get_string(json_object_object_get(Jobj, "PubLan_SubnetMask"));
	pubLanDHCP = json_object_get_boolean(json_object_object_get(Jobj, "PubLan_DHCP"));
	pubLanARP = json_object_get_boolean(json_object_object_get(Jobj, "PubLan_ARP"));

	if(ipIntfObj){
#if 0
		curPubLan = json_object_get_boolean(json_object_object_get(ipIntfObj, "X_ZYXEL_PubLan"));
		curPubLanIP = json_object_get_string(json_object_object_get(ipIntfObj, "X_ZYXEL_PubLan_IP"));
		curPubLanMask = json_object_get_string(json_object_object_get(ipIntfObj, "X_ZYXEL_PubLan_Mask"));
		curPubDHCP = json_object_get_boolean(json_object_object_get(ipIntfObj, "X_ZYXEL_PubLan_DHCP"));
		curPubARP = json_object_get_boolean(json_object_object_get(ipIntfObj, "X_ZYXEL_PubLan_ARP"));
#endif
		if(json_object_object_get(Jobj, "PubLan_Enable"))
			json_object_object_add(ipIntfObj, "X_ZYXEL_PubLan", json_object_new_boolean(pubLan));
		if(json_object_object_get(Jobj, "PubLan_IP") && pubLan)
			json_object_object_add(ipIntfObj, "X_ZYXEL_PubLan_IP", json_object_new_string(pubLanIP));
		else if(!pubLan)
			json_object_object_add(ipIntfObj, "X_ZYXEL_PubLan_IP", json_object_new_string(""));
		if(json_object_object_get(Jobj, "PubLan_SubnetMask") && pubLan)
			json_object_object_add(ipIntfObj, "X_ZYXEL_PubLan_Mask", json_object_new_string(pubLanMask));
		else if(!pubLan)
			json_object_object_add(ipIntfObj, "X_ZYXEL_PubLan_Mask", json_object_new_string(""));
		if(json_object_object_get(Jobj, "PubLan_DHCP"))
			json_object_object_add(ipIntfObj, "X_ZYXEL_PubLan_DHCP", json_object_new_boolean(pubLanDHCP));
		else if(!pubLan)
			json_object_object_add(ipIntfObj, "X_ZYXEL_PubLan_DHCP", json_object_new_boolean(false));
		if(json_object_object_get(Jobj, "PubLan_ARP"))
			json_object_object_add(ipIntfObj, "X_ZYXEL_PubLan_ARP", json_object_new_boolean(pubLanARP));
		else if(!pubLan)
			json_object_object_add(ipIntfObj, "X_ZYXEL_PubLan_ARP", json_object_new_boolean(false));


#ifdef ZYXEL_IP_UNNUMBER
		IPv4AddressNumberOfEntries = 1;
		json_object_object_add(ipIntfObj, "IPv4AddressNumberOfEntries", json_object_new_int(IPv4AddressNumberOfEntries));
#endif	
			zcfgFeObjJsonSet(RDM_OID_IP_IFACE, ipIntfIid, ipIntfObj, NULL);
		zcfgFeJsonObjFree(ipIntfObj);
	}
	return ret;
}
#ifdef ZYXEL_IP_UNNUMBER
zcfgRet_t validateSubnetObj(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	bool newSubnetState;

	if(json_object_object_get(Jobj,"SubnetState")){
		newSubnetState = json_object_get_boolean(json_object_object_get(Jobj,"SubnetState"));
		if(newSubnetState){
			if(!(json_object_object_get(Jobj,"MinAddress")&&
				 json_object_object_get(Jobj,"MaxAddress")&&
				 json_object_object_get(Jobj,"SubnetMask")&&
				 json_object_object_get(Jobj,"LANIP"))){
				printf("When enabling subnet setting, all subnet related parameters should be entered.\n");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
		}
		if(!newSubnetState){
			if(json_object_object_get(Jobj,"MinAddress")||
				 json_object_object_get(Jobj,"MaxAddress")||
				 json_object_object_get(Jobj,"SubnetMask")||
				 json_object_object_get(Jobj,"LANIP")){
				printf("When disabling subnet setting, no subnet related parameter is allowed.\n");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
		}
	}
	else{
		if(json_object_object_get(Jobj,"MinAddress")||
			 json_object_object_get(Jobj,"MaxAddress")||
			 json_object_object_get(Jobj,"SubnetMask")||
			 json_object_object_get(Jobj,"LANIP")){
			printf("Please set \"SubnetState\" to 1 or 0.\n");
			ret = ZCFG_INVALID_PARAM_VALUE;
		}
	}
	return ret;
}


void prepareSubnetObj(struct json_object *Jobj){
	int index = 0;
	objIndex_t subnetIid = {0};
	struct json_object *subnetObj = NULL;

	const char *subnetIntf = NULL;
	uint8_t SubnetIdx =0;
	bool foundSubnet = false;
	bool curSubnetState = false;
	bool newSubnetState = false;

	const char *multiple_intf = NULL;
	const char *subnetInterface = NULL;
	
	IID_INIT(DHCPSubnetIid);	
	IID_INIT(subnetIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DHCPV4_SRV_SUBNET, &subnetIid, &subnetObj) == ZCFG_SUCCESS) {

			multiple_intf = json_object_get_string(json_object_object_get(subnetObj,"Multiple_interface"));	
			if((!strcmp(multiple_intf,"single"))||(!strcmp(multiple_intf,"multiple"))){
				foundSubnet = true;		
				break;
			}else{
				foundSubnet = false;
			}
	}
	
	if(foundSubnet){
		if(json_object_object_get(subnetObj,"Enable")){
			curSubnetState = json_object_get_boolean(json_object_object_get(subnetObj,"Enable"));
		}		
	}else
		curSubnetState = false;
	if(json_object_object_get(Jobj,"SubnetState")){

	newSubnetState = json_object_get_boolean(json_object_object_get(Jobj,"SubnetState"));
	}
//	if(!json_object_object_get(Jobj,"subnetObjAction")){
		if(!curSubnetState && !newSubnetState){
			json_object_object_add(Jobj, "subnetObjAction", json_object_new_string(""));
			subnetDHCPObjAction = "";
		}
		if(!curSubnetState && newSubnetState){
			json_object_object_add(Jobj, "subnetObjAction", json_object_new_string("POST"));
			subnetDHCPObjAction = "POST";
		}
		if(curSubnetState && !newSubnetState){
			json_object_object_add(Jobj, "subnetObjAction", json_object_new_string("DELETE"));
			subnetDHCPObjAction = "DELETE";
		}
		if(curSubnetState && newSubnetState){
			json_object_object_add(Jobj, "subnetObjAction", json_object_new_string("PUT"));
			subnetDHCPObjAction = "PUT";
		}
//	}
#if 0
	else{
		if(subnetDHCPObjAction == NULL){
			json_object_object_add(Jobj, "subnetObjAction", json_object_new_string(""));
			strcpy(subnetDHCPObjAction,"");
		}
	}
#endif
	if(!strcmp(subnetDHCPObjAction,"DELETE") || !strcmp(subnetDHCPObjAction,"PUT")){
		DHCPSubnetIid.level = 1;
		DHCPSubnetIid.idx[0] = subnetIid.idx[0];
	}
	if(!json_object_object_get(Jobj,"Interface")){
		if(newSubnetState){
			//json_object_object_add(Jobj, "Interface", json_object_new_string(ssidIntf));			
		}
	}else{
		subnetInterface = json_object_get_string(json_object_object_get(Jobj,"Interface"));
	}
	if(subnetObj) json_object_put(subnetObj);
		
}

zcfgRet_t cfgDHCPSubnetObject(struct json_object *Jobj, const char* action){

	zcfgRet_t ret = ZCFG_SUCCESS;
	bool SubnetState = false;
	bool IP_unumber_Enable = false;
	const char* Interface = NULL;
	const char* MinAddress = NULL;
	const char* MaxAddress = NULL;
	const char* SubnetMask = NULL;
	const char* LANIP = NULL;
	const char* subnetObjAction = NULL;
	const char* Multiple_interface = NULL; 	
	struct json_object *DHCPSubnetObj;
	objIndex_t DHCPSubnetIid2 = {0};

	IID_INIT(DHCPSubnetIid2);	
	DHCPSubnetObj = NULL;
	SubnetState = json_object_get_boolean(json_object_object_get(Jobj, "SubnetState"));
	IP_unumber_Enable = json_object_get_boolean(json_object_object_get(Jobj, "IP_unumber_Enable"));	
	Interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
	MinAddress = json_object_get_string(json_object_object_get(Jobj, "MinAddress"));
	MaxAddress = json_object_get_string(json_object_object_get(Jobj, "MaxAddress"));
	SubnetMask = json_object_get_string(json_object_object_get(Jobj, "SubnetMask"));
	LANIP = json_object_get_string(json_object_object_get(Jobj, "LANIP"));
	subnetObjAction = json_object_get_string(json_object_object_get(Jobj, "subnetObjAction"));
	Multiple_interface = json_object_get_string(json_object_object_get(Jobj, "Multiple_interface"));	
	if(!strcmp(subnetDHCPObjAction, "DELETE"))
	{
		zcfgFeObjJsonDel(RDM_OID_DHCPV4_SRV_SUBNET, &DHCPSubnetIid, NULL);
		zcfgFeJsonObjFree(DHCPSubnetObj);
			return ret;
	}
	else if(!strcmp(subnetDHCPObjAction, "POST"))
	{
		zcfgFeObjJsonAdd(RDM_OID_DHCPV4_SRV_SUBNET, &DHCPSubnetIid2, NULL);

		if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV_SUBNET, &DHCPSubnetIid2, &DHCPSubnetObj)) != ZCFG_SUCCESS) {
			zcfgFeJsonObjFree(DHCPSubnetObj);
			return ret;
		}
	}
	else if(!strcmp(subnetDHCPObjAction, "PUT"))
	{
		if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV_SUBNET, &DHCPSubnetIid, &DHCPSubnetObj)) != ZCFG_SUCCESS) {
			zcfgFeJsonObjFree(DHCPSubnetObj);			
			return ret;
		}
	}
	
			json_object_object_add(DHCPSubnetObj, "Enable", json_object_new_boolean(SubnetState));
			json_object_object_add(DHCPSubnetObj, "MinAddress", json_object_new_string(MinAddress));
			json_object_object_add(DHCPSubnetObj, "MaxAddress", json_object_new_string(MaxAddress));
			json_object_object_add(DHCPSubnetObj, "SubnetMask", json_object_new_string(SubnetMask));
			json_object_object_add(DHCPSubnetObj, "LANIP", json_object_new_string(LANIP));
			json_object_object_add(DHCPSubnetObj, "Interface", json_object_new_string(Interface));
			json_object_object_add(DHCPSubnetObj, "IP_unumber_Enable", json_object_new_boolean(IP_unumber_Enable));
			json_object_object_add(DHCPSubnetObj, "Multiple_interface", json_object_new_string(Multiple_interface));

	//printf("\n cfgDHCPSubnetObject:%s\n", json_object_to_json_string(DHCPSubnetObj));
	
	if(DHCPSubnetObj){
		if(!strcmp(subnetDHCPObjAction, "POST")){

		   //printf("\n\n POST ret %d:%d != ZCFG_SUCCESS......@\n",DHCPSubnetIid2.level,DHCPSubnetIid2.idx[0]);
		

			ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_SUBNET, &DHCPSubnetIid2, DHCPSubnetObj, NULL);
		}else if((!strcmp(subnetDHCPObjAction, "PUT"))||(!strcmp(subnetDHCPObjAction, "DELETE"))){
			ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_SUBNET, &DHCPSubnetIid, DHCPSubnetObj, NULL);
		}
		if(ret != ZCFG_SUCCESS)		
			printf("\n nzcfgFeObjJsonSet ret %d != ZCFG_SUCCESS@@@@\n",ret);			
			
	}
	//if(DHCPSubnetObj) json_object_put(DHCPSubnetObj);
	return ret;
}
#endif
zcfgRet_t zcfgFeDal_Addnl_Subnet_Edit(struct json_object *Jobj, char *replyMsg){
	struct json_object *ipIntfObj = NULL;
	objIndex_t ipIntfIid = {0};
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *GroupName = NULL, *srvName = NULL, *ifName = NULL;
#ifdef ZYXEL_IP_UNNUMBER
	const char* subnetObjAction = NULL;
#endif
	GroupName = json_object_get_string(json_object_object_get(Jobj, "GroupName"));
#ifdef ZYXEL_SECOND_LAN_IP_ALIAS
	const char *GroupName_2 = NULL;
	GroupName_2 = json_object_get_string(json_object_object_get(Jobj, "GroupName_2"));
#endif

	IID_INIT(ipIntfIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipIntfIid, &ipIntfObj) == ZCFG_SUCCESS) {
		srvName = json_object_get_string(json_object_object_get(ipIntfObj, "X_ZYXEL_SrvName"));
		ifName = json_object_get_string(json_object_object_get(ipIntfObj, "X_ZYXEL_IfName")); 
		if((!strcmp(GroupName, "Default") || !strcmp(GroupName, srvName)) &&
			!strncmp(ifName, "br", 2)){
			break;
		}                                           
#ifdef ZYXEL_SECOND_LAN_IP_ALIAS
		if((!strcmp(GroupName_2, "Default") || !strcmp(GroupName_2, srvName)) &&
			!strncmp(ifName, "br", 2)){
			break;
		}
#endif		
		zcfgFeJsonObjFree(ipIntfObj);
    }
	// For dalcmd to check ipv4 address and submask parameter.
	if(json_object_get_boolean(json_object_object_get(Jobj, "ipAlias_Enable"))){
		if(!json_object_object_get(Jobj, "ipAlias_IPAddress") || !json_object_object_get(Jobj, "ipAlias_SubnetMask")){
			strcpy(replyMsg, "IP Alias IPv4 address or subnet mask error.");
			zcfgFeJsonObjFree(ipIntfObj);
			return ZCFG_INVALID_PARAM_VALUE;
		}	
	}
	if(json_object_get_boolean(json_object_object_get(Jobj, "PubLan_Enable"))){
		if(!json_object_object_get(Jobj, "PubLan_IP") || !json_object_object_get(Jobj, "PubLan_SubnetMask")){
			strcpy(replyMsg, "Public LAN IPv4 address or subnet mask error.");
			zcfgFeJsonObjFree(ipIntfObj);
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}
#ifdef ZYXEL_IP_UNNUMBER
	subnetObjAction = json_object_get_string(json_object_object_get(Jobj, "subnetObjAction"));	
	prepareSubnetObj(Jobj);
#if 0	
	if((ret = validateSubnetObj(Jobj,NULL)) != ZCFG_SUCCESS){
		return ret;
	}
#endif	
	if(subnetDHCPObjAction != NULL){
		if((!strcmp(subnetDHCPObjAction, "POST")) || (!strcmp(subnetDHCPObjAction, "PUT")) || (!strcmp(subnetDHCPObjAction, "DELETE"))) {
				if((ret = cfgDHCPSubnetObject(Jobj, subnetDHCPObjAction)) != ZCFG_SUCCESS){
					printf("\n cfgDHCPSubnetObject, ret!=ZCFG_SUCCESS###\n");
					return ZCFG_INVALID_PARAM_VALUE;
					}
			}	
	}
#endif
	setIPAlias(Jobj, &ipIntfIid);
	setPublicLan(Jobj, ipIntfObj, &ipIntfIid);
	return ret;
}
	
zcfgRet_t zcfgFeDal_Addnl_Subnet_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *bringObj = NULL;
	struct json_object *WanLanList = NULL;
	struct json_object *reqObject = NULL;
	struct json_object *paramJobj = NULL;
	struct json_object *ipv4AddrObj = NULL;
	struct json_object *ipIfaceObj = NULL;
	objIndex_t bringIid = {0};
	objIndex_t ipv4AddrIid = {0};
	objIndex_t ipIfaceIid = {0};
	objIndex_t Iid = {0};
	const char *bridge_name = NULL;
	const char *wanlanlist_name = NULL;
	int i, len = 0;
#ifdef ZYXEL_SECOND_LAN_IP_ALIAS
	const char *ifName = NULL;
	int num = 0, num_1 = 0;
#endif
#ifdef ZYXEL_IP_UNNUMBER
	objIndex_t subnetIid = {0};
	struct json_object *subnetObj = NULL;
#endif
	getWanLanList(&WanLanList);
	len = json_object_array_length(WanLanList);
	while(zcfgFeObjJsonGetNext(RDM_OID_BRIDGING_BR, &bringIid, &bringObj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		bridge_name = json_object_get_string(json_object_object_get(bringObj, "X_ZYXEL_BridgeName"));
		json_object_object_add(paramJobj, "GroupName", json_object_new_string(bridge_name));
#ifdef ZYXEL_SECOND_LAN_IP_ALIAS
		json_object_object_add(paramJobj, "GroupName_2", json_object_new_string(bridge_name));
#endif		
		for(i=0;i<len;i++){
			reqObject = json_object_array_get_idx(WanLanList, i);
#ifndef ZYXEL_IP_UNNUMBER
			const char *ipIfacePath = json_object_get_string(json_object_object_get(reqObject, "IpIfacePath"));
			char ipIfaceIdx = 0;
			char n = sscanf(ipIfacePath, "IP.Interface.%hhu", &ipIfaceIdx);
#endif
			wanlanlist_name = json_object_get_string(json_object_object_get(reqObject, "Name"));
#ifdef ZYXEL_IP_UNNUMBER			
			if(!strcmp(bridge_name,wanlanlist_name)){
#else			
			if(n && !strcmp(bridge_name,wanlanlist_name)){
#endif
				IID_INIT(ipv4AddrIid);
				IID_INIT(Iid);
				ipv4AddrIid.level = 1;
#ifndef ZYXEL_IP_UNNUMBER					
				ipv4AddrIid.idx[0] = ipIfaceIdx;
#else
		ipv4AddrIid.idx[0] = i + 1;
#endif				
				while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &ipv4AddrIid, &Iid, &ipv4AddrObj) == ZCFG_SUCCESS){
					if(json_object_get_boolean(json_object_object_get(ipv4AddrObj, "X_ZYXEL_Alias"))
						&& strstr(json_object_get_string(json_object_object_get(ipv4AddrObj, "X_ZYXEL_Dhcp4Subnet_Ref")), "DHCPv4.Server.X_ZYXEL_Subnet") == NULL){
		                #ifdef ZYXEL_SECOND_LAN_IP_ALIAS
                        ifName = json_object_get_string(json_object_object_get(ipv4AddrObj, "X_ZYXEL_IfName"));
						//printf("ifName = %s, %s(%d)\n\r", ifName, __FUNCTION__, __LINE__);
						sscanf(ifName, "br%d:%d", &num, &num_1);
						//printf("num = %d, num_1 = %d, %s(%d)\n\r", num, num_1, __FUNCTION__, __LINE__);
						if(num_1 == 2){
							json_object_object_add(paramJobj, "ipAlias_Enable", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "Enable")));
							json_object_object_add(paramJobj, "ipAlias_IPAddress", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "IPAddress")));
							json_object_object_add(paramJobj, "ipAlias_SubnetMask", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "SubnetMask")));
							json_object_object_add(paramJobj, "ipAlias_Subnet_Ref", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "X_ZYXEL_Dhcp4Subnet_Ref")));
						}else{
							json_object_object_add(paramJobj, "ipAlias_Enable_2", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "Enable")));
							json_object_object_add(paramJobj, "ipAlias_IPAddress_2", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "IPAddress")));
							json_object_object_add(paramJobj, "ipAlias_SubnetMask_2", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "SubnetMask")));
						}
						#else
						json_object_object_add(paramJobj, "ipAlias_Enable", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "Enable")));
						json_object_object_add(paramJobj, "ipAlias_IPAddress", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "IPAddress")));
						json_object_object_add(paramJobj, "ipAlias_SubnetMask", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "SubnetMask")));
						json_object_object_add(paramJobj, "ipAlias_Subnet_Ref", JSON_OBJ_COPY(json_object_object_get(ipv4AddrObj, "X_ZYXEL_Dhcp4Subnet_Ref")));
						break;
						#endif
					}
					else{
						#ifdef ZYXEL_SECOND_LAN_IP_ALIAS
						json_object_object_add(paramJobj, "ipAlias_Enable", json_object_new_boolean(false));
						json_object_object_add(paramJobj, "ipAlias_IPAddress", json_object_new_string(""));
						json_object_object_add(paramJobj, "ipAlias_SubnetMask", json_object_new_string(""));
						json_object_object_add(paramJobj, "ipAlias_Subnet_Ref", json_object_new_string(""));
						json_object_object_add(paramJobj, "ipAlias_Enable_2", json_object_new_boolean(false));
						json_object_object_add(paramJobj, "ipAlias_IPAddress_2", json_object_new_string(""));
						json_object_object_add(paramJobj, "ipAlias_SubnetMask_2", json_object_new_string(""));
						#else
						json_object_object_add(paramJobj, "ipAlias_Enable", json_object_new_boolean(false));
						json_object_object_add(paramJobj, "ipAlias_IPAddress", json_object_new_string(""));
						json_object_object_add(paramJobj, "ipAlias_SubnetMask", json_object_new_string(""));
						json_object_object_add(paramJobj, "ipAlias_Subnet_Ref", json_object_new_string(""));
						#endif
					}
					zcfgFeJsonObjFree(ipv4AddrObj);
				}
				zcfgFeJsonObjFree(ipv4AddrObj);



				IID_INIT(ipIfaceIid);
				ipIfaceIid.level = 1;
#ifdef ZYXEL_IP_UNNUMBER				
				ipIfaceIid.idx[0] = i + 1;
#else 		
				ipIfaceIid.idx[0] = ipIfaceIdx;

#endif
				if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS){
					json_object_object_add(paramJobj, "PubLan_Enable", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_PubLan")));
					json_object_object_add(paramJobj, "PubLan_IP", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_PubLan_IP")));
					json_object_object_add(paramJobj, "PubLan_SubnetMask", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_PubLan_Mask")));
					json_object_object_add(paramJobj, "PubLan_DHCP", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_PubLan_DHCP")));
					json_object_object_add(paramJobj, "PubLan_ARP", JSON_OBJ_COPY(json_object_object_get(ipIfaceObj, "X_ZYXEL_PubLan_ARP")));
#ifdef ZYXEL_IP_UNNUMBER
					printf("ZYXEL_IP_UNNUMBER\n");
					zcfgFeJsonObjFree(ipIfaceObj);
#else
					zcfgFeJsonObjFree(ipIfaceObj);
#endif					
				}
			}				
		}
#ifdef ZYXEL_IP_UNNUMBER
		int subnetInterface = 0;
		IID_INIT(subnetIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DHCPV4_SRV_SUBNET, &subnetIid, &subnetObj) == ZCFG_SUCCESS){
			char *dhcp_interface;
			dhcp_interface = json_object_get_string(json_object_object_get(subnetObj, "Interface"));
				json_object_object_add(paramJobj, "SubnetState", JSON_OBJ_COPY(json_object_object_get(subnetObj, "Enable")));
				json_object_object_add(paramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(subnetObj, "Interface")));
				json_object_object_add(paramJobj, "Multiple_interface", JSON_OBJ_COPY(json_object_object_get(subnetObj, "Multiple_interface")));				
				json_object_object_add(paramJobj, "MinAddress", JSON_OBJ_COPY(json_object_object_get(subnetObj, "MinAddress")));
				json_object_object_add(paramJobj, "MaxAddress", JSON_OBJ_COPY(json_object_object_get(subnetObj, "MaxAddress")));
				json_object_object_add(paramJobj, "SubnetMask", JSON_OBJ_COPY(json_object_object_get(subnetObj, "SubnetMask")));
				json_object_object_add(paramJobj, "LANIP", JSON_OBJ_COPY(json_object_object_get(subnetObj, "LANIP")));	
				json_object_object_add(paramJobj, "IP_unumber_Enable", JSON_OBJ_COPY(json_object_object_get(subnetObj, "IP_unumber_Enable")));	
		}
#endif
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(bringObj);
	}

	zcfgFeJsonObjFree(WanLanList);

	return ret;
}

zcfgRet_t zcfgFeDalAddnlSubnet(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDal_Addnl_Subnet_Edit(Jobj, replyMsg);
	else if(!strcmp(method,"GET"))
		ret = zcfgFeDal_Addnl_Subnet_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}


