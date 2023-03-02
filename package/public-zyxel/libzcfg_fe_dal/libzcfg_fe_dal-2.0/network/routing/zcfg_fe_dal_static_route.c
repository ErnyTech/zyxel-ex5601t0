
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define MAX_STATIC_ROUTE 128

dal_param_t STATICROUTE_param[] =
{
	{"Index",				dalType_int,			0,	0,	NULL, NULL, 		dal_Edit|dal_Delete},
	{"Name",				dalType_string,			1,	32,	NULL, NULL, 		dal_Add},
	{"Enable",				dalType_boolean,		0,	0,	NULL, NULL,			dal_Add},
	{"ipver",				dalType_string,			0,	0,	NULL, "IPv4|IPv6",	dal_Add},
	{"DestIPAddress",		dalType_v4Addr,			0,	0,	NULL, NULL,			0},
	{"DestSubnetMask",		dalType_v4Mask,			0,	0,	NULL, NULL,			0},
	{"UseGateIpAddr",		dalType_boolean,		0,	0,	NULL, NULL,			dal_Add},
	{"GatewayIPAddress",	dalType_v4Addr,			0,	0,	NULL, NULL,			0},
	{"DestIPPrefix",		dalType_v6AddrPrefix,	0,	0,	NULL, NULL,			0},
	{"NextHop",				dalType_v6Addr,			0,	0,	NULL, NULL,			0},	
	{"Interface",			dalType_LanRtWanWWan,	0,	0,	NULL, NULL,			dal_Add},
	{NULL,					0,						0,	0,	NULL, NULL,			0},

};

void zcfgFeDalShowStaticRoute(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;
	const char *ipver = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-10s %-10s %-7s %-7s %-45s %-17s %-15s %-10s\n",
		     "Index", "Name", "Enable", "IPver", "DestIPAddress/DestPrefix", "DestMask", "Interface", "Gateway/NextHop");		
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		ipver = json_object_get_string(json_object_object_get(obj, "ipver"));
		if(!strcmp(ipver, "IPv4")){
			printf("%-10s %-10s %-7u %-7s %-45s %-17s %-15s %-10s\n",
					json_object_get_string(json_object_object_get(obj, "Index")),
				    json_object_get_string(json_object_object_get(obj, "Name")),
				    json_object_get_boolean(json_object_object_get(obj, "Enable")),
				    ipver,
				    json_object_get_string(json_object_object_get(obj, "DestIPAddress")),
				    json_object_get_string(json_object_object_get(obj, "DestSubnetMask")),
				    json_object_get_string(json_object_object_get(obj, "Interface")),
				    json_object_get_string(json_object_object_get(obj, "GatewayIPAddress")));
		}
		else{
			printf("%-10s %-10s %-7u %-7s %-45s %-17s %-15s %-10s\n",
					json_object_get_string(json_object_object_get(obj, "Index")),
					json_object_get_string(json_object_object_get(obj, "Name")),
					json_object_get_boolean(json_object_object_get(obj, "Enable")),
					ipver,
					json_object_get_string(json_object_object_get(obj, "DestIPPrefix")),
					"",
					json_object_get_string(json_object_object_get(obj, "Interface")),
					json_object_get_string(json_object_object_get(obj, "NextHop")));
		}
	}
	
}

