#include <json/json.h>
#include <time.h>

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
#include "ipv6Spec_parameter.h"

extern tr98Object_t tr98Obj[];

zcfgRet_t zyIpv6PppObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *pppObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_PPP, &objIid, &pppObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/	
		if(!strcmp(paramList->name, "SupportedNCPs")) {
			json_object_object_add(*tr98Jobj, "SupportedNCPs", json_object_new_string("IPCP,IPv6CP"));
			paramList++;
			continue;
		}
		
		paramValue = json_object_object_get(pppObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(pppObj);
	return ZCFG_SUCCESS;	
}

zcfgRet_t zyIpv6PppIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *pppIntfObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.PPP.Interface.%hhu", 
		&objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &objIid, &pppIntfObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get ppp intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(pppIntfObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(pppIntfObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6PppIntfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *pppIntfObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.PPP.Interface.%hhu", 
		&objIid.idx[0]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &objIid, &pppIntfObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get ppp intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = pppIntfObj;
		pppIntfObj = NULL;
		pppIntfObj = zcfgFeJsonMultiObjAppend(RDM_OID_PPP_IFACE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(pppIntfObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(pppIntfObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_PPP_IFACE, &objIid, pppIntfObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(pppIntfObj);
			return ret;
		}
		json_object_put(pppIntfObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6PppIntfObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_PPP_IFACE, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyIpv6PppIntfObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.PPP.Interface.%hhu", 
		&objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_IP_IFACE, &objIid, NULL);
}

zcfgRet_t zyIpv6PppIpv6cpObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *pppIpv6cpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.PPP.Interface.%hhu.IPv6CP", 
		&objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_PPP_INTF_IPV6CP, &objIid, &pppIpv6cpObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get ppp ipv6cp obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(pppIpv6cpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(pppIpv6cpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6RouterObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *routerObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.Routing.Router.%hhu", 
		&objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER, &objIid, &routerObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get router obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		if(!strcmp(paramList->name, "IPv6ForwardingNumberOfEntries")) {
			uint32_t  FWDNum = 0;
			objIndex_t ifsetIid;
			struct json_object *ifsetJobj = NULL;
			IID_INIT(ifsetIid);
			while((ret = feObjJsonSubNext(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, &ifsetIid, &ifsetJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				if(ifsetIid.idx[0]==objIid.idx[0])
					FWDNum++;
				json_object_put(ifsetJobj);
			}
			json_object_object_add(*tr98Jobj, "IPv6ForwardingNumberOfEntries", json_object_new_int(FWDNum));
			paramList++;
			continue;
		}
		
		paramValue = json_object_object_get(routerObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(routerObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6RouterIpv6FwdObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipv6FwdObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.Routing.Router.%hhu.IPv6Forwarding.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, &ipv6FwdObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get router ipv6 fwd obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(ipv6FwdObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(ipv6FwdObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6RouterIpv6FwdObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipv6FwdObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.Routing.Router.%hhu.IPv6Forwarding.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, &ipv6FwdObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get router v6 fwd obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = ipv6FwdObj;
		ipv6FwdObj = NULL;
		ipv6FwdObj = zcfgFeJsonMultiObjAppend(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ipv6FwdObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ipv6FwdObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, ipv6FwdObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ipv6FwdObj);
			return ret;
		}
		json_object_put(ipv6FwdObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6RouterIpv6FwdObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char AdrObjFormate[128] = "InternetGatewayDevice.IPv6Specific.Routing.Router.%hhu.IPv6Forwarding";
	objIndex_t objIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	if(sscanf(tr98FullPathName, AdrObjFormate, &objIid.idx[0]) != 1) 
		return ZCFG_INVALID_OBJECT;	

	if((ret = zcfgFeObjStructAdd(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add InternetGatewayDevice.IPv6Specific.Routing.Router.%hhu.IPv6Forwarding. Fail.\n", __FUNCTION__, objIid.idx[0]);
		return ret;
	}

	*idx = objIid.idx[1];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6RouterIpv6FwdObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.Routing.Router.%hhu.IPv6Forwarding.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	return zcfgFeObjStructDel(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, NULL);
}

zcfgRet_t zyIpv6RouteInfoIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *routeInfoObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.Routing.RouteInformation.InterfaceSetting.%hhu", 
		&objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_RT_INFO_INTF_SET, &objIid, &routeInfoObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get rt info obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		if(!strcmp(paramList->name, "RouteLifetime"))
		{
			//time_t cur_time = time(NULL);
			time_t life_time;
			struct tm *nPtr=NULL;
			char RouteLifeTimeStr[24];

			paramValue = json_object_object_get(routeInfoObj, "X_ZYXEL_RouterLifetimeReal");
			if(paramValue != NULL){
				//printf("X_ZYXEL_RouterLifetimeReal=%d\n",json_object_get_int(paramValue));
				life_time=json_object_get_int(paramValue);
				nPtr = localtime(&life_time);
				memset(RouteLifeTimeStr, 0, sizeof(RouteLifeTimeStr));
				sprintf(RouteLifeTimeStr,"%u-%02u%02uT%02u:%02u:%02uZ",nPtr->tm_year + 1900,
															nPtr->tm_mon + 1,
															nPtr->tm_mday,
															nPtr->tm_hour,
															nPtr->tm_min,
															nPtr->tm_sec);
			}
			else
			{
				memset(RouteLifeTimeStr, 0, sizeof(RouteLifeTimeStr));
				sprintf(RouteLifeTimeStr,"0001-01-01T00:00:00Z");
			}
			//printf("RouteLifeTimeStr=%s\n",RouteLifeTimeStr);
			/*change format*/
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(RouteLifeTimeStr));
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(routeInfoObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(routeInfoObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6NDObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ndObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s enter\n",__FUNCTION__);
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_NEIGHBOR_DISCOVERY, &objIid, &ndObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		if(!strcmp(paramList->name, "InterfaceSettingNumberOfEntries")) {
			uint32_t NDNum = 0;
#ifdef CONFIG_ZCFG_BE_MODULE_NEIGHBOR_DISCOVERY_INTFSET
			objIndex_t ifsetIid;
			struct json_object *ifsetJobj = NULL;
			IID_INIT(ifsetIid);
			while((ret = feObjJsonGetNext(RDM_OID_NEIGHBOR_DISCOVERY_INTF_SET, &ifsetIid, &ifsetJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				NDNum++;
				json_object_put(ifsetJobj);
			}
#endif
			json_object_object_add(*tr98Jobj, "InterfaceSettingNumberOfEntries", json_object_new_int(NDNum));
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(ndObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(ndObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6NDObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ndObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NEIGHBOR_DISCOVERY, &objIid, &ndObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = ndObj;
		ndObj = NULL;
		ndObj = zcfgFeJsonMultiObjAppend(RDM_OID_NEIGHBOR_DISCOVERY, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		if(!strcmp(paramList->name, "Enable")) {
#ifdef CONFIG_ZCFG_BE_MODULE_NEIGHBOR_DISCOVERY_INTFSET
			objIndex_t ndIntfIid;
			struct json_object *ndIntfObj = NULL;
			IID_INIT(ndIntfIid);
			while((ret = zcfgFeObjJsonGetNext(RDM_OID_NEIGHBOR_DISCOVERY_INTF_SET, &ndIntfIid, &ndIntfObj)) == ZCFG_SUCCESS) 
			{
				paramValue = json_object_object_get(tr98Jobj, paramList->name);
				json_object_object_add(ndIntfObj, "X_ZYXEL_NDEnable", JSON_OBJ_COPY(paramValue));
				ret = zcfgFeObjJsonSet(RDM_OID_NEIGHBOR_DISCOVERY_INTF_SET, &ndIntfIid, ndIntfObj, NULL);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					json_object_put(ndIntfObj);
					return ret;
				}
				json_object_put(ndIntfObj);
			}
#else
			printf("%s : RDM_OID_NEIGHBOR_DISCOVERY_INTF_SET is not supported\n", __func__);
#endif
		}

		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ndObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ndObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NEIGHBOR_DISCOVERY, &objIid, ndObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ndObj);
			return ret;
		}
		json_object_put(ndObj);
	}

	return ZCFG_SUCCESS;
}

#ifdef CONFIG_ZCFG_BE_MODULE_NEIGHBOR_DISCOVERY_INTFSET
zcfgRet_t zyIpv6NDIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ndIntfSvrObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.NeighborDiscovery.InterfaceSetting.%hhu", 
		&objIid.idx[0]);

	//printf("objIid.idx[0]=%d\n",objIid.idx[0]);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NEIGHBOR_DISCOVERY_INTF_SET, &objIid, &ndIntfSvrObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get nd intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "Status")) {
			bool enable = false;
			paramValue = json_object_object_get(ndIntfSvrObj, "Enable");
			if(paramValue != NULL)
			{
				enable = json_object_get_boolean(paramValue);
			}
			if(enable)
				json_object_object_add(*tr98Jobj, "Status", json_object_new_string("Enabled"));
			else
				json_object_object_add(*tr98Jobj, "Status", json_object_new_string("Disable"));
			paramList++;
			continue;
		}
		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(ndIntfSvrObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(ndIntfSvrObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6NDIntfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ndIntfSvrObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.NeighborDiscovery.InterfaceSetting.%hhu", 
		&objIid.idx[0]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_NEIGHBOR_DISCOVERY_INTF_SET, &objIid, &ndIntfSvrObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get ip intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = ndIntfSvrObj;
		ndIntfSvrObj = NULL;
		ndIntfSvrObj = zcfgFeJsonMultiObjAppend(RDM_OID_NEIGHBOR_DISCOVERY_INTF_SET, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ndIntfSvrObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ndIntfSvrObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NEIGHBOR_DISCOVERY_INTF_SET, &objIid, ndIntfSvrObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ndIntfSvrObj);
			return ret;
		}
		json_object_put(ndIntfSvrObj);
	}
	return ZCFG_SUCCESS;
}
#endif // CONFIG_ZCFG_BE_MODULE_NEIGHBOR_DISCOVERY_INTFSET

zcfgRet_t zyIpv6IpObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP, &objIid, &ipObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/	
		if(!strcmp(paramList->name, "IPv6Capable")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "IPv6Status")) {
			bool IPv6Enable = true;
			paramValue = json_object_object_get(ipObj, "IPv6Enable");
			if(paramValue != NULL)
			{
				IPv6Enable = json_object_get_boolean(paramValue);
			}
			printf("IPv6Enable=%d\n",IPv6Enable);
			if(IPv6Enable)
				json_object_object_add(*tr98Jobj, "IPv6Status", json_object_new_string("Enabled"));
			else
				json_object_object_add(*tr98Jobj, "IPv6Status", json_object_new_string("Disable"));
			paramList++;
			continue;
		}
		paramValue = json_object_object_get(ipObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(ipObj);
	return ZCFG_SUCCESS;	
}

zcfgRet_t zyIpv6IpObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP, &objIid, &ipObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = ipObj;
		ipObj = NULL;
		ipObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name, "IPv6Enable")) {
				objIndex_t ipIntfIid;
				struct json_object *ipIntfObj = NULL;
				IID_INIT(ipIntfIid);
				while((ret = zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipIntfIid, &ipIntfObj)) == ZCFG_SUCCESS) 
				{
					paramValue = json_object_object_get(tr98Jobj, paramList->name);
					//printf("X_ZYXEL_Global_IPv6Enable=%d\n",json_object_get_int(paramValue));
					json_object_object_add(ipIntfObj, "X_ZYXEL_Global_IPv6Enable", JSON_OBJ_COPY(paramValue));
					ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ipIntfIid, ipIntfObj, NULL);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						json_object_put(ipIntfObj);
						return ret;
					}
					json_object_put(ipIntfObj);
				}
			}
			
			tr181ParamValue = json_object_object_get(ipObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ipObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP, &objIid, ipObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ipObj);
			return ret;
		}
		json_object_put(ipObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6IpIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipIntfObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu", 
		&objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &objIid, &ipIntfObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get ip intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(ipIntfObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(ipIntfObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6IpIntfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipIntfObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu", 
		&objIid.idx[0]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &objIid, &ipIntfObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get ip intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = ipIntfObj;
		ipIntfObj = NULL;
		ipIntfObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ipIntfObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ipIntfObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &objIid, ipIntfObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ipIntfObj);
			return ret;
		}
		json_object_put(ipIntfObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6IpIntfObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyIpv6IpIntfObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu", 
		&objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_IP_IFACE, &objIid, NULL);
}

zcfgRet_t zyIpv6IpIntfIpv6AddrObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipIntfAddrObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu.IPv6Address.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &objIid, &ipIntfAddrObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get ipv6 adrr obj codec object ret=%d\n", __FUNCTION__, ret);
		printf("%s : return ret\n", __FUNCTION__);
		return ret;
	}
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "IPAddressStatus")) {
			int cur_time;
			int prelife_time;
			int valid_time;
			int now_time;
			char statStr[24]={0};

			paramValue = json_object_object_get(ipIntfAddrObj, "X_ZYXEL_PreferredLifetime");
			prelife_time=json_object_get_int(paramValue);
			paramValue = json_object_object_get(ipIntfAddrObj, "X_ZYXEL_ValidLifetime");
			valid_time=json_object_get_int(paramValue);
			paramValue = json_object_object_get(ipIntfAddrObj, "X_ZYXEL_CurrentTime");
			cur_time=json_object_get_int(paramValue);
			now_time=time(NULL);
			//printf("prelife_time=%d, valid_time=%d, cur_time=%d, now_time=%d\n",prelife_time,valid_time,cur_time,now_time);

			/*|--------Preferred-------|---Deprecated---|---Invalid----*/
			/*|-----Preferred lifeTime---|----------------------------*/
			/*|------------Valid lifeTime---------------|------------*/
			if((prelife_time==0)||(valid_time==0)||(cur_time==0))
			{
				sprintf(statStr,"Invalid");
			}
			else if(now_time<=(cur_time+prelife_time))
			{
				sprintf(statStr,"Preferred");
			}
			else if(((cur_time+prelife_time)<now_time)&&(now_time<=(cur_time+valid_time)))
			{
				sprintf(statStr,"Deprecated");
			}
			else
			{
				sprintf(statStr,"Invalid");
			}

			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(statStr));
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "PreferredLifetime")) {
			time_t cur_time;
			time_t life_time;
			time_t show_time;
			struct tm *nPtr=NULL;
			char timeStr[24];

			paramValue = json_object_object_get(ipIntfAddrObj, "X_ZYXEL_PreferredLifetime");
			//printf("X_ZYXEL_PreferredLifetime=%d\n",json_object_get_int(paramValue));
			life_time=json_object_get_int(paramValue);
			if(life_time==0)
			{
				memset(timeStr, 0, sizeof(timeStr));
				sprintf(timeStr,"0001-01-01T00:00:00Z");
			}
			else
			{
				memset(timeStr, 0, sizeof(timeStr));
				paramValue = json_object_object_get(ipIntfAddrObj, "X_ZYXEL_CurrentTime");
				if(paramValue != NULL)
				{
					//printf("X_ZYXEL_CurrentTime=%d\n",json_object_get_int(paramValue));
					cur_time=json_object_get_int(paramValue);
					show_time=cur_time+life_time;
					nPtr = localtime(&show_time);
					sprintf(timeStr,"%u-%02u%02uT%02u:%02u:%02uZ",nPtr->tm_year + 1900,
																nPtr->tm_mon + 1,
																nPtr->tm_mday,
																nPtr->tm_hour,
																nPtr->tm_min,
																nPtr->tm_sec);
				}
				else
				{
					memset(timeStr, 0, sizeof(timeStr));
					sprintf(timeStr,"0001-01-01T00:00:00Z");
				}
			}
			//printf("timeStr=%s\n",timeStr);
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(timeStr));
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "ValidLifetime")) {
			time_t cur_time;
			time_t life_time;
			time_t show_time;
			struct tm *nPtr=NULL;
			char timeStr[24];

			paramValue = json_object_object_get(ipIntfAddrObj, "X_ZYXEL_ValidLifetime");
			//printf("X_ZYXEL_ValidLifetime=%d\n",json_object_get_int(paramValue));
			life_time=json_object_get_int(paramValue);
			if(life_time==0)
			{
				memset(timeStr, 0, sizeof(timeStr));
				sprintf(timeStr,"0001-01-01T00:00:00Z");
			}
			else
			{
				memset(timeStr, 0, sizeof(timeStr));
				paramValue = json_object_object_get(ipIntfAddrObj, "X_ZYXEL_CurrentTime");
				if(paramValue != NULL)
				{
					//printf("X_ZYXEL_CurrentTime=%d\n",json_object_get_int(paramValue));
					cur_time=json_object_get_int(paramValue);
					show_time=cur_time+life_time;
					nPtr = localtime(&show_time);
					sprintf(timeStr,"%u-%02u%02uT%02u:%02u:%02uZ",nPtr->tm_year + 1900,
																nPtr->tm_mon + 1,
																nPtr->tm_mday,
																nPtr->tm_hour,
																nPtr->tm_min,
																nPtr->tm_sec);
				}
				else
				{
					memset(timeStr, 0, sizeof(timeStr));
					sprintf(timeStr,"0001-01-01T00:00:00Z");
				}
			}
			//printf("timeStr=%s\n",timeStr);
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(timeStr));
			paramList++;
			continue;
		}

		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(ipIntfAddrObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(ipIntfAddrObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6IpIntfIpv6AddrObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipIntfAddrObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);
	
	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu.IPv6Address.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &objIid, &ipIntfAddrObj)) != ZCFG_SUCCESS){
		printf("%s : return ret=%d\n", __FUNCTION__,ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = ipIntfAddrObj;
		ipIntfAddrObj = NULL;
		ipIntfAddrObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V6_ADDR, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ipIntfAddrObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ipIntfAddrObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_ADDR, &objIid, ipIntfAddrObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ipIntfAddrObj);
			return ret;
		}
		json_object_put(ipIntfAddrObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6IpIntfIpv6AddrObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char AdrObjFormate[128] = "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu.IPv6Address";
	objIndex_t objIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	if(sscanf(tr98FullPathName, AdrObjFormate, &objIid.idx[0]) != 1) 
		return ZCFG_INVALID_OBJECT;	

	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V6_ADDR, &objIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu.IPv6Address. Fail.\n", __FUNCTION__, objIid.idx[0]);
		return ret;
	}

	*idx = objIid.idx[1];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6IpIntfIpv6AddrObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu.IPv6Address.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	return zcfgFeObjStructDel(RDM_OID_IP_IFACE_V6_ADDR, &objIid, NULL);
}

