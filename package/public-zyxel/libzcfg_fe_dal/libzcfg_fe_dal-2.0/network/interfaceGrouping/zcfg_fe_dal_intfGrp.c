#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>

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
#include "zos_api.h"

#define MAX_NUM_OF_IP_INFO 128 //temp define
//#define JSON_OBJ_COPY(json_object) json_tokener_parse(json_object_to_json_string(json_object))
typedef struct s_IpRangeInfo{
	uint32_t Start;
	uint32_t End;
}s_IpRangeInfo;

typedef struct s_AllIpRangeInfo {
	int length;
	s_IpRangeInfo ipRangInfo[MAX_NUM_OF_IP_INFO];
}s_AllIpRangeInfo;


dal_param_t IntfGrp_param[] = {
	{"Index",				dalType_int, 	0, 0, NULL,	NULL,	dal_Edit|dal_Delete},
	{"Name",				dalType_string, 1, 32, NULL,NULL,	dal_Add},
	{"wanIntf",				dalType_IntfGrpWanList, 0, 0, NULL,	NULL,	dal_Add},
	{"lanIntf",				dalType_IntfGrpLanPortList, 0, 0, NULL,	NULL,	0},	
	{"critInfoLen",			dalType_int, 	0, 0, NULL,	NULL,	dalcmd_Forbid},
	{"type",				dalType_string, 1, 32, NULL,"lanIntf|mac|op60|op61|op125|vlangroup",	dal_Add},
	{"mac_value",			dalType_string, 0, 0, NULL,NULL,	0},
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
	{"apasSrcMac",			dalType_string, 0, 0, NULL,	NULL,	0},
#endif
	{"op60_value",			dalType_string, 0, 0, NULL,NULL,	0},
	{"op61_value",			dalType_string, 0, 0, NULL,NULL,	0},
	{"op125_value",			dalType_string, 0, 0, NULL,NULL,	0},	//need delete and add "Enterprise_Number, OUI, SN, Class"
	{"Enterprise_Number",	dalType_int,	0,	65535	,NULL,	NULL,	0},
	{"OUI",					dalType_string,	0,	0,	NULL,	NULL,	0},
	{"SN",					dalType_string,	0,	0,	NULL,	NULL,	0},	
	{"Class",				dalType_string,	0,	0,	NULL,	NULL,	0},
	{"vlangroup",			dalType_string, 0, 0, NULL,NULL,	0},
	{"wildcardEnable",		dalType_boolean,0, 0, NULL,NULL,	0},
	{"critInfo",			dalType_string, 0, 0, NULL,	NULL,	dalcmd_Forbid},
	{"__modify", 			dalType_string, 0, 0, NULL,	NULL,	dalcmd_Forbid},
	{"org", 				dalType_string, 0, 0, NULL,	NULL,	dalcmd_Forbid},
	{"critInfoDelLen",		dalType_int, 	0, 0, NULL,	NULL,	dalcmd_Forbid},
	{"critInfoDel",			dalType_string, 0, 0, NULL,	NULL,	dalcmd_Forbid},	
	{NULL,		0,	0,	0,	NULL}
};

extern uint32_t StrToIntConv(char *str);

/*convert the Ipv4 string to integer (32bits)*/
uint32_t Ipv4AddrtoInt(char *Ipv4_str) {
	char* pch;
	int check_cnt = 0;
	uint32_t Ipv4_int = 0;
	char tmp[20] = {0};
	char *rest_str = NULL;

	memcpy(tmp, Ipv4_str, strlen(Ipv4_str)+1);
	pch = strtok_r(tmp, ".", &rest_str);
	while(pch!=NULL) {
		Ipv4_int += ((StrToIntConv(pch))<<((3-check_cnt)<<3));
		pch = strtok_r(rest_str, ".", &rest_str);
		check_cnt++;
		if(check_cnt>4) {
			printf("Ipv4AddrtoInt: Error!\n");
			printf("Ipv4_str: %s\n", Ipv4_str);
					break;
				}
			}
	return Ipv4_int;
	}

	
	
#if 1
zcfgRet_t FindIPidx(char* newIP, objIndex_t newBrIid)
#else
zcfgRet_t FindIPidx(int* IP_idx, objIndex_t newBrIid)
#endif
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	json_object* v4AddrObj = NULL;
	objIndex_t v4AddrIid;
	s_AllIpRangeInfo avlIpRange;
	uint32_t IPAddress;
	uint32_t SubnetMask;
	int i = 0;
	int ipIndex = 0;
	uint32_t ipS = 0, ipE = 0;

#if 1
	int ipIdx1 = 192, ipIdx2 = 168;
#endif

	memset(&avlIpRange, 0, sizeof(avlIpRange));
	IID_INIT(v4AddrIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj) == ZCFG_SUCCESS) {
		IPAddress = Ipv4AddrtoInt((char*)json_object_get_string(json_object_object_get(v4AddrObj, "IPAddress")));
		SubnetMask = Ipv4AddrtoInt((char*)json_object_get_string(json_object_object_get(v4AddrObj, "SubnetMask")));
		avlIpRange.ipRangInfo[i].Start = IPAddress & SubnetMask;
		avlIpRange.ipRangInfo[i].End = (avlIpRange.ipRangInfo[i].Start) | (~SubnetMask);
		avlIpRange.length ++;
		if(avlIpRange.length > MAX_NUM_OF_IP_INFO) {
			printf("FindIPidx Error: avlIpRange,length is larger than %d\n", MAX_NUM_OF_IP_INFO);
			zcfgFeJsonObjFree(v4AddrObj);
			break;
		}
		i++;
		zcfgFeJsonObjFree(v4AddrObj);
	}

	/*make sure the ipS~ipE not be used in avlIpRange*/
	ipIndex = newBrIid.idx[0];
	
#if 1
	while(1)
	{
		ipS = (ipIdx1 << 24) + (ipIdx2 << 16) + (ipIndex << 8) + 0;
		ipE = ipS + 255;
		for(i = 0; i < avlIpRange.length; i++)
		{
			if((avlIpRange.ipRangInfo[i].Start <= ipS && ipS <= avlIpRange.ipRangInfo[i].End) &&
				(avlIpRange.ipRangInfo[i].Start <= ipE && ipE <= avlIpRange.ipRangInfo[i].End))
				break;
		}
		if(i >= avlIpRange.length)
			break;
		if(ipIdx2 < 255 && ipIndex >= 255)
		{
			//192.168.255.X
			ipIdx2++;
			ipIndex = 1;
		}
		else if(ipIdx2 >= 255 && ipIndex >= 255)
		{
			//192.255.255.X
			ipIdx1++;
			ipIdx2 = 1;
			ipIndex = 1;
		}
		else
		{
			//192.168.X.X
			ipIndex++;
		}
	}
	sprintf(newIP, "%d.%d.%d.1", ipIdx1, ipIdx2, ipIndex);
#else

	while(ipIndex <= 255) {
		ipS = (192<<24) + (168<<16) + (ipIndex<<8) + 0;
		ipE = ipS + 255;
		for(i=0; i<avlIpRange.length; i++) {
			if((avlIpRange.ipRangInfo[i].Start <= ipS && ipS <= avlIpRange.ipRangInfo[i].End) &&
				(avlIpRange.ipRangInfo[i].Start <= ipE && ipE <= avlIpRange.ipRangInfo[i].End))
					break;
		}
		if(i >= avlIpRange.length)
			break;
		ipIndex++;
	}
	*IP_idx = ipIndex;
#endif
	return ret;
}

bool dalcmdFindObjIid(zcfg_offset_t RDM_OID, int index, objIndex_t* Iid, json_object** Obj) {
	bool find;

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID, Iid, Obj) == ZCFG_SUCCESS) {
		find = true;
		if(Iid->idx[0] != index) {
					find = false;
					}
		if(find == true) {
								break;
							}
						}
	if(find == false) {
		printf("Can't find the target Iid in RDM %d\n", RDM_OID);
					}
				
	dbg_printf("dalcmdFindObjIid: find = %d, idx[0] = %d\n", find, Iid->idx[0]);	
	return find;
			}


zcfgRet_t zcfgFeDalIntrGrpDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	bool find;
	int BrIdx = 0;
	char BrStr[4] = {0};
	char snoopingList[100] = {0};
	const char *snoopingStr = NULL;
	char *single_snpStr = NULL;
	char buff[100] = {0};
	char *rest_str = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	int BrRefKey;
	json_object *vlanGroupObj = NULL;
	objIndex_t vlanGroupIid = {0};
#endif
	json_object *BrObj = NULL;
	json_object *igmpObj = NULL;
	json_object *mldObj = NULL;
	objIndex_t BrIid = {0};
	objIndex_t igmpIid = {0};
	objIndex_t mldIid = {0};

	IID_INIT(BrIid);
	IID_INIT(igmpIid);
	IID_INIT(mldIid);
	int index = 0, len = 0;

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	IID_INIT(vlanGroupIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &vlanGroupIid, &vlanGroupObj) == ZCFG_SUCCESS) {
		BrRefKey = (int)json_object_get_int(json_object_object_get(vlanGroupObj, "BrRefKey"));
		if(BrRefKey == index) {
			json_object_object_add(vlanGroupObj, "BrRefKey", json_object_new_string(""));
			zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &vlanGroupIid, vlanGroupObj, NULL);
			zcfgFeJsonObjFree(vlanGroupObj);
			break;
		}
		zcfgFeJsonObjFree(vlanGroupObj);
	}
#endif
	find = dalcmdFindObjIid(RDM_OID_BRIDGING_BR, index, &BrIid, &BrObj);
	if(find == true) {
		zcfgFeObjJsonDel(RDM_OID_BRIDGING_BR, &BrIid, NULL);
		zcfgFeJsonObjFree(BrObj);
	}
	
	BrIdx = index-1;
	sprintf(BrStr,"br%d",BrIdx);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IGMP, &igmpIid, &igmpObj);
	if(ret == ZCFG_SUCCESS){
		snoopingStr = json_object_get_string(json_object_object_get(igmpObj, "SnoopingBridgeIfName"));
		strcpy(buff, snoopingStr);
		single_snpStr = strtok_r(buff, ",", &rest_str);		
		while(single_snpStr != NULL){
			if(strncmp(single_snpStr,BrStr,3)){
				strcat(snoopingList, single_snpStr);
				strcat(snoopingList, ",");
			}
			single_snpStr = strtok_r(NULL, ",", &rest_str);
		}
		len = strlen(snoopingList);
		if(snoopingList[len-1] == ',')
				snoopingList[len-1] = '\0';
		json_object_object_add(igmpObj, "SnoopingBridgeIfName", json_object_new_string(snoopingList));
		zcfgFeObjJsonSet(RDM_OID_ZY_IGMP, &igmpIid, igmpObj, NULL);
		zcfgFeJsonObjFree(igmpObj);
	}
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MLD, &mldIid, &mldObj);
	if(ret == ZCFG_SUCCESS){
		json_object_object_add(mldObj, "SnoopingBridgeIfName", json_object_new_string(snoopingList));
		zcfgFeObjJsonSet(RDM_OID_ZY_MLD, &mldIid, mldObj, NULL);
		zcfgFeJsonObjFree(mldObj);
	}

	if(igmpObj) zcfgFeJsonObjFree(igmpObj);
	if(mldObj) zcfgFeJsonObjFree(mldObj);

	return ret;
}