zcfgRet_t zcfgFeDalStaticRouteGetAll(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;
	bool StaticRoute;
	const char *Alias, *DestIPAddress, *Origin, *DestIPPrefix;
	int count = 1;
	
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, &iid, &obj) == ZCFG_SUCCESS) {
		Alias = json_object_get_string(json_object_object_get(obj, "Alias"));
		DestIPAddress = json_object_get_string(json_object_object_get(obj, "DestIPAddress"));
		StaticRoute = json_object_get_boolean(json_object_object_get(obj, "StaticRoute"));
		if(StaticRoute && strcmp(Alias, "") && strcmp(DestIPAddress, "")){
			pramJobj = json_object_new_object();
			json_object_object_add(pramJobj, "Index", json_object_new_int(count));
			json_object_object_add(pramJobj, "Name", json_object_new_string(Alias));
			json_object_object_add(pramJobj, "DestIPAddress", json_object_new_string(DestIPAddress));
			json_object_object_add(pramJobj, "StaticRoute", json_object_new_boolean(StaticRoute));
			json_object_object_add(pramJobj, "ipver", json_object_new_string("IPv4"));
			json_object_object_add(pramJobj, "GatewayIPAddress", JSON_OBJ_COPY(json_object_object_get(obj, "GatewayIPAddress")));
			json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(obj, "Enable")));
			json_object_object_add(pramJobj, "DestSubnetMask", JSON_OBJ_COPY(json_object_object_get(obj, "DestSubnetMask")));
			json_object_object_add(pramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(obj, "Interface")));
			json_object_object_add(pramJobj, "idx", json_object_new_int(iid.idx[1]));

			json_object_array_add(Jarray, pramJobj);
			count++;
		}
		zcfgFeJsonObjFree(obj);
	}

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &iid, &obj) == ZCFG_SUCCESS) {
		Alias = json_object_get_string(json_object_object_get(obj, "Alias"));
		DestIPPrefix = json_object_get_string(json_object_object_get(obj, "DestIPPrefix"));
		Origin = json_object_get_string(json_object_object_get(obj, "Origin"));
		if(!strcmp(Origin, "Static") && strcmp(Alias, "") && strcmp(DestIPPrefix, "")){
			pramJobj = json_object_new_object();
			json_object_object_add(pramJobj, "Index", json_object_new_int(count));
			json_object_object_add(pramJobj, "Name", json_object_new_string(Alias));
			json_object_object_add(pramJobj, "DestIPPrefix", json_object_new_string(DestIPPrefix));
			json_object_object_add(pramJobj, "ipver", json_object_new_string("IPv6"));
			json_object_object_add(pramJobj, "NextHop", JSON_OBJ_COPY(json_object_object_get(obj, "NextHop")));
			json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(obj, "Enable")));
			json_object_object_add(pramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(obj, "Interface")));
			json_object_object_add(pramJobj, "idx", json_object_new_int(iid.idx[1]));
			
			json_object_array_add(Jarray, pramJobj);
			count++;
		}
		zcfgFeJsonObjFree(obj);
	}
	return ret;
	
}

