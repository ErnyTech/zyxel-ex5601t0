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

#ifdef ZYXEL_TAAAB_CUSTOMIZATION
dal_param_t IP_Filter_param[] =
{
	{"Filter_Enable", 	dalType_boolean, 	0, 	0, 	NULL,	NULL,	0},
	{"Restrict", 		dalType_boolean, 	0, 	0, 	NULL,	NULL,	0},
	{"Index", 			dalType_int, 		0, 	0, 	NULL,	NULL,	dal_Delete},
	{"Enable", 			dalType_boolean, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{"HostName", 		dalType_string, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{"IPAddress", 		dalType_string, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{NULL, 					0, 				0, 	0, 	NULL}
};
void zcfgFeDalShowIPFilter(struct json_object *Jarray){
	int len = 0, i;
	struct json_object *obj = NULL;
	struct json_object *obj1 = NULL;
	struct json_object *IPObj = NULL;
	struct json_object *whilelistObj = NULL;
	const char *RestrictMode = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	IPObj = json_object_object_get(obj, "IP");
	whilelistObj = json_object_object_get(obj, "whitelist");

	printf("%-30s %s \n","IP Address Filter :", json_object_get_string(json_object_object_get(IPObj, "Filter_Enable")));
	printf("%-30s %s \n","IP Restrict Mode :", json_object_get_string(json_object_object_get(IPObj, "Restrict")));
	printf("%-10s %-10s %-25s %-10s\n","Index", "Status", "Host Name", "IP Address");
	len = json_object_array_length(whilelistObj);
	for(i=0;i<len;i++){
		obj1 = json_object_array_get_idx(whilelistObj, i);
		printf("%-10s %-10s %-25s %-10s\n",
			json_object_get_string(json_object_object_get(obj1, "Index")),
			json_object_get_string(json_object_object_get(obj1, "List_Enable")),
			json_object_get_string(json_object_object_get(obj1, "HostName")),
			json_object_get_string(json_object_object_get(obj1, "IPAddress")));
	}
}


zcfgRet_t setIPFilterEnable(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ipfilterObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t ipfilterIid = {0};
	bool filterEnable = false;
	bool restrictMode = false;
	int reqObjIdx = 0;
	
	IID_INIT(ipfilterIid); 
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IP_FILTER, &ipfilterIid, &ipfilterObj);
	 
	reqObject = json_object_array_get_idx(Jobj, reqObjIdx);

	if(reqObject && ret == ZCFG_SUCCESS){
		filterEnable = json_object_get_boolean(json_object_object_get(reqObject, "Filter_Enable"));
		restrictMode = json_object_get_boolean(json_object_object_get(reqObject, "Restrict"));
		json_object_object_add(ipfilterObj, "Enable", json_object_new_boolean(filterEnable));
		json_object_object_add(ipfilterObj, "Restrict", json_object_new_boolean(restrictMode));
			
		zcfgFeObjJsonSet(RDM_OID_ZY_IP_FILTER, &ipfilterIid, ipfilterObj, NULL);
	}

	zcfgFeJsonObjFree(ipfilterObj);
	return ret;
}

zcfgRet_t zcfgFeDal_IP_Filter_Add(struct json_object *Jobj, char *replyMsg){	//for dalcmd 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *whiteListObj = NULL;
	objIndex_t whiteListIid = {0};
	bool enable = false;
	const char *ipaddr = NULL;
	const char *hostname = NULL;
	zcfg_offset_t oid;
	
	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	ipaddr = json_object_get_string(json_object_object_get(Jobj, "IPAddress"));
	hostname = json_object_get_string(json_object_object_get(Jobj, "HostName"));
	oid = RDM_OID_ZY_IP_FILTER_WHITE_LIST;
	if(json_object_object_get(Jobj, "HostName")){
		ret = dalcmdParamCheck(NULL, hostname, oid, "HostName", "HostName", replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	}
	if(json_object_object_get(Jobj, "IPAddress")){
		ret = dalcmdParamCheck(NULL, ipaddr, oid, "IPAddress", "IPAddress", replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	}
	ret = zcfgFeObjJsonAdd(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGet(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, &whiteListObj);
		if(ret == ZCFG_SUCCESS){
			json_object_object_add(whiteListObj, "Enable", json_object_new_boolean(enable));
			json_object_object_add(whiteListObj, "IPAddress", json_object_new_string(ipaddr));
			json_object_object_add(whiteListObj, "HostName", json_object_new_string(hostname));

			ret = zcfgFeObjJsonSet(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, whiteListObj, NULL);
			zcfgFeJsonObjFree(whiteListObj);
		}		
	}
	return ret;
}

zcfgRet_t zcfgFeDal_IP_Filter_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *whiteListObj = NULL;
	struct json_object *reqObject = NULL;
	struct json_object *ipObj = NULL;
	objIndex_t whiteListIid = {0};
	objIndex_t ipIid = {0};
	int reqObjIdx = 1;
	const char *hostName = NULL;
	const char *IPAddress = NULL;
	const char *action = NULL;
	bool listEnable = false;
	int index = 0;
	bool enable = false;
	bool filterEnable = false;
	bool restrictMode = false;
	zcfg_offset_t oid;

	if(json_object_get_type(Jobj) == json_type_object){// set by dalcmd
		if(json_object_object_get(Jobj, "Filter_Enable") || json_object_object_get(Jobj, "Restrict")){

			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IP_FILTER, &ipIid, &ipObj) == ZCFG_SUCCESS){
				filterEnable = json_object_get_boolean(json_object_object_get(Jobj, "Filter_Enable"));
				restrictMode = json_object_get_boolean(json_object_object_get(Jobj, "Restrict"));

				if(json_object_object_get(Jobj, "Filter_Enable"))
					json_object_object_add(ipObj, "Enable", json_object_new_boolean(filterEnable));
				if(json_object_object_get(Jobj, "Restrict"))
					json_object_object_add(ipObj, "Restrict", json_object_new_boolean(restrictMode));
				ret = zcfgFeObjJsonSet(RDM_OID_ZY_IP_FILTER, &ipIid, ipObj, NULL);
				zcfgFeJsonObjFree(ipObj);
			}
		}else{
 
			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IP_FILTER, &ipIid, &ipObj) == ZCFG_SUCCESS){
				json_object_object_add(Jobj, "Filter_Enable", JSON_OBJ_COPY(json_object_object_get(ipObj, "Enable")));
				json_object_object_add(Jobj, "Restrict", JSON_OBJ_COPY(json_object_object_get(ipObj, "Restrict")));
				ret = zcfgFeObjJsonSet(RDM_OID_ZY_IP_FILTER, &ipIid, ipObj, NULL);
				zcfgFeJsonObjFree(ipObj);
			}

		}
	
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		hostName = json_object_get_string(json_object_object_get(Jobj, "HostName"));
		IPAddress = json_object_get_string(json_object_object_get(Jobj, "IPAddress"));
		oid = RDM_OID_ZY_IP_FILTER_WHITE_LIST;
		if(json_object_object_get(Jobj, "Index")){
			ret = convertIndextoIid(index, &whiteListIid, oid, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
			if(json_object_object_get(Jobj, "HostName")){	
				ret = dalcmdParamCheck(&whiteListIid, hostName, oid, "HostName", "HostName", replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
			}
			if(json_object_object_get(Jobj, "IPAddress")){
				ret = dalcmdParamCheck(&whiteListIid, IPAddress, oid, "IPAddress", "IPAddress", replyMsg);
				if(ret != ZCFG_SUCCESS)
					return ret;
			}
		}
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, &whiteListObj) == ZCFG_SUCCESS){
			if(json_object_object_get(Jobj, "Enable"))
				json_object_object_add(whiteListObj, "Enable", json_object_new_boolean(enable));
			if(json_object_object_get(Jobj, "IPAddress"))
				json_object_object_add(whiteListObj, "IPAddress", json_object_new_string(IPAddress));
			if(json_object_object_get(Jobj, "HostName"))
				json_object_object_add(whiteListObj, "HostName", json_object_new_string(hostName));

			ret = zcfgFeObjJsonSet(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, whiteListObj, NULL);
			zcfgFeJsonObjFree(whiteListObj);
		}
		return ret;
	}

	
	while((reqObject = json_object_array_get_idx(Jobj, reqObjIdx)) !=NULL){
		IID_INIT(whiteListIid);
		index = json_object_get_int(json_object_object_get(reqObject, "Index"));
		action = json_object_get_string(json_object_object_get(reqObject, "Action"));
		listEnable = json_object_get_boolean(json_object_object_get(reqObject, "List_Enable"));
		hostName = json_object_get_string(json_object_object_get(reqObject, "HostName"));
		IPAddress = json_object_get_string(json_object_object_get(reqObject, "IPAddress"));
		oid = RDM_OID_ZY_IP_FILTER_WHITE_LIST;
		reqObjIdx++;
		
		if(!strcmp(action, "DELE")){
			ret = convertIndextoIid(index, &whiteListIid, oid, NULL, NULL, replyMsg);
			if(ret != ZCFG_SUCCESS)
				return ret;
			ret = zcfgFeObjJsonDel(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, NULL);
			continue;
		}else if(!strcmp(action, "ADD")){
			if(json_object_object_get(reqObject, "HostName")){	
				ret = dalcmdParamCheck(&whiteListIid, hostName, oid, "HostName", "HostName", replyMsg);
				if(ret != ZCFG_SUCCESS){
					printf("%s\n",replyMsg);
					json_object_object_add(reqObject, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_name"));
					json_object_array_add(Jobj, reqObject);
					return ret;
				}
			}
			if(json_object_object_get(reqObject, "IPAddress")){
				ret = dalcmdParamCheck(&whiteListIid, IPAddress, oid, "IPAddress", "IPAddress", replyMsg);
				if(ret != ZCFG_SUCCESS){
					printf("%s\n",replyMsg);
					json_object_object_add(reqObject, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_ip"));
					json_object_array_add(Jobj, reqObject);
					return ret;
				}				
			}
			ret = zcfgFeObjJsonAdd(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, NULL);
			if(ret == ZCFG_SUCCESS){
				ret = zcfgFeObjJsonGet(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, &whiteListObj);
			}
		}else if(!strcmp(action, "EDIT")){
			ret = convertIndextoIid(index, &whiteListIid, oid, NULL, NULL, replyMsg);
			if(ret != ZCFG_SUCCESS)
				return ret;
			if(json_object_object_get(reqObject, "HostName")){	
				ret = dalcmdParamCheck(&whiteListIid, hostName, oid, "HostName", "HostName", replyMsg);
				if(ret != ZCFG_SUCCESS){
					printf("%s\n",replyMsg);
					json_object_object_add(reqObject, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_name"));
					json_object_array_add(Jobj, reqObject);
					return ret;
				}
			}
			if(json_object_object_get(reqObject, "IPAddress")){
				ret = dalcmdParamCheck(&whiteListIid, IPAddress, oid, "IPAddress", "IPAddress", replyMsg);
				if(ret != ZCFG_SUCCESS){
					printf("%s\n",replyMsg);
					json_object_object_add(reqObject, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_ip"));
					json_object_array_add(Jobj, reqObject);
					return ret;
				}				
			}
			ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, &whiteListObj);
		}else{
			printf("Unknown action:%s\n", action);
			ret = ZCFG_NOT_FOUND;
		}

		//set Param
		if(ret == ZCFG_SUCCESS){
			if(json_object_object_get(reqObject, "List_Enable"))
				json_object_object_add(whiteListObj, "Enable", json_object_new_boolean(listEnable));
			if(json_object_object_get(reqObject, "IPAddress"))
				json_object_object_add(whiteListObj, "IPAddress", json_object_new_string(IPAddress));
			if(json_object_object_get(reqObject, "HostName"))
				json_object_object_add(whiteListObj, "HostName", json_object_new_string(hostName));
			zcfgFeObjJsonSet(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, whiteListObj, NULL);
		}
		zcfgFeJsonObjFree(whiteListObj);
	}
	
	setIPFilterEnable(Jobj);
	return ret;
	}
	
zcfgRet_t zcfgFeDal_IP_Filter_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *paramJobj1 = NULL;
	struct json_object *ipfilterObj = NULL;
	struct json_object *whiteListObj = NULL, *whiteListtmpObj = NULL;
	struct json_object *allObj = NULL;
	objIndex_t ipfilterIid = {0};
	objIndex_t whiteListIid = {0};
	int count  = 1;
	allObj = json_object_new_object();
	whiteListtmpObj = json_object_new_array();

	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IP_FILTER, &ipfilterIid, &ipfilterObj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();

		json_object_object_add(paramJobj, "Filter_Enable", JSON_OBJ_COPY(json_object_object_get(ipfilterObj, "Enable")));
		json_object_object_add(paramJobj, "Restrict", JSON_OBJ_COPY(json_object_object_get(ipfilterObj, "Restrict")));
		json_object_object_add(allObj, "IP", paramJobj);
		zcfgFeJsonObjFree(ipfilterObj);
	}
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_IP_FILTER_WHITE_LIST, &whiteListIid, &whiteListObj) == ZCFG_SUCCESS){
		paramJobj1 = json_object_new_object();
		json_object_object_add(paramJobj1, "Index", json_object_new_int(count));
		json_object_object_add(paramJobj1, "List_Enable", JSON_OBJ_COPY(json_object_object_get(whiteListObj, "Enable")));
		json_object_object_add(paramJobj1, "IPAddress", JSON_OBJ_COPY(json_object_object_get(whiteListObj, "IPAddress")));
		json_object_object_add(paramJobj1, "HostName", JSON_OBJ_COPY(json_object_object_get(whiteListObj, "HostName")));
		json_object_array_add(whiteListtmpObj, paramJobj1);
	zcfgFeJsonObjFree(whiteListObj);
		count++;
	}
	json_object_object_add(allObj, "whitelist", whiteListtmpObj);
	json_object_array_add(Jarray, allObj);
	return ret;
}

