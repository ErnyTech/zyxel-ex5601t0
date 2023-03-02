
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t D_DNS_param[] =
{
	{"Enable", 			dalType_boolean,	0,	0,		NULL, NULL,		0},
#ifdef ABOG_CUSTOMIZATION
	{"ServiceProvider", 		dalType_string, 	1, 	32,		NULL, "dyndns|zoneedit|noip|dtdns|freedns|dynv6|dynu|userdefined",		0},
#elif ZYXEL_USA_PRODUCT
	{"ServiceProvider", 		dalType_string, 	1,	32,		NULL, "dyndns|zoneedit|noip|dtdns|freedns|dynu|userdefined|dnsomatic",       0},
#else
	{"ServiceProvider", 		dalType_string, 	1, 	32,		NULL, "dyndns|zoneedit|noip|dtdns|freedns|dynu|dyndnsit|userdefined",		0},
#endif
	{"HostName", 			dalType_string, 	0, 	256,	NULL, NULL,		0},
	{"UserName", 			dalType_string, 	0, 	256, 	NULL, NULL,		0},
	{"Password", 			dalType_string, 	0, 	64, 		NULL, NULL,		0},
	{"IPAddressPolicy", 		dalType_int,		1,	1,		NULL, NULL,		dalcmd_Forbid},
	{"UserIPAddress", 		dalType_v4Addr,	0,	0,		NULL, NULL,		dalcmd_Forbid},
	{"Wildcard", 			dalType_boolean,	0,	0,		NULL, NULL,		0},
	{"Offline", 			dalType_boolean,	0,	0,		NULL, NULL,		0},
	{"UpdateURL", 		dalType_string, 	0, 	256,	NULL, NULL,		0},
	{"ConnectionType", 	dalType_string, 	0, 	12,		NULL, "HTTP|HTTPS",		0},
#if (defined(CAAAE_CUSTOMIZATION) || defined(CAAAD_CUSTOMIZATION))
	{"Interface",                    dalType_string,         0,      32,    NULL, NULL,             0},
#endif
	{NULL, 					0, 					0, 	0, 		NULL, NULL,		0}
};

void zcfgFeDalShowDDns(struct json_object *Jarray){
	struct json_object *Jobj = NULL;
		
	if(json_object_get_type(Jarray) != json_type_array){
		printf("%s: wrong Jobj format!\n", __FUNCTION__);
		return;
	}

	Jobj = json_object_array_get_idx(Jarray, 0);
	
	printf("%-25s %u\n", "Enable", json_object_get_boolean(json_object_object_get(Jobj, "Enable")));
	printf("%-25s %s\n", "ServiceProvider", json_object_get_string(json_object_object_get(Jobj, "ServiceProvider")));
	printf("%-25s %s\n", "HostName", json_object_get_string(json_object_object_get(Jobj, "HostName")));
	printf("%-25s %s\n", "UserName", json_object_get_string(json_object_object_get(Jobj, "UserName")));
	printf("%-25s %s\n", "Password", json_object_get_string(json_object_object_get(Jobj, "Password")));
	printf("%-25s %u\n", "Wildcard", json_object_get_boolean(json_object_object_get(Jobj, "Wildcard")));
	printf("%-25s %u\n", "Offline", json_object_get_boolean(json_object_object_get(Jobj, "Offline")));
	printf("\n");
	printf("%-32s %s\n", "User Authentication Result :", json_object_get_string(json_object_object_get(Jobj, "AuthenticationResult")));
	printf("%-32s %s\n", "Last Updated Time :", json_object_get_string(json_object_object_get(Jobj, "UpdatedTime")));
	printf("%-32s %s\n", "Current Dynamic IP :", json_object_get_string(json_object_object_get(Jobj, "DynamicIP")));
	
}

zcfgRet_t zcfgFeDalDDnsGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *paramJobj = NULL;
	struct json_object *obj = NULL;
	paramJobj = json_object_new_object();
	IID_INIT(iid);
	if(zcfgFeObjJsonGet(RDM_OID_D_DNS, &iid, &obj) == ZCFG_SUCCESS) {
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(obj, "Enable")));
		json_object_object_add(paramJobj, "ServiceProvider", JSON_OBJ_COPY(json_object_object_get(obj, "ServiceProvider")));
		json_object_object_add(paramJobj, "HostName", JSON_OBJ_COPY(json_object_object_get(obj, "HostName")));
		json_object_object_add(paramJobj, "UserName", JSON_OBJ_COPY(json_object_object_get(obj, "UserName")));
		json_object_object_add(paramJobj, "Password", JSON_OBJ_COPY(json_object_object_get(obj, "Password")));
		json_object_object_add(paramJobj, "Wildcard", JSON_OBJ_COPY(json_object_object_get(obj, "Wildcard")));
		json_object_object_add(paramJobj, "Offline", JSON_OBJ_COPY(json_object_object_get(obj, "Offline")));
		json_object_object_add(paramJobj, "UpdateURL", JSON_OBJ_COPY(json_object_object_get(obj, "UpdateURL")));
		json_object_object_add(paramJobj, "ConnectionType", JSON_OBJ_COPY(json_object_object_get(obj, "ConnectionType")));
		json_object_object_add(paramJobj, "AuthenticationResult", JSON_OBJ_COPY(json_object_object_get(obj, "AuthenticationResult")));
		json_object_object_add(paramJobj, "UpdatedTime", JSON_OBJ_COPY(json_object_object_get(obj, "UpdatedTime")));
		json_object_object_add(paramJobj, "DynamicIP", JSON_OBJ_COPY(json_object_object_get(obj, "DynamicIP")));
