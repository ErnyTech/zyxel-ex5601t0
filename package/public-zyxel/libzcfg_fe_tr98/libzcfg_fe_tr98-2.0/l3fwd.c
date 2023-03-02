
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcmd_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_msg.h"
#include "zcfg_debug.h"
#include "zcfg_fe_tr98.h"
#include "zcfg_net.h"
#include "l3fwd_parameter.h"
#include "zos_api.h"

extern tr98Object_t tr98Obj[];

zcfgRet_t l3fwdGetDfGwByRtTable(char *dfGateway, int ipver){
        FILE *fpRead;
        char ifname[32] = {0};
        char* v4cmd = (char*) "route -n | grep '0.0.0.0' | awk 'BEGIN{metric = -1} {if($1 == \"0.0.0.0\" && $5 > metric){metric = $5; ifname = $8}} END {print ifname}'";
        char* v6cmd = (char*) "route -n -A inet6 | grep ::/0 | awk 'BEGIN{metric = -1} {if($4 > metric){metric = $4; ifname = $7}} END {print ifname}'";
        char* renewCh;

	printf("%s: enter, %d\n", __FUNCTION__, ipver);

	if(ipver == 4){
	        fpRead = popen(v4cmd, "r");
	}else{
		fpRead = popen(v6cmd, "r");
	}

	if(fpRead != NULL){
		fgets(ifname, sizeof(ifname), fpRead);
		pclose(fpRead);
	}else{
		printf("%s: Execute route cmd fail!!\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

        renewCh = strstr(ifname,"\n");
        if(renewCh) *renewCh= '\0';

        printf("%s: ifname = %s \n", __FUNCTION__, ifname);

        if(strlen(ifname) < 1){
                printf("%s: No find default gateway interface!!\n", __FUNCTION__);
                return ZCFG_INTERNAL_ERROR;
        }else{
		strcpy(dfGateway, ifname );
	}

        return ZCFG_SUCCESS;
}

zcfgRet_t ifNameToIpIfaceName(const char *ifName, char *ipIfaceName)
{
	objIndex_t objIid;
	struct json_object *ifaceObj = NULL;

	//ifName, ipIfaceName
	if(strlen(ifName) == 0)
		return ZCFG_NO_SUCH_OBJECT;

	int objNum = strstr(ifName, "ppp") ? RDM_OID_PPP_IFACE : RDM_OID_IP_IFACE;


	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(objNum, &objIid, &ifaceObj) == ZCFG_SUCCESS){
		const char *X_ZYXEL_IfName = json_object_get_string(json_object_object_get(ifaceObj, "X_ZYXEL_IfName"));
		if(!strcmp(X_ZYXEL_IfName, ifName)){
			json_object_put(ifaceObj);
			if(objNum == RDM_OID_PPP_IFACE){
				char pppIfaceName[30] = {0};
				sprintf(pppIfaceName, "PPP.Interface.%hhu", objIid.idx[0]);
				zcfgFeTr181IfaceStackHigherLayerGet(pppIfaceName, ipIfaceName);
				if(!strstr(ipIfaceName, "IP.Interface"))
					ipIfaceName[0] = '\0';
			}else{
				sprintf(ipIfaceName, "IP.Interface.%hhu", objIid.idx[0]);
			}

			if(strlen(ipIfaceName))
				return ZCFG_SUCCESS;
			else
				return ZCFG_NO_SUCH_OBJECT;
		}
		json_object_put(ifaceObj);
	}

	return ZCFG_NO_SUCH_OBJECT;
}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
int GetIPv4ForwardingNumber(){
	objIndex_t iid;
	int Count = 0;
	rdm_RoutingRouterV4Fwd_t *obj = NULL;
	IID_INIT(iid);
	//printf("%s : Enter\n", __FUNCTION__);
			
	while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, &iid, (void **)&obj) == ZCFG_SUCCESS) {
		if(obj->StaticRoute== true ){
			Count++;
		}
		zcfgFeObjStructFree(obj);
	}
	
	return Count;
}
#endif


/*
 *   TR98 Object Name : InternetGatewayDevice.Layer3Forwarding
 *
 *   Related object in TR181:
 *   Device.Routing.Router.1
 */