zcfgRet_t zcfgFeDalStaticRouteEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *StaticRouteObj = NULL, *obj = NULL, *objtmp = NULL;
	objIndex_t StaticRouteIid = {0};
	zcfg_offset_t oid = 0;
	const char *iptype = NULL;
	const char *origiptype = NULL;
	const char *Name = NULL;
	const char *GatewayIPAddress = NULL;
	const char *NextHop = NULL;
	const char *DestIPPrefix = NULL;
	const char *DestIPAddress = NULL;
	const char *DestSubnetMask = NULL;
	const char *Interface = NULL;
	const char *idx = NULL;
	bool Enable = false;
	bool useGateIpaddr = false;
	int len = 0, i, index = 0;
	
	obj = json_object_new_array();
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	Name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	iptype = json_object_get_string(json_object_object_get(Jobj, "ipver"));
	useGateIpaddr = json_object_get_boolean(json_object_object_get(Jobj, "UseGateIpAddr"));
	GatewayIPAddress = json_object_get_string(json_object_object_get(Jobj, "GatewayIPAddress"));
	NextHop = json_object_get_string(json_object_object_get(Jobj, "NextHop"));
	DestIPPrefix = json_object_get_string(json_object_object_get(Jobj, "DestIPPrefix"));
	Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	DestIPAddress = json_object_get_string(json_object_object_get(Jobj, "DestIPAddress"));
	DestSubnetMask = json_object_get_string(json_object_object_get(Jobj, "DestSubnetMask"));
	Interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
	if(!json_object_object_get(Jobj, "GatewayIPAddress"))
		GatewayIPAddress = "";
	zcfgFeDalStaticRouteGetAll(NULL, obj, NULL);
	len = json_object_array_length(obj);
	for(i=0;i<len;i++){
		objtmp = json_object_array_get_idx(obj, i);
		if(index == atoi(json_object_get_string(json_object_object_get(objtmp, "Index")))){
			idx = json_object_get_string(json_object_object_get(objtmp, "idx"));
			origiptype = json_object_get_string(json_object_object_get(objtmp, "ipver"));
			if(json_object_object_get(Jobj, "ipver")){
			if(strcmp(iptype,json_object_get_string(json_object_object_get(objtmp, "ipver")))){
				if(replyMsg != NULL)
					strcat(replyMsg, "The iptype of rule cannot change.");	
				return ZCFG_INVALID_PARAM_VALUE;
			}				
		}
		}
		else{
			if(json_object_object_get(Jobj, "Name")){
			if(!strcmp(Name,json_object_get_string(json_object_object_get(objtmp, "Name")))){
				if(replyMsg != NULL)
					strcat(replyMsg, "Name is duplicate.");			
				return ZCFG_INVALID_PARAM_VALUE;
				}
}
	}
	}
	StaticRouteIid.level = 2;
	StaticRouteIid.idx[0] = 1;
	StaticRouteIid.idx[1] = atoi(idx);
	if(json_object_object_get(Jobj, "ipver")){
	if(!strcmp(iptype,"IPv4")){
		oid = RDM_OID_ROUTING_ROUTER_V4_FWD;
		if(zcfgFeObjJsonGet(oid, &StaticRouteIid, &StaticRouteObj) == ZCFG_SUCCESS){
			if(json_object_object_get(Jobj, "UseGateIpAddr")){
				if(useGateIpaddr){
					json_object_object_add(StaticRouteObj, "GatewayIPAddress", json_object_new_string(GatewayIPAddress));
				}
				else
					json_object_object_add(StaticRouteObj, "GatewayIPAddress", json_object_new_string(""));
			}
			else if(json_object_object_get(Jobj, "GatewayIPAddress"))
				json_object_object_add(StaticRouteObj, "GatewayIPAddress", json_object_new_string(GatewayIPAddress));
				
			if(json_object_object_get(Jobj, "Name"))
				json_object_object_add(StaticRouteObj, "Alias", json_object_new_string(Name));
			if(json_object_object_get(Jobj, "Enable"))
				json_object_object_add(StaticRouteObj, "Enable", json_object_new_boolean(Enable));
			if(json_object_object_get(Jobj, "DestIPAddress"))
				json_object_object_add(StaticRouteObj, "DestIPAddress", json_object_new_string(DestIPAddress));
			if(json_object_object_get(Jobj, "DestSubnetMask"))
				json_object_object_add(StaticRouteObj, "DestSubnetMask", json_object_new_string(DestSubnetMask));
			if(json_object_object_get(Jobj, "Interface"))
				json_object_object_add(StaticRouteObj, "Interface", json_object_new_string(Interface));
		}
	}
	else if(!strcmp(iptype,"IPv6")){
		oid = RDM_OID_ROUTING_ROUTER_V6_FWD;
		if(zcfgFeObjJsonGet(oid, &StaticRouteIid, &StaticRouteObj) == ZCFG_SUCCESS){
			if(json_object_object_get(Jobj, "UseGateIpAddr")){
				if(useGateIpaddr){
					json_object_object_add(StaticRouteObj, "NextHop", json_object_new_string(NextHop));
				}
				else
					json_object_object_add(StaticRouteObj, "NextHop", json_object_new_string(""));
			}
			else if(json_object_object_get(Jobj, "GatewayIPAddress"))
				json_object_object_add(StaticRouteObj, "NextHop", json_object_new_string(NextHop));
			
			if(json_object_object_get(Jobj, "Name"))
				json_object_object_add(StaticRouteObj, "Alias", json_object_new_string(Name));
			if(json_object_object_get(Jobj, "Enable"))
				json_object_object_add(StaticRouteObj, "Enable", json_object_new_boolean(Enable));
			if(json_object_object_get(Jobj, "DestIPPrefix"))
				json_object_object_add(StaticRouteObj, "DestIPPrefix", json_object_new_string(DestIPPrefix));
			if(json_object_object_get(Jobj, "Interface"))
				json_object_object_add(StaticRouteObj, "Interface", json_object_new_string(Interface));
	}
	}
	}
	else{
		if(!strcmp(origiptype, "IPv4")){
			oid = RDM_OID_ROUTING_ROUTER_V4_FWD;
			if(zcfgFeObjJsonGet(oid, &StaticRouteIid, &StaticRouteObj) == ZCFG_SUCCESS){
				if(json_object_object_get(Jobj, "Enable"))
					json_object_object_add(StaticRouteObj, "Enable", json_object_new_boolean(Enable));
				if(json_object_object_get(Jobj, "Name"))
					json_object_object_add(StaticRouteObj, "Alias", json_object_new_string(Name));
				if(json_object_object_get(Jobj, "Interface"))
					json_object_object_add(StaticRouteObj, "Interface", json_object_new_string(Interface));
			}			
		}
		else if(!strcmp(origiptype, "IPv6")){
			oid = RDM_OID_ROUTING_ROUTER_V6_FWD;
			if(zcfgFeObjJsonGet(oid, &StaticRouteIid, &StaticRouteObj) == ZCFG_SUCCESS){
				if(json_object_object_get(Jobj, "Enable"))
					json_object_object_add(StaticRouteObj, "Enable", json_object_new_boolean(Enable));
				if(json_object_object_get(Jobj, "Name"))
					json_object_object_add(StaticRouteObj, "Alias", json_object_new_string(Name));
				if(json_object_object_get(Jobj, "Interface"))
					json_object_object_add(StaticRouteObj, "Interface", json_object_new_string(Interface));
			}
		}
	}
	ret = zcfgFeObjJsonSet(oid, &StaticRouteIid, StaticRouteObj, NULL);
	zcfgFeJsonObjFree(StaticRouteObj);
	return ret;
}