zcfgRet_t zcfgFeDalIntrGrpGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *bridgeJobj = NULL;
	struct json_object *bridgeportObj = NULL;
	struct json_object *bridgefilterObj = NULL;
	struct json_object *obj = NULL;
	struct json_object *brobj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	struct json_object *vlanObj = NULL;
	objIndex_t vlanIid = {0};
	int vlanID = 0;
	const char *VlanName = NULL;
#endif
	objIndex_t bridgeIid = {0};
	objIndex_t ipifaceIid = {0};
	objIndex_t bridgeportIid = {0};
	objIndex_t bridgefilterIid = {0};
	const char *Group_Name = NULL;
	const char *LAN_IpIface = NULL;
	const char *bridge = NULL;
	const char *SourceMACFromVSIFilter = NULL;
	const char *SourceMACFromClientIDFilter = NULL;
	const char *SourceMACFromVendorClassIDFilter = NULL;
	const char *SourceMACAddressFilterList = NULL;
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
	const char *X_ZYXEL_APASSourceMACAFilter = NULL;
#endif
	bool enable_criteria = false;
	int len = 0, i, idx = 0;
	getWanLanList(&obj);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_BR, &bridgeIid, &bridgeJobj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		Group_Name = json_object_get_string(json_object_object_get(bridgeJobj, "X_ZYXEL_BridgeName"));
		json_object_object_add(paramJobj, "Index", json_object_new_int(bridgeIid.idx[0]));	
		json_object_object_add(paramJobj, "Group_Name", json_object_new_string(Group_Name));
		IID_INIT(ipifaceIid);
		/*if(!strcmp(Group_Name,"Default"))
			json_object_object_add(paramJobj, "wanIntf", json_object_new_string("Any WAN"));*/
		len = json_object_array_length(obj);
		for(i=0;i<len;i++){
			brobj = json_object_array_get_idx(obj, i);
			if(!strcmp(Group_Name,json_object_get_string(json_object_object_get(brobj, "Name")))){
				json_object_object_add(paramJobj, "wanIntf", JSON_OBJ_COPY(json_object_object_get(brobj, "Group_WAN_IpIface")));
			}
		}
		IID_INIT(bridgeportIid);
		bridgeportIid.idx[0] = bridgeIid.idx[0];
		bridgeportIid.level = 3;
		char tmp[256] = {0};
		char LAN[256] = {0};
		char criinfo[2048] = {0};
		char vlanGroupIndex[200] = {0};

		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_BR_PORT, &bridgeportIid, &bridgeportObj) == ZCFG_SUCCESS){
			if(bridgeportIid.idx[1] != 1 && bridgeportIid.idx[0] == bridgeIid.idx[0]){
				LAN_IpIface = json_object_get_string(json_object_object_get(bridgeportObj, "LowerLayers"));
				strcat(tmp, LAN_IpIface);
				strcat(tmp, ",");
				ZOS_STRNCPY(LAN, tmp, sizeof(LAN));
			}
#ifdef ZYXEL_USA_PRODUCT
			if(bridgeportIid.idx[0] == bridgeIid.idx[0] && json_object_get_boolean(json_object_object_get(bridgeportObj, "ManagementPort")))
			{
				//printf("%s %d #<%d> <%s> isolated <%d>\n", __FUNCTION__, __LINE__, bridgeIid.idx[0], Group_Name, json_object_get_boolean(json_object_object_get(bridgeportObj, "X_ZYXEL_BridgeIsolated")));
				json_object_object_add(paramJobj, "isolateBridge", JSON_OBJ_COPY(json_object_object_get(bridgeportObj, "X_ZYXEL_BridgeIsolated")));
			}
#endif
			zcfgFeJsonObjFree(bridgeportObj);
		}
		{
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
			int vlanGroupIndexLen = 0;
			char *vlanGroupIdx = vlanGroupIndex;
			IID_INIT(vlanIid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &vlanIid, &vlanObj) == ZCFG_SUCCESS){
				vlanID = json_object_get_int(json_object_object_get(vlanObj, "BrRefKey"));
				if(bridgeIid.idx[0] == vlanID){
					VlanName = json_object_get_string(json_object_object_get(vlanObj, "GroupName"));
					sprintf(criinfo, "vlangroup<%s>", VlanName);
					if((vlanGroupIndexLen = strlen(vlanGroupIndex)) == 0){
						sprintf(vlanGroupIdx, "%hhu", vlanIid.idx[0]);
					}else{
						char *pos = vlanGroupIdx + vlanGroupIndexLen;
						sprintf(pos, ",%hhu", vlanIid.idx[0]);
					}
				}
				zcfgFeJsonObjFree(vlanObj);
			}
