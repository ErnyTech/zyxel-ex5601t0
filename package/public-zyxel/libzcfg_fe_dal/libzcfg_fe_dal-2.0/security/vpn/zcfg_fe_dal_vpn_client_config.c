
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

dal_param_t VPNClient_param[] =
{
	{"vpnClientIndex",			dalType_int,		0,	0,		NULL,	NULL,			dal_Edit|dal_Delete},
	{"vpnClientEnable", 		dalType_boolean,	0,	0,		NULL,	NULL,			0},
	{"vpnClientName",			dalType_string, 	0, 	64,		NULL,	NULL,			0},
	{"vpnClientProtocol",		dalType_string, 	0, 	8,		NULL,	NULL,			0},
	{"vpnClientServer",			dalType_string, 	0, 	64,		NULL,	NULL,			0},
	{"vpnClientUsername",		dalType_string, 	0, 	256,	NULL,	NULL,			0},
	{"vpnClientPassword",		dalType_string, 	0, 	64,		NULL,	NULL,			0},
	{"vpnClientWanInf",			dalType_string,		0, 	256,	NULL,	NULL,			0},
	{"vpnClientPPTPSecurity",	dalType_string,		0, 	256,	NULL,	NULL,			0},
	{NULL, 						0, 					0, 	0,		NULL, 	NULL,			0}
};

