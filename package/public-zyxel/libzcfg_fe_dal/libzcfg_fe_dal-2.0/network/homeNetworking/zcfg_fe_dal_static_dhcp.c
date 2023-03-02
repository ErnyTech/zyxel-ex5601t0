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

dal_param_t STATIC_DHCP_param[] =
{
	{"Index", 		dalType_int, 		0, 	0,	NULL,	NULL, 	dal_Edit|dal_Delete},
	{"Enable", 		dalType_boolean, 	0, 	0,	NULL,	NULL, 	dal_Add},
	{"IPAddr", 		dalType_v4Addr, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{"MACAddr", 	dalType_string, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{"BrWan", 		dalType_Lan,		0,	0,	NULL,	NULL,	dal_Add},
	{NULL, 			0, 					0, 	0, 	NULL,	NULL,	0}
};
bool subnetCheck(struct json_object *Jobj){
	bool applyFail = false;
	struct json_object *v4AddrObj = NULL;
	objIndex_t brIid = {0};
	objIndex_t v4AddrIid = {0};
	char *brName = NULL, *staticIPAddr = NULL, *lanIPAddress = NULL, *SubnetMask = NULL;

	IID_INIT(brIid);
	brName = (char *)json_object_get_string(json_object_object_get(Jobj, "BrWan"));
	staticIPAddr = (char *)json_object_get_string(json_object_object_get(Jobj, "IPAddr"));

	IID_INIT(v4AddrIid);
	sscanf(brName, "IP.Interface.%hhu", &v4AddrIid.idx[0]);
	v4AddrIid.level = 2;
	v4AddrIid.idx[1] = 1;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj) == ZCFG_SUCCESS) {
		SubnetMask  = (char *)json_object_get_string(json_object_object_get(v4AddrObj, "SubnetMask"));
		lanIPAddress = (char *)json_object_get_string(json_object_object_get(v4AddrObj, "IPAddress"));
		if(isSameSubNet(lanIPAddress, SubnetMask, staticIPAddr) == false)
			applyFail = true;
	}
   
	zcfgFeJsonObjFree(v4AddrObj);
	return applyFail;
}

void zcfgFeDalShowStaticDHCP(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj = NULL;
	

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-10s %-10s %-30s %-30s \n",
		    "Index", "Status", "Mac Address", "IP Address");
	len = json_object_array_length(Jarray);
	for(i=0; i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-10s %-10d %-30s %-30s \n",
		    json_object_get_string(json_object_object_get(obj, "Index")),
		    json_object_get_boolean(json_object_object_get(obj, "Enable")),
		    json_object_get_string(json_object_object_get(obj, "MACAddr")),
		    json_object_get_string(json_object_object_get(obj, "IPAddr")));
	}
	
}


zcfgRet_t zcfgFeDalStatic_DHCP_Add(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dhcpObj = NULL;
	struct json_object *dhcpSrvObj = NULL;
	objIndex_t dhcpIid = {0};
	objIndex_t dhcpSrvIid = {0};
	//const char *IfName = NULL;
	const char *dhcpIfacePath = NULL;
	//int count = 0;
	bool found = false;
	bool enable = false;
	const char *ipAddr = NULL;
	const char *mac = NULL;
	const char *brwan = NULL;
	//char ifacePath[32] = {0};

	if(subnetCheck(Jobj)){
		printf("subnetCheck return fail\n");
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Home_Networking.StaticDHCP.Error.invalid_subnet"));
		return ZCFG_INVALID_PARAM_VALUE;
	}

	brwan = json_object_get_string(json_object_object_get(Jobj, "BrWan"));

	/*while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ifaceIid, &ipIfaceObj) == ZCFG_SUCCESS) {
		IfName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_IfName"));
		if(!strncmp(IfName, "br", 2)){
			if(targetIdx == count){//found
				sprintf(ifacePath, "IP.Interface.%hhu", ifaceIid.idx[0]);
				zcfgFeJsonObjFree(ipIfaceObj);
				break;
			}
				count++;
		}
		zcfgFeJsonObjFree(ipIfaceObj);
	}*/
	
	IID_INIT(dhcpIid);
	IID_INIT(dhcpSrvIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_SRV_POOL, &dhcpIid, &dhcpObj) == ZCFG_SUCCESS) {
		dhcpIfacePath = json_object_get_string(json_object_object_get(dhcpObj, "Interface"));
		if(!strcmp(brwan, dhcpIfacePath)){
			memcpy(&dhcpSrvIid, &dhcpIid, sizeof(objIndex_t));
			found = true;
			zcfgFeJsonObjFree(dhcpObj);
			break;
		}
		zcfgFeJsonObjFree(dhcpObj);
	}

	if(found){//POST
		ret = zcfgFeObjJsonAdd(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &dhcpSrvIid, NULL);
		if(ret == ZCFG_SUCCESS){
			ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &dhcpSrvIid, &dhcpSrvObj);
		}

		if(ret == ZCFG_SUCCESS){
			enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
			ipAddr = json_object_get_string(json_object_object_get(Jobj, "IPAddr"));
			mac = json_object_get_string(json_object_object_get(Jobj, "MACAddr"));
			json_object_object_add(dhcpSrvObj, "Enable", json_object_new_boolean(enable));
			json_object_object_add(dhcpSrvObj, "Yiaddr", json_object_new_string(ipAddr));
			json_object_object_add(dhcpSrvObj, "Chaddr", json_object_new_string(mac));

			ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &dhcpSrvIid, dhcpSrvObj, NULL);
		}

	}

	zcfgFeJsonObjFree(dhcpSrvObj);
	return ret;
}