#endif
			if(!strcmp(criinfo,"")){
				IID_INIT(bridgefilterIid);
				while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_FILTER, &bridgefilterIid, &bridgefilterObj) == ZCFG_SUCCESS){
					enable_criteria = json_object_get_boolean(json_object_object_get(bridgefilterObj, "X_ZYXEL_Enable_Criteria"));
					if(enable_criteria){
						bridge = json_object_get_string(json_object_object_get(bridgefilterObj, "Bridge"));
						sprintf(tmp, "Bridging.Bridge.%d",bridgeIid.idx[0]);
						if(!strcmp(tmp, bridge)){
							char op125tmp[128] = {0};
							SourceMACFromVSIFilter = json_object_get_string(json_object_object_get(bridgefilterObj, "X_ZYXEL_SourceMACFromVSIFilter"));
							if(strcmp(SourceMACFromVSIFilter,"")){
								char *OUI = NULL;
								char *SN = NULL;
								char *CLASS = NULL;
								char oui[64] = {0};
								char sn[64] = {0};
								char class[64] = {0};
								char oui1[64] = {0};
								char sn1[64] = {0};
								char class1[64] = {0};
								char sourcemac[128] = {0};
								
								char *p, *p1, *p2, *p3;			
								if(SourceMACFromVSIFilter != NULL){
									strcpy(sourcemac,SourceMACFromVSIFilter);
									p = strstr(sourcemac, "\%\%");
									if(p != NULL) 
										*p = '\0';
								}
								OUI = strstr(SourceMACFromVSIFilter, "\%\%1=");
								if(OUI)
									OUI = OUI+4;								
								SN = strstr(SourceMACFromVSIFilter, "\%\%2=");
								if(SN)
									SN = SN+4;
								CLASS = strstr(SourceMACFromVSIFilter, "\%\%3=");
								if(CLASS)
									CLASS = CLASS+4;
								if(OUI){
									if(strcmp(OUI,"")){
										strcpy(oui,OUI);
										p1 = strstr(oui, "\%\%");
										if(p1 != NULL) 
											*p1 = '\0';
										strcat(oui1, oui);
									}
								}
								if(SN){
									if(strcmp(SN,"")){
										strcpy(sn,SN);
										p2 = strstr(sn, "\%\%");
										if(p2 != NULL) 
											*p2 = '\0';
										strcat(sn1, sn);
									}
								}
								if(CLASS){
									if(strcmp(CLASS,"")){
										strcpy(class,CLASS);
										p3 = strstr(class, "\%\%");
										if(p3 != NULL) 
											*p3 = '\0';
										strcat(class1, class);
									}
								}
								strcat(op125tmp,sourcemac);
								
								if(OUI){
									if(strcmp(OUI,"")){
										sprintf(oui,",OUI=%s",oui1);
										strcat(op125tmp, oui);
									}
								}
								if(SN){
									if(strcmp(SN,"")){
										sprintf(sn,",SN=%s",sn1);	
										strcat(op125tmp, sn);
									}
								}
								if(CLASS){
									if(strcmp(CLASS,"")){
										sprintf(class,",CLASS=%s",class1);
										strcat(op125tmp, class);
									}
								}
							}							
							SourceMACFromClientIDFilter = json_object_get_string(json_object_object_get(bridgefilterObj, "SourceMACFromClientIDFilter"));
							SourceMACFromVendorClassIDFilter = json_object_get_string(json_object_object_get(bridgefilterObj, "SourceMACFromVendorClassIDFilter"));
							SourceMACAddressFilterList = json_object_get_string(json_object_object_get(bridgefilterObj, "SourceMACAddressFilterList"));
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
							X_ZYXEL_APASSourceMACAFilter = json_object_get_string(json_object_object_get(bridgefilterObj, "X_ZYXEL_APASSourceMACAFilter"));
#endif
							idx = bridgefilterIid.idx[0];
							if(strcmp(SourceMACFromVSIFilter,"")){
								if(strlen(criinfo)>0)
									strcat(criinfo,";");
								sprintf(criinfo+strlen(criinfo),"op125<%s>",op125tmp);
							}else if(strcmp(SourceMACFromClientIDFilter,"")){
								if(strlen(criinfo)>0)
									strcat(criinfo,";");
								sprintf(criinfo+strlen(criinfo),"op61<%s>",SourceMACFromClientIDFilter);
							}else if(strcmp(SourceMACFromVendorClassIDFilter,"")){
								if(strlen(criinfo)>0)
									strcat(criinfo,";");
								sprintf(criinfo+strlen(criinfo),"op60<%s>",SourceMACFromVendorClassIDFilter);
							}
							else if(strcmp(SourceMACAddressFilterList,"")){
								if(strlen(criinfo)>0)
									strcat(criinfo,";");

								sprintf(criinfo+strlen(criinfo),"mac<%s>",SourceMACAddressFilterList);
							}
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
							else if(strcmp(X_ZYXEL_APASSourceMACAFilter,"")){
								if(strlen(criinfo)>0)
									strcat(criinfo,";");
							
								sprintf(criinfo+strlen(criinfo),"mac<%s>",X_ZYXEL_APASSourceMACAFilter);
							}
#endif
						}
					}
					zcfgFeJsonObjFree(bridgefilterObj);
				}
			}
		}
		json_object_object_add(paramJobj, "idx", json_object_new_int(idx));
		json_object_object_add(paramJobj, "lanIntf", json_object_new_string(LAN));
		json_object_object_add(paramJobj, "Criteria", json_object_new_string(criinfo));
		if(strlen(vlanGroupIndex))
			json_object_object_add(paramJobj, "vlangroupnum", json_object_new_string(vlanGroupIndex));
		json_object_array_add(Jarray, JSON_OBJ_COPY(paramJobj));
		zcfgFeJsonObjFree(paramJobj);

		zcfgFeJsonObjFree(bridgeJobj);
	}

	zcfgFeJsonObjFree(obj);

	return ret;
}
zcfgRet_t GUICeireriaReconstruct(struct json_object *Jobj, char *replyMsg);
zcfgRet_t CLICeireriaReconstruct(struct json_object *Jobj, char *replyMsg, char *method);
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
void dalcmdcirteriaToObject(json_object *BrFilterObj, char *brPathName, char *type, char *macvalue, char *op60value, char *op61value, char *op125tmp, char *wildcardEnable, char *apasSrcMac);
#else
void dalcmdcirteriaToObject(json_object *BrFilterObj, char *brPathName, char *type, char *macvalue, char *op60value, char *op61value, char *op125tmp, char *wildcardEnable);
#endif
zcfgRet_t dalcmdIntrGrpAdd(struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
#if 1
	char newIPAddress[16] = {0};
	struct in_addr ip, mask, startip, endip;
#else
	int IP_idx;
#endif	
	char IPAddress_str1[60] = {0};
	char tmp[10] = {0};
	char SubnetMask_str[30] = {0};
	char newbrPathName[60] = {0};
	char BrIdx = 0;
	const char *bridgename = NULL;
	const char *wanintf = NULL;
	const char *lanintf = NULL;
	char *type = NULL;
	const char *macvalue = NULL;
	const char *op60value = NULL;
	const char *op61value = NULL;
	const char *op125value = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	const char *vlanvalue = NULL;
#endif
	const char *wildcardenable = NULL;
	const char *cmd = NULL;
	const char *act = NULL;
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
	const char *apasSrcMac = NULL;
#endif
	bool wildcard = false;
	
	//int criteriaNum = 0;
	int count = 0;
	char cnt[4] = {0};
	
	json_object *v4AddrObj = NULL;
	json_object *newBrObj = NULL;
	json_object *newEthLinkObj = NULL;
	json_object *newIpObj = NULL;
	json_object *newBrFilterObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	json_object *vlanGroupObj = NULL;
	objIndex_t vlanGroupIid = {0};
#endif
	json_object *dhcpV4SrvObj = NULL;
	json_object *newBrPortObj = NULL;
	json_object *igmpObj = NULL;
	json_object *mldObj = NULL;
	objIndex_t v4AddrIid = {0};
	objIndex_t newBrIid = {0};
	objIndex_t newEthLinkIid = {0};
	objIndex_t newIpIid = {0};
	objIndex_t newBrFilterIid = {0};
	objIndex_t dhcpV4SrvIid = {0};
	objIndex_t newBrPortIid = {0};
	objIndex_t igmpIid = {0};
	objIndex_t mldIid = {0};

	IID_INIT(v4AddrIid);
	IID_INIT(newBrIid);
	IID_INIT(newEthLinkIid);
	IID_INIT(newIpIid);
	IID_INIT(newBrFilterIid);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	IID_INIT(vlanGroupIid);
#endif
	IID_INIT(dhcpV4SrvIid);
	IID_INIT(igmpIid);
	IID_INIT(mldIid);

	dbg_printf("zcfgFeDalIntrGrpAdd\n");

	if(json_object_object_get(Jobj, "critInfo"))
		GUICeireriaReconstruct(Jobj, replyMsg);
	if(json_object_object_get(Jobj, "type"))
		CLICeireriaReconstruct(Jobj, replyMsg, NULL);
	//return ret;


	// Get parameter but no criteria
	bridgename = json_object_get_string(json_object_object_get(Jobj, "Name"));
	wanintf = json_object_get_string(json_object_object_get(Jobj, "wanIntf"));
	if(json_object_get_string(json_object_object_get(Jobj, "lanIntf")))
		lanintf = json_object_get_string(json_object_object_get(Jobj, "lanIntf"));
	else
		lanintf = "";

	//GET RDM_OID_IP_IFACE_V4_ADDR
	/*======================================================================*/

	//printf("1. POST RDM_OID_BRIDGING_BR\n");
	// POST RDM_OID_BRIDGING_BR
	if((ret = zcfgFeObjJsonAdd(RDM_OID_BRIDGING_BR, &newBrIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_BRIDGING_BR, &newBrIid, &newBrObj)) != ZCFG_SUCCESS) {
		return ret;
	}
	json_object_object_add(newBrObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(newBrObj, "X_ZYXEL_BridgeName", json_object_new_string(bridgename));

	zcfgFeObjJsonSetWithoutApply(RDM_OID_BRIDGING_BR, &newBrIid, newBrObj, NULL);
	/*===============================================================================*/


	//printf("2. POST RDM_OID_ETH_LINK\n");

	// POST RDM_OID_ETH_LINK
	if((ret = zcfgFeObjJsonAdd(RDM_OID_ETH_LINK, &newEthLinkIid, NULL)) != ZCFG_SUCCESS) {
		printf("2. POST RDM_OID_ETH_LINK zcfgFeObjJsonAdd ERROR!!\n");
		return ret;
	}
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ETH_LINK, &newEthLinkIid, &newEthLinkObj)) != ZCFG_SUCCESS) {
		printf("2. POST RDM_OID_ETH_LINK zcfgFeObjJsonGetWithoutUpdate ERROR!!\n");
		return ret;
	}
	/*===============================================================================*/

	//printf("3. POST RDM_OID_IP_IFACE\n");
	// POST RDM_OID_IP_IFACE
	if((ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE, &newIpIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &newIpIid, &newIpObj)) != ZCFG_SUCCESS) {
		return ret;
	}
	/*===============================================================================*/


	//printf("4. POST RDM_OID_BRIDGING_BR_PORT\n");
	// POST RDM_OID_BRIDGING_BR_PORT
	memcpy(&newBrPortIid, &newBrIid, sizeof(objIndex_t));
	if((ret = zcfgFeObjJsonAdd(RDM_OID_BRIDGING_BR_PORT, &newBrPortIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_BRIDGING_BR_PORT, &newBrPortIid, &newBrPortObj)) != ZCFG_SUCCESS) {
		return ret;
	}
	json_object_object_add(newBrPortObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(newBrPortObj, "ManagementPort", json_object_new_boolean(true));
#ifdef ZYXEL_USA_PRODUCT
	bool isolateBridge = json_object_get_boolean(json_object_object_get(Jobj, "isolateBridge"));
	json_object_object_add(newBrPortObj, "X_ZYXEL_BridgeIsolated", json_object_new_boolean(isolateBridge));	
#endif
	zcfgFeObjJsonBlockedSet(RDM_OID_BRIDGING_BR_PORT, &newBrPortIid, newBrPortObj, NULL);
	/*===============================================================================*/

	char Br_str[64] = {0};
	sprintf(Br_str,"Bridging.Bridge.%d.Port.1",newBrIid.idx[0]);

	json_object_object_add(newEthLinkObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(newEthLinkObj, "LowerLayers", json_object_new_string(Br_str));

	zcfgFeObjJsonBlockedSet(RDM_OID_ETH_LINK, &newEthLinkIid, newEthLinkObj, NULL);
	/*==============================================================================*/

	char Eth_str[60] = {0};
	sprintf(Eth_str,"Ethernet.Link.%d",newEthLinkIid.idx[0]);

	json_object_object_add(newIpObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(newIpObj, "IPv4Enable", json_object_new_boolean(true));
	json_object_object_add(newIpObj, "LowerLayers", json_object_new_string(Eth_str));
	json_object_object_add(newIpObj, "X_ZYXEL_Group_WAN_IpIface", json_object_new_string(wanintf)); //binding wan interface
	json_object_object_add(newIpObj, "X_ZYXEL_ConnectionType", json_object_new_string("IP_Routed"));  //inherit connection type of wan interface
        #ifdef TAAAB_HGW
	char newName[128] = {0};
	strcpy(newName, bridgename);
	strcat(newName,"_WAN");
	json_object_object_add(newIpObj, "X_TT_SrvName", json_object_new_string(newName));
	strcpy(newName, bridgename);
	strcat(newName,"_ETHWAN");
	json_object_object_add(newIpObj, "X_ZYXEL_SrvName", json_object_new_string(newName));  //inherit connection type of wan interface
	#else
	json_object_object_add(newIpObj, "X_ZYXEL_SrvName", json_object_new_string(bridgename));  //inherit connection type of wan interface
	#endif

	zcfgFeObjJsonBlockedSet(RDM_OID_IP_IFACE, &newIpIid, newIpObj, NULL);
    /*==============================================================================*/
#if 1
	if((ret = FindIPidx(newIPAddress, newBrIid)) != ZCFG_SUCCESS)
	{
		return ret;
	}
#else
	if((ret = FindIPidx(&IP_idx, newBrIid)) != ZCFG_SUCCESS) {
		return ret;
	}
#endif
	memcpy(&v4AddrIid, &newIpIid, sizeof(objIndex_t));
	if((ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj)) != ZCFG_SUCCESS) {
		return ret;
	}
#if 1
	strcpy(IPAddress_str1, newIPAddress);
#else
	sprintf(IPAddress_str1,"192.168.%d.1",IP_idx);
#endif
	strcat(SubnetMask_str, "255.255.255.0");
	json_object_object_add(v4AddrObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(v4AddrObj, "IPAddress", json_object_new_string(IPAddress_str1));
	json_object_object_add(v4AddrObj, "SubnetMask", json_object_new_string(SubnetMask_str));

	zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, v4AddrObj, NULL);
	/*===============================================================================*/
	//if(!json_object_object_get(Jobj, "lanIntf"))
	//	criteriaNum = json_object_get_int(json_object_object_get(Jobj, "criteriaNum"));

	strcat(newbrPathName, "Bridging.Bridge.");
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%d", newBrIid.idx[0]);
	strcat(newbrPathName, tmp);
	sprintf(cnt,"%d",count);
	obj = json_object_object_get(Jobj, cnt);
	while(obj != NULL){
		act = "";
		if(json_object_get_string(json_object_object_get(obj, "act"))){
			act = json_object_get_string(json_object_object_get(obj, "act"));
			if(!strcmp(act, "delete")){
				count ++;
				sprintf(cnt,"%d",count);
				obj = json_object_object_get(Jobj, cnt);
				continue;
			}
		}
		type = (char *)json_object_get_string(json_object_object_get(obj, "type"));
		if(!strcmp(type, "vlangroup")) {
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
			vlanvalue = json_object_get_string(json_object_object_get(obj, "vlangroup"));
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &vlanGroupIid, &vlanGroupObj) == ZCFG_SUCCESS){
				if(!strcmp(vlanvalue,json_object_get_string(json_object_object_get(vlanGroupObj, "GroupName")))){
					json_object_object_add(vlanGroupObj, "BrRefKey", json_object_new_int(newBrIid.idx[0]));
					zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &vlanGroupIid, vlanGroupObj, NULL);
					zcfgFeJsonObjFree(vlanGroupObj);
					break;
				}
				zcfgFeJsonObjFree(vlanGroupObj);
			}
#else
			printf("%s : vlan group not support\n", __func__);
#endif
		}
		else {
			cmd = json_object_get_string(json_object_object_get(Jobj, "cmdtype"));
			macvalue = json_object_get_string(json_object_object_get(obj, "mac_value"));
			op60value = json_object_get_string(json_object_object_get(obj, "op60_value"));
			op61value = json_object_get_string(json_object_object_get(obj, "op61_value"));
			op125value = json_object_get_string(json_object_object_get(obj, "op125value"));
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
			apasSrcMac = json_object_get_string(json_object_object_get(obj, "apasSrcMac"));
#endif
			if(!strcmp(cmd, "GUI")){
				wildcardenable = json_object_get_string(json_object_object_get(obj, "wildcardEnable"));
			}
			else if(!strcmp(cmd, "CLI")){
				wildcard = json_object_get_boolean(json_object_object_get(obj, "wildcardEnable"));
				if(wildcard)
					wildcardenable = "Y";
				else
					wildcardenable = "N";

			}
			IID_INIT(newBrFilterIid);
			if((ret = zcfgFeObjJsonAdd(RDM_OID_BRIDGING_FILTER, &newBrFilterIid, NULL)) != ZCFG_SUCCESS) {
				return ret;
			}
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_BRIDGING_FILTER, &newBrFilterIid, &newBrFilterObj)) != ZCFG_SUCCESS) {
				return ret;
			}
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
			dalcmdcirteriaToObject(newBrFilterObj, newbrPathName, type, (char *)macvalue, (char *)op60value, (char *)op61value, (char *)op125value, (char *)wildcardenable, (char *)apasSrcMac);
