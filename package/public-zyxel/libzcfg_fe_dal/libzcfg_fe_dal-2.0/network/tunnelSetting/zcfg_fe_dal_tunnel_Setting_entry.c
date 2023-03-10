
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_msg.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

// TODO: need to add the WAN interface
dal_param_t TunnelSet_param[] =
{
	{"Index",		dalType_int,	0,	0,		NULL,	NULL,	dal_Edit|dal_Delete},
	{"Enable",	dalType_boolean,	0,	0,	NULL,	NULL,	dal_Add},
	{"RemoteEndpoints",	dalType_v4Addr,	0,	0,	NULL,	NULL,	dal_Add},
	{"X_ZYXEL_RemoteEndpoint2", dalType_v4Addr, 0,	0,	NULL,	NULL,	dal_Add},
	{"connectedRemoteEndpoint", dalType_v4Addr, 0,	0,	NULL,	NULL,	dal_Add},
	{"wanIntf",	dalType_IntfGrpWanList, 0, 0, NULL,	NULL,	dal_Add},
	{"X_ZYXEL_LowerLayers", dalType_IntfGrpLanPortList, 0,	0,	NULL,	NULL,	dal_Add},
	{"X_ZYXEL_ShapingRate", dalType_int, 0,	1024000,	NULL,	NULL,	dal_Add},
	{"X_ZYXEL_DownstreamShapingRate", dalType_int, 0,	1024000,	NULL,	NULL,	dal_Add},
	{"X_ZYXEL_DSCP", dalType_int, 0,	63,	NULL,	NULL,	dal_Add},
	{NULL, 			0, 				0, 	0, 		NULL,	NULL,	NULL}
};

void do_get_json_array_proc(json_object *ArrayObj, int length, int *output_array) {
	int i = 0;
	for(i = 0; i < length; i++) {
		output_array[i] = json_object_get_int(json_object_array_get_idx(ArrayObj, i));
	}
}

void zcfgFeDalShowTunnelSetting(struct json_object *Jarray){
	//printf("%s(%d)\n\r", __FUNCTION__, __LINE__);
	int i, len = 0;
	struct json_object *obj;
	const char *ipver = NULL;


	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-6s %-12s %-35s %-55s %-35s %-35s \n",
		    "Index","Enable", "WAN Interface", "Tunnel Interfaces", "RemoteEndpoint", "QoS");


	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		bool enable = false;
		const char *wanInterface = NULL;
		const char *tunelInterface = NULL;
		const char *endpoint = NULL;
		const char *qos = NULL;
		int shapingRate = 0;
		int downshapingRate = 0;
		const char *upRate = NULL;
		const char *downRate = NULL;

		enable = json_object_get_boolean(json_object_object_get(obj, "Enable"));
		wanInterface = json_object_get_string(json_object_object_get(obj, "wanIntf"));
		tunelInterface = json_object_get_string(json_object_object_get(obj, "lanIntf"));
		endpoint = json_object_get_string(json_object_object_get(obj, "RemoteEndpoints"));
		shapingRate = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_ShapingRate"));
		downshapingRate = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_DownstreamShapingRate"));
		sprintf(upRate, "%d", shapingRate);
		sprintf(downRate, "%d", downshapingRate);
		sprintf(qos,"%s", upRate);
		strcat(qos, "Kbps / ");
		strcat(qos, downRate);
		strcat(qos, "Kbps");

		if(!strcmp(wanInterface,""))
			wanInterface = "Hotspot";

		if(!strcmp(tunelInterface,""))
			tunelInterface = "N/A";

		if(!strcmp(qos,""))
			qos = "N/A";

		printf("%-6s %-12s %-35s %-55s %-35s %-35s \n",
				json_object_get_string(json_object_object_get(obj, "Index")),
			    	json_object_get_boolean(json_object_object_get(obj, "Enable")),
			   	 wanInterface,
			    	tunelInterface,
			    	endpoint,
			    	qos);
	}
}