zcfgRet_t zcfgFeDalStaticRouteAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *StaticRouteObj = NULL, *obj = NULL, *objtmp = NULL;
	objIndex_t StaticRouteIid = {0};
	zcfg_offset_t oid = 0;
	const char *iptype = NULL;
	const char *Name = NULL;
	const char *GatewayIPAddress = NULL;
	const char *NextHop = NULL;
	const char *DestIPPrefix = NULL;
	const char *DestIPAddress = NULL;
	const char *DestSubnetMask = NULL;
	const char *Interface = NULL;
	bool Enable = false;
	bool useGateIpaddr = false;
	
	int len = 0, i;
	StaticRouteIid.level = 1;
	StaticRouteIid.idx[0] = 1;
	
	obj = json_object_new_array();
	Name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	iptype = json_object_get_string(json_object_object_get(Jobj, "ipver"));
	useGateIpaddr = json_object_get_boolean(json_object_object_get(Jobj, "UseGateIpAddr"));
	GatewayIPAddress = json_object_get_string(json_object_object_get(Jobj, "GatewayIPAddress"));
	NextHop = json_object_get_string(json_object_object_get(Jobj, "NextHop"));
	DestIPPrefix = json_object_get_string(json_object_object_get(Jobj, "DestIPPrefix"));
	Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	DestIPAddress = json_object_get_string(json_object_object_get(Jobj, "DestIPAddress"));
	DestSubnetMask = json_object_get_string(json_object_object_get(Jobj, "DestSubnetMask"));
	Interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
	if(!useGateIpaddr)
		GatewayIPAddress = "";
	
	zcfgFeDalStaticRouteGetAll(NULL, obj, NULL);
	len = json_object_array_length(obj);
	
	if(len>=MAX_STATIC_ROUTE){
        ret = ZCFG_INVALID_PARAM_VALUE;
        json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Routing.StaticRoute.error.max_rule"));
        return ret;
    }

	for(i=0;i<len;i++){
		objtmp = json_object_array_get_idx(obj, i);
		if(!strcmp(Name,json_object_get_string(json_object_object_get(objtmp, "Name")))){
			if(replyMsg != NULL)
			strcat(replyMsg, "Name is duplicate.");			
			return ZCFG_INVALID_PARAM_VALUE;		
		}	
		if(!strcmp(iptype, "IPv4")){
			if(!strcmp("IPv6",json_object_get_string(json_object_object_get(objtmp, "ipver"))))
				continue;
#ifndef ZYXEL_USA_PRODUCT
			if(!strcmp(Interface,json_object_get_string(json_object_object_get(objtmp, "Interface"))) && !strcmp(DestIPAddress,json_object_get_string(json_object_object_get(objtmp, "DestIPAddress"))) && !strcmp(DestSubnetMask,json_object_get_string(json_object_object_get(objtmp, "DestSubnetMask")))){
				if(replyMsg != NULL)
					strcat(replyMsg, "The rule existed, please enter another destination ip address or subnet mask.");			
				return ZCFG_INVALID_PARAM_VALUE;
			}
#endif
		}
		else if(!strcmp(iptype, "IPv6")){
			if(!strcmp("IPv4",json_object_get_string(json_object_object_get(objtmp, "ipver"))))
				continue;
#ifndef ZYXEL_USA_PRODUCT
			if(!strcmp(Interface,json_object_get_string(json_object_object_get(objtmp, "Interface"))) && !strcmp(DestIPPrefix,json_object_get_string(json_object_object_get(objtmp, "DestIPPrefix"))) && !strcmp(NextHop,json_object_get_string(json_object_object_get(objtmp, "NextHop")))){
				if(replyMsg != NULL)
					strcat(replyMsg, "The rule existed, please enter another destination ip address or prefix length.");			
				return ZCFG_INVALID_PARAM_VALUE;
			}
#endif
		}
	}
	if(!strcmp(iptype,"IPv4")){
		oid = RDM_OID_ROUTING_ROUTER_V4_FWD;
		if(zcfgFeObjJsonAdd(oid, &StaticRouteIid, NULL) == ZCFG_SUCCESS){
			if(zcfgFeObjJsonGet(oid, &StaticRouteIid, &StaticRouteObj) == ZCFG_SUCCESS){
				if(!useGateIpaddr)
					GatewayIPAddress = "";
				json_object_object_add(StaticRouteObj, "Alias", json_object_new_string(Name));
				json_object_object_add(StaticRouteObj, "Enable", json_object_new_boolean(Enable));
				json_object_object_add(StaticRouteObj, "DestIPAddress", json_object_new_string(DestIPAddress));
				json_object_object_add(StaticRouteObj, "DestSubnetMask", json_object_new_string(DestSubnetMask));
				json_object_object_add(StaticRouteObj, "GatewayIPAddress", json_object_new_string(GatewayIPAddress));
				json_object_object_add(StaticRouteObj, "Interface", json_object_new_string(Interface));
			}
		}	
	}
	else if(!strcmp(iptype,"IPv6")){
		oid = RDM_OID_ROUTING_ROUTER_V6_FWD;
		if(zcfgFeObjJsonAdd(oid, &StaticRouteIid, NULL) == ZCFG_SUCCESS){
			if(zcfgFeObjJsonGet(oid, &StaticRouteIid, &StaticRouteObj) == ZCFG_SUCCESS){
				if(!useGateIpaddr)
					NextHop = "";
				json_object_object_add(StaticRouteObj, "Alias", json_object_new_string(Name));
				json_object_object_add(StaticRouteObj, "Enable", json_object_new_boolean(Enable));
				json_object_object_add(StaticRouteObj, "DestIPPrefix", json_object_new_string(DestIPPrefix));
				json_object_object_add(StaticRouteObj, "NextHop", json_object_new_string(NextHop));
				json_object_object_add(StaticRouteObj, "Interface", json_object_new_string(Interface));
	}
	}
	}
	ret = zcfgFeObjJsonSet(oid, &StaticRouteIid, StaticRouteObj, NULL);
	zcfgFeJsonObjFree(StaticRouteObj);
	return ret;
}