zcfgRet_t l3fwdObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	objIndex_t objIid;
	rdm_RoutingRouter_t *routerObj = NULL;
	rdm_IpIface_t *ipIfaceObj = NULL;
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *intf = NULL, *tmp_ptr = NULL;
	char X_ZYXEL_ActiveDefaultGateway[321] = {0};
	char DefaultConnectionService[513] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	/*mapping InternetGatewayDevice.Layer3Forwarding to Device.Routing.Router.1*/
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 1;
	if(feObjStructGet(RDM_OID_ROUTING_ROUTER, &objIid, (void **)&routerObj, TR98_GET_WITHOUT_UPDATE) != ZCFG_SUCCESS ) {
		printf("%s : Get Routing.Router.1 Fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
#if 0
	strcpy(X_ZYXEL_ActiveDefaultGateway, routerObj->X_ZYXEL_ActiveDefaultGateway);

	intf = strtok_r(X_ZYXEL_ActiveDefaultGateway, ",", &tmp_ptr);
	while(intf != NULL){
		if(*intf != '\0' && strstr(intf, "IP.Interface.")){
			IID_INIT(objIid);
			objIid.level = 1;
			sscanf(intf, "IP.Interface.%hhu", &objIid.idx[0]);
			if(feObjStructGet(RDM_OID_IP_IFACE, &objIid, (void **)&ipIfaceObj, TR98_GET_WITHOUT_UPDATE) == ZCFG_SUCCESS ) {
				memset(tr98ObjName, 0, sizeof(tr98ObjName));
				if(strstr(ipIfaceObj->LowerLayers, "PPP.Interface") != NULL)
					zcfgFe181To98MappingNameGet(ipIfaceObj->LowerLayers, tr98ObjName);
				else
					zcfgFe181To98MappingNameGet(intf, tr98ObjName);
				if(tr98ObjName[0] != '\0'){
					if(DefaultConnectionService[0] != '\0')
						strcat(DefaultConnectionService, ",");
					strcat(DefaultConnectionService, tr98ObjName);
				}
				zcfgFeObjStructFree(ipIfaceObj);
			}
		}
		intf = strtok_r(NULL, ",", &tmp_ptr);
	}
#endif
	*tr98Jobj = json_object_new_object();
	//json_object_object_add(*tr98Jobj, "DefaultConnectionService", json_object_new_string(DefaultConnectionService));
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	int IPv4ForwardingNumber = 0;
	IPv4ForwardingNumber = GetIPv4ForwardingNumber();
	json_object_object_add(*tr98Jobj, "ForwardNumberOfEntries", json_object_new_int(IPv4ForwardingNumber));
#else
	json_object_object_add(*tr98Jobj, "ForwardNumberOfEntries", json_object_new_int(routerObj->IPv4ForwardingNumberOfEntries));
#endif
	json_object_object_add(*tr98Jobj, "X_ZYXEL_IPV6ForwardNumberOfEntries", json_object_new_int(routerObj->IPv6ForwardingNumberOfEntries));
	json_object_object_add(*tr98Jobj, "X_ZYXEL_AutoSecureDefaultIface", json_object_new_boolean(routerObj->X_ZYXEL_AutoSecureDefaultIface));
	
	X_ZYXEL_ActiveDefaultGateway[0] = '\0';
	l3fwdGetDfGwByRtTable(X_ZYXEL_ActiveDefaultGateway, 4);
	char ipIfaceName[300] = {0};
	if(ifNameToIpIfaceName((const char *)X_ZYXEL_ActiveDefaultGateway, ipIfaceName) == ZCFG_SUCCESS){
		char *tr98WanIpIface = translateWanIpIface(TRANS_WANIPIFACE_181TO98, ipIfaceName);
		if(tr98WanIpIface){
			json_object_object_add(*tr98Jobj, "DefaultConnectionService", json_object_new_string(tr98WanIpIface));
			free(tr98WanIpIface);
		}
	}
	else{
		json_object_object_add(*tr98Jobj, "DefaultConnectionService", json_object_new_string(""));
	}

	json_object_object_add(*tr98Jobj, "X_ZYXEL_DefaultGwInterface", json_object_new_string(X_ZYXEL_ActiveDefaultGateway));

	zcfgFeObjStructFree(routerObj);
	
	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.Layer3Forwarding
 *
 *   Related object in TR181:
 *   Device.Routing.Router.1
 */
zcfgRet_t l3fwdObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *routerObj = NULL, *paramValue = NULL;
	char DefaultConnectionService[513] = {0};
	char X_ZYXEL_ActiveDefaultGateway[321] = {0};
	char tr181ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ipIfaceName[32] = {0};
	char *intf = NULL, *tmp_ptr = NULL;

	printf("%s\n", __FUNCTION__);

	/*mapping InternetGatewayDevice.Layer3Forwarding to Device.Routing.Router.1*/
	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 1;
	if(zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER, &objIid, &routerObj) != ZCFG_SUCCESS ) {
		printf("%s : Get Routing.Router.1 Fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	const char *tr181_X_ZYXEL_ActiveDefaultGateway = json_object_get_string(json_object_object_get(routerObj, "X_ZYXEL_ActiveDefaultGateway"));

	if(paramValue = json_object_object_get(tr98Jobj, "X_ZYXEL_AutoSecureDefaultIface"))
		json_object_object_add(routerObj, "X_ZYXEL_AutoSecureDefaultIface", JSON_OBJ_COPY(paramValue));

	if(paramValue = json_object_object_get(tr98Jobj, "DefaultConnectionService")){
		const char *tr98DefaultConnectionService = json_object_get_string(paramValue);
		if(strlen(tr98DefaultConnectionService) && (!strstr(tr98DefaultConnectionService, "WANDevice") || !strstr(tr98DefaultConnectionService, "WANConnectionDevice"))){
			ret = ZCFG_INVALID_PARAM_VALUE;
		}else if(strlen(tr98DefaultConnectionService)){
			strcpy(DefaultConnectionService, json_object_get_string(paramValue));
			printf("DefaultConnectionService=%s\n", DefaultConnectionService);
			char *tr181WanIpIface = translateWanIpIface(TRANS_WANIPIFACE_98TO181, DefaultConnectionService);
			if(tr181WanIpIface) {
				if(strlen(tr181_X_ZYXEL_ActiveDefaultGateway) == 0)
					strcpy(X_ZYXEL_ActiveDefaultGateway, tr181WanIpIface);
				else if(!strstr(tr181_X_ZYXEL_ActiveDefaultGateway, tr181WanIpIface))
					sprintf(X_ZYXEL_ActiveDefaultGateway, "%s,%s", tr181_X_ZYXEL_ActiveDefaultGateway, tr181WanIpIface);
				else
					strcpy(X_ZYXEL_ActiveDefaultGateway, tr181_X_ZYXEL_ActiveDefaultGateway);

				free(tr181WanIpIface);
			}else{
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
		}
		
#if 0
		intf = strtok_r(DefaultConnectionService, ",", &tmp_ptr);
		while(intf != NULL){
			if(*intf != '\0' && strstr(intf, "InternetGatewayDevice.WANDevice.")){
				zcfgFe98To181MappingNameGet(intf, tr181ObjName);
				ipIfaceName[0] = '\0';
				if(strstr(tr181ObjName, "PPP.Interface") != NULL){
					zcfgFeTr181IfaceStackHigherLayerGet(tr181ObjName, ipIfaceName);
					printf("ipIfaceName=%s\n", ipIfaceName);
				}
				else if(strstr(tr181ObjName, "IP.Interface") != NULL){
					strcpy(ipIfaceName, tr181ObjName);
				}

				if(ipIfaceName[0] != '\0'){
					if(X_ZYXEL_ActiveDefaultGateway[0] != '\0')
						strcat(X_ZYXEL_ActiveDefaultGateway, ",");
					strcat(X_ZYXEL_ActiveDefaultGateway, ipIfaceName);
				}
			}
			intf = strtok_r(NULL, ",", &tmp_ptr);
		}
#endif

		if(ret == ZCFG_SUCCESS){
			printf("X_ZYXEL_ActiveDefaultGateway=%s\n", X_ZYXEL_ActiveDefaultGateway);
			json_object_object_add(routerObj, "X_ZYXEL_ActiveDefaultGateway", json_object_new_string(X_ZYXEL_ActiveDefaultGateway));
		}else{
			json_object_put(routerObj);
			return ret;
		}
	}

	if(multiJobj) {
		zcfgFeJsonMultiObjAppend(RDM_OID_ROUTING_ROUTER, &objIid, multiJobj, routerObj);
	}
	else {
		if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER, &objIid, routerObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(routerObj);
			return ret;
		}
	}
	json_object_put(routerObj);

	return ZCFG_SUCCESS;
}

#define MAX_IFACE_LENGTH (256+1)
/*
 *   TR98 Object Name : InternetGatewayDevice.Layer3Forwarding.Forwarding.i
 *
 *   Related object in TR181:
 *   Device.Routing.Router.1.IPv4Forwarding.i
 */
zcfgRet_t l3fwdFwdTbObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid, ipifaceIid;
	struct json_object *ipv4FwdObj = NULL, *ipIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char iface[MAX_IFACE_LENGTH] = {0};
	char lowerLayer[MAX_IFACE_LENGTH] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tr181ObjName[MAX_TR181_PATHNAME] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, tr181ObjName)) != ZCFG_SUCCESS) {
		return ret;
	}

	/*mapping InternetGatewayDevice.Layer3Forwarding to Device.Routing.Router.1*/
	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr181ObjName, "Routing.Router.%hhu.IPv4Forwarding.%hhu.", &objIid.idx[0], &objIid.idx[1]);

	if((ret = feObjJsonGet(RDM_OID_ROUTING_ROUTER_V4_FWD, &objIid, &ipv4FwdObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
#ifdef ZYXEL_OPAL_EXTENDER
        if(paramList->attr & PARAMETER_ATTR_EXTENDER_HIDDEN)
        {
            paramList++;
            continue;
        }
#endif
		/*special case*/
		if(!strcmp(paramList->name, "Interface")) {
			paramValue = json_object_object_get(ipv4FwdObj, paramList->name);
			ZOS_STRNCPY(iface, json_object_get_string(paramValue), sizeof(iface));

			IID_INIT(ipifaceIid);
			ipifaceIid.level = 1;
			sscanf(iface, "IP.Interface.%hhu", &ipifaceIid.idx[0]);
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &ipifaceIid, &ipIfaceJobj)) != ZCFG_SUCCESS) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface Fail\n", __FUNCTION__);
				printf("%s : Get IP.Interface Fail\n", __FUNCTION__);
			}
			else {
				ZOS_STRNCPY(lowerLayer, json_object_get_string(json_object_object_get(ipIfaceJobj, "LowerLayers")), sizeof(lowerLayer));

				if(strstr(lowerLayer, "PPP.I") != NULL) {
					strcpy(iface, lowerLayer);
				}

				json_object_put(ipIfaceJobj);
			}

			zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName);
			strcpy(tr98ObjName, tmpTr98ObjName);
			ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName); //move active wan index to WANDEV.1 , no matter what wan type it is.
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));

			paramList++;
			continue;
		}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		if(!strcmp(paramList->name, "StaticRoute") 
			&& !strcmp("routeTableAddobject",json_object_get_string(json_object_object_get(ipv4FwdObj, "X_ZYXEL_RoutAdder")))) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			paramList++;
			continue;
		}
		if(!strcmp(paramList->name, "Status") 
			&& !strcmp("routeTableAddobject",json_object_get_string(json_object_object_get(ipv4FwdObj, "X_ZYXEL_RoutAdder")))) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("enabled"));
			paramList++;
			continue;
		}