#else
			dalcmdcirteriaToObject(newBrFilterObj, newbrPathName, type, (char *)macvalue, (char *)op60value, (char *)op61value, (char *)op125value, (char *)wildcardenable);
#endif
			zcfgFeObjJsonSetWithoutApply(RDM_OID_BRIDGING_FILTER, &newBrFilterIid, newBrFilterObj, NULL);
			zcfgFeJsonObjFree(newBrFilterObj);
		}
		count ++;
		sprintf(cnt,"%d",count);
		obj = json_object_object_get(Jobj, cnt);
	}

	/*===============================================================================*/

	//printf("10. POST RDM_OID_DHCPV4_SRV_POOL\n");
	/* POST RDM_OID_DHCPV4_SRV_POOL */
	if((ret = zcfgFeObjJsonAdd(RDM_OID_DHCPV4_SRV_POOL, &dhcpV4SrvIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DHCPV4_SRV_POOL, &dhcpV4SrvIid, &dhcpV4SrvObj)) != ZCFG_SUCCESS) {
		return ret;
	}
	char DhcpAddrMin[30] = {0};
	char DhcpAddrMax[30] = {0};
	char IpIntf_str[30] = "IP.Interface.";

#if 1
	//refer to zcfg_be\libzcfg_be\libzcfg_be_lan\zcfg_be_lan.c calcIPRange()
	inet_pton(AF_INET, json_object_get_string(json_object_object_get(v4AddrObj, "IPAddress")), &ip);
	inet_pton(AF_INET, json_object_get_string(json_object_object_get(v4AddrObj, "SubnetMask")), &mask);
	startip.s_addr = (ip.s_addr & mask.s_addr) + htonl(1);
	if(startip.s_addr == ip.s_addr)
		startip.s_addr += htonl(1);
	endip.s_addr = (ip.s_addr | ~mask.s_addr) - htonl(1);
	if(endip.s_addr == ip.s_addr)
		endip.s_addr -= htonl(1);
	inet_ntop(AF_INET, &startip, DhcpAddrMin, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &endip, DhcpAddrMax, INET_ADDRSTRLEN);
#else
	sprintf(DhcpAddrMin,"192.168.%d.2",IP_idx);
	sprintf(DhcpAddrMax,"192.168.%d.254",IP_idx);
#endif
	sprintf(IpIntf_str,"IP.Interface.%d",newIpIid.idx[0]);

	json_object_object_add(dhcpV4SrvObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(dhcpV4SrvObj, "MinAddress", json_object_new_string(DhcpAddrMin));
	json_object_object_add(dhcpV4SrvObj, "MaxAddress", json_object_new_string(DhcpAddrMax));
	json_object_object_add(dhcpV4SrvObj, "X_ZYXEL_DNS_Type", json_object_new_string("DNS Proxy"));
	json_object_object_add(dhcpV4SrvObj, "Interface", json_object_new_string(IpIntf_str));

	zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL, &dhcpV4SrvIid, dhcpV4SrvObj, NULL);

	zcfgFeLanCheckWanDhcpServer(NULL);
	/*============================================================================================*/

	//printf("11. PUT RDM_OID_BRIDGING_FILTER\n");
	/* PUT RDM_OID_BRIDGING_FILTER according to lanIntf*/
	char *single_lanIntf = NULL;
	char *BrFilter_interface = NULL;
	char *lanIntf_tmp = NULL;
	char *rest_str = NULL;
	zcfgRet_t ret_debug = ZCFG_SUCCESS;

	if(lanintf != NULL) {
		lanIntf_tmp = (char*)malloc(strlen(lanintf)+1);
		memcpy(lanIntf_tmp, lanintf, strlen(lanintf)+1);
		single_lanIntf =  strtok_r(lanIntf_tmp, ",", &rest_str);
		while(single_lanIntf!=NULL) {
			IID_INIT(newBrFilterIid);
			while((ret_debug = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_FILTER, &newBrFilterIid, &newBrFilterObj)) == ZCFG_SUCCESS) {
				BrFilter_interface = (char*)json_object_get_string(json_object_object_get(newBrFilterObj, "Interface"));
				if((BrFilter_interface!=NULL) && (!strcmp(single_lanIntf, BrFilter_interface))) {
					json_object_object_add(newBrFilterObj, "Bridge", json_object_new_string(newbrPathName));
					zcfgFeObjJsonBlockedSet(RDM_OID_BRIDGING_FILTER, &newBrFilterIid, newBrFilterObj, NULL);
					zcfgFeJsonObjFree(newBrFilterObj);
					break;
				}
				zcfgFeJsonObjFree(newBrFilterObj);
			}
			single_lanIntf = strtok_r(rest_str, ",", &rest_str);
		}
		free(lanIntf_tmp);
	}

	//printf("11. PUT RDM_OID_ZY_IGMP and RDM_OID_ZY_MLD\n");
	char BrStr[30] = {0};
	BrIdx = newBrIid.idx[0]-1;
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%d", BrIdx);
	strcat(BrStr, ",br");
	strcat(BrStr, tmp);
	strcat(BrStr,"|2");
	char snpBr_str[60] = {0};
	char *snpBrName = NULL;
	// PUT RDM_OID_ZY_IGMP
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IGMP, &igmpIid, &igmpObj);
	if(ret == ZCFG_SUCCESS){
		snpBrName = (char*)json_object_get_string(json_object_object_get(igmpObj, "SnoopingBridgeIfName"));
		memcpy(snpBr_str, snpBrName, strlen(snpBrName)+1);
		strcat(snpBr_str, BrStr);
		json_object_object_add(igmpObj, "SnoopingBridgeIfName", json_object_new_string(snpBr_str));
		zcfgFeObjJsonSet(RDM_OID_ZY_IGMP, &igmpIid, igmpObj, NULL);
	}
	// PUT RDM_OID_ZY_MLD
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MLD, &mldIid, &mldObj);
	if(ret == ZCFG_SUCCESS){
		snpBrName = (char*)json_object_get_string(json_object_object_get(mldObj, "SnoopingBridgeIfName"));
		memcpy(snpBr_str, snpBrName, strlen(snpBrName)+1);
		strcat(snpBr_str, BrStr);
		json_object_object_add(mldObj, "SnoopingBridgeIfName", json_object_new_string(snpBr_str));
		zcfgFeObjJsonSet(RDM_OID_ZY_MLD, &mldIid, mldObj, NULL);
	}
	/*============================================================================================*/
	
	if(v4AddrObj) zcfgFeJsonObjFree(v4AddrObj);
	if(newBrObj) zcfgFeJsonObjFree(newBrObj);
	if(newEthLinkObj) zcfgFeJsonObjFree(newEthLinkObj);
	if(newIpObj) zcfgFeJsonObjFree(newIpObj);
	if(newBrFilterObj) zcfgFeJsonObjFree(newBrFilterObj);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	if(vlanGroupObj) zcfgFeJsonObjFree(vlanGroupObj);
#endif
	if(dhcpV4SrvObj) zcfgFeJsonObjFree(dhcpV4SrvObj);

	
	return ret;
}

zcfgRet_t dalcmdIntrGrpEdit(struct json_object *Jobj, char *replyMsg, char *method){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *BridgeBrObj = NULL;
	struct json_object *IpIfaceObj = NULL;
	struct json_object *BridgeFilterObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	struct json_object *VlanObj = NULL;
	objIndex_t VlanIid = {0};
	const char *vlanvalue = NULL, *brRefKey = NULL;
#endif
	struct json_object *EthLinkObj = NULL;
	struct json_object *obj = NULL;
	objIndex_t BridgeBrIid = {0};
	objIndex_t IpIfaceIid = {0};
	objIndex_t BridgeFilterIid = {0};
	objIndex_t EthLinkIid = {0};
	int index = 0, criteriaNum = 0, count = 0, idx = 0, reset = 0;
	//int ethidx = 0;
	const char *name = NULL, *wanintf = NULL, *lanintf = NULL, *type = NULL, *macvalue = NULL, *op60value = NULL, *op61value = NULL;
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
	const char *apasSrcMac = NULL;
#endif
	char brPathName[32] = {0}, lanIntf_tmp[512] = {0}, brPath[32] = {0}, ifname[8] = {0}, cnt[4] = {0}, buf[512] = {0}, bfKey[4] = {0}, preIfaceGroup[32] = {0}, preInterface[32] = {0};
	char *single_lanIntf = NULL, *rest_str = NULL, *op125value = NULL, *tmp = NULL;
	const char *act = NULL, *wildcardenable = NULL, *ptr = NULL, *cmd = NULL;
	bool lanIntfSet = false, wildcard = false;
	bool criteriaCnt = 0;

    printf("Entering dalcmdIntrGrpEdit!\n");
	if(json_object_object_get(Jobj, "critInfo"))
		GUICeireriaReconstruct(Jobj, replyMsg);
	if(json_object_object_get(Jobj, "type"))
		CLICeireriaReconstruct(Jobj, replyMsg, method);
	
    criteriaNum = json_object_get_int(json_object_object_get(Jobj, "criteriaNum"));
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	if(index == 1)
    {
		if(replyMsg != NULL)
			strcat(replyMsg, "Default rule cannot modify.");
		return ZCFG_INVALID_PARAM_VALUE;
	}
	name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	wanintf = json_object_get_string(json_object_object_get(Jobj, "wanIntf"));
	lanintf = json_object_get_string(json_object_object_get(Jobj, "lanIntf"));
	
	sprintf(ifname,"br%d",index-1);
	sprintf(brPath,"Bridging.Bridge.%d",index);
	sprintf(brPathName,"Bridging.Bridge.%d.Port.1",index);
	
    printf("dalcmdIntrGrpEdit: criteriaNum:%d\n", criteriaNum);
	if(name != NULL || wanintf != NULL) //update bridge name, bind wan interface
    {
		if(name != NULL)
        {		// name edit setting
			BridgeBrIid.level = 1;
			BridgeBrIid.idx[0] = index;
			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_BRIDGING_BR, &BridgeBrIid, &BridgeBrObj) == ZCFG_SUCCESS)
            {
				json_object_object_add(BridgeBrObj, "X_ZYXEL_BridgeName", json_object_new_string(name));
                zcfgFeObjJsonBlockedSet(RDM_OID_BRIDGING_BR, &BridgeBrIid, BridgeBrObj, NULL);
#ifdef ZYXEL_USA_PRODUCT
				//printf("JJJJ set group %s\n", name);
				objIndex_t brPortIid;
				struct json_object *brPortObj = NULL;
				IID_INIT(brPortIid);
				while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_BR_PORT, &brPortIid, &brPortObj) == ZCFG_SUCCESS){
					//printf("JJJJ set group port <%s>\n", json_object_get_string(json_object_object_get(brPortObj, "X_ZYXEL_IfName")));
					if(BridgeBrIid.idx[0] == brPortIid.idx[0] && json_object_get_boolean(json_object_object_get(brPortObj, "ManagementPort")))
					{
						bool isolateBridge = json_object_get_boolean(json_object_object_get(Jobj, "isolateBridge"));
						//printf("JJJJ set group port iso <%d>\n", isolateBridge);
						json_object_object_add(brPortObj, "X_ZYXEL_BridgeIsolated", json_object_new_boolean(isolateBridge));	
						zcfgFeObjJsonSet(RDM_OID_BRIDGING_BR_PORT, &brPortIid, brPortObj, NULL);
						zcfgFeJsonObjFree(brPortObj);
						break;
					}
					zcfgFeJsonObjFree(brPortObj);
				}