zcfgRet_t zcfgFeDalStaticRouteDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL, *objtmp = NULL;
	obj = json_object_new_array();
	objIndex_t StaticRouteIid = {0};
	zcfg_offset_t oid = 0;
	const char *iptype = NULL, *idx = NULL;
	int index = 0, len = 0, i;
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	zcfgFeDalStaticRouteGetAll(NULL, obj, NULL);
	len = json_object_array_length(obj);
	for(i=0;i<len;i++){
		objtmp = json_object_array_get_idx(obj, i);
		if(index == atoi(json_object_get_string(json_object_object_get(objtmp, "Index")))){
			iptype = json_object_get_string(json_object_object_get(objtmp, "ipver"));
			idx = json_object_get_string(json_object_object_get(objtmp, "idx"));
			break;
		}
	}
	if(!strcmp(iptype,"IPv4"))
		oid = RDM_OID_ROUTING_ROUTER_V4_FWD;
	else if(!strcmp(iptype, "IPv6"))
		oid = RDM_OID_ROUTING_ROUTER_V6_FWD;
			
	StaticRouteIid.level = 2;
	StaticRouteIid.idx[0] = 1;
	if(idx!=NULL)
	{
		StaticRouteIid.idx[1] = atoi(idx);
	}
	else
	{
		printf("In funtion %s: idx is NULL\n",__func__);
	}
	ret = zcfgFeObjJsonDel(oid, &StaticRouteIid, NULL);
		return ret;
	}


zcfgRet_t zcfgFeDalStaticRoute(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalStaticRouteEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalStaticRouteAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalStaticRouteDelete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalStaticRouteGetAll(Jobj, Jarray, replyMsg);
	}

	return ret;
}