#endif

		paramValue = json_object_object_get(ipv4FwdObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		if(!strcmp(paramList->name, "SourceIPAddress")) {
			if((paramValue = json_object_object_get(ipv4FwdObj, "X_ZYXEL_SourceIpAddress")))
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));

			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "SourceSubnetMask")) {
			if((paramValue = json_object_object_get(ipv4FwdObj, "X_ZYXEL_SourceSubnetMask")))
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));

			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "Type")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "MTU")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(1540));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(ipv4FwdObj);

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.Layer3Forwarding.Forwarding.i
 *
 *   Related object in TR181:
 *   Device.Routing.Router.1.IPv4Forwarding.i
 */
zcfgRet_t l3fwdFwdTbObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipv4FwdObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char iface[MAX_IFACE_LENGTH] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tmpTr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tr181ObjName[MAX_TR181_PATHNAME] = {0};
	char higherLayer[MAX_IFACE_LENGTH] = {0};
	bool wrongParamValue = false;
	char srcIpAddr[MAX_IP4STR_SIZE] = {0}, srcNetMask[MAX_IP4STR_SIZE] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, tr181ObjName)) != ZCFG_SUCCESS) {
		return ret;
	}

	/*mapping InternetGatewayDevice.Layer3Forwarding to Device.Routing.Router.1*/
	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr181ObjName, "Routing.Router.%hhu.IPv4Forwarding.%hhu", &objIid.idx[0], &objIid.idx[1]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER_V4_FWD, &objIid, &ipv4FwdObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(multiJobj){
		tmpObj = ipv4FwdObj;
		ipv4FwdObj = NULL;
		ipv4FwdObj = zcfgFeJsonMultiObjAppend(RDM_OID_ROUTING_ROUTER_V4_FWD, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	const char *paramstr = json_object_get_string(json_object_object_get(ipv4FwdObj, "X_ZYXEL_SourceIpAddress"));
	if(paramstr && strlen(paramstr)) ZOS_STRNCPY(srcIpAddr, paramstr, sizeof(srcIpAddr));
	paramstr = json_object_get_string(json_object_object_get(ipv4FwdObj, "X_ZYXEL_SourceSubnetMask"));
	if(paramstr && strlen(paramstr)) ZOS_STRNCPY(srcNetMask, paramstr, sizeof(srcNetMask));


	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(!paramValue) {
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "Interface")) {
			ZOS_STRNCPY(tmpTr98ConnObjName, json_object_get_string(paramValue), sizeof(tr98ConnObjName));
			strcpy(tr98ConnObjName, tmpTr98ConnObjName);
			ReplaceWANDeviceIndex(tmpTr98ConnObjName, tr98ConnObjName); //recover the active wan from WANDEV.1 back to it original WANDEV.
			if(strlen(tr98ConnObjName) > 0) {
				if(!strstr(tr98ConnObjName, "IPConnection") && !strstr(tr98ConnObjName, "PPPConnection") && !strstr(tr98ConnObjName, "IPInterface")) {
					wrongParamValue = true;
					break;
				}

				if(zcfgFe98To181MappingNameGet(tr98ConnObjName, iface) != ZCFG_SUCCESS) {
					wrongParamValue = true;
					break;
				}
				else {
					if(strstr(iface, "PPP.Interface") != NULL) {
						zcfgFeTr181IfaceStackHigherLayerGet(iface, higherLayer);
						ZOS_STRNCPY(iface, higherLayer, sizeof(iface));
					}
					
					json_object_object_add(ipv4FwdObj, paramList->name, json_object_new_string(iface));
				}
			}
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "GatewayIPAddress") || !strcmp(paramList->name, "DestIPAddress") ||
			!strcmp(paramList->name, "SourceIPAddress")) {
			const char *ipAddress = (const char *)json_object_get_string(paramValue);
			if(ipAddress && (strlen(ipAddress) > 0) && (inet_addr((const char *)ipAddress) < 0)) {
				wrongParamValue = true;
				break;
			}
		}

		if(!strcmp(paramList->name, "SourceIPAddress") || !strcmp(paramList->name, "X_ZYXEL_SourceIpAddress")){
			paramstr = json_object_get_string(paramValue);
			if(paramstr && strlen(paramstr) && strcmp(paramstr, srcIpAddr))
				json_object_object_add(ipv4FwdObj, "X_ZYXEL_SourceIpAddress", JSON_OBJ_COPY(paramValue));
		}
		else if(!strcmp(paramList->name, "SourceSubnetMask") || !strcmp(paramList->name, "X_ZYXEL_SourceSubnetMask")){
			paramstr = json_object_get_string(paramValue);
			if(paramstr && strlen(paramstr) && strcmp(paramstr, srcNetMask))
				json_object_object_add(ipv4FwdObj, "X_ZYXEL_SourceSubnetMask", JSON_OBJ_COPY(paramValue));
		}
		else if(json_object_object_get(ipv4FwdObj, paramList->name)){
				json_object_object_add(ipv4FwdObj, paramList->name, JSON_OBJ_COPY(paramValue));
		}else
			printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(wrongParamValue == true) {
		if(!multiJobj)
			json_object_put(ipv4FwdObj);

		return ZCFG_INVALID_ARGUMENTS;
	}

	/*Set Device.Routing.Router.1.IPv4Forwarding.i*/
	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V4_FWD, &objIid, ipv4FwdObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(ipv4FwdObj);
			return ret;
		}
		json_object_put(ipv4FwdObj);
	}

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.Layer3Forwarding.Forwarding.i
 *
 *   Related object in TR181:
 *   Device.Routing.Router.1.IPv4Forwarding.i
 */
