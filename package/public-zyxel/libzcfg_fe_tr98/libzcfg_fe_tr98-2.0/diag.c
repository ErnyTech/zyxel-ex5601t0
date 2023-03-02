
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
#include "diag_parameter.h"
#include "zcfg_net.h"

extern tr98Object_t tr98Obj[];

extern zcfgRet_t zcfgFeTr181IfaceStackHigherLayerGet(char *, char *);
extern zcfgRet_t zcfgFeTr181IfaceStackLowerLayerGet(char *, char *);
/*
 *   TR98 Object Name : InternetGatewayDevice.IPPingDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.IPPing
 */
zcfgRet_t ipPingDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipPingObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char devIpIface[48] = {0};
	char iface[40] = {0};
	char lowerLayer[32] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_IPPING, &objIid, &ipPingObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*special case*/
		if(!strcmp(paramList->name, "Interface")) {
			paramValue = json_object_object_get(ipPingObj, paramList->name);
			strcpy(devIpIface, json_object_get_string(paramValue));
			if(!strcmp(devIpIface, "")) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			else {
				strcpy(iface, devIpIface);
				if((zcfgFeTr181IfaceStackLowerLayerGet(iface, lowerLayer)) != ZCFG_SUCCESS) {
					json_object_put(ipPingObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				if(strstr(lowerLayer, "PPP.Interface") != NULL) {
					memset(iface, 0, sizeof(iface));
					strcpy(iface, lowerLayer);
				}
				if((zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName)) != ZCFG_SUCCESS) {
					json_object_put(ipPingObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				strcpy(tr98ObjName, tmpTr98ObjName);
				ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName); //move active wan index to WANDEV.1 , no matter what wan type it is.
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));
			}
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(ipPingObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(ipPingObj);

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.IPPingDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.IPPing
 */
zcfgRet_t ipPingDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ipPingObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char devIpIface[300] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	//char higherLayer[32] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_IPPING, &objIid, &ipPingObj)) != ZCFG_SUCCESS)
		return ret;

	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	if(multiJobj){
		//zcfgFeSessMultiObjSetUtilize(multiJobj);

		tmpObj = ipPingObj;
		ipPingObj = NULL;
		ipPingObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_DIAG_IPPING, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_IP_DIAG_IPPING, &objIid, "partialset", json_object_new_boolean(true));
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		if(!strcmp(paramList->name, "Interface")) {
			sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			if(paramValue != NULL && json_object_get_string(paramValue)) {
				bool emptyIface = false;
				strcpy(tmpTr98ConnObjName, json_object_get_string(paramValue));
				strcpy(tr98ConnObjName, tmpTr98ConnObjName);
				ReplaceWANDeviceIndex(tmpTr98ConnObjName, tr98ConnObjName); //recover the active wan from WANDEV.1 back to it original WANDEV.
				if(zcfgFe98To181IpIface(tr98ConnObjName, devIpIface) == ZCFG_SUCCESS){
					json_object_object_add(ipPingObj, paramList->name, json_object_new_string(devIpIface));
				}else{
					if(!multiJobj)
						json_object_put(ipPingObj);

					sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
					return ZCFG_INVALID_PARAM_VALUE;
				}
#if 0
				if(!strcmp(tr98ConnObjName, ""))
					emptyIface = true;
				else if(strcmp(tr98ConnObjName, "") && (zcfgFe98To181MappingNameGet(tr98ConnObjName, iface) != ZCFG_SUCCESS)) {
					json_object_object_add(ipPingObj, paramList->name, json_object_new_string(""));
					if(!multiJobj)
					json_object_put(ipPingObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}

				if(strstr(iface, "PPP.Interface") != NULL) {
					if((zcfgFeTr181IfaceStackHigherLayerGet(iface, higherLayer)) != ZCFG_SUCCESS) {
						if(!multiJobj)
							json_object_put(ipPingObj);
						return ZCFG_INVALID_PARAM_VALUE;
					}
					memset(iface, 0, sizeof(iface));
					strcpy(iface, higherLayer);

					if(!strstr(iface, "IP.Interface")) {
						if(!multiJobj)
							json_object_put(ipPingObj);
						return ZCFG_INVALID_PARAM_VALUE;
					}
				}

				if(strstr(iface, "IP.Interface") != NULL) {
					json_object_object_add(ipPingObj, paramList->name, json_object_new_string(iface));
				}
				else if(emptyIface)
					json_object_object_add(ipPingObj, paramList->name, json_object_new_string(""));
#endif
			}
			strcpy(paramfault, "");
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "DiagnosticsState")) {
			paramValue = json_object_object_get(tr98Jobj, "DiagnosticsState");
			const char *spvParamStr = json_object_get_string(paramValue);

			if(spvParamStr && strcmp(spvParamStr, "Requested"))
				return ZCFG_INVALID_PARAM_VALUE;

			if(paramValue)
				json_object_object_add(ipPingObj, paramList->name, JSON_OBJ_COPY(paramValue));
			else
				json_object_object_del(ipPingObj, paramList->name);
#if 0
			if(paramValue != NULL) {
				if(!strcmp(json_object_get_string(paramValue), "Requested")){
					if(zcfg_msg_eidGet() == ZCFG_EID_TR69){
						json_object_object_add(ipPingObj, "X_ZYXEL_Creator", json_object_new_string(CREATOR_ACS));
					}
				}
			}
#endif
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(ipPingObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ipPingObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	/*Set Device.Routing.Router.1.IPv4Forwarding.i*/
	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		//if((ret = zcfgFeObjJsonSet(RDM_OID_IP_DIAG_IPPING, &objIid, ipPingObj, NULL)) != ZCFG_SUCCESS ) {
		const char *ipping;
		ipping = json_object_to_json_string(ipPingObj);
		if((ret = zcfgFeObjWrite(RDM_OID_IP_DIAG_IPPING, &objIid, ipping, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(ipPingObj);
			return ret;
		}
		json_object_put(ipPingObj);
	}

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.TraceRouteDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.TraceRoute
 */
zcfgRet_t traceRtDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *traceRtObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char devIpIface[48] = {0};
	char lowerLayer[32] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_TRACE_RT, &objIid, &traceRtObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*special case*/
		if(!strcmp(paramList->name, "Interface")) {
			paramValue = json_object_object_get(traceRtObj, paramList->name);
			strcpy(devIpIface, json_object_get_string(paramValue));
			if(!strcmp(devIpIface, "")) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			else {
				if((zcfgFeTr181IfaceStackLowerLayerGet(devIpIface, lowerLayer)) != ZCFG_SUCCESS) {
					json_object_put(traceRtObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				if(strstr(lowerLayer, "PPP.Interface") != NULL) {
					memset(devIpIface, 0, sizeof(devIpIface));
					strcpy(devIpIface, lowerLayer);
				}

				if((zcfgFe181To98MappingNameGet(devIpIface, tmpTr98ObjName)) != ZCFG_SUCCESS) {
					json_object_put(traceRtObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				strcpy(tr98ObjName, tmpTr98ObjName);
				ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName); //move active wan index to WANDEV.1 , no matter what wan type it is.
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));
			}
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(traceRtObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}



		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(traceRtObj);

	return ZCFG_SUCCESS;
}

#define MAX_HOP_COUNT  64

/*
 *   TR98 Object Name : InternetGatewayDevice.TraceRouteDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.TraceRoute
 */
zcfgRet_t traceRtDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *traceRtObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_TRACE_RT, &objIid, &traceRtObj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = traceRtObj;
		traceRtObj = NULL;
		traceRtObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_DIAG_TRACE_RT, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		if(!strcmp(paramList->name, "Interface")) {
			sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
			const char *tmpTr98Iface = json_object_get_string(json_object_object_get(tr98Jobj, paramList->name));
			if(tmpTr98Iface) {
				char ipIface[300] = {0};
				char tr98Iface[300] = {0};
				strcpy(tr98Iface, tmpTr98Iface);
				ReplaceWANDeviceIndex(tmpTr98Iface, tr98Iface); //recover the active wan from WANDEV.1 back to it original WANDEV.
				if(zcfgFe98To181IpIface(tr98Iface, ipIface) == ZCFG_SUCCESS) {
					if(!strcmp(ipIface, "")) {
						//if(!multiJobj)
						//	json_object_put(traceRtObj);
						//return ZCFG_NO_SUCH_OBJECT;
						json_object_object_add(traceRtObj, paramList->name, json_object_new_string(""));
					}
					else
						json_object_object_add(traceRtObj, paramList->name, json_object_new_string(ipIface));
				}
				else {
					if(!multiJobj)
						json_object_put(traceRtObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			else {
				//if(!multiJobj)
				//	json_object_put(traceRtObj);
				//return ZCFG_NO_SUCH_OBJECT;
				json_object_object_add(traceRtObj, paramList->name, json_object_new_string(""));
			}
			strcpy(paramfault, "");
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "NumberOfTries") || !strcmp(paramList->name, "Timeout") ||
				!strcmp(paramList->name, "DataBlockSize")) {
				if(json_object_get_int(paramValue) == 0) {
					if(!multiJobj)
						json_object_put(traceRtObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			else if(!strcmp(paramList->name, "MaxHopCount")) {
				unsigned int maxHopCount = json_object_get_int(paramValue);
				if(!maxHopCount || maxHopCount > MAX_HOP_COUNT) {
					if(!multiJobj)
						json_object_put(traceRtObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			tr181ParamValue = json_object_object_get(traceRtObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(traceRtObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	json_object_object_add(traceRtObj, "X_ZYXEL_Creator", json_object_new_string("ACS"));

	/*Set Device.Routing.Router.1.IPv4Forwarding.i*/
	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_DIAG_TRACE_RT, &objIid, traceRtObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(traceRtObj);
			return ret;
		}
		json_object_put(traceRtObj);
	}

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.TraceRouteDiagnostics.RouteHops.i
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.TraceRoute.RouteHops.i
 */
zcfgRet_t routeHopsObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *rtHopObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char paramName[32] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.TraceRouteDiagnostics.RouteHops.%hhu", &objIid.idx[0]);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_TRACERT_RT_HOPS, &objIid, &rtHopObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		sscanf(paramList->name, "Hop%s", paramName);

		printf("%s : paramName %s\n", __FUNCTION__, paramName);

		paramValue = json_object_object_get(rtHopObj, paramName);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(rtHopObj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.NsLookUpDiagnostics
 *
 *   Related object in TR181:
 *   Device.DNS.Diagnostics.NSLookupDiagnostics. (RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG)
 */
zcfgRet_t nsLookUpDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *nsLookUpObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG, &objIid, &nsLookUpObj, updateFlag)) != ZCFG_SUCCESS) {
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(nsLookUpObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(nsLookUpObj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.NsLookUpDiagnostics
 *
 *   Related object in TR181:
 *   Device.DNS.Diagnostics.NSLookupDiagnostics. (RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG)
 */
zcfgRet_t nsLookUpDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *nsLookUpDiagObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char devIpIface[100] = {0};
	int nParm = 0;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if ((ret = zcfgFeObjJsonGet(RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG, &objIid, &nsLookUpDiagObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	if (multiJobj) {
		tmpObj = nsLookUpDiagObj;
		nsLookUpDiagObj = NULL;
		nsLookUpDiagObj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG, &objIid, "partialset", json_object_new_boolean(true));
	}

	paramList = tr98Obj[handler].parameter;
	while (paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if (strstr(paramList->name, "Diagnost")) {
			const char *tr181paramStr = json_object_get_string(json_object_object_get(nsLookUpDiagObj, "DiagnosticsState"));
			const char *spvParamStr = json_object_get_string(paramValue);
			if (spvParamStr && (strcmp(spvParamStr, "Requested") && strcmp(spvParamStr, ""))) {
				return ZCFG_INVALID_PARAM_VALUE;
			}

			if (paramValue) {
				json_object_object_add(nsLookUpDiagObj, paramList->name, JSON_OBJ_COPY(paramValue));
			} else {
				json_object_object_del(nsLookUpDiagObj, paramList->name);
			}

			if (tr181paramStr && !strcmp(tr181paramStr, "Requested")) {
				// there's diagnostics on going
			} else if (!tr181paramStr || strcmp(tr181paramStr, "Requested")) {
				if (!paramValue) {
				}
			}
			paramList++;
			nParm++;
			continue;
		}

		if (!paramValue) {
			paramList++;
			nParm++;
			continue;
		}

		if (!strcmp(paramList->name, "Interface")) {
			strncpy(tmpTr98ConnObjName, json_object_get_string(paramValue), sizeof(tmpTr98ConnObjName));
			strncpy(tr98ConnObjName, tmpTr98ConnObjName, sizeof(tr98ConnObjName));
			ReplaceWANDeviceIndex(tmpTr98ConnObjName, tr98ConnObjName); //recover the active wan from WANDEV.1 back to it original WANDEV.
			if (zcfgFe98To181IpIface(tr98ConnObjName, devIpIface) != ZCFG_SUCCESS) {
				if (!multiJobj) {
					json_object_put(nsLookUpDiagObj);
				}

				snprintf(paramfault, NAME_SIZE, "%s.%s", tr98FullPathName, paramList->name);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			json_object_object_add(nsLookUpDiagObj, paramList->name, json_object_new_string(devIpIface));
		} else {
			json_object_object_add(nsLookUpDiagObj, paramList->name, JSON_OBJ_COPY(paramValue));
		}
		nParm++;
		paramList++;	
	}

	json_object_object_add(nsLookUpDiagObj, "X_ZYXEL_Creator", json_object_new_string(CREATOR_ACS));

	if (multiJobj) {
		//json_object_put(tmpObj);
	} else {
		if ((ret = zcfgFeObjWrite(RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG, &objIid, nsLookUpDiagObj, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL)) != ZCFG_SUCCESS) {
			json_object_put(nsLookUpDiagObj);
			return ret;
		}
		json_object_put(nsLookUpDiagObj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.NsLookUpDiagnostics
 *
 *   Related object in TR181:
 *   Device.DNS.Diagnostics.NSLookupDiagnostics.Result.i (RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG_RESULT)
 */
zcfgRet_t nsLookUpResultObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *nsLookUpResultObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char paramName[32] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.NsLookUpDiagnostics.Result.%hhu", &objIid.idx[0]);
	if ((ret = feObjJsonGet(RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG_RESULT, &objIid, &nsLookUpResultObj, updateFlag)) != ZCFG_SUCCESS) {
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while (paramList->name != NULL) {
		printf("%s : paramName %s\n", __FUNCTION__, paramList->name);

		paramValue = json_object_object_get(nsLookUpResultObj, paramList->name);
		if (paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(nsLookUpResultObj);

	return ZCFG_SUCCESS;
}


/*
 *   TR98 Object Name : InternetGatewayDevice.ServerSelectionDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.ServerSelectionDiagnostics
 */
zcfgRet_t srvSelectDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *svrSelectDiagObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_SVR_SELECT_DIAG, &objIid, &svrSelectDiagObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(svrSelectDiagObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(svrSelectDiagObj);
	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.ServerSelectionDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.ServerSelectionDiagnostics
 */
zcfgRet_t srvSelectDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *svrSelectDiagObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_SVR_SELECT_DIAG, &objIid, &svrSelectDiagObj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = svrSelectDiagObj;
		svrSelectDiagObj = NULL;
		svrSelectDiagObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_DIAG_SVR_SELECT_DIAG, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		if(!strcmp(paramList->name, "Interface")) {
			sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
			const char *tmpTr98Iface = json_object_get_string(json_object_object_get(tr98Jobj, paramList->name));
			if(tmpTr98Iface) {
				char ipIface[300] = {0};
				char tr98Iface[300] = {0};
				strcpy(tr98Iface, tmpTr98Iface);
				ReplaceWANDeviceIndex(tmpTr98Iface, tr98Iface); //recover the active wan from WANDEV.1 back to it original WANDEV.
				if(zcfgFe98To181IpIface(tr98Iface, ipIface) == ZCFG_SUCCESS) {
					if(!strcmp(ipIface, "")) {
						json_object_object_add(svrSelectDiagObj, paramList->name, json_object_new_string(""));
					}
					else
						json_object_object_add(svrSelectDiagObj, paramList->name, json_object_new_string(ipIface));
				}
				else {
					if(!multiJobj)
						json_object_put(svrSelectDiagObj);
					sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			else {
				json_object_object_add(svrSelectDiagObj, paramList->name, json_object_new_string(""));
			}
			strcpy(paramfault, "");
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "NumberOfRepetitions")) {
				unsigned int numberOfRepetitions = json_object_get_int(paramValue);
				if(numberOfRepetitions < 1 || numberOfRepetitions > 4) {
					if(!multiJobj)
						json_object_put(svrSelectDiagObj);
					sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			else if(!strcmp(paramList->name, "Timeout")) {
				if(json_object_get_int(paramValue) < 1) {
					if(!multiJobj)
						json_object_put(svrSelectDiagObj);
					sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			tr181ParamValue = json_object_object_get(svrSelectDiagObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(svrSelectDiagObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_DIAG_SVR_SELECT_DIAG, &objIid, svrSelectDiagObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(svrSelectDiagObj);
			return ret;
		}
		json_object_put(svrSelectDiagObj);
	}

	return ZCFG_SUCCESS;
}


/*
 *   TR98 Object Name : InternetGatewayDevice.Capabilities.PerformanceDiagnostic
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.DownloadDiagnostics
 *   Device.IP.Diagnostics.UploadDiagnostics
 *   Device.IP.Diagnostics.UDPEchoDiagnostics
 */
zcfgRet_t perfDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dlDiagObj = NULL;
	struct json_object *ulDiagObj = NULL;
	struct json_object *udpEchoDiagObj = NULL;
	struct json_object *ipDiagObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	//char devIpIface[48] = {0};
	//char iface[40] = {0};
	//char lowerLayer[32] = {0};
	//char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_DL_DIAG, &objIid, &dlDiagObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_UL_DIAG, &objIid, &ulDiagObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_UDP_ECHO_DIAG, &objIid, &udpEchoDiagObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG, &objIid, &ipDiagObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(!strcmp(paramList->name, "DownloadTransports") ||
			!strcmp(paramList->name, "DownloadDiagnosticMaxConnections") ||
			!strcmp(paramList->name, "DownloadDiagnosticsMaxIncrementalResult"))
		{
			paramValue = json_object_object_get(dlDiagObj, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			printf("Can't find parameter %s in TR181\n", paramList->name);
			paramList++;
		}
		else if (!strcmp(paramList->name, "UploadTransports") ||
			!strcmp(paramList->name, "UploadDiagnosticsMaxConnections") ||
			!strcmp(paramList->name, "UploadDiagnosticsMaxIncrementalResult"))
		{
			paramValue = json_object_object_get(ulDiagObj, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			printf("Can't find parameter %s in TR181\n", paramList->name);
			paramList++;
		}
		else if (!strcmp(paramList->name, "UDPEchoDiagnosticsMaxResults"))
		{
			paramValue = json_object_object_get(udpEchoDiagObj, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			printf("Can't find parameter %s in TR181\n", paramList->name);
			paramList++;
		}
		else
		{
			paramValue = json_object_object_get(ipDiagObj, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			printf("Can't find parameter %s in TR181\n", paramList->name);
			paramList++;
		}
		
	}

	json_object_put(dlDiagObj);
	json_object_put(ulDiagObj);
	json_object_put(udpEchoDiagObj);
	json_object_put(ipDiagObj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.DownloadDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.DownloadDiagnostics
 */
zcfgRet_t dlDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dlDiagObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char devIpIface[48] = {0};
	char iface[40] = {0};
	char lowerLayer[32] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_DL_DIAG, &objIid, &dlDiagObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*special case*/
		if(!strcmp(paramList->name, "Interface")) {
			paramValue = json_object_object_get(dlDiagObj, paramList->name);
			strcpy(devIpIface, json_object_get_string(paramValue));
			if(!strcmp(devIpIface, "")) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			else {
				strcpy(iface, devIpIface);
				if((zcfgFeTr181IfaceStackLowerLayerGet(iface, lowerLayer)) != ZCFG_SUCCESS) {
					json_object_put(dlDiagObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				if(strstr(lowerLayer, "PPP.Interface") != NULL) {
					memset(iface, 0, sizeof(iface));
					strcpy(iface, lowerLayer);
				}
				if((zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName)) != ZCFG_SUCCESS) {
					json_object_put(dlDiagObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				strcpy(tr98ObjName, tmpTr98ObjName);
				ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName); //move active wan index to WANDEV.1 , no matter what wan type it is.
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));
			}
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(dlDiagObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(dlDiagObj);

	return ZCFG_SUCCESS;
}
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
#ifdef ZYXEL_DIAG_IPERF_SUPPORT
#define DOWNLOADDIAG_SET_PARM_NUM 19
#else
#define DOWNLOADDIAG_SET_PARM_NUM 8
#endif
#else
#ifdef ZYXEL_DIAG_IPERF_SUPPORT
#define DOWNLOADDIAG_SET_PARM_NUM 17
#else
#define DOWNLOADDIAG_SET_PARM_NUM 6
#endif
#endif
#define DOWNLOADDIAG_SET_PARM_NUM 12
/*
 *   TR98 Object Name : InternetGatewayDevice.DownloadDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.DownloadDiagnostics
 */
zcfgRet_t dlDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *dlDiagObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	//char iface[100] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	//char higherLayer[100] = {0};
	char devIpIface[100] = {0};
	int nParm = 0;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_DL_DIAG, &objIid, &dlDiagObj)) != ZCFG_SUCCESS)
		return ret;

	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	if(multiJobj){
		//zcfgFeSessMultiObjSetUtilize(multiJobj);
		tmpObj = dlDiagObj;
		dlDiagObj = NULL;
		dlDiagObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_DIAG_DL_DIAG, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_IP_DIAG_DL_DIAG, &objIid, "partialset", json_object_new_boolean(true));
	}

	paramList = tr98Obj[handler].parameter;
	//This is specific implementation on 'DownloadDiagnostics'
	while((nParm < DOWNLOADDIAG_SET_PARM_NUM) && (paramList->name != NULL)) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(strstr(paramList->name, "Diagnost")) {
			const char *tr181paramStr = json_object_get_string(json_object_object_get(dlDiagObj, "DiagnosticsState"));
			const char *spvParamStr = json_object_get_string(paramValue);
			if(spvParamStr && (strcmp(spvParamStr, "Requested") && strcmp(spvParamStr, "")))
				return ZCFG_INVALID_PARAM_VALUE;

			if(paramValue)
				json_object_object_add(dlDiagObj, paramList->name, JSON_OBJ_COPY(paramValue));
			else
				json_object_object_del(dlDiagObj, paramList->name);

			if(tr181paramStr && !strcmp(tr181paramStr, "Requested")){
				// there's diagnostics on going
			}else if(!tr181paramStr || strcmp(tr181paramStr, "Requested")){
				if(!paramValue){

				}
			}
			paramList++;
			nParm++;
			continue;
		}

		if(!paramValue) {
			paramList++;
			nParm++;
			continue;
		}

		if(!strcmp(paramList->name, "Interface")) {
			strcpy(tmpTr98ConnObjName, json_object_get_string(paramValue));
			strcpy(tr98ConnObjName, tmpTr98ConnObjName);
			ReplaceWANDeviceIndex(tmpTr98ConnObjName, tr98ConnObjName); //recover the active wan from WANDEV.1 back to it original WANDEV.
			if(zcfgFe98To181IpIface(tr98ConnObjName, devIpIface) == ZCFG_SUCCESS){
				json_object_object_add(dlDiagObj, paramList->name, json_object_new_string(devIpIface));
			}else{
				if(!multiJobj)
					json_object_put(dlDiagObj);

				sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
				return ZCFG_INVALID_PARAM_VALUE;
			}

#if 0
			if(zcfgFe98To181MappingNameGet(tr98ConnObjName, iface) != ZCFG_SUCCESS) {
				json_object_object_add(dlDiagObj, paramList->name, json_object_new_string(""));
			}else {
				if(strstr(iface, "PPP.Interface") != NULL) {
					if((zcfgFeTr181IfaceStackHigherLayerGet(iface, higherLayer)) != ZCFG_SUCCESS) {
						json_object_put(dlDiagObj);
						return ZCFG_INVALID_PARAM_VALUE;
					}
					memset(iface, 0, sizeof(iface));
					strcpy(iface, higherLayer);
				}
		
				strcpy(devIpIface, iface);
				json_object_object_add(dlDiagObj, paramList->name, json_object_new_string(devIpIface));
			}			
			strcpy(paramfault, "");
#endif

		}else {
			json_object_object_add(dlDiagObj, paramList->name, JSON_OBJ_COPY(paramValue));
		}
		nParm++;
		paramList++;	
	}

	//json_object_object_add(dlDiagObj, "X_ZYXEL_Creator", json_object_new_string("ACS"));

	/*Set Device.Routing.Router.1.IPv4Forwarding.i*/
	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		//if((ret = zcfgFeObjJsonSet(RDM_OID_IP_DIAG_DL_DIAG, &objIid, dlDiagObj, NULL)) != ZCFG_SUCCESS ) {
		if((ret = zcfgFeObjWrite(RDM_OID_IP_DIAG_DL_DIAG, &objIid, dlDiagObj, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(dlDiagObj);
			return ret;
		}
		json_object_put(dlDiagObj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.DownloadDiagnostics.PerConnectionResult.i
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.DownloadDiagnostics.PerConnectionResult.i
 */
zcfgRet_t dlDiagPerConnRstObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *obj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.DownloadDiagnostics.PerConnectionResult.%hhu", &objIid.idx[0]);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_DL_DIAG_PER_CONN_RST, &objIid, &obj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(obj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;

	}

	json_object_put(obj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.DownloadDiagnostics.IncrementalResult.i
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.DownloadDiagnostics.IncrementalResult.i
 */
zcfgRet_t dlDiagIncremRstObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *obj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.DownloadDiagnostics.IncrementalResult.%hhu", &objIid.idx[0]);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_DL_DIAG_INCREM_RST, &objIid, &obj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(obj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;

	}

	json_object_put(obj);

	return ZCFG_SUCCESS;
}


/*
 *   TR98 Object Name : InternetGatewayDevice.UploadDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.UploadDiagnostics
 */
zcfgRet_t ulDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ulDiagObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char devIpIface[48] = {0};
	char iface[40] = {0};
	char lowerLayer[32] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_UL_DIAG, &objIid, &ulDiagObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*special case*/
		if(!strcmp(paramList->name, "Interface")) {
			paramValue = json_object_object_get(ulDiagObj, paramList->name);
			strcpy(devIpIface, json_object_get_string(paramValue));
			if(!strcmp(devIpIface, "")) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			else {
				strcpy(iface, devIpIface);
				if((zcfgFeTr181IfaceStackLowerLayerGet(iface, lowerLayer)) != ZCFG_SUCCESS) {
					json_object_put(ulDiagObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				if(strstr(lowerLayer, "PPP.Interface") != NULL) {
					memset(iface, 0, sizeof(iface));
					strcpy(iface, lowerLayer);
				}
				if((zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName)) != ZCFG_SUCCESS) {
					json_object_put(ulDiagObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				strcpy(tr98ObjName, tmpTr98ObjName);
				ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName); //move active wan index to WANDEV.1 , no matter what wan type it is.
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));
			}
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(ulDiagObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(ulDiagObj);

	return ZCFG_SUCCESS;
}
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
#ifdef ZYXEL_DIAG_IPERF_SUPPORT
#define UPLOADDIAG_SET_PARM_NUM 20
#else
#define UPLOADDIAG_SET_PARM_NUM 9
#endif
#else
#ifdef ZYXEL_DIAG_IPERF_SUPPORT
#define UPLOADDIAG_SET_PARM_NUM 18
#else
#define UPLOADDIAG_SET_PARM_NUM 7
#endif
#endif
#define UPLOADDIAG_SET_PARM_NUM 13
/*
 *   TR98 Object Name : InternetGatewayDevice.UploadDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.UploadDiagnostics
 */
zcfgRet_t ulDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *ulDiagObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char iface[100] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char higherLayer[100] = {0};
	char devIpIface[100] = {0};
	int nParm = 0;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	//if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_UL_DIAG, &objIid, &ulDiagObj)) != ZCFG_SUCCESS)
	//	return ret;
	ulDiagObj = json_object_new_object();

	if(multiJobj){
		tmpObj = ulDiagObj;
		ulDiagObj = NULL;
		ulDiagObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_DIAG_UL_DIAG, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);

		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_IP_DIAG_UL_DIAG, &objIid, "partialset", json_object_new_boolean(true));
	}

	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	const char *parmDiagStr = json_object_get_string(json_object_object_get(tr98Jobj, "DiagnosticsState"));
	if(parmDiagStr && strcmp(parmDiagStr, "") && !strstr(parmDiagStr, "Requested")) {
		if(!multiJobj)
			json_object_put(ulDiagObj);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	paramList = tr98Obj[handler].parameter;
	//This is specific implementation on 'UploadDiagnostics'
	while((nParm < UPLOADDIAG_SET_PARM_NUM) && (paramList->name != NULL)) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(!paramValue) {
			paramList++;
			nParm++;
			continue;
		}
		if(strstr(paramList->name, "Diagnost")) {
			json_object_object_add(ulDiagObj, paramList->name, JSON_OBJ_COPY(paramValue));
		}else if(!strcmp(paramList->name, "Interface")) {
			sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
			strcpy(tmpTr98ConnObjName, json_object_get_string(paramValue));
			strcpy(tr98ConnObjName, tmpTr98ConnObjName);
			ReplaceWANDeviceIndex(tmpTr98ConnObjName, tr98ConnObjName); //recover the active wan from WANDEV.1 back to it original WANDEV.
			if(zcfgFe98To181MappingNameGet(tr98ConnObjName, iface) != ZCFG_SUCCESS) {
				json_object_object_add(ulDiagObj, paramList->name, json_object_new_string(""));
			}else {
				if(strstr(iface, "PPP.Interface") != NULL) {
					if((zcfgFeTr181IfaceStackHigherLayerGet(iface, higherLayer)) != ZCFG_SUCCESS) {
						json_object_put(ulDiagObj);
						return ZCFG_INVALID_PARAM_VALUE;
					}
					memset(iface, 0, sizeof(iface));
					strcpy(iface, higherLayer);
				}

				strcpy(devIpIface, iface);
				json_object_object_add(ulDiagObj, paramList->name, json_object_new_string(devIpIface));
			}
			strcpy(paramfault, "");
		}else {
			json_object_object_add(ulDiagObj, paramList->name, JSON_OBJ_COPY(paramValue));
		}
		nParm++;
		paramList++;
	}


	/*Set Device.Routing.Router.1.IPv4Forwarding.i*/
	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_DIAG_UL_DIAG, &objIid, ulDiagObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(ulDiagObj);
			return ret;
		}
		json_object_put(ulDiagObj);
	}

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.UploadDiagnostics.PerConnectionResult.i
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.UploadDiagnostics.PerConnectionResult.i
 */
zcfgRet_t ulDiagPerConnRstObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *obj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.UploadDiagnostics.PerConnectionResult.%hhu", &objIid.idx[0]);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_UL_DIAG_PER_CONN_RST, &objIid, &obj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(obj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;

	}

	json_object_put(obj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.UploadDiagnostics.IncrementalResult.i
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.UploadDiagnostics.IncrementalResult.i
 */
zcfgRet_t ulDiagIncremRstObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *obj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.UploadDiagnostics.IncrementalResult.%hhu", &objIid.idx[0]);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_UL_DIAG_INCREM_RST, &objIid, &obj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(obj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;

	}

	json_object_put(obj);

	return ZCFG_SUCCESS;
}


/*
 *   TR98 Object Name : InternetGatewayDevice.UDPEchoConfig
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.UDPEchoConfig
 */
zcfgRet_t udpEchoCfgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *udpEchoObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char iface[100] = {0};
	char lowerLayer[100] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_UDP_ECHO_CFG, &objIid, &udpEchoObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(udpEchoObj, paramList->name);
		if(!paramValue) {
			printf("Can't find parameter %s in TR181\n", paramList->name);
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "Interface")) {
			const char *devIpIface = json_object_get_string(paramValue);
			if(!strcmp(devIpIface, "")) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			else {
				strcpy(iface, devIpIface);
				if((zcfgFeTr181IfaceStackLowerLayerGet(iface, lowerLayer)) != ZCFG_SUCCESS) {
					//zcfgLog(ZCFG_LOG_ERR, "%s : get higher layer interface failed\n", __FUNCTION__);
					json_object_put(udpEchoObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				if(strstr(lowerLayer, "PPP.Interface") != NULL) {
					memset(iface, 0, sizeof(iface));
					strcpy(iface, lowerLayer);
				}
				if((zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName)) != ZCFG_SUCCESS) {
					//zcfgLog(ZCFG_LOG_ERR, "%s : get mapping name failed\n", __FUNCTION__);
					json_object_put(udpEchoObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				strcpy(tr98ObjName, tmpTr98ObjName);
				ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName); //move active wan index to WANDEV.1 , no matter what wan type it is.
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));
			}
		}else {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		}
		paramList++;
	}

	json_object_put(udpEchoObj);

	return ZCFG_SUCCESS;
}

#define UDPECHOCFG_SET_PARM_NUM 5
/*
 *   TR98 Object Name : InternetGatewayDevice.UDPEchoConfig
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.UDPEchoConfig
 */
zcfgRet_t udpEchoCfgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	bool tr98ObjParamEnableValue = 0, tr181ObjParmEnableValue = 0;
	struct json_object *tr98ObjParamEnable = NULL;
	objIndex_t objIid;
	struct json_object *udpEchoObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	char iface[100] = {0};
	char higherLayer[100] = {0};
	int nParm = 0;
	struct json_object *tr181ObjParm = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_UDP_ECHO_CFG, &objIid, &udpEchoObj)) != ZCFG_SUCCESS)
		return ret;

	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	if(multiJobj){
		tmpObj = udpEchoObj;
		udpEchoObj = NULL;
		udpEchoObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_DIAG_UDP_ECHO_CFG, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	tr181ObjParmEnableValue = json_object_get_boolean(json_object_object_get(udpEchoObj, "Enable"));
	paramList = tr98Obj[handler].parameter;
	//This is specific implementation on 'UdpEchoConfig'
	while((nParm < UDPECHOCFG_SET_PARM_NUM) && (paramList->name != NULL)) {
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(!paramValue) {
			paramList++;
			nParm++;
			continue;
		}

		if(!strcmp(paramList->name, "Enable")) {
			tr98ObjParamEnable = paramValue;
			//if(!(tr181ObjParm = json_object_object_get(udpEchoObj, paramList->name))) {
			//}
			tr98ObjParamEnableValue = tr98ObjParamEnable ? json_object_get_boolean(tr98ObjParamEnable) : 0;
			if(tr181ObjParmEnableValue && tr98ObjParamEnableValue){
				if(!multiJobj) json_object_put(udpEchoObj);
				sprintf(paramfault, "UDPEchoConfig running, and require disable!");
				return ZCFG_INVALID_PARAM_VALUE;
			}else {
				json_object_object_add(udpEchoObj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
		}else if(!strcmp(paramList->name, "Interface")) {
			const char *tmpTr98ObjParmValue = json_object_get_string(paramValue);
			char tr98ObjParmValue[TR98_MAX_OBJ_NAME_LENGTH] = {0};
			strcpy(tr98ObjParmValue, tmpTr98ObjParmValue);
			ReplaceWANDeviceIndex(tmpTr98ObjParmValue, tr98ObjParmValue); //recover the active wan from WANDEV.1 back to it original WANDEV.
			if(!strcmp(tr98ObjParmValue, "")) {
				json_object_object_add(udpEchoObj, paramList->name, json_object_new_string(""));
			}else if(zcfgFe98To181IpIface(tr98ObjParmValue, iface) == ZCFG_SUCCESS){
				json_object_object_add(udpEchoObj, paramList->name, json_object_new_string(iface));
			}else{
				sprintf(paramfault, "Interface config not correct!");
				if(!multiJobj) json_object_put(udpEchoObj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}else if(!strcmp(paramList->name, "SourceIPAddress")) {
			char addr[sizeof(struct in_addr)] = {0};
			const char *srcIpAddress = json_object_get_string(paramValue);
			if((!srcIpAddress || !strlen(srcIpAddress)) && tr98ObjParamEnableValue)
			{
				if(!multiJobj) json_object_put(udpEchoObj);
				return ZCFG_INVALID_ARGUMENTS;
			}
			else if((inet_pton(AF_INET, srcIpAddress, (void *)addr)) != 1) {
				//zcfgLog(ZCFG_LOG_ERR, "%s : wrong IP address\n", __FUNCTION__);
				if(!multiJobj) json_object_put(udpEchoObj);
				return ZCFG_INVALID_ARGUMENTS;
			}
			json_object_object_add(udpEchoObj, paramList->name, JSON_OBJ_COPY(paramValue));
		}else {
			json_object_object_add(udpEchoObj, paramList->name, JSON_OBJ_COPY(paramValue));
		}
		paramList++;
		nParm++;
	}

	if(!tr98ObjParamEnable){
		if(tr181ObjParmEnableValue){
			if(!multiJobj) json_object_put(udpEchoObj);
			sprintf(paramfault, "UDPEchoConfig running, and require disable!");
			return ZCFG_INVALID_PARAM_VALUE;
		}
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_IP_DIAG_UDP_ECHO_CFG, &objIid, "apply", json_object_new_boolean(false));
	}else if(tr98ObjParamEnableValue){
		zcfgRet_t rst;
		char *srcIpAddressStr = json_object_get_string(json_object_object_get(udpEchoObj, "SourceIPAddress"));
		const char *iface = json_object_get_string(json_object_object_get(udpEchoObj, "Interface"));
		if(!srcIpAddressStr || strlen(srcIpAddressStr) == 0){
			if(!multiJobj) json_object_put(udpEchoObj);
			sprintf(paramfault, "Require SourceIPAddress specifying!");
			return ZCFG_INVALID_PARAM_VALUE;
		}else if(((rst = zcfgFeTr181IpIfaceRunning(iface, paramfault)) != ZCFG_SUCCESS) && rst != ZCFG_EMPTY_OBJECT){
			if(!multiJobj) json_object_put(udpEchoObj);
			sprintf(paramfault, "Interface not correct!");
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}


	if(!multiJobj) {
		//set parameter
		//zcfgFeObjWrite(RDM_OID_IP_DIAG_IPPING, &objIid, ipping, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL))
		json_object_put(udpEchoObj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_UDPEchoDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.UDPEchoDiagnostics
 */
zcfgRet_t udpEchoDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *udpEchoDiagObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char devIpIface[48] = {0};
	char iface[40] = {0};
	char lowerLayer[32] = {0};
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_UDP_ECHO_DIAG, &objIid, &udpEchoDiagObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*special case*/
		if(!strcmp(paramList->name, "Interface")) {
			paramValue = json_object_object_get(udpEchoDiagObj, paramList->name);
			strcpy(devIpIface, json_object_get_string(paramValue));
			if(!strcmp(devIpIface, "")) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			else {
				strcpy(iface, devIpIface);
				if((zcfgFeTr181IfaceStackLowerLayerGet(iface, lowerLayer)) != ZCFG_SUCCESS) {
					json_object_put(udpEchoDiagObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				if(strstr(lowerLayer, "PPP.Interface") != NULL) {
					memset(iface, 0, sizeof(iface));
					strcpy(iface, lowerLayer);
				}
				if((zcfgFe181To98MappingNameGet(iface, tmpTr98ObjName)) != ZCFG_SUCCESS) {
					json_object_put(udpEchoDiagObj);
					return ZCFG_NO_SUCH_OBJECT;
				}
				strcpy(tr98ObjName, tmpTr98ObjName);
				ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName); //move active wan index to WANDEV.1 , no matter what wan type it is.
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98ObjName));
			}
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(udpEchoDiagObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(udpEchoDiagObj);

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_UDPEchoDiagnostics
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.UDPEchoDiagnostics
 */
zcfgRet_t udpEchoDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *udpEchoDiagObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char iface[40] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tmpTr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char higherLayer[32] = {0};

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_UDP_ECHO_DIAG, &objIid, &udpEchoDiagObj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = udpEchoDiagObj;
		udpEchoDiagObj = NULL;
		udpEchoDiagObj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_DIAG_UDP_ECHO_DIAG, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		if(!strcmp(paramList->name, "Interface")) {
			sprintf(paramfault, "%s.%s", tr98FullPathName, paramList->name);
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			if(paramValue != NULL && json_object_get_string(paramValue)) {
				bool emptyIface = false;
				strcpy(tmpTr98ConnObjName, json_object_get_string(paramValue));
				strcpy(tr98ConnObjName, tmpTr98ConnObjName);
				ReplaceWANDeviceIndex(tmpTr98ConnObjName, tr98ConnObjName); //recover the active wan from WANDEV.1 back to it original WANDEV.
				if(!strcmp(tr98ConnObjName, ""))
					emptyIface = true;
				else if(strcmp(tr98ConnObjName, "") && (zcfgFe98To181MappingNameGet(tr98ConnObjName, iface) != ZCFG_SUCCESS)) {
					json_object_object_add(udpEchoDiagObj, paramList->name, json_object_new_string(""));
					if(!multiJobj)
					json_object_put(udpEchoDiagObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}

				if(strstr(iface, "PPP.Interface") != NULL) {
					if((zcfgFeTr181IfaceStackHigherLayerGet(iface, higherLayer)) != ZCFG_SUCCESS) {
						if(!multiJobj)
							json_object_put(udpEchoDiagObj);
						return ZCFG_INVALID_PARAM_VALUE;
					}
					memset(iface, 0, sizeof(iface));
					strcpy(iface, higherLayer);

					if(!strstr(iface, "IP.Interface")) {
						if(!multiJobj)
							json_object_put(udpEchoDiagObj);
						return ZCFG_INVALID_PARAM_VALUE;
					}
				}

				if(strstr(iface, "IP.Interface") != NULL) {
					json_object_object_add(udpEchoDiagObj, paramList->name, json_object_new_string(iface));
				}
				else if(emptyIface)
					json_object_object_add(udpEchoDiagObj, paramList->name, json_object_new_string(""));
			}
			strcpy(paramfault, "");
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "DiagnosticsState")) {
			paramValue = json_object_object_get(tr98Jobj, "DiagnosticsState");
			if(paramValue != NULL) {
				if(!strcmp(json_object_get_string(paramValue), "Requested")){
					if(zcfg_msg_eidGet() == ZCFG_EID_TR69){
						json_object_object_add(udpEchoDiagObj, "X_ZYXEL_Creator", json_object_new_string(CREATOR_ACS));
					}
				}
			}
		}

		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(udpEchoDiagObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(udpEchoDiagObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	/*Set Device.Routing.Router.1.IPv4Forwarding.i*/
	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_DIAG_UDP_ECHO_DIAG, &objIid, udpEchoDiagObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(udpEchoDiagObj);
			return ret;
		}
		json_object_put(udpEchoDiagObj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_UDPEchoDiagnostics.IndividualPacketResult.{i}
 *
 *   Related object in TR181:
 *   Device.IP.Diagnostics.UDPEchoDiagnostics.IndividualPacketResult.{i}
 */
zcfgRet_t udpEchoDiagPakRstObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *udpEchoPakRstObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	const char *strP = strstr(tr98FullPathName, "IndividualPacketResult");
	if(!strP)
		return ZCFG_NO_SUCH_OBJECT;

	IID_INIT(objIid);
	objIid.level = 1;
	if(sscanf(strP, "IndividualPacketResult.%hhu", &objIid.idx[0]) != 1)
		return ZCFG_NO_SUCH_OBJECT;

	if((ret = feObjJsonGet(RDM_OID_IP_DIAG_UDP_ECHO_PAK_RST, &objIid, &udpEchoPakRstObj, updateFlag)) != ZCFG_SUCCESS){
		ztrdbg(ZTR_DL_DEBUG, "%s: retrieve RDM_OID_IP_DIAG_UDP_ECHO_PAK_RST fail!\n", __FUNCTION__);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(udpEchoPakRstObj, paramList->name);
		if(!paramValue) {
			printf("Can't find parameter %s in TR181\n", paramList->name);
			paramList++;
			continue;
		}

		json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		paramList++;
	}

	json_object_put(udpEchoPakRstObj);

	return ZCFG_SUCCESS;
}