zcfgRet_t zcfgFeDal_IP_Filter_Delete(struct json_object *Jobj, char *replyMsg){	//for dalcmd
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t whiteListIid = {0};
	int index = 0;
	zcfg_offset_t oid;
	oid = RDM_OID_ZY_IP_FILTER_WHITE_LIST;
	
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(index, &whiteListIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	
	zcfgFeObjJsonDel(oid, &whiteListIid, NULL);

	return ret;
}


zcfgRet_t zcfgFeDalIPFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;
	if(!strcmp(method, "POST"))
		ret = zcfgFeDal_IP_Filter_Add(Jobj, replyMsg);
	else if(!strcmp(method, "PUT")) 
		ret = zcfgFeDal_IP_Filter_Edit(Jobj, replyMsg);
	else if(!strcmp(method, "DELETE"))
		ret = zcfgFeDal_IP_Filter_Delete(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_IP_Filter_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
#endif

dal_param_t MAC_Filter_param[] =
{
	{"Filter_Enable", 	dalType_boolean, 	0, 	0, 	NULL,	NULL,	0},
	{"Restrict", 		dalType_boolean, 	0, 	0, 	NULL,	NULL,	0},
	{"Index", 			dalType_int, 		0, 	0, 	NULL,	NULL,	dal_Delete},
	{"Enable", 			dalType_boolean, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{"HostName", 		dalType_string, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{"MACAddress", 		dalType_string, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{NULL, 					0, 				0, 	0, 	NULL}
};
void zcfgFeDalShowMACFilter(struct json_object *Jarray){
	int len = 0, i;
	struct json_object *obj = NULL;
	struct json_object *obj1 = NULL;
	struct json_object *MACObj = NULL;
	struct json_object *whilelistObj = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	MACObj = json_object_object_get(obj, "MAC");
	whilelistObj = json_object_object_get(obj, "whitelist");

	printf("%-30s %s \n","MAC Address Filter :", json_object_get_string(json_object_object_get(MACObj, "Filter_Enable")));
	printf("%-30s %s \n","MAC Restrict Mode :", json_object_get_string(json_object_object_get(MACObj, "Restrict")));
	printf("%-10s %-10s %-25s %-10s\n","Index", "Status", "Host Name", "Mac Address");
	len = json_object_array_length(whilelistObj);
	for(i=0;i<len;i++){
		obj1 = json_object_array_get_idx(whilelistObj, i);
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_FIREWALL_MAC_DEST_FILTER)
		printf("%-10s %-10s %-25s %-10s %-10s\n",
                        json_object_get_string(json_object_object_get(obj1, "Index")),
                        json_object_get_string(json_object_object_get(obj1, "List_Enable")),
                        json_object_get_string(json_object_object_get(obj1, "HostName")),
                        json_object_get_string(json_object_object_get(obj1, "MACAddress")),
			json_object_get_string(json_object_object_get(obj1, "List_Dest")));
#else
		printf("%-10s %-10s %-25s %-10s\n",
			json_object_get_string(json_object_object_get(obj1, "Index")),
			json_object_get_string(json_object_object_get(obj1, "List_Enable")),
			json_object_get_string(json_object_object_get(obj1, "HostName")),
			json_object_get_string(json_object_object_get(obj1, "MACAddress")));
#endif
	}
}


zcfgRet_t setMACFilterEnable(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *macfilterObj = NULL;
	struct json_object *reqObject = NULL;
	objIndex_t macfilterIid = {0};
	bool filterEnable = false;
	bool restrictMode = false;
	int reqObjIdx = 0;
	
	IID_INIT(macfilterIid); 
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MAC_FILTER, &macfilterIid, &macfilterObj);
	 
	reqObject = json_object_array_get_idx(Jobj, reqObjIdx);

	if(reqObject && ret == ZCFG_SUCCESS){
		filterEnable = json_object_get_boolean(json_object_object_get(reqObject, "Filter_Enable"));
		restrictMode = json_object_get_boolean(json_object_object_get(reqObject, "Restrict"));
		json_object_object_add(macfilterObj, "Enable", json_object_new_boolean(filterEnable));
		json_object_object_add(macfilterObj, "Restrict", json_object_new_boolean(restrictMode));
			
		zcfgFeObjJsonSet(RDM_OID_ZY_MAC_FILTER, &macfilterIid, macfilterObj, NULL);
	}

	zcfgFeJsonObjFree(macfilterObj);
	return ret;
}

zcfgRet_t zcfgFeDal_MAC_Filter_Add(struct json_object *Jobj, char *replyMsg){	//for dalcmd 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *whiteListObj = NULL;
	objIndex_t whiteListIid = {0};
	bool enable = false;
	const char *macaddr = NULL;
	const char *hostname = NULL;
	zcfg_offset_t oid;
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_FIREWALL_MAC_DEST_FILTER)
	bool dest = false;
#endif
	
	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	macaddr = json_object_get_string(json_object_object_get(Jobj, "MACAddress"));
	hostname = json_object_get_string(json_object_object_get(Jobj, "HostName"));
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_FIREWALL_MAC_DEST_FILTER)	
	dest = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_Dest"));
#endif
	oid = RDM_OID_ZY_MAC_FILTER_WHITE_LIST;
	if(json_object_object_get(Jobj, "HostName")){
		ret = dalcmdParamCheck(NULL, hostname, oid, "HostName", "HostName", replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	}
	if(json_object_object_get(Jobj, "MACAddress")){
		ret = dalcmdParamCheck(NULL, macaddr, oid, "MACAddress", "MACAddress", replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	}
	ret = zcfgFeObjJsonAdd(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGet(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, &whiteListObj);
		if(ret == ZCFG_SUCCESS){
			json_object_object_add(whiteListObj, "Enable", json_object_new_boolean(enable));
			json_object_object_add(whiteListObj, "MACAddress", json_object_new_string(macaddr));
			json_object_object_add(whiteListObj, "HostName", json_object_new_string(hostname));
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_FIREWALL_MAC_DEST_FILTER)
			json_object_object_add(whiteListObj, "X_ZYXEL_Dest", json_object_new_boolean(dest));
#endif

			ret = zcfgFeObjJsonSet(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, whiteListObj, NULL);
			zcfgFeJsonObjFree(whiteListObj);
		}		
	}
	return ret;
}

zcfgRet_t zcfgFeDal_MAC_Filter_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *whiteListObj = NULL;
	struct json_object *reqObject = NULL;
	struct json_object *macObj = NULL;
	objIndex_t whiteListIid = {0};
	objIndex_t macIid = {0};
	int reqObjIdx = 1;
	const char *hostName = NULL;
	const char *MACAddress = NULL;
	const char *action = NULL;
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_FIREWALL_MAC_DEST_FILTER)
	const char *dest = NULL;
	bool listDest = false;
#endif
	bool listEnable = false;
	int index = 0;
	bool enable = false;
	bool filterEnable = false;
	bool restrictMode = false;
	zcfg_offset_t oid;

	if(json_object_get_type(Jobj) == json_type_object){// set by dalcmd
		if(json_object_object_get(Jobj, "Filter_Enable") || json_object_object_get(Jobj, "Restrict")){

			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MAC_FILTER, &macIid, &macObj) == ZCFG_SUCCESS){
				filterEnable = json_object_get_boolean(json_object_object_get(Jobj, "Filter_Enable"));
				restrictMode = json_object_get_boolean(json_object_object_get(Jobj, "Restrict"));

				if(json_object_object_get(Jobj, "Filter_Enable"))
					json_object_object_add(macObj, "Enable", json_object_new_boolean(filterEnable));
				if(json_object_object_get(Jobj, "Restrict"))
					json_object_object_add(macObj, "Restrict", json_object_new_boolean(restrictMode));
				ret = zcfgFeObjJsonSet(RDM_OID_ZY_MAC_FILTER, &macIid, macObj, NULL);
				zcfgFeJsonObjFree(macObj);
			}
		}else{
 
			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MAC_FILTER, &macIid, &macObj) == ZCFG_SUCCESS){
				json_object_object_add(Jobj, "Filter_Enable", JSON_OBJ_COPY(json_object_object_get(macObj, "Enable")));
				json_object_object_add(Jobj, "Restrict", JSON_OBJ_COPY(json_object_object_get(macObj, "Restrict")));
				ret = zcfgFeObjJsonSet(RDM_OID_ZY_MAC_FILTER, &macIid, macObj, NULL);
				zcfgFeJsonObjFree(macObj);
			}

		}
	
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		hostName = json_object_get_string(json_object_object_get(Jobj, "HostName"));
		MACAddress = json_object_get_string(json_object_object_get(Jobj, "MACAddress"));
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_FIREWALL_MAC_DEST_FILTER)
		dest = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_Dest"));