zcfgRet_t zyIpv6IpIntfIpv6PrefixObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipIntfPrefixObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu.IPv6Prefix.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_PREFIX, &objIid, &ipIntfPrefixObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get rt adv intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "PreferredLifetime")) {
			time_t cur_time;
			time_t life_time;
			time_t show_time;
			struct tm *nPtr=NULL;
			char timeStr[24];

			paramValue = json_object_object_get(ipIntfPrefixObj, "X_ZYXEL_PreferredLifetime");
			//printf("X_ZYXEL_PreferredLifetime=%d\n",json_object_get_int(paramValue));
			life_time=json_object_get_int(paramValue);
			if(life_time==0)
			{
				memset(timeStr, 0, sizeof(timeStr));
				sprintf(timeStr,"0001-01-01T00:00:00Z");
			}
			else
			{
				memset(timeStr, 0, sizeof(timeStr));
				paramValue = json_object_object_get(ipIntfPrefixObj, "X_ZYXEL_CurrentTime");
				if(paramValue != NULL)
				{
					//printf("X_ZYXEL_CurrentTime=%d\n",json_object_get_int(paramValue));
					cur_time=json_object_get_int(paramValue);
					show_time=cur_time+life_time;
					nPtr = localtime(&show_time);
					sprintf(timeStr,"%u-%02u%02uT%02u:%02u:%02uZ",nPtr->tm_year + 1900,
																nPtr->tm_mon + 1,
																nPtr->tm_mday,
																nPtr->tm_hour,
																nPtr->tm_min,
																nPtr->tm_sec);
				}
				else
				{
					memset(timeStr, 0, sizeof(timeStr));
					sprintf(timeStr,"0001-01-01T00:00:00Z");
				}
			}
			//printf("timeStr=%s\n",timeStr);
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(timeStr));
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "ValidLifetime")) {
			time_t cur_time;
			time_t life_time;
			time_t show_time;
			struct tm *nPtr=NULL;
			char timeStr[24];

			paramValue = json_object_object_get(ipIntfPrefixObj, "X_ZYXEL_ValidLifetime");
			//printf("X_ZYXEL_ValidLifetime=%d\n",json_object_get_int(paramValue));
			life_time=json_object_get_int(paramValue);
			if(life_time==0)
			{
				memset(timeStr, 0, sizeof(timeStr));
				sprintf(timeStr,"0001-01-01T00:00:00Z");
			}
			else
			{
				memset(timeStr, 0, sizeof(timeStr));
				paramValue = json_object_object_get(ipIntfPrefixObj, "X_ZYXEL_CurrentTime");
				if(paramValue != NULL)
				{
					//printf("X_ZYXEL_CurrentTime=%d\n",json_object_get_int(paramValue));
					cur_time=json_object_get_int(paramValue);
					show_time=cur_time+life_time;
					nPtr = localtime(&show_time);
					sprintf(timeStr,"%u-%02u%02uT%02u:%02u:%02uZ",nPtr->tm_year + 1900,
																nPtr->tm_mon + 1,
																nPtr->tm_mday,
																nPtr->tm_hour,
																nPtr->tm_min,
																nPtr->tm_sec);
				}
				else
				{
					memset(timeStr, 0, sizeof(timeStr));
					sprintf(timeStr,"0001-01-01T00:00:00Z");
				}
			}
			//printf("timeStr=%s\n",timeStr);
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(timeStr));
			paramList++;
			continue;
		}
		
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(ipIntfPrefixObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(ipIntfPrefixObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6IpIntfIpv6PrefixObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipIntfPrefixObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);
	
	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu.IPv6Prefix.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);
	

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_PREFIX, &objIid, &ipIntfPrefixObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = ipIntfPrefixObj;
		ipIntfPrefixObj = NULL;
		ipIntfPrefixObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V6_PREFIX, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(ipIntfPrefixObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(ipIntfPrefixObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_PREFIX, &objIid, ipIntfPrefixObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(ipIntfPrefixObj);
			return ret;
		}
		json_object_put(ipIntfPrefixObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6IpIntfIpv6PrefixObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char PrefixObjFormate[128] = "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu.IPv6Prefix";
	objIndex_t objIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	if(sscanf(tr98FullPathName, PrefixObjFormate, &objIid.idx[0]) != 1) 
		return ZCFG_INVALID_OBJECT;	

	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V6_PREFIX, &objIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu.IPv6Prefix. Fail.\n", __FUNCTION__, objIid.idx[0]);
		return ret;
	}

	*idx = objIid.idx[1];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6IpIntfIpv6PrefixObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.IP.Interface.%hhu.IPv6Prefix.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	return zcfgFeObjStructDel(RDM_OID_IP_IFACE_V6_PREFIX, &objIid, NULL);
}