zcfgRet_t zcfgFeDalTunnelSettingAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *tunnelSetObj = NULL;
	struct json_object *oldBrPortObj = NULL;
	struct json_object *newBrObj = NULL;
	struct json_object *newBrPortObj = NULL;
	objIndex_t tunnelSetIid = {0};
	objIndex_t newBrIid = {0};
	objIndex_t newBrPortIid = {0};
	bool enable = false;
	const char *remoteEndpoints = NULL;
	const char *remoteEndpoint2 = NULL;
	const char *conectRemoteEndpoint = NULL;
	const char *lanIntf = NULL;
	char tunnelName[32] = {0};
	int shapingRate = 0;
	int downstreamShapingRate = 0;
	int dscp = 0;
	const char *LowerLayer = NULL;
	char lanIntftmp[100] = {0}, lowerLayers[100] = {0};
	char *temp = NULL, *lanIntfStr;
	const char *wanintf = NULL;
	zcfg_offset_t oid;
	oid = RDM_OID_GRE_TUNNEL;

	// POST RDM_OID_GRE_TUNNEL
	IID_INIT(tunnelSetIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_GRE_TUNNEL, &tunnelSetIid, NULL)) != ZCFG_SUCCESS) {
		return ret;
	}
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GRE_TUNNEL, &tunnelSetIid, &tunnelSetObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	remoteEndpoints = json_object_get_string(json_object_object_get(Jobj, "RemoteEndpoints"));
	remoteEndpoint2 = json_object_get_string(json_object_object_get(Jobj, "X_ZYXEL_RemoteEndpoint2"));
	conectRemoteEndpoint = json_object_get_string(json_object_object_get(Jobj, "connectedRemoteEndpoint"));
	lanIntf = json_object_get_string(json_object_object_get(Jobj, "lanIntf"));
	shapingRate = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_ShapingRate"));
	downstreamShapingRate = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_DownstreamShapingRate"));
	dscp = json_object_get_int(json_object_object_get(Jobj, "dscp"));
	wanintf = json_object_get_string(json_object_object_get(Jobj, "wanIntf"));

	strcpy(lanIntftmp, lanIntf);
	lanIntfStr = strtok_r(lanIntftmp, ",", &temp);
	while(lanIntfStr != NULL) {
		IID_INIT(newBrPortIid);
		newBrPortIid.level = 2;

		if(sscanf(lanIntfStr, "Bridging.Bridge.%hhu.Port.%hhu", &newBrPortIid.idx[0], &newBrPortIid.idx[1]) == 2){
			if(zcfgFeObjJsonGet(RDM_OID_BRIDGING_BR_PORT, &newBrPortIid, &oldBrPortObj) == ZCFG_SUCCESS){
				//printf("zcfgFeDalTunnelSettingAdd level:%d idx[0]:%d idx[1]:%d idx[2]:%d idx[3]:%d idx[4]:%d idx[5]:%d, %s(%d)\n", newBrPortIid.level, newBrPortIid.idx[0], newBrPortIid.idx[1], newBrPortIid.idx[2], newBrPortIid.idx[3], newBrPortIid.idx[4], newBrPortIid.idx[5], __FUNCTION__, __LINE__);
				LowerLayer = json_object_get_string(json_object_object_get(oldBrPortObj, "LowerLayers"));

				if(lowerLayers[0] == '\0'){
					sprintf(lowerLayers, "%s", LowerLayer);
				}
				else{
					strcat(lowerLayers, ",");
					strcat(lowerLayers, LowerLayer);
				}
				zcfgFeJsonObjFree(oldBrPortObj);
			}
		}
		lanIntfStr = strtok_r(temp, ",", &temp);
	}
	


	json_object_object_add(tunnelSetObj, "Enable", json_object_new_boolean(enable));
	json_object_object_add(tunnelSetObj, "RemoteEndpoints", json_object_new_string(remoteEndpoints));
	json_object_object_add(tunnelSetObj, "X_ZYXEL_RemoteEndpoint2", json_object_new_string(remoteEndpoint2));
	json_object_object_add(tunnelSetObj, "ConnectedRemoteEndpoint", json_object_new_string(conectRemoteEndpoint));
	json_object_object_add(tunnelSetObj, "X_ZYXEL_LowerLayers", json_object_new_string(lowerLayers));
	json_object_object_add(tunnelSetObj, "X_ZYXEL_ShapingRate", json_object_new_int(shapingRate));
	json_object_object_add(tunnelSetObj, "X_ZYXEL_DownstreamShapingRate", json_object_new_int(downstreamShapingRate));
	json_object_object_add(tunnelSetObj, "X_ZYXEL_DSCP", json_object_new_int(dscp));
	json_object_object_add(tunnelSetObj, "X_ZYXEL_GREInterface", json_object_new_string(wanintf));

	zcfgFeObjJsonSet(RDM_OID_GRE_TUNNEL, &tunnelSetIid, tunnelSetObj, NULL);

	if(tunnelSetObj){
		zcfgFeJsonObjFree(tunnelSetObj);
	}

	return ret;
}
zcfgRet_t zcfgFeDalTunnelSettingEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *tunnelSetObj = NULL;
	struct json_object *org_obj = NULL;
	struct json_object *BrFilterObj = NULL;
	objIndex_t tunnelSetIid = {0};
	objIndex_t BrFilterIid = {0};
	bool enable = false;
	const char *remoteEndpoints = NULL;
	const char *remoteEndpoint2 = NULL;
	const char *conectRemoteEndpoint = NULL;
	char *greBridgeName = NULL;  //raghu
	const char *lanIntf = NULL;
	const char *modify_str = NULL;
	// const char *org_lanIntf;
	int shapingRate = 0;
	int downstreamShapingRate = 0;
	int dscp = 0;
	int index = 0;
	bool modify = false;
	bool find = false;
	char *orglanIntf_tmp = NULL;
	char *lanIntf_tmp = NULL;
	char *rest_str = NULL;
	char *single_lanIntf = NULL;
	char *BrFilter_interface = NULL;
	char brPathName[30] = {0};
	char temp[30] = {0};
	zcfg_offset_t oid;
	oid = RDM_OID_GRE_TUNNEL;
	struct json_object *IidArrayObj = NULL;
	int i, len = 0, old_lid = 0, old_port = 0;
	int *array = NULL;
	char lanIntftmp[100] = {0}, lowerLayers[100] = {0};
	char *tmpstr = NULL, *lanIntfStr;
	const char *LowerLayer = NULL;
	const char *wanintf = NULL;
	objIndex_t newBrPortIid = {0};
	struct json_object *oldBrPortObj = NULL;
	struct json_object *BridgeFilterObj = NULL;
	objIndex_t BridgeFilterIid = {0};
	char buf[512] = {0}, brPath[32] = {0};
	int brIndex = 0;
	const char *ptr = NULL, *tmp = NULL;
	bool lanIntfSet = false;


	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	tunnelSetIid.level = 1;
	tunnelSetIid.idx[0] = index;

	//printf("zcfgFeDalTunnelSettingEdit level:%d idx[0]:%d idx[1]:%d idx[2]:%d idx[3]:%d idx[4]:%d idx[5]:%d, %s(%d)\n", tunnelSetIid.level, tunnelSetIid.idx[0], tunnelSetIid.idx[1], tunnelSetIid.idx[2], tunnelSetIid.idx[3], tunnelSetIid.idx[4], tunnelSetIid.idx[5], __FUNCTION__, __LINE__);
	if(zcfgFeObjJsonGet(RDM_OID_GRE_TUNNEL, &tunnelSetIid, &tunnelSetObj) == ZCFG_SUCCESS){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
		remoteEndpoints = json_object_get_string(json_object_object_get(Jobj, "RemoteEndpoints"));
		remoteEndpoint2 = json_object_get_string(json_object_object_get(Jobj, "X_ZYXEL_RemoteEndpoint2"));
		conectRemoteEndpoint = json_object_get_string(json_object_object_get(Jobj, "connectedRemoteEndpoint"));
		lanIntf = json_object_get_string(json_object_object_get(Jobj, "lanIntf"));
		shapingRate = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_ShapingRate"));
		downstreamShapingRate = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_DownstreamShapingRate"));
		dscp = json_object_get_int(json_object_object_get(Jobj, "dscp"));
		wanintf = json_object_get_string(json_object_object_get(Jobj, "wanIntf"));
	
		IID_INIT(BridgeFilterIid);
		greBridgeName = json_object_get_string(json_object_object_get(tunnelSetObj, "X_ZYXEL_GREBridgeName"));

		if(greBridgeName != NULL)
		{
			sscanf(greBridgeName, "br%d", &brIndex);
			sprintf(brPath,"Bridging.Bridge.%d",brIndex+1);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_FILTER, &BridgeFilterIid, &BridgeFilterObj) == ZCFG_SUCCESS)
			{
				//Check LAN port if not binding, let it to back bind br0.
				if(!strcmp(json_object_get_string(json_object_object_get(BridgeFilterObj, "Bridge")),brPath))
				{
				strcpy(buf, lanIntf);
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
		}

		strcpy(lanIntftmp, lanIntf);
		lanIntfStr = strtok_r(lanIntftmp, ",", &tmpstr);
		while(lanIntfStr != NULL) {
			IID_INIT(newBrPortIid);
			newBrPortIid.level = 2;

			if(sscanf(lanIntfStr, "Bridging.Bridge.%hhu.Port.%hhu", &newBrPortIid.idx[0], &newBrPortIid.idx[1]) == 2){
				if(zcfgFeObjJsonGet(RDM_OID_BRIDGING_BR_PORT, &newBrPortIid, &oldBrPortObj) == ZCFG_SUCCESS){
					//printf("zcfgFeDalTunnelSettingAdd level:%d idx[0]:%d idx[1]:%d idx[2]:%d idx[3]:%d idx[4]:%d idx[5]:%d, %s(%d)\n", newBrPortIid.level, newBrPortIid.idx[0], newBrPortIid.idx[1], newBrPortIid.idx[2], newBrPortIid.idx[3], newBrPortIid.idx[4], newBrPortIid.idx[5], __FUNCTION__, __LINE__);
					LowerLayer = json_object_get_string(json_object_object_get(oldBrPortObj, "LowerLayers"));
					//printf("LowerLayer = %s, %s(%d)\n\r", LowerLayer, __FUNCTION__, __LINE__);

					if(lowerLayers[0] == '\0'){
						sprintf(lowerLayers, "%s", LowerLayer);
					}
					else{
						strcat(lowerLayers, ",");
						strcat(lowerLayers, LowerLayer);
					}

					zcfgFeJsonObjFree(oldBrPortObj);
				}
			}

			lanIntfStr = strtok_r(tmpstr, ",", &tmpstr);
		}

		// TODO: Janice modify
		modify_str = json_object_get_string(json_object_object_get(Jobj, "__modify"));
		if(!strcmp("true", modify_str)) {
			modify = true;
		}
		else {
			modify = false;
		}

		org_obj = json_object_object_get(Jobj, "org");
		if(org_obj) {
			// org_lanIntf = json_object_get_string(json_object_object_get(org_obj, "lanIntf"));
		}
		else {
			printf("%s(%d)\n\r", __FUNCTION__, __LINE__);
		}

		json_object_object_add(tunnelSetObj, "Enable", json_object_new_boolean(enable));
		if(remoteEndpoints)
			json_object_object_add(tunnelSetObj, "RemoteEndpoints", json_object_new_string(remoteEndpoints));
		if(remoteEndpoint2)
			json_object_object_add(tunnelSetObj, "X_ZYXEL_RemoteEndpoint2", json_object_new_string(remoteEndpoint2));
		if(conectRemoteEndpoint)
			json_object_object_add(tunnelSetObj, "ConnectedRemoteEndpoint", json_object_new_string(conectRemoteEndpoint));
		if(lowerLayers)
			json_object_object_add(tunnelSetObj, "X_ZYXEL_LowerLayers", json_object_new_string(lowerLayers));
		if(wanintf)
			json_object_object_add(tunnelSetObj, "X_ZYXEL_GREInterface", json_object_new_string(wanintf));
		if(shapingRate)
			json_object_object_add(tunnelSetObj, "X_ZYXEL_ShapingRate", json_object_new_int(shapingRate));
		if(downstreamShapingRate)
			json_object_object_add(tunnelSetObj, "X_ZYXEL_DownstreamShapingRate", json_object_new_int(downstreamShapingRate));
		if(dscp)
			json_object_object_add(tunnelSetObj, "X_ZYXEL_DSCP", json_object_new_int(dscp));

		ret = zcfgFeObjJsonSet(oid, &tunnelSetIid, tunnelSetObj, NULL);

		if(tunnelSetObj) zcfgFeJsonObjFree(tunnelSetObj);
		if(BrFilterObj) zcfgFeJsonObjFree(BrFilterObj);
	}

	return ret;
}