#endif
		oid = RDM_OID_ZY_MAC_FILTER_WHITE_LIST;
		if(json_object_object_get(Jobj, "Index")){
			ret = convertIndextoIid(index, &whiteListIid, oid, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
			if(json_object_object_get(Jobj, "HostName")){	
				ret = dalcmdParamCheck(&whiteListIid, hostName, oid, "HostName", "HostName", replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
			}
			if(json_object_object_get(Jobj, "MACAddress")){
				ret = dalcmdParamCheck(&whiteListIid, MACAddress, oid, "MACAddress", "MACAddress", replyMsg);
				if(ret != ZCFG_SUCCESS)
					return ret;
			}
		}
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, &whiteListObj) == ZCFG_SUCCESS){
			if(json_object_object_get(Jobj, "Enable"))
				json_object_object_add(whiteListObj, "Enable", json_object_new_boolean(enable));
			if(json_object_object_get(Jobj, "MACAddress"))
				json_object_object_add(whiteListObj, "MACAddress", json_object_new_string(MACAddress));
			if(json_object_object_get(Jobj, "HostName"))
				json_object_object_add(whiteListObj, "HostName", json_object_new_string(hostName));
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_FIREWALL_MAC_DEST_FILTER)
			if(json_object_object_get(Jobj, "X_ZYXEL_Dest"))
				json_object_object_add(whiteListObj, "X_ZYXEL_Dest", json_object_new_boolean(dest));