#if (defined(CAAAE_CUSTOMIZATION) || defined(CAAAD_CUSTOMIZATION))
		json_object_object_add(paramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(obj, "Interface")));
#endif
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(obj);
	}
	return ret;
}

zcfgRet_t zcfgFeDalDDnsEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid = {0};
	struct json_object *ddnsJobj = NULL;
	bool enable = false;
	const char *serviceprovider = NULL;
	const char *hostname = NULL;
	const char *username = NULL;
	const char *password = NULL;
	bool wildcard = false;
	bool offline = false;
	int ipaddrpolicy = 0;
	const char *updateurl = NULL;
	const char *connectiontype = NULL;
#if (defined(CAAAE_CUSTOMIZATION) || defined(CAAAD_CUSTOMIZATION))
	const char *interface = NULL;
#endif

	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	serviceprovider = json_object_get_string(json_object_object_get(Jobj, "ServiceProvider"));
	hostname = json_object_get_string(json_object_object_get(Jobj, "HostName"));
	if(!strcmp(serviceprovider,"dynv6")){
		username = hostname;
	}
	else{
		username = json_object_get_string(json_object_object_get(Jobj, "UserName"));
	}
	password = json_object_get_string(json_object_object_get(Jobj, "Password"));
	wildcard = json_object_get_boolean(json_object_object_get(Jobj, "Wildcard"));
	offline = json_object_get_boolean(json_object_object_get(Jobj, "Offline"));
	updateurl = json_object_get_string(json_object_object_get(Jobj, "UpdateURL"));
	connectiontype = json_object_get_string(json_object_object_get(Jobj, "ConnectionType"));
#if (defined(CAAAE_CUSTOMIZATION) || defined(CAAAD_CUSTOMIZATION))
	interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
#endif

	if(zcfgFeObjJsonGet(RDM_OID_D_DNS, &objIid, &ddnsJobj) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "Enable"))
			json_object_object_add(ddnsJobj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "ServiceProvider"))
			json_object_object_add(ddnsJobj, "ServiceProvider", json_object_new_string(serviceprovider));
		if(json_object_object_get(Jobj, "HostName"))
			json_object_object_add(ddnsJobj, "HostName", json_object_new_string(hostname));
		if(json_object_object_get(Jobj, "UserName"))
			json_object_object_add(ddnsJobj, "UserName", json_object_new_string(username));
		if(json_object_object_get(Jobj, "Password"))
			json_object_object_add(ddnsJobj, "Password", json_object_new_string(password));
		json_object_object_add(ddnsJobj, "IPAddressPolicy", json_object_new_int(ipaddrpolicy));
		if(json_object_object_get(Jobj, "Wildcard"))
			json_object_object_add(ddnsJobj, "Wildcard", json_object_new_boolean(wildcard));
		if(json_object_object_get(Jobj, "Offline"))
			json_object_object_add(ddnsJobj, "Offline", json_object_new_boolean(offline));
		if(json_object_object_get(Jobj, "UpdateURL"))
			json_object_object_add(ddnsJobj, "UpdateURL", json_object_new_string(updateurl));
		if(json_object_object_get(Jobj, "ConnectionType"))
			json_object_object_add(ddnsJobj, "ConnectionType", json_object_new_string(connectiontype));
#if (defined(CAAAE_CUSTOMIZATION) || defined(CAAAD_CUSTOMIZATION))
		if(json_object_object_get(Jobj, "Interface"))
			json_object_object_add(ddnsJobj, "Interface", json_object_new_string(interface));
#endif
		ret = zcfgFeObjJsonBlockedSet(RDM_OID_D_DNS, &objIid, ddnsJobj, NULL);

		zcfgFeJsonObjFree(ddnsJobj);
	}
		return ret;
	}
	
zcfgRet_t zcfgFeDalDDns(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalDDnsEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalDDnsGet(Jobj, Jarray, replyMsg);
	}

	return ret;
}