bool doFindObjIid(zcfg_offset_t RDM_OID, int index, objIndex_t* Iid, json_object** Obj) {
	int k = 0;
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

	dbg_printf("FindObjIid: find = %d, idx[0] = %d\n", find, Iid->idx[0]);
	return find;
}

zcfgRet_t zcfgFeDalTunnelSettingDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t tunnelEntryIid = {0};
	struct json_object *tunnelSetObj = NULL;
	int index = 0, findIid = 0;
	zcfg_offset_t oid;
	oid = RDM_OID_GRE_TUNNEL;
	char *lowerLayers = NULL;
	char *lanIntf_tmp = NULL;
	char *rest_str = NULL;
	char *single_lanIntf = NULL;
	char *BrPortLayers = NULL;
	objIndex_t newBrPortIid = {0};
	struct json_object *oldBrPortObj = NULL;
	bool find = false;
	objIndex_t BrIid = {0};
	json_object *BrObj = NULL;

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(index, &tunnelEntryIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	zcfgFeObjJsonDel(RDM_OID_GRE_TUNNEL, &tunnelEntryIid, NULL);

	return ret;
}
zcfgRet_t zcfgFeDalTunnelSettingGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *tunnelSetObj = NULL;
	struct json_object *paramJobj = NULL;
	objIndex_t tunnelSetIid = {0};
	objIndex_t IpIfaceIid = {0};
	zcfg_offset_t oid;
	oid = RDM_OID_GRE_TUNNEL;

	int count = 1;
	while(zcfgFeObjJsonGetNext(oid, &tunnelSetIid, &tunnelSetObj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(tunnelSetObj, "Enable")));
		json_object_object_add(paramJobj, "X_ZYXEL_LowerLayers", JSON_OBJ_COPY(json_object_object_get(tunnelSetObj, "X_ZYXEL_LowerLayers")));
		json_object_object_add(paramJobj, "RemoteEndpoints", JSON_OBJ_COPY(json_object_object_get(tunnelSetObj, "RemoteEndpoints")));
		json_object_object_add(paramJobj, "X_ZYXEL_RemoteEndpoint2", JSON_OBJ_COPY(json_object_object_get(tunnelSetObj, "X_ZYXEL_RemoteEndpoint2")));
		json_object_object_add(paramJobj, "connectedRemoteEndpoint", JSON_OBJ_COPY(json_object_object_get(tunnelSetObj, "ConnectedRemoteEndpoint")));		
		json_object_object_add(paramJobj, "X_ZYXEL_ShapingRate", JSON_OBJ_COPY(json_object_object_get(tunnelSetObj, "X_ZYXEL_ShapingRate")));
		json_object_object_add(paramJobj, "X_ZYXEL_DownstreamShapingRate", JSON_OBJ_COPY(json_object_object_get(tunnelSetObj, "X_ZYXEL_DownstreamShapingRate")));
		json_object_object_add(paramJobj, "X_ZYXEL_DSCP", JSON_OBJ_COPY(json_object_object_get(tunnelSetObj, "X_ZYXEL_DSCP")));
		json_object_object_add(paramJobj, "wanIntf", JSON_OBJ_COPY(json_object_object_get(tunnelSetObj, "X_ZYXEL_GREInterface")));
		json_object_object_add(paramJobj, "Index", json_object_new_int(count));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(tunnelSetObj);
		count++;
	}

	return ret;
}

zcfgRet_t zcfgFeDalTunnelSetting(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	//printf("%s(%d)\n\r", __FUNCTION__, __LINE__);
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalTunnelSettingEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalTunnelSettingAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalTunnelSettingDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method,"GET")){
		ret = zcfgFeDalTunnelSettingGet(Jobj, Jarray, NULL);
	}

	return ret;
}