zcfgRet_t l3fwdFwdTbObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 1;

	if((ret = zcfgFeObjStructAdd(RDM_OID_ROUTING_ROUTER_V4_FWD, &objIid, NULL)) != ZCFG_SUCCESS) {
		printf("%s : Add Instance Fail!!\n", __FUNCTION__);
		return ret;
	}

	struct json_object *rtObj;
	if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER_V4_FWD, &objIid, &rtObj)) != ZCFG_SUCCESS){

	}

	json_object_object_add(rtObj, "X_ZYXEL_RoutAdder", json_object_new_string("acsaddobject"));
	zcfgFeObjJsonSetWithoutApply(RDM_OID_ROUTING_ROUTER_V4_FWD, &objIid, rtObj, NULL);
	json_object_put(rtObj);

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	*idx = objIid.idx[1];
#else
	{
		objIndex_t tr98ObjIid;

		IID_INIT(tr98ObjIid);
		if(zcfg98To181ObjMappingAdd(e_TR98_FWD, &tr98ObjIid, RDM_OID_ROUTING_ROUTER_V4_FWD, &objIid) == ZCFG_SUCCESS) {
			printf("%s: Added Layer3Forwarding.Forwarding %hhu\n", __FUNCTION__, tr98ObjIid.idx[0]);
			*idx = tr98ObjIid.idx[0];
		}
	}