zcfgRet_t zyIpv6RouterAdverObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *rtAdvObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_RT_ADV, &objIid, &rtAdvObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/ 
		if(!strcmp(paramList->name, "Enable")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "InterfaceSettingNumberOfEntries")) {
			uint32_t  RANum = 0;
			objIndex_t ifsetIid;
			struct json_object *ifsetJobj = NULL;
			IID_INIT(ifsetIid);
			while((ret = feObjJsonGetNext(RDM_OID_RT_ADV_INTF_SET, &ifsetIid, &ifsetJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				RANum++;
				json_object_put(ifsetJobj);
			}
			json_object_object_add(*tr98Jobj, "InterfaceSettingNumberOfEntries", json_object_new_int(RANum));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(rtAdvObj);
	return ZCFG_SUCCESS;	
}

zcfgRet_t zyIpv6RouterAdverObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *rtAdvObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_RT_ADV, &objIid, &rtAdvObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = rtAdvObj;
		rtAdvObj = NULL;
		rtAdvObj = zcfgFeJsonMultiObjAppend(RDM_OID_RT_ADV, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(rtAdvObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(rtAdvObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_RT_ADV, &objIid, rtAdvObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(rtAdvObj);
			return ret;
		}
		json_object_put(rtAdvObj);
	}

	return ZCFG_SUCCESS;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_RouterAdvertisement.
 *
 *   Related object in TR181:
 *   Device.RouterAdvertisement.
 *   Device.RouterAdvertisement.InterfaceSetting.i.
 */
zcfgRet_t zyIpv6RouterAdverIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profIid;
	struct json_object *getObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	IID_INIT(profIid);
	profIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.RouterAdvertisement.InterfaceSetting.%hhu", 
		&profIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_RT_ADV_INTF_SET, &profIid, &getObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get rt adv intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(!strcmp(paramList->name, "OptionNumberOfEntries")) {
			uint32_t  optNum = 0;
#ifdef CONFIG_ZCFG_BE_MODULE_RADV_INTFSET_OPT
			objIndex_t ifsetIid;
			struct json_object *ifsetJobj = NULL;
			IID_INIT(ifsetIid);
			while((ret = feObjJsonSubNext(RDM_OID_RT_ADV_INTF_SET_OPT, &profIid, &ifsetIid, &ifsetJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				if(ifsetIid.idx[0]==profIid.idx[0])
					optNum++;
				json_object_put(ifsetJobj);
			}
#endif
			json_object_object_add(*tr98Jobj, "OptionNumberOfEntries", json_object_new_int(optNum));
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "Status")) {
			bool enable = false;
			paramValue = json_object_object_get(getObj, "Enable");
			if(paramValue != NULL)
			{
				enable = json_object_get_boolean(paramValue);
			}
			if(enable)
				json_object_object_add(*tr98Jobj, "Status", json_object_new_string("Enabled"));
			else
				json_object_object_add(*tr98Jobj, "Status", json_object_new_string("Disable"));
			paramList++;
			continue;
		}
		
		paramValue = json_object_object_get(getObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

	json_object_put(getObj);
	return ZCFG_SUCCESS;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_RouterAdvertisement.
 *
 *   Related object in TR181:
 *   Device.RouterAdvertisement.
 *   Device.RouterAdvertisement.InterfaceSetting.i.
 */
zcfgRet_t zyIpv6RouterAdverIntfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ifBuf[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ifsetIid;
	struct json_object *ifsetJobj = NULL, *ipIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	IID_INIT(ifsetIid);
	ifsetIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.RouterAdvertisement.InterfaceSetting.%hhu", 
		&ifsetIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_RT_ADV_INTF_SET, &ifsetIid, &ifsetJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get rt adv intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(ifsetJobj == NULL)
		return ret;


	if(multiJobj){
		if(ifsetJobj){
			tmpObj = ifsetJobj;
			ifsetJobj = NULL;
			ifsetJobj = zcfgFeJsonMultiObjAppend(RDM_OID_RT_ADV_INTF_SET, &ifsetIid, multiJobj, tmpObj);
		}
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ifsetJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ifsetJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;	
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_RT_ADV_INTF_SET, &ifsetIid, ifsetJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.RouterAdvertisement.InterfaceSetting.%d Fail\n", __FUNCTION__, ifsetIid.idx[0]);
			json_object_put(ifsetJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.RouterAdvertisement.InterfaceSetting.%d Fail\n", __FUNCTION__, ifsetIid.idx[0]);
		}
		json_object_put(ifsetJobj);
	}

	return ret;
}
#ifdef CONFIG_ZCFG_BE_MODULE_RADV_INTFSET_OPT
zcfgRet_t zyIpv6RouterAdverOptObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profIid, objIid;
	struct json_object *getObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	IID_INIT(profIid);
	profIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.RouterAdvertisement.InterfaceSetting.%hhu.Option.%hhu", 
		&profIid.idx[0], &profIid.idx[1]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_RT_ADV_INTF_SET_OPT, &profIid, &getObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get rt adv intf opt obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(getObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

	json_object_put(getObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6RouterAdverOptObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ifBuf[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t setIid;
	struct json_object *ifsetJobj = NULL, *ipIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	char value[257];

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	printf("%s : Enter\n", __FUNCTION__);
	IID_INIT(setIid);
	setIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.RouterAdvertisement.InterfaceSetting.%hhu.Option.%hhu", 
		&setIid.idx[0], &setIid.idx[1]);

	//printf("setIid.idx[0]=%d, setIid.idx[1]=%d\n",setIid.idx[0],setIid.idx[1]);
	if((ret = zcfgFeObjJsonGet(RDM_OID_RT_ADV_INTF_SET_OPT, &setIid, &ifsetJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get rt adv intf opt obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(ifsetJobj == NULL)
		return ret;

	if(multiJobj){
		if(ifsetJobj){
			tmpObj = ifsetJobj;
			ifsetJobj = NULL;
			ifsetJobj = zcfgFeJsonMultiObjAppend(RDM_OID_RT_ADV_INTF_SET_OPT, &setIid, multiJobj, tmpObj);
		}
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "Value")) {
				int len=0, value_len=0;
				memset(value, 0, sizeof(value));
				strcpy(value, json_object_get_string(paramValue));
				value_len = strlen(value);
				len = ((value_len/2)+2)%8;
				//printf("value_len=%d, len=%d\n",value_len,len);
				if(len!=0 && value_len!=0)
				{
					//printf("error value length\n");
					json_object_put(ifsetJobj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}

			tr181ParamValue = json_object_object_get(ifsetJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ifsetJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;	
	}

	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_RT_ADV_INTF_SET_OPT, &setIid, ifsetJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.RouterAdvertisement.InterfaceSetting.%d.option.%d Fail\n", __FUNCTION__, setIid.idx[0], setIid.idx[1]);
			json_object_put(ifsetJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.RouterAdvertisement.InterfaceSetting.%d.option.%d Fail\n", __FUNCTION__, setIid.idx[0], setIid.idx[1]);
		}
		json_object_put(ifsetJobj);
	}

	return ret;
}

zcfgRet_t zyIpv6RouterAdverOptObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char AdrObjFormate[128] = "InternetGatewayDevice.IPv6Specific.RouterAdvertisement.InterfaceSetting.%hhu.Option";
	objIndex_t objIid;
	objIndex_t intfIid;
	char inface[33];
	struct json_object *getObj = NULL;
	struct json_object *setObj = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	printf("Enter %s\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	if(sscanf(tr98FullPathName, AdrObjFormate, &objIid.idx[0]) != 1) 
		return ZCFG_INVALID_OBJECT;	

	if((ret = zcfgFeObjStructAdd(RDM_OID_RT_ADV_INTF_SET_OPT, &objIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add InternetGatewayDevice.IPv6Specific.RouterAdvertisement.InterfaceSetting.%hhu.Option. Fail.\n", __FUNCTION__, objIid.idx[0]);
		return ret;
	}

	*idx = objIid.idx[1];

	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6RouterAdverOptObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.RouterAdvertisement.InterfaceSetting.%hhu.Option.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	//printf("objIid.idx[0]=%d, objIid.idx[1]=%d\n",objIid.idx[0],objIid.idx[1]);

	return zcfgFeObjStructDel(RDM_OID_RT_ADV_INTF_SET_OPT, &objIid, NULL);
}
#endif //CONFIG_ZCFG_BE_MODULE_RADV_INTFSET_OPT

zcfgRet_t zyIpv6HostsHostObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *hostObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	/*get hosts obj for update host*/
	rdm_Hosts_t *hostsObj = NULL;
	objIndex_t hostsIid;
	IID_INIT(hostsIid);
	if(zcfgFeObjStructGet(RDM_OID_HOSTS, &hostsIid, (void **)&hostsObj) == ZCFG_SUCCESS) {
		zcfgFeObjStructFree(hostsObj);
	}

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.Hosts.Host.%hhu", 
		&objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_HOSTS_HOST, &objIid, &hostObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get host obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "IPv6AddressNumberOfEntries")) {
			uint32_t  optNum = 0;
			objIndex_t ifsetIid;
			struct json_object *ifsetJobj = NULL;
			IID_INIT(ifsetIid);
			while((ret = feObjJsonSubNext(RDM_OID_HOSTS_HOST_V6_ADDR, &objIid, &ifsetIid, &ifsetJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				if(ifsetIid.idx[0]==objIid.idx[0])
					optNum++;
				json_object_put(ifsetJobj);
			}
			json_object_object_add(*tr98Jobj, "IPv6AddressNumberOfEntries", json_object_new_int(optNum));
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "IPAddress")) {
			paramValue = json_object_object_get(hostObj, "IPAddress6");
			if(paramValue != NULL){
				json_object_object_add(*tr98Jobj, "IPAddress", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(!strcmp(paramList->name, "AddressSource")) {
			paramValue = json_object_object_get(hostObj, "X_ZYXEL_Address6Source");
			if(paramValue != NULL){
				json_object_object_add(*tr98Jobj, "AddressSource", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(!strcmp(paramList->name, "DHCPClient")) {
			paramValue = json_object_object_get(hostObj, "X_ZYXEL_DHCP6Client");
			if(paramValue != NULL){
				json_object_object_add(*tr98Jobj, "DHCPClient", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		
		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(hostObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(hostObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6HostsHostObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	char tr98Notify[256] = {0};
	tr98Parameter_t *paramList = NULL;
	char *attrParamNamePtr = NULL;
	
	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, tr181ParamName)){
			paramList++;
			continue;
		}
		attrParamNamePtr = tr181ParamName;
		break;
	}

	if(attrParamNamePtr && strlen(attrParamNamePtr) > 0) {
		sprintf(tr98Notify, "%s.%s", tr98ObjName, attrParamNamePtr);
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int zyIpv6HostsHostObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char mapObj[128] = {0};
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		attrValue = zcfgFeParamAttrGetByName(RDM_OID_HOSTS_HOST, paramList->name);
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t zyIpv6HostsHostObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	uint32_t oid = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;

		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		break;
	}

	if(attrValue >= 0) {
		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(oid, multiAttrJobj, paramList->name, attrValue);

		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, oid, paramList->name);
		}
	}
	else {
		ret = ZCFG_INVALID_ARGUMENTS;
	}

	return ret;
}

zcfgRet_t zyIpv6HostsHostIpv6AddrObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *hostIpv6AdrObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.Hosts.Host.%hhu.IPv6Address.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_HOSTS_HOST_V6_ADDR, &objIid, &hostIpv6AdrObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get host ipv6 adr obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(hostIpv6AdrObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(hostIpv6AdrObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6DnsSvrObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dnsSvrObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DNS.Client.Server.%hhu", 
		&objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS_CLIENT_SRV, &objIid, &dnsSvrObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get dns server obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(dnsSvrObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dnsSvrObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6DnsSvrObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dnsSvrObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DNS.Client.Server.%hhu", 
		&objIid.idx[0]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS_CLIENT_SRV, &objIid, &dnsSvrObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get ip intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = dnsSvrObj;
		dnsSvrObj = NULL;
		dnsSvrObj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_CLIENT_SRV, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(dnsSvrObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(dnsSvrObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_CLIENT_SRV, &objIid, dnsSvrObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(dnsSvrObj);
			return ret;
		}
		json_object_put(dnsSvrObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6DnsSvrObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_DNS_CLIENT_SRV, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyIpv6DnsSvrObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DNS.Client.Server.%hhu", 
		&objIid.idx[0]);

	//return zcfgFeObjStructDel(RDM_OID_DNS_RELAY_FWD, &objIid, NULL);
	return zcfgFeObjStructDel(RDM_OID_DNS_CLIENT_SRV, &objIid, NULL);
}

#ifdef CONFIG_ZCFG_BE_MODULE_DNS_RELAY
zcfgRet_t zyIpv6DnsRlyObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dnsRlyObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_DNS_RELAY, &objIid, &dnsRlyObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/			
		paramValue = json_object_object_get(dnsRlyObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dnsRlyObj);
	return ZCFG_SUCCESS;	
}

zcfgRet_t zyIpv6DnsRlyObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dnsRlyObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS_RELAY, &objIid, &dnsRlyObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = dnsRlyObj;
		dnsRlyObj = NULL;
		dnsRlyObj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_RELAY, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/ 
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(dnsRlyObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(dnsRlyObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_RELAY, &objIid, dnsRlyObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(dnsRlyObj);
			return ret;
		}
		json_object_put(dnsRlyObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6DnsRlyFwdObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dnsRlyObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DNS.Relay.Forwarding.%hhu", 
		&objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS_RELAY_FWD, &objIid, &dnsRlyObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get ns relay fwd obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(dnsRlyObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dnsRlyObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6DnsRlyFwdObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dnsRlyObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DNS.Relay.Forwarding.%hhu", 
		&objIid.idx[0]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS_RELAY_FWD, &objIid, &dnsRlyObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get ip intf obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = dnsRlyObj;
		dnsRlyObj = NULL;
		dnsRlyObj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_RELAY_FWD, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(dnsRlyObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(dnsRlyObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_RELAY_FWD, &objIid, dnsRlyObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(dnsRlyObj);
			return ret;
		}
		json_object_put(dnsRlyObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6DnsRlyFwdObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	//printf("zcfgFeObjStructAdd RDM_OID_DNS_RELAY_FWD\n");
	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_DNS_RELAY_FWD, &objIid, NULL);
	*idx = objIid.idx[0];
	//printf("objIid->idx[0]=%d\n",objIid.idx[0]);

	return ret;
}

zcfgRet_t zyIpv6DnsRlyFwdObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DNS.Relay.Forwarding.%hhu", 
		&objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_DNS_RELAY_FWD, &objIid, NULL);
}
#endif // CONFIG_ZCFG_BE_MODULE_DNS_RELAY

zcfgRet_t zyIpv6Dhcpv6ObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_DHCPV6, &objIid, &dhcpObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "ClientNumberOfEntries")) {
			uint32_t  optNum = 0;
			objIndex_t ifsetIid;
			struct json_object *ifsetJobj = NULL;
			IID_INIT(ifsetIid);
			while((ret = feObjJsonGetNext(RDM_OID_DHCPV6_CLIENT, &ifsetIid, &ifsetJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				optNum++;
				json_object_put(ifsetJobj);
			}
			json_object_object_add(*tr98Jobj, "ClientNumberOfEntries", json_object_new_int(optNum));
			paramList++;
			continue;
		}

		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;	
}

zcfgRet_t zyIpv6Dhcpv6ClientObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Client.%hhu", 
		&objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_CLIENT, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get dhcpv6 client obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		if(!strcmp(paramList->name, "ServerNumberOfEntries")) {
			uint32_t  optNum = 0;
			objIndex_t ifsetIid;
			struct json_object *ifsetJobj = NULL;
			IID_INIT(ifsetIid);
			while((ret = feObjJsonSubNext(RDM_OID_DHCPV6_CLIENT_SRV, &objIid, &ifsetIid, &ifsetJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				if(ifsetIid.idx[0]==objIid.idx[0])
					optNum++;
				json_object_put(ifsetJobj);
			}
			json_object_object_add(*tr98Jobj, "ServerNumberOfEntries", json_object_new_int(optNum));
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "SentOptionNumberOfEntries")) {
			uint32_t  optNum = 0;
			objIndex_t ifsetIid;
			struct json_object *ifsetJobj = NULL;
			IID_INIT(ifsetIid);
			while((ret = feObjJsonSubNext(RDM_OID_DHCPV6_CLIENT_SENT_OPT, &objIid, &ifsetIid, &ifsetJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				if(ifsetIid.idx[0]==objIid.idx[0])
					optNum++;
				json_object_put(ifsetJobj);
			}
			json_object_object_add(*tr98Jobj, "SentOptionNumberOfEntries", json_object_new_int(optNum));
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "ReceivedOptionNumberOfEntries")) {
			uint32_t  optNum = 0;
			objIndex_t ifsetIid;
			struct json_object *ifsetJobj = NULL;
			IID_INIT(ifsetIid);
			while((ret = feObjJsonSubNext(RDM_OID_DHCPV6_CLIENT_RECEIVED_OPT, &objIid, &ifsetIid, &ifsetJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				if(ifsetIid.idx[0]==objIid.idx[0])
					optNum++;
				json_object_put(ifsetJobj);
			}
			json_object_object_add(*tr98Jobj, "ReceivedOptionNumberOfEntries", json_object_new_int(optNum));
			paramList++;
			continue;
		}
		
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6ClientObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Client.%hhu", 
		&objIid.idx[0]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_CLIENT, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get dhcpv6 client obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = dhcpObj;
		dhcpObj = NULL;
		dhcpObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV6_CLIENT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(dhcpObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(dhcpObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV6_CLIENT, &objIid, dhcpObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(dhcpObj);
			return ret;
		}
		json_object_put(dhcpObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6CntSrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Client.%hhu.Server.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_CLIENT_SRV, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpv6 client srv obj codec object ret=%d\n", __FUNCTION__, ret);
		printf("%s : return ret\n", __FUNCTION__);
		return ret;
	}
	printf("%s : aaa\n", __FUNCTION__);
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6CntSentObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Client.%hhu.SentOption.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_CLIENT_SENT_OPT, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpv6 client sent obj codec object ret=%d\n", __FUNCTION__, ret);
		printf("%s : return ret\n", __FUNCTION__);
		return ret;
	}
	printf("%s : aaa\n", __FUNCTION__);
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6CntSentObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Client.%hhu.SentOption.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_CLIENT_SENT_OPT, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get dhcpv6 client obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = dhcpObj;
		dhcpObj = NULL;
		dhcpObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV6_CLIENT_SENT_OPT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(dhcpObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(dhcpObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV6_CLIENT_SENT_OPT, &objIid, dhcpObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(dhcpObj);
			return ret;
		}
		json_object_put(dhcpObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6CntSentObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char AdrObjFormate[128] = "InternetGatewayDevice.IPv6Specific.DHCPv6.Client.%hhu.SentOption";
	objIndex_t objIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	if(sscanf(tr98FullPathName, AdrObjFormate, &objIid.idx[0]) != 1) 
		return ZCFG_INVALID_OBJECT;	

	if((ret = zcfgFeObjStructAdd(RDM_OID_DHCPV6_CLIENT_SENT_OPT, &objIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add InternetGatewayDevice.IPv6Specific.DHCPv6.Client.%hhu.SentOption. Fail.\n", __FUNCTION__, objIid.idx[0]);
		return ret;
	}

	*idx = objIid.idx[1];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6CntSentObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Client.%hhu.SentOption.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	return zcfgFeObjStructDel(RDM_OID_DHCPV6_CLIENT_SENT_OPT, &objIid, NULL);
}

zcfgRet_t zyIpv6Dhcpv6CntRrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Client.%hhu.ReceivedOption.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_CLIENT_RECEIVED_OPT, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpv6 client rcv obj codec object ret=%d\n", __FUNCTION__, ret);
		printf("%s : return ret\n", __FUNCTION__);
		return ret;
	}
	printf("%s : aaa\n", __FUNCTION__);
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_DHCPV6_SRV, &objIid, &dhcpObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		if(!strcmp(paramList->name, "PoolNumberOfEntries")) {
			uint32_t  optNum = 0;
			objIndex_t ifsetIid;
			struct json_object *ifsetJobj = NULL;
			IID_INIT(ifsetIid);
			while((ret = feObjJsonGetNext(RDM_OID_DHCPV6_SRV_POOL, &ifsetIid, &ifsetJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				optNum++;
				json_object_put(ifsetJobj);
			}
			json_object_object_add(*tr98Jobj, "PoolNumberOfEntries", json_object_new_int(optNum));
			paramList++;
			continue;
		}

		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;	
}

zcfgRet_t zyIpv6Dhcpv6SrvObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = dhcpObj;
		dhcpObj = NULL;
		dhcpObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV6_SRV, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(dhcpObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(dhcpObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP, &objIid, dhcpObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(dhcpObj);
			return ret;
		}
		json_object_put(dhcpObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvPoolObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu", 
		&objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV_POOL, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get dhcpv6 server pool obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/	
		if(!strcmp(paramList->name, "ClientNumberOfEntries")) {
			uint32_t  optNum = 0;
			objIndex_t dhcp6SrvCltIid;
			struct json_object *dhcp6SrvCltJobj = NULL;
			IID_INIT(dhcp6SrvCltIid);
			while((ret = feObjJsonSubNext(RDM_OID_DHCPV6_SRV_POOL_CLIENT, &objIid, &dhcp6SrvCltIid, &dhcp6SrvCltJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				optNum++;
				json_object_put(dhcp6SrvCltJobj);
			}
			json_object_object_add(*tr98Jobj, "ClientNumberOfEntries", json_object_new_int(optNum));
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "OptionNumberOfEntries")) {
			uint32_t  optNum = 0;
			objIndex_t dhcp6SrvOptIid;
			struct json_object *dhcp6SrvOptJobj = NULL;
			IID_INIT(dhcp6SrvOptIid);
			while((ret = feObjJsonSubNext(RDM_OID_DHCPV6_SRV_POOL_OPT, &objIid, &dhcp6SrvOptIid, &dhcp6SrvOptJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				optNum++;
				json_object_put(dhcp6SrvOptJobj);
			}
			json_object_object_add(*tr98Jobj, "OptionNumberOfEntries", json_object_new_int(optNum));
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvPoolObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *orderValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu", 
		&objIid.idx[0]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV_POOL, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get dhcpv6 client obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = dhcpObj;
		dhcpObj = NULL;
		dhcpObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV6_SRV_POOL, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		if(!strcmp(paramList->name, "Order")){
			/*check duplicate order*/
			objIndex_t srvPoolIid;
			struct json_object *srvPoolJobj = NULL;
			IID_INIT(srvPoolIid);
			int order_cur=0, order_rdm=0; 
			while((ret = feObjJsonGetNext(RDM_OID_DHCPV6_SRV_POOL, &srvPoolIid, &srvPoolJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) 
			{
				if(srvPoolIid.idx[0]!=objIid.idx[0])
				{
					paramValue = json_object_object_get(tr98Jobj, paramList->name);
					orderValue = json_object_object_get(srvPoolJobj, "Order");
					if(orderValue!=NULL && paramValue!=NULL)
					{
						order_cur = json_object_get_int(paramValue);
						order_rdm = json_object_get_int(orderValue);
						//printf("order_cur=%d, order_rdm=%d\n",order_cur, order_rdm);
						if((order_cur==order_rdm)&&(order_cur!=0)&&(order_rdm!=0))
						{
							printf("%s : duplicate order!\n", __FUNCTION__);
							json_object_put(srvPoolJobj);
							return ZCFG_INVALID_PARAM_VALUE;
						}
					}
				}
				json_object_put(srvPoolJobj);
			}
		}

		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(dhcpObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(dhcpObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV6_SRV_POOL, &objIid, dhcpObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(dhcpObj);
			return ret;
		}
		json_object_put(dhcpObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvPoolObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	//printf("zcfgFeObjStructAdd RDM_OID_DNS_RELAY_FWD\n");
	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_DHCPV6_SRV_POOL, &objIid, NULL);
	*idx = objIid.idx[0];
	//printf("objIid->idx[0]=%d\n",objIid.idx[0]);

	return ret;
}

zcfgRet_t zyIpv6Dhcpv6SrvPoolObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu", 
		&objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_DHCPV6_SRV_POOL, &objIid, NULL);
}

zcfgRet_t zyIpv6Dhcpv6SrvCntObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu.Client.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV_POOL_CLIENT, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpv6 srv cnt obj codec object ret=%d\n", __FUNCTION__, ret);
		printf("%s : return ret\n", __FUNCTION__);
		return ret;
	}
	printf("%s : aaa\n", __FUNCTION__);
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvCntObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu.Client.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV_POOL_CLIENT, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get dhcpv6 srv cnt obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = dhcpObj;
		dhcpObj = NULL;
		dhcpObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV6_SRV_POOL_CLIENT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(dhcpObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(dhcpObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV6_SRV_POOL_CLIENT, &objIid, dhcpObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(dhcpObj);
			return ret;
		}
		json_object_put(dhcpObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvCntV6AddrObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu.Client.%hhu.IPv6Address.%hhu", 
		&objIid.idx[0], &objIid.idx[1], &objIid.idx[2]);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV_POOL_CLIENT_V6_ADDR, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpv6 srv cnt v6addr obj codec object ret=%d\n", __FUNCTION__, ret);
		printf("%s : return ret\n", __FUNCTION__);
		return ret;
	}
	printf("%s : aaa\n", __FUNCTION__);
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvCntV6PrefixObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu.Client.%hhu.IPv6Prefix.%hhu", 
		&objIid.idx[0], &objIid.idx[1], &objIid.idx[2]);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV_POOL_CLIENT_V6_PREFIX, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpv6 srv cnt v6 prefix obj codec object ret=%d\n", __FUNCTION__, ret);
		printf("%s : return ret\n", __FUNCTION__);
		return ret;
	}
	printf("%s : aaa\n", __FUNCTION__);
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvCntOptObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu.Client.%hhu.Option.%hhu", 
		&objIid.idx[0], &objIid.idx[1], &objIid.idx[2]);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV_POOL_CLIENT_OPT, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpv6 srv cnt opt obj codec object ret=%d\n", __FUNCTION__, ret);
		printf("%s : return ret\n", __FUNCTION__);
		return ret;
	}
	printf("%s : aaa\n", __FUNCTION__);
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvOptObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	printf("%s enter\n",__FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu.Option.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV_POOL_OPT, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpv6 srv opt obj codec object ret=%d\n", __FUNCTION__, ret);
		printf("%s : return ret\n", __FUNCTION__);
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dhcpObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}
	
	json_object_put(dhcpObj);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvOptObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dhcpObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu.Option.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	/*Get related tr181 objects*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV6_SRV_POOL_OPT, &objIid, &dhcpObj)) != ZCFG_SUCCESS){
		printf("%s: fail to get dhcpv6 srv opt obj codec object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if(multiJobj){
		tmpObj = dhcpObj;
		dhcpObj = NULL;
		dhcpObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV6_SRV_POOL_OPT, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			//check tag&value
			int tagNum = 0, value_len = 0, i = 0;
			size_t valueSize = 65536;
			bool tagfound = false;
			char optValue[257];

			int tagArr[5] = {1, 2, 3, 14};
			memset(optValue, 0, sizeof(optValue));
			if(!strcmp(paramList->name, "Tag")){
				tagNum = json_object_get_int(paramValue);
				for(i = 0; i < 4; i++){
					if(tagNum == tagArr[i]) {
						tagfound = true;
						break;
					}
				}
			}

			if(!strcmp(paramList->name, "Tag") && !tagfound){
				printf("find parameter %s in TR181 fail, %s(%d)\n", paramList->name, __FUNCTION__, __LINE__);			
				return ZCFG_INVALID_PARAM_VALUE;
			}


			if(!strcmp(paramList->name, "Value")){
				strcpy(optValue, json_object_get_string(paramValue));
				value_len = strlen(optValue);
				if(value_len > valueSize)
					return ZCFG_INVALID_PARAM_VALUE;
			}
			
			tr181ParamValue = json_object_object_get(dhcpObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(dhcpObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV6_SRV_POOL_OPT, &objIid, dhcpObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(dhcpObj);
			return ret;
		}
		json_object_put(dhcpObj);
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvOptObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char AdrObjFormate[128] = "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu.Option";
	objIndex_t objIid;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	if(sscanf(tr98FullPathName, AdrObjFormate, &objIid.idx[0]) != 1) 
		return ZCFG_INVALID_OBJECT;	

	if((ret = zcfgFeObjStructAdd(RDM_OID_DHCPV6_SRV_POOL_OPT, &objIid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG,"%s : Add InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu.Option. Fail.\n", __FUNCTION__, objIid.idx[0]);
		return ret;
	}

	*idx = objIid.idx[1];
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyIpv6Dhcpv6SrvOptObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.%hhu.Option.%hhu", 
		&objIid.idx[0], &objIid.idx[1]);

	return zcfgFeObjStructDel(RDM_OID_DHCPV6_SRV_POOL_OPT, &objIid, NULL);
}