#endif
				zcfgFeJsonObjFree(BridgeBrObj);
			}
		}

		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ETH_LINK, &EthLinkIid, &EthLinkObj) == ZCFG_SUCCESS)
        {
			if(!strcmp(json_object_get_string(json_object_object_get(EthLinkObj, "LowerLayers")), brPathName))
            {
				//ethidx = EthLinkIid.idx[0];
				zcfgFeJsonObjFree(EthLinkObj);
				break;
			}
			zcfgFeJsonObjFree(EthLinkObj);
		}

		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &IpIfaceIid, &IpIfaceObj) == ZCFG_SUCCESS)
        {
			if(!strcmp(ifname,json_object_get_string(json_object_object_get(IpIfaceObj, "X_ZYXEL_IfName"))))
            {
			if(wanintf != NULL)
				json_object_object_add(IpIfaceObj, "X_ZYXEL_Group_WAN_IpIface", json_object_new_string(wanintf));
			if(name != NULL)
				json_object_object_add(IpIfaceObj, "X_ZYXEL_SrvName", json_object_new_string(name));
			zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &IpIfaceIid, IpIfaceObj, NULL);
			zcfgFeJsonObjFree(IpIfaceObj);
				break;
			}
			zcfgFeJsonObjFree(IpIfaceObj);
		}
	}
	
	zcfgFeLanCheckWanDhcpServer(NULL);

    if(criteriaNum > 0) {
		sprintf(cnt, "%d", count);
		obj = json_object_object_get(Jobj, cnt);
		while(obj != NULL) {
			bfKey[0] = '\0';
			act = NULL;
			idx = 0;
			type = json_object_get_string(json_object_object_get(obj, "type"));
			if(!strcmp(type, "vlangroup")) {
#ifndef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
				printf("%s : vlan group not support\n", __func__);
#else
				vlanvalue = NULL;
				printf("dalcmdIntrGrpEdit: with vlangroup!\n");
				if(json_object_object_get(obj, "act")){
					act = json_object_get_string(json_object_object_get(obj, "act"));
				}
				if(json_object_object_get(obj, "idx")){
					idx = json_object_get_int(json_object_object_get(obj, "idx"));
				}
				if(json_object_get_string(json_object_object_get(obj, "vlangroup"))){
					vlanvalue = json_object_get_string(json_object_object_get(obj, "vlangroup"));
				}

				if(!strcmp(act, "add")) {
					++criteriaCnt;
					IID_INIT(VlanIid);
					while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &VlanIid, &VlanObj) == ZCFG_SUCCESS) {
						if(!strcmp(vlanvalue, json_object_get_string(json_object_object_get(VlanObj, "GroupName")))) {
							sprintf(bfKey, "%d", index);
							json_object_object_add(VlanObj, "BrRefKey", json_object_new_string(bfKey));
							zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &VlanIid, VlanObj, NULL);
							zcfgFeJsonObjFree(VlanObj);
							break;
						}
						zcfgFeJsonObjFree(VlanObj);
					}

                    while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, &BridgeFilterObj) == ZCFG_SUCCESS) {
                        if(!strcmp(json_object_get_string(json_object_object_get(BridgeFilterObj, "Bridge")), brPath)) {
                            if(strcmp(json_object_get_string(json_object_object_get(BridgeFilterObj, "Interface")), "")) {
								json_object_object_add(BridgeFilterObj, "Bridge", json_object_new_string("Bridging.Bridge.1"));
								zcfgFeObjJsonBlockedSet(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, BridgeFilterObj, NULL);
								zcfgFeJsonObjFree(BridgeFilterObj);
							}
                            else {	//for CLI edit criteria vlan
								zcfgFeObjJsonDel(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, NULL);
							}							
						}
						zcfgFeJsonObjFree(BridgeFilterObj);
					}
				}
                else if(!strcmp(act, "edit")) {
					++criteriaCnt;
                    VlanIid.idx[0] = idx;
                    VlanIid.level = 1;
                    if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VLAN_GROUP, &VlanIid, &VlanObj) == ZCFG_SUCCESS) {
                        json_object_object_add(VlanObj, "BrRefKey", json_object_new_string(""));
                        zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &VlanIid, VlanObj, NULL);
                        zcfgFeJsonObjFree(VlanObj);
                    }
                    IID_INIT(VlanIid);
                    while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &VlanIid, &VlanObj) == ZCFG_SUCCESS)
                    {
                        if(!strcmp(vlanvalue,json_object_get_string(json_object_object_get(VlanObj, "GroupName"))))
                        {
                            sprintf(bfKey,"%d",index);
                            json_object_object_add(VlanObj, "BrRefKey", json_object_new_string(bfKey));
                            zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &VlanIid, VlanObj, NULL);
                            zcfgFeJsonObjFree(VlanObj);
                            break;
                        }
                        zcfgFeJsonObjFree(VlanObj);
                    }
                }
				else if(!strcmp(act,"delete")) {
					VlanIid.idx[0] = idx;
					VlanIid.level = 1;
					if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VLAN_GROUP, &VlanIid, &VlanObj) == ZCFG_SUCCESS) {
						json_object_object_add(VlanObj, "BrRefKey", json_object_new_string(""));
						zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &VlanIid, VlanObj, NULL);
						zcfgFeJsonObjFree(VlanObj);
					}
				}
                else if(!strcmp(act, "re_add")) {
                    /*
                     * If only change port bind to option bind, lanIntf will be null,
                     * also same as only set option bind.
                    */
					++criteriaCnt;
                    if((ret = zcfgFeDalIntrGrpDelete(Jobj, NULL)) == ZCFG_SUCCESS) {
                        if((ret = dalcmdIntrGrpAdd(Jobj, replyMsg)) == ZCFG_SUCCESS) {
                            printf("dalcmdIntrGrpEdit: SUCCESS Delete rule, and re-add one!\n");
                        }
                        else {
                            printf("dalcmdIntrGrpEdit:SUCCESS Delete rule, but FAILED re-add one!\n");
                        }
                    }
                    else {
                        printf("dalcmdIntrGrpEdit:FAILED Delete rule!\n");
                    }
                }
				else {	// for CLI change criteria, original criteria exist vlan group rule, new criteria do not have. Rex Chen
                    while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &VlanIid, &VlanObj) == ZCFG_SUCCESS) {
						brRefKey = json_object_get_string(json_object_object_get(VlanObj, "BrRefKey"));
                        if(strcmp(brRefKey, "")) {
                            if(index == atoi(brRefKey)) {
                                json_object_object_add(VlanObj, "BrRefKey", json_object_new_string(""));
                                zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &VlanIid, VlanObj, NULL);
                                zcfgFeJsonObjFree(VlanObj);
                                break;
                            }
                        }
                        zcfgFeJsonObjFree(VlanObj);
                    }
				}
#endif // CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
			}//if(!strcmp(type,"vlangroup"))
			else
            {
                printf("dalcmdIntrGrpEdit: without vlangroup!\n");
				if(json_object_object_get(obj, "act")){
					act = json_object_get_string(json_object_object_get(obj, "act"));
				}
				if(json_object_object_get(obj, "idx")){
					idx = json_object_get_int(json_object_object_get(obj, "idx"));
				}
				macvalue = json_object_get_string(json_object_object_get(obj, "mac_value"));
				op60value = json_object_get_string(json_object_object_get(obj, "op60_value"));
				op61value = json_object_get_string(json_object_object_get(obj, "op61_value"));
				op125value = (char *)json_object_get_string(json_object_object_get(obj, "op125value"));
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
				apasSrcMac = json_object_get_string(json_object_object_get(obj, "apasSrcMac"));
#endif
                cmd = json_object_get_string(json_object_object_get(Jobj, "cmdtype"));
				if(!strcmp(cmd, "GUI")){
					wildcardenable = json_object_get_string(json_object_object_get(obj, "wildcardEnable"));
				}
				else if(!strcmp(cmd ,"CLI")) {
					wildcard = json_object_get_boolean(json_object_object_get(obj, "wildcardEnable"));
					if(wildcard)
						wildcardenable = "Y";
					else
						wildcardenable = "N";
				}
				
				IID_INIT(BridgeFilterIid);
				if(!strcmp(act, "add")) {
                    IID_INIT(BridgeFilterIid);
                    if((ret = zcfgFeObjJsonAdd(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, NULL)) != ZCFG_SUCCESS) {
                        return ret;
                    }

                    if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, &BridgeFilterObj)) != ZCFG_SUCCESS) {
                        return ret;
                    }

					if(cmd != NULL) {
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
						IID_INIT(VlanIid);
						while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &VlanIid, &VlanObj) == ZCFG_SUCCESS){
							brRefKey = json_object_get_string(json_object_object_get(VlanObj, "BrRefKey"));
							if(strcmp(brRefKey, "")){
								if(index == atoi(brRefKey)){
									json_object_object_add(VlanObj, "BrRefKey", json_object_new_string(""));
									zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &VlanIid, VlanObj, NULL);
									zcfgFeJsonObjFree(VlanObj);
									break;
								}
							}
							zcfgFeJsonObjFree(VlanObj);
						}
#else
						printf("%s : vlan group not support\n", __func__);
#endif
					}
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
					dalcmdcirteriaToObject(BridgeFilterObj, brPath, (char *)type, (char *)macvalue, (char *)op60value, (char *)op61value, (char *)op125value, (char *)wildcardenable, (char *)apasSrcMac);
#else
					dalcmdcirteriaToObject(BridgeFilterObj, brPath, (char *)type, (char *)macvalue, (char *)op60value, (char *)op61value, (char *)op125value, (char *)wildcardenable);
#endif
                    zcfgFeObjJsonBlockedSet(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, BridgeFilterObj, NULL);
					zcfgFeJsonObjFree(BridgeFilterObj);
					reset++;
				}	
				else if(!strcmp(act,"edit"))
                {
					BridgeFilterIid.level = 1;
					BridgeFilterIid.idx[0] = idx;
					if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, &BridgeFilterObj)) != ZCFG_SUCCESS)
                    {
						return ret;
					}
                    printf("dalcmdIntrGrpEdit: brPath:%s\n", brPath);
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
					dalcmdcirteriaToObject(BridgeFilterObj, brPath, (char *)type, (char *)macvalue, (char *)op60value, (char *)op61value, (char *)op125value, (char *)wildcardenable, (char *)apasSrcMac);