#endif

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.Layer3Forwarding.Forwarding.i
 *
 *   Related object in TR181:
 *   Device.Routing.Router.1.IPv4Forwarding.i
 */
zcfgRet_t l3fwdFwdTbObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	uint8_t idx = 0;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);
	
	sscanf(tr98FullPathName, "InternetGatewayDevice.Layer3Forwarding.Forwarding.%hhu", &idx);

	IID_INIT(objIid);
	objIid.level = 2;
	objIid.idx[0] = 1;	
	objIid.idx[1] = idx;

	ret = zcfgFeObjJsonDel(RDM_OID_ROUTING_ROUTER_V4_FWD, &objIid, NULL);
	if(ret != ZCFG_SUCCESS) {
		printf("%s : Delete Object Fail\n", __FUNCTION__);	
	}
	
	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.Layer3Forwarding.Forwarding.i
 *
 *   Related object in TR181:
 *   Device.Routing.Router.1.IPv4Forwarding.i
 */
zcfgRet_t l3fwdFwdTbObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgFeSetParmAttr_t *setParmAttr = NULL;
	uint32_t seqnum = 0, oid = 0, attr = 0;
	objIndex_t tr98ObjIid, tr181ObjIid;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(!strcmp(paramName, "GatewayIPAddress")) {
		IID_INIT(tr98ObjIid);
		if(zcfgFe98NameToSeqnum((const char *)tr98FullPathName, &seqnum, &tr98ObjIid) != ZCFG_SUCCESS)
			return ZCFG_NO_SUCH_PARAMETER;

		attr = PARAMETER_ATTR_SET_TR98(newNotify);
		setParmAttr = zcfgFeTr98ParamAttrSetByName(seqnum, &tr98ObjIid, paramName, 0, NULL, NULL, attr);
		json_object_array_add(multiAttrJobj, json_object_new_int((int)setParmAttr));
		IID_INIT(tr181ObjIid);
		if(zcfgFe98To181ObjMappingGet(seqnum, &tr98ObjIid, &oid, &tr181ObjIid) == ZCFG_SUCCESS) {
			attr = PARAMETER_ATTR_SET_TR181(newNotify);
			setParmAttr = zcfgFeTr98ParamAttrSetByName(RDM_OID_ROUTING_ROUTER_V4_FWD, &tr181ObjIid, paramName, seqnum, &tr98ObjIid, paramName, attr);
			json_object_array_add(multiAttrJobj, json_object_new_int((int)setParmAttr));
		}
	}

	return ZCFG_SUCCESS;
}