static zcfgRet_t validateVPNClientConfig(struct json_object *Jobj, objIndex_t *iid, char* replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t vpnClientCfgIid;
	struct json_object *vpnClientCfgObj = NULL;
	struct json *paramValue = NULL;
	const char *paramName = NULL;
	const char *clientName = NULL;
	const char *clientServer = NULL;
	const char *clientUsername = NULL;
	zcfg_offset_t oid = RDM_OID_VPN_CLIENT_CFG;
	int i;

	i = 1;//Skip vpnClientIndex
	paramName = (const char *)VPNClient_param[i++].paraName;
	while(paramName != NULL){
		paramValue = json_object_object_get(Jobj, paramName);
		if(paramValue == NULL){
			printf("\n Value of parameter:%s is invalid", paramName);
			ret = ZCFG_INVALID_PARAM_VALUE;
			return ret;
		} else {
			paramName = (const char *)VPNClient_param[i++].paraName;
		}
	}

	/* Check duplicated client name */
	clientName = json_object_get_string(json_object_object_get(Jobj, "vpnClientName"));
	ret = dalcmdParamCheck(iid, clientName, oid, "Name", "Name", replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	clientServer = json_object_get_string(json_object_object_get(Jobj, "vpnClientServer"));
	clientUsername = json_object_get_string(json_object_object_get(Jobj, "vpnClientUsername"));
	IID_INIT(vpnClientCfgIid);
	while((zcfgFeObjJsonGetNextWithoutUpdate(oid, &vpnClientCfgIid, &vpnClientCfgObj)) == ZCFG_SUCCESS)
	{
		/* When Editing config entry, skip checking the entry configure we are editing */
		if(iid != NULL)
		{
			if( vpnClientCfgIid.idx[0] == iid->idx[0])
			{
				continue;
			}
		}

		/* Checking the  same Username on the same Server */
		if(!strcmp(clientServer, json_object_get_string(json_object_object_get(vpnClientCfgObj, "Server"))))
		{
			if(!strcmp(clientUsername, json_object_get_string(json_object_object_get(vpnClientCfgObj, "Username"))))
			{
				strcpy(replyMsg, "Duplicated user for the same VPN server");
				ret = ZCFG_INVALID_PARAM_VALUE;
			}
		}

		zcfgFeJsonObjFree(vpnClientCfgObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalVPNClientEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t vpnClientCfgIid;
	struct json_object *vpnClientCfgObj = NULL;
	zcfg_offset_t oid = RDM_OID_VPN_CLIENT_CFG;
	int index;


	IID_INIT(vpnClientCfgIid);
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(index, &vpnClientCfgIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	ret = validateVPNClientConfig(Jobj, &vpnClientCfgIid, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	if((ret = zcfgFeObjJsonGetWithoutUpdate(oid, &vpnClientCfgIid, &vpnClientCfgObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get VPN client config obj fail oid(%d)\n ",__FUNCTION__,RDM_OID_DNS_RT_ENTRY);
		return ret;
	} else {

		json_object_object_add(vpnClientCfgObj, "Enable", json_object_new_boolean(json_object_get_boolean(json_object_object_get(Jobj, "vpnClientEnable"))));
		json_object_object_add(vpnClientCfgObj, "Name", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientName"))));
		json_object_object_add(vpnClientCfgObj, "VPNProtocol", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientProtocol"))));
		json_object_object_add(vpnClientCfgObj, "Server", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientServer"))));
		json_object_object_add(vpnClientCfgObj, "Username", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientUsername"))));
		json_object_object_add(vpnClientCfgObj, "Password", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientPassword"))));
		json_object_object_add(vpnClientCfgObj, "WanInf", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientWanInf"))));
		json_object_object_add(vpnClientCfgObj, "LanInf", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientLanInf"))));
		json_object_object_add(vpnClientCfgObj, "pptpSecurity", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientPPTPSecurity"))));

		if((ret = zcfgFeObjJsonSet(oid, &vpnClientCfgIid, vpnClientCfgObj, NULL)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, oid);
		}
		zcfgFeJsonObjFree(vpnClientCfgObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalVPNClientAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t vpnClientCfgIid;
	struct json_object *vpnClientCfgObj = NULL;
	zcfg_offset_t oid = RDM_OID_VPN_CLIENT_CFG;

	ret = validateVPNClientConfig(Jobj, NULL, replyMsg);

	if(ret != ZCFG_SUCCESS)
		return ret;


	IID_INIT(vpnClientCfgIid);
	if(zcfgFeObjJsonAdd(oid, &vpnClientCfgIid, NULL) == ZCFG_SUCCESS){
		if(zcfgFeObjJsonGetWithoutUpdate(oid, &vpnClientCfgIid, &vpnClientCfgObj) == ZCFG_SUCCESS){
			json_object_object_add(vpnClientCfgObj, "Enable", json_object_new_boolean(json_object_get_boolean(json_object_object_get(Jobj, "vpnClientEnable"))));
			json_object_object_add(vpnClientCfgObj, "Name", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientName"))));
			json_object_object_add(vpnClientCfgObj, "VPNProtocol", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientProtocol"))));
			json_object_object_add(vpnClientCfgObj, "Server", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientServer"))));
			json_object_object_add(vpnClientCfgObj, "Username", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientUsername"))));
			json_object_object_add(vpnClientCfgObj, "Password", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientPassword"))));
			json_object_object_add(vpnClientCfgObj, "WanInf", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientWanInf"))));
			json_object_object_add(vpnClientCfgObj, "LanInf", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientLanInf"))));
			json_object_object_add(vpnClientCfgObj, "pptpSecurity", json_object_new_string(json_object_get_string(json_object_object_get(Jobj, "vpnClientPPTPSecurity"))));

			ret = zcfgFeObjJsonSet(oid, &vpnClientCfgIid, vpnClientCfgObj, NULL);
			if(ret != ZCFG_SUCCESS) {
				printf("%s: Write RDM_OID_VPN_CLIENT_CFG object fail\n", __FUNCTION__);
			}
		}

		zcfgFeJsonObjFree(vpnClientCfgObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalVPNClientDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t vpnClientIid;
	int index = 0;
	zcfg_offset_t oid = RDM_OID_VPN_CLIENT_CFG;

	IID_INIT(vpnClientIid);
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));

	ret = convertIndextoIid(index, &vpnClientIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	if((ret = zcfgFeObjJsonDel(oid, &vpnClientIid, NULL)) != ZCFG_SUCCESS) {
		printf("%s: delete VPN client config obj fail\n", __FUNCTION__);
	}

	return ret;
}

zcfgRet_t zcfgFeDalVPNClientGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t vpnClientIid;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;
	int status;
	int count = 1;
	const char *type;
	zcfg_offset_t oid = RDM_OID_VPN_CLIENT_CFG;

	IID_INIT(vpnClientIid);
	while(zcfgFeObjJsonGetNext(oid, &vpnClientIid, &obj) == ZCFG_SUCCESS) {
		type = json_object_get_string(json_object_object_get(obj, "VPNProtocol"));
		status = json_object_get_int(json_object_object_get(obj, "Status"));

		pramJobj = json_object_new_object();
		json_object_object_add(pramJobj, "vpnClientIndex", json_object_new_int(count));
		json_object_object_add(pramJobj, "vpnClientEnable", JSON_OBJ_COPY(json_object_object_get(obj, "Enable")));
		json_object_object_add(pramJobj, "vpnClientName", JSON_OBJ_COPY(json_object_object_get(obj, "Name")));
		if(!strcmp(type, "PPTP"))
			json_object_object_add(pramJobj, "vpnClientProtocol", json_object_new_string("PPTP"));
		else if(!strcmp(type, "L2TP"))
			json_object_object_add(pramJobj, "vpnClientProtocol", json_object_new_string("L2TP"));
		else
			json_object_object_add(pramJobj, "vpnClientProtocol", json_object_new_string("Others"));

		json_object_object_add(pramJobj, "vpnClientServer", JSON_OBJ_COPY(json_object_object_get(obj, "Server")));
		json_object_object_add(pramJobj, "vpnClientUsername", JSON_OBJ_COPY(json_object_object_get(obj, "Username")));
		json_object_object_add(pramJobj, "vpnClientPassword", JSON_OBJ_COPY(json_object_object_get(obj, "Password")));
		json_object_object_add(pramJobj, "vpnClientWanInf", JSON_OBJ_COPY(json_object_object_get(obj, "WanInf")));
		json_object_object_add(pramJobj, "vpnClientLanInf", JSON_OBJ_COPY(json_object_object_get(obj, "LanInf")));
		json_object_object_add(pramJobj, "vpnClientPPTPSecurity", JSON_OBJ_COPY(json_object_object_get(obj, "pptpSecurity")));

		if(status == 1) {
			json_object_object_add(pramJobj, "vpnClientStatus", json_object_new_string("Disconnected"));
		} else if (status == 2) {
			json_object_object_add(pramJobj, "vpnClientStatus", json_object_new_string("Connecting"));
		} else if (status == 3){
			json_object_object_add(pramJobj, "vpnClientStatus", json_object_new_string("Connected"));
		} else { //Unknown
			json_object_object_add(pramJobj, "vpnClientStatus", json_object_new_string("Disconnected"));
		}

		json_object_object_add(pramJobj, "vpnClientLocalIP", JSON_OBJ_COPY(json_object_object_get(obj, "LocalIP")));
		json_object_object_add(pramJobj, "vpnClientRemoteIP", JSON_OBJ_COPY(json_object_object_get(obj, "RemotePeerIP")));


        if (json_object_object_get(Jobj, "Index")) {
			if(count == json_object_get_int(json_object_object_get(Jobj, "Index"))) {
				json_object_array_add(Jarray, pramJobj);
				zcfgFeJsonObjFree(obj);
				break;
			}
		} else {
			json_object_array_add(Jarray, pramJobj);
		}

		zcfgFeJsonObjFree(obj);
		count++;
	}

	return ret;
}

void zcfgFeDalShowVPNClient(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("\n%-6s %-9s %-30s %-9s %-15s %-20s %-15s% -15s %-10s %-9s\n",
			"Index","Active","Name","VPNType","Server","UserName","WANIntf","LANIntf","Security","Status");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-6s %-9s %-30s %-9s %-15s %-20s %-15s% -15s %-10s %-9s\n",
			json_object_get_string(json_object_object_get(obj, "vpnClientIndex")),
			json_object_get_string(json_object_object_get(obj, "vpnClientEnable")),
			json_object_get_string(json_object_object_get(obj, "vpnClientName")),
			json_object_get_string(json_object_object_get(obj, "vpnClientProtocol")),
			json_object_get_string(json_object_object_get(obj, "vpnClientServer")),
			json_object_get_string(json_object_object_get(obj, "vpnClientUsername")),
			json_object_get_string(json_object_object_get(obj, "vpnClientWanInf")),
			json_object_get_string(json_object_object_get(obj, "vpnClientLanInf")),
			json_object_get_string(json_object_object_get(obj, "vpnClientPPTPSecurity")),
			json_object_get_string(json_object_object_get(obj, "vpnClientStatus"))
		);
	}
}

zcfgRet_t zcfgFeDalVPNClient(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalVPNClientEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalVPNClientAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalVPNClientDelete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalVPNClientGet(Jobj, Jarray, replyMsg);
	}

	return ret;
}