#else
					dalcmdcirteriaToObject(BridgeFilterObj, brPath, (char *)type, (char *)macvalue, (char *)op60value, (char *)op61value, (char *)op125value, (char *)wildcardenable);
#endif
                    zcfgFeObjJsonBlockedSet(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, BridgeFilterObj, NULL);
                    zcfgFeJsonObjFree(BridgeFilterObj);
					reset++;

				}
				else if(!strcmp(act,"delete"))
                {
					BridgeFilterIid.level = 1;
					BridgeFilterIid.idx[0] = idx;
					ret = zcfgFeObjJsonDel(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, NULL);
				}
				else if(!strcmp(act, "re_add"))
                {
                    /*
                     * If only change port bind to option bind, lanIntf will be null,
                     * also same as only set option bind.
                    */
                    if((ret = zcfgFeDalIntrGrpDelete(Jobj, NULL)) == ZCFG_SUCCESS)
                    {
                        if((ret = dalcmdIntrGrpAdd(Jobj, replyMsg)) == ZCFG_SUCCESS)
                        {
                            printf("dalcmdIntrGrpEdit: SUCCESS Delete rule, and re-add one!\n");
                        }
                        else
                        {
                            printf("dalcmdIntrGrpEdit:SUCCESS Delete rule, but FAILED re-add one!\n");
                        }
                    }
                    else
                    {
                        printf("dalcmdIntrGrpEdit:FAILED Delete rule!\n");
                    }
                }
                else
                {
                    while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, &BridgeFilterObj) == ZCFG_SUCCESS)
                    {
                        if(!strcmp(brPath,json_object_get_string(json_object_object_get(BridgeFilterObj, "Bridge"))) && !strcmp("",json_object_get_string(json_object_object_get(BridgeFilterObj, "Interface"))))
                        {
							zcfgFeObjJsonDel(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, NULL);
						}
						zcfgFeJsonObjFree(BridgeFilterObj);
					}
				}
			}
			count++;
			sprintf(cnt,"%d",count);
			obj = json_object_object_get(Jobj, cnt);
		}//while(obj != NULL)
	}

    //if only port bind setting, no criteria option.
	if(json_object_object_get(Jobj, "lanIntf") != NULL && criteriaCnt == 0)
    {
		strcpy(lanIntf_tmp, lanintf);
		single_lanIntf = strtok_r(lanIntf_tmp, ",", &rest_str);
		IID_INIT(BridgeFilterIid);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
		IID_INIT(VlanIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &VlanIid, &VlanObj) == ZCFG_SUCCESS) {
			if(atoi(json_object_get_string(json_object_object_get(VlanObj, "BrRefKey"))) == index) {
				json_object_object_add(VlanObj, "BrRefKey", json_object_new_string(""));
				zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &VlanIid, VlanObj, NULL);
				zcfgFeJsonObjFree(VlanObj);
				break;
			}
			zcfgFeJsonObjFree(VlanObj);
		}
#endif
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, &BridgeFilterObj) == ZCFG_SUCCESS)
		{
			//Check LAN port if not binding, let it to back bind br0.
			if(!strcmp(json_object_get_string(json_object_object_get(BridgeFilterObj, "Bridge")),brPath))
			{
				strcpy(buf, lanintf);
				ptr = strtok_r(buf, ",", &tmp);
				while(ptr != NULL)
				{
					if(!strncmp(ptr,brPath,17))
					{
						if(!strcmp(ptr,json_object_get_string(json_object_object_get(BridgeFilterObj, "Interface"))))
						{
							//filter already bind lan port.
							printf("dalcmdIntrGrpEdit:idx:%d, already have interface: %s, lanIntfSet:%s, bridge:%s \n",BridgeFilterIid.idx[0], ptr, lanIntfSet ? "true": "false", json_object_get_string(json_object_object_get(BridgeFilterObj, "Bridge")));
							printf("dalcmdIntrGrpEdit:idx:%d, X_ZYXEL_PreIfaceGroup:%s\n",BridgeFilterIid.idx[0],json_object_get_string(json_object_object_get(BridgeFilterObj, "X_ZYXEL_PreIfaceGroup")));
							lanIntfSet = true;
							printf("dalcmdIntrGrpEdit: idx %d only port bind without crit!\n",BridgeFilterIid.idx[0]);
							break;
						}
					}
					ptr = strtok_r(NULL, ",", &tmp);
				}
				buf[0] = '\0';
				/* 
				* if lanIntf already set, then do nothing, continue parse next idx.
				* if lanIntf not set, and interface was empty, delete it, if not empty, set to bind br0.
				*/
				if(!lanIntfSet)
				{
					printf("dalcmdIntrGrpEdit: idx:%d, lanIntf not match data model, ptr:%s, X_ZYXEL_Enable_Criteria:%s\n Bridge:%s\n",
					BridgeFilterIid.idx[0], ptr, json_object_get_string(json_object_object_get(BridgeFilterObj, "X_ZYXEL_Enable_Criteria")),json_object_get_string(json_object_object_get(BridgeFilterObj, "Bridge")));
					//if already bind, and no change port, it should pass it.
					json_object_object_add(BridgeFilterObj, "Bridge", json_object_new_string("Bridging.Bridge.1"));
					if(!strcmp(json_object_get_string(json_object_object_get(BridgeFilterObj, "Interface")),""))
					{
						zcfgFeObjJsonDel(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, NULL);
					}
					else
					{
						printf("dalcmdIntrGrpEdit: idx:%d, set bridge filter, ptr:%s, X_ZYXEL_Enable_Criteria:%s\n Bridge:%s\n",
						BridgeFilterIid.idx[0], ptr, json_object_get_string(json_object_object_get(BridgeFilterObj, "X_ZYXEL_Enable_Criteria")),json_object_get_string(json_object_object_get(BridgeFilterObj, "Bridge")));
						zcfgFeObjJsonBlockedSet(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, BridgeFilterObj, NULL);
					}
				}
				lanIntfSet = false;
				zcfgFeJsonObjFree(BridgeFilterObj);
			}
		}
		while(single_lanIntf != NULL)
		{
			IID_INIT(BridgeFilterIid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, &BridgeFilterObj) == ZCFG_SUCCESS)
			{
				ZOS_STRNCPY(preIfaceGroup, json_object_get_string(json_object_object_get(BridgeFilterObj, "X_ZYXEL_PreIfaceGroup")), sizeof(preIfaceGroup));
				if(!strcmp(json_object_get_string(json_object_object_get(BridgeFilterObj, "Interface")),single_lanIntf) && !strcmp(preIfaceGroup,""))
				{
					printf("dalcmdIntrGrpEdit: idx:%d, signel lanIntf, X_ZYXEL_Enable_Criteria:%s\n Bridge:%s, brPath:%s\n",
					BridgeFilterIid.idx[0], json_object_get_string(json_object_object_get(BridgeFilterObj, "X_ZYXEL_Enable_Criteria")),json_object_get_string(json_object_object_get(BridgeFilterObj, "Bridge")), brPath);
					json_object_object_add(BridgeFilterObj, "Bridge", json_object_new_string(brPath));
					zcfgFeObjJsonBlockedSet(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, BridgeFilterObj, NULL);
					zcfgFeJsonObjFree(BridgeFilterObj);
					break;
				}
				else if(!strcmp(json_object_get_string(json_object_object_get(BridgeFilterObj, "Interface")),single_lanIntf) && strcmp(preIfaceGroup,"") != 0)
				{
					printf("dalcmdIntrGrpEdit: idx %d signel lanIntf, and have been bind by crit!\n",BridgeFilterIid.idx[0]);
					sprintf(preInterface,"Bridging.Bridge.1.Port.%d",(BridgeFilterIid.idx[0]+1));
					json_object_object_add(BridgeFilterObj, "Bridge", json_object_new_string("Bridging.Bridge.1"));
					json_object_object_add(BridgeFilterObj, "X_ZYXEL_PreIfaceGroup", json_object_new_string(""));
					zcfgFeObjJsonBlockedSet(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, BridgeFilterObj, NULL);
                        
					printf("dalcmdIntrGrpEdit: idx %d signel lanIntf, modify to port bind only!\n",BridgeFilterIid.idx[0]);
					json_object_object_add(BridgeFilterObj, "Bridge", json_object_new_string(brPath));
					json_object_object_add(BridgeFilterObj, "Interface", json_object_new_string(preInterface));
					zcfgFeObjJsonBlockedSet(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, BridgeFilterObj, NULL);
					zcfgFeJsonObjFree(BridgeFilterObj);
					break;
				}
				zcfgFeJsonObjFree(BridgeFilterObj);
			}
			single_lanIntf = strtok_r(rest_str, ",", &rest_str);
		}
	}

	return ret;
}