#endif

			ret = zcfgFeObjJsonSet(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, whiteListObj, NULL);
			zcfgFeJsonObjFree(whiteListObj);
		}
		return ret;
	}

	
	while((reqObject = json_object_array_get_idx(Jobj, reqObjIdx)) !=NULL){
		IID_INIT(whiteListIid);
		index = json_object_get_int(json_object_object_get(reqObject, "Index"));
		action = json_object_get_string(json_object_object_get(reqObject, "Action"));
		listEnable = json_object_get_boolean(json_object_object_get(reqObject, "List_Enable"));
		hostName = json_object_get_string(json_object_object_get(reqObject, "HostName"));
		MACAddress = json_object_get_string(json_object_object_get(reqObject, "MACAddress"));
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_FIREWALL_MAC_DEST_FILTER)
		listDest = json_object_get_boolean(json_object_object_get(reqObject, "List_Dest"));
#endif
		oid = RDM_OID_ZY_MAC_FILTER_WHITE_LIST;
		reqObjIdx++;
		
		if(!strcmp(action, "DELE")){
			ret = convertIndextoIid(index, &whiteListIid, oid, NULL, NULL, replyMsg);
			if(ret != ZCFG_SUCCESS)
				return ret;
			ret = zcfgFeObjJsonDel(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, NULL);
			continue;
		}else if(!strcmp(action, "ADD")){
			if(json_object_object_get(reqObject, "HostName")){	
				ret = dalcmdParamCheck(&whiteListIid, hostName, oid, "HostName", "HostName", replyMsg);
				if(ret != ZCFG_SUCCESS){
					printf("%s\n",replyMsg);
					json_object_object_add(reqObject, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_name"));
					json_object_array_add(Jobj, reqObject);
					return ret;
				}
			}
			if(json_object_object_get(reqObject, "MACAddress")){
				ret = dalcmdParamCheck(&whiteListIid, MACAddress, oid, "MACAddress", "MACAddress", replyMsg);
				if(ret != ZCFG_SUCCESS){
					printf("%s\n",replyMsg);
					json_object_object_add(reqObject, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_mac"));
					json_object_array_add(Jobj, reqObject);
					return ret;
				}				
			}
			ret = zcfgFeObjJsonAdd(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, NULL);
			if(ret == ZCFG_SUCCESS){
				ret = zcfgFeObjJsonGet(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, &whiteListObj);
			}
		}else if(!strcmp(action, "EDIT")){
			ret = convertIndextoIid(index, &whiteListIid, oid, NULL, NULL, replyMsg);
			if(ret != ZCFG_SUCCESS)
				return ret;
			if(json_object_object_get(reqObject, "HostName")){	
				ret = dalcmdParamCheck(&whiteListIid, hostName, oid, "HostName", "HostName", replyMsg);
				if(ret != ZCFG_SUCCESS){
					printf("%s\n",replyMsg);
					json_object_object_add(reqObject, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_name"));
					json_object_array_add(Jobj, reqObject);
					return ret;
				}
			}
			if(json_object_object_get(reqObject, "MACAddress")){
				ret = dalcmdParamCheck(&whiteListIid, MACAddress, oid, "MACAddress", "MACAddress", replyMsg);
				if(ret != ZCFG_SUCCESS){
					printf("%s\n",replyMsg);
					json_object_object_add(reqObject, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_mac"));
					json_object_array_add(Jobj, reqObject);
					return ret;
				}				
			}
			ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, &whiteListObj);
		}else{
			printf("Unknown action:%s\n", action);
			ret = ZCFG_NOT_FOUND;
		}

		//set Param
		if(ret == ZCFG_SUCCESS){
			if(json_object_object_get(reqObject, "List_Enable"))
				json_object_object_add(whiteListObj, "Enable", json_object_new_boolean(listEnable));
			if(json_object_object_get(reqObject, "MACAddress"))
				json_object_object_add(whiteListObj, "MACAddress", json_object_new_string(MACAddress));
			if(json_object_object_get(reqObject, "HostName"))
				json_object_object_add(whiteListObj, "HostName", json_object_new_string(hostName));
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_FIREWALL_MAC_DEST_FILTER)
			if(json_object_object_get(reqObject, "List_Dest"))
				json_object_object_add(whiteListObj, "X_ZYXEL_Dest", json_object_new_boolean(listDest));
#endif
			zcfgFeObjJsonSet(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, whiteListObj, NULL);
		}
		zcfgFeJsonObjFree(whiteListObj);
	}
	
	setMACFilterEnable(Jobj);
	return ret;
	}
	