zcfgRet_t zcfgFeDal_Static_DHCP_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dhcpSrvObj = NULL;
	objIndex_t dhcpSrvIid = {0};
	int rowIndex = 0;
	int count = 1;
	bool enable = false;
	const char *ipAddr = NULL;
	bool found = false;

	if(subnetCheck(Jobj)){
		printf("subnetCheck return fail\n");
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Home_Networking.StaticDHCP.Error.invalid_subnet"));
		return ZCFG_INVALID_PARAM_VALUE;
	}

	rowIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));

	IID_INIT(dhcpSrvIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &dhcpSrvIid, &dhcpSrvObj) == ZCFG_SUCCESS) {
		if(count == rowIndex){
			found = true;
			break;
		}
			count++;
		zcfgFeJsonObjFree(dhcpSrvObj);
	}
	if(found){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		ipAddr = json_object_get_string(json_object_object_get(Jobj, "IPAddr"));
		if(json_object_object_get(Jobj, "Enable") != NULL)
		json_object_object_add(dhcpSrvObj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "IPAddr") != NULL)
		json_object_object_add(dhcpSrvObj, "Yiaddr", json_object_new_string(ipAddr));

		ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &dhcpSrvIid, dhcpSrvObj, NULL);
		zcfgFeJsonObjFree(dhcpSrvObj);
	}
	return ret;
}
zcfgRet_t zcfgFeDalStatic_DHCP_Delete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dhcpSrvObj = NULL;
	objIndex_t dhcpSrvIid = {0};
	int rowIndex = 0;
	int count = 1;
	bool found = false;

	rowIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	IID_INIT(dhcpSrvIid);

	while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &dhcpSrvIid, &dhcpSrvObj) == ZCFG_SUCCESS) {
		if(count == rowIndex){
			found = true;
			break;
		}
			count++;
		zcfgFeJsonObjFree(dhcpSrvObj);
	}

	if(found){
		ret = zcfgFeObjJsonDel(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &dhcpSrvIid, NULL);
		zcfgFeJsonObjFree(dhcpSrvObj);
	}

	return ret;
	
}

void getBrName(int iid, char *brName){
	struct json_object *brObj = NULL;	
	objIndex_t brIid = {0};
	IID_INIT(brIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_BR, &brIid, &brObj) == ZCFG_SUCCESS) {
		if(iid == brIid.idx[0] ){
			strcpy(brName, json_object_get_string(json_object_object_get(brObj, "X_ZYXEL_BridgeName")));
		}
		zcfgFeJsonObjFree(brObj);
	}
}

void getStaticDHCPObj(int count, int iid, struct json_object *dhcpSrvAddrObj, struct json_object *Jarray){
		struct json_object *paramJobj = NULL;
		char brName[33] = {0};

		paramJobj = json_object_new_object();
		memset(brName, 0, sizeof(char)*33);
		getBrName(iid, brName);
		json_object_object_add(paramJobj, "Index", json_object_new_int(count));
		json_object_object_add(paramJobj, "BrWan", json_object_new_string(brName));
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(dhcpSrvAddrObj,"Enable")));
		json_object_object_add(paramJobj, "MACAddr", JSON_OBJ_COPY(json_object_object_get(dhcpSrvAddrObj,"Chaddr")));
		json_object_object_add(paramJobj, "IPAddr", JSON_OBJ_COPY(json_object_object_get(dhcpSrvAddrObj,"Yiaddr")));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(dhcpSrvAddrObj);
}

zcfgRet_t zcfgFeDalStatic_DHCP_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dhcpSrvAddrObj = NULL;	//Get data model parameter
	objIndex_t dhcpSrvAddrIid = {0};
	int index = 0;

	IID_INIT(dhcpSrvAddrIid);
	if(json_object_object_get(Jobj, "Index")){ //for New GUI get single obj
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		ret = convertIndextoIid(index, &dhcpSrvAddrIid, RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, NULL, NULL, replyMsg);		
		if(ret != ZCFG_SUCCESS)
			return ret;
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &dhcpSrvAddrIid , &dhcpSrvAddrObj) == ZCFG_SUCCESS){
			getStaticDHCPObj(index, dhcpSrvAddrIid.idx[0], dhcpSrvAddrObj, Jarray);
		}
	}else{
		while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &dhcpSrvAddrIid, &dhcpSrvAddrObj) == ZCFG_SUCCESS){
			index ++;
			getStaticDHCPObj(index, dhcpSrvAddrIid.idx[0], dhcpSrvAddrObj, Jarray);
		}
	}
	return ret;
}


zcfgRet_t zcfgFeDalStaticDHCP(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_Static_DHCP_Edit(Jobj, NULL);
	}else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalStatic_DHCP_Add(Jobj, NULL);
	}else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalStatic_DHCP_Delete(Jobj, NULL);
	}else if(!strcmp(method, "GET"))
		ret = zcfgFeDalStatic_DHCP_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