#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
void dalcmdcirteriaToObject(json_object *BrFilterObj, char *brPathName, char *type, char *macvalue, char *op60value, char *op61value, char *op125tmp, char *wildcardEnable, char *apasSrcMac)
#else
void dalcmdcirteriaToObject(json_object *BrFilterObj, char *brPathName, char *type, char *macvalue, char *op60value, char *op61value, char *op125tmp, char *wildcardEnable)
#endif
{
	json_object_object_add(BrFilterObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(BrFilterObj, "Bridge", json_object_new_string(brPathName));
	json_object_object_add(BrFilterObj, "X_ZYXEL_Enable_Criteria", json_object_new_boolean(true));
	json_object_object_add(BrFilterObj, "SourceMACAddressFilterList", json_object_new_string(""));
	json_object_object_add(BrFilterObj, "SourceMACAddressFilterExclude", json_object_new_boolean(false));
	json_object_object_add(BrFilterObj, "SourceMACFromVendorClassIDFilter", json_object_new_string(""));
	json_object_object_add(BrFilterObj, "SourceMACFromVendorClassIDFilterExclude", json_object_new_boolean(false));
	json_object_object_add(BrFilterObj, "SourceMACFromVendorClassIDMode", json_object_new_string("Exact"));
	json_object_object_add(BrFilterObj, "SourceMACFromClientIDFilter", json_object_new_string(""));
	json_object_object_add(BrFilterObj, "SourceMACFromClientIDFilterExclude", json_object_new_boolean(false));
	json_object_object_add(BrFilterObj, "X_ZYXEL_SourceMACFromVSIFilter", json_object_new_string(""));	
	if(!strcmp(type, "mac")) {
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
		if(apasSrcMac) json_object_object_add(BrFilterObj, "X_ZYXEL_APASSourceMACAFilter", json_object_new_string(apasSrcMac));
#endif
		if(macvalue)
		{
			json_object_object_add(BrFilterObj, "SourceMACAddressFilterList", json_object_new_string(macvalue));
			json_object_object_add(BrFilterObj, "SourceMACAddressFilterExclude", json_object_new_boolean(false));
		}
	}
	else if(!strcmp(type, "op60")) {
		json_object_object_add(BrFilterObj, "SourceMACFromVendorClassIDFilter", json_object_new_string(op60value));
		json_object_object_add(BrFilterObj, "SourceMACFromVendorClassIDFilterExclude", json_object_new_boolean(false));
		if(!strcmp(wildcardEnable, "Y"))
			json_object_object_add(BrFilterObj, "SourceMACFromVendorClassIDMode", json_object_new_string("Substring"));
		else 
			json_object_object_add(BrFilterObj, "SourceMACFromVendorClassIDMode", json_object_new_string("Exact"));
		}
	else if(!strcmp(type, "op61")) {
		json_object_object_add(BrFilterObj, "SourceMACFromClientIDFilter", json_object_new_string(op61value));
		json_object_object_add(BrFilterObj, "SourceMACFromClientIDFilterExclude", json_object_new_boolean(false));
	}
	else if(!strcmp(type, "op125")) 
		json_object_object_add(BrFilterObj, "X_ZYXEL_SourceMACFromVSIFilter", json_object_new_string(op125tmp));

	return;
}

zcfgRet_t op125ParameterTrahsfer(struct json_object *obj);
// This function is only for CLI to transfer json object struct.
zcfgRet_t CLICeireriaReconstruct(struct json_object *Jobj, char *replyMsg, char *method){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *criteria = NULL;
	struct json_object *currArray = NULL;
	struct json_object *currObj = NULL;
	const char *tmp = NULL;
	const char *origcriteria = NULL;
	char type[16] = {0}, act[6] = {0};
	int idx = 0, index = 0;
	criteria = json_object_new_object();
	currArray = json_object_new_array();
	zcfgFeDalIntrGrpGet(NULL, currArray, NULL);
	if(json_object_object_get(Jobj, "Index"))	//	add case no Index
		idx = json_object_get_int(json_object_object_get(Jobj, "Index"));
	else
		idx = 1;
	currObj = json_object_array_get_idx(currArray, idx-1);
	origcriteria = json_object_get_string(json_object_object_get(currObj, "Criteria"));
	index = json_object_get_int(json_object_object_get(currObj, "idx"));
	tmp = json_object_get_string(json_object_object_get(Jobj, "type"));

	if(!strcmp(tmp, "lanIntf"))
	{
		json_object_object_add(Jobj, "criteriaNum", json_object_new_int(0));
		return ret;
	}
	else if(origcriteria)
	{
		if(strstr(origcriteria,tmp))
			strcpy(act,"edit");
		else
			strcpy(act,"add");
	}
	else 
		strcpy(act,"add");
	
	strcpy(type, tmp);
	json_object_object_add(criteria, "type", JSON_OBJ_COPY(json_object_object_get(Jobj, "type")));
	json_object_object_del(Jobj, "type");
	if(!strcmp(type, "mac")){
		json_object_object_add(criteria, "type", json_object_new_string("mac"));
		json_object_object_add(criteria, "mac_value", JSON_OBJ_COPY(json_object_object_get(Jobj, "mac_value")));
		json_object_object_add(criteria, "wildcardEnable", json_object_new_string("N"));
		json_object_object_add(criteria, "act", json_object_new_string(act));
		json_object_object_add(criteria, "idx", json_object_new_int(index));
		json_object_object_add(criteria, "cmd", json_object_new_string("CLI"));
		json_object_object_del(Jobj, "mac_value");
		json_object_object_add(Jobj, "0", JSON_OBJ_COPY(criteria));
	}
	else if(!strcmp(type, "op125")) {
		json_object_object_add(criteria, "type", json_object_new_string("op125"));
		if(json_object_object_get(Jobj, "Enterprise_Number")){
			json_object_object_add(criteria, "Enterprise_Number", JSON_OBJ_COPY(json_object_object_get(Jobj, "Enterprise_Number")));
			json_object_object_add(criteria, "act", json_object_new_string(act));
			json_object_object_del(Jobj, "Enterprise_Number");
		}
		if(json_object_object_get(Jobj, "OUI")){
			json_object_object_add(criteria, "OUI", JSON_OBJ_COPY(json_object_object_get(Jobj, "OUI")));
			json_object_object_del(Jobj, "OUI");
		}
		if(json_object_object_get(Jobj, "SN")){
			json_object_object_add(criteria, "SN", JSON_OBJ_COPY(json_object_object_get(Jobj, "SN")));
			json_object_object_del(Jobj, "SN");
		}
		if(json_object_object_get(Jobj, "Class")){
			json_object_object_add(criteria, "Class", JSON_OBJ_COPY(json_object_object_get(Jobj, "Class")));
			json_object_object_del(Jobj, "Class");
		}
		json_object_object_add(criteria, "wildcardEnable", json_object_new_string("Y"));
		json_object_object_add(criteria, "act", json_object_new_string(act));
		json_object_object_add(criteria, "idx", json_object_new_int(index));
		op125ParameterTrahsfer(criteria);
		json_object_object_add(criteria, "cmd", json_object_new_string("CLI"));
		json_object_object_add(Jobj, "0", JSON_OBJ_COPY(criteria));
	}
	else if(!strcmp(type, "op60")){
		json_object_object_add(criteria, "type", json_object_new_string("op60"));
		json_object_object_add(criteria, "op60_value", JSON_OBJ_COPY(json_object_object_get(Jobj, "op60_value")));
		json_object_object_add(criteria, "wildcardEnable", JSON_OBJ_COPY(json_object_object_get(Jobj, "wildcardEnable")));
		json_object_object_add(criteria, "act", json_object_new_string(act));
		json_object_object_add(criteria, "cmd", json_object_new_string("CLI"));
		json_object_object_add(criteria, "idx", json_object_new_int(index));
		json_object_object_del(Jobj, "op60_value");
		json_object_object_del(Jobj, "wildcardEnable");
		json_object_object_add(Jobj, "0", JSON_OBJ_COPY(criteria));
	}
	else if(!strcmp(type, "op61")){
		json_object_object_add(criteria, "type", json_object_new_string("op61"));
		json_object_object_add(criteria, "op61_value", JSON_OBJ_COPY(json_object_object_get(Jobj, "op61_value")));
		json_object_object_add(criteria, "wildcardEnable", json_object_new_string("N"));
		json_object_object_add(criteria, "act", json_object_new_string(act));
		json_object_object_add(criteria, "cmd", json_object_new_string("CLI"));
		json_object_object_add(criteria, "idx", json_object_new_int(index));
		json_object_object_del(Jobj, "op61_value");
		json_object_object_add(Jobj, "0", JSON_OBJ_COPY(criteria));
	}
	else if(!strcmp(type, "vlangroup")){
		json_object_object_add(criteria, "type", json_object_new_string("vlangroup"));
		json_object_object_add(criteria, "vlangroup", JSON_OBJ_COPY(json_object_object_get(Jobj, "vlangroup")));
		json_object_object_add(criteria, "wildcardEnable", json_object_new_string("N/A"));
		json_object_object_add(criteria, "act", json_object_new_string(act));
		json_object_object_add(criteria, "cmd", json_object_new_string("CLI"));
		json_object_object_del(Jobj, "vlangroup");
		json_object_object_add(Jobj, "0", JSON_OBJ_COPY(criteria));
	}
	
	json_object_object_add(Jobj, "criteriaNum", json_object_new_int(1));
	return ret;
}

zcfgRet_t op125ParameterTrahsfer(struct json_object *obj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char en[64] = {0};
	char oui[64] = {0};
	char sn[64] = {0};
	char class[64] = {0};
	char op125value[256] = {0};
	const char *EN = NULL;
	const char *OUI = NULL;
	const char *SN = NULL;
	const char *Class = NULL;
	if(json_object_object_get(obj, "Enterprise_Number")){
		EN = json_object_get_string(json_object_object_get(obj, "Enterprise_Number"));
		strcat(op125value, "enterprise=");
		strcat(op125value,EN);
		strcat(op125value,en);
	}
	if(json_object_object_get(obj, "OUI")){
		OUI = json_object_get_string(json_object_object_get(obj, "OUI"));
		sprintf(oui,"\%\%\%\%1=%s",OUI);
		strcat(op125value, oui);
	}
	if(json_object_object_get(obj, "SN")){
		SN = json_object_get_string(json_object_object_get(obj, "SN"));
		sprintf(sn,"\%\%\%\%2=%s",SN);
		strcat(op125value, sn);
	}
	if(json_object_object_get(obj, "Class")){
		Class = json_object_get_string(json_object_object_get(obj, "Class"));
		sprintf(class,"\%\%\%\%3=%s",Class);
		strcat(op125value, class);
	}
	json_object_object_add(obj, "op125value", json_object_new_string(op125value));
	return ret;
}

//This function is only for GUI to transfer json object struct.
zcfgRet_t GUICeireriaReconstruct(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL, *mac = NULL, *opt60 = NULL, *opt61 = NULL, *opt125 = NULL, *vlan = NULL;
	const char *tmp = NULL;
	int count = 0, len = 0;
	char buf[256], cnt[4] = {0}, pt[256], OUI[33], SN[33], Class[33], Enterprise_Number[9], newmac[256] = {0};
	char *ptr = NULL, *type = NULL, *mac_value = NULL ,*op125type = NULL, *ptrtmp = NULL, *idx = NULL, *ptr1 = NULL, *act = NULL;
	char *op60_value = NULL, *op61_value = NULL, *vlangroup = NULL, *wildcard = NULL, *op125typeStr = NULL;

	buf[0]='\0';
	pt[0]='\0';
	OUI[0]='\0';
	SN[0]='\0';
	Class[0]='\0';
	Enterprise_Number[0]='\0';
	
	mac = json_object_new_object();
	opt60 = json_object_new_object();
	opt61 = json_object_new_object();
	opt125 = json_object_new_object();
	vlan = json_object_new_object();
	obj = json_object_object_get(Jobj, "critInfo");
	sprintf(cnt,"%d",count);
	tmp = json_object_get_string(json_object_object_get(obj,cnt));

	while(tmp != NULL){
		ZOS_STRNCPY(buf, tmp, sizeof(buf));
		type = strtok_r(buf, ":", &ptr);
		ptr = ptr+1;
		memset(newmac, 0, sizeof(newmac));
		if(!strcmp(type, "MAC address")){
			mac_value = strtok_r(NULL, ",", &ptr);
			while(mac_value != NULL){
				if(!strncmp(mac_value, "wildcardEnable", 14))
					break;
				strcat(newmac, mac_value);
				strcat(newmac, ",");
				mac_value = strtok_r(NULL, ",", &ptr);
			}
			len = strlen(newmac);
			if(len > 0){
				if(newmac[len-1] == ',')
					newmac[len-1] = '\0';
			}
			ptrtmp = strtok_r(NULL, ",", &ptr);
			if(ptrtmp!=NULL){
				if(!strncmp(ptrtmp, "idx", 3)){
					ptr1 = strtok_r(NULL, ":", &ptrtmp);
					act = strtok_r(NULL, ":", &ptr);
					if( ( ptr1 == NULL ) && ( act == NULL) ) printf("%s_%d: cannot found delimiters! the data format may not correct?\n", __FUNCTION__, __LINE__);
					json_object_object_add(mac, "idx", json_object_new_int(atoi(ptrtmp)));
					json_object_object_add(mac, "act", json_object_new_string(ptr));	
				}
				else if(!strncmp(ptrtmp, "act", 3)){
					act = strtok_r(NULL, ":", &ptrtmp);
					if( act == NULL) printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
					json_object_object_add(mac, "act", json_object_new_string(ptrtmp));
				}
			}
			json_object_object_add(mac, "type", json_object_new_string("mac"));
			json_object_object_add(mac, "mac_value", json_object_new_string(newmac));
			json_object_object_add(mac, "wildcardEnable", json_object_new_string("N"));
			json_object_object_add(Jobj, cnt, JSON_OBJ_COPY(mac));
		}
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE_INTFGRP
		if(!strcmp(type, "APAS Mac Filter")){
			mac_value = strtok_r(NULL, ",", &ptr);
			while(mac_value != NULL){
				if(!strncmp(mac_value, "wildcardEnable", 14))
					break;
				strcat(newmac, mac_value);
				strcat(newmac, ",");
				mac_value = strtok_r(NULL, ",", &ptr);
			}
			len = strlen(newmac);
			if(len > 0){
				if(newmac[len-1] == ',')
					newmac[len-1] = '\0';
			}
			ptrtmp = strtok_r(NULL, ",", &ptr);
			if(ptrtmp!=NULL){
				if(!strncmp(ptrtmp, "idx", 3)){
					ptr1 = strtok_r(NULL, ":", &ptrtmp);
					act = strtok_r(NULL, ":", &ptr);
					json_object_object_add(mac, "idx", json_object_new_int(atoi(ptrtmp)));
					json_object_object_add(mac, "act", json_object_new_string(ptr));
				}
				else if(!strncmp(ptrtmp, "act", 3)){
					act = strtok_r(NULL, ":", &ptrtmp);
					json_object_object_add(mac, "act", json_object_new_string(ptrtmp));
				}
			}
			json_object_object_add(mac, "type", json_object_new_string("mac"));
			json_object_object_add(mac, "apasSrcMac", json_object_new_string(newmac));
			json_object_object_add(mac, "wildcardEnable", json_object_new_string("N"));
			json_object_object_add(Jobj, cnt, JSON_OBJ_COPY(mac));
		}
#endif
		else if(!strcmp(type, "Option 60")){
			op60_value = strtok_r(NULL, ",", &ptr);
			wildcard = strtok_r(NULL, ":", &ptr);
			ptrtmp = strtok_r(NULL, ",", &ptr);
			if( (op60_value == NULL) || (wildcard == NULL) || (ptrtmp == NULL) )
				printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
			if(ptr!=NULL){
				if(!strncmp(ptr, "idx", 3)){
					ptr1 = strtok_r(NULL, ":", &ptr);
					idx = strtok_r(NULL, ",", &ptr);
					act = strtok_r(NULL, ":", &ptr);
					if( (ptr1 == NULL) || (idx == NULL) || (act == NULL) )
						printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
					json_object_object_add(opt60, "idx", json_object_new_int(atoi(idx)));
					json_object_object_add(opt60, "act", json_object_new_string(ptr));
				}
				else if(!strncmp(ptr, "act", 3)){
					act = strtok_r(NULL, ":", &ptr);
					if(act == NULL)
						printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
					json_object_object_add(opt60, "act", json_object_new_string(ptr));
				}
			}
			json_object_object_add(opt60, "type", json_object_new_string("op60"));
			json_object_object_add(opt60, "op60_value", json_object_new_string(op60_value));
			json_object_object_add(opt60, "wildcardEnable", json_object_new_string(ptrtmp));
			
			json_object_object_add(Jobj, cnt, JSON_OBJ_COPY(opt60));
		}
		else if(!strcmp(type, "Opion 61")){
			op61_value = strtok_r(NULL, ",", &ptr);
			ptrtmp = strtok_r(NULL, ",", &ptr);
			if(ptrtmp == NULL)
				printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
			if(ptr!=NULL){
				if(!strncmp(ptr, "idx", 3)){
					ptr1 = strtok_r(NULL, ":", &ptr);
					idx = strtok_r(NULL, ",", &ptr);
					act = strtok_r(NULL, ":", &ptr);
					if( (ptr1 == NULL) || (idx == NULL) || (act == NULL) )
						printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
					json_object_object_add(opt61, "idx", json_object_new_int(atoi(idx)));
					json_object_object_add(opt61, "act", json_object_new_string(ptr));
				}
				else if(!strncmp(ptr, "act", 3)){
					act = strtok_r(NULL, ":", &ptr);
					if(act == NULL)
						printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
					json_object_object_add(opt61, "act", json_object_new_string(ptr));
				}
			}
			json_object_object_add(opt61, "type", json_object_new_string("op61"));
			json_object_object_add(opt61, "op61_value", json_object_new_string(op61_value));
			json_object_object_add(opt61, "wildcardEnable", json_object_new_string("N"));
			json_object_object_add(Jobj, cnt, JSON_OBJ_COPY(opt61));
		}
		else if(!strcmp(type, "Option 125")){
			json_object_object_add(opt125, "type", json_object_new_string("op125"));
			op125typeStr = strtok_r(NULL, ",", &ptr);
			while(op125typeStr != NULL){
				ZOS_STRNCPY(pt, op125typeStr, sizeof(pt));
				op125type = strtok_r(pt, "=", &ptrtmp);
				if( op125type != NULL ) {
					if(!strcmp(op125type, "Enterprice-Number"))
						ZOS_STRNCPY(Enterprise_Number, ptrtmp, sizeof(Enterprise_Number));
					else if( NULL !=strstr(op125type, "OUI") )
						ZOS_STRNCPY(OUI, ptrtmp, sizeof(OUI));
					else if( NULL !=strstr(op125type, "SN") )
						ZOS_STRNCPY(SN, ptrtmp, sizeof(SN));
					else if( NULL !=strstr(op125type, "Class") )
						ZOS_STRNCPY(Class, ptrtmp, sizeof(Class));
					else
						printf("%s_%d: know parameters for Option 125(%s)\n", __FUNCTION__, __LINE__, op125type);
				}
				else
					printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
				if(ptr!=NULL){
					if(!strncmp(ptr, "idx", 3)){
						ptr1 = strtok_r(NULL, ":", &ptr);
						idx = strtok_r(NULL, ",", &ptr);
						act = strtok_r(NULL, ":", &ptr);
						if( (ptr1 == NULL) || (idx == NULL) || (act == NULL) )
							printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
						json_object_object_add(opt125, "idx", json_object_new_int(atoi(idx)));
						json_object_object_add(opt125, "act", json_object_new_string(ptr));
					}
					else if(!strncmp(ptr, "act", 3)){
						act = strtok_r(NULL, ":", &ptr);
						if(act == NULL)
							printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
						json_object_object_add(opt125, "act", json_object_new_string(ptr));
					}
				}
				op125typeStr = strtok_r(NULL, ",", &ptr);
			}
			if(strcmp(Enterprise_Number,"")){
				json_object_object_add(opt125, "Enterprise_Number", json_object_new_string(Enterprise_Number));
			}
			if(strcmp(OUI,"")){
				json_object_object_add(opt125, "OUI", json_object_new_string(OUI));
			}
			if(strcmp(SN,"")){
				json_object_object_add(opt125, "SN", json_object_new_string(SN));
			}
			if(strcmp(Class,"")){
				json_object_object_add(opt125, "Class", json_object_new_string(Class));
			}
			json_object_object_add(opt125, "wildcardEnable", json_object_new_string("Y"));
			op125ParameterTrahsfer(opt125);
			json_object_object_add(Jobj, cnt, JSON_OBJ_COPY(opt125));

		}
		else if(!strcmp(type, "VlanGroup")){
			vlangroup = strtok_r(NULL, ",", &ptr);
			ptrtmp = strtok_r(NULL, ",", &ptr);
			if( (vlangroup == NULL) || (ptrtmp == NULL) )
				printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
			if(ptr!=NULL){
				if(!strncmp(ptr, "idx", 3)){
					ptr1 = strtok_r(NULL, ":", &ptr);
					if(ptr1 == NULL)
						printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
					if( NULL !=strstr(tmp,"act")){
						idx = strtok_r(NULL, ",", &ptr);
						act = strtok_r(NULL, ":", &ptr);
						if( (idx == NULL) || (act == NULL) )
							printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
						json_object_object_add(vlan, "idx", json_object_new_int(atoi(idx)));
						json_object_object_add(vlan, "act", json_object_new_string(ptr));
					}else{
						json_object_object_add(vlan, "idx", json_object_new_int(atoi(ptr)));
					}
				}
				else if(!strncmp(ptr, "act", 3)){
					act = strtok_r(NULL, ":", &ptr);
					if(act == NULL)
						printf("%s_%d: cannot found delimiters! the data format may not correct!\n", __FUNCTION__, __LINE__);
					json_object_object_add(vlan, "act", json_object_new_string(ptr));
				}
			}
			json_object_object_add(vlan, "type", json_object_new_string("vlangroup"));
			json_object_object_add(vlan, "vlangroup", json_object_new_string(vlangroup));
			json_object_object_add(vlan, "wildcardEnable", json_object_new_string("N/A"));
			json_object_object_add(Jobj, cnt, JSON_OBJ_COPY(vlan));
		}
		count++;
		sprintf(cnt,"%d",count);
		tmp = json_object_get_string(json_object_object_get(obj,cnt));
	}
	json_object_object_add(Jobj, "criteriaNum", json_object_new_int(count));
	json_object_object_del(Jobj, "critInfo");
	return ret;
}

void zcfgFeDalShowIntrGrp(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;
	// const char *ipver = NULL;


	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-6s %-12s %-35s %-55s %-35s \n",
		    "Index","Name", "WAN Interface", "LAN Interfaces", "Criteria");
	
	
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		const char *criteria = NULL;
		const char *LanInterface = NULL;
		const char *wanInterface = NULL;
		criteria = json_object_get_string(json_object_object_get(obj, "Criteria"));
		LanInterface = json_object_get_string(json_object_object_get(obj, "lanIntf"));
		wanInterface = json_object_get_string(json_object_object_get(obj, "wanIntf"));
		if(!strcmp(LanInterface,""))
			LanInterface = "N/A";

		if(!strcmp(criteria,""))
			criteria = "N/A";

		if(!strcmp(wanInterface,""))
			wanInterface = "Any WAN";
		
		// ipver = json_object_get_string(json_object_object_get(obj, "ipver"));
			printf("%-6s %-12s %-35s %-55s %-35s \n",
					json_object_get_string(json_object_object_get(obj, "Index")),
				    json_object_get_string(json_object_object_get(obj, "Group_Name")),
				    wanInterface,
				    LanInterface,
				    criteria);
	}
}

zcfgRet_t zcfgFeDalIntrGrp(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if(!strcmp(method, "POST")) {
			ret = dalcmdIntrGrpAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "PUT")) {
		ret = dalcmdIntrGrpEdit(Jobj, replyMsg, (char *)method);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalIntrGrpDelete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalIntrGrpGet(Jobj, Jarray, NULL);
	}
	else {
		printf("Unknown method:%s\n", method);
	}

	return ret;
}