zcfgRet_t zcfgFeDal_MAC_Filter_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *paramJobj1 = NULL;
	struct json_object *macfilterObj = NULL;
	struct json_object *whiteListObj = NULL, *whiteListtmpObj = NULL;
	struct json_object *allObj = NULL;
	objIndex_t macfilterIid = {0};
	objIndex_t whiteListIid = {0};
	int count  = 1;
	allObj = json_object_new_object();
	whiteListtmpObj = json_object_new_array();

	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MAC_FILTER, &macfilterIid, &macfilterObj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();

		json_object_object_add(paramJobj, "Filter_Enable", JSON_OBJ_COPY(json_object_object_get(macfilterObj, "Enable")));
		json_object_object_add(paramJobj, "Restrict", JSON_OBJ_COPY(json_object_object_get(macfilterObj, "Restrict")));
		json_object_object_add(allObj, "MAC", paramJobj);
		zcfgFeJsonObjFree(macfilterObj);
	}
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_MAC_FILTER_WHITE_LIST, &whiteListIid, &whiteListObj) == ZCFG_SUCCESS){
		paramJobj1 = json_object_new_object();
		json_object_object_add(paramJobj1, "Index", json_object_new_int(count));
		json_object_object_add(paramJobj1, "List_Enable", JSON_OBJ_COPY(json_object_object_get(whiteListObj, "Enable")));
		json_object_object_add(paramJobj1, "MACAddress", JSON_OBJ_COPY(json_object_object_get(whiteListObj, "MACAddress")));
		json_object_object_add(paramJobj1, "HostName", JSON_OBJ_COPY(json_object_object_get(whiteListObj, "HostName")));
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_FIREWALL_MAC_DEST_FILTER)
		json_object_object_add(paramJobj1, "List_Dest", JSON_OBJ_COPY(json_object_object_get(whiteListObj, "X_ZYXEL_Dest")));
#endif
		json_object_array_add(whiteListtmpObj, paramJobj1);
	zcfgFeJsonObjFree(whiteListObj);
		count++;
	}
	json_object_object_add(allObj, "whitelist", whiteListtmpObj);
	json_object_array_add(Jarray, allObj);
	return ret;
}

zcfgRet_t zcfgFeDal_MAC_Filter_Delete(struct json_object *Jobj, char *replyMsg){	//for dalcmd
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t whiteListIid = {0};
	int index = 0;
	zcfg_offset_t oid;
	oid = RDM_OID_ZY_MAC_FILTER_WHITE_LIST;
	
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(index, &whiteListIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	
	zcfgFeObjJsonDel(oid, &whiteListIid, NULL);

	return ret;
}


zcfgRet_t zcfgFeDalMACFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;
	if(!strcmp(method, "POST"))
		ret = zcfgFeDal_MAC_Filter_Add(Jobj, replyMsg);
	else if(!strcmp(method, "PUT")) 
		ret = zcfgFeDal_MAC_Filter_Edit(Jobj, replyMsg);
	else if(!strcmp(method, "DELETE"))
		ret = zcfgFeDal_MAC_Filter_Delete(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_MAC_Filter_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