#if defined(L3IPV6FWD) && !defined(CONFIG_TAAAB_CUSTOMIZATION_TR069)
zcfgRet_t l3fwdIpv6FwdTbObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid, ipifaceIid;
	struct json_object *ipv6FwdObj = NULL, *ipIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char iface[MAX_IFACE_LENGTH] = {0};
	char lowerLayer[MAX_IFACE_LENGTH] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tr181ObjName[MAX_TR181_PATHNAME] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, tr181ObjName)) != ZCFG_SUCCESS) {
		return ret;
	}

	/*mapping InternetGatewayDevice.Layer3Forwarding to Device.Routing.Router.1*/
	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr181ObjName, "Routing.Router.%hhu.IPv6Forwarding.%hhu.", &objIid.idx[0], &objIid.idx[1]);
	//printf("tr181ObjName=%s\n",tr181ObjName);
	if((ret = feObjJsonGet(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, &ipv6FwdObj, updateFlag)) != ZCFG_SUCCESS)
	{
		return ret;
	}
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*special case*/
		//printf("paramList->name=%s\n",paramList->name);
		if(!strcmp(paramList->name, "Interface")) {
			paramValue = json_object_object_get(ipv6FwdObj, paramList->name);
			strncpy(iface, json_object_get_string(paramValue), sizeof(iface));

			IID_INIT(ipifaceIid);
			ipifaceIid.level = 1;
			sscanf(iface, "IP.Interface.%hhu", &ipifaceIid.idx[0]);
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &ipifaceIid, &ipIfaceJobj)) != ZCFG_SUCCESS) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface Fail\n", __FUNCTION__);
				printf("%s : Get IP.Interface Fail\n", __FUNCTION__);
			}
			else {
				strncpy(lowerLayer, json_object_get_string(json_object_object_get(ipIfaceJobj, "LowerLayers")), sizeof(lowerLayer));

				if(strstr(lowerLayer, "PPP.I") != NULL) {
					strcpy(iface, lowerLayer);
				}

				json_object_put(ipIfaceJobj);
			}

			zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName);
			strcpy(tr98ObjName, tmpTr98ObjName);
			ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName); //move active wan index to WANDEV.1 , no matter what wan type it is.
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));

			paramList++;
			continue;
		}

		paramValue = json_object_object_get(ipv6FwdObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(ipv6FwdObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t l3fwdIpv6FwdTbObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipv6FwdObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char iface[MAX_IFACE_LENGTH] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tmpTr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tr181ObjName[MAX_TR181_PATHNAME] = {0};
	char higherLayer[MAX_IFACE_LENGTH] = {0};
	bool wrongParamValue = false;
	char srcIpAddr[MAX_IP4STR_SIZE] = {0}, srcNetMask[MAX_IP4STR_SIZE] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, tr181ObjName)) != ZCFG_SUCCESS) {
		return ret;
	}

	/*mapping InternetGatewayDevice.Layer3Forwarding to Device.Routing.Router.1*/
	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(tr181ObjName, "Routing.Router.%hhu.IPv6Forwarding.%hhu", &objIid.idx[0], &objIid.idx[1]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, &ipv6FwdObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(multiJobj){
		tmpObj = ipv6FwdObj;
		ipv6FwdObj = NULL;
		ipv6FwdObj = zcfgFeJsonMultiObjAppend(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(!paramValue) {
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "Interface")) {
			strncpy(tmpTr98ConnObjName, json_object_get_string(paramValue), sizeof(tr98ConnObjName));
			strcpy(tr98ConnObjName, tmpTr98ConnObjName);
			ReplaceWANDeviceIndex(tmpTr98ConnObjName, tr98ConnObjName); //recover the active wan from WANDEV.1 back to it original WANDEV.
			if(strlen(tr98ConnObjName) > 0) {
				if(!strstr(tr98ConnObjName, "IPConnection") && !strstr(tr98ConnObjName, "PPPConnection") && !strstr(tr98ConnObjName, "IPInterface")) {
					wrongParamValue = true;
					break;
				}

				if(zcfgFe98To181MappingNameGet(tr98ConnObjName, iface) != ZCFG_SUCCESS) {
					wrongParamValue = true;
					break;
				}
				else {
					if(strstr(iface, "PPP.Interface") != NULL) {
						zcfgFeTr181IfaceStackHigherLayerGet(iface, higherLayer);
						strncpy(iface, higherLayer, sizeof(iface)-1);
					}
					
					json_object_object_add(ipv6FwdObj, paramList->name, json_object_new_string(iface));
				}
			}
			paramList++;
			continue;
		}
#if 0
		if(!strcmp(paramList->name, "GatewayIPAddress") || !strcmp(paramList->name, "DestIPAddress") ||
			!strcmp(paramList->name, "SourceIPAddress")) {
			const char *ipAddress = (const char *)json_object_get_string(paramValue);
			if(ipAddress && (strlen(ipAddress) > 0) && (inet_addr((const char *)ipAddress) < 0)) {
				wrongParamValue = true;
				break;
			}
		}
#endif
		if(json_object_object_get(ipv6FwdObj, paramList->name)){
				json_object_object_add(ipv6FwdObj, paramList->name, JSON_OBJ_COPY(paramValue));
		}else
			printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(wrongParamValue == true) {
		if(!multiJobj)
			json_object_put(ipv6FwdObj);

		return ZCFG_INVALID_ARGUMENTS;
	}

	/*Set Device.Routing.Router.1.IPv6Forwarding.i*/
	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, ipv6FwdObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(ipv6FwdObj);
			return ret;
		}
		json_object_put(ipv6FwdObj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.Layer3Forwarding.Forwarding.i
 *
 *   Related object in TR181:
 *   Device.Routing.Router.1.X_ZYXEL_Ipv6Forwarding.i
 */
zcfgRet_t l3fwdIpv6FwdTbObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	objIid.idx[0] = 1;

	if((ret = zcfgFeObjStructAdd(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, NULL)) != ZCFG_SUCCESS) {
		printf("%s : Add Instance Fail!!\n", __FUNCTION__);
		return ret;
	}

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	*idx = objIid.idx[1];
#else
	{
		objIndex_t tr98ObjIid;

		IID_INIT(tr98ObjIid);
		if(zcfg98To181ObjMappingAdd(e_TR98_IPV6FWD, &tr98ObjIid, RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid) == ZCFG_SUCCESS) {
			printf("%s: Added Layer3Forwarding.X_ZYXEL_Ipv6Forwarding %hhu\n", __FUNCTION__, tr98ObjIid.idx[0]);
			*idx = tr98ObjIid.idx[0];
		}
	}
#endif

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.Layer3Forwarding.Forwarding.i
 *
 *   Related object in TR181:
 *   Device.Routing.Router.1.X_ZYXEL_Ipv6Forwarding.i
 */
zcfgRet_t l3fwdIpv6FwdTbObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	uint8_t idx = 0;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);
	
	sscanf(tr98FullPathName, "InternetGatewayDevice.Layer3Forwarding.X_ZYXEL_Ipv6Forwarding.%hhu", &idx);

	IID_INIT(objIid);
	objIid.level = 2;
	objIid.idx[0] = 1;	
	objIid.idx[1] = idx;

	ret = zcfgFeObjJsonDel(RDM_OID_ROUTING_ROUTER_V6_FWD, &objIid, NULL);
	if(ret != ZCFG_SUCCESS) {
		printf("%s : Delete Object Fail\n", __FUNCTION__);	
	}
	
	return ret;
}
#endif
